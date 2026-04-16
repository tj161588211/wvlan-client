# WireGuard 桥接测试示例代码

本目录包含用于验证 C++/CLI 桥接层功能的示例代码和测试脚本。

## 文件说明

- `BridgeTestStubs.cpp` - C++/CLI桥接层测试存根
- `BridgeIntegrationTest.cs` - C#集成测试示例
- `README.md` - 测试指南

## 运行测试

```csharp
// C# 测试示例
using WVLAN.Bridge;

namespace WVLAN.Tests.Integration
{
    public class BridgeIntegrationTest
    {
        [TestMethod]
        public void TestNativeBridgeInitialization()
        {
            var bridge = new NativeBridge();
            Assert.IsNotNull(bridge);
            
            bool isAdmin = bridge.IsRunningAsAdmin();
            // 注意：需要管理员权限才能测试完整功能
        }
        
        [TestMethod]
        public async Task TestWireGuardBridgeAsync()
        {
            var bridge = new WireGuardBridge();
            
            // 检查驱动状态
            bool installed = await Task.Run(() => bridge.IsDriverInstalled());
            
            if (installed)
            {
                var interfaces = bridge.GetAllInterfaces();
                Assert.IsNotNull(interfaces);
            }
        }
    }
}
```

## 注意事项

C++/CLI 桥接层测试需要：
1. .NET 6.0 或更高版本运行时
2. 以管理员身份运行（某些操作）
3. WireGuard 驱动已安装（完整功能测试）

---

**最后更新**: 2026-04-15
