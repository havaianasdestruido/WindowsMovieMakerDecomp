@echo off
setlocal
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x86
if %ERRORLEVEL% neq 0 (
    echo vcvarsall.bat failed!
    exit /b 1
)
echo === Compiling test_gallery.cpp ===
cl.exe /nologo /EHsc /MDd /W3 "%~dp0test_gallery.cpp" /Fe:"%~dp0test_gallery.exe" /link /OUT:"%~dp0test_gallery.exe"
if %ERRORLEVEL% neq 0 (
    echo Compilation FAILED
    exit /b 1
)
echo === Compilation successful ===
echo === Running test_gallery.exe ===
"%~dp0test_gallery.exe"
echo === Done ===
