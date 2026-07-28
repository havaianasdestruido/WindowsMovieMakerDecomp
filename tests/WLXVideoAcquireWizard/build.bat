@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x86
cl.exe /EHsc /MD "%~dp0test.cpp" /Fe:"%~dp0test_wizard.exe"
if %ERRORLEVEL% EQU 0 (
    echo BUILD SUCCESS
    "%~dp0test_wizard.exe"
) else (
    echo BUILD FAILED
    exit /b 1
)
