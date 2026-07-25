# WLXFaceRecognition.dll Analysis

## Overview
Face detection and recognition COM DLL from Windows Live Movie Maker 2012 / Photo Gallery. PE32 x86, linker v11.0 (MSVC 2012), image base 0x10000000, ASLR+NX. Timestamp: 2014-04-01 01:27:18 (PE header) / 01:15:34 (export timestamp). PDB: `{2FEBA5E0-6376-46CD-9ADA-4A79335C37BD}`, 1, WLXFaceRecognition.pdb. Total image: 4,726,784 bytes (~4.5 MB).

**Origin**: Microsoft Research Asia (MSRA) — Registry path `Software\Microsoft\MSRA\FaceRecognition\` confirms this technology was developed by MSRA. Used in Windows Live Photo Gallery for face tagging.

## PE Structure

| Section | VA | VirtSize | RawSize | Flags |
|---------|------|----------|---------|-------|
| `.text` | 0x1000 | 27CBF (163 KB) | 27E00 (163 KB) | Code, Execute Read |
| `.data` | 0x29000 | 3A68 (14.9 KB) | 1600 (5.5 KB) | Init Data, Read Write |
| `.rsrc` | 0x2D000 | 466CB0 (4.4 MB) | 466E00 (4.4 MB) | Init Data, Read Only |
| `.reloc` | 0x494000 | 5D0E (23.3 KB) | 5E00 (23.5 KB) | Init Data, Discardable, Read Only |

**Key observation**: The `.rsrc` section is ~4.4 MB — the vast majority of the DLL. This contains cascade classifier data, neural network weights, and trained models (not code).

## Exports (4 functions — standard COM DLL pattern)

| Ordinal | Name | RVA | Purpose |
|---------|------|-----|---------|
| 1 | `DllCanUnloadNow` | 0x51FC | COM unload check |
| 2 | `DllGetClassObject` | 0x5215 | COM class factory |
| 3 | `DllRegisterServer` | 0x5225 | COM self-registration |
| 4 | `DllUnregisterServer` | 0x5241 | COM unregistration |

Standard ATL COM DLL exports. No face detection/recognition functions are exported directly — all access is via COM interfaces.

## Import Table

### CRT / Runtime
- **MSVCR110.dll** (43 functions): CRT core — memory (malloc/free/calloc/realloc/aligned), SSE2 math (cos/sin/exp/log/pow/sqrt), string (memcpy/memset/memmove/wcscat/wcscpy/wcsstr/swscanf), exceptions (CxxThrowException, CxxFrameHandler3, terminate), file I/O (fopen/fclose/fread), qsort, locale (iswalnum)
- **MSVCP110.dll** (6 functions): C++ stdlib — `_Syserror_map`, `_Winerror_map`, `_Xbad_alloc`, `_Xlength_error`, `_Xout_of_range`, `_Orphan_all`

### OS
- **KERNEL32.dll** (32 functions): File I/O (CreateFileW/ReadFile/GetFileSizeEx), module loading (LoadLibraryExW/FreeLibrary/GetModuleHandle/GetProcAddress), resources (FindResourceW/LoadResource/LockResource/SizeofResource), sync (InitializeCriticalSection/Enter/Leave/Delete), threading (GetCurrentThreadId/GetCurrentProcessId), perf counters (QueryPerformanceCounter/Frequency), encoding (MultiByteToWideChar), ASLR (EncodePointer/DecodePointer), time (GetSystemTime/GetTickCount64)
- **USER32.dll** (4 functions): `CopyRect`, `SetRectEmpty`, `CharNextW`, `EqualRect` — minimal UI (rect handling for face regions)
- **ADVAPI32.dll** (14 functions): Registry (RegCreateKeyExW/RegOpenKeyExW/RegQueryValueExW/RegSetValueExW/RegEnumKeyExW/RegDeleteKeyW/etc.), ETW tracing (RegisterTraceGuidsW/TraceEvent/GetTraceEnableLevel/GetTraceEnableFlags/GetTraceLoggerHandle/UnregisterTraceGuids)

### COM
- **ole32.dll** (5 functions): `CoCreateInstance`, `CoTaskMemAlloc/Free/Realloc`, `StringFromGUID2`
- **OLEAUT32.dll** (10 ordinal-only imports): Ordinals 2,4,6,7,149,150,161,163,186,277 — COM marshaling/Unicode helpers (likely SysAllocString/SysFreeString/Variant* functions)

### Windows Live DLLs
- **WLXPhotoBase.dll** (6 functions): `Base::Throw`, `Base::ThrowLastError`, `Base::Exception::~Exception`, `BasePrivate::Delete`, `BasePrivate::New`, `ATL::BaseAtlThrow` — memory management and exception handling infrastructure
- **WLXPhotoSqm.dll** (4 functions): `Sqm::AddToStream` (3 overloads: DWORD+DWORD, timer+tuple, wide string), `Sqm::AddToStreamTimer` — SQM telemetry for face detection usage metrics

### Imaging
- **WindowsCodecs.dll** (1 function): `WICConvertBitmapSource` — WIC bitmap format conversion for face image processing

## COM Architecture

### ATL Module
Standard ATL COM DLL with `CAtlDllModuleT<...>` pattern. String references show:
- `AVCAtlModule`, `AVCAtlException`, `AVCComModule`, `AVCComClassFactory`, `AVCRegObject`
- `AVCComObjectRootBase`, `CComObject`, `CComObjectCached`, `CComObjectRootEx`
- `CSingletonWrapper`, `CNonGlobalClassFactorySingleton`, `AUIClassFactory`
- `VCComModule`, `VCComMultiThreadModel`

### COM Classes (7 COM-servable classes via ATL OBJECT_MAP)

| ATL Class | CLSID Constant | Interface | Purpose |
|-----------|---------------|-----------|---------|
| `VFaceDetection` | `CLSID_FaceDetection` | `IFaceDetection` | Face detection in images |
| `VFaceRecognitionPipeline` | `CLSID_FaceRecognitionPipeline` | `IFaceRecognitionPipeline` | Full recognition pipeline |
| `VFaceRegion` | `CLSID_FaceRegion` | `IFaceRegion` | Individual face region |
| `VFaceRegionSet` | `CLSID_FaceRegionSet` | `IFaceRegionSet` | Collection of face regions |
| `VFaceRepImpl` | `CLSID_FaceRepImpl` | `IFaceRepresentation` | Face feature representation |
| `VImageData` | `CLSID_ImageData` | `IImageData` | Image data wrapper |
| `VImageManager` | `CLSID_ImageManager` | `IImageManager` | Image loading/management |

### COM GUIDs Found in Binary

| GUID | Likely Purpose |
|------|---------------|
| `{EF401225-1260-4716-A842-7D180DC14C1E}` | `CLSID_FaceRecognitionPipeline` (confirmed in RGS registry script string) |
| `{4107FA03-3FD3-4406-B4F3-68E6D610EC2B}` | Likely `CLSID_FaceDetection` |
| `{483A53CD-EF18-4b19-8AA3-2E2E3214EB41}` | Likely `CLSID_ImageManager` |
| `{D01C34A5-A6DC-4d28-ABBD-78D06EA27B60}` | Likely `CLSID_FaceRegion` or `CLSID_FaceRegionSet` |

### Type Library
- `FaceRecognition.tlb` — embedded COM type library
- `FaceRecognitionObjects` — coclass group name
- `resource.h` — resource header reference

### Registry Script (RGS) Strings
- Registry path: `Software\Microsoft\MSRA\FaceRecognition\`
- CLSID registration entries for `FaceDetection`, `ImageManager`, `FaceRecognitionPipeline`
- `InprocServer32`, `ThreadingModel`, `ForceRemove`, `NoRemove` — standard ATL RGS keywords

## Face Detection / Recognition Pipeline Architecture

### Algorithm Pipeline String Hierarchy
From RTTI/mangled names, the recognition pipeline is a multistage process:

1. **Detection** → 2. **Parts Localization** → 3. **Geometry Rectification** → 4. **Feature Extraction** → 5. **Distance Matching** → 6. **Grouping** → 7. **Recognition**

### 1. Face Detection
| Class | Description |
|-------|-------------|
| `AVFaceDetector` (interface: `AVIFaceDetector`) | Top-level face detector |
| `AVFaceDetection`, `AVFaceDetectionBase` | Base detection implementations |
| `AVFaceDetectionSoftCascade` | Soft cascade classifier (AdaBoost-derived) |
| `AVClassifier`, `AVClassifierBase` | Generic classifier base |
| `AVPoly2Classifier` | 2nd-degree polynomial classifier |
| `AVCNeuralNet` | Neural network (for parts detection) |

**Cascade/model files loaded** (in `.rsrc` section as embedded resources):
- `DetectNeuralNet.bin` — neural network binary for detection
- `background.bf` — background model file (boosting framework)
- `FaceRecognition.rcv` — RCV cascade/classifier model

**Face poses detected** (with CW30 = CCW 30-degree rotations):
- `FacePoseFrontal`, `FacePoseFrontal_CCW30` — frontal faces (0°, ±30°)
- `FacePoseLeftProfile`, `FacePoseLeftProfile_CCW30` — left profile (−90°, −120°)
- `FacePoseLeftHalfProfile`, `FacePoseLeftHalfProfile_CCW30` — left half-profile
- `FacePoseRightProfile`, `FacePoseRightProfile_CCW30` — right profile (+90°, +60°)
- `FacePoseRightHalfProfile`, `FacePoseRightHalfProfile_CCW30` — right half-profile
- `FacePoseNone` — unknown/unclassified pose

The system detects faces across 11 pose categories (5 main poses × 2 rotations + None).

### 2. Face Parts Detection
| Class | Description |
|-------|-------------|
| `AVFacePartsDetectionBase` | Parts detector base |
| `AVFacePartsDetectionNeuralNet` | Neural-network based eye/nose/mouth localization |

### 3. Geometric Rectification
| Class | Description |
|-------|-------------|
| `AVFaceGeomRectBase` | Geometric rectification base |
| `AVFaceGeomRectEyes` | Eye-alignment rectification |
| `AVFaceGeomRectRPTexton` | RP Texton-based rectification |
| `AVFacePhotoRectBase` | Photo-quality rectification base |
| `AVFacePhotoRectLBP` | LBP-based photo rectification |
| `AVFacePhotoRectRawPCA` | Raw PCA photo rectification |

### 4. Feature Extraction
| Class | Description |
|-------|-------------|
| `AVFaceFeatureExtractorBase` | Feature extractor base |
| `AVFaceFeatureExtractorLBP` | **Local Binary Patterns** (LBP) — texture descriptor |
| `AVFaceFeatureExtractorRawPCA` | Raw pixel PCA features |
| `AVFaceFeatureExtractorTexton` | Texton features (texture classification) |
| `AVFaceFeatureExtractorRPTexton` | Random Projection + Texton features |
| `AVFaceFeatureExtractorRPTextonPCA` | RP Texton → PCA reduced features |

### 5. Distance / Matching
| Class | Description |
|-------|-------------|
| `AVFaceDistanceBase` | Distance metric base |
| `AVFaceDistanceLBP` | LBP histogram distance |
| `AVFaceDistanceRawPCA` | PCA Euclidean distance |
| `AVFaceDistanceRPTextonPCA` | RP Texton PCA distance |
| `AVCFaceRecognizor` (interface: `AVIFaceRecognizor`) | Recognizer interface |
| `AVFaceMatchLookupBase` | Match lookup base |
| `AVFaceMatchLookupIteration` | Iterative match refinement |
| `AVFaceMatchLookupLBP` | LBP-based match lookup |

### 6. Face Representation
| Class | Description |
|-------|-------------|
| `AVCFaceRepresentor` (interface: `AVIFaceRepresentor`) | Representation creator |
| `AVFaceRepresentBase` | Representation base |
| `AVFaceRepImpl` | Representation implementation (COM object) |

### 7. Face Grouping
| Class | Description |
|-------|-------------|
| `AVFaceGroupBase` | Grouping base |
| `AVFaceGroupRankOrder` | Rank-order clustering for grouping same-person faces |

### Match Thresholds
String constants suggest configurable matching strictness:
- `MatchThresholdDefault`
- `MatchThresholdLoose`
- `MatchThresholdMax`
- `MatchThresholdModerate`
- `MatchThresholdStrict`

## COM Interface Methods (from method name strings)

### IFaceDetection (`IFaceDetection`, `VFaceDetection`)
- `RunFaceDetection(ImageData, ...)` → returns `FaceRegionSet`
- `FaceDetection(ImageData, ...)` variants
- `InitializeCustom(...)` — custom model initialization

### IFaceRegionSet (`IFaceRegionSet`, `VFaceRegionSet`)
- `GetRegion(index)` → `IFaceRegion`
- `SetRegion(index, region)`
- `GetCount()` → `uNumRegions`

### IFaceRegion (`IFaceRegion`, `VFaceRegion`)
- `GetFacePose()` → `FacePose` enum
- `SetFacePose(pose)`
- `GetFaceRegionRect()` → rect
- `SetFaceRegionRect(rect)`
- `GetPersonId()` / `SetPersonId(id)`
- `GetPersonName()` / `SetPersonName(name)` — person tagging
- `GetPersonData()` / `GetPersonCount()`
- `GetConfidence()` → `flConfidence`
- `GetRepresentation()` → `IFaceRepresentation`
- `SetRepresentation(rep)`
- `GetRecommendations()` / `SetRecommendations(recommendations)`
- `GetThumbnailImageData()` / `SetThumbnailImageData(imageData)` — thumbnail for display
- `CreateFaceRepresentations()` / `CreateFaceRepresentationSet()`

### IFaceRecognitionPipeline (`IFaceRecognitionPipeline`, `VFaceRecognitionPipeline`)
- `RecognizeFacesFromImage(ImageData, ...)` — full pipeline: detect → extract → recognize
- `RecognizeFacesFromFaceRegions(ImageData, regionSet, ...)` — recognize from existing regions
- `RecognizeFacesFromRepresentations(representations, ...)` — match existing representations
- `GroupFaces(representations, ...)` → group IDs — rank-order clustering
- `SetMatchThreshold(threshold)` — configure recognition strictness
- `GetCacheStamp()` / `SetImageId()` — caching for performance
- `GetExemplarCache()` — exemplar/template caching

### IFaceRepresentation (`IFaceRepresentation`, `VFaceRepImpl`)
- `GetRepresentation(buffer, size)` — raw feature vector
- `SetRepresentation(buffer, size)`
- `GetPersonId()` / `SetPersonId(id)`
- `GetPersonName()` / `SetPersonName(name)`

### IImageData (`IImageData`, `VImageData`)
- Image data wrapper loading from various sources:
  - `CreateImageDataFromHBITMAP(HBITMAP)` — from GDI bitmap
  - `CreateImageDataFromWICBitmap(IWICBitmap)` — from WIC
  - `CreateImageDataFromBuffer(buffer, size, format)` — from raw buffer
- `GetPixelFormat()`, `GetResolution()`, `GetStride()`
- `GetDataPointer()`, `GetStride()`
- `IsGrayscale()`, `IsBlackWhite()`
- `WICConvertBitmapSource()` delegate

### IImageManager (`IImageManager`, `VImageManager`)
- `ImageManager` — factory/manager for loading images
- `LoadResource()` / `FindResource()` — loading from module resources

### IMatchLookup / IExemplarCache
- `ExemplarCache` — caches known face representations for fast matching
- `GetCacheStamp()` — cache versioning

## Face Photo Rectification Pipeline
The `AVFacePhotoRect*` classes handle aligning faces to canonical pose before recognition:
1. Detect face region
2. Locate eyes (via neural net)
3. Rectify geometry (rotate/scale to standard position)
4. Extract features from rectified face

## SQM Telemetry
Tracks face detection usage and performance metrics via `WLXPhotoSqm.dll`:
- `Sqm::AddToStream(DWORD, DWORD)` — simple counter
- `Sqm::AddToStream(DWORD, wide_string)` — named string metric
- `Sqm::AddToStreamTimer(DWORD, DWORD, Tuple)` — timing metrics
- `Sqm::AddToStreamTimer(DWORD, DWORD, Tuple)` — timed events

## Registry Access
- `Software\Microsoft\MSRA\FaceRecognition\` — configuration/model paths
- `UseIterative` — flag for iterative matching mode

## Key Strings and Resources
- `cascade resource` — loaded cascades from embedded resources
- `background.bf` — background/boosting model file
- `FaceRecognition.rcv` — RCV classifier model
- `DetectNeuralNet.bin` — neural network detection model
- `Poly2Classifier` — polynomial kernel classifier
- `89_FaceRecognitionPipeline` — pipeline version/identifier string
- `8hppFaceRepresentations` — face representation storage
- `$Windows Essential`, `Legal_policy_statement` — not present (unlike WLXPhotoBase)

## Dependencies Summary
| DLL | Functions | Role |
|-----|-----------|------|
| MSVCR110.dll | 43 | CRT core |
| MSVCP110.dll | 6 | C++ stdlib |
| KERNEL32.dll | 32 | OS kernel |
| USER32.dll | 4 | Rect utilities |
| ADVAPI32.dll | 14 | Registry + ETW |
| ole32.dll | 5 | COM core |
| OLEAUT32.dll | 10 ordinals | COM automation |
| WindowsCodecs.dll | 1 | WIC bitmap conversion |
| WLXPhotoBase.dll | 6 | Memory/exception base |
| WLXPhotoSqm.dll | 4 | SQM telemetry |

## Architecture Summary
WLXFaceRecognition.dll is a complete face detection and recognition engine from Microsoft Research Asia (MSRA), packaged as a COM DLL for use in Windows Live Photo Gallery 2012. It implements a multistage pipeline with 11 pose detectors, neural network eye localization, multiple feature extractors (LBP, PCA, Texton, RPTexton), and rank-order clustering for face grouping. The majority of the DLL (4.4 MB of 4.5 MB) is classifier cascade data embedded in the resource section. The engine uses ATL COM for interop, SQM telemetry for tracking, and registry-based configuration under the MSRA path.
