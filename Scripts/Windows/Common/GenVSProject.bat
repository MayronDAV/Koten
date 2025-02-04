@echo off
setlocal enabledelayedexpansion

if "%1"=="" (
	echo Please provide a target to build.
	exit /b 1
)

set "target=%1"

pushd ..\..

call Tools\premake5.exe %target%
popd