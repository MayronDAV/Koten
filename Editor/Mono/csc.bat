@echo off
pushd %~dp0
.\bin\mono lib\mono\4.5\csc.exe %*
popd