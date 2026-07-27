# wlsoxe.dll Analysis

## Overview

**File:** wlsoxe.dll (Windows Live Movie Maker 2012)
**Full Name:** Windows Live Social Object Extractor Engine
**Role:** COM in-process server for extracting social metadata (images, properties) from web pages via embedded IE/MSHTML
**Image size:** 0x2E000 (~1.8 MB)
**PDB:** `wlsoxe.pdb` `{CE344970-AFCD-45BE-9A4D-03A4781A2FDC}`
**Build:** 16.4.3528.0331_ship (timestamp: 2014-04-01)
**User-Agent:** `Windows-Live-Social-Object-Extractor-Engine/1.0`

## PE Structure

| Field | Value |
|-------|-------|
| Machine | x86 (0x14C) |
| Subsystem | Windows GUI (2) |
| Linker | MSVC 11.00 |
| Image Base | 0x10000000 |
| Image Size | 0x2E000 |
| Entry Point | 0x10015AA4 |
| OS/Image Version | 6.02 / 6.02 |
| DLL Characteristics | Dynamic Base (ASLR), NX Compatible |
| Stack reserve/commit | 256 KB / 4 KB |
| Heap reserve/commit | 1 MB / 4 KB |
| Checksum | 0x34FCD |

### Sections

| Section | VA | VirtSize | RawSize | Characteristics |
|---------|-------|----------|---------|-----------------|
| `.text` | 0x1000 | 0x187C9 | 0x18800 | Code, Execute Read |
| `.data` | 0x1A000 | 0x1C94 | 0x1400 | Init Data, Read Write |
| `.rsrc` | 0x1C000 | 0xE920 | 0xEA00 | Init Data, Read Only (58 KB) |
| `.reloc` | 0x2B000 | 0x26AA | 0x2800 | Init Data, Discardable, Read Only |

### Directories

| Directory | RVA | Size |
|-----------|-----|------|
| Export | 0x19760 | 0x69 |
| Import | 0x18A1C | 0xF0 |
| Resource | 0x1C000 | 0xE920 |
| Base Reloc | 0x2B000 | 0x20C8 |
| Debug | 0x12E0 | 0x38 |
| Load Config | 0x4000 | 0x40 |
| IAT | 0x1000 | 0x294 |
| Certificates | 0x2B200 | 0x3EC0 |

## Export Table (2 functions)

| Ordinal | RVA | Name |
|---------|-----|------|
| 1 | 0x000068DF | `DllCanUnloadNow` |
| 2 | 0x000068F8 | `DllGetClassObject` |

Minimal COM DLL — only factory and unload queries exported. No `DllRegisterServer`/`DllUnregisterServer` (registration handled externally by the Windows Live installer).

## COM GUIDs Discovered

### Registered COM Classes (CLSID variable names in RTTI)
- `CLSID_SocialObjectExtractorEngine` — the primary COM class for extracting social objects from URLs
- `CLSID_SoxeDefinitionUpdater` — background task for updating extractor definitions

### COM Interfaces Defined (25 interfaces)

#### Core Engine Interfaces
- `ISocialObjectExtractorEngine` — main entry point: `ExtractSocialObjectFromUrl(url, doc, &socObj)`
- `ISocialObject` — represents an extracted social object (URL, MIME type, images, properties)
- `ISocialObjectPrivate` — private/internal extension of ISocialObject
- `ISocialObjectDataStore` — backing store for social objects

#### Extraction Definition Interfaces
- `IExtractorDefinition` — defines an extraction rule (domain, category, MIME type, properties)
- `IExtractorDefinitionProperty` — property extraction rule (name, type, CSS selector)
- `IExtractorDefinitionPropertyRule` — rule for matching/extracting property values
- `ITaskHandler` — background task handler for SoxeDefinitionUpdater

#### CSS Selector Engine Interfaces
- `ICSSSelector` — CSS selector interface
- `ICSSSelectorImpl` — CSS selector implementation
- `IFormatter` — value formatting interface

#### Data Source Interfaces
- `ISoxeXmlDataSource` — XML configuration data source
- `ISoxeValue` — abstract value interface for extractor properties

#### MSHTML/IE Integration Interfaces
- `IMshtmlWrapper` — wraps IHTMLDocument2 for page parsing
- `IIOleClientSite` — OLE client site for embedded browser
- `IServiceProvider` — service provider for MSHTML
- `IAuthenticate` — authentication handler for HTTP
- `IInternetSecurityManager` — internet security manager
- `IPropertyNotifySink` — property change notifications
- `HTMLWindowEvents` — HTML window event sink (DIID)

#### Collection Interfaces
- `IExtractedImageList` — list of extracted images (GetCount, GetItem)
- `IClassFactory` / `IUnknown` — standard COM

### Registered COM Classes (12 concrete C++ classes via ATL)

| Class | Role |
|-------|------|
| `ExtractorEngine` | Primary COM object — `CComCoClass<ExtractorEngine, CLSID_SocialObjectExtractorEngine>` |
| `SocialObject` | Represents extracted social data |
| `ExtractorDefinition` | Defines extraction rules per domain |
| `PriZeroExtractorDefinition` | Priority-zero (built-in/default) extractor definition |
| `ExtractorDefinitionPropertyImpl` | Property extraction implementation |
| `ExtractorDefinitionPropertyRuleImpl` | Property rule implementation |
| `MshtmlWrapper` | Wraps MSHTML `IHTMLDocument2` for DOM traversal |
| `FeedParser` | Parses RSS/Atom feeds from URLs |
| `FormatterProperty` | CSS property value formatter |
| `CSSSelectorProperty` | CSS selector matching |
| `FilesystemSoxeXmlDataSource` | Loads extractor definitions from filesystem XML |
| `SoxeDefinitionUpdater` | `CComCoClass<SoxeDefinitionUpdater, CLSID_SoxeDefinitionUpdater>` — background update task |

### CSS Selector Engine (10 classes)

| Class | Role |
|-------|------|
| `BaseSelector` | Base class for selectors |
| `TypeSelector` | Matches element tag name (e.g., `div`, `img`) |
| `IDSelector` | Matches element ID (`#id`) |
| `ClassSelector` | Matches element class (`.className`) |
| `AttributeSelector` | Matches element attribute (`[attr=value]`) |
| `ChildCombinator` | Direct child combinator (`>`) |
| `DescendantCombinator` | Descendant combinator (space) |
| `ICSSSelectorImpl` | CSS selector interface implementation |
| `CSSSelectorProperty` | Wraps CSS selectors as COM objects |

### Value Type Classes (8 ATL COM objects)

| Class | Role |
|-------|------|
| `SoxeValueBase` | Base value type |
| `SoxeBstrValue` | BSTR string value |
| `SoxeGuidValue` | GUID value |
| `SoxeBoolValue` | Boolean value |
| `SoxeDefinitionPriorityValue` | Definition priority enum |
| `SoxePropertyDataTypeValue` | Property data type enum |
| `SoxeRulePriorityValue` | Rule priority enum |

### Template/Container Classes

| Class | Role |
|-------|------|
| `DirectoryWatcher<0xEAA>` | Watches directory for changes (extractor definition hot-reload) |
| `CAutoDPA<UIExtractorDefinition>` | Dynamic pointer array for extractor definitions |
| `CAutoDPA<UIExtractorDefinitionProperty>` | Dynamic pointer array for properties |
| `CAutoDPA<UIExtractorDefinitionPropertyRule>` | Dynamic pointer array for property rules |
| `CAutoDPA<UISoxeXmlDataSource>` | Dynamic pointer array for XML data sources |
| `CAutoDPA<UAdServerDefinition>` | Dynamic pointer array for ad server definitions |

### ATL Module Classes

| Class | Role |
|-------|------|
| `CSoxeModule` | Custom ATL module for SOXE |
| `CAtlDllModuleT<CSoxeModule>` | DLL module specialization |
| `CAtlModuleT<CSoxeModule>` | Base ATL module |
| `CAtlValidateModuleConfiguration<0, CSoxeModule>` | Module config validation |
| `_ATL_MODULE70` | ATL 7.0 module structure |

## Import Table

### Direct Imports (load-time)

#### MSVCR110.dll (36 functions)
VC++ 2012 CRT: memory (`malloc`, `calloc`, `free`, `calloc`, `_recalloc`, `memcpy_s`, `memmove_s`, `memset`, `memcmp`), string (`wcscpy_s`, `wcsncpy_s`, `wcsnlen`, `_wcsicmp`, `wcschr`, `wcsncmp`, `wcscspn`, `wcsspn`, `wcstol`, `wcstoul`, `_wcslwr_s`, `iswspace`, `swscanf_s`, `wmemcpy_s`, `_wtoi`), CRT init (`_initterm`, `_initterm_e`, `_amsg_exit`, `_malloc_crt`, `_calloc_crt`), exception (`__CxxFrameHandler3`, `__CppXcptFilter`, `_except_handler4_common`, `terminate`), RTTI (`??1type_info@@UAE@XZ`), allocation (`_callnewh`, `??_V@YAXPAX@Z`, `??3@YAXPAX@Z`), threading (`_lock`, `_unlock`, `__dllonexit`, `_onexit`), debug (`_crt_debugger_hook`).

#### WININET.dll (10 functions)
HTTP client: `InternetOpenW`, `InternetConnectW`, `InternetCanonicalizeUrlW`, `InternetCombineUrlW`, `HttpOpenRequestW`, `HttpSendRequestExW`, `HttpEndRequestW`, `HttpQueryInfoW`, `InternetSetStatusCallbackW`, `InternetCloseHandle`.

#### urlmon.dll (2 functions)
URL monikers: `URLDownloadToCacheFileW`, `CreateURLMonikerEx`.

#### SHELL32.dll (2 functions)
Shell paths: `SHGetFolderPathAndSubDirW`, `SHGetKnownFolderPath`.

#### KERNEL32.dll (40+ functions)
Process/thread (`GetCurrentThreadId`, `DisableThreadLibraryCalls`), synchronization (`CreateEventW`, `SetEvent`, `ResetEvent`, `Sleep`, `InterlockedIncrement`, `InterlockedDecrement`, `EnterCriticalSection`, `LeaveCriticalSection`, `InitializeCriticalSectionAndSpinCount`, `DeleteCriticalSection`), memory (`HeapAlloc`, `HeapFree`, `HeapReAlloc`, `HeapSize`, `HeapDestroy`, `GetProcessHeap`), file I/O (`CreateFileW`, `CopyFileW`, `ReadDirectoryChangesW`, `GetOverlappedResult`), modules (`LoadLibraryExW`, `FreeLibrary`, `GetModuleHandleW`, `GetModuleFileNameW`, `GetProcAddress`), resources (`FindResourceW`, `FindResourceExW`, `LoadResource`, `LockResource`, `SizeofResource`), string (`lstrcmpW`, `lstrcmpiW`), conversion (`MultiByteToWideChar`, `WideCharToMultiByte`), timing (`GetTickCount64`, `QueryPerformanceCounter`, `GetSystemTimeAsFileTime`), PE helpers (`EncodePointer`, `DecodePointer`), error (`GetLastError`, `RaiseException`), wait (`RegisterWaitForSingleObject`, `UnregisterWait`, `UnregisterWaitEx`), process info (`IsDebuggerPresent`, `IsProcessorFeaturePresent`).

#### USER32.dll (4 functions)
Message loop: `PeekMessageW`, `DispatchMessageW`, `MsgWaitForMultipleObjects`, `CharLowerBuffW`.

#### ADVAPI32.dll (12 functions)
Registry (`RegOpenKeyExW`, `RegCreateKeyExW`, `RegSetValueExW`, `RegQueryValueExW`, `RegCloseKey`), ETW tracing (`RegisterTraceGuidsW`, `UnregisterTraceGuids`, `TraceEvent`, `TraceMessage`, `GetTraceLoggerHandle`, `GetTraceEnableLevel`, `GetTraceEnableFlags`).

#### ole32.dll (6 functions)
COM: `CoCreateInstance`, `CoWaitForMultipleHandles`, `CreateBindCtx`, `CoTaskMemFree`, `CoUnmarshalInterface`, `CoMarshalInterThreadInterfaceInStream`.

#### OLEAUT32.dll (12 ordinal imports)
Ordinals: 2, 4, 6, 7, 8, 9, 12, 149, 150, 161, 162, 314 — likely `SysAllocString`, `SysFreeString`, `SysStringLen`, `VariantInit`, `VariantClear`, `SafeArray*` functions.

#### COMCTL32.dll (9 ordinal imports)
Common controls: Ordinals 328, 331, 332, 334, 337, 338, 339, 385, 386.

#### SHLWAPI.dll (2 functions)
Path utilities: `PathAppendW`, ordinal 16 (`PathCombine` or `StrCmpW`).

### No Delay-Load Imports

Unlike most other WL* DLLs, wlsoxe.dll has **no delay-loaded imports**. It directly imports all dependencies at load time.

## RTTI Classes (106 total)

### Class Hierarchy Summary

```
CSoxeModule (ATL module)
├── CAtlDllModuleT<CSoxeModule>
├── CAtlModuleT<CSoxeModule>
│
ExtractorEngine (CComCoClass)
├── CComObject<ExtractorEngine>
│
SocialObject
├── CComObject<SocialObject>
│
ExtractorDefinition (CComCoClass)
├── CComObject<ExtractorDefinition>
├── PriZeroExtractorDefinition
│   └── CComObject<PriZeroExtractorDefinition>
│
ExtractorDefinitionPropertyImpl
ExtractorDefinitionPropertyRuleImpl
│
MshtmlWrapper
├── CComObject<MshtmlWrapper>
├── IServiceProviderImpl<MshtmlWrapper>
├── IDispatchImpl<HTMLWindowEvents, DIID_HTMLWindowEvents, LIBID_MSHTML>
│
FeedParser
├── CComObject<FeedParser>
│
CSS Selector Engine:
├── BaseSelector → CComObject<BaseSelector>
├── TypeSelector → CComObject<TypeSelector>
├── IDSelector → CComObject<IDSelector>
├── ClassSelector → CComObject<ClassSelector>
├── AttributeSelector → CComObject<AttributeSelector>
├── ChildCombinator → CComObject<ChildCombinator>
├── DescendantCombinator → CComObject<DescendantCombinator>
│
Value Types:
├── SoxeValueBase
│   ├── SoxeBstrValue → CComObject<SoxeBstrValue>
│   ├── SoxeGuidValue → CComObject<SoxeGuidValue>
│   └── SoxeBoolValue → CComObject<SoxeBoolValue>
├── SoxeDefinitionPriorityValue → CComObject<SoxeDefinitionPriorityValue>
├── SoxePropertyDataTypeValue → CComObject<SoxePropertyDataTypeValue>
├── SoxeRulePriorityValue → CComObject<SoxeRulePriorityValue>
│
FormatterProperty → CComObject<FormatterProperty>
CSSSelectorProperty → CComObject<CSSSelectorProperty>
FilesystemSoxeXmlDataSource → CComObject<FilesystemSoxeXmlDataSource>
SoxeDefinitionUpdater (CComCoClass) → CComObject<SoxeDefinitionUpdater>
DirectoryWatcher<0xEAA>
```

## Extractor Definition XML Schema

The DLL reads extractor definitions from XML files matching the XPath `//config:ExtractorDefinitions` with namespace `xmlns:config="http://hmdevsrv/schemas/common/v2/Config.xsd"`.

### XML Config Paths (from string extraction)
| XPath | Purpose |
|-------|---------|
| `//config:ExtractorDefinitions` | Root element |
| `./config:entry` | Individual extractor definition entry |
| `./config:id` | Unique extractor ID |
| `./config:category` | Category (e.g., "image", "generic") |
| `./config:domain` | Target domain pattern |
| `./config:allowSubDomains` | Whether subdomains match |
| `./config:priority` | Extraction priority |
| `./config:property` | Property definition |
| `./config:name` | Property name |
| `./config:type` | Property type |
| `./config:mimeType` | MIME type filter |
| `./config:dataType` | Data type for values |
| `./config:required` | Whether property is required |
| `./config:rule` | Extraction rule |
| `./config:value` | Rule value |
| `./config:attr` | Attribute selector |
| `./config:adFilter` | Ad content filter |

### Built-in Selector/Formatter Types
| Value | Purpose |
|-------|---------|
| `cssselector` | CSS selector rule type |
| `formatter` | Value formatter type |
| `generic` | Generic extractor category |
| `text/plain` | Plain text MIME type |
| `text/html` | HTML MIME type |

## URL Scheme Support

The DLL understands these URL schemes for extraction:
- `http`, `https` — web pages (primary targets)
- `file` — local HTML files
- `gopher` — legacy protocol
- `mailto` — email links
- `news` — news groups
- `socks` — SOCKS proxy

## HTML Entity Table

The DLL contains a complete HTML 4.01 entity reference table (~250 entities) for decoding extracted text, including:
- Standard entities: `&amp;`, `&lt;`, `&gt;`, `&quot;`, `&apos;`
- Latin-1 entities: `&nbsp;`, `&iexcl;`, `&cent;`, `&pound;`, etc.
- Greek letters: `&Alpha;` through `&omega;`
- Mathematical symbols: `&forall;`, `&part;`, `&infin;`, etc.
- Typographic: `&ndash;`, `&mdash;`, `&ldquo;`, `&rdquo;`, etc.
- Card suits: `&spades;`, `&clubs;`, `&hearts;`, `&diams;`

## Registry Paths

### SOXE Configuration
```
Software\Microsoft\Windows Live\SOXE
```

### Registry Values
| Value | Purpose |
|-------|---------|
| `ExtractionAttempted` | Tracks whether extraction was attempted |

## File System Paths

### Extractor Definitions
```
Microsoft\Windows Live\SOXE\extractorDefinitions.xml
Microsoft\Windows Live\SOXE\
```

### Messenger Companion Integration
```
Microsoft\Messenger Companion
```

## Telemetry

### ETW Tracing (via ADVAPI32)
- `RegisterTraceGuidsW` — registers SOXE trace provider
- `TraceEvent` / `TraceMessage` — emits ETW events
- `GetTraceLoggerHandle` / `GetTraceEnableLevel` / `GetTraceEnableFlags` — trace control

### BICI (via DmxBici)
- Not directly imported; ETW-based telemetry only

## Function Categories

### 1. COM Infrastructure
- `DllCanUnloadNow`, `DllGetClassObject`
- ATL module initialization (`CSoxeModule`)
- COM object creation via `CComCoClass` / `CComObject` templates
- Marshaling: `CoMarshalInterThreadInterfaceInStream`, `CoUnmarshalInterface`

### 2. Social Object Extraction Pipeline
1. `ISocialObjectExtractorEngine::ExtractSocialObjectFromUrl(url, doc, &socObj)`
2. Load extractor definitions from `extractorDefinitions.xml`
3. Match URL domain against extractor definitions
4. Use CSS selectors to find elements in `IHTMLDocument2`
5. Extract images (`IExtractedImageList`), properties, and metadata
6. Create `ISocialObject` with results

### 3. CSS Selector Engine (10 classes)
Full CSS selector implementation:
- **TypeSelector** — matches tag names
- **IDSelector** — matches `#id`
- **ClassSelector** — matches `.className`
- **AttributeSelector** — matches `[attr=value]`
- **ChildCombinator** — direct child (`>`)
- **DescendantCombinator** — descendant (space)
- Selector evaluation against MSHTML DOM elements

### 4. MSHTML/IE Integration
- `MshtmlWrapper` — wraps `IHTMLDocument2` for DOM access
- Implements `IOleClientSite`, `IServiceProvider`, `IDispatch`
- Handles `HTMLWindowEvents` for page load notifications
- Implements `IAuthenticate` for HTTP authentication
- Implements `IInternetSecurityManager` for security zone control
- Uses `IHTMLElementCollection` for element enumeration
- Accesses element properties: `tagName`, `className`, `id`, `innerHTML`, `innerText`, `getAttribute`, `setAttribute`

### 5. Feed Parsing
- `FeedParser` class for RSS/Atom feed extraction
- Works with `ISoxeXmlDataSource` interface

### 6. HTTP Client (WININET)
- `InternetOpenW` with user-agent `Windows-Live-Social-Object-Extractor-Engine/1.0`
- `HttpOpenRequestW` / `HttpSendRequestExW` / `HttpEndRequestW` for HTTP operations
- `HttpQueryInfoW` for response headers
- `URLDownloadToCacheFileW` for caching downloads
- `InternetSetStatusCallbackW` for async status

### 7. Background Update Task
- `SoxeDefinitionUpdater` COM class
- Implements `ITaskHandler` for task scheduler integration
- `DirectoryWatcher<0xEAA>` watches for XML definition file changes
- Hot-reload of extractor definitions without restart

### 8. HTML Entity Decoding
- Full HTML 4.01 entity table (~250 entities)
- Converts `&entity;` references to Unicode characters
- Handles named entities, numeric references

### 9. String Formatting
- GUID formatting: `%8x-%4x-%4x-%4x-%12x`
- URL pattern matching: `%[_A-Za-z]`, `%[_A-Za-z0-9-]`
- `swscanf_s` for format string parsing

## Architecture Summary

wlsoxe.dll is the **Social Object Extractor Engine** — a Windows Live component that extracts structured social metadata (images, titles, descriptions, MIME types) from web pages. It:

1. **Loads extractor definition rules** from XML files that define per-domain extraction logic
2. **Embeds an IE/MSHTML browser** (`MshtmlWrapper`) to parse web pages
3. **Applies CSS selectors** (full CSS selector engine with 7 selector/combinator types) to find target elements
4. **Extracts image URLs** and metadata properties from matched elements
5. **Returns structured `ISocialObject` data** to the caller (Movie Maker, Photo Gallery, Messenger)

The DLL is used by Windows Live Movie Maker and Photo Gallery to generate rich previews when users share URLs — it downloads the page, renders it in an embedded browser, extracts the Open Graph/metadata images and properties, and provides them for social sharing tiles.

### Key Integration Points
- **Movie Maker / Photo Gallery**: Call `ExtractSocialObjectFromUrl` to get share preview data
- **Messenger Companion**: Shares social objects for chat link previews
- **MSHTML (ieframe.dll)**: Hosted browser for DOM rendering
- **WININET**: HTTP client for fetching web pages
- **Registry**: Stores extraction state under `Software\Microsoft\Windows Live\SOXE`
