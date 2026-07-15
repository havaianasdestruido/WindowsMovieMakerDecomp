@echo off
timeout /t 2 /nobreak >nul
copy /Y "C:\Users\mcmco\Desktop\WMMR\src\MovieMakerCore\HMREngine\DXResources\MeshResourceDX.h.new" "C:\Users\mcmco\Desktop\WMMR\src\MovieMakerCore\HMREngine\DXResources\MeshResourceDX.h"
copy /Y "C:\Users\mcmco\Desktop\WMMR\src\MovieMakerCore\HMREngine\DXResources\RenderLoopDX.cpp.fixed" "C:\Users\mcmco\Desktop\WMMR\src\MovieMakerCore\HMREngine\DXResources\RenderLoopDX.cpp"
copy /Y "C:\Users\mcmco\Desktop\WMMR\src\MovieMakerCore\HMREngine\DXResources\TextResourceDX.cpp.fixed" "C:\Users\mcmco\Desktop\WMMR\src\MovieMakerCore\HMREngine\DXResources\TextResourceDX.cpp"
echo DONE
