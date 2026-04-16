using WVLAN.Client.Models;
using NativeBridge;

namespace WVLAN.Client.Bridge
{
    /// <summary>
    /// C++/CLI 桥接层封装 - 提供 .NET 友好的接口
    /// </summary>
    public static class BridgeWrapper
    {
        /// <summary>
        /// 检查是否以管理员权限运行
        /// </summary>
        public static bool IsRunningAsAdmin()
        {
            return NativeBridge.NetworkOperations.IsRunAsAdmin();
        }

        /// <summary>
        /// 检查 WireGuard 驱动是否已安装
        /// </summary>
        public static bool IsWireGuardDriverInstalled()
        {
            return NativeBridge.WireGuardBridge.IsDriverInstalled();
        }

        /// <summary>
        /// 获取本地 IP 地址列表
        /// </summary>
        public static List<string> GetLocalIPAddresses()
        {
            var ops = new NativeBridge.NetworkOperations();
            return ops.GetLocalIPAddresses().ToList();
        }

        /// <summary>
        /// 检测网络类型
        /// </summary>
        public static Models.NetworkType DetectNetworkType()
        {
            var ops = new NativeBridge.NetworkOperations();
            return (Models.NetworkType)ops.DetectNetworkType();
        }

        /// <summary>
        /// 生成密钥对
        /// </summary>
        public static (string privateKey, string publicKey) GenerateKeyPair()
        {
            var bridge = new NativeBridge.ConfigBridge();
            var keys = bridge.GenerateKeyPair();
            return (keys["PrivateKey"], keys["PublicKey"]);
        }

        /// <summary>
        /// 从私钥计算公钥
        /// </summary>
        public static string GetPublicKeyFromPrivateKey(string privateKey)
        {
            var bridge = new NativeBridge.ConfigBridge();
            return bridge.GetPublicKeyFromPrivateKey(privateKey);
        }
    }
}
