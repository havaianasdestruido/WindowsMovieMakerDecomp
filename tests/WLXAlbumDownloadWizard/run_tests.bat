@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars32.bat"
cd /d C:\Users\mcmco\Desktop\WMMR\tests\WLXAlbumDownloadWizard
test.exe
echo Exit code: %ERRORLEVEL%
