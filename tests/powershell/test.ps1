# Basic test for MovieMakerCore.dll
$dllPath = Join-Path $PSScriptRoot "..\..\tests\WLXPhotoCinematic\MovieMakerCore.dll"
Add-Type -Path $dllPath
[MovieMakerCore.MovieMakerMain]::Invoke()
