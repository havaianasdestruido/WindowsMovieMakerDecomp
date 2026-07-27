# WLXDSPA.dll — Reverse Engineering Analysis

## Overview
| Property | Value |
|----------|-------|
| **File** | `undecomp\Photo Gallery\WLXDSPA.dll` |
| **PDB** | `WLXDSPA.pdb` (GUID `4B8F9B3C-AC0A-42BE-B068-328111777FCE`) |
| **Machine** | x86 (PE32) |
| **Timestamp** | Tue Apr 1 01:28:02 2014 |
| **Subsystem** | Windows CUI (3) |
| **Image Base** | `0x10000000` |
| **Image Size** | `0x32000` (200 KB) |
| **Code Size** | `0x29600` (~166 KB) |
| **Linker** | MSVC 11.00 (VS2012) |
| **DLL Characteristics** | Dynamic base, NX compatible |

## Exports (4) — Standard COM DLL
| Ordinal | Hint | RVA | Name |
|---------|------|-----|------|
| 1 | 0 | `0xDD1F` | **DllCanUnloadNow** |
| 2 | 1 | `0xDD30` | **DllGetClassObject** |
| 3 | 2 | `0xDD8F` | **DllRegisterServer** |
| 4 | 3 | `0xDE4F` | **DllUnregisterServer** |

All exports are standard COM boilerplate. The DLL exposes **11 COM coclasses** (registered via embedded registry script in `.rsrc`).

## DLL Imports (9 libraries)
| Library | Key Functions | Role |
|---------|--------------|------|
| **MSVCR110.dll** | malloc, free, memcpy, `_libm_sse2_sin_precise`, `_libm_sse2_cos_precise`, `_libm_sse2_log_precise`, `_libm_sse2_sqrt_precise` | CRT + **SSE2 math** (sin, cos, log, sqrt) |
| **KERNEL32.dll** | CreateFileW, ReadFile, WriteFile, SetFilePointer, SetEndOfFile, GetFileSize, FindResourceW, LoadResource, LockResource | File I/O, resource loading |
| **USER32.dll** | CharNextW | Minimal UI |
| **ADVAPI32.dll** | RegCreateKeyExW, RegOpenKeyExW, RegQueryInfoKeyW, RegSetValueExW, RegDeleteKeyW, RegDeleteValueW, RegEnumKeyExW | Registry (COM registration) |
| **OLEAUT32.dll** | Ordinals 2,6,7,161,163,186,277 | VARIANT operations, SysAllocString |
| **ole32.dll** | CoCreateInstance, CoTaskMemAlloc/Realloc/Free, PropVariantCopy/Clear, StringFromGUID2 | COM + property storage |
| **msdmo.dll** | **DMORegister**, **DMOUnregister**, **MoInitMediaType**, **MoFreeMediaType**, **MoCopyMediaType** | **DirectX Media Object** registration and media type management |
| **gdiplus.dll** | GdipCreateBitmapFromScan0, GdipSaveImageToFile, GdipGetImageThumbnail, GdipCreatePen1, GdipDrawRectangleI, GdipCloneImage, GdipGetImageGraphicsContext | GDI+ image output / thumbnails |

## Notable: SSE2 Math Intrinsics
WLXDSPA imports four **SSE2 precision math** functions from MSVCR110:
- `_libm_sse2_sin_precise` — Sine
- `_libm_sse2_cos_precise` — Cosine
- `_libm_sse2_log_precise` — Logarithm
- `_libm_sse2_sqrt_precise` — Square root

This confirms the DLL performs **mathematical/analytical computations** (signal analysis, feature extraction).

## Registered COM Classes (from embedded REGSRV32 script)

| CLSID | Name | Purpose |
|-------|------|---------|
| `{BC8CA1B3-B013-4866-9621-825957DF23F3}` | **CWLXTocGeneratorDmo** | DMO that generates Table of Contents from media |
| `{09BC59C2-70DD-45f9-A5B7-DE9F2A5CA34B}` | **CWLXThumbnailGeneratorDmo** | DMO that generates thumbnails |
| `{15CD2459-C14B-457b-B57B-3DBA111B9D09}` | **CClusterDetectorEx** | Cluster detection (scene grouping) |
| `{1D8D19C8-0A33-45a4-9B3E-255B85C363A8}` | **CTocEntry** | Table of Contents entry |
| `{15A4E6E5-A9E5-49cb-AFFC-E822F082D427}` | **CTocEntryList** | List of ToC entries |
| `{C9FF4813-CB5F-4ac6-B003-4D79AE2F43E9}` | **CToc** | Table of Contents object |
| `{CE1D8A09-77EA-4eaa-9619-832A9E5DB447}` | **CTocCollection** | Collection of ToC objects |
| `{75704D6C-09BA-4d19-AFEA-5F21FC08B3DB}` | **CTocParser** | Generic ToC parser |
| `{7F2CE947-4E80-446d-9AE4-17DD9D82A353}` | **CFileIo` | File I/O abstraction |
| `{BF620143-7420-460a-9EEE-178B78D4939D}` | **CAsfTocParser** | ASF (WMV/WMA) ToC parser |
| `{9FAE79C9-BA02-43d9-9382-C7BEF740A596}` | **CAviTocParser** | AVI ToC parser |

All registered as `ThreadingModel = 'Both'` (free-threaded COM).

## RTTI Class Hierarchy (93 classes)

### ATL/COM Infrastructure
- `CAtlModule@ATL@@`, `CComModule@ATL@@`, `CAtlModuleT<CComModule>` — ATL module
- `CRegObject@ATL@@` — Registry object (self-registration)
- `CComClassFactory@ATL@@` — Standard class factory
- `CComObjectRootBase`, `CComObjectRootEx<CComMultiThreadModel>`, `CComObjectRootEx<CComMultiThreadModelNoCS>`

### DMO (DirectX Media Object) Classes
- **`CWLXTocGeneratorDmo`** — Table of Contents generator DMO
  - `CVideoTransformBaseDmo<CWLXTocGeneratorDmo>` — Video transform base
  - `IMediaObjectImpl<CWLXTocGeneratorDmo>` — DMO implementation
  - Interfaces: `IMediaObject`, `IPropertyStore`
- **`CWLXThumbnailGeneratorDmo`** — Thumbnail generator DMO
  - `CVideoTransformBaseDmo<CWLXThumbnailGeneratorDmo>` — Video transform base
  - `IMediaObjectImpl<CWLXThumbnailGeneratorDmo>` — DMO implementation

### Table of Contents (ToC) System
- **`CWlxToc`** — Main ToC object (`IToc`, `IFileClient`)
- **`CWlxTocEntry`** — Individual ToC entry (`ITocEntry`)
- **`CWlxTocEntryList`** — List of entries (`ITocEntryList`)
- **`CWlxTocCollection`** — Collection of ToC objects (`ITocCollection`)

### ToC Parsers
- **`CWlxTocParser`** — Generic Wlx ToC parser (`ITocParser`)
- **`CAsfTocParser`** — ASF (Advanced Streaming Format) parser (`ITocParser`)
- Supports `RIFFAVI` (AVI) and ASF/WMV container formats
- **`CAviTocParser`** — AVI RIFF parser (`ITocParser`)

### File I/O
- **`CWlxFileIo`** — File I/O abstraction (`IFileIo`)
  - Direct file I/O: `CreateFileW`, `ReadFile`, `WriteFile`, `SetFilePointer`, `SetEndOfFile`

### Video/Image Analysis Engine
- **`CImageHandler`** — Image processing
- **`CColorAnalyzer`** — Color analysis (histogram, dominant colors)
- **`CMotionAnalyzer`** — Motion analysis (frame differencing)
- **`CMetadataHandler`** — Metadata extraction
- **`CDvDataHandler`** — DV (digital video) data handling

### Scene Detection / Clustering
- **`CShotDetector`** — Shot boundary detection
  - `CShotDetectorIntegrator` — Integrates detection over time
  - `CShotDetectorTime` — Time-based shot detection
- **`CClusterDetector`** — Cluster detection (groups related shots)
  - `CClusterDetectorIntegrator` — Integrates cluster detection
  - `CClusterDetectorTime` — Time-based cluster detection
  - `CClusterInfo` — Cluster metadata

### Feature Extraction
- **`CFeatureExtractor`** — Extracts features from media for analysis
- **`CDistanceMeasurer`** — Measures distance/similarity between features

### Thumbnail Generation
- **`CThumbnailGenerator`** — Generates thumbnails from video frames

### Property Store
- **`CPropertyStore`** — Property storage with sparse blocks
  - `CTPtrArray<UPROP_REC>` — Property record array
  - `CTSparseBlock<K, UPROP_REC>` — Sparse block storage
  - `CTPtrArray<ULANG_REC>` — Language record array
  - `CTSparseBlock<K, ULANG_REC>` — Language block storage

### Custom Template Containers
- `CTDynArray<T>` — Dynamic array template
  - Instantiated for: `UIToc*`, `U_INDEX_PAIR`, `U_BASE_ENTRY`, `CClusterInfo*`
- `CTPtrArray<T>` — Pointer array template
  - Instantiated for: `U_CLUSTER`, `UPROP_REC`, `ULANG_REC`
- `CTSparseBlock<K,T,BlockSz>` — Sparse block storage
  - Block sizes: `$00` (1 entry), `$0A` (10 entries)

### GDI+ Wrappers
- `GdiplusBase@Gdiplus@@`, `Bitmap@Gdiplus@@`, `Image@Gdiplus@@`

## Video Codec Support (from string constants)

Fourcc codes found in `.data` section:
- **Video**: `dvhd`, `dvsl`, `dv25`, `dv50`, `dvh1` (DV variants)
- **Video**: `YUYV`, `IYUV`, `Y411`, `Y41P`, `YUY2`, `YVYU`, `UYVY`, `Y211`, `YV12`
- **Media**: `RIFFAVI`, `mtoc` (media table of contents marker)

## DSP / Signal Processing Pipeline

```
Media File Input
    │
    ├── ASF Parser (CAsfTocParser) ──► ASF/WMV ToC generation
    ├── AVI Parser (CAviTocParser) ──► AVI/RIFF ToC generation
    ├── DvDataHandler ──────────────► DV tape handling
    └── MetadataHandler ────────────► Metadata extraction
            │
            ▼
    ToC System (CWlxToc → CWlxTocEntryList → CWlxTocEntry)
            │
            ▼
    Analysis Pipeline:
    ├── CFeatureExtractor ──► Feature vectors (SSE2 math: sin/cos/log/sqrt)
    ├── CColorAnalyzer ─────► Color histograms, dominant colors
    ├── CMotionAnalyzer ────► Motion vectors, activity levels
    ├── CDistanceMeasurer ──► Similarity between features
    ├── CShotDetector ──────► Shot boundary detection
    │   ├── CShotDetectorIntegrator
    │   └── CShotDetectorTime
    ├── CClusterDetector ───► Shot grouping/clustering
    │   ├── CClusterDetectorIntegrator
    │   ├── CClusterDetectorTime
    │   └── CClusterInfo
    └── CThumbnailGenerator ──► GDI+ thumbnail output
            │
            ▼
    DMO Outputs:
    ├── CWLXTocGeneratorDmo (IMediaObject)
    └── CWLXThumbnailGeneratorDmo (IMediaObject)
```

## DMO Registration Pattern

The DLL uses **msdmo.dll** for DMO registration:
- `DMORegister` — Registers DMOs with the system
- `DMOUnregister` — Removes DMO registration
- `MoInitMediaType` / `MoFreeMediaType` / `MoCopyMediaType` — Media type lifecycle

This means the DLL's DMOs are discoverable by DirectShow/WME graphs via the standard DMO enumeration system.

## Test Harness Results
| Export | Status | Notes |
|--------|--------|-------|
| DllCanUnloadNow | **OK** → S_OK | Reports can unload |
| DllGetClassObject | SEH `0xC0000005` | Expected — no `OleInitialize`, null CLSID |
| DllRegisterServer | Skipped | Would write 11 CLSID entries to registry |
| DllUnregisterServer | Skipped | Would remove 11 CLSID entries |

## Key Findings

1. **Not "Audio DSP"** — Despite the filename, WLXDSPA is primarily a **video analysis and Table of Contents** DLL, not audio processing. The `DMA` likely stands for **Digital Media Analysis**.

2. **Shot Detection Engine** — Contains a complete shot boundary detection + clustering system for automatic scene segmentation of video files.

3. **SSE2 Math Processing** — Uses hardware-accelerated sin/cos/log/sqrt for feature extraction computations.

4. **DMO-Based Architecture** — Core processing is implemented as DirectX Media Objects, making them composable in DirectShow graphs.

5. **Multi-Format ToC** — Parses both ASF (WMV/WMA) and AVI (RIFF) containers to build a unified Table of Contents structure.

6. **Property Store** — Sparse-block property storage system with multi-language support (`LANG_REC`).
