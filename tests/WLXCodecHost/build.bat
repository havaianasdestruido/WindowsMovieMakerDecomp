@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x86
cl.exe /EHsc /MDd /Zi "%~dp0test_codec_host.cpp" /Fe:"%~dp0test_codec_host.exe" /link user32.lib ole32.lib oleaut32.lib advapi32.lib shlwapi.lib version.lib
if %ERRORLEVEL% EQU 0 (
    echo BUILD SUCCESS
    "%~dp0test_codec_host.exe"
) else (
    echo BUILD FAILED
)
