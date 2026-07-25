# WLXFaceRecognition.dll - Dynamic/Static Analysis Deep Dive

## Build Info
- **Timestamp**: 2014-04-01 01:27:18 (PE) / 01:15:34 (export)
- **PDB**: `{2FEBA5E0-6376-46CD-9ADA-4A79335C37BD}` → WLXFaceRecognition.pdb
- **Linker**: MSVC 11.0 (VS2012)
- **Code size**: 163 KB (.text) — relatively small codebase
- **Data size**: 14.9 KB (.data)
- **Resource size**: 4.4 MB (.rsrc) — cascade classifiers, neural net weights
- **Image**: 4,726,784 bytes (~4.5 MB)

## PE Headers
- **Machine**: x86 (0x14C)
- **Subsystem**: Windows GUI (2)
- **Characteristics**: Dynamic base (ASLR), NX compatible, 32-bit word machine
- **Image Base**: 0x10000000
- **Section Alignment**: 0x1000
- **Entry Point**: 0x23371 (DllMain CRT startup)
- **Stack Reserve**: 0x40000 (256 KB)
- **Heap Reserve**: 0x100000 (1 MB)
- **Security Guard**: /GUARD:CF enabled
- **Debug GUID**: `{2FEBA5E0-6376-46CD-9ADA-4A79335C37BD}`

## Import Table (12 DLLs, ~120 functions)

### MSVCR110.dll (43 functions) — Core CRT
| Category | Functions |
|----------|-----------|
| **Memory** | `malloc`, `free`, `calloc`, `realloc`, `_recalloc`, `_aligned_malloc`, `_aligned_free`, `memset`, `memcpy`, `memcpy_s`, `memmove`, `memmove_s` |
| **SSE2 Math** | `_libm_sse2_cos_precise`, `_libm_sse2_sin_precise`, `_libm_sse2_exp_precise`, `_libm_sse2_log_precise`, `_libm_sse2_pow_precise`, `_libm_sse2_sqrt_precise`, `floor`, `ceil` |
| **String** | `wcscat_s`, `wcscpy_s`, `wcsncpy_s`, `wcsstr`, `strncpy_s`, `swscanf_s`, `iswalnum` |
| **File I/O** | `fopen_s`, `_wfopen_s`, `fclose`, `fread` |
| **C++ RTTI** | `__RTDynamicCast`, `__CxxFrameHandler3`, `_CxxThrowException` |
| **Exception** | `_except_handler4_common`, `??1exception@std@@UAE@XZ`, `??0exception@std@@QAE@ABQBD@Z`, `??0exception@std@@QAE@ABV01@@Z`, `?what@exception@std@@UBEPBDXZ`, `?terminate@@YAXXZ` |
| **Init** | `_initterm`, `_initterm_e`, `_amsg_exit`, `_calloc_crt`, `_malloc_crt` |
| **Misc** | `qsort`, `_errno`, `__iob_func`, `_onexit`, `__dllonexit`, `??1type_info@@UAE@XZ` |

### MSVCP110.dll (6 functions) — C++ Standard Library
- `_Syserror_map`, `_Winerror_map`, `_Xbad_alloc`, `_Xlength_error`, `_Xout_of_range`, `_Orphan_all`

### KERNEL32.dll (32 functions) — OS Kernel
| Category | Functions |
|----------|-----------|
| **File** | `CreateFileW`, `ReadFile`, `GetFileSizeEx`, `FindResourceW`, `LoadResource`, `LockResource`, `SizeofResource` |
| **Module** | `LoadLibraryExW`, `FreeLibrary`, `GetProcAddress`, `GetModuleHandleW`, `GetModuleFileNameW` |
| **Thread** | `GetCurrentThreadId`, `GetCurrentProcessId`, `DisableThreadLibraryCalls` |
| **Sync** | `InitializeCriticalSection`, `InitializeCriticalSectionAndSpinCount`, `EnterCriticalSection`, `LeaveCriticalSection`, `DeleteCriticalSection`, `InterlockedIncrement`, `InterlockedDecrement` |
| **Encoding** | `MultiByteToWideChar` |
| **Time** | `GetSystemTime`, `GetSystemTimeAsFileTime`, `SystemTimeToFileTime`, `GetTickCount64` |
| **Misc** | `IsDebuggerPresent`, `IsProcessorFeaturePresent`, `QueryPerformanceCounter`, `QueryPerformanceFrequency`, `RaiseException`, `EncodePointer`, `DecodePointer`, `GetLastError` |

### USER32.dll (4 functions) — Minimal UI
- `CopyRect`, `SetRectEmpty`, `EqualRect`, `CharNextW`
- Used only for face region rectangle manipulation

### ADVAPI32.dll (14 functions) — Registry + ETW
| Category | Functions |
|----------|-----------|
| **Registry** | `RegCreateKeyExW`, `RegOpenKeyExW`, `RegQueryValueExW`, `RegSetValueExW`, `RegQueryInfoKeyW`, `RegEnumKeyExW`, `RegDeleteKeyW`, `RegDeleteValueW`, `RegCloseKey` |
| **ETW Tracing** | `RegisterTraceGuidsW`, `UnregisterTraceGuids`, `TraceEvent`, `GetTraceLoggerHandle`, `GetTraceEnableLevel`, `GetTraceEnableFlags` |

### COM System DLLs
| DLL | Functions |
|-----|-----------|
| **ole32.dll** | `CoCreateInstance`, `CoTaskMemAlloc`, `CoTaskMemFree`, `CoTaskMemRealloc`, `StringFromGUID2` |
| **OLEAUT32.dll** | Ordinals 2,4,6,7,149,150,161,163,186,277 (SysAllocString, SysFreeString, SysStringLen, VariantInit, VariantClear, etc.) |
| **WindowsCodecs.dll** | `WICConvertBitmapSource` |

### Windows Live Dependencies
| DLL | Functions | Purpose |
|-----|-----------|---------|
| **WLXPhotoBase.dll** | `Base::Throw(J)`, `Base::ThrowLastError()`, `Base::Exception::~Exception()`, `BasePrivate::Delete(PAX)`, `BasePrivate::New(UI, bool)`, `ATL::BaseAtlThrow(J)` | Memory management, exception handling |
| **WLXPhotoSqm.dll** | `Sqm::AddToStream(K, DWORD)`, `Sqm::AddToStream(K, wchar_t*)`, `Sqm::AddToStreamTimer(K, K, Tuple*)`, `Sqm::AddToStreamTimer(K, K, K, Tuple*)` | SQM telemetry |

## RTTI Class Hierarchy (100+ classes extracted)

### COM Infrastructure Classes (ATL)
| RTTI Name | Description |
|-----------|-------------|
| `?AV?$CAtlModuleT@VCComModule@ATL@@@ATL` | ATL module base |
| `?AVCComModule@ATL` | COM module implementation |
| `?AVCComClassFactory@ATL` | IClassFactory implementation |
| `?AV?$CComObjectRootEx@VCComMultiThreadModel@ATL@@@ATL` | Multi-threaded COM root |
| `?AVCComObjectRootBase@ATL` | COM object root base |
| `?AV?$CComObjectCached@V?$CNonGlobalClassFactorySingleton@...` | Cached class factory singletons |
| `?AVCRegObject@ATL` | Registry object for self-registration |

### ATL COM Servable Classes (7 objects)
| RTTI Name | CLSID Symbol | Description |
|-----------|-------------|-------------|
| `?AVFaceDetection@@` | `CLSID_FaceDetection` | Face detection engine |
| `?AVFaceRecognitionPipeline@@` | `CLSID_FaceRecognitionPipeline` | Full recognition pipeline |
| `?AVFaceRegionSet@@` | `CLSID_FaceRegionSet` | Collection of face regions |
| `?AVFaceRegion@@` | `CLSID_FaceRegion` | Individual face region |
| `?AVFaceRepImpl@@` | `CLSID_FaceRepImpl` | Face representation |
| `?AVImageData@@` | `CLSID_ImageData` | Image data wrapper |
| `?AVImageManager@@` | `CLSID_ImageManager` | Image loader factory |

### COM Interface Classes
| RTTI Name | Interface | Description |
|-----------|-----------|-------------|
| `?AUIFaceDetection@@` | `IFaceDetection` | Detection interface |
| `?AUIFaceRecognitionPipeline@@` | `IFaceRecognitionPipeline` | Pipeline interface |
| `?AUIFaceRegionSet@@` | `IFaceRegionSet` | Region collection interface |
| `?AUIFaceRegion@@` | `IFaceRegion` | Region interface |
| `?AUIFaceRepresentation@@` | `IFaceRepresentation` | Feature representation interface |
| `?AUIImageData@@` | `IImageData` | Image data interface |
| `?AUIImageManager@@` | `IImageManager` | Image manager interface |
| `?AVIFaceDetector@@` | `IFaceDetector` | Internal detector interface |
| `?AVIFaceRecognizor@@` | `IFaceRecognizor` | Internal recognizer interface |
| `?AVIFaceRepresentor@@` | `IFaceRepresentor` | Internal representor interface |

### ML Pipeline Classes
| RTTI Name | Category | Description |
|-----------|----------|-------------|
| **Detection** | | |
| `?AVFaceDetectionBase@@` | Detection | Base detection |
| `?AVFaceDetectionSoftCascade@@` | Detection | Soft cascade classifier (AdaBoost) |
| **Parts** | | |
| `?AVFacePartsDetectionBase@@` | Parts | Base parts detector |
| `?AVFacePartsDetectionNeuralNet@@` | Parts | Neural net eye/nose/mouth detector |
| **Geometry** | | |
| `?AVFaceGeomRectBase@@` | Geometry | Geometric rectification base |
| `?AVFaceGeomRectEyes@@` | Geometry | Eye-alignment rectification |
| `?AVFaceGeomRectRPTexton@@` | Geometry | RP Texton rectification |
| **Photo** | | |
| `?AVFacePhotoRectBase@@` | Photo | Photo rectification base |
| `?AVFacePhotoRectLBP@@` | Photo | LBP-based photo rectification |
| `?AVFacePhotoRectRawPCA@@` | Photo | Raw PCA photo rectification |
| **Feature Extraction** | | |
| `?AVFaceFeatureExtractorBase@@` | Features | Base feature extractor |
| `?AVFaceFeatureExtractorLBP@@` | Features | Local Binary Patterns |
| `?AVFaceFeatureExtractorRawPCA@@` | Features | Raw pixel PCA |
| `?AVFaceFeatureExtractorTexton@@` | Features | Texton features |
| `?AVFaceFeatureExtractorRPTexton@@` | Features | Random Projection + Texton |
| `?AVFaceFeatureExtractorRPTextonPCA@@` | Features | RP Texton → PCA reduced |
| **Distance/Matching** | | |
| `?AVFaceDistanceBase@@` | Distance | Base distance metric |
| `?AVFaceDistanceLBP@@` | Distance | LBP histogram distance |
| `?AVFaceDistanceRawPCA@@` | Distance | PCA Euclidean distance |
| `?AVFaceDistanceRPTextonPCA@@` | Distance | RP Texton PCA distance |
| `?AVFaceMatchLookupBase@@` | Matching | Base match lookup |
| `?AVFaceMatchLookupIteration@@` | Matching | Iterative match refinement |
| `?AVFaceMatchLookupLBP@@` | Matching | LBP-based match lookup |
| **Representation** | | |
| `?AVFaceRepresentBase@@` | Represent | Base representation |
| `?AVFaceRepresentLBP@@` | Represent | LBP representation |
| `?AVFaceGroupBase@@` | Grouping | Base grouping |
| `?AVFaceGroupRankOrder@@` | Grouping | Rank-order clustering |
| **Core** | | |
| `?AVCFaceDetector@@` | Core | Internal detector class |
| `?AVCFaceRecognizor@@` | Core | Internal recognizer class |
| `?AVCFaceRepresentor@@` | Core | Internal representor class |

### LiveLabs ML Engine Classes
| RTTI Name | Description |
|-----------|-------------|
| `?AVClassifier@LiveLabs@@` | LiveLabs classifier |
| `?AVClassifierBase@LiveLabs@@` | LiveLabs classifier base |
| `?AVCNeuralNet@LiveLabs@@` | LiveLabs neural network |
| `?AVCErrorBase@vt@@` | LiveLabs error base |

### Image/Utility Classes
| RTTI Name | Description |
|-----------|-------------|
| `?AVCImg@vt@@` | Image object (vt namespace) |
| `?AV?$CTypedImg@vt@@` | Typed image template |
| `?AVCObj@vt@@` | Generic vt object |
| `?AVCMem@vt@@` | Memory manager (vt namespace) |

### MSVC STL Classes
| RTTI Name | Description |
|-----------|-------------|
| `?AV_Iostream_error_category@std@@` | iostream error category |
| `?AV_System_error_category@std@@` | system error category |
| `?AVerror_category@std@@` | base error category |
| `?AV_Generic_error_category@std@@` | generic error category |
| `?AVlogic_error@std@@` | logic_error exception |
| `?AVexception@std@@` | base exception |

## CLSIDs (7 COM Classes)

| Symbol | GUID | Confirmed Via |
|--------|------|---------------|
| `CLSID_FaceDetection` | `{4107FA03-3FD3-4406-B4F3-68E6D610EC2B}` | RGS + **dynamic test S_OK** |
| `CLSID_FaceRecognitionPipeline` | `{D01C34A5-A6DC-4d28-ABBD-78D06EA27B60}` | RGS + **dynamic test S_OK** |
| `CLSID_ImageManager` | `{483A53CD-EF18-4b19-8AA3-2E2E3214EB41}` | RGS + **dynamic test S_OK** |
| `CLSID_FaceRegionSet` | `{D01C34A5...}` or separate | RTTI only |
| `CLSID_FaceRegion` | Unknown | RTTI only |
| `CLSID_FaceRepImpl` | Unknown | RTTI only |
| `CLSID_ImageData` | Unknown | RTTI only |

**Note**: `{EF401225-1260-4716-A842-7D180DC14C1E}` appears in RGS strings but returns CLASS_E_CLASSNOTAVAILABLE — it is NOT a servable CLSID via DllGetClassObject.

## COM Interface IID

| Interface | IID |
|-----------|-----|
| `IFaceDetection` (main) | `{E1ED7C08-0549-4FC5-B61D-3A3AB625F97C}` |

## Registry Configuration

### Self-Registration Paths (RGS scripts embedded in .rsrc)
```
HKEY_CLASSES_ROOT\CLSID\{483A53CD-EF18-4b19-8AA3-2E2E3214EB41}
    = "Windows Live Photo Gallery Face Recognition ImageManager Class"
    InprocServer32 = "%MODULE%"
    ThreadingModel = "both"

HKEY_CLASSES_ROOT\CLSID\{4107FA03-3FD3-4406-B4F3-68E6D610EC2B}
    = "Windows Live Photo Gallery Face Recognition FaceDetection Class"
    InprocServer32 = "%MODULE%"
    ThreadingModel = "both"

HKEY_CLASSES_ROOT\CLSID\{D01C34A5-A6DC-4d28-ABBD-78D06EA27B60}
    = "Windows Live Photo Gallery Face Recognition FaceRecognitionPipeline Class"
    InprocServer32 = "%MODULE%"
    ThreadingModel = "both"
```

### Runtime Configuration Path
```
HKCU\Software\Microsoft\MSRA\FaceRecognition\
```
- `UseIterative` — enables iterative matching mode

## Face Detection Pipeline Architecture

### Algorithm Flow (7 stages)
```
Input Image
    │
    ▼
┌─────────────────────────┐
│ 1. Face Detection       │  SoftCascade / NeuralNet
│    (11 pose classes)    │  → Returns face regions + confidence
└──────────┬──────────────┘
           │
           ▼
┌─────────────────────────┐
│ 2. Parts Localization   │  NeuralNet eye/nose/mouth detection
│                         │  → Returns landmark positions
└──────────┬──────────────┘
           │
           ▼
┌─────────────────────────┐
│ 3. Geometry Rect.       │  Eye-alignment / RPTexton
│                         │  → Rotates/scales to canonical pose
└──────────┬──────────────┘
           │
           ▼
┌─────────────────────────┐
│ 4. Feature Extraction   │  LBP / PCA / Texton / RPTexton
│                         │  → Returns feature vector
└──────────┬──────────────┘
           │
           ▼
┌─────────────────────────┐
│ 5. Distance Matching    │  LBP histogram / PCA Euclidean
│                         │  → Returns match scores
└──────────┬──────────────┘
           │
           ▼
┌─────────────────────────┐
│ 6. Match Lookup         │  Iterative / LBP lookup
│                         │  → Refines matches
└──────────┬──────────────┘
           │
           ▼
┌─────────────────────────┐
│ 7. Face Grouping        │  Rank-order clustering
│                         │  → Groups same-person faces
└──────────┬──────────────┘
           │
           ▼
        Output
   (recognition results)
```

### Face Pose Categories (11 classes)
| ID | Name | Description |
|----|------|-------------|
| 0 | `FacePoseFrontal` | Front-facing (0°) |
| 1 | `FacePoseFrontal_CCW30` | Front-facing, rotated 30° CCW |
| 2 | `FacePoseLeftProfile` | Left profile (-90°) |
| 3 | `FacePoseLeftProfile_CCW30` | Left profile rotated 30° CCW |
| 4 | `FacePoseLeftHalfProfile` | Left half-profile (-45°) |
| 5 | `FacePoseLeftHalfProfile_CCW30` | Left half rotated 30° CCW |
| 6 | `FacePoseRightProfile` | Right profile (+90°) |
| 7 | `FacePoseRightProfile_CCW30` | Right profile rotated 30° CCW |
| 8 | `FacePoseRightHalfProfile` | Right half-profile (+45°) |
| 9 | `FacePoseRightHalfProfile_CCW30` | Right half rotated 30° CCW |
| 10 | `FacePoseNone` | Unknown/unclassified |

### Match Threshold Settings
| ID | Name | Effect |
|----|------|--------|
| 0 | `MatchThresholdDefault` | Balanced |
| 1 | `MatchThresholdLoose` | More matches, more false positives |
| 2 | `MatchThresholdModerate` | Moderate |
| 3 | `MatchThresholdStrict` | Fewer matches, fewer false positives |
| 4 | `MatchThresholdMax` | Strictest matching |

### Embedded Model Files (in .rsrc section)
| File | Purpose |
|------|---------|
| `DetectNeuralNet.bin` | Neural network for face detection |
| `background.bf` | Background/boosting model file |
| `FaceRecognition.rcv` | RCV cascade/classifier model |
| `FaceRecognition.tlb` | Embedded COM type library |
| `cascade resource` | Loaded cascades from resources |
| `ILabsPostFilter` | LiveLabs post-filter |

## Error Messages (extracted from binary)
| String | Context |
|--------|---------|
| `unknown error` | Generic error |
| `iostream stream error` | Stream I/O error |
| `invalid scale for pre computing offsets` | Cascade configuration |
| `invalid offsets for pre computing` | Cascade configuration |
| `Can't allocate face image` | Memory allocation failure |
| `Poly2Classifier load from buffer error` | Model loading failure |
| `Invalid length of X (does not match number of features in model)` | Feature/model mismatch |
| `no rects in a merge rect list` | Empty merge results |
| `insufficient buffer size for grouped regions` | Buffer too small |
| `Classifier categories don't have the same classifier count` | Configuration error |
| `invalid classifier array size` | Array validation |
| `invalid feature array size` | Array validation |
| `invalid scaled feature buffer size or offset` | Buffer validation |
| `cannot find cascade resource` | Missing resource |
| `invalid cascade resource` | Corrupt resource |
| `invalid buffer or position` | Buffer overflow check |
| `buffer overflow!` | Buffer safety |
| `cannot find ILabsPostFilter resource` | Missing LiveLabs resource |
| `invalid ILabsPostFilter resource` | Corrupt LiveLabs resource |
| `Too many rects for feature!` | Feature detection limit |

## SQM Telemetry
Tracks face detection usage via WLXPhotoSqm.dll:
- `Sqm::AddToStream(DWORD counterId, DWORD value)` — simple metric
- `Sqm::AddToStream(DWORD counterId, wchar_t* value)` — string metric
- `Sqm::AddToStreamTimer(DWORD, DWORD, Tuple)` — timing metrics
- `Sqm::AddToStreamTimer(DWORD, DWORD, DWORD, Tuple)` — timed events

## Resource Section (.rsrc) Analysis
The .rsrc section (4.4 MB, 93% of the DLL) contains:
- **Type Library**: `FaceRecognition.tlb` — COM type library for late binding
- **Registry Scripts**: RGS files for 3 confirmed CLSIDs
- **Classifier Data**: Cascade classifier weights and thresholds
- **Neural Network Weights**: Detection and parts localization models
- **String Tables**: Interface parameter names, error messages
- **Version Info**: File version, product version

## Thread Safety Notes
- Uses `CComMultiThreadModel` — thread-safe COM apartment
- `DisableThreadLibraryCalls` called in DllMain
- `InterlockedIncrement`/`InterlockedDecrement` for module refcount
- `InitializeCriticalSectionAndSpinCount` for internal locking
- Face detection NOT reentrant per object instance

## Test Harness Results

### Live Test Output (2026-07-25)
```
=== Results: 31 passed, 1 failed, 32 total ===
```

### Key Findings from Dynamic Testing

#### CLSID Corrections (Critical)

| CLSID | Expected Name | Actual Result | Correct Name |
|-------|---------------|---------------|--------------|
| `{4107FA03-3FD3-4406-B4F3-68E6D610EC2B}` | FaceDetection | **S_OK** ✓ | **FaceDetection** (confirmed) |
| `{483A53CD-EF18-4b19-8AA3-2E2E3214EB41}` | ImageManager | **S_OK** ✓ | **ImageManager** (confirmed) |
| `{D01C34A5-A6DC-4d28-ABBD-78D06EA27B60}` | FaceRecognitionPipeline | **S_OK** ✓ | **FaceRecognitionPipeline** (confirmed) |
| `{EF401225-1260-4716-A842-7D180DC14C1E}` | FaceRecognitionPipeline | **CLASS_E_CLASSNOTAVAILABLE** ✗ | **Unknown** — NOT a servable CLSID via DllGetClassObject |

**Correction**: The RGS string `{EF401225...}` was previously mapped to FaceRecognitionPipeline, but dynamic testing proves **{D01C34A5}** is the real FaceRecognitionPipeline CLSID. The {EF401225} GUID likely appears in RGS as a different object or type library reference.

#### DllGetClassObject Detailed Results

| CLSID | HRESULT | Factory | CreateInstance | Notes |
|-------|---------|---------|----------------|-------|
| `{4107FA03}` FaceDetection | S_OK | ✓ 0x00B5C8F0 | S_OK → IUnknown 0x00B38648 | Fully functional |
| `{EF401225}` (unknown) | 0x80040111 | ✗ | — | CLASS_E_CLASSNOTAVAILABLE |
| `{483A53CD}` ImageManager | S_OK | ✓ 0x00B5CFB8 | S_OK → IUnknown 0x00B5CF48 | Fully functional |
| `{D01C34A5}` FaceRecognitionPipeline | S_OK | ✓ 0x00B5CF48 | S_OK → IUnknown 0x02E9EC58 | Fully functional |
| GUID_NULL | 0x80040111 | ✗ | — | Correctly rejected |
| Random GUID | 0x80040111 | ✗ | — | Correctly rejected |
| NULL ppv | SEH exception | — | — | Expected crash |

#### CoCreateInstance Results

| CLSID | HRESULT | Notes |
|-------|---------|-------|
| FaceDetection `{4107FA03}` | **S_OK** | Creates successfully |
| FaceRecognitionPipeline `{EF401225}` | **0x80040154** | CLASS_E_CLASSNOTAVAILABLE (not registered in COM) |
| ImageManager `{483A53CD}` | **S_OK** | Creates successfully |
| FaceRecognitionPipeline `{D01C34A5}` | **S_OK** | Creates successfully (this is the real one) |
| Random GUID | **0x80040154** | Correctly rejected |

#### DllCanUnloadNow
- Returns `S_OK` (0x00000000) before any COM objects created — DLL can be unloaded

#### DllRegisterServer / DllUnregisterServer
- DllRegisterServer returns `0x8002801C` (TYPE_E_REGISTRYACCESS) — requires admin rights
- DllUnregisterServer returns `0x80070005` (E_ACCESSDENIED) — requires admin rights
- Both require elevated privileges for HKCR registry writes

#### IFaceDetection Interface Probing
- QueryInterface for `{E1ED7C08-0549-4FC5-B61D-3A3AB625F97C}` returned `0x80004002` (E_NOINTERFACE)
- The IFaceDetection interface may use a different IID or the COM object exposes IUnknown only
- IClassInfo query also returned E_NOINTERFACE

#### FreeLibrary
- Valid handle: succeeds
- NULL handle: returns FALSE (expected)
- Already-freed handle: returns FALSE (expected)

## Source Files
- `tests/WLXFaceRecognition/test_face.cpp` — Test harness (LoadLibrary, COM probing, DllGetClassObject, CoCreateInstance)
- `tests/WLXFaceRecognition/build_and_run.bat` — Build and run script (x86 MSVC)
- `analysis/WLXFaceRecognition/analysis.md` — Comprehensive static analysis
- `analysis/WLXFaceRecognition/harness.md` — COM interface documentation and SDK wrappers
- `analysis/WLXFaceRecognition/dynamic_analysis.md` — This file (dynamic test results)
