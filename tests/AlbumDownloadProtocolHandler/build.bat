@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x86
cl.exe /EHsc /MDd /Zi "%~dp0test.cpp" /Fe:"%~dp0test.exe"
if %ERRORLEVEL% EQU 0 (
    echo BUILD SUCCESS
    "%~dp0test.exe"
) else (
    echo BUILD FAILED
)
