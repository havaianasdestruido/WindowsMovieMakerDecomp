# WLXFaceRecognition.dll — Test Harness & SDK Wrappers

## 1. Exported Function Signatures

### 1.1 COM Infrastructure Exports (4 functions)

All four use `__stdcall` calling convention (STDAPI = `HRESULT __stdcall`).

| # | Name | Signature | Ordinal | RVA |
|---|------|-----------|---------|-----|
| 1 | `DllCanUnloadNow` | `HRESULT __stdcall DllCanUnloadNow(void)` | 1 | 0x51FC |
| 2 | `DllGetClassObject` | `HRESULT __stdcall DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)` | 2 | 0x5215 |
| 3 | `DllRegisterServer` | `HRESULT __stdcall DllRegisterServer(void)` | 3 | 0x5225 |
| 4 | `DllUnregisterServer` | `HRESULT __stdcall DllUnregisterServer(void)` | 4 | 0x5241 |

### 1.2 COM Interfaces (7 COM objects, accessed via DllGetClassObject + CoCreateInstance)

#### IFaceDetection (CLSID `{4107FA03-3FD3-4406-B4F3-68E6D610EC2B}`)
```cpp
// {4107FA03-3FD3-4406-B4F3-68E6D610EC2B}
DECLARE_INTERFACE_(IFaceDetection, IUnknown)
{
    STDMETHOD(RunFaceDetection)(IImageData* pImage, IFaceRegionSet** ppRegions) = 0;
    STDMETHOD(FaceDetection)(IImageData* pImage, IFaceRegionSet** ppRegions) = 0;
    STDMETHOD(InitializeCustom)(LPCWSTR wszModelPath) = 0;
};
```

#### IFaceRegionSet (CLSID `{D01C34A5-A6DC-4d28-ABBD-78D06EA27B60}`)
```cpp
DECLARE_INTERFACE_(IFaceRegionSet, IUnknown)
{
    STDMETHOD(GetRegion)(UINT index, IFaceRegion** ppRegion) = 0;
    STDMETHOD(SetRegion)(UINT index, IFaceRegion* pRegion) = 0;
    STDMETHOD(GetCount)(UINT* puCount) = 0;
};
```

#### IFaceRegion
```cpp
DECLARE_INTERFACE_(IFaceRegion, IUnknown)
{
    STDMETHOD(GetFacePose)(UINT* puPose) = 0;
    STDMETHOD(SetFacePose)(UINT uPose) = 0;
    STDMETHOD(GetFaceRegionRect)(RECT* pRect) = 0;
    STDMETHOD(SetFaceRegionRect)(const RECT* pRect) = 0;
    STDMETHOD(GetPersonId)(UINT* puPersonId) = 0;
    STDMETHOD(SetPersonId)(UINT uPersonId) = 0;
    STDMETHOD(GetPersonName)(BSTR* pbstrName) = 0;
    STDMETHOD(SetPersonName)(LPCWSTR wszName) = 0;
    STDMETHOD(GetConfidence)(FLOAT* pfConfidence) = 0;
    STDMETHOD(GetRepresentation)(IFaceRepresentation** ppRep) = 0;
    STDMETHOD(SetRepresentation)(IFaceRepresentation* pRep) = 0;
    STDMETHOD(GetThumbnailImageData)(IImageData** ppImage) = 0;
    STDMETHOD(SetThumbnailImageData)(IImageData* pImage) = 0;
};
```

#### IFaceRecognitionPipeline (CLSID `{EF401225-1260-4716-A842-7D180DC14C1E}`)
```cpp
DECLARE_INTERFACE_(IFaceRecognitionPipeline, IUnknown)
{
    STDMETHOD(RecognizeFacesFromImage)(IImageData* pImage, IFaceRegionSet** ppRegions) = 0;
    STDMETHOD(RecognizeFacesFromFaceRegions)(IImageData* pImage, IFaceRegionSet* pRegions, IFaceRegionSet** ppResults) = 0;
    STDMETHOD(RecognizeFacesFromRepresentations)(IFaceRegionSet* pReps, IFaceRegionSet** ppResults) = 0;
    STDMETHOD(GroupFaces)(IFaceRegionSet* pReps, IFaceRegionSet** ppGroups) = 0;
    STDMETHOD(SetMatchThreshold)(UINT uThreshold) = 0;
    STDMETHOD(GetCacheStamp)(UINT* puStamp) = 0;
    STDMETHOD(SetImageId)(UINT uImageId) = 0;
    STDMETHOD(GetExemplarCache)(IUnknown** ppCache) = 0;
};
```

#### IFaceRepresentation
```cpp
DECLARE_INTERFACE_(IFaceRepresentation, IUnknown)
{
    STDMETHOD(GetRepresentation)(BYTE* pBuffer, UINT* pcbSize) = 0;
    STDMETHOD(SetRepresentation)(const BYTE* pBuffer, UINT cbSize) = 0;
    STDMETHOD(GetPersonId)(UINT* puPersonId) = 0;
    STDMETHOD(SetPersonId)(UINT uPersonId) = 0;
    STDMETHOD(GetPersonName)(BSTR* pbstrName) = 0;
    STDMETHOD(SetPersonName)(LPCWSTR wszName) = 0;
};
```

#### IImageData
```cpp
DECLARE_INTERFACE_(IImageData, IUnknown)
{
    STDMETHOD(CreateImageDataFromHBITMAP)(HBITMAP hBitmap) = 0;
    STDMETHOD(CreateImageDataFromWICBitmap)(IUnknown* pWICBitmap) = 0;
    STDMETHOD(CreateImageDataFromBuffer)(const BYTE* pBuffer, UINT cbSize, UINT uFormat) = 0;
    STDMETHOD(GetPixelFormat)(UINT* puFormat) = 0;
    STDMETHOD(GetResolution)(UINT* puWidth, UINT* puHeight) = 0;
    STDMETHOD(GetStride)(INT* pStride) = 0;
    STDMETHOD(GetDataPointer)(BYTE** ppData) = 0;
    STDMETHOD(IsGrayscale)(BOOL* pbGray) = 0;
    STDMETHOD(IsBlackWhite)(BOOL* pbBW) = 0;
};
```

#### IImageManager (CLSID `{483A53CD-EF18-4b19-8AA3-2E2E3214EB41}`)
```cpp
DECLARE_INTERFACE_(IImageManager, IUnknown)
{
    STDMETHOD(LoadResource)(HINSTANCE hInst, LPCWSTR wszName, LPCWSTR wszType, IImageData** ppImage) = 0;
};
```

### 1.3 FacePose Enum
```cpp
enum FacePose
{
    FacePoseFrontal                = 0,
    FacePoseFrontal_CCW30         = 1,
    FacePoseLeftProfile           = 2,
    FacePoseLeftProfile_CCW30     = 3,
    FacePoseLeftHalfProfile       = 4,
    FacePoseLeftHalfProfile_CCW30 = 5,
    FacePoseRightProfile          = 6,
    FacePoseRightProfile_CCW30    = 7,
    FacePoseRightHalfProfile      = 8,
    FacePoseRightHalfProfile_CCW30 = 9,
    FacePoseNone                  = 10
};
```

### 1.4 MatchThreshold Enum
```cpp
enum MatchThreshold
{
    MatchThresholdDefault   = 0,
    MatchThresholdLoose     = 1,
    MatchThresholdModerate  = 2,
    MatchThresholdStrict    = 3,
    MatchThresholdMax       = 4
};
```

---

## 2. Test Harness Functions (Per Export)

### 2.1 DllCanUnloadNow Harness

```cpp
struct DllCanUnloadNowTest
{
    const char* name = "DllCanUnloadNow";

    bool Run(HMODULE hDll, int variation, TestLog& log)
    {
        typedef HRESULT (__stdcall *FnDllCanUnloadNow)(void);
        auto fn = (FnDllCanUnloadNow)GetProcAddress(hDll, "DllCanUnloadNow");
        if (!fn) { log.Fail("GetProcAddress failed"); return false; }

        __try
        {
            HRESULT hr = fn();
            // Expected: S_OK when no outstanding references, S_FALSE otherwise
            log.Log("DllCanUnloadNow() returned 0x%08X", hr);
            return (hr == S_OK || hr == S_FALSE);
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            log.Fail("SEH exception caught");
            return false;
        }
    }
};
```

### 2.2 DllGetClassObject Harness

```cpp
struct DllGetClassObjectTest
{
    const char* name = "DllGetClassObject";

    // Known CLSIDs from analysis
    static const CLSID CLSID_FaceDetection;
    static const CLSID CLSID_FaceRecognitionPipeline;
    static const CLSID CLSID_ImageManager;
    static const CLSID CLSID_FaceRegion;

    static const IID IID_IClassFactory;

    bool Run(HMODULE hDll, int variation, TestLog& log)
    {
        typedef HRESULT (__stdcall *FnDllGetClassObject)(REFCLSID, REFIID, LPVOID*);
        auto fn = (FnDllGetClassObject)GetProcAddress(hDll, "DllGetClassObject");
        if (!fn) { log.Fail("GetProcAddress failed"); return false; }

        REFCLSID clsid = GetCLSID(variation);
        IClassFactory* pFactory = nullptr;

        __try
        {
            HRESULT hr = fn(clsid, IID_IClassFactory, (LPVOID*)&pFactory);

            if (SUCCEEDED(hr) && pFactory)
            {
                log.Log("DllGetClassObject(%d) succeeded, pFactory=0x%p", variation, pFactory);
                // Test IClassFactory::CreateInstance and LockServer
                TestClassFactory(pFactory, log);
                pFactory->Release();
                return true;
            }
            else
            {
                log.Log("DllGetClassObject(%d) returned 0x%08X (expected for unknown CLSIDs)", variation, hr);
                return (hr == CLASS_E_CLASSNOTAVAILABLE || hr == E_NOINTERFACE);
            }
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            log.Fail("SEH exception caught");
            if (pFactory) pFactory->Release();
            return false;
        }
    }

    REFCLSID GetCLSID(int variation)
    {
        switch (variation)
        {
        case 0: return CLSID_FaceDetection;
        case 1: return CLSID_FaceRecognitionPipeline;
        case 2: return CLSID_ImageManager;
        case 3: return CLSID_FaceRegion;
        default: return GUID_NULL;
        }
    }

    void TestClassFactory(IClassFactory* pFactory, TestLog& log)
    {
        IUnknown* pUnk = nullptr;
        HRESULT hr = pFactory->CreateInstance(nullptr, IID_IUnknown, (LPVOID*)&pUnk);
        if (SUCCEEDED(hr) && pUnk)
        {
            log.Log("  CreateInstance(IUnknown) succeeded: 0x%p", pUnk);
            pUnk->Release();
        }
        else
        {
            log.Log("  CreateInstance(IUnknown) returned 0x%08X", hr);
        }

        hr = pFactory->LockServer(TRUE);
        log.Log("  LockServer(TRUE) returned 0x%08X", hr);
        hr = pFactory->LockServer(FALSE);
        log.Log("  LockServer(FALSE) returned 0x%08X", hr);
    }
};
```

### 2.3 DllRegisterServer Harness

```cpp
struct DllRegisterServerTest
{
    const char* name = "DllRegisterServer";

    bool Run(HMODULE hDll, int variation, TestLog& log)
    {
        typedef HRESULT (__stdcall *FnDllRegisterServer)(void);
        auto fn = (FnDllRegisterServer)GetProcAddress(hDll, "DllRegisterServer");
        if (!fn) { log.Fail("GetProcAddress failed"); return false; }

        __try
        {
            HRESULT hr = fn();
            log.Log("DllRegisterServer() returned 0x%08X", hr);
            // Expected: S_OK (self-reg), or SELFREG_E_CLASS if class registration fails
            if (SUCCEEDED(hr))
            {
                log.Log("  Registry entries created under Software\\Microsoft\\MSRA\\FaceRecognition\\");
                return true;
            }
            log.Fail("Registration failed");
            return false;
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            log.Fail("SEH exception caught");
            return false;
        }
    }
};
```

### 2.4 DllUnregisterServer Harness

```cpp
struct DllUnregisterServerTest
{
    const char* name = "DllUnregisterServer";

    bool Run(HMODULE hDll, int variation, TestLog& log)
    {
        typedef HRESULT (__stdcall *FnDllUnregisterServer)(void);
        auto fn = (FnDllUnregisterServer)GetProcAddress(hDll, "DllUnregisterServer");
        if (!fn) { log.Fail("GetProcAddress failed"); return false; }

        __try
        {
            HRESULT hr = fn();
            log.Log("DllUnregisterServer() returned 0x%08X", hr);
            if (SUCCEEDED(hr))
            {
                log.Log("  Registry entries removed from Software\\Microsoft\\MSRA\\FaceRecognition\\");
                return true;
            }
            log.Fail("Unregistration failed");
            return false;
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            log.Fail("SEH exception caught");
            return false;
        }
    }
};
```

---

## 3. Complete Test Harness main.cpp

```
// File: test/WLXFaceRecognitionHarness/main.cpp
```

Features:
- Loads the DLL via LoadLibraryW with variations (normal path, NULL, invalid path)
- Iterates all 4 exports via GetProcAddress
- For each export, runs multiple input variations:
  - **DllCanUnloadNow**: call before/after COM object creation
  - **DllGetClassObject**: test all known CLSIDs + GUID_NULL + random GUID
  - **DllRegisterServer**: test normal registration, double-registration
  - **DllUnregisterServer**: test unregistration, double-unregistration
- Tests COM interface creation via CoCreateInstance for all 7 known CLSIDs
- Tests LoadLibrary edge cases: empty path, very long path, special chars
- Calls GetProcAddress with: valid name, invalid name, NULL, ordinal
- Tests FreeLibrary with: valid handle, NULL, invalid handle
- Uses SEH (`__try/__except`) to catch access violations from invalid calls
- Logs all results with timestamps
- Returns 0 for all-pass, nonzero for any failure

---

## 4. CMakeLists.txt for Test Harness

```
cmake_minimum_required(VERSION 3.15)

project(WLXFaceRecognitionHarness
    VERSION 1.0.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- Compile definitions ----
add_compile_definitions(
    WINVER=0x0601
    _WIN32_WINNT=0x0601
    _WINDOWS
    UNICODE
    _UNICODE
    NOMINMAX
    WIN32_LEAN_AND_MEAN
    _CRT_SECURE_NO_WARNINGS
)

# ---- Compile options ----
if(MSVC)
    add_compile_options(/W4 /WX- /MP /GS /Gy /Zc:forScope /Zc:wchar_t /Zi /Od)
endif()

# ---- Dependencies (matching the original binary's known deps) ----
# System DLLs that WLXFaceRecognition.dll imports at runtime.
# We don't link against them directly; we need them for PE loading.
set(REQUIRED_SYSTEM_DLLS
    kernel32
    user32
    advapi32
    ole32
    oleaut32
    gdi32
    gdiplus
    windowscodecs
    shlwapi
    version
)

# Windows Live dependencies (must be findable at test runtime)
# WLXPhotoBase.dll and WLXPhotoSqm.dll are delay-loaded by the original.
# The test needs to have these DLLs in PATH or alongside the test binary.
set(WLX_DEPENDENCY_DIR "${CMAKE_SOURCE_DIR}/undecomp/Photo Gallery" CACHE PATH
    "Directory containing original WLX DLLs for test harness linking")

# ---- Executable target ----
add_executable(${PROJECT_NAME} WIN32
    main.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/WLXFaceRecognition
    ${CMAKE_SOURCE_DIR}/src/WLXPhotoBase
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${REQUIRED_SYSTEM_DLLS}
    # Delay-load the original WLX DLLs so the test harness links but allows
    # test-time resolution to the original binary under test.
    delayimp
)

# The test EXE should sit alongside the original DLLs for LoadLibrary to find them.
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/test/WLXFaceRecognitionHarness"
)

# ---- Custom target to set up test environment ----
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/undecomp/Photo Gallery/WLXFaceRecognition.dll"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/"
    COMMENT "Copying original WLXFaceRecognition.dll for test harness"
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/undecomp/Photo Gallery/WLXPhotoBase.dll"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/"
    COMMENT "Copying WLXPhotoBase.dll dependency"
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/undecomp/Photo Gallery/WLXPhotoSqm.dll"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/"
    COMMENT "Copying WLXPhotoSqm.dll dependency"
)

# ---- Test registration ----
include(CTest)
add_test(
    NAME WLXFaceRecognition_Harness
    COMMAND ${PROJECT_NAME}
    WORKING_DIRECTORY "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
)
```

**Dependency Notes:**
- The original binary links against **MSVCR110.dll** and **MSVCP110.dll** (VC 2012 CRT). These must be present on the test machine (installed via vcredist_x86.exe for VS 2012).
- **WLXPhotoBase.dll** and **WLXPhotoSqm.dll** must be in the same directory as the test EXE (or in PATH) because the original binary loads them at runtime.
- **WindowsCodecs.dll** (WIC) is a system component on Windows 7+.

---

## 5. SDk Wrapper: COM Interface Wrappers

### 5.1 SDK Wrapper Header: FaceRecognitionSdk.h

```cpp
#pragma once
#ifndef FACERECOGNITIONSDK_H
#define FACERECOGNITIONSDK_H

#include <windows.h>
#include <objbase.h>
#include <initguid.h>
#include <assert.h>
#include <string>
#include <vector>

// ============================================================================
// CLSIDs (from binary string analysis)
// ============================================================================

// {EF401225-1260-4716-A842-7D180DC14C1E} - Confirmed in RGS script
DEFINE_GUID(CLSID_FaceRecognitionPipeline,
    0xef401225, 0x1260, 0x4716, 0xa8, 0x42, 0x7d, 0x18, 0x0d, 0xc1, 0x4c, 0x1e);

// {4107FA03-3FD3-4406-B4F3-68E6D610EC2B} - Likely FaceDetection
DEFINE_GUID(CLSID_FaceDetection,
    0x4107fa03, 0x3fd3, 0x4406, 0xb4, 0xf3, 0x68, 0xe6, 0xd6, 0x10, 0xec, 0x2b);

// {483A53CD-EF18-4b19-8AA3-2E2E3214EB41} - Likely ImageManager
DEFINE_GUID(CLSID_ImageManager,
    0x483a53cd, 0xef18, 0x4b19, 0x8a, 0xa3, 0x2e, 0x2e, 0x32, 0x14, 0xeb, 0x41);

// {D01C34A5-A6DC-4d28-ABBD-78D06EA27B60} - Likely FaceRegion or FaceRegionSet
DEFINE_GUID(CLSID_FaceRegion,
    0xd01c34a5, 0xa6dc, 0x4d28, 0xab, 0xbd, 0x78, 0xd0, 0x6e, 0xa2, 0x7b, 0x60);

DEFINE_GUID(CLSID_FaceRegionSet,
    0xd01c34a5, 0xa6dc, 0x4d28, 0xab, 0xbd, 0x78, 0xd0, 0x6e, 0xa2, 0x7b, 0x60);

DEFINE_GUID(CLSID_FaceRepImpl,
    0xd01c34a5, 0xa6dc, 0x4d28, 0xab, 0xbd, 0x78, 0xd0, 0x6e, 0xa2, 0x7b, 0x60);

DEFINE_GUID(CLSID_ImageData,
    0xd01c34a5, 0xa6dc, 0x4d28, 0xab, 0xbd, 0x78, 0xd0, 0x6e, 0xa2, 0x7b, 0x60);

// ============================================================================
// FacePose enum (11 pose categories)
// ============================================================================
enum FacePose
{
    FacePoseFrontal                = 0,
    FacePoseFrontal_CCW30         = 1,
    FacePoseLeftProfile           = 2,
    FacePoseLeftProfile_CCW30     = 3,
    FacePoseLeftHalfProfile       = 4,
    FacePoseLeftHalfProfile_CCW30 = 5,
    FacePoseRightProfile          = 6,
    FacePoseRightProfile_CCW30    = 7,
    FacePoseRightHalfProfile      = 8,
    FacePoseRightHalfProfile_CCW30 = 9,
    FacePoseNone                  = 10
};
```

### 5.3 SDK Wrapper Implementation (partial): FaceRecognitionSdk.cpp

The SDK wrapper provides C++ RAII wrappers around the COM interfaces. Key classes:

- **FaceRecognitionDll** — Loads/unloads the DLL, wraps the 4 exports
- **FaceDetectionPtr** — COM smart pointer for IFaceDetection
- **FaceRecognitionPipelinePtr** — COM smart pointer for IFaceRecognitionPipeline
- **FaceRegionSetPtr** — COM smart pointer for IFaceRegionSet
- **FaceRegionPtr** — COM smart pointer for IFaceRegion
- **FaceRepresentationPtr** — COM smart pointer for IFaceRepresentation
- **ImageDataPtr** — COM smart pointer for IImageData
- **ImageManagerPtr** — COM smart pointer for IImageManager

Each wrapper:
- Automatically calls CoCreateInstance in constructor
- Provides typed access to interface methods
- Uses AddRef/Release for lifetime management via _com_ptr_t or custom RefCounted base
- Wraps HRESULT returns into C++ exceptions

---

## 6. Expected Behaviors

### 6.1 DllCanUnloadNow
| Condition | Expected HRESULT | Notes |
|-----------|-----------------|-------|
| No COM objects created | `S_OK` (0x00000000) | DLL can be unloaded |
| Outstanding COM objects | `S_FALSE` (0x00000001) | DLL still in use |
| After CoInitialize failure | `S_OK` | Module refcount may be 0 |

### 6.2 DllGetClassObject
| CLSID | Expected | Notes |
|-------|----------|-------|
| `CLSID_FaceDetection` | `S_OK` + valid IClassFactory | Creates VFaceDetection objects |
| `CLSID_FaceRecognitionPipeline` | `S_OK` + valid IClassFactory | Creates VFaceRecognitionPipeline objects |
| `CLSID_ImageManager` | `S_OK` + valid IClassFactory | Creates VImageManager objects |
| `CLSID_FaceRegion` | `S_OK` + valid IClassFactory | Creates VFaceRegion objects |
| `CLSID_FaceRegionSet` | `S_OK` + valid IClassFactory | Creates VFaceRegionSet objects |
| `CLSID_FaceRepImpl` | `S_OK` + valid IClassFactory | Creates VFaceRepImpl objects |
| `CLSID_ImageData` | `S_OK` + valid IClassFactory | Creates VImageData objects |
| Random GUID | `CLASS_E_CLASSNOTAVAILABLE` | Not a registered coclass |
| `GUID_NULL` | `CLASS_E_CLASSNOTAVAILABLE` | No null CLSID |
| `riid != IID_IClassFactory` | `E_NOINTERFACE` | Only IClassFactory supported |
| `ppv == NULL` | `E_POINTER` | Invalid output pointer |

### 6.3 DllRegisterServer
| Condition | Expected | Notes |
|-----------|----------|-------|
| Normal (admin) | `S_OK` | Creates registry keys under `Software\Microsoft\MSRA\FaceRecognition\` |
| Without admin rights | `SELFREG_E_CLASS` or `E_ACCESSDENIED` | Registry write fails |
| Double registration | `S_OK` | Idempotent |
| Corrupted DLL | `SELFREG_E_TYPELIB` | Type library registration fails |

### 6.4 DllUnregisterServer
| Condition | Expected | Notes |
|-----------|----------|-------|
| Normal (admin) | `S_OK` | Removes registry keys |
| Already unregistered | `S_OK` | Idempotent |
| Without admin rights | `E_ACCESSDENIED` | Registry delete fails |

### 6.5 COM Interface CoCreateInstance

| Interface | Expected Behavior |
|-----------|------------------|
| **IFaceDetection** | `RunFaceDetection` returns empty region set if no face found; returns regions otherwise. `InitializeCustom` with NULL path returns `E_INVALIDARG`. |
| **IFaceRecognitionPipeline** | Without prior face enrollment, `RecognizeFacesFromImage` returns regions with `bIsKnown=FALSE`. Threshold affects matching sensitivity: Strict = fewer matches, Loose = more matches. |
| **IFaceRegionSet** | `GetCount()` returns 0 for empty set. `GetRegion(0)` on empty set returns `E_INVALIDARG` or `E_BOUNDS`. |
| **IFaceRegion** | `GetConfidence()` returns 0.0-1.0 (higher = more confident). `GetFacePose()` returns one of the 11 `FacePose` enum values. |
| **IFaceRepresentation** | `GetRepresentation(NULL, &size)` returns required buffer size. `SetRepresentation` with NULL buffer returns `E_INVALIDARG`. |
| **IImageData** | `CreateImageDataFromHBITMAP(NULL)` returns `E_INVALIDARG`. `GetDataPointer()` on uninitialized object returns `E_UNEXPECTED`. |
| **IImageManager** | `LoadResource` with NULL module returns `E_INVALIDARG`. |

### 6.6 LoadLibrary Edge Cases
| Input | Expected Behavior |
|-------|------------------|
| `L"WLXFaceRecognition.dll"` (normal) | Success, valid HMODULE |
| `L"C:\\Full\\Path\\To\\WLXFaceRecognition.dll"` | Success if path valid |
| `NULL` | LoadLibrary returns NULL, GetLastError = ERROR_INVALID_PARAMETER |
| `L""` (empty string) | Failure, GetLastError = ERROR_MOD_NOT_FOUND |
| `L"nonexistent.dll"` | Failure, GetLastError = ERROR_MOD_NOT_FOUND |
| Very long path (>260 chars) | Depends on Windows version; may fail on pre-10 |

### 6.7 GetProcAddress Edge Cases
| Input | Expected Behavior |
|-------|------------------|
| `"DllCanUnloadNow"` | Valid function pointer |
| `"DllGetClassObject"` | Valid function pointer |
| `"DllRegisterServer"` | Valid function pointer |
| `"DllUnregisterServer"` | Valid function pointer |
| `"NonexistentFunction"` | NULL returned |
| `NULL` (name) | NULL returned, last error = ERROR_INVALID_PARAMETER |
| `""` (empty name) | NULL returned |
| Ordinal 1-4 | Valid function pointer |
| Ordinal 999 | NULL returned |

### 6.8 Known Failure Modes
1. **MSVCR110.dll missing**: LoadLibrary fails with ERROR_MOD_NOT_FOUND (126). Install VS 2012 redistributable.
2. **WLXPhotoBase.dll missing**: LoadLibrary succeeds (delay-load), but first call to Base::New/Delete/Throw crashes.
3. **WLXPhotoSqm.dll missing**: Same as above (delay-load).
4. **CoInitialize not called**: DllGetClassObject may fail or COM operations crash.
5. **Non-admin**: DllRegisterServer/DllUnregisterServer fail with E_ACCESSDENIED.
6. **64-bit process**: LoadLibrary fails with ERROR_BAD_EXE_FORMULAT (193). Must run x86.
7. **Resource loading fails**: Face detection returns E_FAIL if cascade data cannot be extracted from .rsrc section.

### 6.9 Thread Safety
- The DLL uses ATL `CComMultiThreadModel` — all COM objects support multi-threaded apartments.
- CoInitializeEx(COINIT_MULTITHREADED) is supported but COINIT_APARTMENTTHREADED is the default.
- DllCanUnloadNow uses Interlocked operations for module refcount — thread-safe.
- Face detection is NOT reentrant within the same object instance; each thread should create its own detector.

### 6.10 Memory Management
- All COM objects use `BasePrivate::New`/`BasePrivate::Delete` from WLXPhotoBase.dll for allocations.
- Memory leaks in the original binary are unlikely but possible in error paths.
- CoTaskMemAlloc/Free used for COM interface marshaling (BSTR, VARIANT).
- Face representation data buffers are caller-allocated (call `GetRepresentation(NULL, &size)` first to query required size, then allocate and call again).

---

## 7. Test Variations Summary

| Test | Variations | Description |
|------|-----------|-------------|
| LoadLibrary | 6 | Valid path, NULL, empty, invalid, long path, UNC path |
| DllCanUnloadNow | 3 | Before CreateInstance, after Release all, with outstanding refs |
| DllGetClassObject | 10 | 7 known CLSIDs + GUID_NULL + random GUID + NULL ppv |
| DllRegisterServer | 2 | Normal, double-register |
| DllUnregisterServer | 2 | Normal, double-unregister |
| GetProcAddress | 7 | 4 valid names, 1 invalid, NULL, ordinal variants |
| FreeLibrary | 3 | Valid handle, NULL, invalid handle |
| CoCreateInstance | 7 | All 7 known CLSIDs |
| IFaceDetection methods | 4 | RunFaceDetection (valid/NULL image), InitializeCustom, FaceDetection |
| IFaceRecognitionPipeline | 6 | Recognize from image/regions/reps, GroupFaces, SetMatchThreshold, CacheStamp |
| IFaceRegionSet | 3 | GetCount, GetRegion(0), GetRegion(out of bounds) |
| IFaceRegion | 8 | Get/Set for pose, rect, person ID, confidence, name |
| IFaceRepresentation | 4 | GetRepresentation (NULL/sized buffer), SetRepresentation, PersonId |
| IImageData | 5 | CreateFromHBITMAP (NULL/valid), GetResolution, GetPixelFormat, IsGrayscale |
| IImageManager | 2 | LoadResource (valid/NULL params) |
