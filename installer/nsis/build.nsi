; ============================================================================
; WVLAN - Windows Virtual LAN Installer
; NSIS Script
; ============================================================================
!include "MUI2.nsh"
!include "FileFunc.nsh"
!include "LogicLib.nsh"
!include "WinMessages.nsh"

; 基本信息
Name "WVLAN Client"
OutFile "WVLAN-Setup.exe"
InstallDir "$PROGRAMFILES\WVLAN"
InstallDirRegKey HKLM "Software\WVLAN" "InstallDir"
RequestExecutionLevel admin

; 页面设置
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "${NSISDIR}\Contrib\License.txt"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

; 语言
!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "SimpChinese"

; ============================================================================
; 安装过程
; ============================================================================
Section "WVLAN Client" SEC01
  SetOutPath "$INSTDIR"
  
  ; 主程序
  File /r "src\WpfClient\bin\Release\net8.0-windows\*"
  
  ; 配置文件
  File "config\appsettings.json"
  File "config\wireguard.conf.example"
  
  ; 文档
  File "README.md"
  File "CHANGELOG.md"
  
  ; WireGuard 驱动 (假设已下载)
  ; File "drivers\wireguard-x64.msi"
  
  ; 创建快捷方式
  CreateDirectory "$SMPROGRAMS\WVLAN"
  CreateShortcut "$SMPROGRAMS\WVLAN\WVLAN Client.lnk" "$INSTDIR\WVLAN.exe"
  CreateShortcut "$SMPROGRAMS\WVLAN\Uninstall.lnk" "$INSTDIR\uninstall.exe"
  
  ; 创建桌面快捷方式
  CreateShortcut "$DESKTOP\WVLAN Client.lnk" "$INSTDIR\WVLAN.exe"
  
  ; 注册表
  WriteRegStr HKLM "Software\WVLAN" "InstallDir" "$INSTDIR"
  WriteRegStr HKLM "Software\WVLAN" "Version" "1.0.0"
  
  ; 创建卸载程序
  WriteUninstaller "$INSTDIR\uninstall.exe"
  
  ; 设置文件关联 (可选)
  ; WriteRegStr HKCR ".wg" "WVLAN.Config" ""
  ; WriteRegStr HKCR "WVLAN.Config" "" "WireGuard Configuration"
  
  ; 通知用户
  MessageBox MB_ICONINFORMATION "WVLAN 安装完成！\n\n请重启计算机后运行 WVLAN Client。"
SectionEnd

; ============================================================================
; 卸载过程
; ============================================================================
Section "Uninstall"
  ; 删除文件
  RMDir /r "$INSTDIR"
  
  ; 删除快捷方式
  Delete "$SMPROGRAMS\WVLAN\WVLAN Client.lnk"
  Delete "$SMPROGRAMS\WVLAN\Uninstall.lnk"
  RMDir "$SMPROGRAMS\WVLAN"
  
  Delete "$DESKTOP\WVLAN Client.lnk"
  
  ; 删除注册表
  DeleteRegKey HKLM "Software\WVLAN"
  
  ; 清理
  RMDir "$INSTDIR"
  
  MessageBox MB_ICONINFORMATION "WVLAN 已完全卸载。"
SectionEnd

; ============================================================================
; 自定义安装逻辑 (可选)
; ============================================================================
Function .onInit
  ; 检查管理员权限
  IfAdmin 0 noAdmin
  goto continue
  
  noAdmin:
    MessageBox MB_OK|MB_ICONSTOP "需要管理员权限才能安装 WVLAN。$\n$\n请点击"确定"后以管理员身份重新运行。"
    Quit
  
  continue:
FunctionEnd

Function .onInstSuccess
  ; 安装成功后询问是否立即运行
  MessageBox MB_YESNO|MB_ICONQUESTION "是否立即启动 WVLAN?" IDNO noRun
    Exec "$INSTDIR\WVLAN.exe"
  noRun:
FunctionEnd
