@echo off
echo compile-windows.bat can auto compile all windows solutions.
echo ==========================================================
SETLOCAL

IF "%1"=="32" (set Machine=x86) else (set Machine=x64)
set ProjName=%2
set Config=%3
set platform=%4
echo Machine: %Machine%
echo ProjName: %ProjName%
echo Config: %Config%
echo Platform:%platform%

set Local_Path=%~dp0%
echo LocalPath: %Local_Path%
cd %Local_Path%
set VCINSTALLDIR=C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build
set PATH=%VCINSTALLDIR%\bin;%PATH%
call "%VCINSTALLDIR%\vcvarsall.bat" %Machine%
msbuild %ProjName% /t:Rebuild /p:Configuration=%Config% /p:Platform=%platform% /p:SubsystemVersion=5.1

