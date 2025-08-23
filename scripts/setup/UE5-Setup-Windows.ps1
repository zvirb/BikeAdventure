# AI Agent Executable Setup Script for Windows
param(
    [string]$ProjectName = "BikeAdventure",
    [string]$InstallPath = "C:\UE5Projects"
)

Write-Host "Setting up BikeAdventure development environment on Windows..." -ForegroundColor Green

# Check if running as administrator
if (-NOT ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole] "Administrator")) {
    Write-Host "This script requires administrator privileges. Please run as administrator." -ForegroundColor Red
    exit 1
}

# Install Chocolatey if not present
if (!(Get-Command choco -ErrorAction SilentlyContinue)) {
    Write-Host "Installing Chocolatey..." -ForegroundColor Yellow
    Set-ExecutionPolicy Bypass -Scope Process -Force
    [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072
    iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))
}

# Install dependencies via Chocolatey
Write-Host "Installing development dependencies..." -ForegroundColor Yellow
choco install -y visualstudio2022community git git-lfs cmake

# Configure Visual Studio for UE5
Write-Host "Configuring Visual Studio workloads..." -ForegroundColor Yellow
$vsPath = "${env:ProgramFiles}\Microsoft Visual Studio\2022\Community"
$workloads = @(
    "Microsoft.VisualStudio.Workload.NativeDesktop",
    "Microsoft.VisualStudio.Workload.NativeGame",
    "Microsoft.VisualStudio.Component.VC.Tools.x86.x64",
    "Microsoft.VisualStudio.Component.Windows11SDK.22000"
)

if (Test-Path $vsPath) {
    foreach ($workload in $workloads) {
        Write-Host "Adding workload: $workload" -ForegroundColor Cyan
        & "$vsPath\Common7\Tools\VsDevCmd.bat" -add $workload
    }
} else {
    Write-Host "Visual Studio 2022 Community not found at expected path" -ForegroundColor Yellow
}

# Create project directory structure
Write-Host "Creating project directory structure..." -ForegroundColor Yellow
$projectPath = Join-Path $InstallPath $ProjectName
New-Item -ItemType Directory -Force -Path $projectPath
New-Item -ItemType Directory -Force -Path "$projectPath\Source"
New-Item -ItemType Directory -Force -Path "$projectPath\Content"
New-Item -ItemType Directory -Force -Path "$projectPath\Config"
New-Item -ItemType Directory -Force -Path "$projectPath\Documentation"
New-Item -ItemType Directory -Force -Path "$projectPath\Tests"

# Set environment variables
Write-Host "Setting environment variables..." -ForegroundColor Yellow
[Environment]::SetEnvironmentVariable("BIKEADVENTURE_PROJECT_PATH", $projectPath, "User")
[Environment]::SetEnvironmentVariable("UE5_PROJECT_NAME", $ProjectName, "User")

Write-Host "Setup complete!" -ForegroundColor Green
Write-Host "Next steps:" -ForegroundColor Cyan
Write-Host "1. Download and install Unreal Engine 5.4+ from Epic Games Launcher"
Write-Host "2. Create new C++ project using Third Person template"
Write-Host "3. Replace generated files with BikeAdventure-specific code"
Write-Host "Project path: $projectPath" -ForegroundColor Yellow