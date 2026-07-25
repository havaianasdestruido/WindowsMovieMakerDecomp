# Cross-DLL Import Analysis: Imports Table

## Legend: System DLL Grid

| System DLL Imported          | MME | MMC | PB  | SSD | VTD | FRD | PCD | PLD | PTD | MPS | CCH | TRC | DSD | FRW | MLD | MPD |
|------------------------------|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|
| **KERNEL32.dll**             |  X  |  X  |  X  |  X  |  X  |  X  |  X  |  X  |  X  |  X  |  X  |  X  |  X  |  X  |  X  |  X  |
| **MSVCR110.dll**             |  X  |  X  |  X  |  X  |  X  |  X  |  X  |  X  |  X  |  X  |  X  |  X  |  X  |  X  |  X  |  X  |
| **USER32.dll**               |     |  X  |     |  X  |  X  |  X  |  X  |  X  |  X  |  X  |  X  |  X  |  X  |  X  |  X  |  X  |
| **ADVAPI32.dll**             |     |  X  |     |  X  |  X  |  X  |  X  |  X  |  X  |  X  |  X  |  X  |  X  |  X  |  X  |  X  |
| **ole32.dll**                |     |  X  |  X  |  X  |  X  |  X  |  X  |  X  |  X  |  X  |  X  |  X  |  X  |  X  |  X  |  X  |
| **OLEAUT32.dll**             |     |  X  |     |  X  |  X  |  X  |  X  |  X  |  X  |  X  |  X  |  X  |  X  |  X  |  X  |  X  |
| **GDI32.dll**                |     |  X  |     |  X  |     |     |     |  X  |  X  |  X  |  X  |     |     |     |     |     |
| **SHELL32.dll**              |     |  X  |     |  X  |  X  |     |     |     |     |  X  |  X  |  X  |  X  |     |     |     |
| **SHLWAPI.dll**              |     |  X  |  X  |  X  |  X  |     |  X  |  X  |  X  |  X  |     |  X  |  X  |     |  X  |     |
| **gdiplus.dll**              |     |  X  |     |  X  |     |     |     |  X  |  X  |  X  |  X  |     |     |     |     |  X  |
| **VERSION.dll**              |     |  X  |  X  |  X  |     |     |     |     |     |  X  |     |     |  X  |     |     |     |
| **UXCore.dll**               |     |  X  |     |  X  |     |     |     |     |     |  X  |     |     |     |     |     |     |
| **MFPlat.DLL**               |     |  X  |     |     |     |     |     |     |     |     |     |  X  |  X  |  X  |  X  |     |
| **MF.dll**                   |     |  X  |     |     |     |     |     |     |     |     |     |  X  |  X  |  X  |  X  |     |
| **d3d9.dll**                 |     |  X  |     |  X  |     |     |     |  X  |     |     |     |  X  |     |     |  X  |     |
| **d3d11.dll**                |     |  X  |     |     |     |     |     |     |     |     |     |  X  |     |     |  X  |     |
| **dxva2.dll**                |     |  X  |     |     |     |     |     |     |     |     |     |  X  |     |     |  X  |     |
| **d2d1.dll**                 |     |  X  |     |     |     |     |     |     |     |     |     |     |     |     |     |     |
| **DWrite.dll**               |     |  X  |     |     |     |     |     |     |     |     |     |     |     |     |     |     |
| **d3dx9_32.dll**             |     |     |     |  X  |     |     |  X  |  X  |  X  |     |     |     |     |     |     |     |
| **PROPSYS.dll**              |     |  X  |     |     |     |     |     |     |     |  X  |  X  |  X  |     |  X  |  X  |     |
| **WINMM.dll**                |     |  X  |     |     |     |     |     |  X  |     |     |     |     |  X  |     |  X  |  X  |
| **OLEACC.dll**               |     |  X  |     |  X  |     |     |     |     |     |  X  |     |     |     |     |     |     |
| **UxTheme.dll**              |     |  X  |     |     |     |     |     |     |     |  X  |     |     |     |     |     |     |
| **dwmapi.dll**               |     |  X  |  X  |  X  |     |     |     |     |     |     |     |     |     |     |     |     |
| **XmlLite.dll**              |     |  X  |     |     |     |     |     |     |     |     |     |     |     |     |     |     |
| **WindowsCodecs.dll**        |     |  X  |     |     |     |  X  |     |     |     |     |     |     |     |     |     |     |
| **D3DCOMPILER_46.dll**       |     |  X  |     |     |     |     |     |     |     |     |     |     |     |     |     |     |
| **WLXPhotoBase.dll**         |  X  |  X  |  -  |  X  |  X  |  X  |  X  |     |     |  X  |  X  |  X  |     |     |  X  |     |
| **WLXPhotoSqm.dll**          |     |  X  |     |  X  |     |  X  |     |     |     |  X  |     |     |     |     |     |     |
| **DmxBici.dll**              |     |  X  |     |  X  |     |     |     |     |     |  X  |     |     |     |     |     |     |
| **MSVCP110.dll**             |     |     |     |     |     |  X  |     |     |     |     |     |     |     |     |     |     |
| **PSAPI.DLL**                |     |     |  X  |  X  |     |     |     |     |  X  |     |     |     |     |     |     |     |
| **COMCTL32.dll**             |     |     |     |     |     |     |     |     |  X  |     |     |     |     |     |     |     |
| **ESENT.dll**                |     |  X  |     |     |     |     |     |     |     |     |     |     |     |     |     |     |
| **wlidcli.dll**              |     |  X  |     |     |     |     |     |     |     |  X  |     |     |     |     |     |     |
| **WINHTTP.dll**              |     |     |     |     |     |     |     |     |     |  X  |     |     |     |     |     |     |
| **WININET.dll**              |     |     |     |     |     |     |     |     |     |  X  |     |     |     |     |     |     |
| **CRYPT32.dll**              |     |     |     |     |     |     |     |     |     |  X  |     |     |     |     |     |     |
| **MetadataSys.dll**          |     |  X  |     |     |     |     |     |     |     |  X  |  X  |     |     |     |     |     |
| **DDRAW.dll**                |     |     |     |     |     |     |     |  X  |     |     |     |     |     |     |     |     |
| **msdmo.dll**                |     |     |     |     |     |     |     |  X  |     |     |     |     |     |     |     |     |
| **EVR.dll**                  |     |     |     |     |     |     |     |     |     |     |     |     |  X  |  X  |     |     |
| **AVRT.dll**                 |     |     |     |     |     |     |     |     |     |     |     |     |  X  |     |     |     |
| **WMVCore.DLL**              |     |     |     |     |  X  |     |     |     |     |     |     |     |     |     |     |     |
| **WLXMP4Parser.dll**         |     |     |     |     |  X  |     |     |     |     |     |     |     |     |     |     |     |
| **wer.dll**                  |     |     |  X  |     |     |     |     |     |     |     |     |     |     |     |     |     |
| **WTSAPI32.dll**             |     |     |     |  X  |     |     |     |     |     |     |     |     |     |     |     |     |
| **msi.dll**                  |     |     |     |  X  |     |     |     |     |     |  X  |     |     |     |     |     |     |
| **uxctl.dll**                |     |  X  |     |     |     |     |     |     |     |     |     |     |     |     |     |     |

## Legend: Binary Abbreviations

| Abbr | Binary Name |
|------|-------------|
| MME  | MovieMaker.exe |
| MMC  | MovieMakerCore.dll |
| PB   | WLXPhotoBase.dll |
| SSD  | WLXSlideshow.dll |
| VTD  | WLXVideoTrim.dll |
| FRD  | WLXFaceRecognition.dll |
| PCD  | WLXPhotoCinematic.dll |
| PLD  | WLXPipeline.dll |
| PTD  | WLXPipetran.dll |
| MPS  | WLXMediaPublishSubscribe.dll |
| CCH  | WLXCodecHost.exe |
| TRC  | WLXTranscode.exe |
| DSD  | WLMFDS.dll |
| FRW  | WLMFReadWrite.dll |
| MLD  | WLXMovieLibrary.dll |
| MPD  | WLXMP4Parser.dll |

## WLXPhotoBase.dll Exported API Usage (cross-DLL)

| Exported Function                    | MME | MMC | SSD | VTD | FRD | PCD | MPS | CCH | TRC | MLD |
|--------------------------------------|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|
| `?Delete@BasePrivate@@YAXPAX@Z`      |  X  |  X  |  X  |  X  |  X  |  X  |  X  |  X  |  X  |  X  |
| `?New@BasePrivate@@YAPAXI_N@Z`       |     |  X  |  X  |  X  |  X  |  X  |  X  |  X  |  X  |  X  |
| `??1Exception@Base@@UAE@XZ`          |     |  X  |  X  |  X  |  X  |  X  |  X  |  X  |  X  |  X  |
| `?Throw@Base@@YGXJ@Z`                |     |  X  |  X  |  X  |  X  |  X  |  X  |  X  |  X  |  X  |
| `?BaseAtlThrow@ATL@@YGXJ@Z`          |     |  X  |  X  |  X  |  X  |  X  |  X  |  X  |  X  |  X  |
| `?ThrowLastError@Base@@YGXXZ`        |     |  X  |  X  |  X  |  X  |     |  X  |     |  X  |  X  |
| `?GetBaseStringManager@String@...`   |     |  X  |  X  |  X  |     |  X  |  X  |     |  X  |  X  |
| `?IsWin7OrGreater@OS@Base@@YG_NXZ`   |     |  X  |  X  |  X  |     |     |     |  X  |  X  |  X  |
| `?IsWin8OrGreater@OS@Base@@YG_NXZ`   |     |  X  |  X  |  X  |     |     |     |     |  X  |  X  |
| `??BException@Base@@QBEJXZ`          |     |  X  |     |  X  |     |     |  X  |     |  X  |  X  |
| `?GdiplusStatusToHresult@Base@@...`  |     |  X  |  X  |     |     |     |  X  |     |     |     |
| `??0Exception@Base@@IAE@J@Z`         |     |  X  |  X  |     |     |     |  X  |     |     |     |
| `??0Exception@Base@@QAE@ABV01@@Z`    |     |  X  |  X  |     |     |     |  X  |     |     |     |
| `?GetProcessorCount@CPU@Base@@...`   |     |  X  |     |     |     |     |     |     |     |     |
| `?IsOutOfMemoryError@Base@@YG_NJ@Z`  |     |     |  X  |     |     |     |     |     |     |     |
