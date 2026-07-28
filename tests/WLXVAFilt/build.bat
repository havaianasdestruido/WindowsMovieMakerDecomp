@echo off
setlocal
cd /d "%~dp0"
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x86
if %ERRORLEVEL% neq 0 (
    echo vcvarsall.bat failed!
    exit /b 1
)
echo === Compiling test_wlxvafilt.cpp ===
cl.exe /nologo /EHsc /MDd /W3 test_wlxvafilt.cpp /Fe:test_wlxvafilt.exe /link /OUT:test_wlxvafilt.exe version.lib
if %ERRORLEVEL% neq 0 (
    echo Compilation FAILED
    exit /b 1
)
echo === Compilation successful ===
echo === Running test_wlxvafilt.exe ===
test_wlxvafilt.exe
echo === Done ===
