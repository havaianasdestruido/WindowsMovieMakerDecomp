@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x86
cl.exe /nologo /EHsc /MDd /W3 "%~dp0test_sqm.cpp" /Fe:"%~dp0test_sqm.exe" /link /OUT:"%~dp0test_sqm.exe"
if %ERRORLEVEL% EQU 0 (
    echo BUILD SUCCESS
    "%~dp0test_sqm.exe"
) else (
    echo BUILD FAILED
    exit /b 1
)
