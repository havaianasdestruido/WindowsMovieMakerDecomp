# WLAVRes.dll - Static Analysis

## Overview
**WLAVRes.dll** (Windows Live AV Resources) is a **resource-only DLL** containing UI resources, string tables, menu definitions, and manifest data for the Windows Live Audio/Video components. It contains **no meaningful code** - its entire functional payload is in the `.rsrc` section.

**PDB:** `WLAVRes.pdb` (GUID: `{A0ABF56C-9452-48D9-AAFB-643FBE1D3E49}`)

**Assembly Identity:** `Microsoft.Windows.personalMedia.WLAVRes` (version 5.1.0.0, x86)

## PE Header
| Field | Value |
|-------|-------|
| Machine | x86 (0x14C) |
| Linker | MSVC 11.00 (VS2012) |
| Timestamp | 2014-04-01 01:28:32 UTC |
| Image Base | 0x10000000 |
| Size of Image | 0x5000 (20 KB) |
| Entry Point | 0x1410 |
| Subsystem | Windows GUI (2) |
| DLL Characteristics | Dynamic base, NX compatible |
| File Size | 25,288 bytes |

## Sections
| Name | VirtAddr | VirtSize | RawSize | Flags |
|------|----------|----------|---------|-------|
| .text | 0x1000 | 0xE94 | 0x1000 | Code, Execute Read |
| .data | 0x2000 | 0x354 | 0x200 | Initialized Data, Read Write |
| .rsrc | 0x3000 | 0x918 | 0xA00 | Initialized Data, Read Only |
| .reloc | 0x4000 | 0x330 | 0x400 | Discardable, Read Only |

## Exports
**None.** The export directory exists but has 0 functions and 0 names - this is a pure resource library.

## Imports

### MSVCR110.dll (CRT Bootstrap)
Only standard CRT initialization: `_onexit`, `__clean_type_info_names_internal`, `_except_handler4_common`, `__dllonexit`, `_crt_debugger_hook`, `__crtUnhandledException`, `__crtTerminateProcess`, `_calloc_crt`, `_unlock`, `_lock`, `_initterm_e`, `_initterm`, `_malloc_crt`, `free`, `_amsg_exit`, `__CppXcptFilter`.

### KERNEL32.dll
Only security/initialization APIs: `IsDebuggerPresent`, `DisableThreadLibraryCalls`, `GetTickCount64`, `GetSystemTimeAsFileTime`, `GetCurrentThreadId`, `QueryPerformanceCounter`, `DecodePointer`, `EncodePointer`, `IsProcessorFeaturePresent`.

**No UI, file, registry, or COM imports** - this DLL is purely a data container.

## Resource Content

The `.rsrc` section contains MUI (Multilingual User Interface) resource data with the following content:

### Menu Definitions
| Menu | Items |
|------|-------|
| Main Menu | `&Cut`, `&Copy`, `&Paste`, `&Delete`, `Select &all`, `Change &font`, `&Right-to-left reading order` |
| Context Menu | `Cu&t`, `&Copy`, `&Paste`, `Select &All` |
| Window Controls | `Show menu`, `Minimize`, `Maximize`, `Close` |
| Scroll Bar | `Scroll left`, `Scroll right`, `Line up`, `Line down`, etc. |

### UI Strings
| Category | Strings |
|----------|---------|
| Calendar | "the day before yesterday", "yesterday", "today", "tomorrow", "the day after tomorrow", "now", "noon", "midnight" |
| Navigation | "Click to view next month", "Go to next month", "Go to last month", "Switch to month view", "Switch to year view", "Go to today" |
| Colors | "Sky", "Twilight", "Sea", "Lime", "Sun", "Pumpkin", "Ruby", "Fuchsia", "Blush", "Violet", "Slate", "Smoke" |
| Help | "For help on the Server Status, click a topic:", "Help" |
| Status | "View all events for this day", "No events for this day" |

### Windows Live Sign-In UI Strings
| String | Context |
|--------|---------|
| "Windows Live ID" | Dialog title |
| "Don't have a Microsoft account?" | Sign-up prompt |
| "Sign up" | Link |
| "Sign in to Windows Live" | Dialog header |
| "example555@hotmail.com" | Email hint text |
| "Enter your email address" | Prompt |
| "Caps Lock is on" | Warning |
| "Sign in as:" | Label |
| "Remember my ID and password" | Checkbox |
| "Save your email address and password on this computer" | Tooltip |
| "Sign in automatically" | Checkbox |
| "Server status" | Link |
| "Check the service status of {23080}" | Help text |
| "Forgot your password?" | Link |
| "Reset your password" | Link |
| "Please verify your account" | Error |

### URLs
| URL | Purpose |
|-----|---------|
| `http://g.msn.com/5meen_us/122` | Help topic |
| `http://g.msn.com/5me%1/190` | Server status |
| `http://g.msn.com/5me%1/98` | Privacy statement |
| `http://g.msn.com/5me%1/231` | Terms of use |
| `http://g.msn.com/5me%1/157` | Sign-up page |
| `http://g.msn.com/5me%1/180` | About page |
| `http://g.msn.com/5me%1/55` | Help page |

### Identity/Auth Strings
- "Select email address"
- "Enter your email address in this format: yourname@example.com"
- "Sign in using a different email address"
- "Select an email address to sign in with"
- "Is this a public computer?" with "Sign in" link
- "Authentication using your @%1 ID will take place on %1 servers"

### Language Info
- Primary MUI language: `en-us`
- Language identifier: `0x0409`

### Embedded Manifest
```xml
<assembly xmlns="urn:schemas-microsoft-com:asm.v1" manifestVersion="1.0">
  <assemblyIdentity version="5.1.0.0" processorArchitecture="x86"
    name="Microsoft.Windows.personalMedia.WLAVRes" type="win32" />
  <description>Microsoft Windows Live AVRes</description>
  <!-- Registers COM objects for WLMFReadWrite, WLMFDS, MPG4DEMUX, RSCMFT -->
</assembly>
```

### COM Class Registrations (via Manifest)
| COM Class | CLSID | Threading |
|-----------|-------|-----------|
| WLMFReadWrite | `{48e2ed0f-98c2-4a37-bed5-166312ddd83f}` | Both |
| WLMFDS | `{f792beee-aeaf-4ebb-ab14-8bc5c8c695a8}` | Both |
| WLMFDS | `{0344ec28-5339-4124-a186-2e8eef168785}` | Both |
| WLMFDS | `{743a6e3b-a5df-43ed-b615-4256add790b8}` | Both |
| WLMFDS | `{eb4d075a-65c0-476b-956c-c605eade03f7}` | Both |
| WLMFDS | `{14d7a407-396b-44b3-be85-5199a0f0f80a}` | Both |
| MPG4DEMUX | `{354FDED2-09E6-41df-9839-EFD82DCD2533}` | Both |
| RSCMFT | `{B5754F1A-D387-49D0-B32D-B5A7633FF3D4}` | Both |

## Code Analysis
The ~3.7 KB of code in `.text` is entirely CRT initialization/teardown boilerplate:
- `_DllMainCRTStartup` / `__DllMainCRTStartup`
- C++ global constructor/destructor initialization (`_initterm` / `_initterm_e`)
- Exception handler registration (`_except_handler4_common`)
- Standard security cookie (`__security_cookie`, `__GSHandlerCheck_EH`)

**No exported functions, no application logic.**

## Digital Signatures
The DLL is signed with **multiple Microsoft certificates**:
1. **Microsoft Code Signing PCA** (time-stamped 2014-04-01)
2. **Microsoft Time-Stamp PCA 2010** (multiple timestamps)
3. **Microsoft Code Signing PCA 2011** (additional signing chain)

Signed by: `Microsoft Corporation` with nCipher DSE ESN: `B8EC-30A4-71441`

## Architecture
```
WLAVRes.dll (Resource-Only)
  |
  +-- .rsrc: MUI strings, menus, UI definitions
  |     |
  |     +-- Calendar UI strings (date navigation)
  |     +-- Windows Live sign-in UI text
  |     +-- Menu definitions (Cut/Copy/Paste etc.)
  |     +-- Color names for UI themes
  |     +-- Help/support URLs (g.msn.com)
  |     +-- COM class manifest (WLMFDS, WLMFReadWrite, MPG4DEMUX, RSCMFT)
  |
  +-- Consumed by: wlidux.dll, WLMFDS.dll, WLMFReadWrite.dll, other WL components
```

## Key Observations
1. **Pure resource DLL** - zero application logic, only data
2. **Shared UI strings** for Windows Live media components (Photo Gallery, Movie Maker)
3. **Windows Live ID sign-in** strings embedded - this is the shared login dialog text
4. **MUI-enabled** - proper Multilingual UI resource structure for localization
5. **COM manifest** registers 8 COM objects across 4 media pipeline components
6. **Same timestamp** as wlidux.dll (2014-04-01) - built in the same release cycle
7. **MSVC 2012** toolchain (same as all other WL binaries)
8. **File size**: 25,288 bytes - extremely small, consistent with resource-only DLL
9. **No delay imports**, no imports beyond CRT bootstrap
10. **URLs** all point to `g.msn.com` Windows Live help/support infrastructure
