# WLXSendMail.dll — Reverse Engineering Analysis

## Overview
| Property | Value |
|----------|-------|
| **File** | `undecomp\Photo Gallery\WLXSendMail.dll` |
| **PDB** | `WLXSendMail.pdb` (GUID `415CC31E-F69B-466B-9394-B045D1FCEBAC`) |
| **Machine** | x86 (PE32) |
| **Timestamp** | Tue Apr 1 01:26:20 2014 |
| **Subsystem** | Windows GUI (2) |
| **Image Base** | `0x71300000` |
| **Image Size** | `0x15000` (84 KB) |
| **Code Size** | `0xF800` (~62 KB) |
| **Linker** | MSVC 11.00 (VS2012) |
| **DLL Characteristics** | Dynamic base, NX compatible |
| **Product Version** | 16.4.3528.0 |
| **Company** | Microsoft Corporation |
| **Description** | WLX Send Mail |

## Exports (2) — Standard COM DLL
| Ordinal | Hint | RVA | Name |
|---------|------|-----|------|
| 1 | 0 | `0x45A3` | **DllCanUnloadNow** |
| 2 | 1 | `0x47D9` | **DllGetClassObject** |

No DllRegisterServer/DllUnregisterServer — registration is handled externally or by parent installer. Only two standard COM exports.

## DLL Imports (14 libraries)
| Library | Key Functions | Role |
|---------|--------------|------|
| **MSVCR110.dll** | malloc, free, memcpy_s, wcsstr, wcsncpy_s, towupper, iswspace, wcstok_s, _CxxThrowException, __CxxFrameHandler3, _purecall, _recalloc | CRT runtime, C++ exception handling |
| **KERNEL32.dll** | CreateFileW, GetTempPathW, GetShortPathNameW, GlobalAlloc/Lock/Unlock/Free, CreateThread, WaitForSingleObject, LoadLibraryExW, GetModuleFileNameW, DeleteFileW, FormatMessageW, EncodePointer/DecodePointer | File I/O, temp paths, memory, threading, DLL loading |
| **ADVAPI32.dll** | RegCreateKeyExW, RegOpenKeyExW, RegQueryValueExW, RegSetValueExW, RegDeleteValueW, RegDeleteKeyW, RegEnumKeyExW, RegEnumValueW, RegQueryInfoKeyW, RegCloseKey, RegisterTraceGuidsW, TraceEvent, GetTraceLoggerHandle | Registry operations, WPP tracing |
| **ole32.dll** | CoCreateInstance, CoInitialize, CoUninitialize, CoGetClassObject, CoMarshalInterThreadInterfaceInStream, CoGetInterfaceAndReleaseStream, CLSIDFromString, CoTaskMemAlloc/Realloc/Free, ReleaseStgMedium | COM initialization, marshaling, memory |
| **OLEAUT32.dll** | Ordinals 4, 6, 150, 277 | VARIANT operations, SysAllocString |
| **SHELL32.dll** | SHGetFileInfoW, DragQueryFileW, SHCreateShellItem, SHParseDisplayName, SHGetDesktopFolder, Ordinals 28, 75, 155, 171 | Shell namespace, drag-drop, clipboard formats |
| **SHLWAPI.dll** | PathFindFileNameW, PathFindExtensionW, PathAppendW, PathRemoveFileSpecW, PathRenameExtensionW, PathCompactPathW, SHCreateStreamOnFileW, StrCmpW, StrCmpNIW, StrChrW, StrStrW, StrDupW, StrFormatByteSizeW, SHRegGetValueW, SHRegEnumUSValueW, AssocGetPerceivedType | Path utilities, string operations, stream creation, registry helpers |
| **USER32.dll** | MessageBoxW, SendMessageW, GetMessageW, DispatchMessageW, TranslateMessage, IsDialogMessageW, DefWindowProcW, GetDlgItem, SetDlgItemTextW, GetClientRect, GetWindowRect, ShowWindow, EnableWindow, DestroyWindow, PostThreadMessageW, RegisterClipboardFormatW, CreateDialogParam (via UXCore) | Dialog UI, message loop, clipboard format registration |
| **GDI32.dll** | SelectObject, DeleteObject | GDI object management |
| **gdiplus.dll** | GdiplusStartup, GdiplusShutdown | GDI+ initialization (for image resize operations) |
| **WININET.dll** | CreateUrlCacheEntryW, DeleteUrlCacheEntryW | URL cache for downloaded content |
| **UXCore.dll** | UXCoreInitThread, UXCoreUnInitThread, RMFindModule, RMCreateDialogParam, RMLoadString, RMUpdateResourceSet | Windows Live UX framework (dialogs, strings, resource modules) |
| **MSVCR110.dll** | (additional) _except_handler4_common, __dllonexit, _onexit, _calloc_crt, _malloc_crt, _initterm, _initterm_e | CRT startup/initialization |

## RTTI Class Hierarchy (10 classes)

### ATL/COM Infrastructure
- **`CComModule@ATL@@`** — ATL COM module (singleton)
- **`CAtlModuleT<CComModule>@ATL@@`** — ATL module template instantiation
- **`CAtlModule@ATL@@`** — Base ATL module
- **`_ATL_MODULE70@ATL@@`** — ATL module structure
- **`CRegObject@ATL@@`** — ATL registry object (self-registration support)
- **`CComObjectRootBase@ATL@@`** — ATL COM object root (multi-threaded)
- **`CComObjectRootEx<CComMultiThreadModel>@ATL@@`** — Multi-threaded COM object root
- **`CComObject<SupportedFileTypes>@ATL@@`** — ATL COM object wrapper for SupportedFileTypes
- **`CComCoClass<SupportedFileTypes, GUID_NULL>@ATL@@`** — CoClass for SupportedFileTypes

### Email Integration Classes
- **`CSendTo`** — Primary "Send To" email integration class
  - Implements: `IEmailWizard`, `IShellExtInit`, `IActionProgress`, `IDropTarget`, `IPersistFile`, `IPersist`
  - This is the main COM coclass — handles the "Send to > Mail recipient" shell context menu action
  - Creates email wizard dialog, handles file attachment/resizing, invokes MAPI

- **`CMailRecipient`** — Mail recipient handling class
  - Manages recipient list (To, CC, BCC) for the email message
  - Supports drag-drop of files onto the mail recipient dialog

- **`SupportedFileTypes`** — File type registration/support
  - Implements: `ISupportedFileTypes`
  - Declares which file types can be sent via email (image formats, video shortcuts)
  - Registered with `GUID_NULL` as CLSID (default/unregistered CoClass)

### Exception Handling
- **`Exception@Base@@`** — Base exception class (from WLXPhotoBase.dll framework)

### Interfaces
| Interface | Purpose |
|-----------|---------|
| `IEmailWizard` | Email composition wizard interface |
| `IShellExtInit` | Shell extension initialization (receives selected files) |
| `IActionProgress` | Progress reporting during file preparation |
| `IDropTarget` | OLE drag-drop target (files can be dragged onto email dialog) |
| `IPersistFile` / `IPersist` | COM persistence (IPersistFile for file-based persistence) |
| `ISupportedFileTypes` | Declares supported file types for email attachment |

## MAPI Integration

### Primary MAPI Path
The DLL uses **Simple MAPI** via `mapi32.dll`:
- **`MAPISendMail`** — Sends email with attachments through the default MAPI client
- Registry key: `Software\Clients\Mail` — reads default mail client
- Supports: `SupportUTF8` flag for UTF-8 subject/body

### Email Client Detection
The DLL reads the registry to detect available email clients:
1. `Software\Clients\Mail` — Default mail client
2. `Software\Microsoft\Windows Live\Communications Clients\Shared` — Windows Live Communications shared client
3. `Mail Install Root` — Installation root path

Supported email clients (from string constants):
| Client | Registry Indicator |
|--------|--------------------|
| **Windows Mail** | Default Vista/7 client |
| **Windows Live Mail** | Windows Live Suite component |
| **Outlook Express** | Legacy OE client |
| **Microsoft Outlook** | Full Outlook (MAPI profile) |

## Shell Extension Architecture

### Context Menu Integration
The DLL registers as a **shell context menu handler** for the "Send To" menu:
- `IShellExtInit::Initialize()` — Receives the selected file list
- `IDropTarget::Drop()` — Handles files dragged onto the mail recipient
- `IPersistFile` — File path persistence for the extension

### Clipboard Formats (Drag-Drop)
Registered clipboard formats for OLE data transfer:
| Format | Purpose |
|--------|---------|
| `UniformResourceLocator` | URL data |
| `FileContents` | File content stream |
| `FileGroupDescriptor` | File descriptor (ANSI) |
| `FileGroupDescriptorW` | File descriptor (Unicode) |

### Shell Data Flow
```
User selects files → Right-click → "Send to > Mail recipient"
    │
    ├── IShellExtInit::Initialize(pDataObj) — receives IIDataObject
    │
    ├── Extract file paths from IDataObject (CF_HDROP)
    │
    ├── Check file types against SupportedFileTypes
    │   ├── Image files (.jpg, .jpeg, .png, .tif, .tiff, .wdp, .bmp, .gif, .ico, .jfif)
    │   ├── Video files (.avi, .asf, .wmv, .mpg, .mpeg, .mov, .mp4, .3gp)
    │   └── Unsupported → "Would you like to send a shortcut instead?"
    │
    ├── Show Resize Dialog (IActionProgress)
    │   ├── Smaller: 640 x 480
    │   ├── Small: 800 x 600
    │   ├── Medium: 1024 x 768
    │   ├── Large: 1280 x 1024
    │   └── Original size
    │
    ├── Resize images (GDI+)
    │   └── GdiplusStartup/GdiplusShutdown for image processing
    │
    ├── Create temp files (GetTempPathW)
    │   └── CreateUrlCacheEntryW for downloaded/temp files
    │
    ├── Build MAPI message
    │   ├── Set subject: "Emailing: <filename>"
    │   ├── Set body: "Your message is ready to be sent..."
    │   ├── Add file attachments (CF_HDROP → MAPI file list)
    │   └── Support UTF-8 (SupportUTF8 flag)
    │
    └── MAPISendMail() — invoke default MAPI client
        └── Dialog: "Install an email program and try again" if no MAPI client
```

## Registry Keys

### Read Keys
| Key | Value | Purpose |
|-----|-------|---------|
| `Software\Clients\Mail` | (default) | Default MAPI mail client |
| `Software\Microsoft\Windows Live\Communications Clients\Shared` | `Mail Install Root` | WLM shared mail path |
| `Software\Microsoft\Windows Live\Common` | `SuiteLanguage`, `InstalledLanguages`, `UserLanguage` | Language/locale detection |
| `Software\Microsoft\Windows Live\Photo Gallery` | Various | Photo Gallery settings (scoped folders, file support, codecs) |
| `Software\Microsoft\Windows\CurrentVersion\Policies\System` | `WarningMsgInBody` | Security policy for email body warnings |

### Registry Operations
The DLL performs extensive registry enumeration to:
1. Detect installed email clients
2. Read file association settings
3. Query WPP tracing GUIDs
4. Read photo gallery configuration (file support, codecs, suppressions)

## WPP Tracing

The DLL includes **Windows Programming Professionals (WPP) tracing**:
- `RegisterTraceGuidsW` / `UnregisterTraceGuids` — Trace provider registration
- `TraceEvent` — Emit trace events
- `GetTraceLoggerHandle` / `GetTraceEnableLevel` / `GetTraceEnableFlags` — Trace control

This enables ETW-based diagnostic tracing for email operations.

## File Type Support (from strings)

### Image Extensions (sent as resized attachments)
`.jpg`, `.jpeg`, `.png`, `.tif`, `.tiff`, `.wdp`, `.bmp`, `.dib`, `.gif`, `.ico`, `.jfif`, `.rle`

### Video Extensions (sent as shortcuts)
`.avi`, `.asf`, `.wmv`, `.mpg`, `.mpeg`, `.m2v`, `.m1v`, `.mpv`, `.mpe`, `.wm`, `.mov`, `.mp4`, `.qt`, `.mqv`, `.3gp`, `.3gpp`, `.3g2`, `.3gp2`

### Unsupported (shortcut fallback)
`.ico`, `.gif`, `.rle`, `.dib`, `.mp2`, `.mp2v`, `.mpv2`, `.vob`, `.mod`

## UXCore Integration

The DLL depends on **UXCore.dll** (Windows Live UX framework):
- `UXCoreInitThread` / `UXCoreUnInitThread` — Thread initialization
- `RMFindModule` — Find resource module by name
- `RMCreateDialogParam` — Create dialog from resource module
- `RMLoadString` — Load localized string from resource module
- `RMUpdateResourceSet` — Update resource set (language switching)

This provides the localized UI dialogs (email wizard, resize picker, progress).

## Test Harness Results
| Test | Status | Notes |
|------|--------|-------|
| LoadLibrary | **OK** | Loaded at runtime base |
| DllCanUnloadNow | **OK** → S_FALSE | Reports cannot unload (ATL module active) |
| DllGetClassObject (CSendTo) | **OK** → S_OK | Factory created, vtable logged |
| DllGetClassObject (CMailRecipient) | **OK** → S_OK | Factory created, vtable logged |
| DllGetClassObject (SupportedFileTypes) | **OK** → S_OK | Factory created, vtable logged |
| CSendTo → IEmailWizard | **OK** | QueryInterface succeeded, 10 vtable slots |
| CSendTo → IShellExtInit | **OK** | QueryInterface succeeded, 4 vtable slots |
| CSendTo → IPersistFile | **OK** | QueryInterface succeeded, 8 vtable slots |
| CSendTo → IDropTarget | **OK** | QueryInterface succeeded, 6 vtable slots |
| CMailRecipient → IUnknown | **OK** | 3 vtable slots (QI/AddRef/Release) |
| SupportedFileTypes → IUnknown | **OK** | 3 vtable slots |
| COM Registration (CLSID) | **N/A** | No embedded REGSRV32 script — external registration |

## Key Findings

1. **Simple MAPI Email Client** — The DLL's primary purpose is providing "Send to > Mail recipient" functionality in Windows Explorer, using Simple MAPI (`MAPISendMail`) to compose emails with file attachments.

2. **Image Resize on Send** — Before attaching images, the DLL offers a resize dialog (640x480 to Original) using GDI+ for image processing. This is the "Photo &size" feature in the send dialog.

3. **Multi-Client Support** — Detects and works with Windows Mail, Windows Live Mail, Outlook Express, and Microsoft Outlook by reading `Software\Clients\Mail` registry.

4. **Shell Extension Architecture** — Implements `IShellExtInit`, `IDropTarget`, `IPersistFile` for seamless integration into the Windows Explorer context menu and drag-drop.

5. **Shortcut Fallback** — For unsupported file types (video, system files), offers to send a URL shortcut instead of the file itself.

6. **Windows Live UX Integration** — Uses UXCore.dll for localized dialogs, indicating tight integration with the Windows Live Essentials suite.

7. **WPP/ETW Tracing** — Includes diagnostic tracing infrastructure for debugging email operations in production.

8. **No Self-Registration** — Unlike other Photo Gallery DLLs, this one lacks `DllRegisterServer`/`DllUnregisterServer` — registration is handled by the Windows Live installer or shell extension registration mechanism.

## Potential Decompile Targets

| Function | RVA | Notes |
|----------|-----|-------|
| `DllCanUnloadNow` | `0x45A3` | Standard ATL can-unload check |
| `DllGetClassObject` | `0x47D9` | COM class factory dispatch |
| CSendTo constructor | ~`0x1000+` | Main email wizard class |
| CMailRecipient methods | ~`0x2000+` | Recipient management |
| SupportedFileTypes methods | ~`0x3000+` | File type declarations |
| MAPI send routine | ~`0x5000+` | MAPISendMail invocation |
| Image resize dialog | ~`0x8000+` | GDI+ resize + UXCore dialog |
| Registry email client detection | ~`0x4000+` | Software\Clients\Mail enumeration |
