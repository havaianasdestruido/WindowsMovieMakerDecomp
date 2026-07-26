# MovieMakerCore.dll — Deep Static Analysis

## 1. Binary Overview

| Property | Value |
|----------|-------|
| **File** | MovieMakerCore.dll |
| **Type** | DLL (PE32, x86) |
| **Image Base** | 0x10000000 |
| **Image Size** | 0xA22000 (10,626,048 bytes, ~10.1 MB) |
| **Entry Point** | 0x104C8E3D (DllMain CRT init) |
| **Subsystem** | Windows GUI (2) |
| **Linker** | 11.00 (MSVC 11.0 / Visual Studio 2012) |
| **OS Version** | 6.02 (Windows 8) |
| **Timestamp** | 0x533A3FDC (Tue Apr 1 01:26:04 2014) |
| **DLL Characteristics** | Dynamic base (ASLR), NX compatible (DEP) |
| **Stack** | Reserve 0x40000 / Commit 0x1000 |
| **Heap** | Reserve 0x100000 / Commit 0x1000 |
| **Checksum** | 0xA23F3E |
| **PDB** | MovieMakerCore.pdb ({D5217874-B614-477C-B45B-E0CE638C6496}) |
| **Product Version** | 16.4.3528.0331 |
| **Copyright** | (c) 2012 Microsoft Corporation |
| **Internal Codename** | Sundance |

## 2. Section Layout

| Section | VA | Virtual Size | Raw Size | Flags |
|---------|----|-------------|----------|-------|
| `.text` | 0x1000 | 0x574654 (5.46 MB) | 0x574800 | Code, Exec Read |
| `.data` | 0x576000 | 0x3929C (229 KB) | 0x2E200 | Init Data, Read Write |
| `.rsrc` | 0x5B0000 | 0x40B478 (4.07 MB) | 0x40B600 | Init Data, Read Only |
| `.reloc` | 0x9BC000 | 0x6593E (406 KB) | 0x65A00 | Discardable, Read Only |

- `.text` = 5.46 MB — entire application logic in one section
- `.rsrc` = 4.07 MB — DirectUI layouts, icons, strings, templates, COM REGISTRY
- `.data` = 229 KB — global variables, vtables, RTTI data, vtable pointers
- `.reloc` = 406 KB — ASLR relocation table

## 3. Export Table

| Ordinal | Hint | RVA | Name |
|---------|------|-----|------|
| 1 | 0 | 0x000E96C0 | `MovieMakerMain` |

**Single export function.** The DLL exposes exactly one entry point.

### MovieMakerMain Signature (from disassembly)

```
100E96C0: mov  edi,edi         ; hotpatch prologue
100E96C2: push ebp
100E96C3: mov  ebp,esp
100E96C5: push 0FFFFFFFFh     ; SEH frame
100E96C7: push 104CC469h       ; SEH handler
100E96D4: mov  eax,1268h       ; stack frame size = 0x1268 (4712 bytes!)
```

The function allocates a 4712-byte stack frame, indicating extensive local variable usage. It sets up SEH (Structured Exception Handling) with handler at 0x104CC469. The first parameter `argc` is accessed at `[ebp+8]`. The function returns a 32-bit int (observed: `0xC945001A` when called with minimal args — an internal error code).

### Calling Convention

`__cdecl` (x86 default) — caller cleans the stack. The prologue follows MSVC hotpatch convention (`mov edi,edi` / `push ebp` / `mov ebp,esp`).

## 4. Import Table — 34 DLLs, 763 Functions

### 4.1 Core Windows APIs (5 DLLs, ~364 functions)

| DLL | Imports | Key Functions |
|-----|---------|---------------|
| **KERNEL32.dll** | 117 | CreateFileW, CreateThread, VirtualAlloc, CreateIoCompletionPort, LoadLibraryExW, GetModuleHandleW, QueryPerformanceCounter, SleepConditionVariableCS, CreateFileMappingW, MapViewOfFile, InterlockedCompareExchange, GetQueuedCompletionStatus, CreatePipe, CreateProcessW, HeapSetInformation, HeapAlloc, HeapFree, HeapReAlloc, DisableThreadLibraryCalls, GetCurrentProcess, GetCurrentProcessId, GetCurrentThreadId, GetModuleFileNameW, GetProcAddress, GetVersion, FormatMessageW, MultiByteToWideChar, WideCharToMultiByte, GetTempPathW, GetTempFileNameW, GetFileAttributesW, DeleteFileW, CopyFileW, MoveFileExW, CreateDirectoryW, FindFirstFileExW, FindNextFileW, FindClose, GetLocaleInfoEx, GetDateFormatEx, GetTimeFormatEx, SetThreadPreferredUILanguages, GetUserPreferredUILanguages, CompareStringW, RaiseException, DebugBreak, IsDebuggerPresent, IsWow64Process, QueueUserWorkItem, etc. |
| **USER32.dll** | 84 | CreateWindowExW, RegisterClassExW, DefWindowProcW, PostMessageW, SendMessageW, SetWindowPos, SystemParametersInfoW, TrackPopupMenuEx, RegisterDeviceNotificationW, MonitorFromPoint, GetClientRect, GetWindowRect, GetDC, ReleaseDC, ShowWindow, EnableWindow, SetFocus, SetForegroundWindow, DestroyWindow, MessageBeep, LoadCursorW, LoadIconW, LoadImageW, LoadStringW, MessageBoxW, PeekMessageW, TranslateMessage, DispatchMessageW, AnimateWindow, FlashWindowEx, SetTimer, KillTimer, GetSysColor, GetSystemMetrics, EnumDisplayMonitors, GetMonitorInfoW, etc. |
| **GDI32.dll** | 17 | CreateDIBSection, BitBlt, SelectObject, GetDIBits, GetDeviceCaps, GetStockObject, ExtCreateRegion, GetRegionData, CreateSolidBrush, CreateCompatibleDC, DeleteDC, DeleteObject, GetObjectW, GetLayout, SetLayout, GetClipRgn, CreateRectRgnIndirect, OffsetRgn |
| **ADVAPI32.dll** | 21 | RegCreateKeyExW, RegQueryValueExW, RegSetValueExW, RegOpenKeyExW, RegCloseKey, RegDeleteKeyW, RegDeleteValueW, RegEnumKeyExW, RegQueryInfoKeyW, RegGetValueW, RegOpenCurrentUser, CryptAcquireContextW, CryptReleaseContext, CryptSignHashW, CryptHashData, CryptCreateHash, CryptDestroyHash, CryptImportKey, CryptDestroyKey, RegisterTraceGuidsW, TraceMessage, TraceEvent, GetTraceLoggerHandle, GetTraceEnableLevel, GetTraceEnableFlags |
| **SHELL32.dll** | 17 | SHGetDesktopFolder, SHCreateShellItem, SHCreateShellItemArrayFromShellItem, SHGetFolderPathW, SHParseDisplayName, SHOpenFolderAndSelectItems, CommandLineToArgvW, SHGetDiskFreeSpaceExW, ShellExecuteW, SHGetKnownFolderPath, SHCreateItemFromParsingName, ShellExecuteExW, SHBindToParent, SHOpenWithDialog, SHGetPropertyStoreFromParsingName |

### 4.2 COM/OLE (3 DLLs, 47 functions)

| DLL | Imports | Key Functions |
|-----|---------|---------------|
| **ole32.dll** | 23 | CoCreateInstance, CoInitializeEx, CoSetProxyBlanket, CoInitialize, CoUninitialize, CreateStreamOnHGlobal, StgOpenStorage, OleInitialize, OleUninitialize, OleSetClipboard, OleGetClipboard, OleIsCurrentClipboard, PropVariantCopy, PropVariantClear, CLSIDFromString, StringFromCLSID, StringFromGUID2, CoTaskMemAlloc, CoTaskMemFree, CoTaskMemRealloc, CoGetObject, ReleaseStgMedium, CoAllowSetForegroundWindow |
| **OLEAUT32.dll** | 22 (ordinals) | SysAllocString (Ord 2), SysFreeString (Ord 4), SysStringLen (Ord 6), VariantInit (Ord 8), VariantClear (Ord 9), VariantCopy (Ord 10), VariantChangeType (Ord 12), SafeArrayCreateVector (Ord 147), SafeArrayDestroy (Ord 149), SafeArrayCopy (Ord 150), SafeArrayPutElement (Ord 161), LoadTypeLib (Ord 162), LoadRegTypeLib (Ord 184/185), DispCallFunc (Ord 277), SystemTimeToVariantTime (Ord 194), SysAllocStringLen (Ord 26), SysReAllocStringLen (Ord 27), SystemTimeToVariantTime (Ord 146) |
| **OLEACC.dll** | 2 | AccessibleChildren, AccessibleObjectFromWindow |

### 4.3 Media Foundation (2 DLLs, 21 functions)

| DLL | Imports | Key Functions |
|-----|---------|---------------|
| **MF.dll** | 2 | MFGetService, MFTranscodeGetAudioOutputAvailableTypes |
| **MFPlat.DLL** | 19 | MFStartup, MFShutdown, MFCreateMediaType, MFCreateSample, MFCreateAttributes, MFCreateMemoryBuffer, MFCreateAlignedMemoryBuffer, MFCreateSourceResolver, MFCreateCollection, MFPutWorkItemEx, MFLockPlatform, MFUnlockPlatform, MFFrameRateToAverageTimePerFrame, MFAverageTimePerFrameToFrameRate, MFGetPluginControl, MFInvokeCallback, MFCreateWaveFormatExFromMFMediaType, MFInitMediaTypeFromWaveFormatEx, MFCalculateImageSize |

### 4.4 DirectX/Graphics (9 DLLs, 36 functions)

| DLL | Imports | Key Functions |
|-----|---------|---------------|
| **d3d11.dll** | 1 | D3D11CreateDevice |
| **d3d9.dll** | 2 | Direct3DCreate9, Direct3DCreate9Ex |
| **d2d1.dll** | 1 (ordinal) | Ordinal 1 (D2D1CreateFactory) |
| **DWrite.dll** | 1 | DWriteCreateFactory |
| **dxva2.dll** | 2 | DXVA2CreateDirect3DDeviceManager9, DXVA2CreateVideoService |
| **D3DCOMPILER_46.dll** | 2 | D3DGetInputSignatureBlob, D3DReflect |
| **gdiplus.dll** | 24 | GdiplusStartup, GdiplusShutdown, GdipCreateFromHDC, GdipDrawImageRectRect, GdipCreateFont, GdipCreateFontFamilyFromName, GdipDeleteFont, GdipDeleteFontFamily, GdipCreateSolidFill, GdipDeleteBrush, GdipCreateBitmapFromStream, GdipCloneImage, GdipDisposeImage, GdipGetImageWidth, GdipGetImageHeight, GdipSetClipHrgn, GdipFillRectangle, GdipSetRenderingOrigin, GdipTranslateWorldTransform, GdipGetGenericFontFamilySansSerif, GdipGetFontHeightGivenDPI, GdipFree, GdipAlloc, GdipCloneBrush |
| **dwmapi.dll** | 1 | DwmExtendFrameIntoClientArea |
| **UxTheme.dll** | 3 | OpenThemeData, GetThemeMetric, CloseThemeData |

### 4.5 Windows Live / Microsoft (7 DLLs, 235 functions)

| DLL | Imports | Key Functions |
|-----|---------|---------------|
| **UXCore.dll** | 192 | CDUIDialog (OnInitIcon, OnDetach, OnSaveHiddenState, OnLoadHiddenState, OnCreateRegion, OnUpdateFrame, OnDefaultFrameColorChanged, OnCommand, ExitDialog, OnMessage, OnSize, OnDestroy, OnNCDestroy, OnActivate, OnSysCommand, OnInitDialog, OnClose, OnGetMinMaxInfo, OnShowWindow, OnPostCreateDialog, FilterMessage, FindDialogElement, ShowDialog, DoModal, GetDUIParser, GetDialogHWND), CFramelessHost, Element (Add, Insert, Remove, Destroy, DestroyAll, FindDescendent, IsDescendent, SetValue, GetValue, GetDataContext, SetDataContext, FireEvent, EnsureVisible, GetElementDC, ReleaseElementDC, MapElementPoint, QIBehaviors, GetLayer, ExecCmd, _UpdateDesiredSize, _UpdateLayoutSize, _UpdateLayoutPosition, StartDefer, EndDefer), VirtualLayout (GetTemplate, GetTemplateIndex, GetLastKeyFocusedTemplate, SetNextKeyFocus), NativeHWNDHost, SuperPopup, PopupWindow, FillLayout, DuiCreateObject, UXCoreInitProcess, UXCoreInitThread, UXCoreUnInitProcess, UXCoreUnInitThread, GetMessageEx, PeekMessageEx, CRMImage, CRMDUIParser, CRMResource, CRMStringResource, IDuiDataSourceImpl, AttachWndProc, DetachWndProc, SetGadgetStyle, GetGadgetSize, GetGadgetRect, MapGadgetPoints, BuildDropTarget, LayerManagerInitThread, LayerManagerUnInitThread, DuiGetLayerManager, UxGetClassObject, RMLoadString, RMLoadImage, RMLoadRect, RMLoadMenu, RMLoadStringBSTR, RMFindModule, RMFindModuleForResource, RMUpdateResourceSet, FocusElement, EnableElement, ElementFromGadget, VirtualListView (Register), NavReference (Init), IDWriteTextRenderer/IDWritePixelSnapping, WLEditT, BorderSplitter, etc. |
| **WLXPhotoBase.dll** | 14 | Exception@Base (constructor, copy constructor, destructor, operator J), Base::Throw, Base::ThrowLastError, Base::GetBaseStringManager, Base::GdiplusStatusToHresult, BasePrivate::New, BasePrivate::Delete, OS::IsWin7OrGreater, OS::IsWin8OrGreater, CPU::GetProcessorCount, ATL::BaseAtlThrow |
| **WLXPhotoSqm.dll** | 13 | Sqm::Startup, Sqm::Shutdown, Sqm::AddToStream (3 overloads), Sqm::Set (2 overloads), Sqm::IsEnabled, Sqm::EnableShipAsserts, Sqm::ReportAppLaunchStatus, Sqm::ReportAppCloseStatus, Sqm::SetOptInPreference, Sqm::GetOptInState |
| **MetadataSys.dll** | 1 | WLXPSGetItemPropertyHandler |
| **DmxBici.dll** | 5 | BiciWrapper::StartExperience, BiciWrapper::EndExperience, BiciWrapper::AddToDataPoint (2 overloads), BiciWrapper::AddStringToDataPoint, BiciWrapper::TransferExperienceToWeb |
| **wlidcli.dll** | 7 (ordinals) | Ordinals 2, 3, 8, 29, 41, 108, 113 (Windows Live ID client) |
| **uxctl.dll** | 3 | UxControlsInitProcess, UxControlsCreateObject, UxControlsUninitProcess |

### 4.6 Storage/Database (1 DLL, 31 functions)

| DLL | Imports | Key Functions |
|-----|---------|---------------|
| **ESENT.dll** | 31 | JetCreateInstanceW, JetInit2, JetBeginSessionW, JetCreateDatabaseW, JetOpenDatabaseW, JetAttachDatabaseW, JetDetachDatabaseW, JetCloseDatabase, JetCreateTableW, JetOpenTableW, JetCloseTable, JetAddColumnW, JetCreateIndex2W, JetSetCurrentIndexW, JetBeginTransaction, JetCommitTransaction, JetRollback, JetRetrieveColumn, JetSetColumn, JetPrepareUpdate, JetUpdate, JetSeek, JetMakeKey, JetMove, JetDelete, JetGetColumnInfoW, JetGetDatabaseInfoW, JetGetSystemParameterW, JetSetSystemParameterW, JetEndSession, JetTerm |

### 4.7 Other Libraries (7 DLLs, 50 functions)

| DLL | Imports | Key Functions |
|-----|---------|---------------|
| **SHLWAPI.dll** | 26 | PathFileExistsW, PathCanonicalizeW, PathFindFileNameW, PathAppendW, PathRemoveBackslashW, PathAddBackslashW, PathIsRelativeW, PathIsDirectoryW, PathIsFileSpecW, PathIsNetworkPathW, PathIsURLW, PathStripToRootW, PathRenameExtensionW, PathRemoveExtensionW, PathRemoveFileSpecW, PathFindExtensionW, PathParseIconLocationW, StrCmpIW, StrRChrW, SHCreateStreamOnFileW, SHRegGetUSValueW, SHRegOpenUSKeyW, SHRegCloseUSKey, SHRegEnumUSValueW, SHDeleteValueW, SHStrDupW, AssocGetPerceivedType |
| **PROPSYS.dll** | 8 | PSGetPropertyKeyFromName, VariantToPropVariant, PropVariantChangeType, PSCreateMemoryPropertyStore, PropVariantToInt32, PropVariantToStringWithDefault, PropVariantToUInt32, PropVariantToUInt32WithDefault |
| **XmlLite.dll** | 4 | CreateXmlReader, CreateXmlWriter, CreateXmlReaderInputWithEncodingName, CreateXmlWriterOutputWithEncodingName |
| **VERSION.dll** | 3 | GetFileVersionInfoSizeW, GetFileVersionInfoW, VerQueryValueW |
| **WINMM.dll** | 1 | PlaySoundW |
| **WindowsCodecs.dll** | 1 | WICConvertBitmapSource |

### 4.8 Import Summary by Category

| Category | DLLs | Functions |
|----------|------|-----------|
| Core Windows | 5 | 356 |
| COM/OLE | 3 | 47 |
| Media Foundation | 2 | 21 |
| DirectX/Graphics | 9 | 36 |
| Windows Live / Microsoft | 7 | 235 |
| Storage (ESENT) | 1 | 31 |
| Path/String/Properties | 3 | 38 |
| XML/Versioning/Audio | 3 | 8 |
| Imaging | 1 | 1 |
| **Total** | **34** | **773** |

### 4.9 Complete Import Function List by DLL

#### VERSION.dll (3)
```
GetFileVersionInfoSizeW, GetFileVersionInfoW, VerQueryValueW
```

#### MSVCR110.dll (93)
```
_wcsicmp_l, _swprintf_s_l, _wcstoui64, cos, sin, tan, _isnan, atol,
strchr, isspace, strstr, atoi, _except_handler4_common,
__clean_type_info_names_internal, __crtTerminateProcess,
__crtUnhandledException, _crt_debugger_hook, _onexit, __dllonexit,
_calloc_crt, _unlock, _lock, ??1type_info@@UAE@XZ, ?terminate@@YAXXZ,
_initterm_e, _initterm, _malloc_crt, _amsg_exit, __CppXcptFilter,
_wtol, isdigit, memcpy, _wcsnicmp, sqrt, _wtoi, _wtof, wcspbrk,
strlen, _localtime64_s, _beginthreadex, wcstoul, wcsncmp, labs,
qsort_s, _time64, bsearch_s, iswdigit, wcstol, swscanf_s,
_vsnwprintf_l, _wcstoi64, _HUGE, atan2, bsearch, abs, rand, ceil,
isalnum, fmod, memcmp, _vsnwprintf, log, wcsrchr, iswspace, wcschr,
floor, _controlfp, srand, wcsncpy_s, qsort, fabs, _vscwprintf,
vswprintf_s, _wcstod_l, _free_locale, _wcstoul_l, _create_locale,
towupper, wcsstr, _wcsicmp, malloc, _wcslwr_s, wcscspn, wcsspn,
wcsnlen, _swscanf_s_l, swprintf_s, _recalloc, wcscmp, memmove,
calloc, free, wmemcpy_s, memmove_s, memcpy_s, wcslen, _vsnprintf,
_controlfp_s, wcscpy_s, _splitpath_s, _makepath_s, _purecall, memset,
exp, log10, pow, _CxxThrowException, __CxxFrameHandler3, _stricmp
```

#### SHELL32.dll (17)
```
SHGetDesktopFolder, SHCreateShellItem, SHCreateShellItemArrayFromShellItem,
SHGetFolderPathW, SHParseDisplayName, SHOpenFolderAndSelectItems,
CommandLineToArgvW, SHGetDiskFreeSpaceExW, Ordinal 88,
ShellExecuteW, SHGetKnownFolderPath, Ordinal 165,
SHCreateItemFromParsingName, ShellExecuteExW, Ordinal 18,
SHBindToParent, SHOpenWithDialog, Ordinal 75, Ordinal 51,
SHGetPropertyStoreFromParsingName
```

#### KERNEL32.dll (117)
```
FindResourceW, FindResourceExW, GetProcessHeap, HeapAlloc, HeapReAlloc,
HeapFree, GetCurrentThreadId, GetModuleHandleA, GetModuleHandleW,
GetProcAddress, GetVersion, GetFileAttributesW, LoadResource,
GetModuleFileNameW, OutputDebugStringA, EnterCriticalSection,
LeaveCriticalSection, RaiseException, GetCurrentProcess,
FlushInstructionCache, FreeLibrary, lstrcmpiW, MultiByteToWideChar,
LoadLibraryExW, InitializeCriticalSectionAndSpinCount,
DisableThreadLibraryCalls, GetTickCount, HeapSetInformation, Sleep,
CreateFileMappingW, MapViewOfFile, UnmapViewOfFile, CloseHandle,
OpenFileMappingW, DeleteFileW, GetFileAttributesExW, GetLocaleInfoEx,
FormatMessageW, LocalFree, GetThreadUILanguage, CreateMutexW,
GetTempPathW, SetThreadExecutionState, GetVersionExW, GetDateFormatEx,
GetTimeFormatEx, SetFileAttributesW, MoveFileExW, GetFullPathNameW,
CreateFileW, GetTempFileNameW, GetNumberFormatEx, LocaleNameToLCID,
lstrcmpW, GetLocaleInfoW, GetSystemDirectoryW, CreateProcessW,
ExpandEnvironmentStringsW, IsWow64Process, GetCurrentProcessId,
FindClose, FindNextFileW, FindFirstFileExW, CompareStringW,
QueryPerformanceFrequency, QueryPerformanceCounter, GetDateFormatW,
GetUserDefaultLocaleName, QueueUserWorkItem, CreateEventW, SetEvent,
WaitForSingleObject, GetExitCodeThread, CreateDirectoryW,
WaitForMultipleObjects, ResetEvent, CopyFileW,
InterlockedCompareExchange, InitializeSListHead,
InterlockedPopEntrySList, InterlockedPushEntrySList,
IsProcessorFeaturePresent, VirtualAlloc, VirtualFree, DecodePointer,
HeapDestroy, HeapSize, EncodePointer, IsDebuggerPresent,
GetSystemTimeAsFileTime, GetTickCount64, CreateSemaphoreW,
GetSystemInfo, TerminateThread, InterlockedExchange,
PostQueuedCompletionStatus, GetQueuedCompletionStatus,
CreateIoCompletionPort, LockResource, SizeofResource, GetLastError,
SetLastError, DeleteCriticalSection, InterlockedDecrement,
InterlockedIncrement, MulDiv, WideCharToMultiByte, WriteFile,
FindResourceA, TerminateProcess, PeekNamedPipe, SetPriorityClass,
GetExitCodeProcess, OpenProcess, SetHandleInformation, CreatePipe,
LoadLibraryW, ReleaseSemaphore, LocalAlloc, ReleaseMutex, GlobalFree,
GetSystemTime, ReadFile, SetThreadLocale, SetThreadPreferredUILanguages,
GetUserPreferredUILanguages, SetThreadPriority, GetFileSizeEx,
LoadLibraryA, GlobalSize, GlobalUnlock, GlobalLock, GlobalAlloc,
SystemTimeToTzSpecificLocalTime, SystemTimeToFileTime,
FileTimeToSystemTime, CreateThread, CompareFileTime, SetFileTime,
GetFileTime, DebugBreak, WakeAllConditionVariable, WakeConditionVariable,
SleepConditionVariableCS, InitializeConditionVariable,
InterlockedFlushSList, ReplaceFileW, TryEnterCriticalSection,
InterlockedCompareExchange64, InitializeCriticalSection, lstrlenW,
DuplicateHandle
```

#### USER32.dll (84)
```
PostMessageW, SetWindowPos, UpdateWindow, IsWindow, DestroyWindow,
MessageBeep, RegisterClassExW, GetClassInfoExW, LoadCursorW,
CreateWindowExW, SetWindowLongW, GetClientRect, FillRect, BeginPaint,
EndPaint, SetFocus, EnumDisplayDevicesW, EnumDisplaySettingsW,
DefWindowProcW, GetWindowLongW, CallWindowProcW, CharNextW,
GetActiveWindow, FindWindowW, IsWindowEnabled, IsIconic,
GetProcessDefaultLayout, EnumDisplayMonitors, GetParent, CallMsgFilterW,
MsgWaitForMultipleObjectsEx, WindowFromDC, GetWindowThreadProcessId,
GetForegroundWindow, DestroyIcon, GetIconInfo, GetKeyboardState,
FindWindowExW, SetRect, GetAncestor, ReleaseDC, GetDC, ShowWindow,
SetForegroundWindow, GetLastActivePopup, SendMessageW, GetSysColor,
GetWindowRect, MoveWindow, SetWindowTextW, AdjustWindowRectEx,
MapWindowPoints, OffsetRect, FlashWindowEx, SetCursor,
UnregisterDeviceNotification, SetProcessDefaultLayout, LoadIconW,
SetTimer, SetPropW, TranslateMessage, DispatchMessageW,
RegisterDeviceNotificationW, AllowSetForegroundWindow, KillTimer,
GetMessagePos, WindowFromPoint, IsChild, CreatePopupMenu,
TrackPopupMenu, GetMenuItemCount, InsertMenuItemW, DestroyMenu,
MonitorFromPoint, GetMonitorInfoW, SendMessageTimeoutW,
ChangeClipboardChain, SetClipboardViewer, LoadStringW, MessageBoxW,
IsZoomed, PeekMessageW, SystemParametersInfoW, PostQuitMessage, GetFocus,
RegisterClipboardFormatW, ScreenToClient, GetKeyState, PtInRect,
GetCursorPos, IntersectRect, InflateRect, IsRectEmpty, TrackPopupMenuEx,
GetSubMenu, EnableMenuItem, GetSystemMetrics, SetWindowRgn,
SetWindowPlacement, RemovePropW, GetWindowPlacement, MonitorFromWindow,
BringWindowToTop, GetTopWindow, AnimateWindow, ShowCursor, LoadImageW,
EnableWindow, MsgWaitForMultipleObjects, UnregisterClassW,
GetDesktopWindow, IsWindowVisible
```

#### ADVAPI32.dll (21)
```
RegOpenCurrentUser, TraceMessage, GetTraceLoggerHandle,
GetTraceEnableLevel, GetTraceEnableFlags, RegisterTraceGuidsW,
UnregisterTraceGuids, RegDeleteKeyW, RegCloseKey, RegQueryInfoKeyW,
RegEnumKeyExW, RegOpenKeyExW, RegSetValueExW, RegCreateKeyExW,
RegDeleteValueW, RegQueryValueExW, CryptAcquireContextW,
CryptReleaseContext, CryptSignHashW, CryptHashData, CryptCreateHash,
CryptDestroyHash, CryptImportKey, CryptDestroyKey, RegGetValueW, TraceEvent
```

#### GDI32.dll (17)
```
DeleteDC, DeleteObject, CreateSolidBrush, SelectObject,
CreateCompatibleDC, BitBlt, GetDIBits, GetObjectW, CreateDIBSection,
GetDeviceCaps, GetStockObject, GetLayout, SetLayout, GetClipRgn,
ExtCreateRegion, CreateRectRgnIndirect, OffsetRgn, GetRegionData
```

#### ole32.dll (23)
```
PropVariantCopy, CoAllowSetForegroundWindow, GetHGlobalFromStream,
CoCreateInstance, OleIsCurrentClipboard, OleGetClipboard, OleSetClipboard,
CoSetProxyBlanket, CoUninitialize, CoInitialize, PropVariantClear,
StgOpenStorage, OleInitialize, OleUninitialize, CoInitializeEx,
CoTaskMemFree, StringFromGUID2, CLSIDFromString, StringFromCLSID,
CreateStreamOnHGlobal, CoTaskMemRealloc, CoTaskMemAlloc,
ReleaseStgMedium, CoGetObject
```

#### OLEAUT32.dll (22 ordinals)
```
Ord 10, Ord 2, Ord 12, Ord 9, Ord 8, Ord 147, Ord 6, Ord 277,
Ord 194, Ord 26, Ord 411, Ord 27, Ord 16, Ord 162, Ord 161, Ord 7,
Ord 146, Ord 4, Ord 150, Ord 149, Ord 185, Ord 184
```

#### WLXPhotoBase.dll (14)
```
??1Exception@Base@@UAE@XZ, ?Throw@Base@@YGXJ@Z,
??BException@Base@@QBEJXZ, ?GetBaseStringManager@String@Base@@SGAAVCAtlStringMgr@ATL@@XZ,
?ThrowLastError@Base@@YGXXZ, ?GdiplusStatusToHresult@Base@@YGJH@Z,
??0Exception@Base@@IAE@J@Z, ??0Exception@Base@@QAE@ABV01@@Z,
?IsWin8OrGreater@OS@Base@@YG_NXZ, ?IsWin7OrGreater@OS@Base@@YG_NXZ,
?Delete@BasePrivate@@YAXPAX@Z, ?New@BasePrivate@@YAPAXI_N@Z,
?GetProcessorCount@CPU@Base@@YGHXZ, ?BaseAtlThrow@ATL@@YGXJ@Z
```

#### UXCore.dll (192)
```
OnInitIcon@CDUIDialog, OnDetach@CDUIDialog, OnSaveHiddenState@CDUIDialog,
OnLoadHiddenState@CDUIDialog, OnCreateRegion@CFramelessHost,
OnUpdateFrame@CFramelessHost, OnDefaultFrameColorChanged@CFramelessHost,
SetDefaultFocus@NativeHWNDHost, DestroyWindow@NativeHWNDHost,
Destroy@CDUIDialog, OnCommand@CDUIDialog, ExitDialog@CDUIDialog,
OnMessage@CDUIDialog, FontSizeProp@Element, GetDialogHWND@CDUIDialog,
LayoutPosProp@Element, FindDescendent@Element, ??1CDUIDialog,
??0CDUIDialog, Class@Element, IDProp@Element, DirectionProp@Element,
DesiredSizeProp@Element, _UpdateDesiredSize@Element,
MinSizeProp@Element, StrToID, UXCoreUnInitProcess,
UXCoreUnInitThread, GetMessageEx, PeekMessageEx, UXCoreInitThread,
_UpdateLayoutSize@Element, _UpdateLayoutPosition@Element,
SetKeyFocus@VirtualLayout, InvalidateElement, ExtentProp@Element,
DoModal@CDUIDialog, SetDataContext@Element, Host@NativeHWNDHost,
ChildrenProp@Element, Create@PopupWindow, Add@Element, Insert@Element,
FontStyleProp@Element, FontWeightProp@Element, EnabledProp@Element,
VisibleProp@Element, AccNameProp@Element, ContentProp@Element,
CreateInt@Value, SetValue@Element, ??0CRMStringResource,
OnSize@CDUIDialog, Load@CRMStringResource, Length@CRMStringResource,
ToString@CRMStringResource, SelectionProp@Selector,
Clicked@MenuItem, KeyboardNavigate@Element, ParserRequest@Element,
CmdIDProp@Element, Click@Button, DuiCreateObject,
GetValue@Element, KeyFocusedProp@Element, _ZeroRelease@Value,
CreateString@Value, CreateBool@Value, pvUnset@Value,
StartDefer@Element, EndDefer@Element, SizeInLayoutProp@Element,
ParentProp@Element, SetGadgetStyle, LayerManagerUnInitThread,
LayerManagerInitThread, DuiGetLayerManager, ForegroundProp@Element,
OnCreate@CDUIDialog, BackgroundProp@Element,
WrapKeyboardNavigateProp@HWNDElement, AccRoleProp@Element,
AccessibleProp@Element, Class@Button, PressedProp@Button,
RMUpdateResourceSet, ??0CRMResource, Class@DialogHost,
FrameTitleProp@DialogHost, Create@FillLayout, OnDestroy@CDUIDialog,
OnNCDestroy@CDUIDialog, OnActivate@CDUIDialog, OnSysCommand@CDUIDialog,
OnInitDialog@CDUIDialog, PositionProp@Slider, GetDataContext@Element,
OnPostCreateDialog@CDUIDialog, UrlProp@Hyperlink, Class@Hyperlink,
GetKeyFocusedElement@HWNDElement, Resize@BorderSplitter,
ExecCmd@Element, ReLayout@Label, CreateLayout@Value,
LayoutProp@Element, Create@HWNDElement, QIBehaviors@Element,
RMLoadString, ActiveProp@Element, Load@CRMDUIParser,
Register@VirtualListView, ??1CRMResource,
Initialize@NativeHWNDHost, AttachWndProc, DetachWndProc,
YProp@Element, HeightProp@Element, KeyWithinProp@Element,
IsDescendent@Element, Class@HWNDHost, MouseWithinProp@Element,
GetRootHWND@HWNDElement, GetTopHWNDParent, EnsureVisible@Element,
Remove@Element, FocusElement, EnableElement,
SelectionChange@Selector, Class@Selector, CreateElementRef@Value,
DestroyAll@Element, FindDialogElement@CDUIDialog,
Class@Edit, Class@Combobox, ItemsProp@Combobox,
SelectionProp@Combobox, LostFocus@Edit, Change@Edit,
SelectionChange@Combobox, TooltipProp@Element, PosInLayoutProp@Element,
LoadFromResource@CRMImage, ElementFromGadget,
ReleaseElementDC@Element, GetElementDC@Element, Class@WLEditT,
GetContentSize@WLEditT, GetPosFromChar@WLEditT,
GetLayer@Element, Detach@CRMImage, CreateGraphic@Value,
ConvertToARGB@CRMImage, Attach@CRMImage, ??1CRMImage, ??0CRMImage,
CreateColor@Value, FontFaceProp@Element, ContentAlignProp@Element,
Class@Thumb, LocationProp@Element, RMLoadMenu, Class@TabButton,
Class@TabPage, Class@TabControl, Class@Checkbox, Class@ButtonText,
InsertItem@SuperPopup, SetNoPrefixOption@SuperPopup,
CreatePopupMenu@SuperPopup, Create@SuperPopup,
SetFocusOnChild@TabControl, CheckedProp@Checkbox,
UpdateStateProp@Checkbox, External@PopupMenu2, Class@Slider,
ClassProp@Element, AccDescProp@Element, MinimumProp@Slider,
MaximumProp@Slider, FireEvent@Element, Class@VirtualLayout,
AnimationsEnabledProp@VirtualLayout,
LayoutModeInterfaceProp@VirtualLayout,
TemplateSourceProp@VirtualLayout, CreateUnknown@Value,
DataSourceProp@VirtualLayout, CreateValueList@Value, pvNull@Value,
GetTemplate@VirtualLayout, SetNextKeyFocus@VirtualLayout,
PaddingProp@Element, TemplateSizeProp@VirtualLayout,
MouseFocusedProp@Element, SelectedProp@Element, BorderColorProp@Element,
GetGadgetSize, RMFindModuleForResource, Class@ScrollBar,
GetGadgetRect, LineProp@ScrollBar, MaximumProp@ScrollBar,
PositionProp@ScrollBar, MapElementPoint@Element,
XProp@Element, UxGetClassObject, MapGadgetPoints,
FindConnectionPoint@IDuiDataSourceImpl,
EnumConnectionPoints@IDuiDataSourceImpl, RemoveItems@IDuiDataSourceImpl,
AddItems@IDuiDataSourceImpl, RetrieveItems@IDuiDataSourceImpl,
GetLength@IDuiDataSourceImpl, CanModifyList@IDuiDataSourceImpl,
FireRemoved@IDuiDataSourceImpl, FireAdded@IDuiDataSourceImpl,
FireReset@IDuiDataSourceImpl, ??1IDuiDataSourceImpl,
??0IDuiDataSourceImpl, RMLoadStringBSTR, RMLoadImage,
GetTemplateIndex@VirtualLayout, GetLastKeyFocusedTemplate@VirtualLayout,
TooltipString@Element, Init@NavReference, Class@HWNDElement,
??1CRMStringResource, Class@Label, ??0CRMResource, RMFindModule,
??1CRMDUIParser, ??0CRMDUIParser, CreateFloat@Value,
CreateRect@Value, CreateSize@Value, WidthProp@Element,
FilterMessage@CDUIDialog, OnClose@CDUIDialog, ShowDialog@CDUIDialog,
OnGetMinMaxInfo@CDUIDialog, OnShowWindow@CDUIDialog,
GetDUIParser@CDUIDialog
```

#### SHLWAPI.dll (26)
```
PathIsFileSpecW, PathParseIconLocationW, Ordinal 158, PathIsURLW,
PathRemoveBackslashW, StrRChrW, StrCmpIW, AssocGetPerceivedType,
SHRegEnumUSValueW, SHRegGetUSValueW, SHRegOpenUSKeyW, SHStrDupW,
SHCreateStreamOnFileW, PathCanonicalizeW, PathStripToRootW,
PathIsRelativeW, PathAppendW, SHDeleteValueW, PathRenameExtensionW,
PathRemoveExtensionW, PathFindFileNameW, PathIsNetworkPathW,
PathRemoveFileSpecW, PathAddBackslashW, PathFileExistsW,
PathFindExtensionW, SHRegCloseUSKey, PathIsDirectoryW
```

#### WINMM.dll (1)
```
PlaySoundW
```

#### WLXPhotoSqm.dll (13)
```
Sqm::AddToStream (K, K), Sqm::Startup, Sqm::Shutdown,
Sqm::EnableShipAsserts, Sqm::ReportAppLaunchStatus,
Sqm::ReportAppCloseStatus, Sqm::AddToStream (K, Tuple),
Sqm::IsEnabled, Sqm::Set (K, W), Sqm::Set (K, K),
Sqm::AddToStream (K, W), Sqm::GetOptInState,
Sqm::SetOptInPreference
```

#### MF.dll (2)
```
MFGetService, MFTranscodeGetAudioOutputAvailableTypes
```

#### MFPlat.DLL (19)
```
MFCreateWaveFormatExFromMFMediaType, MFCreateMemoryBuffer,
MFCreateMediaType, MFCreateAlignedMemoryBuffer, MFCreateSample,
MFInitMediaTypeFromWaveFormatEx, MFShutdown, MFCalculateImageSize,
MFCreateCollection, MFGetPluginControl, MFCreateSourceResolver,
MFInvokeCallback, MFLockPlatform, MFUnlockPlatform,
MFPutWorkItemEx, MFCreateAttributes,
MFFrameRateToAverageTimePerFrame, MFAverageTimePerFrameToFrameRate,
MFStartup
```

#### XmlLite.dll (4)
```
CreateXmlWriterOutputWithEncodingName, CreateXmlWriter,
CreateXmlReader, CreateXmlReaderInputWithEncodingName
```

#### gdiplus.dll (24)
```
GdipDisposeImage, GdipCloneImage, GdipSetClipHrgn,
GdipCreateBitmapFromStream, GdipCreateFont,
GdipCreateFontFamilyFromName, GdipCreateSolidFill,
GdipGetGenericFontFamilySansSerif, GdipDeleteFont, GdipDeleteBrush,
GdipSetRenderingOrigin, GdipTranslateWorldTransform,
GdipGetImageHeight, GdipGetImageWidth, GdipFree, GdipAlloc,
GdipDeleteGraphics, GdipCreateFromHDC, GdiplusShutdown,
GdiplusStartup, GdipCloneBrush, GdipFillRectangle,
GdipDeleteFontFamily, GdipDrawImageRectRect,
GdipGetFontHeightGivenDPI
```

#### WindowsCodecs.dll (1)
```
WICConvertBitmapSource
```

#### dwmapi.dll (1)
```
DwmExtendFrameIntoClientArea
```

#### DmxBici.dll (5)
```
BiciWrapper::AddStringToDataPoint, BiciWrapper::EndExperience,
BiciWrapper::StartExperience, BiciWrapper::TransferExperienceToWeb,
BiciWrapper::AddToDataPoint
```

#### MetadataSys.dll (1)
```
WLXPSGetItemPropertyHandler
```

#### ESENT.dll (31)
```
JetCommitTransaction, JetBeginTransaction, JetGetColumnInfoW,
JetRollback, JetDelete, JetMove, JetGetDatabaseInfoW,
JetGetSystemParameterW, JetTerm, JetEndSession, JetCreateIndex2W,
JetAddColumnW, JetCreateTableW, JetCreateDatabaseW,
JetBeginSessionW, JetInit2, JetUpdate, JetSetColumn,
JetPrepareUpdate, JetSeek, JetMakeKey, JetSetCurrentIndexW,
JetCloseDatabase, JetDetachDatabaseW, JetAttachDatabaseW,
JetOpenDatabaseW, JetCreateInstanceW, JetSetSystemParameterW,
JetCloseTable, JetOpenTableW, JetRetrieveColumn
```

#### wlidcli.dll (7 ordinals)
```
Ordinal 108, Ordinal 41, Ordinal 2, Ordinal 8,
Ordinal 3, Ordinal 29, Ordinal 113
```

#### PROPSYS.dll (8)
```
PSGetPropertyKeyFromName, VariantToPropVariant,
PropVariantChangeType, PSCreateMemoryPropertyStore,
PropVariantToInt32, PropVariantToStringWithDefault,
PropVariantToUInt32, PropVariantToUInt32WithDefault
```

#### uxctl.dll (3)
```
UxControlsUninitProcess, UxControlsCreateObject,
UxControlsInitProcess
```

#### UxTheme.dll (3)
```
OpenThemeData, GetThemeMetric, CloseThemeData
```

#### d3d11.dll (1)
```
D3D11CreateDevice
```

#### d2d1.dll (1 ordinal)
```
Ordinal 1 (D2D1CreateFactory)
```

#### DWrite.dll (1)
```
DWriteCreateFactory
```

#### d3d9.dll (2)
```
Direct3DCreate9Ex, Direct3DCreate9
```

#### dxva2.dll (2)
```
DXVA2CreateDirect3DDeviceManager9, DXVA2CreateVideoService
```

#### OLEACC.dll (2)
```
AccessibleChildren, AccessibleObjectFromWindow
```

#### D3DCOMPILER_46.dll (2)
```
D3DGetInputSignatureBlob, D3DReflect
```

## 5. RTTI Class Inventory — 1,018 Unique Types

### 5.1 Type Distribution

| Type Prefix | Count | Purpose |
|-------------|-------|---------|
| `?AV` (Classes) | 683 | Concrete and abstract classes with vtables |
| `?AU` (Structs) | 335 | C++ structs, COM interfaces, D3DX11 types |
| **Total** | **1,018** | |

### 5.2 Namespace Distribution (from class naming patterns)

| Namespace/Category | Type Count | Purpose |
|--------------------|-----------|---------|
| **(global / HMREngine)** | ~232 | 3D rendering engine (X3D, effects, shaders, textures) |
| **StoryboardManagerNamespace** | ~141 | Timeline/project/storyboard management |
| **D3DX11Effects** (S*/SEffectInvalid*) | ~94 | D3DX11 effect framework internal types |
| **CCodec_* (D3DX11Texture)** | ~107 | D3D11 texture codec classes (DXGI format variants) |
| **UI/DUI/Sundance** | ~91 | DirectUI behaviors, dialogs, application framework |
| **X3D** | ~35 | VRML/X3D 9777 node implementations |
| **Serialization/Bound** | ~34 | Serialization, property binding, data containers |
| **PatternMesh** | ~28 | Transition shape meshes |
| **Media/Transcode/Encode** | ~40 | Audio/video source, encoding, transcoding |
| **DX/Texture** | ~44 | Direct3D texture management, render targets |
| **EffectResource/Shader** | ~25 | Visual effect resources and shader implementations |
| **Theme** | ~25 | Theme system for project styling |
| **Publish** | ~9 | Publishing/sharing functionality |
| **Timeline** | ~14 | Timeline UI behaviors and data |
| **Audio** | ~12 | Audio processing, capture, output |
| **Render** | ~12 | Rendering pipeline |
| **ATL/Base/Gdiplus/Other** | ~93 | Framework utilities, exceptions, wrappers |

### 5.3 Complete RTTI Class List (?AV — 683 classes, alphabetically)

```
?AV_IDispEvent
?AVAboveHomerBehavior
?AVAMPCommandBarBehavior
?AVAMPDataContext
?AVAmpFadeAnimationBehavior
?AVAMPMainWindowBehavior
?AVAmpPreventSparseModeBehavior
?AVAmpSlideAnimationBehavior
?AVAnimatedEffectResourceBase
?AVAppearanceImpl
?AVAreaOfInterestGenerator
?AVAsyncSourceResolver
?AVAsyncWorkerObject
?AVAudioBoost
?AVAudioCaptureSession
?AVAudioClip
?AVAudioClipImpl
?AVAudioDuckingProperties
?AVAudioOutput
?AVAudioOutputCommandParam
?AVAudioQueue
?AVAudioResamplerHelper
?AVAudioRMSData
?AVAudioStreamSink
?AVAudioVideoMediaClip
?AVAuthCredentials
?AVAuthProvider
?AVAutofitProcess
?AVAutoSaveManager
?AVAVCaptureCore
?AVAVCaptureSession
?AVAVData
?AVAVResObj
?AVAVResource
?AVAVResourceClock
?AVAVResourceDX
?AVAVSink
?AVAVSource
?AVAVSourceFactory
?AVAVSourceProxy
?AVBackBufferDX
?AVBackgroundOrchestrator
?AVBaseBackgroundRequest
?AVBaseTemplate
?AVBaseX3DTemplate
?AVBitmap
?AVBlurEffectResource
?AVBlurShaderImpl
?AVBooleanFilterImpl
?AVBoundPlaceholder
?AVBoundPlaceholderMapContainer
?AVBoundPlaceholderSerializer
?AVBoundPropertiesDictionaryContainer
?AVBoundProperty
?AVBoundPropertyBool
?AVBoundPropertyDictionary
?AVBoundPropertyFloat
?AVBoundPropertyFloatElement
?AVBoundPropertyFloatSet
?AVBoundPropertyInt
?AVBoundPropertyString
?AVBoundPropertyStringElement
?AVBoundPropertyStringSet
?AVBowTiePatternMesh
?AVBrightnessEffectResource
?AVBrightnessShaderImpl
?AVBrowser
?AVBrush
?AVBufferedPainter
?AVBufferedPainterBase
?AVCachedWFSection
?AVCaptureUIBehavior
?AVCaretBehavior
?AVCaretScrollBehavior
?AVCAsyncMemoryLoader
?AVCAsyncTextureInfoProcessor
?AVCAsyncTextureProcessor
?AVCAtlModule
?AVCCodec
?AVCCodec_A1R5G5B5
?AVCCodec_A2R10G10B10
?AVCCodec_A2W10V10U10
?AVCCodec_A4L4
?AVCCodec_A4R4G4B4
?AVCCodec_A8_UNORM
?AVCCodec_A8L8
?AVCCodec_A8P8
?AVCCodec_A8R3G3B2
?AVCCodec_A8R8G8B8
?AVCCodec_B8G8R8A8_UNORM
?AVCCodec_B8G8R8A8_UNORM_SRGB
?AVCCodec_B8G8R8X8_UNORM
?AVCCodec_B8G8R8X8_UNORM_SRGB
?AVCCodec_BC1_UNORM
?AVCCodec_BC1_UNORM_SRGB
?AVCCodec_BC2_UNORM
?AVCCodec_BC2_UNORM_SRGB
?AVCCodec_BC3_UNORM
?AVCCodec_BC3_UNORM_SRGB
?AVCCodec_BC4_SNORM
?AVCCodec_BC4_UNORM
?AVCCodec_BC5_SNORM
?AVCCodec_BC5_UNORM
?AVCCodec_BC6H_SF16
?AVCCodec_BC6H_UF16
?AVCCodec_BC7_UNORM
?AVCCodec_BC7_UNORM_SRGB
?AVCCodec_CxV8U8
?AVCCodec_D16_LOCKABLE
?AVCCodec_D16_UNORM
?AVCCodec_D24_UNORM_S8_UINT
?AVCCodec_D32_FLOAT
?AVCCodec_D32_FLOAT_S8X24_UINT
?AVCCodec_D32F_LOCKABLE
?AVCCodec_D3DX_A16L16
?AVCCodec_D3DX_R16G16B16
?AVCCodec_G8R8_G8B8
?AVCCodec_G8R8_G8B8_UNORM
?AVCCodec_L16
?AVCCodec_L6V5U5
?AVCCodec_L8
?AVCCodec_P8
?AVCCodec_Q16W16V16U16
?AVCCodec_Q8W8V8U8
?AVCCodec_R1_UNORM
?AVCCodec_R10G10B10_XR_BIAS_A2_UNORM
?AVCCodec_R10G10B10A2_UINT
?AVCCodec_R10G10B10A2_UNORM
?AVCCodec_R11G11B10_FLOAT
?AVCCodec_R16_FLOAT
?AVCCodec_R16_SINT
?AVCCodec_R16_SNORM
?AVCCodec_R16_UINT
?AVCCodec_R16_UNORM
?AVCCodec_R16G16_FLOAT
?AVCCodec_R16G16_SINT
?AVCCodec_R16G16_SNORM
?AVCCodec_R16G16_UINT
?AVCCodec_R16G16_UNORM
?AVCCodec_R16G16B16A16_FLOAT
?AVCCodec_R16G16B16A16_SINT
?AVCCodec_R16G16B16A16_SNORM
?AVCCodec_R16G16B16A16_UINT
?AVCCodec_R16G16B16A16_UNORM
?AVCCodec_R24_UNORM_X8_TYPELESS
?AVCCodec_R32_FLOAT
?AVCCodec_R32_FLOAT_X8X24_TYPELESS
?AVCCodec_R32_SINT
?AVCCodec_R32_UINT
?AVCCodec_R32G32_FLOAT
?AVCCodec_R32G32_SINT
?AVCCodec_R32G32_UINT
?AVCCodec_R32G32B32_FLOAT
?AVCCodec_R32G32B32_SINT
?AVCCodec_R32G32B32_UINT
?AVCCodec_R32G32B32A32_FLOAT
?AVCCodec_R32G32B32A32_SINT
?AVCCodec_R32G32B32A32_UINT
?AVCCodec_R3G3B2
?AVCCodec_R5G6B5
?AVCCodec_R8_SINT
?AVCCodec_R8_SNORM
?AVCCodec_R8_UINT
?AVCCodec_R8_UNORM
?AVCCodec_R8G8_B8G8
?AVCCodec_R8G8_B8G8_UNORM
?AVCCodec_R8G8_SINT
?AVCCodec_R8G8_SNORM
?AVCCodec_R8G8_UINT
?AVCCodec_R8G8_UNORM
?AVCCodec_R8G8B8
?AVCCodec_R8G8B8A8_SINT
?AVCCodec_R8G8B8A8_SNORM
?AVCCodec_R8G8B8A8_UINT
?AVCCodec_R8G8B8A8_UNORM
?AVCCodec_R8G8B8A8_UNORM_SRGB
?AVCCodec_R9G9B9E5_SHAREDEXP
?AVCCodec_V16U16
?AVCCodec_V8U8
?AVCCodec_X1R5G5B5
?AVCCodec_X24_TYPELESS_G8_UINT
?AVCCodec_X32_TYPELESS_G8X24_UINT
?AVCCodec_X4R4G4B4
?AVCCodec_X8B8G8R8
?AVCCodec_X8L8V8U8
?AVCCodec_X8R8G8B8
?AVCCodecDXT
?AVCCodecYUV
?AVCColorPickerSite
?AVCComModule
?AVCComObjectRootBase
?AVCDdsLoader
?AVCDUIDialog
?AVCEffect
?AVCFontSite
?AVCFramelessHost
?AVCGallerySite
?AVCGenericSite
?AVCGroupSite
?AVChannelMixerEffectResource
?AVChannelMixerShaderImpl
?AVCheckerboardPatternMesh
?AVChunkMediaFileList
?AVCImageSlice
?AVCirclePatternMesh
?AVCirclesPatternMesh
?AVClipboardChainWindow
?AVClipboardManager
?AVCMessageMap
?AVCMRUItem
?AVCMRUSite
?AVCMsgFilter
?AVColorImpl
?AVColorRGBAImpl
?AVCommandBin
?AVCommandLineMediaFileList
?AVCommandLineParser
?AVCommonEffectResourceDX
?AVComplexIntro
?AVComplexProperty
?AVComposedGeometryResourceDX
?AVCompositeX3DTemplate
?AVConductor
?AVContactStore
?AVContainer
?AVCoordinateImpl
?AVCPaintDC
?AVCRegObject
?AVCSpinnerSite
?AVCSundanceDialog
?AVCTexture
?AVCWinCodecLoader
?AVCWindow
?AVDatabasePropertyManager
?AVDefaultEffectResourceDX
?AVDefaultPreviewDX
?AVDeviceClientDXImpl
?AVDiagonalBoxPatternMesh
?AVDiagonalCrossPatternMesh
?AVDissolveEffectResource
?AVDissolveShaderImpl
?AVDontShowPromptDialog
?AVDShowMFSourceReaderBuilder
?AVDuckingTrackDataSource
?AVDuiLayerBehaviorImpl
?AVDXVA2VideoProc
?AVDynamicDataObjectWrapper
?AVDynamicRouteManager
?AVEdgeDetectionEffectResource
?AVEffectListContainer
?AVEffectResource
?AVEffectResourceDX
?AVEffectTemplateHandler
?AVEffectX3DTemplate
?AVEncodeDX
?AVEncodeInitializationException
?AVEncodeProfile
?AVEngine
?AVEngineDX
?AVEnumerateNodes
?AVErrHandler
?AVEventHandlerData
?AVEventImpl
?AVException
?AVExceptionWithString
?AVExecutionContext
?AVExtension
?AVExtentIdSetSelectionRangeIterator
?AVExtentListRegionIterator
?AVExtentMapContainer
?AVExtentRefCountMultiThreaded
?AVExtentSelector
?AVExtentSelectorValidator
?AVExtentSocketsHandler
?AVExtentX3DTemplate
?AVEyePatternMesh
?AVFadeEffectResource
?AVFadeShaderImpl
?AVFanAndSweepBasePatternMesh
?AVFanAndSweepPatternMesh
?AVFanInPatternMesh
?AVFanOutPatternMesh
?AVFanUpPatternMesh
?AVFieldCollection
?AVFieldImpl
?AVFile
?AVFilenameArrayList
?AVFilenameList
?AVFillPaintContext
?AVFillPropertiesNodeImpl
?AVFillVPatternMesh
?AVFinder
?AVFindFile
?AVFloatVertexAttributeImpl
?AVFogCoordinateImpl
?AVFontStyleImpl
?AVFourCCCode
?AVFrameBufferImplDX
?AVFullscreenBackgroundWindow
?AVFullscreenLayoutBehavior
?AVFullTextRenderTask
?AVGapCountFunctional
?AVGdiException
?AVGdiplusBase
?AVGdiplusStartupWrapper
?AVGraphChanged
?AVGridEffectResource
?AVGridImpl
?AVGridNodeImpl
?AVGridResourceDX
?AVGridShaderImpl
?AVHcdpiBehavior
?AVHcdpiParser
?AVHcdpiResourceCache
?AVHeartPatternMesh
?AVHelpBehavior
?AVHomerHeavyLayerBehavior
?AVHueEffectResource
?AVIContactStore
?AVIContactStoreNotify
?AVIDuiBehaviorImpl
?AVIDuiDataSourceImpl
?AVIDuiHandlerNotifyImpl
?AVIDuiVirtualLayoutModeImpl
?AVImage
?AVImageClip
?AVImageLoaderWrapper
?AVImageResObj
?AVImageTextureImpl
?AVImageThumbnail
?AVIndexedFaceSetImpl
?AVInlinePreviewLayoutBehavior
?AVInlinePreviewSliderBehavior
?AVInvalidAssetFinder
?AVIrisPatternMesh
?AVIsActiveBase
?AVIsolationAwareTaskDialog
?AVITextureLoader
?AVJetException
?AVKeyholePatternMesh
?AVLayerImpl
?AVLayerSetImpl
?AVLegacyExtent
?AVLegacyParagraph
?AVLegacyProjectSupport
?AVLegacyTextExtent
?AVLegacyTransform
?AVLinePropertiesNodeImpl
?AVLocationTagSerializer
?AVMaterialImpl
?AVMathScalarImpl
?AVMathVectorImpl
?AVMatrix3VertexAttributeImpl
?AVMatrix4VertexAttributeImpl
?AVMediaBrowser
?AVMediaClipBase
?AVMediaItem
?AVMediaItemBase
?AVMediaItemExtentFinder
?AVMediaItemExtentHarvester
?AVMediaItemMapContainer
?AVMediaLoadBackgroundRequest
?AVMeshResource
?AVMeshResourceDX
?AVMetadata
?AVMetadataBoolImpl
?AVMetadataDoubleImpl
?AVMetadataFloatImpl
?AVMetadataIntegerImpl
?AVMetadataSetImpl
?AVMetadataStringImpl
?AVMFAsyncResult
?AVMFByteStreamOnStream
?AVMFByteStreamOnStreamAsyncResult
?AVMFRateControlHelper
?AVMFSourceReaderBuilder
?AVMixer
?AVMixerBuffer
?AVModBeginDocument
?AVModBeginElement
?AVModContainer
?AVModContainerWithAttribute
?AVModContainerWithIDLookAhead
?AVModEndElement
?AVMonolithicThemeOperation
?AVMotionTextureImpl
?AVMotionTextureNodeImpl
?AVMotionTextureResource
?AVMotionTextureResourceDX
?AVMovieEffect
?AVMovieExtent
?AVMovieProject
?AVMovieTextureImpl
?AVMovieThumbnail
?AVMovieThumbnailDX
?AVMovieTransport
?AVMTOListContainer
?AVMTOMakeAssetBkgrdForTitleCredit
?AVMTOMoveVisualTrackExtent
?AVMultipleEffectBehavior
?AVMultipleEffectDialog
?AVNarrationUI
?AVNativeHWNDHost
?AVNativeMFSourceReaderBuilder
?AVNodeReference
?AVNoHeap
?AVNormalImpl
?AVOnReadAsyncCallback
?AVOnWriteAsyncCallback
?AVOptionsDialogBehavior
?AVPageCurlGridImpl
?AVPageCurlGridResourceDX
?AVPaintContext
?AVPanAndZoomShapeEffect
?AVParsePhotoGalleryTransferFile
?AVPartialTextPreRenderTask
?AVPartialTextRendererDX
?AVPartialTextRenderTask
?AVPatternMesh
?AVPersonTagSerializer
?AVPhotoDurationAnalyzer
?AVPixelateEffectResource
?AVPixelateShaderImpl
?AVPopUpSlider
?AVPopUpSliderTrackBar
?AVPosterizeEffectResource
?AVPosterizeShaderImpl
?AVPreprocessMesh
?AVPreviewClientPath
?AVPreviewClientPathContainer
?AVPreviewClientTransferFile
?AVPreviewCommandParam
?AVPreviewDataContext
?AVPreviewDX
?AVPreviewPresenterWrapper
?AVPrivateAutoPerfTrace
?AVProgressBase
?AVProgressDialog
?AVProgressStatusBar
?AVProjectWorkspaceLayoutBehavior
?AVProxyInfo
?AVPSAAuthenticationStore
?AVPublishBackgroundJob
?AVPublishBackgroundWorker
?AVPublishItemProperties
?AVPublishItemPropertyStore
?AVPublishJob
?AVPublishManager
?AVPublishProgressCallBack
?AVPublishSessionXmlManifestFactory
?AVPublishSummaryDialog
?AVReadOnlyStreamOnStaticMemory
?AVRectanglePatternMesh
?AVRectanglesPatternMesh
?AVRefCountBase
?AVRefCountBaseMultiThreaded
?AVReloadImage
?AVRenderCommandBin
?AVRenderingList
?AVRenderLoop
?AVRenderLoopDX
?AVRenderSummaryDialog
?AVRenderTransport
?AVResetThumbnail
?AVResizeablePaneBehavior
?AVResObj
?AVResourceCache
?AVResourceCacheDX
?AVRetranscodeRequester
?AVRevealPatternMesh
?AVRibbonApp
?AVRibbonCategoryItem
?AVRibbonCategoryList
?AVRibbonList
?AVRibbonListItem
?AVRichEditControlBehavior
?AVRippleEffectResource
?AVSAIRouteImpl
?AVSAXProfileBuilder
?AVScene
?AVSceneEncode
?AVSceneMergeBackgroundRequest
?AVSceneMergeContext
?AVScenePreview
?AVScriptInitializationException
?AVScrollingTextEffectResourceDX
?AVScrollingTextImpl
?AVScrollingTextResourceDX
?AVScrollingTextShaderImpl
?AVSelectionIndex
?AVSelectionRootImpl
?AVSelectionRootImplWrapper
?AVSelectorExtentRefContainer
?AVSelectorExtentRefSerializer
?AVSelectorFinder
?AVSerializationContext
?AVSerializationMemoryReaderWriter
?AVSerializationReader
?AVSerializationWriter
?AVSFTime
?AVShaderSetImpl
?AVShaderSetNodeImpl
?AVShapeImpl
?AVSharedBitmap
?AVSharedSwapChainDX
?AVShatterGridImpl
?AVShatterGridResourceDX
?AVSimpleCodecEncDecCallbackCore
?AVSimpleEffect
?AVSimpleEffectResourceBase
?AVSimpleIntro
?AVSingleProperty
?AVSingleThread
?AVSnapShot
?AVSnapShotDX
?AVSnapShotExtentMapContainer
?AVSolidBrush
?AVSoundImpl
?AVSplitPatternMesh
?AVSqmStartupWrapper
?AVStandardLayerBehavior
?AVStarPatternMesh
?AVStarsPatternMesh
?AVStartEncodeProxyTranscodesIfNecessary
?AVStoryboardManager
?AVStreamSinkHelper
?AVStreamSinkHost
?AVString
?AVSundanceAppDataContext
?AVSundanceApplicationOptionsDialog
?AVSundanceAppMain
?AVSundanceBehaviorFactory
?AVSundanceClipboardChainWindow
?AVSundanceDontShowPromptDialog
?AVSundanceMainElementBehavior
?AVSundanceNativeHwndHost
?AVSwapChainCommandParam
?AVSweepInPatternMesh
?AVSweepOutPatternMesh
?AVSweepUpPatternMesh
?AVSwitchImpl
?AVSyncVideoSampleSource
?AVTaskFinishNotificationCallback
?AVTempFile
?AVTemplateInitializationException
?AVTemplatePlaceholder
?AVTemplateProperty
?AVTemplateSocket
?AVTemplateSocketsParser
?AVTemplateTable
?AVTextBitmapRenderTask
?AVTextBoxBehavior
?AVTextComposer
?AVTextEffect
?AVTextEffectResourceDX
?AVTextImpl
?AVTextManager
?AVTextProperty
?AVTextRenderTask
?AVTextResourceBaseDX
?AVTextResourceDX
?AVTextShaderImpl
?AVTextureCoordinateImpl
?AVTextureInterOp
?AVTextureInterOpDX11
?AVTextureInterOpDX9
?AVTextureLoadException
?AVTexturePropertiesNodeImpl
?AVTextureResource
?AVTextureResourceDX
?AVTextureResourceFromResourceDX
?AVTextureTransformContainerImpl
?AVTextureTransformImpl
?AVTheme
?AVThemeComplexTitle
?AVThemeComplexType
?AVThemeDependentTrack
?AVThemeEffect
?AVThemeEffectTemplate
?AVThemeFirstEffect
?AVThemeFirstTransition
?AVThemeInterior
?AVThemeIntro
?AVThemeLastEffect
?AVThemeLastTransition
?AVThemeManager
?AVThemeMid
?AVThemeOperationLogger
?AVThemeOutro
?AVThemePrimaryTrack
?AVThemeProject
?AVThemeSimpleElement
?AVThemeSimpleTitle
?AVThemeTitle
?AVThemeTrack
?AVThemeTransition
?AVThemeX3DTemplate
?AVThread
?AVThumbnail
?AVThumbnailPaintContext
?AVTimeDependentNodesEnumeratorImpl
?AVTimelineBaseBehavior
?AVTimelineBehavior
?AVTimelineDataSource
?AVTimelineDragDrop
?AVTimelineExtentUIObject
?AVTimelineInstructionsBehavior
?AVTimelineItemBehavior
?AVTimelineItemHandler
?AVTimelineItemInputBehavior
?AVTimelineLayoutMode
?AVTimelineSecondaryTrackItemBehavior
?AVTimelineSelectionRootBehavior
?AVTimelineTemplateSource
?AVTimelineVisualTrackItemBehavior
?AVTimeSensorImpl
?AVTimeSnapshotRegionIterator
?AVTimeTriggerImpl
?AVTitleClip
?AVToggleSite
?AVTranscodeBackgroundRequest
?AVTranscodeConfig
?AVTranscodeMetadataParser
?AVTranscodeProcess
?AVTransformImpl
?AVTransformProperty
?AVTransitionListContainer
?AVTransitionSerializer
?AVTransparentObject
?AVTransportBase
?AVTrimBehavior
?AVtype_info
?AVUserEncodeProfileBehavior
?AVUserEncodeProfileDialog
?AVUserEncodeProfileInfo
?AVUserEncodeProfileRecommended
?AVUserEncodeProfileWLVS
?AVUXBrush
?AVValueVariantRef
?AVVideoClip
?AVViewpointImpl
?AVViewportImpl
?AVWaveform
?AVWaveformCallback
?AVWebcamElementBehavior
?AVWebcamUI
?AVWheelPatternMesh
?AVWipeEffectResource
?AVWipeMeshResourceDX
?AVWipeShaderImpl
?AVX3DAnimatedShaderNodeImpl
?AVX3DAppearanceNodeImpl
?AVX3DBindableNodeImpl
?AVX3DBoundedObjectImpl
?AVX3DChildNodeImpl
?AVX3DChildObjectImpl
?AVX3DColorNodeImpl
?AVX3DComposedGeometryNodeImpl
?AVX3DCoordinateNodeImpl
?AVX3DGeometryNodeImpl
?AVX3DGroupingNodeImpl
?AVX3DLayerNodeImpl
?AVX3DMaterialNodeImpl
?AVX3DMetadataObjectImpl
?AVX3DNodeImpl
?AVX3DReader
?AVX3DSensorNodeImpl
?AVX3DShaderNodeImpl
?AVX3DShapeNodeImpl
?AVX3DSoundNodeImpl
?AVX3DSoundSourceNodeImpl
?AVX3DSoundSourceObjectImpl
?AVX3DTextNodeImpl
?AVX3DTexture2DNodeImpl
?AVX3DTextureNodeImpl
?AVX3DTimeDependentObjectImpl
?AVX3DUrlObjectImpl
?AVX3DVertexAttributeNodeImpl
?AVX3DViewpointNodeImpl
?AVX3DViewportNodeImpl
?AVXmlManifestFactory
?AVXVideoProc
?AVZigzagPatternMesh
```

### 5.4 Complete RTTI Struct List (?AU — 335 structs, alphabetically)

```
?AU_ATL_MODULE70
?AUAppearance
?AUAudioClip
?AUBlurShader
?AUBooleanFilter
?AUBrightnessShader
?AUChannelMixerShader
?AUColor
?AUColorRGBA
?AUCoordinate
?AUDissolveShader
?AUEdgeDetectionShader
?AUEffectMeshProvider
?AUEvent
?AUFadeShader
?AUFillPropertiesNode
?AUFloatVertexAttribute
?AUFogCoordinate
?AUFontStyle
?AUGrid
?AUGridNode
?AUGridShader
?AUHueShader
?AUIAmpPublishHandler
?AUIAudioBoost
?AUIAudioSessionEvents
?AUIAuthProgressCallback
?AUIAVCaptureSessionCallback
?AUIAVProcessor
?AUIAVSampleSource
?AUIAVSink
?AUIAVSource
?AUIAVSourceFactory
?AUIAVSourceFactoryInternal
?AUIAVStreamSink
?AUIAVTickTimeSource
?AUICaptureSessionUIControl
?AUIClipboardChainCallback
?AUICodecEncDecCallbackCore
?AUIConnectionPointContainer
?AUICopyFields
?AUICreateEngineAsync
?AUID3DX11Effect
?AUID3DX11EffectBlendVariable
?AUID3DX11EffectClassInstanceVariable
?AUID3DX11EffectConstantBuffer
?AUID3DX11EffectDepthStencilVariable
?AUID3DX11EffectDepthStencilViewVariable
?AUID3DX11EffectGroup
?AUID3DX11EffectInterfaceVariable
?AUID3DX11EffectMatrixVariable
?AUID3DX11EffectPass
?AUID3DX11EffectRasterizerVariable
?AUID3DX11EffectRenderTargetViewVariable
?AUID3DX11EffectSamplerVariable
?AUID3DX11EffectScalarVariable
?AUID3DX11EffectShaderResourceVariable
?AUID3DX11EffectShaderVariable
?AUID3DX11EffectStringVariable
?AUID3DX11EffectTechnique
?AUID3DX11EffectType
?AUID3DX11EffectUnorderedAccessViewVariable
?AUID3DX11EffectVariable
?AUID3DX11EffectVectorVariable
?AUIDataObject
?AUIDeviceClientDX
?AUIDeviceManagerDX
?AUIDispatch
?AUIDropTarget
?AUIDuiBehavior
?AUIDuiBehaviorFactory
?AUIDuiDataSelectable
?AUIDuiDataSource
?AUIDuiHandlerDispatch
?AUIDuiHandlerNotify
?AUIDuiTemplateSource
?AUIDuiTimerCallback
?AUIDuiVirtualLayoutMode
?AUIDUserDropDescription
?AUIDWritePixelSnapping
?AUIDWriteTextRenderer
?AUIEncodeProfile
?AUIEngine
?AUIEnumerateNode
?AUIEnumFORMATETC
?AUIExecutionContextEvent
?AUIFadeAnimationBehavior
?AUIFieldCollection
?AUIFrameBuffer
?AUIHomerHeavyLayer
?AUIImageLoaderWrapper
?AUIImageThumbnail
?AUImageTexture
?AUIMFAsyncCallback
?AUIMFAsyncResult
?AUIMFByteStream
?AUIMFSourceReaderCallback
?AUIMMNotificationClient
?AUIMovieThumbnail
?AUIMXSchemaDeclHandler
?AUIndexedFaceSet
?AUIOnlineMediaItemProperties
?AUIOnlineMediaItemPropertyStore
?AUIOnlineMediaProgressCallback
?AUIPreviewPresenter
?AUIProgressCancelCallback
?AUIPSAAuthentication
?AUIRefCounted
?AUIRegistrarBase
?AUIResourceCache
?AUISAXContentHandler
?AUISAXErrorHandler
?AUISceneEncode
?AUIScenePreview
?AUIScrollingTextResource
?AUISelectionRangeIterator
?AUISelectionRoot
?AUISelectionRootProxy
?AUISelectionRootTarget
?AUISequentialStream
?AUIServiceProvider
?AUISharedBitmap
?AUISlideAnimationBehavior
?AUISparseModeActivationHandler
?AUISparseModeBehavior
?AUIStream
?AUISunRibbonCategoryList
?AUISunRibbonList
?AUITextureProviderDX
?AUIThreadPoolConfig
?AUIThumbnail
?AUITimelineExtentUIObject
?AUITimelineItem
?AUITimelineItemHandlerElement
?AUITimelinePreviewPaint
?AUITimelineSecondaryTrackItemBehavior
?AUITransparentObject
?AUIUIApplication
?AUIUICommandHandler
?AUIUIPropertyUpdate
?AUIUISimplePropertySet
?AUIUnknown
?AUIUserEncodeProfileBehavior
?AUIUxIdentityControlCallback
?AUIUxStoryboardNotifications
?AUIValidateBinding
?AUIVideoProcessor
?AUIVideoSampleSource
?AUIWaveform
?AUIWaveformCallback
?AUIWLRMruWriteProperty
?AUIWorkerObjectCallback
?AUIWorkerThreadClient
?AULayer
?AULayerSet
?AULinePropertiesNode
?AUMaterial
?AUMathScalar
?AUMathVector
?AUMatrix3VertexAttribute
?AUMatrix4VertexAttribute
?AUMetadataNodeRemover
?AUMixStream
?AUMotionTextureData
?AUMotionTextureNode
?AUMovieTexture
?AUNormal
?AUPageCurlGrid
?AUPixelateShader
?AUPosterizeShader
?AURegionIterator
?AURipple
?AUSAIBrowserRef
?AUSAIExecutionContext
?AUSAIRouteService
?AUSAIScene
?AUSAnonymousShader
?AUSBaseBlock
?AUSBlendGlobalVariable
?AUSBlendGlobalVariableMember
?AUSBoolScalarAnnotation
?AUSBoolScalarAnnotationMember
?AUSBoolScalarGlobalVariable
?AUSBoolScalarGlobalVariableMember
?AUSBoolVectorAnnotation
?AUSBoolVectorAnnotationMember
?AUSBoolVectorGlobalVariable
?AUSBoolVectorGlobalVariableMember
?AUSClassInstanceGlobalVariable
?AUSClassInstanceGlobalVariableMember
?AUSConstantBuffer
?AUScrollingText
?AUScrollingTextShader
?AUSDepthStencilGlobalVariable
?AUSDepthStencilGlobalVariableMember
?AUSDepthStencilViewGlobalVariable
?AUSDepthStencilViewGlobalVariableMember
?AUSEffectInvalidBlendVariable
?AUSEffectInvalidClassInstanceVariable
?AUSEffectInvalidConstantBuffer
?AUSEffectInvalidDepthStencilVariable
?AUSEffectInvalidDepthStencilViewVariable
?AUSEffectInvalidGroup
?AUSEffectInvalidInterfaceVariable
?AUSEffectInvalidMatrixVariable
?AUSEffectInvalidPass
?AUSEffectInvalidRasterizerVariable
?AUSEffectInvalidRenderTargetViewVariable
?AUSEffectInvalidSamplerVariable
?AUSEffectInvalidScalarVariable
?AUSEffectInvalidShaderResourceVariable
?AUSEffectInvalidShaderVariable
?AUSEffectInvalidStringVariable
?AUSEffectInvalidTechnique
?AUSEffectInvalidType
?AUSEffectInvalidUnorderedAccessViewVariable
?AUSEffectInvalidVectorVariable
?AUSerializableObject
?AUSerializationContainer
?AUSerializationContainerValidator
?AUSerializationModifier
?AUSFloatScalarAnnotation
?AUSFloatScalarAnnotationMember
?AUSFloatScalarGlobalVariable
?AUSFloatScalarGlobalVariableMember
?AUSFloatVector4GlobalVariable
?AUSFloatVector4GlobalVariableMember
?AUSFloatVectorAnnotation
?AUSFloatVectorAnnotationMember
?AUSFloatVectorGlobalVariable
?AUSFloatVectorGlobalVariableMember
?AUSGroup
?AUShaderSetNode
?AUShape
?AUShatterGrid
?AUSingleFieldString
?AUSInterfaceGlobalVariable
?AUSInterfaceGlobalVariableMember
?AUSIntScalarAnnotation
?AUSIntScalarAnnotationMember
?AUSIntScalarGlobalVariable
?AUSIntScalarGlobalVariableMember
?AUSIntVectorAnnotation
?AUSIntVectorAnnotationMember
?AUSIntVectorGlobalVariable
?AUSIntVectorGlobalVariableMember
?AUSMatrix4x4ColumnMajorGlobalVariable
?AUSMatrix4x4ColumnMajorGlobalVariableMember
?AUSMatrix4x4RowMajorGlobalVariable
?AUSMatrix4x4RowMajorGlobalVariableMember
?AUSMatrixAnnotation
?AUSMatrixAnnotationMember
?AUSMatrixGlobalVariable
?AUSMatrixGlobalVariableMember
?AUSnapshotTimeComparer
?AUSNumericAnnotation
?AUSNumericAnnotationMember
?AUSNumericGlobalVariable
?AUSNumericGlobalVariableMember
?AUSound
?AUSPassBlock
?AUSRasterizerGlobalVariable
?AUSRasterizerGlobalVariableMember
?AUSRenderTargetViewGlobalVariable
?AUSRenderTargetViewGlobalVariableMember
?AUSSamplerGlobalVariable
?AUSSamplerGlobalVariableMember
?AUSShaderGlobalVariable
?AUSShaderGlobalVariableMember
?AUSShaderResourceGlobalVariable
?AUSShaderResourceGlobalVariableMember
?AUSSingleElementType
?AUSStringAnnotation
?AUSStringAnnotationMember
?AUSStringGlobalVariable
?AUSStringGlobalVariableMember
?AUSTechnique
?AUSType
?AUSUnorderedAccessViewGlobalVariable
?AUSUnorderedAccessViewGlobalVariableMember
?AUSVariable
?AUSwitch
?AUtagMFASYNCRESULT
?AUText
?AUTextShader
?AUTextureCoordinate
?AUTextureData
?AUTextureFromResourceData
?AUTexturePropertiesNode
?AUTextureProvider
?AUTextureTransform
?AUTextureTransformContainer
?AUTimeSensor
?AUTimeTrigger
?AUTransform
?AUVideoBuffer
?AUViewpoint
?AUViewport
?AUWipeShader
?AUX3DAnimatedShaderNode
?AUX3DAppearanceChildNode
?AUX3DAppearanceNode
?AUX3DBindableNode
?AUX3DBoundedObject
?AUX3DChildNode
?AUX3DColorNode
?AUX3DComposedGeometryNode
?AUX3DCoordinateNode
?AUX3DField
?AUX3DFontStyleNode
?AUX3DGeometricPropertyNode
?AUX3DGeometryNode
?AUX3DGroupingNode
?AUX3DInterpolatorNodeBase
?AUX3DLayerNode
?AUX3DMaterialNode
?AUX3DMetadataObject
?AUX3DNode
?AUX3DNormalNode
?AUX3DSensorNode
?AUX3DShaderNode
?AUX3DShapeNode
?AUX3DSoundNode
?AUX3DSoundSourceNode
?AUX3DTextNode
?AUX3DTexture2DNode
?AUX3DTextureCoordinateNode
?AUX3DTextureNode
?AUX3DTextureTransformNode
?AUX3DTimeDependentNode
?AUX3DTriggerNode
?AUX3DUrlObject
?AUX3DVertexAttributeNode
?AUX3DViewpointNode
?AUX3DViewportNode
```

### 5.5 Key vtable / Inheritance Hierarchy (inferred from RTTI naming)

```
IUnknown (base)
├── IDispatch
│   └── IDuiHandlerDispatch
├── IAVSource
│   ├── AVSource
│   ├── AVSourceProxy
│   └── AVSourceFactory
├── IAVSink
│   └── AudioStreamSink
├── IAVProcessor
│   └── VideoBuffer
├── IAVSampleSource
│   └── SyncVideoSampleSource
├── IAVStreamSink
├── IFrameBuffer
│   └── FrameBufferImplDX
├── IEngine
│   ├── Engine
│   └── EngineDX
├── ISceneEncode
│   └── SceneEncode
├── IScenePreview
│   └── ScenePreview
├── IPreviewPresenter
│   └── PreviewPresenterWrapper
├── IResourceCache
│   ├── ResourceCache
│   └── ResourceCacheDX
├── IEncodeProfile
│   └── EncodeProfile
├── ISelectionRoot
│   ├── SelectionRootImpl
│   ├── SelectionRootImplWrapper
│   ├── SelectionRootProxy
│   └── SelectionRootTarget
├── ITimelineItem
├── ITimelineItemHandler
│   └── TimelineItemHandler
├── ITextureProviderDX
├── IImageThumbnail / IMovieThumbnail
├── IWaveform / IWaveformCallback
├── IPublishHandler (IAmpPublishHandler)
├── IAudioBoost
├── IAudioSessionEvents
├── IMovieThumbnail
├── IMFAsyncCallback
├── IMFByteStream
├── IMFSourceReaderCallback
├── IDuiBehavior
├── IDuiBehaviorFactory
├── IDuiDataSource
│   └── IDuiDataSourceImpl
├── IDuiHandlerNotify
├── IDuiTemplateSource
├── IDuiVirtualLayoutMode
├── IDWriteTextRenderer / IDWritePixelSnapping
├── IDataObject
├── IDropTarget
├── IConnectionPointContainer
├── IEnumFORMATETC
├── ISequentialStream / IStream
├── IServiceProvider
├── IReferenceCounted
├── IValidateBinding
├── IUIApplication
├── IUICommandHandler
├── IUserEncodeProfileBehavior
├── IUxStoryboardNotifications
├── IWorkerObjectCallback / IWorkerThreadClient

RefCountBase
├── RefCountBaseMultiThreaded
│   ├── ExtentRefCountMultiThreaded
│   ├── AVResource / AVResourceDX
│   ├── EffectResource / EffectResourceDX
│   └── MeshResource / MeshResourceDX

Exception (Base)
├── ExceptionWithString
├── GdiException
├── JetException
├── EncodeInitializationException
├── ScriptInitializationException
├── TemplateInitializationException
└── TextureLoadException

CDUIDialog
├── CSundanceDialog
│   ├── SundanceApplicationOptionsDialog
│   ├── MultipleEffectDialog
│   ├── RenderSummaryDialog
│   └── PublishSummaryDialog
└── DontShowPromptDialog
    └── SundanceDontShowPromptDialog

DirectUI::Element
├── DirectUI::HWNDElement
├── DirectUI::NativeHWNDHost
├── DirectUI::HWNDElement
├── DirectUI::Button
├── DirectUI::Checkbox
├── DirectUI::Edit
├── DirectUI::Combobox
├── DirectUI::Selector
├── DirectUI::Label
├── DirectUI::Thumb
├── DirectUI::Slider
├── DirectUI::ScrollBar
├── DirectUI::Hyperlink
├── DirectUI::TabControl / TabButton / TabPage
├── DirectUI::PopupMenu2
├── DirectUI::SuperPopup
├── DirectUI::PopupWindow
├── DirectUI::WLEditT
├── DirectUI::VirtualLayout
└── DirectUI::VirtualListView

CFramelessHost
```

### 5.6 RTTI Type Categories (functional grouping)

#### Application Framework (12)
```
SundanceAppMain, SundanceAppDataContext, SundanceBehaviorFactory,
SundanceMainElementBehavior, SundanceNativeHwndHost,
SundanceClipboardChainWindow, SundanceDontShowPromptDialog,
SundanceApplicationOptionsDialog, CommandLineParser,
AutoSaveManager, MediaBrowser, TemplateTable
```

#### Timeline (14)
```
TimelineBaseBehavior, TimelineBehavior, TimelineDataSource,
TimelineDragDrop, TimelineExtentUIObject, TimelineInstructionsBehavior,
TimelineItemBehavior, TimelineItemHandler, TimelineItemInputBehavior,
TimelineLayoutMode, TimelineSecondaryTrackItemBehavior,
TimelineSelectionRootBehavior, TimelineTemplateSource,
TimelineVisualTrackItemBehavior
```

#### 3D Engine — Core (16)
```
Engine, EngineDX, Scene, SceneEncode, SceneMergeBackgroundRequest,
SceneMergeContext, ScenePreview, RenderLoop, RenderLoopDX,
PreviewDX, DefaultPreviewDX, EncodeDX, Conductor,
MovieTransport, TransportBase, RenderTransport
```

#### 3D Engine — Audio/Video Processing (12)
```
Mixer, MixerBuffer, AudioQueue, MixStream, AudioBoost,
AudioOutput, AudioOutputCommandParam, AudioResamplerHelper,
AudioRMSData, AudioDuckingProperties, Waveform, WaveformCallback
```

#### 3D Engine — Effects & Shaders (25)
```
EffectResource, EffectResourceDX, AnimatedEffectResourceBase,
SimpleEffect, SimpleEffectResourceBase, CommonEffectResourceDX,
DefaultEffectResourceDX, ComposedGeometryResourceDX, EffectListContainer,
EffectTemplateHandler, MovieEffect, ThemeEffect, MultipleEffectBehavior,
BlurEffectResource/ShaderImpl, BrightnessEffectResource/ShaderImpl,
ChannelMixerEffectResource/ShaderImpl, DissolveEffectResource/ShaderImpl,
EdgeDetectionEffectResource, FadeEffectResource/ShaderImpl,
GridEffectResource/ShaderImpl, HueEffectResource,
PixelateEffectResource/ShaderImpl, PosterizeEffectResource/ShaderImpl,
RippleEffectResource, WipeEffectResource/ShaderImpl,
ScrollingTextEffectResourceDX/ScrollingTextShaderImpl
```

#### 3D Engine — Pattern Meshes (28)
```
PatternMesh, BowTiePatternMesh, CheckerboardPatternMesh,
CirclePatternMesh, CirclesPatternMesh, DiagonalBoxPatternMesh,
DiagonalCrossPatternMesh, EyePatternMesh, FanAndSweepBasePatternMesh,
FanAndSweepPatternMesh, FanInPatternMesh, FanOutPatternMesh,
FanUpPatternMesh, FillVPatternMesh, HeartPatternMesh,
IrisPatternMesh, KeyholePatternMesh, RectanglePatternMesh,
RectanglesPatternMesh, RevealPatternMesh, SplitPatternMesh,
StarPatternMesh, StarsPatternMesh, SweepInPatternMesh,
SweepOutPatternMesh, SweepUpPatternMesh, WheelPatternMesh,
ZigzagPatternMesh
```

#### 3D Engine — Textures (15)
```
TextureProvider, TextureResource, TextureResourceDX,
TextureResourceFromResourceDX, ImageTexture, ImageTextureImpl,
MotionTexture, MotionTextureImpl, MotionTextureResource,
MotionTextureResourceDX, MovieTexture, MovieTextureImpl,
TextureTransform, TextureTransformContainer, TextureTransformImpl,
TextureCoordinateImpl, TexturePropertiesNodeImpl, TextureLoadException,
TextureInterOp, TextureInterOpDX9, TextureInterOpDX11
```

#### 3D Engine — Grids & Shatter (8)
```
GridImpl, GridNodeImpl, GridResourceDX, GridEffectResource,
GridShaderImpl, ShatterGridImpl, ShatterGridResourceDX,
PageCurlGridImpl, PageCurlGridResourceDX
```

#### 3D Engine — Rendering (8)
```
BackBufferDX, SharedSwapChainDX, FrameBufferImplDX,
SwapChainCommandParam, SnapShot, SnapShotDX,
PreviewPresenterWrapper, RenderLoopDX
```

#### 3D Engine — Scene Nodes (46 X3D types)
```
X3DNodeImpl, X3DGroupingNodeImpl, X3DShapeNodeImpl,
X3DGeometryNodeImpl, X3DComposedGeometryNodeImpl,
X3DAppearanceNodeImpl, X3DMaterialNodeImpl,
X3DTexture2DNodeImpl, X3DTextureNodeImpl,
X3DCoordinateNodeImpl, X3DNormalNodeImpl, X3DColorNodeImpl,
X3DViewpointNodeImpl, X3DViewportNodeImpl,
X3DTimeDependentObjectImpl, X3DSensorNodeImpl, X3DTriggerNodeImpl,
X3DShaderNodeImpl, X3DSoundNodeImpl, X3DSoundSourceNodeImpl,
X3DSoundSourceObjectImpl, X3DAnimatedShaderNodeImpl,
X3DTextNodeImpl, X3DUrlObjectImpl,
X3DVertexAttributeNodeImpl, X3DLayerNodeImpl,
X3DBoundedObjectImpl, X3DBindableNodeImpl,
X3DInterpolatorNodeBase, X3DChildNodeImpl, X3DChildObjectImpl,
X3DFontStyleNodeImpl, X3DGeometricPropertyNodeImpl,
X3DMetadataObjectImpl, X3DReader
```

#### D3DX11 Effect Framework (94 S* types)
```
CEffect, SGroup, SPassBlock, STechnique, SType, SBaseBlock,
SConstantBuffer, SAnonymousShader, SVariable, SSingleElementType
Scalar types: SFloatScalar*, SBoolScalar*, SIntScalar*
Vector types: SFloatVector*, SBoolVector*, SIntVector*, SFloatVector4*
Matrix types: SMatrix*, SMatrix4x4ColumnMajor*, SMatrix4x4RowMajor*
Annotation types: SNumericAnnotation*, SStringAnnotation*, SMatrixAnnotation*
Global variables: SBlend*, SClassInstance*, SConstantBuffer,
  SDepthStencil*, SDepthStencilView*, SRasterizer*,
  SRenderTargetView*, SShader*, SShaderResource*, SSampler*,
  SInterface*, SUnorderedAccessView*
Invalid types: SEffectInvalid* (19 types — null object pattern)
```

#### D3DX11 Texture Codecs (107 CCodec_* types)
```
Base: CCodec, CCodecDXT, CCodecYUV
DXGI formats: A1R5G5B5, A2R10G10B10, A2W10V10U10, A4L4, A4R4G4B4,
  A8_UNORM, A8L8, A8P8, A8R3G3B2, A8R8G8B8,
  B8G8R8A8_UNORM (+SRGB), B8G8R8X8_UNORM (+SRGB),
  BC1-BC7_UNORM (+SRGB), BC4/BC5 SNORM/UNORM, BC6H_SF16/UF16,
  CxV8U8, D16_LOCKABLE/UNORM, D24_UNORM_S8_UINT,
  D32_FLOAT (+S8X24), D32F_LOCKABLE,
  D3DX_A16L16, D3DX_R16G16B16, G8R8_G8B8 (+UNORM),
  L16, L6V5U5, L8, P8,
  Q16W16V16U16, Q8W8V8U8, R1_UNORM,
  R10G10B10*, R11G11B10_FLOAT, R16* (8 variants),
  R16G16* (6 variants), R16G16B16A16* (5 variants),
  R24_UNORM_X8, R32* (4 variants), R32G32* (3 variants),
  R32G32B32* (3 variants), R32G32B32A32* (3 variants),
  R3G3B2, R5G6B5, R8* (4 variants), R8G8* (6 variants),
  R8G8B8, R8G8B8A8* (5 variants), R9G9B9E5,
  V16U16, V8U8, X1R5G5B5, X24/X32 typeless,
  X4R4G4B4, X8B8G8R8, X8L8V8U8, X8R8G8B8
Supporting: CDdsLoader, CImageSlice, CWinCodecLoader, CTexture,
  CAsyncTextureProcessor, CAsyncTextureInfoProcessor, CAsyncMemoryLoader
```

#### Audio/Video Sources (12)
```
AVSource, AVSourceProxy, AVSourceFactory,
AVSink, AudioStreamSink, StreamSinkHost, StreamSinkHelper,
AVResource, AVResourceClock, AVResourceDX, AVResObj, AVData,
AVCaptureCore, AVCaptureSession, AVResourceDX
```

#### Publishing (9)
```
PublishManager, PublishJob, PublishBackgroundJob,
PublishBackgroundWorker, PublishProgressCallBack,
PublishItemProperties, PublishItemPropertyStore,
PublishSessionXmlManifestFactory, XmlManifestFactory
```

#### Serialization & Property Binding (34)
```
SerializationReader, SerializationWriter, SerializationContext,
SerializationMemoryReaderWriter, SerializationModifier,
SerializationContainer, SerializationContainerValidator,
CommandBin, RenderCommandBin, SerializableObject, ValueVariantRef,
ModBeginDocument, ModBeginElement, ModEndElement, ModContainer,
ModContainerWithAttribute, ModContainerWithIDLookAhead,
BoundProperty (+ Bool/Float/Int/String variants),
BoundPropertyFloatElement, BoundPropertyStringElement,
BoundPropertyFloatSet, BoundPropertyStringSet,
BoundPropertyDictionary, BoundPropertiesDictionaryContainer,
BoundPlaceholder, BoundPlaceholderMapContainer,
BoundPlaceholderSerializer
```

## 6. String Constants

### 6.1 Application Identity
- `"Windows Live Movie Maker"` / `"Movie Maker"` — Product names
- `"MovieMakerCore"` / `"MovieMakerCore.DLL"` — Module name
- `"MovieMakerMainWindowClass"` — Main window class
- `"MovieMaker_PopUpSlider"` — Popup slider window class
- `"16.4.3528.0331"` — Version string
- `"Microsoft"` / `"Microsoft.Windows.Live"` — Company strings
- `"(c) 2012 Microsoft Corporation"` — Copyright

### 6.2 Registry Keys
- `Software\Microsoft\Windows Live\Movie Maker` — Main settings
- `Software\Microsoft\Windows Live\Movie Maker\Post` — Post-publish
- `Software\Microsoft\Windows Live\Movie Maker\Recent` — Recent files
- `Software\Microsoft\Windows Live\Movie Maker\RecentWLVS` — Recent WLVS
- `Software\Microsoft\Windows Live\Movie Maker\Suppressed` — Suppressed prompts
- `Software\Microsoft\Windows Live\Photo Gallery` — Shared WL settings
- `Software\Microsoft\Windows Live\Photo Gallery\FileSupport\DownloadableCodecs`
- `Software\Microsoft\Windows Live\Photo Gallery\FileSupport\DownloadableWLRawCodecs`
- `Software\Microsoft\Windows Live\Photo Gallery\FileSupport\QuickTimeMovieExtensions`
- `Software\Microsoft\Windows Live\Photo Gallery\FileSupport\RawPhotoExtensions`
- `Software\Microsoft\Windows Live\Photo Gallery\SuppressFileTypes`
- `Software\Microsoft\Windows Live\Common\Movie Library`
- `Software\Microsoft\Windows Live\Common`
- `Software\Microsoft\Windows Live\Environment\PhotoGallery`
- `Software\Microsoft\Windows Live\Installer`
- `Software\Microsoft\Windows Live\Installer\ProductStatus`
- `Software\Microsoft\Windows Live\Video Profiles`

### 6.3 File Paths & Locations
- `%LOCALAPPDATA%\Microsoft\Windows Live Movie Maker` — App data directory
- `Windows Live Movie Maker Proxy Index DB` — ESENT database name
- `Windows Live Movie Maker isn't installed properly. Please try reinstalling it. (0x%x)` — Error message

### 6.4 Project Serialization Strings
- `//MovieMaker/Project/DataStr` — Project data root XPath
- `MovieMaker_SerializedProject_16.4.3528.0331` — Serialization version
- `MovieMaker_SelectedSet_16.4.3528.0331` — Selection data version
- `MovieMakerFwdCmdMapping` — Command mapping

### 6.5 Media/Transcoding Strings
- `TranscodeConfig.xml` — Transcode configuration
- Video codecs: `MP43`, `MP4S`, `MP4V`, `WMV1`, `WMV2`, `WMV3`, `WMASPDIF`, `WMAudioV8`, `WMAudioV9`
- Audio: `mp3`, `mp4`, `wma`, `wmv`, `mpeg`
- Properties: `BITRATE`, `FrameRate`, `EncodeProfile`, `FrameRateEdit`, `FrameRateLabel`, `FrameHeightEdit`, `FrameHeightLabel`, `FrameWidthEdit`, `FrameWidthLabel`, `VideoBitrate`, `VideoBitrateEdit`, `VideoBitrateLabel`

### 6.6 Storyboard/Theme/Effect Strings
- Theme system: `ThemeProject`, `ThemeScript`, `ThemeOperationLog`, `ThemeOperations`, `ThemePseudoRandomSeed`
- Titles: `TitleClip`, `TitleClipTemplate`, `TitleCredit`, `TitleDescription`, `TitleTextDefault`, `TitleIndexedFaceSet`, `TitleShape`
- Credits: `CreditClipTemplate`, `CreditTextDefault`, `Credits: Scroll, Up Stacked`
- Effects: `EffectAutomaticTemplate`, `EffectDialog`, `EffectKind`, `EffectMaterial`, `EffectTemplate`, `EffectText`, `EffectTimer_%u`
- Transitions: `TransitionCompatibility`, `TransitionDuration`, `TransitionTemplate`, `TransitionTimer_%u`
- Text: `TextEffect`, `TextEffectMaterial`, `TextEffectTemplate`, `TextEffectText`, `TextScriptId`, `TextTrackItem`
- Cinematic: `CinematicThemeScript`, `CinematicCaption1TextScript`, `CinematicCaption2LeftTextScript`, `CinematicCaption2RightTextScript`, `CinematicCreditsLeftTextScript`, `CinematicCreditsRightTextScript`, `CinematicTitleTextScript`
- Audio: `AudioFadeEffectTemplate`, `AudioFadeInDuration`, `AudioFadeOutDuration`, `AudioFileUrl`, `AudioFormatsCombo`, `AudioTrackItem`
- Pan/Zoom: `PanAndZoomEffectAutomaticTemplate`, `PanAndZoomPseudoRandomSeed`, `PanAndZoomShapeEffect`

### 6.7 UI Property IDs
- `PropertyID_AppPaneEnabled`, `PropertyID_AppItemCountAndPositionString`
- `PropertyID_TimelineIsEmpty`, `PropertyID_TimelineIsNotEmpty`
- `PropertyID_TimelineTemplateSize`, `PropertyID_TimelineRowHeight`, `PropertyID_TimelineCaretPadding`
- `PropertyID_AppTimePerCell`, `PropertyID_AppTimePerCellMin`, `PropertyID_AppTimePerCellMax`, `PropertyID_AppTimePerCellStep`, `PropertyID_AppTimePerCellUnScaled`
- `PropertyID_ProgressControl_MinValue/MaxValue/Position/ContentString/DetailsString`
- `PropertyID_ProgressCancel_Visible`, `PropertyID_ProgressStatus_Visible/LayoutPos`
- `PropertyID_AudioDataStatus_Visible`

### 6.8 Resource IDs
- `IDR_BRIGHTNESSCONTROL_BRIGHTERICON_96/120/144`
- `IDR_BRIGHTNESSCONTROL_DARKERICON_96/120/144`
- `IDR_MIXCONTROL_MOVIEICON_96/120/144`
- `IDR_MIXCONTROL_NARRATIONICON_96/120/144`
- `IDR_MIXCONTROL_SOUNDTRACKICON_96/120/144`
- `IDR_MIXCONTROL_VOLUMEMINUSICON_96/120/144`
- `IDR_MIXCONTROL_VOLUMEPLUSICON_96/120/144`
- `IDR_TRANSPARENCY_MINUSICON_96/120/144`
- `IDR_TRANSPARENCY_PLUSICON_96/120/144`

### 6.9 Window/Dialog Resource Names
- `ideHomerAbove`, `idrAboveHomerWindow`
- `ideHomerHeavyPreviewZone`, `ideHomerHeavyPreview`
- `idrSundanceMainWindow`, `idrAutoMoviePlayerWindow`
- `ideAutoMoviePlayerWindow`, `idePreviewPane`
- `ideTimelineInstructionsText`, `idrOptionsDialog`
- `ProgressDialogBox`, `ideInlinePreviewSliderParent`
- `ideThumbZoomButton`, `ideWebCamPreview`
- `ideProductName`, `ideProductVersion`, `ideProductCopyright`
- `ideFeedbackLink`, `ideServiceAgreementLink`, `idePrivacyStatementLink`, `ideLearnMoreLink`
- `ideVideoDeviceList`, `ideAudioDeviceList`
- `ideVideoDeviceSelected`, `ideAudioDeviceSelected`
- `ideAudioSettings`, `ideProxyDefaults`
- `idePreviewTextCheckbox`, `ideEncodeTextCheckbox`, `ideok`, `ideSQMCheckBoxButton`
- `ideAutomaticTextCheckbox`, `ideTranscodeOptionsNote`, `ideProxyNote`
- `ideTab`, `ideAudioAndVideoOptionsTab`, `ideAudioVideoSettingsTabButton`

### 6.10 Telemetry Endpoints
- `http://g.live.com` — Production telemetry
- `http://g.live-int.com` — Internal/test telemetry
- `https://account.live.com/` — Authentication
- `https://profile.live.com/` — Profile services

### 6.11 External DLL References
```
Comctl32.dll, d3d10level9.dll, d3d11.dll, eWLXPhotoLibraryDatabase.dll,
Kernel32.dll, MF.dll, MovieMakerCore.DLL, MFplat.dll,
Advapi32.dll, WindowsCodecs.dll, WLAVRes.dll, WLXPhotoBase.dll
```

### 6.12 Supported File Formats
`.mp4, .wmv, .m4a, .wma, .wlmp, .wlvs, .mpg, .asf, .avi, .mpv, .m1v, .m2v, .mpeg, .qt, .mov, .wm, .mpe, .3g2, .3gpp, .3gp, .mqv, .rle, .gif, .ico, .3gp2, .mpv2, .mp2v, .mp2, .dib, .mod, .vob, .xmp`

### 6.13 X3D/Scene Graph Strings
```
HMREngine-3 — Scene graph version identifier
Scene, head, content, name, version, profile, X3D, component,
USE, ROUTE, meta, toField, toIndex, fromNode, toNode,
fromField, containerField
```

### 6.14 DX11 Shader Variable Prefixes
- `s_*` — Shader resource variables (s_changed, s_ContentString, s_DetailsString, s_LayoutPos, s_MaxDetailsString, s_MaxValue, s_MinValue, s_Position, s_PositionAccValue, s_Protected, s_trackbar, s_Visible)
- `t_*` — Texture/node variables (t_activeLayer, t_bboxCenter, t_bboxSize, t_bind, t_center, t_CenterOfRotation, t_changed, t_children, t_clipBoundary, t_Field, t_layers, t_order, t_rotation, t_scale, t_scaleOrientation, t_translation, t_value)

## 7. COM GUIDs

| GUID | Purpose |
|------|---------|
| `{30B71B3D-CC48-4650-8D0A-1A106B282AF5}` | Telemetry/error reporting endpoint |
| `{9590CDCE-BFFC-45d5-85EA-493C65FB2C4F}` | Transition/Effect type GUID |
| `{CE8B9537-708C-4784-9DD4-127B635DD348}` | COM APPID for MovieMaker |

## 8. Architecture Summary

### Dependency Graph (34 imported DLLs)

```
MovieMakerCore.dll
├── Core Windows (5 DLLs, 356 fns)
│   ├── KERNEL32.dll (117 fns) — Process, thread, memory, file I/O, sync
│   ├── USER32.dll (84 fns) — Window management, message loop, UI
│   ├── GDI32.dll (17 fns) — Device contexts, regions, bitmaps
│   ├── ADVAPI32.dll (21 fns) — Registry, crypto, WPP tracing
│   └── SHELL32.dll (17 fns) — Shell items, known folders, execution
├── COM/OLE (3 DLLs, 47 fns)
│   ├── ole32.dll (23 fns) — COM initialization, clipboard, storage
│   ├── OLEAUT32.dll (22 fns) — BSTR, VARIANT, SafeArray, type libs
│   └── OLEACC.dll (2 fns) — Accessibility
├── Media Foundation (2 DLLs, 21 fns)
│   ├── MF.dll (2 fns) — MF services, transcode output types
│   └── MFPlat.DLL (19 fns) — MF platform, media types, samples
├── DirectX/Graphics (9 DLLs, 36 fns)
│   ├── d3d11.dll (1 fn) — D3D11 device creation
│   ├── d3d9.dll (2 fns) — D3D9 fallback rendering
│   ├── d2d1.dll (1 fn) — Direct2D factory
│   ├── DWrite.dll (1 fn) — DirectWrite text
│   ├── dxva2.dll (2 fns) — Hardware video decode
│   ├── D3DCOMPILER_46.dll (2 fns) — HLSL compilation
│   ├── gdiplus.dll (24 fns) — GDI+ 2D rendering
│   ├── dwmapi.dll (1 fn) — Desktop Window Manager
│   └── UxTheme.dll (3 fns) — Visual styles
├── Windows Live (7 DLLs, 235 fns)
│   ├── UXCore.dll (192 fns) — DirectUI framework
│   ├── WLXPhotoBase.dll (14 fns) — Base utilities
│   ├── WLXPhotoSqm.dll (13 fns) — SQM telemetry
│   ├── DmxBici.dll (5 fns) — BICI telemetry
│   ├── MetadataSys.dll (1 fn) — Photo metadata
│   ├── wlidcli.dll (7 fns) — Windows Live ID
│   └── uxctl.dll (3 fns) — UX controls
├── Storage (1 DLL)
│   └── ESENT.dll (31 fns) — Extensible Storage Engine (Jet DB)
├── Utilities (4 DLLs, 38 fns)
│   ├── SHLWAPI.dll (26 fns) — Path/string utilities
│   ├── PROPSYS.dll (8 fns) — Property system
│   ├── XmlLite.dll (4 fns) — XML reader/writer
│   └── VERSION.dll (3 fns) — File version info
└── Other (2 DLLs, 2 fns)
    ├── WINMM.dll (1 fn) — PlaySound
    └── WindowsCodecs.dll (1 fn) — WIC bitmap conversion
```

### Key Architectural Patterns

1. **Single-export monolith** — 5.46 MB of code, one export. The DLL is functionally an EXE packaged as a DLL.

2. **Three-layer rendering** — GDI+ (2D UI) + D3D9 (legacy) + D3D11 (primary preview/encode).

3. **Embedded D3DX11 framework** — Complete D3DX11 effect system (94 types) compiled into the DLL, not loaded from external .fx files.

4. **VRML/X3D engine** — Full X3D 9777 node hierarchy (46 node types) for 3D transitions and effects.

5. **Pattern mesh system** — 28 transition patterns (heart, star, iris, page curl, etc.) for video transitions.

6. **ESENT database** — 31 Jet API imports for internal structured storage (thumbnail cache, project index, MRU).

7. **Dual telemetry** — SQM (Microsoft Quality Metrics) + BICI (Behavioral Instrumentation) + custom HTTP error reporting.

8. **Storyboard namespace** — 141+ types managing project serialization, theme application, template instantiation, and media item lifecycle.

9. **Extensive COM integration** — ATL module, OLE clipboard, COM class registration via resource section (REGISTRY resources in .rsrc).

10. **Multi-threaded architecture** — IoCompletionPort, condition variables, thread pool, interlocked operations indicate heavy async processing.

11. **107 texture codec classes** — Complete DXGI format coverage via CCodec_* pattern classes for pixel format conversion.

12. **192 UXCore imports** — Heavy reliance on the DirectUI framework for all UI rendering and layout.

### Initialization Order

```
DllMain (DLL_PROCESS_ATTACH):
  1. Store hInstance
  2. DisableThreadLibraryCalls

MovieMakerMain:
  3. CrtInit (SEH frame, stack guard)
  4. ATL module state check
  5. Command-line parsing
  6. CoInitializeEx (STA)
  7. ATL _Module.Init / RegisterClassObjects
  8. GdiplusStartup
  9. MFStartup
  10. UXCoreInitProcess / UXCoreInitThread
  11. DirectUI initialization
  12. D3D11CreateDevice
  13. D2D1CreateFactory / DWriteCreateFactory / WIC factory
  14. Main message loop

ShutdownSubsystems (DLL_PROCESS_DETACH):
  Release WIC → DWrite → D2D → D3D11 → DirectUI → UXCore
  MFShutdown → GdiplusShutdown → _Module.Term → CoUninitialize
```

## 9. RTTI Count Summary

| Metric | Count |
|--------|-------|
| Classes (?AV) | 683 |
| Structs (?AU) | 335 |
| **Total RTTI types** | **1,018** |
| CCodec DXGI format variants | 107 |
| D3DX11 Effect types (S* + SEffectInvalid*) | ~94 |
| D3DX11 Effect variable types | ~40 |
| X3D node implementations | ~35 |
| Pattern mesh types | 28 |
| Imported DLLs | 34 |
| Imported functions | 763 |
| Exported functions | 1 |
| PE sections | 4 |
