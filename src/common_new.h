#pragma once
#ifndef WMMR_COMMON_H
#define WMMR_COMMON_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0602
#endif
#ifndef WINVER
#define WINVER 0x0602
#endif

#include <windows.h>
#include <objbase.h>
#include <ole2.h>
#include <olectl.h>
#include <shlobj.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <commctrl.h>
#include <commdlg.h>
#include <windowsx.h>
#include <prsht.h>
#include <unknwn.h>
#include <exdisp.h>
#include <mshtmhst.h>

#include <gdiplus.h>
#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#include <d3d11.h>
#include <d3d9.h>
#include <d3dcompiler.h>
#include <dxva2api.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include <mfmediaengine.h>
#include <evr.h>
#include <dwmapi.h>
#include <uxtheme.h>
#include <vssym32.h>
#include <vsstyle.h>
#include <wincodec.h>
#include <wincodecsdk.h>
#include <xmllite.h>
#include <oleacc.h>
#include <propkey.h>
#include <propvarutil.h>
#include <propsys.h>
#include <shobjidl.h>
#include <knownfolders.h>
#include <appmodel.h>
#include <versionhelpers.h>

#include <winerror.h>
#include <esent.h>
#include <mmsystem.h>

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
#include <comdef.h>
#include <atlbase.h>

// Forward declare IUnknown for ComPtr usage
struct IUnknown;

#define WMMR_VERSION_MAJOR 16
#define WMMR_VERSION_MINOR 4
#define WMMR_VERSION_BUILD 3528
#define WMMR_VERSION_REVISION 331

#define WMMR_VERSION_STRING "16.4.3528.331"
#define WMMR_VERSION_PRODUCT "Windows Live Movie Maker 2012"

#define MAKE_VERSION_ULL(major, minor, build, rev) \
    ((((unsigned long long)(major)) << 48) | \
     (((unsigned long long)(minor)) << 32) | \
     (((unsigned long long)(build)) << 16) | \
     ((unsigned long long)(rev)))

#define WMMR_VERSION_ULL \
    MAKE_VERSION_ULL(WMMR_VERSION_MAJOR, WMMR_VERSION_MINOR, WMMR_VERSION_BUILD, WMMR_VERSION_REVISION)

#ifndef VERIFY
#ifdef _DEBUG
#define VERIFY(x) assert(x)
#else
#define VERIFY(x) ((void)(x))
#endif
#endif

#ifndef ASSERT
#define ASSERT assert
#endif

#ifndef TRACE
#ifdef _DEBUG
#define TRACE OutputDebugStringA
#else
#define TRACE(...) ((void)0)
#endif
#endif

#ifndef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(p) (void)(p)
#endif

#ifndef ARRAYSIZE
#define ARRAYSIZE(a) (sizeof(a)/sizeof((a)[0]))
#endif

namespace WMMR {

using Byte = unsigned char;
using BytePtr = Byte*;
using ConstBytePtr = const Byte*;
using WideString = std::wstring;
using String = std::string;

template<typename T>
using ComPtr = CComPtr<T>;

template<typename T>
using ComQIPtr = CComQIPtr<T>;

inline bool Succeeded(HRESULT hr) { return SUCCEEDED(hr); }
inline bool Failed(HRESULT hr) { return FAILED(hr); }

constexpr size_t kMaxPath = MAX_PATH;
constexpr size_t kMaxFileName = 256;

constexpr CLSID CLSID_MovieMakerCore = { 0x00000000, 0x0000, 0x0000, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };

struct VersionInfo {
    UINT major;
    UINT minor;
    UINT build;
    UINT revision;
};

inline VersionInfo GetVersion() {
    return { WMMR_VERSION_MAJOR, WMMR_VERSION_MINOR, WMMR_VERSION_BUILD, WMMR_VERSION_REVISION };
}

struct MovieMakerException : public std::runtime_error {
    explicit MovieMakerException(const std::string& msg) : std::runtime_error(msg) {}
    explicit MovieMakerException(HRESULT hr) : std::runtime_error("HRESULT error"), m_hr(hr) {}
    HRESULT GetHResult() const { return m_hr; }
private:
    HRESULT m_hr = E_FAIL;
};

struct CoInitializer {
    CoInitializer() { CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE); }
    ~CoInitializer() { CoUninitialize(); }
};

struct GdiplusInit {
    Gdiplus::GdiplusStartupInput m_input;
    ULONG_PTR m_token;
    GdiplusInit() { Gdiplus::GdiplusStartup(&m_token, &m_input, NULL); }
    ~GdiplusInit() { Gdiplus::GdiplusShutdown(m_token); }
};

} // namespace WMMR

#endif // WMMR_COMMON_H
