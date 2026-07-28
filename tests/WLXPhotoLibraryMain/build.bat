@echo off
setlocal
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x86
if %ERRORLEVEL% neq 0 (
    echo vcvarsall.bat failed!
    exit /b 1
)
echo === Compiling test_main.cpp ===
cl.exe /nologo /EHsc /MDd /W3 "%~dp0test_main.cpp" /Fe:"%~dp0test_main.exe" /link /OUT:"%~dp0test_main.exe"
if %ERRORLEVEL% neq 0 (
    echo Compilation FAILED
    exit /b 1
)
echo === Compilation successful ===
echo === Running test_main.exe ===
"%~dp0test_main.exe"
echo === Done ===
