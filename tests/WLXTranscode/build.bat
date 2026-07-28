@echo off
setlocal enabledelayedexpansion

call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x86
if %ERRORLEVEL% NEQ 0 (
    echo Error: vcvarsall.bat failed
    exit /b 1
)

set SRCDIR=%~dp0
set EXE="%SRCDIR%test_transcode.exe"

echo === Building test_transcode.exe ===
cl.exe /nologo /EHsc /W4 /Zi "%SRCDIR%test_transcode.cpp" /Fe%EXE% /link ole32.lib oleaut32.lib advapi32.lib shlwapi.lib version.lib
if %ERRORLEVEL% NEQ 0 (
    echo BUILD FAILED
    exit /b 1
)
echo BUILD SUCCESS
echo.

echo === Running test_transcode.exe ===
%EXE%
set TESTEXIT=%ERRORLEVEL%
echo.
echo Test harness exit code: %TESTEXIT%
exit /b %TESTEXIT%
