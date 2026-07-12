/*
 * pch.h
 *
 * Precompiled header for MovieMakerCore.dll.
 * Includes the full set of Windows SDK, ATL, WTL, GDI+, Direct3D,
 * Media Foundation, and project headers used throughout the DLL.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once
#ifndef MOVIECORE_PCH_H
#define MOVIECORE_PCH_H

// ============================================================================
// Target version
// ============================================================================
#define WINVER          0x0602
#define _WIN32_WINNT    0x0602
#define _WIN32_IE       0x0800
#define NTDDI_VERSION   0x06020000

// ============================================================================
// Compile definitions
// ============================================================================
#define STRICT
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define _CRT_SECURE_NO_WARNINGS
#define _WINDLL
#define _AFXDLL
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
#define _ATL_ALL_WARNINGS

// ============================================================================
// Windows SDK
// ============================================================================
#include <windows.h>
#include <windowsx.h>
#include <objbase.h>
#include <ole2.h>
#include <olectl.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <shellapi.h>
#include <shobjidl.h>
#include <commctrl.h>
#include <commdlg.h>
#include <prsht.h>
#include <unknwn.h>
#include <exdisp.h>
#include <mshtmhst.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <strsafe.h>
#include <process.h>
#include <tchar.h>
#include <appmodel.h>
#include <versionhelpers.h>
#include <synchapi.h>

// ============================================================================
// COM / OLE
// ============================================================================
#include <comdef.h>
#include <comsvcs.h>

// ============================================================================
// ATL (Active Template Library)
// ============================================================================
#include <atlbase.h>
#include <atlstr.h>
#include <atlcoll.h>
#include <atlcom.h>
#include <atlctl.h>
#include <atlconv.h>
#include <atlsync.h>

// ============================================================================
// WTL (Windows Template Library)
// ============================================================================
#include <atlapp.h>
#include <atlcrack.h>
#include <atlctrls.h>
#include <atlddx.h>
#include <atldlgs.h>
#include <atlframe.h>
#include <atlhost.h>
#include <atlmisc.h>
#include <atlprint.h>
#include <atlscrl.h>
#include <atlSplit.h>
#include <atlwin.h>

// ============================================================================
// GDI+
// ============================================================================
#include <gdiplus.h>

// ============================================================================
// Direct2D / DirectWrite
// ============================================================================
#include <d2d1.h>
#include <d2d1helper.h>
#include <d2d1effects.h>
#include <dwrite.h>

// ============================================================================
// Direct3D 11 / Direct3D 9 / D3D Compiler
// ============================================================================
#include <d3d11.h>
#include <d3d11_1.h>
#include <d3d11_2.h>
#include <d3d9.h>
#include <d3dcompiler.h>
#include <dxva2api.h>

// ============================================================================
// Media Foundation
// ============================================================================
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include <mfmediaengine.h>
#include <mfmp2t.h>
#include <mfobjects.h>
#include <mfplay.h>
#include <mfsession.h>
#include <evr.h>
#include <evr9.h>

// ============================================================================
// Desktop Window Manager / Visual Styles
// ============================================================================
#include <dwmapi.h>
#include <uxtheme.h>
#include <vssym32.h>
#include <vsstyle.h>

// ============================================================================
// Windows Imaging Component (WIC)
// ============================================================================
#include <wincodec.h>
#include <wincodecsdk.h>
#include <imagingfactory.h>

// ============================================================================
// XML
// ============================================================================
#include <xmllite.h>

// ============================================================================
// Accessibility / Property System
// ============================================================================
#include <oleacc.h>
#include <propkey.h>
#include <propvarutil.h>
#include <propsys.h>

// ============================================================================
// ESE (Extensible Storage Engine)
// ============================================================================
#include <esent.h>

// ============================================================================
// Multimedia
// ============================================================================
#include <winmm.h>

// ============================================================================
// C++ Standard Library
// ============================================================================
#include <new>
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <list>
#include <deque>
#include <queue>
#include <array>
#include <algorithm>
#include <functional>
#include <utility>
#include <type_traits>
#include <cstdint>
#include <cstring>
#include <cstddef>
#include <cassert>
#include <cwchar>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <initializer_list>
#include <chrono>
#include <mutex>
#include <atomic>
#include <thread>
#include <condition_variable>

// ============================================================================
// Project headers
// ============================================================================
#include "common.h"
#include "exports.h"
#include "MovieMakerCore.h"
#include "WLXPhotoBase.h"

// ============================================================================
// PCH guard verification
// ============================================================================
#ifdef _MSC_VER
    #pragma warning(disable: 4505) // unreferenced local function has been removed
    #pragma warning(disable: 4100) // unreferenced formal parameter
    #pragma warning(disable: 4127) // conditional expression is constant
#endif

#endif // MOVIECORE_PCH_H
