; ============================================================================
; WVLAN - Windows Virtual LAN Installer
; Inno Setup Script
; ============================================================================
#define MyAppName "WVLAN Client"
#define MyAppVersion "1.0.0"
#define MyAppPublisher "WVLAN Team"
#define MyAppExeName "WVLAN.exe"

[Setup]
AppId={{A8F5F167-F4CC-42cc-BF3D-0E6B9C7D8E9F}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
Compression=lzma2/ultra64
SolidCompression=yes
WizardStyle=modern
OutputDir=..\output
OutputBaseFilename=WVLAN-Setup-{#MyAppVersion}
DisableProgramGroupPage=yes
PrivilegesRequired=admin
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64
UninstallDisplayIcon={app}\{#MyAppExeName}
LicenseFile=LICENSE
InfoBeforeMsg=本软件需要管理员权限来安装网络驱动。\n\n请继续以管理员身份运行此安装程序。

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "simpchinese"; MessagesFile: "compiler:Languages\SimpChinese.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked onlyuser
Name: "run"; Description: "{cm:RunProgramAfterInstallation}"; Flags: unchecked

[Files]
Source: "..\wvlan-client\src\WpfClient\bin\Release\net8.0-windows\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "config\appsettings.json"; DestDir: "{app}"; Flags: ignoreversion
Source: "config\wireguard.conf.example"; DestDir: "{app}\examples"; Flags: ignoreversion
Source: "README.md"; DestDir: "{app}"; Flags: ignoreversion
Source: "CHANGELOG.md"; DestDir: "{app}"; Flags: ignoreversion

; 注意：确保在编译前已将 WireGuard 驱动文件准备好
; Source: "drivers\wireguard-installer.msi"; DestDir: "{tmp}"; Flags: deleteafterinstall

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon
Name: "{autocmdprograms}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: quicklaunchicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent; Tasks: run

[ uninstall ]
; 自定义卸载逻辑

[Code]
// 检测并请求管理员权限
procedure InitializeSetup();
begin
  if not IsAdminLoggedOn() then begin
    MsgBox('本安装程序需要管理员权限。' + #13#10 + '请点击"确定"后，重新以管理员身份运行。', mbError, MB_OK);
    Log('Administrator privileges required but not present.');
    Result := false;
    Exit;
  end;
end;

// 安装完成后检查 WireGuard 驱动
function InitializeUninstall(): Boolean;
var
  ErrorCode: Integer;
begin
  Result := true;
end;

// 自定义安装步骤
procedure CurStepChanged(CurStep: TSetupStep);
begin
  case CurStep of
    ssInstall:
      begin
        Log('Starting WVLAN installation...');
      end;
    ssPostInstall:
      begin
        Log('Installation completed successfully.');
        // 可以在此处执行额外任务，如注册服务、配置防火墙等
      end;
  end;
end;
