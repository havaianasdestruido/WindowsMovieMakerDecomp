@echo off
setlocal
cd /d "%~dp0"
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x86
if %ERRORLEVEL% neq 0 (
    echo vcvarsall.bat failed!
    exit /b 1
)
echo === Compiling test_wlxsendmail.cpp ===
cl.exe /nologo /EHsc /MDd /W3 test_wlxsendmail.cpp /Fe:test_wlxsendmail.exe /link /OUT:test_wlxsendmail.exe user32.lib advapi32.lib ole32.lib
if %ERRORLEVEL% neq 0 (
    echo Compilation FAILED
    exit /b 1
)
echo === Compilation successful ===
echo === Running test_wlxsendmail.exe ===
test_wlxsendmail.exe
set EXITCODE=%ERRORLEVEL%
echo === Exit code: %EXITCODE% ===
exit /b %EXITCODE%
