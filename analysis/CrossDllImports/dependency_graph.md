# Dependency Graph

## ASCII Art Dependency Graph

```
                                                         +------------------+
                                                         |  MovieMaker.exe  |
                                                         +--------+---------+
                                                                  |
                                                                  | (delay-load)
                                                                  v
                                            +---------------------+----------------------+
                                            |        MovieMakerCore.dll                 |
                                            +--+------------+---------+--------+--------+
                                               |            |         |        |
                            +------------------+  +---------+  +------+  +-----+--------+
                            |                     |            |         |              |
                            v                     v            v         v              v
                    +---------------+   +-------------+  +--------+ +--------+   +--------------+
                    | WLXPhotoBase  |   | WLXPhotoSqm |  | DmxBici| |ESENT.dll|   | MetadataSys  |
                    | .dll (base    |   | .dll (SQM   |  | .dll   | (database) |   | .dll         |
                    |  library)     |   | telemetry)  |  |(BICI   |            |   | (property    |
                    +-------+------+   +------+------+  |exper-  |            |   |  handler)    |
                            |                 |         |iments) |            |   +--------------+
                            |                 |         +--------+            |
                            |                 |                               |
                            v                 v                               v
              +-------------+-------+   +-----+----------+          +----------+---------+
              | UXCore.dll (DirectUI)|   | MFPlat.DLL     |          | MF.dll             |
              +----------------------+   | (Media Found.) |          | (Media Foundation) |
                                         +-------+--------+          +----+------+--------+
                                                 |                       |      |
                                                 v                       v      v
                                     +-----------+-----------+   +-------+------+-------+
                                     | d3d11.dll | d2d1.dll  |   | dxva2.dll    | DWrite|
                                     | (Direct3D)| (Direct2D)|   | (DXVA)       | .dll |
                                     +-----------+-----------+   +--------------+------+

                                      WLXSlideshow.dll
                                     +--+---+---+---+----+
                                        |   |   |   |
                              +---------+   |   |   +--------+
                              v             v   v            v
                     +-----------+   +-----------+   +------------------+
                     | d3dx9_32  |   | gdiplus   |   | d3d9.dll         |
                     | .dll      |   | .dll      |   | (Direct3D9)      |
                     +-----------+   +-----------+   +------------------+

                                      WLXPipeline.dll
                                     +--+---+---+---+----+------+
                                        |   |   |   |    |      |
                              +---------+   |   |   +----+---+  +--------+
                              v             v   v        v      v        v
                     +-----------+   +-----------+  +--------+ +------+ +-------+
                     | msdmo.dll |   | DDRAW.dll |  | WINMM  | |d3d9  | |d3dx9_ |
                     | (DMO)     |   | (DirectDraw)| | .dll  | |.dll  | |32.dll |
                     +-----------+   +-----------+  +--------+ +------+ +-------+

                                      WLXPipetran.dll
                                     +--+---+---+---+---+----+
                                        |   |   |   |   |    |
                              +---------+   |   |   |   |    +------------+
                              v             v   v   v   v                 v
                     +-----------+   +-----------+ +------+   +-----------------------+
                     | d3dx9_32  |   | gdiplus   | |GDI32 |   | COMCTL32.dll          |
                     | .dll (3D  |   | .dll      | |.dll  |   | (ImageList_Create...) |
                     |  meshes)  |   +-----------+ +------+   +-----------------------+
                     +-----------+

                                      WLXVideoTrim.dll
                                     +--+---+---+----+
                                        |   |   |    |
                              +---------+   |   |    +-------------------+
                              v             v   v                        v
                     +-----------+   +-----------+   +----------------------------+
                     | WMVCore   |   | WLXMP4    |   | WLXPhotoBase.dll           |
                     | .DLL      |   | Parser.dll|   | (error handling, alloc)    |
                     | (WMV r/w) |   | (delay)   |   +----------------------------+
                     +-----------+   +-----------+

                                WLXMediaPublishSubscribe.dll
                               +--+---+---+---+---+---+---+----+
                                  |   |   |   |   |   |   |    |
                       +----------+   |   |   |   |   |   |    +---------------+
                       v              v   v   v   v   v   v                    v
              +-----------+   +-----------+ +------+ +------+   +--------------------------+
              | WINHTTP   |   | WININET   | |CRYPT | |wlidcli|   | UXCore.dll               |
              | .dll      |   | .dll      | |32.dll| |.dll   |   | (CRMResource, RMLoad*)   |
              +-----------+   +-----------+ +------+ +-------+   +--------------------------+

                WLMFDS.dll                    WLMFReadWrite.dll
               +--+---+---+---+              +--+---+---+---+---+
                  |   |   |   |                 |   |   |   |   |
        +---------+   |   |   +----+   +--------+   |   |   |   +---------+
        v             v   v        v   v            v   v   v             v
  +-----------+ +--------+ +-------+ +-------+ +--------+ +----+   +------------+
  | MFPlat.DLL| | EVR.dll| |AVRT   | | VERSION| |EVR.dll | |MF  |   | PROPSYS    |
  | (MFTEnum, | |        | |.dll   | |.dll    | |        | |.dll|   | .dll       |
  | async)    | +--------+ +-------+ +--------+ +--------+ +----+   +------------+
  +-----------+

             WLXMovieLibrary.dll                    WLXMP4Parser.dll
            +--+---+---+---+----+                +--+---+---+---+
               |   |   |   |    |                   |   |   |   |
     +---------+   |   |   |    +----+              |   |   |   +------+
     v             v   v   v         v              v   v   v          v
+-----------+ +--------+ +-----+ +--------+   +--------+ +------+ +--------+
| MFPlat.DLL| | MF.dll | |d3d9 | |dxva2   |   | WINMM  | |gdiplus| |ole32  |
| (MFStartup,| |        | |.dll | |.dll    |   | .dll   | |.dll   | |.dll   |
|MFShutdown) | +--------+ +-----+ +--------+   |(timers)| |       | +--------+
+-----------+            |d3d11 |               +--------+ +-------+
                         |.dll  |
                         +------+

          WLXCodecHost.exe                    WLXTranscode.exe
         +--+---+---+---+----+               +--+---+---+---+---+----+
            |   |   |   |    |                  |   |   |   |   |    |
  +---------+   |   |   |    +----+   +---------+   |   |   |   |    +--------+
  v             v   v   v         v   v             v   v   v   v             v
+--------+ +--------+ +----+ +--------+ +-------+ +--------+ +----+   +------------+
|WLXPhoto | | PROPSYS| |GDI | |SHCreate | |MF.dll | |MFPlat  | |d3d9 |   | dxva2     |
|Base.dll | | .dll   | |32  | |ItemFrom | |(MFCreate| |.DLL    | |.dll |   | .dll      |
+---------+ |        | |.dll| |Parsing  | |Transcode| +--------+ +-----+   +-----------+
            +--------+ +----+ |Name)    | |Topology)| |d3d11   |
                              +---------+ +---------+ |.dll    |
                                                       +--------+
```

## Dependency Hierarchy

```
Layer 0 (Entry Points):
  MovieMaker.exe ──────────────────────────────► WLXPhotoBase.dll (direct)
    └──► MovieMakerCore.dll (delay-load)
         └──► MovieMakerMain()

Layer 1 (Core Framework):
  WLXPhotoBase.dll ───► KERNEL32, MSVCR110, VERSION, SHLWAPI, PSAPI, wer
  WLXPhotoSqm.dll ───► (telemetry/SQM service)

Layer 2 (Application Modules):
  MovieMakerCore.dll ───► UXCore.dll (DirectUI) ───► Direct2D/DirectWrite/D3D11
                       ───► ESENT.dll (database)
                       ───► MFPlat.DLL / MF.dll (Media Foundation)
                       ───► DmxBici.dll (experimentation)
                       ───► XmlLite.dll (XML parsing)
                       ───► d3d9/d3d11/dxva2/d2d1/DWrite (rendering stack)
                       ───► WLXPhotoBase/WLXPhotoSqm/DmxBici

  WLXSlideshow.dll ───► WLXPhotoBase.dll (delay)
                     ───► WLXPhotoSqm.dll (delay)
                     ───► d3d9.dll + d3dx9_32.dll (sprite rendering)
                     ───► gdiplus.dll

  WLXPipeline.dll ───► DDRAW.dll + d3d9.dll + d3dx9_32.dll
                   ───► msdmo.dll (DirectX Media Objects)
                   ───► WINMM.dll (multimedia timers)
                   ───► gdiplus.dll

  WLXPipetran.dll ───► d3dx9_32.dll (heavy: mesh creation, shader compilation)
                   ───► gdiplus.dll (text rendering: GdipDrawString, fonts)
                   ───► COMCTL32.dll (ImageList)

  WLXVideoTrim.dll ───► WMVCore.DLL (WMCreateReader/WMCreateWriter)
                    ───► WLXMP4Parser.dll (delay: AddMP4SourceFilter)
                    ───► WLXPhotoBase.dll

  WLXFaceRecognition.dll ───► WindowsCodecs.dll (WIC)
                         ───► MSVCP110.dll (C++ std library)
                         ───► WLXPhotoSqm.dll (timed events)
                         ───► WLXPhotoBase.dll

  WLXPhotoCinematic.dll ───► d3dx9_32.dll (matrix transforms)

Layer 3 (Media/Transcoding):
  WLXTranscode.exe ───► MF.dll (MFCreateTranscodeProfile/Topology)
                   ───► MFPlat.DLL
                   ───► d3d9/d3d11/dxva2 (hardware acceleration)
                   ───► WLXPhotoBase.dll

  WLXCodecHost.exe ───► MetadataSys.dll
                   ───► WLXPhotoBase.dll

Layer 3 (Shared Libraries):
  WLMFDS.dll ───► MFPlat.DLL (MFTEnum, MFCreateSample)
             ───► EVR.dll (Enhanced Video Renderer)
             ───► AVRT.dll (multimedia scheduling)
             ───► WINMM.dll

  WLMFReadWrite.dll ───► MFPlat.DLL (MFAllocateWorkQueue, MFTEnum)
                    ───► EVR.dll (MFCreateVideoSampleAllocator)
                    ───► MF.dll (MFCreateASFMediaSink)

  WLXMovieLibrary.dll ───► MFPlat.DLL (MFStartup/MFShutdown)
                      ───► d3d9/d3d11/dxva2
                      ───► WINMM.dll (waveOut* audio playback)
                      ───► WLXPhotoBase.dll (delay)

  WLXMP4Parser.dll ───► gdiplus.dll
                   ───► WINMM.dll (timers)
                   ───► No Media Foundation imports (standalone MP4 parser)

Layer 4 (Service/Telemetry):
  WLXMediaPublishSubscribe.dll ───► WINHTTP.dll (HTTP requests)
                              ───► WININET.dll (URL cache)
                              ───► CRYPT32.dll (CryptProtectData)
                              ───► wlidcli.dll (Live ID/auth)
                              ───► WLXPhotoBase.dll (delay)
                              ───► MetadataSys.dll (delay)
                              ───► UXCore.dll (CRMResource)
                              ───► DmxBici.dll
                              ───► PROPSYS.dll
```
