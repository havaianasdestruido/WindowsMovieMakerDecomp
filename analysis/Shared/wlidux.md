# wlidux.dll - Static Analysis

## Overview
**wlidux.dll** (Windows Live ID UX) is a **large COM-based DLL** implementing the Windows Live ID (Microsoft Account) sign-in user interface. It provides the modal and modeless sign-in dialogs, password entry, email selection, CAPTCHA/HIP challenge, single-use code authentication, and user status display. Built on the **DirectUI** (UXCore) framework for rich visual dialogs.

**PDB:** `wlidux.pdb` (GUID: `{DFF384A0-E55F-436E-B388-1C56B2AAA74B}`)

**Version:** 16.4.3528.0331 (Windows Essential / Windows Live 2013 wave)

**File Size:** 2,785,472 bytes (~2.7 MB) - the largest shared DLL

## PE Header
| Field | Value |
|-------|-------|
| Machine | x86 (0x14C) |
| Linker | MSVC 11.00 (VS2012) |
| Timestamp | 2014-04-01 01:26:46 UTC |
| Image Base | 0x10000000 |
| Size of Image | 0x2A7000 (~2.6 MB) |
| Entry Point | 0x15C0 |
| Subsystem | Windows GUI (2) |
| DLL Characteristics | Dynamic base, NX compatible |

## Sections
| Name | VirtAddr | VirtSize | RawSize | Flags |
|------|----------|----------|---------|-------|
| .text | 0x1000 | 0x15EC2 | 0x16000 | Code, Execute Read |
| .data | 0x17000 | 0x8B8 | 0x600 | Initialized Data, Read Write |
| .rsrc | 0x18000 | 0x28C0E8 | 0x28C200 | Initialized Data, Read Only |
| .reloc | 0x2A5000 | 0x1408 | 0x1600 | Discardable, Read Only |

**Critical observation**: The `.rsrc` section is **0x28C0E8 bytes** (~2.6 MB) - 99.5% of the DLL is resources. The code section is only ~90 KB.

## Exports
| Ordinal | Hint | RVA | Name |
|---------|------|-----|------|
| 1 | 0 | 0x7050 | `??0CSigninDialog@@QAE@XZ` (CSigninDialog constructor) |
| 2 | 1 | 0xD5FA | `DllCanUnloadNow` |
| 3 | 2 | 0xD60E | `DllGetClassObject` |
| 4 | 3 | 0x17B0 | `DllMain` |
| 5 | 4 | 0xD62A | `DllRegisterServer` |
| 6 | 5 | 0xD637 | `DllUnregisterServer` |
| 7 | 6 | 0x7008 | `WlidUxCreateObject` |
| 8 | 7 | 0x2ADD | `WlidUxInitProcess` |
| 9 | 8 | 0x18A9 | `WlidUxUninitProcess` |

### Key Exported Functions
| Function | Purpose |
|----------|---------|
| `WlidUxInitProcess` | Process-level initialization (UXCore, COM, etc.) |
| `WlidUxUninitProcess` | Process-level cleanup |
| `WlidUxCreateObject` | Factory - creates sign-in dialog objects |
| `??0CSigninDialog@@QAE@XZ` | CSigninDialog default constructor (C++ mangled) |

The 4 standard COM DLL exports (`DllCanUnloadNow`, `DllGetClassObject`, `DllRegisterServer`, `DllUnregisterServer`) confirm this is an **in-process COM server**.

## Imports

### MSVCR110.dll (Visual C++ 2012 Runtime)
Standard CRT plus extensive C++ support: `operator new/delete/new[]/delete[]`, `wcscat_s`, `wcscpy_s`, `wcsncpy_s`, `wcsstr`, `wcschr`, `wcscspn`, `wcsspn`, `wcsnlen`, `wcstol`, `_wcsicmp`, `_vsnwprintf`, `calloc`, `malloc`, `realloc`, `_recalloc`, `free`, `qsort`, `iswspace`, `strnlen`, `memcpy_s`, `memmove`, `memmove_s`, `memset`, `_purecall`, `_callnewh`.

### KERNEL32.dll
Rich set including:
- **Resource loading**: `FindResourceExW`, `FindResourceW`, `LoadResource`, `SizeofResource`, `LockResource`, `LoadLibraryExA`
- **Thread management**: `CreateThread`, `TlsGetValue`, `GetCurrentThreadId`
- **Synchronization**: `CreateEventW`, `WaitForSingleObject`, `SetEvent`, `CloseHandle`
- **Locale**: `LCIDToLocaleName`, `GetThreadLocale`, `SetThreadLocale`, `GetUserDefaultUILanguage`, `GetThreadUILanguage`, `CompareStringW`
- **Heap**: `HeapAlloc`, `HeapReAlloc`, `HeapFree`, `HeapSize`, `HeapDestroy`, `GetProcessHeap`
- **Module**: `GetModuleFileNameW`, `GetModuleHandleW`, `GetProcAddress`, `FreeLibrary`
- **String**: `WideCharToMultiByte`, `FormatMessageW`, `lstrcmpiW`
- **Security**: `IsDebuggerPresent`, `IsProcessorFeaturePresent`, `QueryPerformanceCounter`
- **Misc**: `RaiseException`, `GetTickCount`, `GetTickCount64`, `GetSystemTimeAsFileTime`
- **Critical Section**: `InitializeCriticalSectionAndSpinCount`, `EnterCriticalSection`, `LeaveCriticalSection`, `DeleteCriticalSection`
- **Interlocked**: `InterlockedIncrement`, `InterlockedDecrement`, `InterlockedExchange`
- **Atom**: `FindAtomW`

### USER32.dll (Window Management)
`GetClassNameW`, `GetWindowLongW`, `MessageBoxIndirectW`, `SetWindowTextW`, `CharNextW`, `GetMenuItemInfoW`, `TrackPopupMenuEx`, `GetProcessDefaultLayout`, `CreatePopupMenu`, `DestroyMenu`, `DestroyIcon`, `SendMessageW`, `GetKeyState`, `MsgWaitForMultipleObjectsEx`, `SetWindowPos`, `IsWindow`, `PostMessageW`, `PeekMessageW`, `DispatchMessageW`, `TranslateMessage`, `SystemParametersInfoW`, `AppendMenuW`, `SetProcessDefaultLayout`

### ADVAPI32.dll (Registry & ETW Tracing)
- **Registry**: `RegQueryInfoKeyW`, `RegDeleteKeyW`, `RegGetValueW`, `RegCloseKey`, `RegOpenKeyExW`, `RegOpenCurrentUser`, `RegQueryValueExW`, `RegSetValueExW`
- **ETW Tracing**: `RegisterTraceGuidsW`, `UnregisterTraceGuids`, `GetTraceLoggerHandle`, `GetTraceEnableLevel`, `GetTraceEnableFlags`

### ole32.dll (COM)
`CoCreateInstance`, `CoUninitialize`, `CoInitialize`, `StringFromGUID2`

### OLEAUT32.dll (OLE Automation, by ordinal)
Ordinals 186, 163, 161, 7, 6, 2 (VariantClear, SysAllocStringLen, SysAllocString, SysStringLen, SysFreeString, SysReAllocString)

### SHLWAPI.dll
`SHCreateStreamOnFileW`, `StrRChrW`

### UXCore.dll (DirectUI Framework) - **Dominant Import**
This is the heaviest import (~150+ symbols) providing the entire DirectUI/UXCore framework:

| Category | Key Imports |
|----------|-------------|
| **Process/Thread** | `UXCoreInitProcess`, `UXCoreUnInitProcess`, `UXCoreInitThread`, `UXCoreUnInitThread` |
| **Dialog Base** | `CDUIDialog` (constructor, destructor, DoModal, DoModeless, ShowDialog, OnActivate, OnClose, OnCommand, OnCreate, OnInitDialog, OnSize, etc.) |
| **Frameless Host** | `CFramelessHost` (OnCreateRegion, OnDefaultFrameColorChanged, OnUpdateFrame) |
| **URL Download** | `CUrlDownload` (constructor, Abort, SetDownloadCompleteSite, RevokeDownloadCompleteSite) |
| **Image** | `CRMImage` (Load from bytes/strings, ConvertToARGB, Detach, GetBPP, GetHeight, IsNull) |
| **String Resource** | `CRMStringResource` (Load, Length, ToString) |
| **DUI Parser** | `CRMDUIParser` (constructor, destructor) |
| **Element** | `Element` (GetValue, SetValue, FindDescendent, FireEvent, QIBehaviors, AddHandler, StartDefer, EndDefer) |
| **Value System** | `Value` (CreateInt, CreateBool, CreateString, CreateSize, CreateGraphic, CreateColorize, CreateUnknown, _ZeroRelease) |
| **HWNDElement** | NativeHWNDHost wrapper |
| **Widget Classes** | `Button`, `Checkbox` (CheckedProp, StateChange, UpdateStateProp), `Combobox` (SelectionProp, SelectionChange, AddString, GetSelectedString), `WLEditT` (SetMaxTextLength) |
| **Properties** | Background, Content, Active, Selected, Visible, Height, Enabled, Parent, Cursor, Direction, Accessible, AccName, AccDesc, AccValue, Tooltip, CmdContext, MinSize, IDProp, ColorizeProp, ExecCmd |
| **Layer Manager** | `SetGadgetStyle`, `GetGadgetRect`, `SetElementPropertyFromImage`, `DuiGetLayerManager`, `InvalidateElement`, `GetLayer` |
| **Global State** | `g_dwElSlot`, `StrToID`, `pvUnset` |
| **Misc** | `RMUpdateResourceSet`, `GetTopHWNDParent` |

### SHELL32.dll
`ShellExecuteW` - for launching URLs, help pages, etc.

### PSAPI.DLL
`GetModuleBaseNameW` - for process identification

### wlidcli.dll (Windows Live ID Client) - **by ordinal**
| Ordinal | Likely Function |
|---------|----------------|
| 2, 3, 4, 5, 7, 8 | Core auth operations (sign-in, sign-out, token management) |
| 16, 17, 18, 19 | Account management |
| 23, 29 | Token/credential operations |
| 55, 56 | Extended auth flows |
| 112, 115 | Single-use code / federated auth |

### WLDCore.dll - **by ordinal**
| Ordinal | Likely Function |
|---------|----------------|
| 43 | Windows Live Data Core service call |

### Delay-Loaded Import
| DLL | Function |
|-----|----------|
| WINMM.dll | `PlaySoundW` - for notification/error sounds in the sign-in dialog |

## RTTI Class Hierarchy (from exports and imports)

The DLL reveals a sophisticated UI dialog framework:

### Core Dialog Classes
| Class | Description |
|-------|-------------|
| `CSigninDialog` | Main sign-in dialog (exported constructor) |
| `CDUIDialog` | Base dialog class (DoModal, DoModeless, ShowDialog, message handling) |
| `CFramelessHost` | Frameless window hosting (region creation, frame color) |

### UI Widget Classes (DirectUI)
| Class | Description |
|-------|-------------|
| `Element` | Base UI element with property system |
| `HWNDElement` | HWND-backed element |
| `Button` | Clickable button |
| `Checkbox` | Toggle with checked state |
| `Combobox` | Dropdown selector (for email selection) |
| `WLEditT` | Text input (password, email) |
| `DialogHost` | Dialog container |

### Resource Classes
| Class | Description |
|-------|-------------|
| `CRMImage` | Image loading/manipulation |
| `CRMStringResource` | Localized string loading |
| `CRMDUIParser` | DirectUI XML parser |
| `CUrlDownload` | HTTP download for HIP images |

## UI Element IDs (from string data)

The DLL contains extensive DirectUI element ID strings mapping to the sign-in dialog:

### Email Input Flow
`idEmailInput`, `idEmailInputContainer`, `idEmailCombobox`, `idEmailComboboxContainer`, `idEmailFlyout`, `idEmailHint`

### Password Flow
`idPasswordInput`, `idPasswordHint`

### Sign-In Controls
`idSignIn`, `idRememberMe`, `idAutoSignin`, `idCancel`, `idOk`

### Status/Mode
`idSigninDialogScene`, `idModeSwitchContainer`, `idStatusSelector`, `idSigninStatusChicklet`, `idSigninStatusText`, `idStatusFlyoutMenu`

### CAPTCHA/HIP
`idHipImage`, `idHipInput`, `idAudio`, `HipDialog`, `UxHipDialogClass`

### Error Handling
`idInlineError`, `idInlineErrorText`, `idInlineErrorContainer`, `idFederatedWarningText`, `idFederatedWarningContainer`

### Links
`idLinkHelp`, `idLinkForgotPassword`, `idLinkPrivacy`, `idLinkTOS`, `idLinkServerStatus`, `idLinkOptions`, `idLinkSignup`, `idLinkHelpMenu`, `idSingleUseCodeLink`, `idUsernamePasswordLink`

### Layout Groups
`idHeaderTextGroup`, `idControlsGroup`, `idMainContent`, `idSigninDialogCore`, `idSignupLinkContainer`, `idSingleUseHeader`, `idSingleUseCodeControls`, `idUsernamePasswordControls`, `idUsernamePasswordHeader`, `idForgetMeContainer`

### CAPTCHA Warning
`idCapsLockIcon`

## Sign-In Dialog Scene Graph
```
SigninDialogRoot (root)
  +-- idSigninDialogScene
  +-- idMainContent
  |     +-- idHeaderTextGroup
  |     |     +-- idSigninAppLabel
  |     |     +-- idDescriptiveText
  |     +-- idControlsGroup
  |     |     +-- idEmailInputContainer
  |     |     |     +-- idEmailInput (WLEditT)
  |     |     |     +-- idEmailComboboxContainer
  |     |     |           +-- idEmailCombobox (Combobox)
  |     |     +-- idPasswordInput (WLEditT)
  |     |     +-- idCapsLockIcon
  |     |     +-- idUsernamePasswordControls
  |     |     +-- idSingleUseCodeControls
  |     +-- idInlineErrorContainer
  |     |     +-- idInlineErrorText
  |     +-- idFederatedWarningContainer
  |           +-- idFederatedWarningText
  +-- idSignupLinkContainer
  |     +-- idSignupLinkContainer
  +-- idLinkOptionsContainer
  |     +-- idLinkHelp
  |     +-- idLinkForgotPassword
  |     +-- idLinkPrivacy
  |     +-- idLinkTOS
  |     +-- idLinkServerStatus
  +-- idForgetMeContainer
  +-- idSigninStatusChicklet
  +-- idSigninStatusText
  +-- idModeSwitchContainer
  +-- idStatusSelector
```

## Application Integration

The DLL recognizes and integrates with multiple Windows Live applications:

| Application | Executable | Context |
|-------------|------------|---------|
| Windows Live Messenger | `msnmsgr.exe` | Primary messenger app |
| Windows Live Mail | `wlmail.exe` | Email client |
| Windows Live Photo Gallery | `WLXPhotoGallery.exe` | Photo management |
| WL Album Download Wizard | `WLXAlbumDownloadWizard.exe` | Album download |
| Windows Live Movie Maker | `MovieMaker.exe` | Video editing |
| Windows Live Sync | `WLSync.exe` | File synchronization |
| FSUI | `fsui.exe` | File save UI |
| Internet Explorer | `iexplore.exe` | Web browser |

## Sign-In Mock Data (for testing)
| Field | Value |
|-------|-------|
| Mock Email | `foo@bar.com`, `fake@example.com`, `example555@hotmail.com`, `food@network.com`, `foobar@live.com` |
| Registry Key | `SigninDialogUseMockData` |
| Mock State | `DefaultSignInState` |

## Locale Support
The DLL contains locale strings for 50+ locales including:
`en-gb`, `en-us`, `pt-br`, `pt-pt`, `zh-cn`, `zh-tw`, `ja-ploc-jp`, `ko-kr`, `de-de`, `fr-fr`, `es-es`, `it-it`, `ru-ru`, `ar-sa`, `he-il`, `th-th`, `vi-vn`, `pl-pl`, `cs-cz`, `hu-hu`, `ro-ro`, `bg-bg`, `hr-hr`, `sk-sk`, `sl-si`, `et-ee`, `lv-lv`, `lt-lt`, `uk-ua`, `tr-tr`, `el-gr`, `fi-fi`, `nb-no`, `sv-se`, `da-dk`, `nl-nl`, `ca-es`, `eu-es`, `gl-es`, `id-id`, `ms-my`, `fil-ph`, `hi-in`, `bn-bd`, `ta-in`, `te-in`, `mr-in`, `gu-in`, `kn-in`, `ml-in`, `si-lk`, `my-mm`, `km-kh`, `lo-la`, `ka-ge`, `hy-am`, `az-latn-az`, `uz-latn-uz`, `kk-kz`, `ky-kg`, `tg-cyrl`, `tk-tm`, `mn-mn`, `ps-af`, `fa-ir`, `ur-pk`, `sw-ke`, `am-et`, `ha-latn-ng`, `yo-ng`, `zu-za`, `af-za`, `mt-mt`, `cy-gb`, `ga-ie`, `is-is`, `mk-mk`, `sq-al`, `sr-cyrl-BA`, `bs-latn-ba`, `nn-no`, `chr-Cher`, `iu-latn-ca`, `mi-nz`, `xh-za`

## Digital Signatures
Signed by Microsoft Corporation with multiple certificate chains:
1. Microsoft Code Signing PCA (2011)
2. Microsoft Time-Stamp PCA 2010
3. Microsoft Root Certificate Authority 2011

nCipher DSE ESN: `7D2E-3782-B0F7`

## Architecture
```
Windows Live Applications (wlarp.exe, etc.)
  |
  | WlidUxInitProcess() -> WlidUxCreateObject()
  v
wlidux.dll
  |
  +-- CSigninDialog (CDUIDialog)
  |     +-- DirectUI Scene Graph (from .rsrc)
  |     +-- Email Input (WLEditT / Combobox)
  |     +-- Password Input (WLEditT)
  |     +-- CAPTCHA/HIP (CUrlDownload -> idHipImage)
  |     +-- Status Display
  |
  +-- wlidcli.dll (Windows Live ID Client)
  |     +-- Authentication, tokens, OAuth
  |
  +-- WLDCore.dll (Windows Live Data Core)
  |     +-- Service communication
  |
  +-- UXCore.dll (DirectUI Framework)
  |     +-- UI rendering, layout, input handling
  |
  +-- WLAVRes.dll (Resource strings)
  |     +-- Localized UI text
  |
  +-- wliduxloc.dll (Localized resources)
        +-- MUI strings per locale
```

## Key Observations
1. **Largest shared DLL at 2.7 MB** - 99.5% is resources (.rsrc = 2.6 MB)
2. **Complete sign-in UX** for Microsoft Account / Windows Live ID
3. **DirectUI-based** - heavy dependency on UXCore.dll for visual rendering
4. **Multiple auth modes**: email+password, single-use code, federated, CAPTCHA/HIP
5. **50+ locale support** in the embedded MUI resources
6. **6 app integrations** - shared across all Windows Live 2013 desktop apps
7. **ETW tracing** via ADVAPI32 `RegisterTraceGuidsW` for diagnostics
8. **Registry-based** config: `SOFTWARE\Microsoft\IdentityCRL`, `SOFTWARE\Microsoft\Windows Live\Common`
9. **Mock data** built-in for testing sign-in flows
10. **9 exported functions** including C++ mangled constructor
11. **Delay-loaded WINMM.dll** for optional sound effects
12. **PlaySoundW** for notification/error audio feedback
13. **Same build timestamp** as WLAVRes.dll (2014-04-01) - same release
14. **MSVC 2012** toolchain (linker 11.00)

---

# wliduxloc.dll - Satellite Localization Resource DLL

## Overview
**wliduxloc.dll** is a **pure resource-only satellite DLL** providing localized MUI (Multilingual User Interface) resources for `wlidux.dll`. It contains **no code, no exports, no imports** - only a single `.rsrc` section with language-specific string tables.

**File Size:** 27,840 bytes

## PE Header
| Field | Value |
|-------|-------|
| Machine | x86 (0x14C) |
| Linker | MSVC 11.00 (VS2012) |
| Timestamp | 2014-04-01 01:27:42 UTC |
| Image Base | 0x10000000 |
| Size of Image | 0x4000 (16 KB) |
| Entry Point | 0x0 (none) |
| Subsystem | Windows GUI (2) |
| DLL Characteristics | Dynamic base, NX compatible, **No structured exception handler** (0x540) |

## Sections
| Name | VirtAddr | VirtSize | RawSize | Flags |
|------|----------|----------|---------|-------|
| .rsrc | 0x1000 | 0x2A18 | 0x2C00 | Initialized Data, Read Only |

**Single section only.** No `.text`, `.data`, or `.reloc` sections.

## Exports / Imports
**None.** Zero exports, zero imports. This DLL has no code whatsoever.

## Resource Content

The `.rsrc` section contains MUI string tables organized by locale. The resource tree structure:

### MUI Resource Data
The resources are organized as:
- **String Table** (type 6) with multiple language-specific entries
- Primary language: `en-us` (0x0409)
- Resource ID: `MUI` (0x0409)

### UI Strings (English baseline)
From the resource data, the DLL contains localized versions of:
- "Select &all" / "Select &A ll"
- "Show menu" / "Shows the menu bar"
- "Hide the menu bar"
- "Shows the menu bar at the top of the screen"
- "Click to view next month" / "Go to next month"
- "Click to view previous month" / "Go to last month"
- "Month and Year shown."
- "Mini-Month picker"
- "Switch to month view" / "Switch to year view"
- Calendar navigation strings
- Scroll bar labels ("Scroll left", "Scroll right", "Line up", etc.)
- Color picker labels ("Default color", "More Colors...")
- Date/time picker strings ("the day before yesterday", "yesterday", "today", etc.)
- "Less", "Position", "More"
- "Press" (keyboard prompt)

## Architecture
```
wlidux.dll (main sign-in UI)
  |
  | LoadLibrary("wliduxloc.dll")
  v
wliduxloc.dll (MUI satellite)
  |
  +-- .rsrc: String tables per locale
        +-- en-us (primary)
        +-- Other locales via MUI fallback
```

The MUI framework (via `Kernel32!FindResourceExW`) locates the appropriate string table based on the user's UI language. If the requested locale is not present in wliduxloc.dll, it falls back to the primary language.

## Key Observations
1. **Zero code DLL** - only a `.rsrc` section, no `.text` section
2. **No entry point** (RVA 0x0) - purely a data container
3. **Satellite pattern** - loaded alongside wlidux.dll for localization
4. **MUI-compliant** - proper Multilingual User Interface resource structure
5. **Lightweight** at 27 KB - contains only the strings that differ across locales
6. **Same build timestamp** as wlidux.dll (2014-04-01, within 1 minute)
7. **No CRT dependency** - since there's no code, there are no CRT imports
8. **No SEH** - DLL characteristic `No structured exception handler` (0x540)
9. **Language fallback** handled by Windows MUI infrastructure
10. **Complementary to WLAVRes.dll** - WLAVRes has shared strings, this has locale-specific overrides
