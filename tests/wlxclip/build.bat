@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x86
cl.exe /EHsc /MDd /Zi "%~dp0test.cpp" /Fe:"%~dp0test.exe" ole32.lib
if %ERRORLEVEL% EQU 0 (
    echo BUILD SUCCESS
    cd /d "%~dp0..\.."
    "%~dp0test.exe"
) else (
    echo BUILD FAILED
)
