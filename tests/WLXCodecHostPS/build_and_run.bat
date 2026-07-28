@echo off
setlocal
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x86
if %ERRORLEVEL% neq 0 (
    echo vcvarsall.bat failed!
    exit /b 1
)
echo === Compiling test_proxy.cpp ===
cl.exe /nologo /EHsc /MDd /W3 test_proxy.cpp /Fe:test_proxy.exe /link ole32.lib oleaut32.lib /OUT:test_proxy.exe
if %ERRORLEVEL% neq 0 (
    echo Compilation FAILED
    exit /b 1
)
echo === Compilation successful ===
echo === Running test_proxy.exe ===
cd /d "%~dp0..\.."
tests\WLXCodecHostPS\test_proxy.exe
echo === Done ===
