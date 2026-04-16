# ============================================================================
# WVLAN 安装包构建脚本 (Windows PowerShell)
# 使用方法: .\build-installer.ps1
# ============================================================================

[CmdletBinding()]
param(
    [Parameter(Mandatory=$false)]
    [switch]$UseNSIS,
    
    [Parameter(Mandatory=$false)]
    [switch]$UseInnoSetup,
    
    [Parameter(Mandatory=$false)]
    [string]$Version = "1.0.0",
    
    [Parameter(Mandatory=$false)]
    [switch]$Release
)

$ErrorActionPreference = "Stop"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  WVLAN Installer Build Script" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# 检查运行环境
if ($PSVersionTable.PSVersion.Major -lt 5) {
    Write-Error "PowerShell 5.0+  required!"
    exit 1
}

# 设置工作目录
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = Split-Path -Parent $ScriptDir
$OutputDir = Join-Path $ScriptDir "output"

if (!(Test-Path $OutputDir)) {
    New-Item -ItemType Directory -Path $OutputDir | Out-Null
}

# 默认使用 Inno Setup (推荐)
$BuildInno = ($null -eq $UseNSIS -and $null -eq $UseInnoSetup) -or $UseInnoSetup
$BuildNsis = $UseNSIS

Write-Host "[INFO] Build configuration:" -ForegroundColor Yellow
Write-Host "  - Version:       $Version" -ForegroundColor White
Write-Host "  - Release Mode:  $($Release.IsPresent)" -ForegroundColor White
Write-Host "  - NSIS:          $($BuildNsis)" -ForegroundColor White
Write-Host "  - Inno Setup:    $BuildInno" -ForegroundColor White
Write-Host ""

# ============================================================================
# 步骤 1: 构建 WPF 客户端
# ============================================================================
Write-Host "[Step 1/4] Building WPF Client..." -ForegroundColor Green

$WpfProject = Join-Path $ProjectRoot "wvlan-client\src\WpfClient\WpfClient.csproj"

if (!(Test-Path $WpfProject)) {
    Write-Error "WPF project not found: $WpfProject"
    exit 1
}

$BuildConfig = if ($Release.IsPresent) { "Release" } else { "Debug" }

dotnet build $WpfProject --configuration $BuildConfig --framework net8.0-windows --no-incremental

if ($LASTEXITCODE -ne 0) {
    Write-Error "WPF build failed!"
    exit 1
}

Write-Host "[OK] WPF client built successfully." -ForegroundColor Green
Write-Host ""

# ============================================================================
# 步骤 2: 准备安装文件
# ============================================================================
Write-Host "[Step 2/4] Preparing installation files..." -ForegroundColor Green

$PublishDir = Join-Path $ScriptDir "staging"
if (Test-Path $PublishDir) {
    Remove-Item -Recurse -Force $PublishDir
}
New-Item -ItemType Directory -Path $PublishDir | Out-Null

# 复制发布文件
$SourceDir = Join-Path $ProjectRoot "wvlan-client\src\WpfClient\bin\$BuildConfig\net8.0-windows"

Write-Host "  Copying from: $SourceDir" -ForegroundColor DarkGray
Copy-Item -Path "$SourceDir\*" -Destination $PublishDir -Recurse -Force

# 复制配置文件
$ConfigDir = Join-Path $ProjectRoot "config"
if (Test-Path $ConfigDir) {
    Copy-Item -Path "$ConfigDir\*" -Destination $PublishDir -Force -ErrorAction SilentlyContinue
}

# 复制文档
Copy-Item -Path "$ProjectRoot\README.md" -Destination $PublishDir -ErrorAction SilentlyContinue
Copy-Item -Path "$ProjectRoot\CHANGELOG.md" -Destination $PublishDir -ErrorAction SilentlyContinue

Write-Host "[OK] Files prepared in: $PublishDir" -ForegroundColor Green
Write-Host ""

# ============================================================================
# 步骤 3: 使用 Inno Setup 构建 (推荐)
# ============================================================================
if ($BuildInno) {
    Write-Host "[Step 3/4] Building with Inno Setup..." -ForegroundColor Green
    
    $InnoScript = Join-Path $ScriptDir "innosetup\setup.iss"
    $InnoCompiler = "ISCC.exe"
    
    # 检测 Inno Setup
    $IsccPath = Get-Command $InnoCompiler -ErrorAction SilentlyContinue
    
    if (-not $IsccPath) {
        Write-Warning "Inno Setup compiler (ISCC.exe) not found in PATH!"
        Write-Warning ""
        Write-Warning "Please install Inno Setup from: https://jrsoftware.org/isdl.php"
        Write-Warning "Then add 'C:\Program Files (x86)\Inno Setup 6' to your PATH"
        Write-Warning ""
        Write-Warning "Alternatively, use NSIS: .\build-installer.ps1 -UseNSIS"
        
        if ($PSScriptRoot.Contains("GitHub")) {
            # CI/CD环境下继续执行
            Write-Host "[SKIPPED] Inno Setup compilation (will be done in CI)." -ForegroundColor Yellow
        } else {
            $continue = Read-Host "Press Enter to skip, or type 'Y' to cancel"
            if ($continue -eq "Y" -or $continue -eq "y") { exit 1 }
        }
    } else {
        & $InnoCompiler "/O$OutputDir" "/dMyAppVersion=$Version" $InnoScript
        
        if ($LASTEXITCODE -ne 0) {
            Write-Error "Inno Setup compilation failed!"
            exit 1
        }
        
        Write-Host "[OK] Inno Setup installer built successfully!" -ForegroundColor Green
    }
    Write-Host ""
}

# ============================================================================
# 步骤 4: 使用 NSIS 构建 (可选)
# ============================================================================
if ($BuildNsis) {
    Write-Host "[Step 3/4] Building with NSIS..." -ForegroundColor Green
    
    $NsisScript = Join-Path $ScriptDir "nsis\build.nsi"
    $NsisCompiler = "makensis.exe"
    
    # 检测 NSIS
    $NsisPath = Get-Command $NsisCompiler -ErrorAction SilentlyContinue
    
    if (-not $NsisPath) {
        Write-Warning "NSIS compiler (makensis.exe) not found in PATH!"
        Write-Warning ""
        Write-Warning "Please install NSIS from: https://nsis.sourceforge.io/Download"
        Write-Warning "Then add NSIS installation directory to your PATH"
        Write-Warning ""
        Write-Warning "Alternatively, use Inno Setup: .\build-installer.ps1"
        
        if ($PSScriptRoot.Contains("GitHub")) {
            Write-Host "[SKIPPED] NSIS compilation (will be done in CI)." -ForegroundColor Yellow
        } else {
            $continue = Read-Host "Press Enter to skip, or type 'Y' to cancel"
            if ($continue -eq "Y" -or $continue -eq "y") { exit 1 }
        }
    } else {
        & $NsisCompiler "/V2" "/DVERSION=$Version" $NsisScript
        
        if ($LASTEXITCODE -ne 0) {
            Write-Error "NSIS compilation failed!"
            exit 1
        }
        
        # 移动输出到 output 目录
        $NsisOutput = Join-Path $ScriptDir "WVLAN-Setup.exe"
        if (Test-Path $NsisOutput) {
            Move-Item -Path $NsisOutput -Destination "$OutputDir\WVLAN-Setup-$Version.exe" -Force
        }
        
        Write-Host "[OK] NSIS installer built successfully!" -ForegroundColor Green
    }
    Write-Host ""
}

# ============================================================================
# 构建摘要
# ============================================================================
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  Build Summary" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

$InstallerFiles = Get-ChildItem -Path $OutputDir -Filter "*.exe" -ErrorAction SilentlyContinue

if ($InstallerFiles.Count -gt 0) {
    Write-Host "[SUCCESS] Installers created:" -ForegroundColor Green
    foreach ($file in $InstallerFiles) {
        $size = "{0:N2}" -f ($file.Length / 1MB)
        Write-Host "  ✓ $($file.Name) (${size} MB)" -ForegroundColor White
    }
    Write-Host ""
    Write-Host "Output Directory: $OutputDir" -ForegroundColor Cyan
} else {
    Write-Warning "No installer generated. Check the warnings above."
    Write-Warning ""
    Write-Warning "To build in CI/CD environment, push to GitHub repository."
    Write-Warning "GitHub Actions will automatically build and release."
}

Write-Host ""
Write-Host "Done! Press any key to exit..." -ForegroundColor DarkGray
$null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
