@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x86
cl.exe /EHsc /MD /Zi "%~dp0test_imagetranscode.cpp" /Fe:"%~dp0test_imagetranscode.exe" /link ole32.lib oleaut32.lib
if %ERRORLEVEL% EQU 0 (
    echo BUILD SUCCESS
    "%~dp0test_imagetranscode.exe"
) else (
    echo BUILD FAILED
)
