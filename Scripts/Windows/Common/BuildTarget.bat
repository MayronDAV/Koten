@echo off
setlocal enabledelayedexpansion

if "%1"=="" (
    echo Please provide a target to build using the /target: switch.
    exit /b 1
)

set "target=%1"
set "configuration=Debug"
set "platform=x64"

if not "%2"=="" set "configuration=%2"
if not "%3"=="" set "platform=%3"

set "script_dir=%~dp0"
set "vswhere_path=%script_dir%..\vswhere.exe"

for /f "tokens=*" %%a in ('%vswhere_path% -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath') do set "VS_PATH=%%a"

if not defined VS_PATH (
    echo Visual Studio not found.
    exit /b 1
)

set "MSBUILD_PATH=%VS_PATH%\MSBuild\Current\Bin\MSBuild.exe"

echo Building target: %target% with configuration: %configuration% and platform: %platform%

"%MSBUILD_PATH%" "Koten.sln" /t:%target% /p:Configuration=%configuration% /p:Platform=%platform%

echo Build complete.
endlocal
