# AlbumDownloadProtocolHandler.dll — Reverse Engineering Analysis

## Overview

| Property | Value |
|----------|-------|
| **File** | `AlbumDownloadProtocolHandler.dll` |
| **Original Path** | `Program Files\Windows Live\Photo Gallery\AlbumDownloadProtocolHandler.dll` |
| **Description** | Windows Live Photo Gallery Album Download Protocol Handler |
| **CLSID** | `{E43EF6CD-A37A-4A9B-9E6F-83F89B8E6324}` |
| **TypeLib** | `{60E1FA84-4F2F-417C-AEE4-7681A960D09E}` |
| **Scheme** | `wlalbumdownload:` |
| **Type** | Native C++ (VC++ 2012 / MSVCR110.dll) |
| **Subsystem** | Windows GUI |
| **Image Base** | `0x10000000` |
| **Size** | ~45 KB (0xB000) |
| **PDB** | `AlbumDownloadProtocolHandler.pdb` |
| **Timestamp** | 2014-04-01 (build 16.4.3528.0331) |
| **Company** | Microsoft Corporation |
| **Product** | Photo Gallery |

## COM Architecture

The DLL exports four standard COM self-registration entry points:

| Export | RVA | Description |
|--------|-----|-------------|
| `DllCanUnloadNow` | `0x22C0` | Reference-counted unload check via `CComModule` |
| `DllGetClassObject` | `0x22D9` | Delegates to ATL class factory (`0x10003129`) |
| `DllRegisterServer` | `0x22E9` | Registers CLSID via ATL Registrar with embedded `.rgs` |
| `DllUnregisterServer` | `0x2325` | Unregisters CLSID |

### COM Class: `AlbumDownloadLauncher`

Implements three interfaces (ATL `CComObject` with composite vtable):

| Interface | RTTI Name | Role |
|-----------|-----------|------|
| `IInternetProtocolRoot` | `?AUIInternetProtocolRoot@@` | Base protocol handler |
| `IInternetProtocol` | `?AUIInternetProtocol@@` | Read/stream data |
| `IInternetProtocolInfo` | `?AUIInternetProtocolInfo@@` | URL parsing/combination |

ATL base classes:
- `CComObjectRootEx<CComMultiThreadModelNoCS>` — thread safety
- `CComCoClass<AlbumDownloadLauncher, &GUID_AlbumDownloadLauncher>` — class factory
- `CComObject<AlbumDownloadLauncher>` — COM object instantiation

## Dependencies

| DLL | Functions Used | Purpose |
|-----|---------------|---------|
| **MSVCR110.dll** | `malloc`, `free`, `memset`, `wcscat_s`, `wcscpy_s`, `wcsncpy_s`, `wcsstr`, `memcpy_s`, `_CxxThrowException` | VC++ 2012 runtime |
| **SHELL32.dll** | `ShellExecuteW` | Launch URL/process |
| **WLXPhotoBase.dll** | `BasePrivate::New`, `BasePrivate::Delete` | Memory allocation |
| **msi.dll** (ord. 90) | `MsiGetShortcutTargetW` | Windows Installer component lookup |
| **ADVAPI32.dll** | `RegCreateKeyExW`, `RegOpenKeyExW`, `RegSetValueExW`, `RegEnumKeyExW`, `RegQueryInfoKeyW`, `RegDeleteKeyW`, `RegDeleteValueW`, `RegCloseKey` | Registry self-registration |
| **OLEAUT32.dll** | Type library registration (ord. 2, 6, 7, 161, 163, 186, 277) | TypeLib install/uninstall |
| **ole32.dll** | `CoCreateInstance`, `CoTaskMemAlloc/Free/Realloc`, `StringFromGUID2` | COM infrastructure |
| **USER32.dll** | `CharNextW` | Character iteration |
| **KERNEL32.dll** | Standard memory/file/resource functions | OS primitives |

## Protocol Handler Behavior

### Registration

The embedded `.rgs` resource registers under:

```
HKLM\Software\Classes\CLSID\{E43EF6CD-A37A-4A9B-9E6F-83F89B8E6324}
  @ = "Windows Live Photo Gallery Album Downloader Protocol Handler"
  InprocServer32 = %MODULE%
    ThreadingModel = Both
  TypeLib = {60E1FA84-4F2F-417C-AEE4-7681A960D09E}
    Version = 1.0
```

The `wlalbumdownload:` scheme is registered as an asynchronous pluggable protocol under `HKLM\Software\Classes\PROTOCOLS\Handler`.

### `IInternetProtocol::Start` (`0x100026B5`)

1. Validates `pdwResult` parameter is non-null
2. Clears `*pdwResult = 0`
3. Checks BINDINFO structure fields:
   - `cbSize == 0`
   - `szExtraInfo == NULL`
   - `grfFlags == 0xC0` (BINDF_ASYNCHRONOUS | BINDF_ASYNCSTORAGE | BINDF_PULLDATA)
   - `dwCodePage == 0x460000` (UTF-8? or CP_UNICODE?)
4. If BINDINFO doesn't match, returns `INET_E_DEFAULT_ACTION` (`0x80040110`) — falls back to default handler
5. Otherwise calls internal dispatch at vtable+0x24

### Internal URL Handler (`0x100020F8`)

This is the core logic:

1. **Stores the URL** into the object at offset `+0x14`
2. **Calls `msi!ord90`** (`MsiGetShortcutTargetW`) with:
   - Component path string: `L"Download"` (from `0x100013C0`)
   - Output buffer (260 WCHARs)
   - Size pointer
3. **If MSI lookup returns `ERROR_PATH_NOT_FOUND` (3)**:
   - Calls `ShellExecuteW(NULL, NULL, buffer, URL, &size, SW_SHOWNORMAL)`
   - Uses the looked-up path as the executable (or launch target) and passes the URL as a parameter
4. **If MSI lookup succeeds** (component installed):
   - Calls `IInternetProtocol::Terminate` on itself
   - Returns `S_OK`

### `IInternetProtocolInfo::ParseUrl` (`0x100021E1`)

Partial implementation:
- For `PARSE_SECURITY_URL` (param 7): validates URL is non-empty and ≤ 0x46 chars (70)
- For `PARSE_SCHEMA` (param 0xC/D): returns `INET_E_DEFAULT_ACTION` 
- Other operations: returns `INET_E_DEFAULT_ACTION`

### Other IInternetProtocol Methods

| Method | RVA | Behavior |
|--------|-----|----------|
| `Read` | `0x10002196` | Returns `E_NOTIMPL` (`0x80004001`) |
| `Seek` | `0x100021A3` | Returns `E_NOTIMPL` |
| `LockRequest` | `0x100021B0` | Returns `E_NOTIMPL` |
| `UnlockRequest` | `0x100021BD` | Returns `S_OK` |
| `Continue` | `0x10002708` | Delegates to `0x100024C9` |
| `Abort` | `0x10002571` | Destructor + clean up |
| `Terminate` | `0x10001EE8` | Delegates to ATL module shutdown |

## String References

### Unicode Strings (from `.text`)

| Address | String | Context |
|---------|--------|---------|
| `0x1000134C` | `CLSID` | ATL registrar registry key |
| `0x10001358` | `AppID` | ATL registrar |
| `0x10001378` | `ForceRemove`, `NoRemove` | .rgs parsing |
| `0x100013B0` | `Type` | MSI component/feature? |
| `0x100013C0` | `Download` | MSI component qualifier |
| `0x100013E8` | `Version` | TypeLib version |

### ASCII Strings (from `.rdata`)

| Address | String | Context |
|---------|--------|---------|
| `0x10007480` | `AlbumDownloadProtocolHandler.dll` | Module name |
| `0x100074A0` | `DllCanUnloadNow` | Export names |
| `0x100074B0` | `DllGetClassObject` | Export names |
| `0x100074C0` | `DllRegisterServer` | Export names |
| `0x100074D0` | `DllUnregisterServer` | Export names |

## Data Flow

```
Browser navigates to: wlalbumdownload:{GUID}/some/path
                          │
                          ▼
    Urlmon.dll resolves wlalbumdownload: handler
                          │
                          ▼
    COM creates AlbumDownloadLauncher instance
                          │
                          ▼
    IInternetProtocol::Start(szUrl, ...) called
                          │
                          ▼
    Internal handler (0x100020F8):
      ├─ Check MSI for component "Download"
      │   ├─ Found? → Terminate protocol (no-op)
      │   └─ Not found? → ShellExecuteW with URL as parameter
      └─ Return
```

## Test Harness

See `tests/OtherDlls/AlbumDownloadProtocolHandler_test.cpp` for a COM test harness.

### Test Results

The test harness compiles and links against ole32.lib, oleaut32.lib, and urlmon.lib. At runtime:

- `LoadLibrary` succeeds when the Photo Gallery directory is in the DLL search path (providing `WLXPhotoBase.dll`)
- `DllGetClassObject` returns `E_UNEXPECTED` (`0x8000FFFF`) because the ATL module object map global is not populated outside the normal Photo Gallery runtime context
- The DLL requires the VC++ 2012 CRT (`MSVCR110.dll`) and its full dependency chain to function correctly

To run the test properly, the DLL must be in a directory with all Photo Gallery dependencies, and ideally be loaded as part of the Photo Gallery application context where the ATL module is initialized.

## Notable Observations

1. **Not .NET** — despite being in a managed-code product suite, this DLL is pure native C++ using ATL 7.0/Visual C++ 2012.
2. **Minimal implementation** — most `IInternetProtocol` methods return `E_NOTIMPL` or `INET_E_DEFAULT_ACTION`. The handler is essentially a launcher, not a data streamer.
3. **MSI integration** — checks via `MsiGetShortcutTargetW` whether Photo Gallery is installed before deciding action.
4. **ShellExecuteW fallback** — if the component isn't found, it shells out, which would trigger a "download this software" flow or pass the URL to the default browser.
5. **wlalbumdownload: scheme** — handles URIs like `wlalbumdownload:{album-guid}?...` used by Windows Live products for album download operations.
