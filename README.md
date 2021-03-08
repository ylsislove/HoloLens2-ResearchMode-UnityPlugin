# HoloLens2-ResearchMode-UnityPlugin
本插件可在 Unity 项目中，获取 Hololens2 研究模式下的传感器数据和摄像机。目前已实现 IMU 数据的获取。

## 使用插件
1. 在 Visual Studio 中打开 `HL2RmUnityPlugin`
2. 用 `Release, ARM64` 方式构建解决方案
3. 在 Unity 项目中，创建 `Assets/Plugins/HL2RmUnityPlugin` 文件夹
4. 将构建生成的 `HL2RmUnityPlugin.dll` 复制到 `Assets/Plugins/HL2RmUnityPlugin` 文件夹
5. 在 Unity 脚本中调用 DLL 函数，参考 [IMUMamager.cs](./) 脚本
6. 在菜单栏打开 `Edit -> Project Settings`。在 `Player -> Publishing Settings` 确保以下权限被勾上：
    - InternetClient
    - InternetClientServer
    - PrivateNetworkClientServer
    - WebCam
    - SpatialPerception
7. 构建 Unity 项目
8. 在生成的文件夹中找到 `Package.appxmanifest`，确保以下代码被添加：
```xml
<?xml version="1.0" encoding="utf-8"?>
<Package ... IgnorableNamespaces="... rescap" xmlns:rescap="http://schemas.microsoft.com/appx/manifest/foundation/windows10/restrictedcapabilities">
  ...
  <Capabilities>
    <rescap:Capability Name="perceptionSensorsExperimental" />
    <Capability Name="internetClient" />
    <Capability Name="internetClientServer" />
    <Capability Name="privateNetworkClientServer" />
    <uap2:Capability Name="spatialPerception" />
    <DeviceCapability Name="webcam" />
    <DeviceCapability Name="backgroundSpatialPerception" />
  </Capabilities>
</Package>
```
9. 在 Visual Studio 打开生成文件夹，用 `Release, ARM64` 方式构建解决方案并部署到 Hololens2