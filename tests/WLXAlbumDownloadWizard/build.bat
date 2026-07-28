@echo off
setlocal enabledelayedexpansion

:: Locate VS 2022
set VSWHERE="%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist %VSWHERE% set VSWHERE="%ProgramFiles%\Microsoft Visual Studio\Installer\vswhere.exe"

if not exist %VSWHERE% (
    echo Error: vswhere.exe not found
    exit /b 1
)

for /f "tokens=*" %%i in ('%VSWHERE% -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath') do set VSINST=%%i

if not defined VSINST (
    echo Error: Visual Studio installation not found
    exit /b 1
)

call "%VSINST%\VC\Auxiliary\Build\vcvars32.bat" >nul 2>&1

set OUTDIR=.
cl.exe /nologo /O2 /GS- /W3 /EHsc test.cpp /Fe%OUTDIR%\test.exe /link /SUBSYSTEM:CONSOLE /MACHINE:X86
