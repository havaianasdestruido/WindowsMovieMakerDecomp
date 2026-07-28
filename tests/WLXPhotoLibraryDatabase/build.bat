@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x86
if %ERRORLEVEL% neq 0 (
    echo vcvarsall.bat failed!
    exit /b 1
)
echo === Compiling test.cpp ===
cl.exe /nologo /EHsc /MDd /W3 "%~dp0test.cpp" /Fe:"%~dp0test.exe"
if %ERRORLEVEL% neq 0 (
    echo Compilation FAILED
    exit /b 1
)
echo === Compilation successful ===
echo === Running test.exe from project root ===
cd /d "%~dp0..\.."
tests\WLXPhotoLibraryDatabase\test.exe
echo === Done ===
