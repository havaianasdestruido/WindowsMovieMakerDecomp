@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" x86
cl.exe /EHsc /MDd /Zi "%~dp0test_npwlp.cpp" /Fe:"%~dp0test_npwlp.exe"
if %ERRORLEVEL% EQU 0 ("%~dp0test_npwlp.exe")
