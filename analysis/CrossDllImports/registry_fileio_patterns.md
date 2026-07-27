# Cross-DLL Registry Operations & File I/O Patterns

**Scope**: All DLLs/EXEs in `undecomp/Photo Gallery/` and `undecomp/Shared/`  
**Method**: dumpbin /IMPORTS for API imports, binary string extraction for registry paths & file extensions  
**Date**: 2026-07-27  

---

## 1. Registry API Import Summary

### DLLs with Full Registry CRUD (Open, Create, SetValue, QueryValue, Delete, EnumKey)

| Binary | RegOpenKeyExW | RegCreateKeyExW | RegSetValueExW | RegQueryValueExW | RegDeleteKeyW | RegEnumKeyExW | RegEnumValueW | RegGetValueW | SHRegGetValueW |
|--------|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|
| **MovieMakerCore.dll** | Y | Y | Y | Y | Y | Y | - | Y | - |
| **WLXPhotoLibraryMain.dll** | Y | Y | Y | Y | Y | Y | Y | Y | Y |
| **WLXPhotoLibraryDatabase.dll** | Y | Y | Y | Y | Y | Y | Y | - | Y |
| **WLXPhotoViewer.dll** | Y | Y | Y | Y | Y | Y | Y | Y | - |
| **WLXPhotoAcq.dll** | Y | Y | Y | Y | Y | Y | Y | - | Y |
| **WLXMediaPublishSubscribe.dll** | Y | Y | Y | Y | Y | Y | Y | - | - |
| **WLXPhotoSqm.dll** | Y | Y | Y | Y | Y | Y | - | - | - |
| **WLXPhotoVoyager.dll** | Y | Y | Y | Y | Y | Y | - | - | - |
| **WLXPhotoCinematic.dll** | Y | Y | Y | Y | Y | Y | - | - | - |
| **WLXPhotoClassic.dll** | Y | Y | Y | Y | Y | Y | - | - | - |
| **WLXFaceRecognition.dll** | Y | Y | Y | Y | Y | Y | - | - | - |
| **WLXGrinderScheduler.dll** | Y | Y | Y | Y | Y | Y | - | - | - |
| **WLXImageTranscode.dll** | Y | Y | Y | - | Y | Y | - | - | - |
| **WLXSlideshow.dll** | Y | Y | Y | Y | Y | Y | Y | - | - |
| **WLXSendMail.dll** | Y | Y | Y | Y | Y | Y | Y | - | Y |
| **WLXVideoTrim.dll** | Y | Y | Y | Y | Y | Y | - | - | - |
| **WLXDSPA.dll** | Y | Y | Y | - | Y | Y | - | - | - |
| **Imaging.dll** | Y | Y | Y | Y | Y | Y | - | - | - |
| **MetadataSys.dll** | Y | Y | Y | Y | Y | Y | - | - | - |
| **PhotoViewerShim.dll** | Y | Y | Y | - | Y | Y | - | - | - |
| **AlbumDownloadProtocolHandler.dll** | Y | Y | Y | - | Y | Y | - | - | - |
| **PublishPluginsInterop.dll** | Y | Y | Y | - | Y | Y | - | - | - |
| **SubscribePluginsInterop.dll** | Y | Y | Y | - | Y | Y | - | - | - |
| **wlxclip.dll** | Y | Y | Y | Y | - | Y | - | - | - |
| **WLXMP4Parser.dll** | Y | Y | Y | - | Y | Y | - | - | - |
| **WLXQuickTimeShellExt.dll** | Y | Y | Y | - | Y | Y | - | - | - |
| **WLXVAFilt.dll** | Y | Y | Y | - | Y | Y | - | - | - |

### DLLs with Limited Registry Access (Read-Only / Write-Only)

| Binary | Operations |
|--------|-----------|
| **WLXPipeline.dll** | Open, Create, SetValue, Query, Close |
| **WLXPipetran.dll** | Open, Query, Close (read-only) |
| **WLXCodecHost.exe** | Open, Create, SetValue, Delete, Enum, Close |
| **WLXTranscode.exe** | Open, Query, Close (read-only) |
| **WLXPhotoGallery.exe** | Open, SetValue, Query, Close |
| **WLXVideoCameraAutoPlayManager.exe** | Open, Create, SetValue, Query, Delete, Enum, Close |
| **WLXAlbumDownloadWizard.exe** | Open, Create, SetValue, Query, Delete, Enum, GetValue, EnumValue, Close |
| **WLXPhotoAcquireWizard.exe** | Open, Create, SetValue, Query, Delete, Enum, GetValue, Close |
| **WLXVideoAcquireWizard.exe** | Open, Create, SetValue, Query, Delete, Enum, GetValue, EnumValue, Close |
| **WLXQuickTimeControlHost.exe** | OpenA/W, Create, SetValue, Delete, Enum, QueryA, Close |
| **WLXPhotoGalleryRepair.exe** | (no registry imports detected) |

### Shared DLLs

| Binary | Operations |
|--------|-----------|
| **wldcore.dll** | Full CRUD + RegOpenKeyExA, RegCreateKeyExA (dual ANSI/Unicode) |
| **wlbici.dll** | Open, Query, SetValue, GetValue, Delete, EnumValue, Create, Close |
| **sqmapi.dll** | OpenA/W, Create, SetValue, QueryA/W, EnumValue, Close |
| **WLMFDS.dll** | Open, Create, SetValue, Query, Delete, Enum, EnumValue, GetValue, Close |
| **WLMFReadWrite.dll** | Open, Create, SetValue, Query, Delete, Enum, Close |
| **WLXMovieLibrary.dll** | Open, Query, Close (read-only) |
| **uxcore.dll** | OpenA/W, Create, QueryA/W, SetValue, Delete, EnumA/W, GetValue, Close |
| **uxctl.dll** | Open, Create, SetValue, Query, Delete, Close |
| **wlidux.dll** | Open, Query, SetValue, GetValue, Delete, Close |
| **wldlog.dll** | OpenA, CreateA, QueryA/W, SetValueExA, Close |
| **RSCMFT.dll** | Create, SetValue, Close |
| **uxcalendar.dll** | Open, Delete, Close |
| **uxcontacts.dll** | Open, Delete, Close |

---

## 2. Registry Key Map (by DLL Ownership)

### MovieMakerCore.dll — **Core Movie Maker Settings**
```
Software\Microsoft\Windows Live\Movie Maker
Software\Microsoft\Windows Live\Movie Maker\Post
Software\Microsoft\Windows Live\Movie Maker\Recent
Software\Microsoft\Windows Live\Movie Maker\RecentWLVS
Software\Microsoft\Windows Live\Movie Maker\Suppressed
Software\Microsoft\Windows Live\Photo Gallery
Software\Microsoft\Windows Live\Photo Gallery\FileSupport\DownloadableCodecs
Software\Microsoft\Windows Live\Photo Gallery\FileSupport\DownloadableWLRawCodecs
Software\Microsoft\Windows Live\Photo Gallery\FileSupport\QuickTimeMovieExtensions
Software\Microsoft\Windows Live\Photo Gallery\FileSupport\RawPhotoExtensions
Software\Microsoft\Windows Live\Photo Gallery\SuppressFileTypes
Software\Microsoft\Windows Live\Common
SOFTWARE\Microsoft\Windows Live\Common\Movie Library
Software\Microsoft\Windows Live\Environment\PhotoGallery
Software\Microsoft\Windows Live\Installer
Software\Microsoft\Windows Live\Installer\ProductStatus
Software\Microsoft\Windows Media Foundation\ByteStreamHandlers
Software\Microsoft\Homer
Microsoft\Live\MovieMaker
Microsoft\Windows Live\Video Profiles
```

### WLXPhotoLibraryMain.dll — **Gallery Library Hub**
```
Software\Microsoft\Windows Live\Photo Gallery
Software\Microsoft\Windows Live\Photo Gallery\Library
Software\Microsoft\Windows Live\Photo Gallery\Library\AddFolders
Software\Microsoft\Windows Live\Photo Gallery\Library\MDSyncInFlight\Active
Software\Microsoft\Windows Live\Photo Gallery\Library\MDSyncInFlight\Prompted
Software\Microsoft\Windows Live\Photo Gallery\Library\Suppressed
Software\Microsoft\Windows Live\Photo Gallery\PublishPluginsMru
Software\Microsoft\Windows Live\Photo Gallery\QuickPublishMru
Software\Microsoft\Windows Live\Photo Gallery\Slideshow
Software\Microsoft\Windows Live\Photo Gallery\SuppressedDialogs
Software\Microsoft\Windows Live\Photo Gallery\SuppressFileTypes
Software\Microsoft\Windows Live\Photo Gallery\Viewer
Software\Microsoft\Windows Live\Photo Gallery\WLRaw
Software\Microsoft\Windows Live\Photo Gallery\WLRaw\CurrentWLRawVersionOnDLC
Software\Microsoft\Windows Live\Photo Gallery\Extensibility
Software\Microsoft\Windows Live\Common
SOFTWARE\Microsoft\Windows Live\Common\Movie Library
Software\Microsoft\Windows Live\Environment\PhotoGallery
Software\Microsoft\Windows Live\Installer
Software\Microsoft\Windows Live\Installer\ProductStatus
SOFTWARE\Microsoft\Windows Live\Movie Maker
Software\Microsoft\Windows Live\Communications Clients\Shared
Software\Microsoft\Windows Photo Gallery
Software\Microsoft\VisionTools\Debug
Software\Microsoft\Windows Live Mail
SOFTWARE\Classes\Applications\WLXPhotoViewer.dll\shell\open\command
Software\Microsoft\Windows Media Foundation\ByteStreamHandlers
SOFTWARE\Microsoft\Windows\CurrentVersion\Media Center\Decoder
Software\Microsoft\Windows\CurrentVersion\PropertySystem\PropertyHandlers
```

### WLXPhotoLibraryDatabase.dll — **Library DB & File Support**
```
Software\Microsoft\Windows Live\Photo Gallery
Software\Microsoft\Windows Live\Photo Gallery\Library
Software\Microsoft\Windows Live\Photo Gallery\Library\MDSyncInFlight\Active
Software\Microsoft\Windows Live\Photo Gallery\Viewer
Software\Microsoft\Windows Live\Photo Gallery\FileSupport\DownloadableCodecs
Software\Microsoft\Windows Live\Photo Gallery\FileSupport\DownloadableWLRawCodecs
Software\Microsoft\Windows Live\Photo Gallery\FileSupport\QuickTimeMovieExtensions
Software\Microsoft\Windows Live\Photo Gallery\FileSupport\RawPhotoExtensions
Software\Microsoft\Windows Live\Photo Gallery\SuppressFileTypes
Software\Microsoft\Windows Live\Common
Software\Microsoft\Windows Photo Gallery\Library
Microsoft\Windows Photo Gallery\Original Images
Software\Microsoft\IdentityCRL
Software\Microsoft\Windows Media Foundation\ByteStreamHandlers
SOFTWARE\Microsoft\Windows\CurrentVersion\Media Center\Decoder
Software\Microsoft\Windows\CurrentVersion\PropertySystem\PropertyHandlers
SOFTWARE\Microsoft\Windows\CurrentVersion\ThumbnailCache
```

### WLXPhotoAcq.dll — **Photo Acquisition**
```
Software\Microsoft\Windows Live\Photo Acquisition
Software\Microsoft\Windows Live\Photo Acquisition\CharacterReplacements
Software\Microsoft\Windows Live\Photo Gallery
Software\Microsoft\Windows Live\Photo Gallery\FileSupport\DownloadableCodecs
Software\Microsoft\Windows Live\Photo Gallery\FileSupport\DownloadableWLRawCodecs
Software\Microsoft\Windows Live\Photo Gallery\FileSupport\QuickTimeMovieExtensions
Software\Microsoft\Windows Live\Photo Gallery\FileSupport\RawPhotoExtensions
Software\Microsoft\Windows Live\Photo Gallery\Library
Software\Microsoft\Windows Live\Photo Gallery\SuppressFileTypes
Software\Microsoft\Windows Live\Common
Software\Microsoft\Windows Live\Environment\PhotoGallery
Software\Microsoft\Windows\CurrentVersion\Photo Acquisition\Plugins
Software\Microsoft\Windows\CurrentVersion\PropertySystem\PropertyHandlers
Software\Microsoft\Windows Media Foundation\ByteStreamHandlers
SOFTWARE\Microsoft\Windows\CurrentVersion\Media Center\Decoder
SOFTWARE\Microsoft\Windows NT\CurrentVersion\LanguagePack
```

### WLXMediaPublishSubscribe.dll — **Publish/Subscribe Plugins**
```
Software\Microsoft\Windows Live\Photo Gallery\PublishPlugins
Software\Microsoft\Windows Live\Photo Gallery\SubscribePlugins
Software\Microsoft\Windows Live\Photo Gallery\FlickrAccounts
Software\Microsoft\Windows Live\Photo Gallery\PubSubProviders
Software\Microsoft\Windows Live\Photo Gallery\Library
Microsoft\WindowsLive\PublishPlugins\PersistentData
Software\Microsoft\Windows Live\Common
Software\Microsoft\Windows Live\Environment\PhotoGallery
Software\Microsoft\IdentityCRL
SOFTWARE\Microsoft\Windows NT\CurrentVersion\LanguagePack
```

### WLXPhotoViewer.dll — **Viewer Settings**
```
Software\Microsoft\Windows Live\Photo Gallery
Software\Microsoft\Windows Live\Photo Gallery\Viewer
Software\Microsoft\Windows Live\Photo Gallery\Viewer\Suppressed
Software\Microsoft\Windows Live\Photo Gallery\Slideshow
Software\Microsoft\Windows Live\Photo Gallery\FileSupport\DownloadableCodecs
Software\Microsoft\Windows Live\Photo Gallery\FileSupport\DownloadableWLRawCodecs
Software\Microsoft\Windows Live\Photo Gallery\FileSupport\QuickTimeMovieExtensions
Software\Microsoft\Windows Live\Photo Gallery\FileSupport\RawPhotoExtensions
Software\Microsoft\Windows Live\Photo Gallery\SuppressFileTypes
Software\Microsoft\Windows Live\Common
Software\Microsoft\Windows Live\Environment\PhotoGallery
Software\Microsoft\Windows Live\Installer
Software\Microsoft\Windows Live\Installer\ProductStatus
Software\Microsoft\Windows Live\Movie Maker
Software\Microsoft\Windows Live\Communications Clients\Shared
Software\Microsoft\Windows Live Mail
Software\Microsoft\Windows Media Foundation\ByteStreamHandlers
SOFTWARE\Microsoft\Windows\CurrentVersion\Media Center\Decoder
Software\Microsoft\Windows\CurrentVersion\PropertySystem\PropertyHandlers
```

### WLXSlideshow.dll — **Slideshow Settings**
```
Software\Microsoft\Windows Live\Photo Gallery\Slideshow
Software\Microsoft\Windows Live\Photo Gallery\Slideshow\LastRunSettings
Software\Microsoft\Windows Live\Photo Gallery\WLXGPUPipeline
Software\Microsoft\Windows Live\Photo Gallery\SuppressFileTypes
Software\Microsoft\Windows Live\Photo Gallery\Viewer
Software\Microsoft\Windows Live\Photo Gallery\FileSupport\*
Software\Microsoft\Windows Live\Common
Software\Microsoft\Windows Live\Environment\PhotoGallery
Software\Microsoft\Windows Media Foundation\ByteStreamHandlers
SOFTWARE\Microsoft\Windows\CurrentVersion\Media Center\Decoder
Software\Microsoft\Windows\CurrentVersion\PropertySystem\PropertyHandlers
```

### WLXVideoTrim.dll — **Video Trimming**
```
SOFTWARE\Microsoft\VideoTrim\StreamBufferCopier Settings
SOFTWARE\Microsoft\Windows Live\Common\Movie Library
SOFTWARE\Microsoft\Windows Live\Movie Maker
Software\Microsoft\Windows Live\Photo Gallery\VideoTrim
```

### WLXPhotoCinematic.dll / WLXPhotoClassic.dll / WLXPhotoVoyager.dll — **GPU Pipeline**
```
Software\Microsoft\Windows Live\Photo Gallery\WLXGPUPipeline
Software\Microsoft\Windows Live\Photo Gallery\Slideshow  (Classic only)
```

### WLXPipeline.dll / WLXPipetran.dll — **GPU Pipeline Infrastructure**
```
Software\Microsoft\GPUPipeline
Software\Microsoft\Windows Live\Photo Gallery\WLXGPUPipeline
```

### WLXFaceRecognition.dll — **Face Recognition**
```
Software\Microsoft\MSRA\FaceRecognition
Software\Microsoft\MSRA\FaceRecognition\Iterative
```

### WLXVideoAcquireWizard.exe — **Video Capture**
```
Microsoft\Movie Maker\AddOnTFX
Software\Microsoft\Windows Live
Software\Microsoft\Windows Live\Common
Software\Microsoft\Windows Live\Installer
Software\Microsoft\Windows Live\Installer\ProductStatus
SOFTWARE\Microsoft\MM20\Video Capture Wizard\HD Capture Settings
Software\Policies\Microsoft\CaptureWizard
```

### WLXPhotoSqm.dll — **SQM Telemetry**
```
Microsoft\Windows Live Client Album Viewer\SqmApi
Microsoft\Windows Live Movie Maker\SqmApi
Microsoft\Windows Live Photo Gallery\SqmApi
Microsoft\Windows Live\SqmApi
Software\Microsoft\Windows Live\Ship Asserts
Software\Microsoft\Windows Live\Ship Asserts\Response Table
Software\Microsoft\Windows Live\Client Album Viewer\DebugSQM
Software\Microsoft\Windows Live\Movie Maker\DebugSQM
Software\Microsoft\Windows Live\Photo Gallery\Library\DebugSQM
Software\Microsoft\Windows Live\Photo Gallery\Slideshow\DebugSQM
```

### Shared: wldcore.dll — **Core Windows Live**
```
Software\Microsoft\Windows Live\Common
Software\Microsoft\Internet Explorer
Windows Live\Shared\sqmapi.dll
```

### Shared: wlbici.dll — **BICI Telemetry**
```
Microsoft\Windows Live\Bici
Software\Microsoft\SQMClient
Software\Microsoft\SQMClient\UploadUrlMap
Software\Microsoft\SQMClient\Windows
Software\Microsoft\Windows Live\Common
Software\Microsoft\Windows Live\Common\Activations
Software\Policies\Microsoft\SQMClient
Software\Policies\Microsoft\SQMClient\Windows
SOFTWARE\Microsoft\Reliability Analysis\RAC
```

### Shared: sqmapi.dll — **SQM Client**
```
Software\Microsoft\SQMClient
Software\Microsoft\SQMClient\Windows
Software\Microsoft\SQMClient\Windows\AdaptiveSQM\ManifestInfo
Software\Microsoft\SQMClient\Windows\AdaptiveSQM\Throttling
Software\Microsoft\SQMClient\Windows\CommonDatapoints
Software\Microsoft\SQMClient\Windows\DisabledProcesses
Software\Microsoft\SQMClient\Windows\DisabledSessions
Software\Microsoft\SQMClient\Windows\ServerSync
Software\Microsoft\SQMClient\Windows\Uploader\PendingUpload
Software\Microsoft\SQMClient\Windows\Users
Software\Microsoft\SQMClient\UploadUrlMap
Software\Policies\Microsoft\SQMClient
Software\Policies\Microsoft\SQMClient\Windows
Software\Microsoft\Windows NT\CurrentVersion\UnattendSettings\SQMClient
Microsoft\Windows\SoftwareQualityMetricsClient
SOFTWARE\Microsoft\Reliability Analysis\RAC
```

### Shared: WLMFDS.dll — **Media Foundation Demuxer**
```
SOFTWARE\Microsoft\MPEG2Demultiplexer
SOFTWARE\Microsoft\MPEG2Demultiplexer\Program
SOFTWARE\Microsoft\MPEG2Demultiplexer\Transport
SOFTWARE\Microsoft\Windows Media Foundation\Platform
```

### Shared: uxcore.dll — **UX Framework**
```
Software\Microsoft\Windows Live\Common
Software\Microsoft\Windows Live\Ux
Software\Microsoft\Windows Live\UX\HwBlockList
Software\Microsoft\Office\12.0\Common\ObjectBlocking
SOFTWARE\Microsoft\Windows NT\CurrentVersion\LanguagePack\SurrogateFallback
```

### Shared: uxctl.dll
```
Software\Microsoft\Windows Live\WhatsNew
```

### Other
```
Imaging.dll:             Software\Microsoft\Imaging
WLXDSPA.dll:             (no explicit registry path strings; uses standard APIs)
wlxclip.dll:             Software\Microsoft\GPUPipeline, Software\Microsoft\MM20
                        SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths\dvdmaker.exe
WLXGrinderScheduler.dll: Software\Microsoft\IdentityCRL
WLXImageTranscode.dll:   Software\Microsoft\IdentityCRL
WLXQuickTimeShellExt.dll:(no path strings; CLSID registration only)
RSCMFT.dll:              Software\Classes\CLSID
```

---

## 3. File I/O Operation Map

### Full-Spectrum I/O (Read, Write, Create, Delete, Move, Copy, Find, Map)

| Binary | Create | Read | Write | Delete | Move | Copy | Find | Map | Temp | Dir | Attr |
|--------|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|:---:|
| **MovieMakerCore.dll** | W | R/W | W | Del | MoveEx | Copy | Find | Map | GetTemp | Mkdir | Get/Set |
| **WLXPhotoAcq.dll** | W | R/W | W | Del | MoveEx,Move | CopyEx | Find | Map | GetTemp | Mkdir,RmDir | Get/Set |
| **WLXPhotoLibraryMain.dll** | W | R/W | W/Ex | Del | MoveEx | Copy | Find | Map | GetTemp | Mkdir | Get/Set |
| **WLXPhotoLibraryDatabase.dll** | W | R/W | W | Del | MoveEx | Copy | Find | Map | GetTemp | Mkdir | Get/Set |
| **WLXPhotoViewer.dll** | W | R/W | W | Del | MoveEx | Copy | Find | Map | GetTemp | Mkdir | Get/Set |
| **WLXPhotoSqm.dll** | W | - | W | Del | - | Copy | - | - | GetTemp | Mkdir | Get |
| **WLXMediaPublishSubscribe.dll** | W | R/W | W | Del | - | - | - | Map | GetTemp | - | Get/Set |
| **uxcore.dll** | W | R/W | W | Del | - | Copy | - | Map | GetTemp | Mkdir | Get |
| **wlbici.dll** | W | R/W | W | Del | - | - | Find | Map | GetTemp | Mkdir | Get/Set |
| **WLXVAFilt.dll** | W | R/W | W | Del | - | - | - | - | GetTemp | - | - |
| **WLXVideoTrim.dll** | W | - | W | Del | MoveEx | - | - | - | GetTemp | - | Get |
| **WLXAlbumDownloadWizard.exe** | W | R/W | W | Del | MoveEx | - | - | - | GetTemp | - | Get/Set |
| **WLXVideoAcquireWizard.exe** | W | R/W | W | Del | MoveEx | - | Find | - | GetTemp | Mkdir | Get |
| **sqmapi.dll** | W | R/W | W | Del | - | - | Find | Map | GetTemp | Mkdir | Get/Set |
| **RSCMFT.dll** | W | R/Ex | W/Ex | Del | - | - | - | - | GetTemp | - | Get |
| **WLXDSPA.dll** | W | R/W | W | - | - | - | - | - | - | - | Get |

### Read-Only / Minimal I/O

| Binary | Operations |
|--------|-----------|
| **MetadataSys.dll** | GetFileAttributes only |
| **WLXGrinderScheduler.dll** | GetFileAttributes only |
| **WLXImageTranscode.dll** | GetFileAttributes only |
| **WLXPipeline.dll** | GetFileAttributes only |
| **WLXPhotoGallery.exe** | GetFileAttributes only |
| **WLXPhotoAcquireWizard.exe** | GetFileAttributes only |
| **WLXQuickTimeControlHost.exe** | GetFileAttributes only |
| **WLXPhotoGalleryRepair.exe** | GetFileAttributes, PathRemoveFileSpec |
| **WLXPhotoCinematic.dll** | PathRemoveFileSpec only |
| **WLXPhotoClassic.dll** | PathRemoveFileSpec only |
| **WLXPhotoVoyager.dll** | PathRemoveFileSpec only |
| **WLXMovieLibrary.dll** | GetFileAttributes, PathRemoveFileSpec |

### Network I/O

| Binary | Operations |
|--------|-----------|
| **uxctl.dll** | InternetReadFile, InternetWriteFile |

---

## 4. File Extension Map

### Video Container Formats

| Extension | Referenced By |
|-----------|--------------|
| **.wlmp** | MovieMakerCore.dll (native project format) |
| **.wmv** | wlxclip, MovieMakerCore, PhotoAcq, PhotoLibraryDatabase, PhotoLibraryMain, PhotoViewer, Pipeline, Pipetran, SendMail, Slideshow, AlbumDownloadWizard, VideoAcquireWizard, Transcode |
| **.mp4** | MovieMakerCore, wlxclip, PhotoAcq, PhotoLibraryDatabase, PhotoLibraryMain, PhotoViewer, MediaPublishSubscribe, VideoTrim, Transcode |
| **.avi** | wlxclip, MovieMakerCore, PhotoAcq, PhotoLibraryDatabase, PhotoLibraryMain, PhotoViewer, Pipeline, Pipetran, SendMail, Slideshow, AlbumDownloadWizard, VideoAcquireWizard |
| **.mov** | MovieMakerCore, wlxclip, PhotoAcq, PhotoLibraryDatabase, PhotoLibraryMain, PhotoViewer, SendMail, Slideshow, AlbumDownloadWizard, VideoTrim |
| **.asf/.ASF** | MovieMakerCore, wlxclip, PhotoAcq, PhotoLibraryDatabase, PhotoLibraryMain, PhotoViewer, Pipeline, Pipetran |
| **.3gp/.3gp2/.3gpp** | MovieMakerCore, PhotoAcq, PhotoLibraryDatabase, PhotoLibraryMain, PhotoViewer, SendMail, Slideshow, AlbumDownloadWizard, VideoTrim |
| **.mpg/.MPEG** | MetadataSys, MovieMakerCore, wlxclip, PhotoAcq, PhotoLibraryDatabase, PhotoLibraryMain, PhotoViewer, Pipeline, Pipetran, VideoAcquireWizard |
| **.wtv** | TranscodeConfig.xml only |
| **.dvr-ms** | TranscodeConfig.xml only |
| **.mts/.m2ts/.m2t** | TranscodeConfig.xml only |
| **.m4v** | PhotoLibraryMain |
| **.flv/.mkv** | (not found) |

### Audio Formats

| Extension | Referenced By |
|-----------|--------------|
| **.wma** | MovieMakerCore, wlxclip, PhotoAcq, Transcode |
| **.mp3** | wlxclip, PhotoAcq, Pipeline |
| **.wav** | wlxclip, PhotoAcq, Pipeline |
| **.m4a** | MovieMakerCore, Transcode |

### Image Formats

| Extension | Referenced By |
|-----------|--------------|
| **.jpg/.jpeg** | wlxclip, MovieMakerCore, PhotoAcq, PhotoLibraryDatabase, PhotoLibraryMain, PhotoViewer, Pipeline, SendMail, Slideshow, AlbumDownloadWizard, VideoAcquireWizard, MediaPublishSubscribe, WLXDSPA, WLXImageTranscode |
| **.png** | wlxclip, MovieMakerCore, PhotoAcq, PhotoLibraryDatabase, PhotoLibraryMain, PhotoViewer, Pipeline, Pipetran, SendMail, Slideshow, AlbumDownloadWizard, VideoAcquireWizard, Imaging, uxcore |
| **.bmp/.BMP** | wlxclip, PhotoAcq, PhotoLibraryDatabase, PhotoLibraryMain, PhotoViewer, Pipeline, SendMail, Slideshow, AlbumDownloadWizard, VideoAcquireWizard, WLXImageTranscode |
| **.gif/.GIF** | wlxclip, PhotoAcq, PhotoLibraryDatabase, PhotoLibraryMain, PhotoViewer, Pipeline, SendMail, Slideshow, AlbumDownloadWizard |
| **.tif/.tiff** | wlxclip, PhotoAcq, PhotoLibraryDatabase, PhotoLibraryMain, PhotoViewer, Pipeline, SendMail, Slideshow, AlbumDownloadWizard, VideoAcquireWizard, Imaging |
| **.ico/.ICO** | MovieMakerCore, PhotoAcq, PhotoLibraryDatabase, PhotoLibraryMain, PhotoViewer, SendMail, Slideshow, AlbumDownloadWizard |
| **.wmf/.WMF** | wlxclip, Pipeline, VideoAcquireWizard |
| **.emf/.EMF** | wlxclip, Pipeline, VideoAcquireWizard |
| **.dng** | PhotoAcq |
| **.psd** | PhotoAcq |
| **.raw** | PhotoAcq |

### Configuration & Data Files

| Extension | Referenced By |
|-----------|--------------|
| **.xml/.XML** | MovieMakerCore, wlxclip, PhotoAcq, PhotoLibraryMain, PhotoViewer, PhotoCinematic, PhotoVoyager, WLXDSPA, MediaPublishSubscribe, PhotoSqm, Pipeline, Pipetran, Transcode, VideoAcquireWizard, MovieLibrary |
| **.dat/.Dat** | MovieMakerCore, MediaPublishSubscribe, uxctl |
| **.db** | PhotoAcq |
| **.ini** | PhotoAcq |
| **.sqm** | PhotoSqm, wlbici, wldcore |
| **.wlvs** | Video Profiles/Default.wlvs (video profile binary) |
| **.wlmx** | MovieMakerTemplates/ (191 template files) |
| **.wlms** | MovieMakerTemplates/ (10 script files) |

---

## 5. Configuration Storage Patterns

### Pattern 1: Common Settings Hub (shared across many DLLs)
Almost every DLL reads `Software\Microsoft\Windows Live\Photo Gallery` and `Software\Microsoft\Windows Live\Common`. These are **central configuration keys** that multiple binaries query at startup.

### Pattern 2: File Type Registry (codec/format support)
A consistent set of 4 registry keys controls supported file types, read by 8+ DLLs:
```
Software\Microsoft\Windows Live\Photo Gallery\FileSupport\DownloadableCodecs
Software\Microsoft\Windows Live\Photo Gallery\FileSupport\DownloadableWLRawCodecs
Software\Microsoft\Windows Live\Photo Gallery\FileSupport\QuickTimeMovieExtensions
Software\Microsoft\Windows Live\Photo Gallery\FileSupport\RawPhotoExtensions
```

### Pattern 3: Movie Maker-specific Settings
Owned by MovieMakerCore.dll with contributions from WLXVideoTrim.dll, WLXMovieLibrary.dll, and WLXTranscode.exe:
```
Software\Microsoft\Windows Live\Movie Maker
Software\Microsoft\Windows Live\Movie Maker\Post
Software\Microsoft\Windows Live\Movie Maker\Recent
Software\Microsoft\Windows Live\Movie Maker\RecentWLVS
Software\Microsoft\Windows Live\Movie Maker\Suppressed
SOFTWARE\Microsoft\Windows Live\Common\Movie Library
```

### Pattern 4: GPU Pipeline Configuration
Distributed across 5 DLLs:
```
Software\Microsoft\GPUPipeline                      (WLXPipeline, wlxclip)
Software\Microsoft\Windows Live\Photo Gallery\WLXGPUPipeline  (WLXPipeline, WLXPipetran, WLXPhotoCinematic, WLXPhotoClassic, WLXPhotoVoyager, WLXSlideshow)
```

### Pattern 5: Telemetry / SQM
Three DLLs manage Windows Live telemetry:
- **sqmapi.dll**: Core SQM client infrastructure (20+ keys under `Software\Microsoft\SQMClient`)
- **wlbici.dll**: BICI (Behavioral Infrastructure for Client Insights) telemetry
- **WLXPhotoSqm.dll**: Product-specific SQM data for Movie Maker, Photo Gallery, Album Viewer

### Pattern 6: Identity / Authentication
```
Software\Microsoft\IdentityCRL   (WLXGrinderScheduler, WLXImageTranscode, WLXFaceRecognition, 
                                  WLXMediaPublishSubscribe, WLXPhotoLibraryDatabase, wlxclip,
                                  WLXAlbumDownloadWizard, WLXPhotoAcquireWizard, WLXPhotoGallery,
                                  WLXVideoAcquireWizard, WLXPhotoViewer)
```

### Pattern 7: Temp File Lifecycle
20+ DLLs use `GetTempPathW` + `GetTempFileNameW`, indicating heavy temporary file creation during:
- Photo acquisition/transcoding (WLXPhotoAcq, WLXImageTranscode)
- Video trimming (WLXVideoTrim)
- Slideshow rendering (WLXSlideshow)
- Library indexing (WLXPhotoLibraryDatabase, WLXPhotoLibraryMain)
- Publishing (WLXMediaPublishSubscribe, WLXSendMail)
- BICI/SQM upload staging (wlbici, sqmapi)

### Pattern 8: Memory-Mapped File I/O
12+ DLLs use `CreateFileMappingW` + `MapViewOfFile`, indicating large binary data processing:
- MovieMakerCore.dll, WLXPhotoAcq.dll, WLXPhotoLibraryMain.dll, WLXPhotoLibraryDatabase.dll
- WLXPhotoViewer.dll, WLXMediaPublishSubscribe.dll, uxcore.dll, wlbici.dll, sqmapi.dll
- WLMFDS.dll, wlxclip.dll, D3DCOMPILER_46.dll

---

## 6. Cross-DLL Dependency Matrix

| Domain | Owner DLLs | Shared Dependencies |
|--------|-----------|-------------------|
| **Movie Editing** | MovieMakerCore, WLXVideoTrim, WLXTranscode, WLXPipeline | Common, Environment, Movie Library, ByteStreamHandlers |
| **Photo Viewing** | WLXPhotoViewer, WLXPhotoClassic, WLXPhotoCinematic, WLXPhotoVoyager | Photo Gallery, GPUPipeline, Viewer, Slideshow |
| **Photo Library** | WLXPhotoLibraryMain, WLXPhotoLibraryDatabase | Library, FileSupport, SuppressFileTypes |
| **Photo Acquisition** | WLXPhotoAcq, WLXPhotoAcquireWizard | Photo Acquisition, FileSupport, PropertyHandlers |
| **Video Acquisition** | WLXVideoAcquireWizard, WLXVideoCameraAutoPlayManager | MM20, CaptureWizard |
| **Publishing** | WLXMediaPublishSubscribe, WLXSendMail, WLFacebookPlugin, WLFlickrPlugin, WLVimeoPlugin, WLYouTubePlugin | PublishPlugins, SubscribePlugins |
| **GPU Effects** | WLXPipeline, WLXPipetran, WLXPhotoCinematic, WLXPhotoClassic, WLXPhotoVoyager | GPUPipeline, WLXGPUPipeline |
| **Telemetry** | sqmapi, wlbici, WLXPhotoSqm | SQMClient, Bici |
| **Identity** | wlidux, WLXGrinderScheduler, WLXImageTranscode | IdentityCRL |
| **Codec/Transcode** | WLXCodecHost, WLXMP4Parser, WLXVAFilt, WLXDSPA, MPG4DEMUX, WLMFDS, WLMFReadWrite, WLAVRes | MF Platform, ByteStreamHandlers |

---

## 7. Key CLSID Registrations

### Media Foundation GUIDs (shared across multiple DLLs)
```
{8095E7A5-4AF7-448d-9548-0DBE027FBEB0}  — MF Transcode DLL (wlxclip, MediaPublish, Pipeline, Pipetran, VideoTrim, VideoAcquire, VideoCamera)
{33564D57-0000-0010-8000-00AA00389B71}  — WMV Media Type
{73646976-0000-0010-8000-00AA00389B71}  — DV Media Type
{73647561-0000-0010-8000-00AA00389B71}  — DVA Media Type
{00000161-0000-0010-8000-00AA00389B71}  — ASF Media Type
{DC619D43-32E3-4981-9D9A-DBE91A8B2061}  — MF Source Reader (wlxclip, PhotoLibraryMain, PhotoViewer)
```

### Windows Live Component GUIDs
```
{66557ED9-C5F8-4815-A8CC-D157272CFFCE}  — MovieMakerLang, MediaPublish, PhotoLibraryMain
{8BE133F2-9F23-4344-B5A1-A49BDD09FC0F}  — WLXMediaPublish, PhotoLibraryMain, uxctl
{8DB100C7-50C5-46EB-B535-618AB68A3E22}  — WLXMediaPublish, PhotoLibraryMain, uxctl
{30B71B3D-CC48-4650-8D0A-1A106B282AF5}  — MovieMakerCore, PhotoLibraryMain
{84FBA192-4F8D-4a5d-94DE-083446ACC1D0}  — PhotoAcq, Slideshow, AlbumDownload, VideoAcquire
```

### Shell Extension GUIDs
```
{083863F1-70DE-11d0-BD40-00A0C911CE86}  — IShellFolder (wlxclip)
{BB2E617C-0920-11d1-9A0B-00C04FC2D6C1}  — IExtractImage (QuickTimeShellExt)
{E357FCCD-A995-4576-B01F-234630154E96}  — IThumbnailProvider (QuickTimeShellExt)
```

### QuickTime Support
```
{007EFBDF-8A5D-4930-97CC-A4B437CBA777}  — QT thumbnail
{0A18A436-2A7A-49F3-A488-30538A2F6323}  — QT extension
```
