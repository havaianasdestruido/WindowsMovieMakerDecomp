# WLXVideoAcquireWizard.exe Analysis

## Overview
Video acquisition (capture/import) wizard from Windows Live Movie Maker 2012 (Photo Gallery suite). A **standalone GUI wizard** (`Windows GUI` subsystem) that guides users through importing video from DV camcorders (IEEE 1394), HDV cameras, and other capture devices. Uses DirectShow filter graphs for capture, WMV profile management for encoding, and COM/ATL framework for UI structure.

- **Files:** `WLXVideoAcquireWizard.exe` + `WLXVideoAcquireWizardResources.dll`
- **Assembly Name:** `Microsoft.Windows.personalMedia.WLXVideoAcquireWizard` (v5.1.0.0)
- **Build:** `16.4.3528.0331_ship.client.main.w5m4 (ship)`
- **Timestamp:** 2014-04-01 01:26:51 (EXE) / 01:28:22 (DLL)
- **EXE Size:** 870,592 bytes (850 KB)
- **DLL Size:** 108,736 bytes (106 KB) — resource-only satellite DLL
- **PDB:** `WLXVideoAcquireWizard.pdb`, GUID `{F1DC3B12-48D2-48B9-91E2-6E14F6E41D0B}`, age 1
- **FileVersion:** 16.4.3528.331.16.4
- **ProductVersion:** 16.4.3528.331.0.63
- **LegalCopyright:** `(c) 2012 Microsoft Corporation. All rights reserved.`

## PE Structure — EXE

| Section | VA | VirtSize | RawSize | Flags |
|---------|------|----------|---------|-------|
| `.text`  | 0x1000 | 0x8A142 | 0x8A200 | Code + Execute Read |
| `.data`  | 0x8C000 | 0x75BC | 0x5800 | Init Data + Read Write |
| `.rsrc`  | 0x94000 | 0xBB50 | 0xBC00 | Init Data + Read Only |
| `.reloc` | 0xA0000 | 0x34E58 | 0x35000 | Init Data + Discardable + Read Only |

**EXE Headers:**
- **Machine:** x86 (0x14C), **Subsystem:** Windows GUI (2)
- **Linker:** MSVC 11.00 (VS 2012)
- **Entry Point:** 0x441E3 (`__wmainCRTStartup` / C runtime init)
- **Image Base:** 0x400000
- **Image Size:** 0xD5000 (872 KB)
- **DLL Characteristics:** Dynamic Base (ASLR), NX Compatible (DEP), Terminal Server Aware
- **Stack:** 512KB reserve / 64KB commit
- **Heap:** 1MB reserve / 4KB commit
- **Checksum:** 0xDB191

## PE Structure — Resources DLL

| Section | VA | VirtSize | RawSize | Flags |
|---------|------|----------|---------|-------|
| `.rsrc`  | 0x1000 | 0x167C0 | 0x16800 | Init Data + Read Only |

**DLL Headers:**
- **Machine:** x86 (0x14C), **Subsystem:** Windows CUI (3) — typical for resource-only DLLs
- **Entry Point:** 0 (no DllMain code — pure resource satellite)
- **Image Base:** 0x10000000
- **DLL Characteristics:** Dynamic Base (ASLR), NX Compatible (DEP)
- **No imports** — resource-only DLL, loaded by the EXE via `LoadLibraryEx` + `FindResource`/`LoadResource`

## Manifest

Embedded Win32 manifest (RT_MANIFEST, ID=1, Lang=1033):
- **Assembly:** `Microsoft.Windows.personalMedia.WLXVideoAcquireWizard` v5.1.0.0 (x86)
- **DPI Aware:** Yes
- **Common Controls v6.0** dependency (visual styles)
- **Requested Execution Level:** `asInvoker` (no elevation required)
- **Trust Info:** `requestedExecutionLevel level="asInvoker" uiAccess="false"`

## Export Table — EXE

**None.** This is a standalone application, not a DLL.

## Export Table — Resources DLL

**None.** Resource-only satellite DLL with no code exports.

## Import Table — EXE

### Direct Imports (load-time)

| DLL | Functions | Purpose |
|-----|-----------|---------|
| **ADVAPI32.dll** | 14 | Registry operations (RegCreateKey, RegOpenKeyEx, RegQueryValueEx, RegSetValueEx, RegDeleteKey, RegEnumKeyEx, etc.) |
| **KERNEL32.dll** | 120+ | Process/thread management, file I/O, module loading, synchronization (CRITICAL_SECTION, Events, Mutexes), resources, locale/language, string formatting, memory |
| **GDI32.dll** | 16 | GDI drawing: CreatePen, CreateSolidBrush, CreateFontIndirect, BitBlt, Rectangle, text rendering |
| **USER32.dll** | 100+ | Window management: CreateWindowEx, DialogBoxParam, PropertySheetW, MessagePump, controls, timers, clipboard, monitors, system metrics |
| **MSVCR110.dll** | 45+ | VC++ 2012 CRT: memory allocation, exception handling, string operations, file I/O, thread creation (_beginthreadex) |
| **SHELL32.dll** | 8 | Shell operations: ShellExecuteEx, SHFileOperation, SHGetSpecialFolderPath, SHGetDiskFreeSpaceEx, CommandLineToArgvW |
| **ole32.dll** | 20 | COM: CoCreateInstance, CoInitializeEx, CoGetClassObject, CoRegisterClassObject, CoCreateGuid, OleInitialize, clipboard |
| **OLEAUT32.dll** | 22 ordinals | COM automation: BSTR/SafeArray operations, VariantInit, SysAllocString, etc. |
| **COMCTL32.dll** | 8 | Common Controls: PropertySheetW, PropertySheetPage, InitCommonControlsEx, ImageList operations |
| **SHLWAPI.dll** | 7 | Path utilities: PathAppend, PathFileExists, PathFindFileName, PathStripToRoot, StrRChr, StrStrI |
| **msi.dll** | 1 ordinal | Windows Installer query |
| **VERSION.dll** | 3 | File version info: GetFileVersionInfo, VerQueryValue |
| **UxTheme.dll** | 4 | Visual themes: OpenThemeData, GetThemeColor, SetWindowTheme, CloseThemeData |
| **SETUPAPI.dll** | 5 | Device enumeration: SetupDiOpenDeviceInterface, SetupDiGetDeviceInterfaceDetail, SetupDiCreateDeviceInfoList |
| **RPCRT4.dll** | 5 | UUID operations: UuidCreate, UuidEqual, UuidToString, UuidFromString, RpcStringFree |
| **WMVCore.DLL** | 1 | `WMCreateProfileManager` — Windows Media profile management |
| **d3d9.dll** | 1 | `Direct3DCreate9` — Direct3D 9 for video rendering |
| **gdiplus.dll** | 19 | GDI+: bitmap operations, image thumbnail, save, encoder enumeration, graphics context |

### Delay-Load Imports

| DLL | Functions | Purpose |
|-----|-----------|---------|
| **UXCore.dll** | 9 | UXCore framework: `UXCoreInitProcess`, `UXCoreUnInitProcess`, `UXCoreInitThread`, `UXCoreUnInitThread`, `RMUpdateResourceSet`, `RMFindModule`, `RMLoadImage`, `RMLoadString`, `RMLoadIcon` |

### Key External DLL

| DLL | Purpose |
|-----|---------|
| **WLXVideoTrim.dll** | `CreateAVICopierDirect` — imported for AVI file copying during import |

## COM Architecture

### Custom COM Classes (CLSID registered)

| CLSID | Class | Purpose |
|-------|-------|---------|
| `{5F5AFF4A-2F7F-4279-88C2-CD88EB39D144}` | `CCapWizModule` | Main ATL module for the capture wizard |
| `{8095E7A5-4AF7-448d-9548-0DBE027FBEB0}` | (external) | Referenced via InprocServer32 path |

### Internal COM Objects (from RTTI)

**ATL Module / App:**
- `CAppModule@WTL` — WTL application module
- `CComModule@ATL` — ATL module
- `CCapWizModule` — Custom capture wizard ATL module

**Wizard UI (Property Sheet Pages):**
- `CRecordWizard` — Main wizard (CPropertySheetImpl)
- `CRecWiz_DevicePage` — Device selection page (Step 1)
- `CRecWiz_MethodPage` — Import method selection page (Step 2: entire tape vs. parts)
- `CRecWiz_AutoRecordPage` — Automatic recording/import page
- `CRecWiz_ManualRecordPage` — Manual recording/import page (with DV camera controls)

**Capture & Device Management:**
- `CCapture` / `CCaptureBase` — DirectShow capture graph management
- `CDeviceMgr` — Device enumeration and management
- `CDeviceInterface` — Device interface wrapper
- `CDevice` / `CHardwareDevice` — Individual device representation
- `CEnumDevices` — Device enumerator
- `CDVState` — DV camcorder state tracking
- `CCaptureFileCollection` — Collected capture files

**DirectShow Filter Classes:**
- `CVideoSourceFilter` / `CAudioSourceFilter` — Source filters for video/audio capture pins
- `CWMTCaptureFilter` / `CWMTFilter` — Windows Media Technology capture filters
- `CDVSplitter15Filter` / `CDVSplitterDynamicFilter` / `CNormalDVSplitterFilter` — DV stream splitters
- `CDVDecoderAVIShotDetectFilter` / `CDVDecoderCaptureFilter` / `CDVDecoderPreviewFilter` — DV decoder variants
- `CDVInfoExtractFilter` — DV metadata extraction
- `CMPEG2DeMuxCaptureFilter` / `CMPEG2DeMuxFilter` / `CMPEG2DeMuxPreviewFilter` — MPEG-2 demux variants
- `CMPEG2DecoderFilter` / `CMPEG2DecoderPreviewFilter` — MPEG-2 decoder variants
- `CMPEG2VideoStreamAnalyzerFilter` — MPEG-2 stream analysis
- `CVideoRendererFilter` / `CVideoNullRendererFilter` / `CAudioRendererFilter` / `CAudioNullRendererFilter` — Renderers
- `CVideoSmartTeeFilter` / `CAudioSmartTeeFilter` / `CSmartTeeFilter` — Stream splitting
- `CVideoRecordQueueFilter` / `CAudioRecordQueueFilter` / `CRecordQueueFilter` — Queue buffers
- `CSceneDetectContentFilter` / `CSceneDetectDVFilter` — Scene change detection
- `CAVIMUXFilter` / `CAVIWriterFilter` — AVI muxing/writing
- `CVMRFilter` — Video Mixing Renderer filter
- `CStreamBufferEngineSinkFilter` — Stream Buffer Engine (DVR-MS recording)
- `CSampleGrabberFilter` — Sample grabber for frame capture
- `CSwitchFilter` — Stream switching
- `CInfiniteTeeFilter` — Infinite output pin splitter
- `CWMTSampleInfoFilter` — WMV sample info
- `CWMTProfile` / `CWMCollectionImpl` — WMV profile management

**Windows Media Profile/Config:**
- `CWMProfileManager` — Profile management
- `CWMProfile` — Individual profile
- `CWMStreamConfig` — Stream configuration
- `CWMErrorObject` — Error handling object
- `CGraphManager` / `CWMModule` / `CWMTWizardBase` / `CWMObjectModelBase` — Core WM framework

**UI Window Classes:**
- `CCaptureWizard` — Main capture wizard window
- `CDeviceWindow` — Device info/selection window
- `CVideoPreviewWindow` / `CVideoPreviewBorderWindow` — Video preview
- `CFullScreenWindow` — Full-screen video playback
- `CToolBarCtrlEx` — Custom toolbar
- `CQueueUIDLG` — Queue processing UI dialog
- `CWMTCMessageBox` — Message box wrapper
- `CSelectionMgr` — Selection management

**Helper/Utility:**
- `CCaptureUtil` / `CDeviceUtil` / `CProfileUtil` / `CGraphUtil` / `CWMTGraphUtil` / `CWMTMediaTypeUtil` — Utility classes
- `CRecordUserPreferences` — User preferences (from registry)
- `CFileTester` — File validation
- `CFireGun` — Scene change trigger
- `CDiskSpaceMonitor` — Disk space monitoring
- `CSplitProgressUI` — Split operation progress UI
- `CGDIImage` — GDI+ image wrapper
- `CEnumVariant` — VARIANT enumerator

### IDispatch Interface Implementations

| Interface | GUID | Purpose |
|-----------|------|---------|
| `UIMMCapture2` | `{35C61E89-1501-435C-9124-8DE4A38D2A8C}` | Capture device control dispatch |
| `UIMMDVDevice2` | `{90752A4D-01DE-481F-95DD-66234E99A1CC}` | DV device dispatch |
| `UIMMDVState2` | `{C1BEF173-2C74-4155-BF96-86EB6D9F833F}` | DV state dispatch |
| `UIMMDeviceManager` | `{8781C19E-A4C0-4A73-952F-EF715CAAE0F4}` | Device manager dispatch |
| `UIWMErrorObject` | `{B80E5B6D-9A79-4490-B0F8-E70BB0096144}` | Error object dispatch |
| `UIWMTGraphManager` | `{84CDE77C-23B4-4C35-BC6B-3225339152DA}` | Graph manager dispatch |
| `UIWMTProfile2` | `{B452A0F6-8898-4752-B546-96888D826F98}` | Profile dispatch |
| `UIWMTProfileManager` | `{BB56DCC5-6C29-4B1E-87C7-2B4CCB9A5B79}` | Profile manager dispatch |
| `UIWMTStreamConfig2` | `{DB21AE51-8380-4505-991C-0C168D4315E2}` | Stream config dispatch |
| `UIAxWinAmbientDispatchEx` | `{B2D0778B-AC99-4C58-A5C8-E772409C9001}` | ActiveX host ambient dispatch |

### Other GUIDs

| GUID | Purpose |
|------|---------|
| `{84FBA192-4F8D-4A5D-94DE-083446ACC1D0}` | Application identifier (found in registry/settings) |

## Resources — EXE

| Type | ID | Size | Description |
|------|-----|------|-------------|
| REGISTRY (102) | 1033 | 1,506 B | COM registration info |
| RT_PRX (custom) | A000 | 2,634 B | Custom property sheet resource |
| RT_PRX (custom) | CAPWIZ | 6,462 B | Capture wizard layout |
| RT_PRX (custom) | MMFIT | 5,802 B | Movie Maker Fit-To-Disk profile |
| RT_PRX (custom) | MMRECOMMENDED | 6,806 B | Movie Maker Recommended profile |
| RT_PRX (custom) | VN000 | 3,318 B | Video profile (NTSC) |
| RT_PRX (custom) | VP000 | 3,318 B | Video profile (PAL) |
| RT_ICON (3) | 1-9 | various | Application icons (9 sizes/variants) |
| RT_GROUP_ICON (14) | 129 | 132 B | Icon group |
| RT_VERSION (16) | 1 | 860 B | Version information |
| RT_MANIFEST (24) | 1 | 1,181 B | Application manifest |

## Resources — DLL

| Type | ID | Size | Description |
|------|-----|------|-------------|
| MUI | 1 | 248 B | MUI resource descriptor |
| RT_BITMAP (2) | 302 | 11,240 B | Bitmap resource |
| RT_BITMAP (2) | 2332 | 18,472 B | Bitmap resource |
| RT_ICON (3) | 1-15 | various | Icons (15 variants, likely for device states) |
| RT_DIALOG (5) | 102, 205, 207, 234, 292, 347, 406, 1879 | various | 8 dialog templates |
| RT_STRING (6) | 7-42+ (49 IDs) | various | 49 string table blocks |
| RT_RCDATA (11) | 1 | 7,916 B | Raw data (likely wizard page data) |
| RT_GROUP_ICON (14) | 128, 424, 425 | various | Icon groups |
| RT_VERSION (16) | 1 | 988 B | Version information |
| Custom (241) | 302 | 22 B | Unknown custom resource |

## Registry Paths Used

- `Software\Policies\Microsoft\CaptureWizard` — Group policy settings
- `Software\Microsoft\Windows Live` — Windows Live product info
- `Software\Microsoft\Windows Live\Common` — Common settings (TOUVersion, SuiteLanguage, InstalledLanguages)
- `Software\Microsoft\Windows Live\Installer` — Product status, install info
- `Software\Microsoft\Windows Live\Installer\ProductStatus` — Product status flags
- `SOFTWARE\Microsoft\MM20\Video Capture Wizard\HD Capture Settings` — HD capture settings
- `HKEY_CLASSES_ROOT`, `HKEY_CURRENT_USER`, `HKEY_LOCAL_MACHINE`, `HKEY_USERS` — Root hive access
- `\RecordSettings` — Capture device settings
- `\RecordSettings\Add Device` — Whitelisted devices
- `\RecordSettings\Ignore Device` — Blacklisted devices
- `\RecordSettings\CaptureOnlyList` — Capture-only devices
- `CLSID\{5F5AFF4A-2F7F-4279-88C2-CD88EB39D144}` — COM class registration

## Wizard Page Flow

The wizard uses `PropertySheetW` (Win32 Property Sheet API) with WTL `CPropertySheetImpl`:

1. **DevicePage** (`CRecWiz_DevicePage`) — "Select the device you want to use to import video"
2. **MethodPage** (`CRecWiz_MethodPage`) — "Import entire videotape or just parts?"
3. **AutoRecordPage** (`CRecWiz_AutoRecordPage`) — Auto-import entire tape with progress
4. **ManualRecordPage** (`CRecWiz_ManualRecordPage`) — Manual import with camera controls (play/stop/rewind/FF/prev/next frame)

## User-Visible Strings (from DLL)

### Wizard UI Strings
- "Import video" / "Import live video" / "Import HD video"
- "Select the device you want to use to import video"
- "Import entire videotape or just parts?"
- "Cue the videotape and then start importing video"
- "Use the camera controls to find the scene you want, and click Import"
- "&Import entire videotape or just parts?"
- "Import the &entire video"
- "Choose &parts of the video to import"
- "Completing video import"
- "Importing video..."
- "Stop importing after (min):"
- "Show &widescreen preview"
- "Burn the entire video to &DVD"
- "DVD &menu title:"

### DV Camera State Messages
- "The digital video camera has been paused."
- "The digital video camera is rewinding."
- "The digital video camera is in fast-forward mode."
- "The digital video camera is in play mode."
- "The digital video camera is in record mode."
- "The digital video camera has been stopped."
- "The digital video camera has been disconnected."

### Import Progress/Error Messages
- "Importing video..." / "Saving file..." / "Rewinding tape..."
- "Time remaining: %1!d! seconds" / "%1!d! minutes, %2!d! seconds"
- "Video file size:" / "Dropped frames:" / "Tape position:"
- "Photo Gallery has successfully imported %1!s! of your tape."
- Various disconnection and error messages for DV device loss, disk space, 4GB FAT32 limit, time limit

### Device Type Strings (video inputs)
- TV Tuner, Composite, SVideo, RGB, YRYBY, SerialDigital, ParallelDigital
- SCSI, AUX, 1394, USB, VideoDecoder, VideoEncoder

### Device Type Strings (audio inputs)
- Black, Tuner, Line, Mic, AESDigital, SPDIFDigital, SCSI, AUX, 1394, USB, AudioDecoder

### Profile Names
- "Movie Maker Recommended" (RT_PRX: MMRECOMMENDED)
- "Movie Maker Fit To Disk" (RT_PRX: MMFIT)
- "Capture Wizard Profile" (RT_PRX: CAPWIZ)
- "High-quality video (NTSC)" / "High-quality video (PAL)" / "High-quality audio"

### WMV Profile Templates (embedded XML)
- Profile versions `458752` (v6) and `589824` (v9)
- Video bitrates: 0, 100000, 1799000, 4000000 bps
- Audio format: `waveformatex wFormatTag="353"` (WMA)
- Stream config with video/audio major types and subtypes for WMV, DV, MPEG-2
- Data unit extensions for WMV content protection (`{1B1EE554-F9EA-4BC8-821A-376B74E4C4B8}`)

### Key Configuration Values (from registry strings)
- `VideoDevice`, `AudioDevice` — Selected capture devices
- `DV Capture Type`, `DV Capture Mode` — DV capture settings
- `Shot Detect On Import` — Scene detection
- `Audio Line Volume Level` — Audio input level
- `Manual Record Uses Time Limit` / `Manual Record Time Limit` — Time-limited recording
- `Minimum Split Offset` — Scene split threshold
- `Show DV Preview During Capture` / `Preview 16x9 During Capture` — Preview options
- `StripMediaFiles`, `NormalizeAudio` — Post-processing options
- `Profiles` — WMV profile path

## Key Dependencies & Frameworks

- **ATL (Active Template Library):** COM class implementation (CComObject, CComAggObject, CComModule, CComContainedObject, etc.)
- **WTL (Windows Template Library):** UI framework (CPropertySheetImpl, CPropertyPageImpl, CDialogImpl, CWindowImpl, CThemeImpl, CToolBarCtrlT, etc.)
- **DirectShow:** All video/audio capture, rendering, and processing via filter graphs
- **Windows Media Format SDK (WMVCore.DLL):** Profile management (`WMCreateProfileManager`)
- **Direct3D 9 / VMR-9:** Video rendering with hardware acceleration
- **GDI+:** Bitmap/image operations for thumbnails and preview
- **COMCTL32:** Property sheet, image lists, common controls
- **UXCore (delay-load):** Windows Live UX framework for resource management, theming, localized resources
- **WLXVideoTrim.dll (imported):** `CreateAVICopierDirect` for AVI file operations during import

## Global Mutex/Events

- `Global\CaptureWizard` — Single-instance mutex (prevents multiple wizard instances)
- `Windows_Capture_Still_Alive` — Heartbeat signal for the capture process

## Key Design Observations

1. **ATL/WTL architecture:** The entire application is built on ATL COM + WTL UI framework. The main wizard (`CRecordWizard`) extends `CPropertySheetImpl` with four wizard pages, each extending `CPropertyPageImpl` via intermediate `CWMTPropertyPageImpl` and `CWMTWizardPageBase` base classes.

2. **COM-heavy design:** Extensive use of COM for all internal subsystems (device management, capture, profiles, error handling). Seven custom CLSIDs registered. Heavy use of `CComObject`, `CComAggObject`, and `CComContainedObject` patterns for object lifecycle.

3. **DirectShow filter graph architecture:** The capture pipeline is built entirely on DirectShow with 20+ custom filters. The graph is assembled dynamically based on the selected device type (DV, HDV, USB, 1394) and includes source, decoder, splitter, tee, queue, mux, and renderer filters.

4. **DV camcorder focus:** Extensive DV-specific code: `CDVState` for tape transport control, multiple DV splitter variants, DV decoder filters with shot detection, DV info extraction. The UI includes transport controls (play, pause, rewind, FF, stop, frame stepping).

5. **Dual import modes:** Auto-record (cue tape and let it play) vs. manual record (user controls camera). Each mode has its own wizard page with separate capture callback implementations and queue UI.

6. **WMV profile-driven encoding:** Uses Windows Media profiles (XML format) for encoding configuration. Ships with predefined profiles: Recommended, Fit-to-Disk, NTSC/PAL high-quality, and a generic "Capture Wizard Profile". Profiles define video bitrate, audio format, and stream priorities.

7. **Resource satellite DLL pattern:** UI resources (dialogs, strings, bitmaps, icons) are separated into `WLXVideoAcquireWizardResources.dll` loaded via `LoadLibraryEx` + `FindResource`. This enables MUI (Multilingual User Interface) localization without recompiling the main EXE. The DLL has a MUI resource entry (type 241, ID 302).

8. **HDV support via MPEG-2:** HDV cameras use MPEG-2 transport streams. The wizard includes MPEG-2 demux, decoder, and stream analyzer filters. HD capture settings are stored under `MM20\Video Capture Wizard\HD Capture Settings`.

9. **Disk space and file size awareness:** `CDiskSpaceMonitor` class, FAT32 4GB limit handling, network drive detection ("cannot save directly to a network drive"), and disk space warnings are built into the import flow.

10. **Scene detection:** `CSceneDetectContentFilter` and `CSceneDetectDVFilter` provide automatic scene break detection during import, with configurable minimum split offset. This enables automatic clip creation from continuous tape imports.

11. **Video preview pipeline:** Separate preview and capture paths in the DirectShow graph using smart tee filters. Preview goes to `CVideoPreviewWindow` (with border) using VMR-9, with optional full-screen mode. 16:9 aspect ratio preview is configurable.

12. **Multiple video codec paths:** The filter classes support DV, MPEG-2, WMV, and AVI output formats. `CAVIMUXFilter`/`CAVIWriterFilter` handle AVI output; Stream Buffer Engine handles DVR-MS; WMV profiles handle WMV output.

## Conditional Compilation / Feature Flags

- `OMD-API-Enabled` — OM Document API enabled
- `CaptureWizard-HiDef` — HD video camera support
- `-QueueRequests -module:tou` — Queue request mode for TOU (Terms of Use) module
- `-QueueRequests -firstrun` — First-run queue mode
- `/AppRestart` — Application restart support
- `/AutoPlay` — AutoPlay launch mode
- `dvdbrn.xml` — DVD burn wizard configuration file
