/**
 * @file ConfigParser.cpp
 * @brief WireGuard 配置解析器实现
 */

#include "ConfigParser.h"
#include <fstream>
#include <sstream>
#include <regex>
#include <stdexcept>
#include <algorithm>

namespace WVLAN {
namespace Core {

std::string ConfigParser::Trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

void ConfigParser::SplitKeyValue(const std::string& line, std::string& key, std::string& value) {
    size_t eqPos = line.find('=');
    if (eqPos == std::string::npos) {
        key = Trim(line);
        value = "";
        return;
    }
    
    key = Trim(line.substr(0, eqPos));
    value = Trim(line.substr(eqPos + 1));
    
    // 移除引号（如果有）
    if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
        value = value.substr(1, value.size() - 2);
    }
}

ParseResult ConfigParser::ParseFromFile(const std::string& filePath) {
    ParseResult result;
    
    std::ifstream file(filePath);
    if (!file.is_open()) {
        result.ErrorMessage = "无法打开文件: " + filePath;
        return result;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    
    result = ParseFromString(buffer.str());
    result.Config.FileName = filePath;
    
    return result;
}

ParseResult ConfigParser::ParseFromString(const std::string& content) {
    ParseResult result;
    result.Config = NetworkConfig();
    
    std::map<std::string, std::string> currentSection;
    std::string currentSectionName;
    std::regex sectionRegex(R"(\[([^\]]+)\])");
    std::regex commentRegex(R"(^[;\#])");
    
    std::istringstream stream(content);
    std::string line;
    int lineNum = 0;
    
    auto processSection = [&]() {
        if (currentSectionName.empty()) return;
        
        if (currentSectionName == "Interface") {
            ParseResult sectionResult = ParseSection("Interface", currentSection, result.Config);
            if (!sectionResult.Success) {
                result = sectionResult;
            }
        } else if (currentSectionName == "Peer") {
            PeerConfig peer;
            for (const auto& kv : currentSection) {
                std::string keyLower = kv.first;
                std::transform(keyLower.begin(), keyLower.end(), keyLower.begin(), ::tolower);
                
                if (keyLower == "publickey") peer.PublicKey = kv.second;
                else if (keyLower == "presharedkey") peer.PresharedKey = kv.second;
                else if (keyLower == "endpoint") peer.Endpoint = kv.second;
                else if (keyLower == "allowedips") peer.AllowedIPs = kv.second;
                else if (keyLower == "persistentkeepalive") {
                    try { peer.PersistentKeepalive = std::stoi(kv.second); }
                    catch (...) { peer.PersistentKeepalive = 0; }
                }
            }
            
            if (!peer.PublicKey.empty()) {
                result.Config.Peers.push_back(peer);
            }
        }
    };
    
    while (std::getline(stream, line)) {
        lineNum++;
        line = Trim(line);
        
        // 跳过空行和注释
        if (line.empty() || std::regex_search(line, commentRegex)) {
            continue;
        }
        
        // 检查是否为段标题
        std::smatch match;
        if (std::regex_match(line, match, sectionRegex)) {
            // 处理前一个段
            processSection();
            
            // 开始新段
            currentSectionName = match[1].str();
            currentSection.clear();
        } else {
            // 解析键值对
            std::string key, value;
            SplitKeyValue(line, key, value);
            
            // 键名转小写
            std::transform(key.begin(), key.end(), key.begin(), ::tolower);
            
            currentSection[key] = value;
        }
    }
    
    // 处理最后一个段
    processSection();
    
    // 验证配置
    if (!Validate(result.Config)) {
        result.ErrorMessage = "配置验证失败";
        return result;
    }
    
    result.Success = true;
    return result;
}

ParseResult ConfigParser::ParseSection(const std::string& sectionName,
                                        const std::map<std::string, std::string>& values,
                                        NetworkConfig& config) {
    ParseResult result;
    result.Success = true;
    
    if (sectionName == "Interface") {
        for (const auto& kv : values) {
            std::string key = kv.first;
            
            if (key == "privatekey") config.Interface.PrivateKey = kv.second;
            else if (key == "address") config.Interface.Address = kv.second;
            else if (key == "dns") config.Interface.DNS = kv.second;
            else if (key == "listenport") {
                try { config.Interface.ListenPort = std::stoi(kv.second); }
                catch (...) { config.Interface.ListenPort = 51820; }
            } else if (key == "mtu") {
                try { config.Interface.MTU = std::stoi(kv.second); }
                catch (...) { config.Interface.MTU = 1420; }
            }
        }
    }
    
    return result;
}

std::string ConfigParser::ExportToString(const NetworkConfig& config) {
    std::ostringstream oss;
    
    // 输出 Interface 段
    oss << "[Interface]\n";
    if (!config.Interface.PrivateKey.empty()) {
        oss << "PrivateKey = " << config.Interface.PrivateKey << "\n";
    }
    if (!config.Interface.Address.empty()) {
        oss << "Address = " << config.Interface.Address << "\n";
    }
    if (!config.Interface.DNS.empty()) {
        oss << "DNS = " << config.Interface.DNS << "\n";
    }
    if (config.Interface.ListenPort > 0) {
        oss << "ListenPort = " << config.Interface.ListenPort << "\n";
    }
    if (config.Interface.MTU > 0) {
        oss << "MTU = " << config.Interface.MTU << "\n";
    }
    
    oss << "\n";
    
    // 输出 Peer 段
    for (const auto& peer : config.Peers) {
        oss << "[Peer]\n";
        if (!peer.PublicKey.empty()) {
            oss << "PublicKey = " << peer.PublicKey << "\n";
        }
        if (!peer.PresharedKey.empty()) {
            oss << "PresharedKey = " << peer.PresharedKey << "\n";
        }
        if (!peer.Endpoint.empty()) {
            oss << "Endpoint = " << peer.Endpoint << "\n";
        }
        if (!peer.AllowedIPs.empty()) {
            oss << "AllowedIPs = " << peer.AllowedIPs << "\n";
        }
        if (peer.PersistentKeepalive > 0) {
            oss << "PersistentKeepalive = " << peer.PersistentKeepalive << "\n";
        }
        oss << "\n";
    }
    
    return oss.str();
}

bool ConfigParser::SaveToFile(const NetworkConfig& config, const std::string& filePath) {
    std::ofstream file(filePath);
    if (!file.is_open()) {
        return false;
    }
    
    file << ExportToString(config);
    file.close();
    return true;
}

bool ConfigParser::Validate(const NetworkConfig& config) {
    // 验证私钥
    if (config.Interface.PrivateKey.empty()) {
        return false;
    }
    
    if (!IsValidKey(config.Interface.PrivateKey)) {
        return false;
    }
    
    // 至少需要一个对等节点
    if (config.Peers.empty()) {
        return false;
    }
    
    // 验证每个对等节点
    for (const auto& peer : config.Peers) {
        if (peer.PublicKey.empty()) {
            return false;
        }
        
        if (!IsValidKey(peer.PublicKey)) {
            return false;
        }
        
        if (!peer.Endpoint.empty() && !IsValidEndpoint(peer.Endpoint)) {
            return false;
        }
    }
    
    return true;
}

bool ConfigParser::IsValidKey(const std::string& key) {
    // WireGuard 密钥是 32 字节的 Base64 编码
    // Base64 编码后应该是 44 个字符（包括填充）
    if (key.length() != 44) {
        return false;
    }
    
    // 检查 Base64 字符集
    std::regex base64Regex(R"(^[A-Za-z0-9+/]+=*$)");
    return std::regex_match(key, base64Regex);
}

bool ConfigParser::IsValidIPAddress(const std::string& ip) {
    // 简单验证 IPv4 地址格式
    std::regex ipv4Regex(R"(^(\d{1,3}\.){3}\d{1,3}(/\d{1,2})?$)");
    return std::regex_match(ip, ipv4Regex);
}

bool ConfigParser::IsValidEndpoint(const std::string& endpoint) {
    // 验证 IP:Port 格式
    std::regex endpointRegex(R"(^(\d{1,3}\.){3}\d{1,3}:\d{1,5}$)");
    return std::regex_match(endpoint, endpointRegex);
}

bool ConfigParser::GenerateKeyPair(std::string& privateKey, std::string& publicKey) {
    // 实际实现需要调用 WireGuard 的密钥生成 API
    // 这里返回 false 表示需要实际实现
    return false;
}

bool ConfigParser::GetPublicKey(const std::string& privateKey, std::string& publicKey) {
    // 实际实现需要调用 WireGuard 的公钥计算 API
    return false;
}

} // namespace Core
} // namespace WVLAN
