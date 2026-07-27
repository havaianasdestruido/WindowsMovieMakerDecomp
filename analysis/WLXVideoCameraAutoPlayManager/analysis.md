# WLXVideoCameraAutoPlayManager.exe Analysis

## Overview

**File:** WLXVideoCameraAutoPlayManager.exe (Windows Live Photo Gallery 2012 — Video Camera AutoPlay Manager)
**Role:** COM LocalServer32 EXE implementing `IHWEventHandler` for video camera arrival autoplay
**Image size:** 0x12000 (~72 KB)
**PDB:** `WLXVideoCameraAutoPlayManager.pdb` `{0ADBCAB9-BCDD-4175-9D9E-62906F016ACC}`
**Build:** 16.4.3528.0331 (timestamp: 2014-04-01 01:28:12)

## PE Structure

| Field | Value |
|-------|-------|
| Machine | x86 (0x14C) |
| Subsystem | Windows GUI (2) |
| Linker | MSVC 11.00 |
| Image Base | 0x00400000 |
| Image Size | 0x12000 |
| Entry Point | 0x00406D75 |
| OS/Image Version | 6.02 / 6.02 |
| DLL Characteristics | Dynamic Base (ASLR), NX Compatible, Terminal Server Aware |
| Stack reserve/commit | 256 KB / 8 KB |
| Heap reserve/commit | 1 MB / 4 KB |

### Sections

| Section | VA | VirtSize | RawSize | Characteristics |
|---------|------|----------|---------|-----------------|
| `.text` | 0x1000 | 0x7450 (29 KB) | 0x7600 | Code, Execute Read |
| `.data` | 0x9000 | 0x8BC (2.2 KB) | 0x600 | Init Data, Read Write |
| `.rsrc` | 0xA000 | 0x6F40 (28 KB) | 0x7000 | Init Data, Read Only |
| `.reloc` | 0x11000 | 0x8B0 (2.2 KB) | 0xA00 | Init Data, Discardable, Read Only |

**Key observation**: Very small binary (~72 KB). The `.rsrc` section at 28 KB contains COM registration scripts (RGSIDL/RGS), version info, and icon resources. The `.text` section at 29 KB implements the entire autoplay manager.

## Exports

**None** — this is an EXE, not a DLL. No exported functions.

## Import Table

### CRT / Runtime
- **MSVCR110.dll** (33 functions): CRT core — memory (malloc/free/calloc/realloc/memcpy_s/memset/memmove_s/wmemcpy_s), string (wcscat_s/wcscpy_s/wcsncpy_s/wcsstr), exceptions (CxxThrowException/CxxFrameHandler3/terminate), C++ new/delete (`??2@YAPAXI@Z`/`??3@YAXPAX@Z`/`??_U@YAPAXI@Z`/`??_V@YAXPAX@Z`), locale, init (_initterm/_initterm_e), process (_amsg_exit/__wgetmainargs/__set_app_type), math (exit/_exit/_cexit), debug (_crt_debugger_hook/__crtTerminateProcess/__crtUnhandledException), threading (_lock/_unlock/_calloc_crt/_onexit/__dllonexit), security (_except_handler4_common/_controlfp_s)

### OS
- **KERNEL32.dll** (53 functions): Module loading (LoadLibraryExW/FreeLibrary/GetModuleHandleW/GetProcAddress), registry (none — uses ADVAPI32), resources (FindResourceW/LoadResource/LockResource/SizeofResource/FindResourceExW), sync (CreateEventW/WaitForSingleObject/SetEvent/CRITICAL_SECTION), threading (CreateThread/GetCurrentThreadId), process (ExpandEnvironmentStringsW/GetCommandLineW), file (GetModuleFileNameW), time (GetSystemTimeAsFileTime/GetTickCount64/QueryPerformanceCounter/Sleep), debug (IsDebuggerPresent/RaiseException), heap (HeapAlloc/HeapFree/HeapReAlloc/HeapSize/HeapSetInformation/HeapDestroy), memory (EncodePointer/DecodePointer), string (CompareStringW/lstrcmpiW/MultiByteToWideChar), error (GetLastError/SetLastError/FormatMessageW/LocalFree)
- **USER32.dll** (7 functions): Message loop (GetMessageW/TranslateMessage/DispatchMessageW/PostThreadMessageW), window (CharNextW/CharUpperW), UI (MessageBoxW)
- **ADVAPI32.dll** (9 functions): Registry only — RegCloseKey/RegCreateKeyExW/RegDeleteKeyW/RegDeleteValueW/RegEnumKeyExW/RegOpenKeyExW/RegQueryInfoKeyW/RegSetValueExW/RegQueryValueExW

### COM
- **ole32.dll** (12 functions): COM LocalServer infrastructure (CoRegisterClassObject/CoResumeClassObjects/CoReleaseServerProcess/CoAddRefServerProcess/CoRevokeClassObject), COM core (CoInitialize/CoUninitialize/CoCreateInstance/StringFromGUID2/CoTaskMemAlloc/CoTaskMemRealloc/CoTaskMemFree)
- **OLEAUT32.dll** (7 ordinal-only): Ordinals 2,6,7,163,186,277 — likely SysAllocString/SysFreeString/SysStringLen/VariantInit/VariantClear

### Shell
- **SHELL32.dll** (1 function): `ShellExecuteW` — launch import wizard or Movie Maker

### Device
- **SETUPAPI.dll** (5 functions): Device interface enumeration (SetupDiCreateDeviceInfoList/SetupDiOpenDeviceInterfaceW/SetupDiGetDeviceInterfaceDetailW/SetupDiGetDeviceInterfaceAlias/SetupDiDestroyDeviceInfoList)

## COM Architecture

### IHWEventHandler Implementation

This EXE is registered as a COM LocalServer32 implementing the `IHWEventHandler` interface for Shell AutoPlay device arrival events.

#### COM Classes

| Class | CLSID | Interface | Purpose |
|-------|-------|-----------|---------|
| `WLXHWEventHandler` | `{9B5C97F6-B3A5-4A6D-8B03-993EC7291A22}` | `IHWEventHandler` | Autoplay handler for video cameras |

#### IHWEventHandler Vtable Layout

| Slot | Method | Description |
|------|--------|-------------|
| 0 | `QueryInterface` | COM QI |
| 1 | `AddRef` | Reference counting |
| 2 | `Release` | Reference counting |
| 3 | `Initialize(pszDeviceID, pszAltDeviceID, pszEventType)` | Called when handler is loaded |
| 4 | `HandleEvent(clsidHandler, ptszDeviceID, ptszAltDeviceID, pDataObject)` | Device arrival handler |
| 5 | `HandleEventWithProgress(...)` | With progress sink |
| 6 | `LegacyHandleNotification(...)` | Legacy notification |
| 7 | `BindToDevice(ptszDeviceID, ptszAltDeviceID)` | Device binding |
| 8 | `UnBindDevice(ptszDeviceID, ptszAltDeviceID)` | Device unbinding |

### COM Registration

#### ProgID
- `WXLAutoPlayMgr.WLXHWEventHandler.1`

#### AppID
- `{9B5C97F6-B3A5-4A6D-8B03-993EC7291A22}` — "Photo Gallery Video Camera Auto Play Manager"

## Autoplay Handler Flow

### Device Arrival Sequence
1. Shell detects new video camera device (via `DeviceArrival` event)
2. Shell enumerates registered `IHWEventHandler` implementations
3. COM instantiates `WLXVideoCameraAutoPlayManager.exe` as LocalServer32
4. `Initialize()` called with device ID, alt device ID, and event type (`/AutoPlay`)
5. `HandleEvent()` or `HandleEventWithProgress()` called with device info and data object
6. Handler examines device type — checks if video camera via SetupAPI device interface
7. If video camera: launches `WLXVideoAcquireWizard.exe` for import
8. If not recognized: may launch `MovieMaker.exe` or show error message

### Device Enumeration
Uses `SetupDiCreateDeviceInfoList` → `SetupDiOpenDeviceInterfaceW` → `SetupDiGetDeviceInterfaceDetailW` chain to enumerate and classify connected devices. `SetupDiGetDeviceInterfaceAlias` is used to check device interface aliases.

## Registry Paths

| Registry Path | Purpose |
|---------------|---------|
| `HKCR\CLSID\{9B5C97F6-...}\LocalServer32` | COM LocalServer registration |
| `HKCR\CLSID\{9B5C97F6-...}\Implemented Categories` | COM categories |
| `HKCR\AppID\{9B5C97F6-...}` | AppID description |
| `HKCR\WXLAutoPlayMgr.WLXHWEventHandler.1` | ProgID |
| `HKLM\Software\Microsoft\Windows\CurrentVersion\Explorer\AutoplayHandlers\Handlers\*` | Shell autoplay handler registration |

## Key Strings

- "WLXAutoPlayMgr" — internal module name
- "Import Video AutoPlay" — user-facing handler name
- "AutoPlay could not start Import Video. Import Video could not be found." — error message
- "AutoPlay could not start Windows Movie Maker." — error message
- "/AutoPlay" — event type string
- "DeviceArrival" — device event type
- "OneClickCapture" — one-click capture handler name
- "AutoCaptureBurnDVD" — auto capture burn DVD handler name
- "WLXVideoAcquireWizard" — wizard executable name
- "WLXVideoCameraAutoPlayManager.dll" — reference to companion DLL

## Architecture Summary

WLXVideoCameraAutoPlayManager.exe is a minimal COM LocalServer32 (~72 KB) implementing the `IHWEventHandler` Shell AutoPlay interface. When a video camera is connected via USB/IEEE 1394, the Shell's AutoPlay subsystem instantiates this EXE and calls `HandleEvent()` to determine the appropriate action. The handler examines the device via SetupAPI device interface queries and either launches the Photo Gallery video import wizard (`WLXVideoAcquireWizard.exe`) or falls back to Windows Movie Maker. The entire binary is remarkably small — the 29 KB `.text` section implements all device detection, COM boilerplate, and launch logic. The COM class is registered under `{9B5C97F6-B3A5-4A6D-8B03-993EC7291A22}` with ProgID `WXLAutoPlayMgr.WLXHWEventHandler.1`.
