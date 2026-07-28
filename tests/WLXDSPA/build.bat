@echo off
setlocal
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x86
if %ERRORLEVEL% neq 0 (
    echo vcvarsall.bat failed!
    exit /b 1
)
echo === Compiling test.cpp ===
cl.exe /nologo /EHsc /MDd /W3 test.cpp /Fe:test.exe /link ole32.lib oleaut32.lib msdmo.lib dmoguids.lib /OUT:test.exe
if %ERRORLEVEL% neq 0 (
    echo Compilation FAILED
    exit /b 1
)
echo === Compilation successful ===
echo === Running test.exe ===
cd /d "%~dp0..\.."
tests\WLXDSPA\test.exe
echo === Done ===
