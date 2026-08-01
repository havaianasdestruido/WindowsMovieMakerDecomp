@echo off
setlocal
rem Invoke MovieMakerCore.dll MovieMakerMain
rundll32.exe "${PWD}\MovieMakerCore.dll",MovieMakerMain
endlocal