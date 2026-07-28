@echo off
setlocal
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x86
if %ERRORLEVEL% neq 0 (
    echo vcvarsall.bat failed!
    exit /b 1
)
echo === Compiling test_base.cpp ===
cl.exe /nologo /EHsc /MDd /W3 test_base.cpp /Fe:test_base.exe /link /OUT:test_base.exe
if %ERRORLEVEL% neq 0 (
    echo Compilation FAILED
    exit /b 1
)
echo === Compilation successful ===
echo === Running test_base.exe ===
test_base.exe
echo === Done ===
