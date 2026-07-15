Set fso = CreateObject("Scripting.FileSystemObject")
WScript.Sleep 3000
fso.CopyFile "C:\Users\mcmco\Desktop\WMMR\src\MovieMakerCore\HMREngine\DXResources\MeshResourceDX.h.new", "C:\Users\mcmco\Desktop\WMMR\src\MovieMakerCore\HMREngine\DXResources\MeshResourceDX.h", True
fso.CopyFile "C:\Users\mcmco\Desktop\WMMR\src\MovieMakerCore\HMREngine\DXResources\RenderLoopDX.cpp.fixed", "C:\Users\mcmco\Desktop\WMMR\src\MovieMakerCore\HMREngine\DXResources\RenderLoopDX.cpp", True
fso.CopyFile "C:\Users\mcmco\Desktop\WMMR\src\MovieMakerCore\HMREngine\DXResources\TextResourceDX.cpp.fixed", "C:\Users\mcmco\Desktop\WMMR\src\MovieMakerCore\HMREngine\DXResources\TextResourceDX.cpp", True
