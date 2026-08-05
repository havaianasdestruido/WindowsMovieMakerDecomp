/*
 * MetadataSys.cpp
 *
 * Implementation of MetadataSys.dll -- the media metadata service used by
 * Windows Live Movie Maker 2012 / Photo Gallery.
 *
 * The DLL's single custom export, WLXPSGetItemPropertyHandler (ordinal 1),
 * hands out an IPropertyStore for a shell item so the imaging pipeline can
 * read and write photo/video metadata (EXIF/XMP/IPTC tags, MP4/ASF tags,
 * duration, dimensions, bitrate, codecs, ratings) through the standard
 * Windows Property System.
 *
 * Implementation notes:
 *   - The primary property store is obtained from the Windows Property
 *     System via PSGetItemPropertyHandler (propsys.dll), so every property
 *     the shell's registered file handlers can read/write flows through
 *     unchanged (EXIF for JPEG/TIFF, tags for MP4/ASF, video dimensions,
 *     and so on).
 *   - The store is wrapped in a thread-safe adapter (CSafePropertyStore)
 *     that serializes access and normalizes values with
 *     PSCoerceToCanonicalValue -- matching the "CSafePropertyStore" class
 *     found in the original binary's RTTI.
 *   - When the item resolves to a file path, a Media Foundation extension
 *     store (CMediaMetadataStore) derives media attributes (duration, frame
 *     size, bitrate, frame rate, codec FOURCC, audio channels / sample rate)
 *     with an IMFSourceReader, image dimensions with GDI+, and file facts
 *     (size, dates, names) directly. Extension values only fill gaps the
 *     shell handler leaves empty, so they never override authoritative data.
 *   - The four standard COM exports (DllCanUnloadNow / DllGetClassObject /
 *     DllRegisterServer / DllUnregisterServer) live in dllmain.cpp. They are
 *     ATL-style boilerplate: the original binary's CLSIDs were never
 *     extracted, so no COM class is registered by this recreation and
 *     DllGetClassObject correctly reports CLASS_E_CLASSNOTAVAILABLE.
 *
 * Known limitations (documented, honest-feasible scope):
 *   - Embedded tag readback for formats with no shell property handler is
 *     not reimplemented here. Bridging container tag metadata via
 *     IMFMetadata is left out because the MF key mapping is container
 *     specific and untested. Mainstream formats (JPEG/TIFF/MP4/WMV) are
 *     covered by the shell handlers.
 *   - Write-back goes through the shell handler via IPropertyStore::
 *     SetValue/Commit (this is what writes EXIF/XMP/tags for photos and
 *     video). Direct MF-side tag writes via IMFMetadata are not attempted:
 *     file sources are read-only, and the Media Foundation extension store
 *     built here is read-only by design.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "WLXPhotoBase.h"

#include <objbase.h>
#include <unknwn.h>

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include <mfobjects.h>

#include <propsys.h>
#define INITGUID // PKEY_* constants are only declared without this; see propkeydef.h
#include <propkey.h>
#undef INITGUID
#include <propidl.h>
#include <shlwapi.h>
#include <shobjidl.h>

#include <string>
#include <vector>
#include <new>

// ============================================================================
// Internal helpers
// ============================================================================
namespace
{

// ----------------------------------------------------------------------------
// One-shot Media Foundation startup. MF is reference-counted at the OS level;
// like the GDI+ token in WLXPhotoBase_Init, this is deliberately process
// lifetime so no MFShutdown is ever emitted.
// ----------------------------------------------------------------------------
class MFHelper
{
public:
    MFHelper()
        : m_fStarted(false)
        , m_fInitialized(false)
    {
        ::InitializeCriticalSection(&m_cs);
    }

    ~MFHelper()
    {
        ::DeleteCriticalSection(&m_cs);
    }

    bool EnsureStarted()
    {
        ::EnterCriticalSection(&m_cs);
        if (!m_fInitialized)
        {
            m_fInitialized = true;
            m_fStarted = SUCCEEDED(::MFStartup(MF_VERSION, MFSTARTUP_FULL));
        }
        const bool fStarted = m_fStarted;
        ::LeaveCriticalSection(&m_cs);
        return fStarted;
    }

private:
    CRITICAL_SECTION m_cs;
    bool             m_fStarted;
    bool             m_fInitialized;
};

MFHelper& GetMFHelper()
{
    static MFHelper s_helper;
    return s_helper;
}

bool EnsureMediaFoundationStarted()
{
    return GetMFHelper().EnsureStarted();
}

// ----------------------------------------------------------------------------
// One-shot GDI+ startup (reference-counted inside GDI+).
// ----------------------------------------------------------------------------
class GdiplusHelper
{
public:
    GdiplusHelper()
        : m_token(0)
        , m_fStarted(false)
        , m_fInitialized(false)
    {
        ::InitializeCriticalSection(&m_cs);
    }

    ~GdiplusHelper()
    {
        ::DeleteCriticalSection(&m_cs);
    }

    bool EnsureStarted()
    {
        ::EnterCriticalSection(&m_cs);
        if (!m_fInitialized)
        {
            m_fInitialized = true;
            Gdiplus::GdiplusStartupInput startupInput;
            m_fStarted =
                (Gdiplus::GdiplusStartup(&m_token, &startupInput, NULL) == Gdiplus::Ok);
        }
        const bool fStarted = m_fStarted;
        ::LeaveCriticalSection(&m_cs);
        return fStarted;
    }

private:
    CRITICAL_SECTION m_cs;
    ULONG_PTR        m_token;
    bool             m_fStarted;
    bool             m_fInitialized;
};

GdiplusHelper& GetGdiplusHelper()
{
    static GdiplusHelper s_helper;
    return s_helper;
}

bool EnsureGdiplusStarted()
{
    return GetGdiplusHelper().EnsureStarted();
}

// ----------------------------------------------------------------------------
// Renders an MF media subtype as a displayable codec name. Compressed video
// subtypes store the FOURCC in Data1 in native byte order, so the string is
// reconstructed low byte first ("H264", "WMV3", ...). Non-printable payloads
// (uncompressed formats such as RGB32, which carry a D3DFORMAT value) are
// reported as hex.
// ----------------------------------------------------------------------------
std::wstring FourCCName(REFGUID guidSubtype)
{
    const DWORD dwFourCC = guidSubtype.Data1;

    const BYTE b0 = (BYTE)(dwFourCC & 0xFF);
    const BYTE b1 = (BYTE)((dwFourCC >> 8) & 0xFF);
    const BYTE b2 = (BYTE)((dwFourCC >> 16) & 0xFF);
    const BYTE b3 = (BYTE)((dwFourCC >> 24) & 0xFF);

    if (b0 >= 0x20 && b0 <= 0x7E &&
        b1 >= 0x20 && b1 <= 0x7E &&
        b2 >= 0x20 && b2 <= 0x7E &&
        b3 >= 0x20 && b3 <= 0x7E)
    {
        std::wstring str;
        str.push_back((wchar_t)b0);
        str.push_back((wchar_t)b1);
        str.push_back((wchar_t)b2);
        str.push_back((wchar_t)b3);
        return str;
    }

    WCHAR szHex[16];
    wsprintfW(szHex, L"0x%08X", dwFourCC);
    return std::wstring(szHex);
}

// ----------------------------------------------------------------------------
// True when the extension names an image type GDI+ can decode.
// ----------------------------------------------------------------------------
bool IsImageExtension(LPCWSTR pszExt)
{
    static const LPCWSTR s_rgImageExts[] =
    {
        L".jpg", L".jpeg", L".jpe", L".png", L".bmp", L".dib",
        L".gif", L".tif", L".tiff", L".wdp", L".jxr"
    };

    if (!pszExt || !pszExt[0])
        return false;

    for (size_t i = 0; i < (sizeof(s_rgImageExts) / sizeof(s_rgImageExts[0])); ++i)
    {
        if (lstrcmpiW(pszExt, s_rgImageExts[i]) == 0)
            return true;
    }

    return false;
}

} // namespace

// ============================================================================
// CMediaMetadataStore -- read-only IPropertyStore derived from the file itself
// ============================================================================
// Produces a small, self-contained property set for a file path: file facts
// (size, dates, name/extension) from Win32, media attributes (duration, frame
// size, bitrate, frame rate, codec, audio channels / sample rate) via an
// IMFSourceReader, and image dimensions via GDI+ for stills. Used as the
// fallback layer when the shell has no property handler for a format.
// ============================================================================
class CMediaMetadataStore : public IPropertyStore
{
public:
    static HRESULT Create(LPCWSTR pszPath, CMediaMetadataStore** ppStore);

    // IUnknown
    STDMETHOD(QueryInterface)(REFIID riid, void** ppv);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

    // IPropertyStore
    STDMETHOD(GetCount)(DWORD* pcProps);
    STDMETHOD(GetAt)(DWORD iProp, PROPERTYKEY* pkey);
    STDMETHOD(GetValue)(REFPROPERTYKEY key, PROPVARIANT* pv);
    STDMETHOD(SetValue)(REFPROPERTYKEY key, REFPROPVARIANT propvar);
    STDMETHOD(Commit)();

private:
    CMediaMetadataStore(LPCWSTR pszPath);
    ~CMediaMetadataStore();

    struct PropertyEntry
    {
        PROPERTYKEY key;
        PROPVARIANT value;
    };

    void LoadFileAttributes(const WIN32_FILE_ATTRIBUTE_DATA& fad);
    void LoadMediaAttributes();
    void LoadImageAttributes();

    void AddProp(REFPROPERTYKEY key, PROPVARIANT* pv);
    void AddStringProp(REFPROPERTYKEY key, LPCWSTR wsz);
    void AddUInt32Prop(REFPROPERTYKEY key, UINT32 uValue);
    void AddUInt64Prop(REFPROPERTYKEY key, ULONGLONG ullValue);
    void AddFileTimeProp(REFPROPERTYKEY key, const FILETIME& ft);

    std::wstring            m_strPath;
    std::vector<PropertyEntry> m_props;
    LONG                    m_cRef;
};

// ============================================================================
// CMediaMetadataStore implementation
// ============================================================================

CMediaMetadataStore::CMediaMetadataStore(LPCWSTR pszPath)
    : m_strPath(pszPath ? pszPath : L"")
    , m_cRef(1)
{
}

CMediaMetadataStore::~CMediaMetadataStore()
{
    for (size_t i = 0; i < m_props.size(); ++i)
        PropVariantClear(&m_props[i].value);
}

HRESULT CMediaMetadataStore::Create(LPCWSTR pszPath, CMediaMetadataStore** ppStore)
{
    if (!ppStore)
        return E_POINTER;
    *ppStore = NULL;

    if (!pszPath || !pszPath[0])
        return E_INVALIDARG;

    WIN32_FILE_ATTRIBUTE_DATA fad;
    if (!GetFileAttributesExW(pszPath, GetFileExInfoStandard, &fad))
        return HRESULT_FROM_WIN32(GetLastError());

    CMediaMetadataStore* pStore = NULL;
    try
    {
        pStore = new (std::nothrow) CMediaMetadataStore(pszPath);
        if (!pStore)
            return E_OUTOFMEMORY;

        pStore->LoadFileAttributes(fad);
        pStore->LoadMediaAttributes();   // MF (best-effort)
        pStore->LoadImageAttributes();   // GDI+ (best-effort)

        *ppStore = pStore;
        return S_OK;
    }
    catch (const std::bad_alloc&)
    {
        delete pStore;
        return E_OUTOFMEMORY;
    }
    catch (...)
    {
        delete pStore;
        return E_FAIL;
    }
}

STDMETHODIMP CMediaMetadataStore::QueryInterface(REFIID riid, void** ppv)
{
    if (!ppv)
        return E_POINTER;
    *ppv = NULL;

    if (IsEqualIID(riid, __uuidof(IUnknown)) ||
        IsEqualIID(riid, __uuidof(IPropertyStore)))
    {
        *ppv = static_cast<IPropertyStore*>(this);
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CMediaMetadataStore::AddRef()
{
    return (ULONG)InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CMediaMetadataStore::Release()
{
    const LONG cRef = InterlockedDecrement(&m_cRef);
    if (cRef == 0)
        delete this;
    return (ULONG)cRef;
}

STDMETHODIMP CMediaMetadataStore::GetCount(DWORD* pcProps)
{
    if (!pcProps)
        return E_POINTER;
    *pcProps = (DWORD)m_props.size();
    return S_OK;
}

STDMETHODIMP CMediaMetadataStore::GetAt(DWORD iProp, PROPERTYKEY* pkey)
{
    if (!pkey)
        return E_POINTER;
    if (iProp >= (DWORD)m_props.size())
        return E_INVALIDARG;
    *pkey = m_props[iProp].key;
    return S_OK;
}

STDMETHODIMP CMediaMetadataStore::GetValue(REFPROPERTYKEY key, PROPVARIANT* pv)
{
    if (!pv)
        return E_POINTER;
    PropVariantInit(pv);

    for (size_t i = 0; i < m_props.size(); ++i)
    {
        if (IsEqualPropertyKey(m_props[i].key, key))
            return PropVariantCopy(pv, &m_props[i].value);
    }

    // Not present: the contract says S_OK with an empty PROPVARIANT.
    return S_OK;
}

STDMETHODIMP CMediaMetadataStore::SetValue(REFPROPERTYKEY /*key*/, REFPROPVARIANT /*propvar*/)
{
    // This store is derived from file/media attributes and is read-only.
    return STG_E_ACCESSDENIED;
}

STDMETHODIMP CMediaMetadataStore::Commit()
{
    // Nothing to persist; the store keeps no cached modifications.
    return S_OK;
}

// ----------------------------------------------------------------------------
// Property accumulation helpers
// ----------------------------------------------------------------------------

void CMediaMetadataStore::AddProp(REFPROPERTYKEY key, PROPVARIANT* pv)
{
    if (!pv || pv->vt == VT_EMPTY)
    {
        if (pv)
            PropVariantClear(pv);
        return;
    }

    PropertyEntry entry;
    entry.key = key;
    PropVariantInit(&entry.value);

    const HRESULT hr = PropVariantCopy(&entry.value, pv);
    PropVariantClear(pv); // source is consumed by the copy

    if (SUCCEEDED(hr))
    {
        m_props.push_back(entry);
    }
    else
    {
        PropVariantClear(&entry.value);
    }
}

void CMediaMetadataStore::AddStringProp(REFPROPERTYKEY key, LPCWSTR wsz)
{
    if (!wsz)
        return;

    const size_t cch = wcslen(wsz) + 1;
    LPWSTR pszCopy = (LPWSTR)CoTaskMemAlloc(cch * sizeof(WCHAR));
    if (!pszCopy)
        return;
    wcscpy_s(pszCopy, cch, wsz);

    PROPVARIANT pv;
    PropVariantInit(&pv);
    pv.vt = VT_LPWSTR;
    pv.pwszVal = pszCopy;
    AddProp(key, &pv); // copies then clears pv (releases pszCopy)
}

void CMediaMetadataStore::AddUInt32Prop(REFPROPERTYKEY key, UINT32 uValue)
{
    PROPVARIANT pv;
    PropVariantInit(&pv);
    pv.vt = VT_UI4;
    pv.ulVal = uValue;
    AddProp(key, &pv);
}

void CMediaMetadataStore::AddUInt64Prop(REFPROPERTYKEY key, ULONGLONG ullValue)
{
    PROPVARIANT pv;
    PropVariantInit(&pv);
    pv.vt = VT_UI8;
    pv.uhVal.QuadPart = ullValue;
    AddProp(key, &pv);
}

void CMediaMetadataStore::AddFileTimeProp(REFPROPERTYKEY key, const FILETIME& ft)
{
    PROPVARIANT pv;
    PropVariantInit(&pv);
    pv.vt = VT_FILETIME;
    pv.filetime = ft;
    AddProp(key, &pv);
}

// ----------------------------------------------------------------------------
// File facts (size, dates, names) -- always available for an existing file.
// ----------------------------------------------------------------------------

void CMediaMetadataStore::LoadFileAttributes(const WIN32_FILE_ATTRIBUTE_DATA& fad)
{
    LARGE_INTEGER liSize;
    liSize.HighPart = (LONG)fad.nFileSizeHigh;
    liSize.LowPart = fad.nFileSizeLow;
    AddUInt64Prop(PKEY_Size, (ULONGLONG)liSize.QuadPart);

    AddFileTimeProp(PKEY_DateModified, fad.ftLastWriteTime);
    AddFileTimeProp(PKEY_DateCreated, fad.ftCreationTime);

    LPCWSTR pszName = PathFindFileNameW(m_strPath.c_str());
    if (pszName && pszName[0])
    {
        AddStringProp(PKEY_FileName, pszName);
        AddStringProp(PKEY_ItemNameDisplay, pszName);
    }

    LPCWSTR pszExt = PathFindExtensionW(m_strPath.c_str());
    if (pszExt && pszExt[0])
        AddStringProp(PKEY_FileExtension, pszExt);

    AddStringProp(PKEY_ItemPathDisplay, m_strPath.c_str());
}

// ----------------------------------------------------------------------------
// Media attributes via Media Foundation (best-effort; audio-only files and
// non-media files simply contribute nothing).
// ----------------------------------------------------------------------------

void CMediaMetadataStore::LoadMediaAttributes()
{
    if (!EnsureMediaFoundationStarted())
        return;

    CComPtr<IMFSourceReader> spReader;
    HRESULT hr = MFCreateSourceReaderFromURL(m_strPath.c_str(), NULL, &spReader);
    if (FAILED(hr) || !spReader)
        return;

    // Presentation-wide attributes (duration in 100ns units).
    PROPVARIANT prop;
    PropVariantInit(&prop);
    if (SUCCEEDED(spReader->GetPresentationAttribute(
            MF_SOURCE_READER_MEDIASOURCE, MF_PD_DURATION, &prop)) &&
        prop.vt == VT_UI8)
    {
        AddUInt64Prop(PKEY_Media_Duration, prop.uhVal.QuadPart);
    }
    PropVariantClear(&prop);

    // Video stream attributes.
    CComPtr<IMFMediaType> spVideoType;
    if (SUCCEEDED(spReader->GetNativeMediaType(
            MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, &spVideoType)) && spVideoType)
    {
        UINT32 uWidth = 0, uHeight = 0;
        if (SUCCEEDED(MFGetAttributeSize(spVideoType, MF_MT_FRAME_SIZE,
                                         &uWidth, &uHeight)) &&
            uWidth > 0 && uHeight > 0)
        {
            AddUInt32Prop(PKEY_Video_FrameWidth, uWidth);
            AddUInt32Prop(PKEY_Video_FrameHeight, uHeight);
        }

        UINT32 uNum = 0, uDen = 0;
        if (SUCCEEDED(MFGetAttributeRatio(spVideoType, MF_MT_FRAME_RATE,
                                          &uNum, &uDen)) &&
            uNum > 0 && uDen > 0)
        {
            AddUInt32Prop(PKEY_Video_FrameRate, (uNum + (uDen >> 1)) / uDen);
        }

        UINT32 uBitrate = 0;
        if (SUCCEEDED(spVideoType->GetUINT32(MF_MT_AVG_BITRATE, &uBitrate)) &&
            uBitrate > 0)
        {
            AddUInt32Prop(PKEY_Video_EncodingBitrate, uBitrate);
        }

        GUID guidSubtype = {};
        if (SUCCEEDED(spVideoType->GetGUID(MF_MT_SUBTYPE, &guidSubtype)))
            AddStringProp(PKEY_Video_Compression, FourCCName(guidSubtype).c_str());
    }

    // Audio stream attributes.
    CComPtr<IMFMediaType> spAudioType;
    if (SUCCEEDED(spReader->GetNativeMediaType(
            MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, &spAudioType)) && spAudioType)
    {
        UINT32 uBitrate = 0;
        if (SUCCEEDED(spAudioType->GetUINT32(MF_MT_AVG_BITRATE, &uBitrate)) &&
            uBitrate > 0)
        {
            AddUInt32Prop(PKEY_Audio_EncodingBitrate, uBitrate);
        }

        UINT32 uChannels = 0;
        if (SUCCEEDED(spAudioType->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &uChannels)) &&
            uChannels > 0)
        {
            AddUInt32Prop(PKEY_Audio_ChannelCount, uChannels);
        }

        UINT32 uSampleRate = 0;
        if (SUCCEEDED(spAudioType->GetUINT32(
                MF_MT_AUDIO_SAMPLES_PER_SECOND, &uSampleRate)) &&
            uSampleRate > 0)
        {
            AddUInt32Prop(PKEY_Audio_SampleRate, uSampleRate);
        }
    }
}

// ----------------------------------------------------------------------------
// Image dimensions via GDI+ (stills only; skipped for non-image extensions).
// ----------------------------------------------------------------------------

void CMediaMetadataStore::LoadImageAttributes()
{
    LPCWSTR pszExt = PathFindExtensionW(m_strPath.c_str());
    if (!IsImageExtension(pszExt))
        return;

    if (!EnsureGdiplusStarted())
        return;

    Gdiplus::Bitmap bitmap(m_strPath.c_str(), FALSE);
    if (bitmap.GetLastStatus() != Gdiplus::Ok)
        return;

    const UINT uWidth = bitmap.GetWidth();
    const UINT uHeight = bitmap.GetHeight();
    if (uWidth > 0 && uHeight > 0)
    {
        AddUInt32Prop(PKEY_Image_HorizontalSize, uWidth);
        AddUInt32Prop(PKEY_Image_VerticalSize, uHeight);
    }
}

// ============================================================================
// CSafePropertyStore -- thread-safe IPropertyStore wrapper
// ============================================================================
// Serializes access to an inner shell property handler with a critical
// section (the imaging pipeline touches metadata from several threads) and
// normalizes values through PSCoerceToCanonicalValue. An optional extension
// store fills in values the inner handler does not provide (see
// CMediaMetadataStore). Write-back is delegated to the inner handler; when
// created in read-only mode SetValue/Commit fail with STG_E_ACCESSDENIED.
// ============================================================================
class CSafePropertyStore : public IPropertyStore, public IPropertyStoreCapabilities
{
public:
    static HRESULT Create(IPropertyStore* pInner, BOOL fReadWrite,
                          IPropertyStore* pExtension, CSafePropertyStore** ppStore);

    // IUnknown
    STDMETHOD(QueryInterface)(REFIID riid, void** ppv);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

    // IPropertyStore
    STDMETHOD(GetCount)(DWORD* pcProps);
    STDMETHOD(GetAt)(DWORD iProp, PROPERTYKEY* pkey);
    STDMETHOD(GetValue)(REFPROPERTYKEY key, PROPVARIANT* pv);
    STDMETHOD(SetValue)(REFPROPERTYKEY key, REFPROPVARIANT propvar);
    STDMETHOD(Commit)();

    // IPropertyStoreCapabilities
    STDMETHOD(IsPropertyWritable)(REFPROPERTYKEY key);

private:
    CSafePropertyStore(IPropertyStore* pInner, BOOL fReadWrite,
                       IPropertyStore* pExtension);
    ~CSafePropertyStore();

    bool HasKeyInStore(IPropertyStore* pStore, REFPROPERTYKEY key);

    CRITICAL_SECTION        m_cs;
    CComPtr<IPropertyStore> m_spInner;
    CComPtr<IPropertyStore> m_spExtension;
    BOOL                    m_fReadWrite;
    LONG                    m_cRef;
};

// ============================================================================
// CSafePropertyStore implementation
// ============================================================================

CSafePropertyStore::CSafePropertyStore(IPropertyStore* pInner, BOOL fReadWrite,
                                       IPropertyStore* pExtension)
    : m_fReadWrite(fReadWrite)
    , m_cRef(1)
{
    ::InitializeCriticalSection(&m_cs);
    m_spInner = pInner;
    m_spExtension = pExtension;
}

CSafePropertyStore::~CSafePropertyStore()
{
    ::DeleteCriticalSection(&m_cs);
}

HRESULT CSafePropertyStore::Create(IPropertyStore* pInner, BOOL fReadWrite,
                                   IPropertyStore* pExtension,
                                   CSafePropertyStore** ppStore)
{
    if (!ppStore)
        return E_POINTER;
    *ppStore = NULL;

    CSafePropertyStore* pStore =
        new (std::nothrow) CSafePropertyStore(pInner, fReadWrite, pExtension);
    if (!pStore)
        return E_OUTOFMEMORY;

    *ppStore = pStore;
    return S_OK;
}

STDMETHODIMP CSafePropertyStore::QueryInterface(REFIID riid, void** ppv)
{
    if (!ppv)
        return E_POINTER;
    *ppv = NULL;

    if (IsEqualIID(riid, __uuidof(IUnknown)) ||
        IsEqualIID(riid, __uuidof(IPropertyStore)))
    {
        *ppv = static_cast<IPropertyStore*>(this);
        AddRef();
        return S_OK;
    }

    if (IsEqualIID(riid, __uuidof(IPropertyStoreCapabilities)))
    {
        *ppv = static_cast<IPropertyStoreCapabilities*>(this);
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CSafePropertyStore::AddRef()
{
    return (ULONG)InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CSafePropertyStore::Release()
{
    const LONG cRef = InterlockedDecrement(&m_cRef);
    if (cRef == 0)
        delete this;
    return (ULONG)cRef;
}

bool CSafePropertyStore::HasKeyInStore(IPropertyStore* pStore, REFPROPERTYKEY key)
{
    if (!pStore)
        return false;

    DWORD cProps = 0;
    if (FAILED(pStore->GetCount(&cProps)))
        return false;

    for (DWORD i = 0; i < cProps; ++i)
    {
        PROPERTYKEY keyAt;
        if (SUCCEEDED(pStore->GetAt(i, &keyAt)) && IsEqualPropertyKey(keyAt, key))
            return true;
    }

    return false;
}

STDMETHODIMP CSafePropertyStore::GetCount(DWORD* pcProps)
{
    if (!pcProps)
        return E_POINTER;

    ::EnterCriticalSection(&m_cs);

    DWORD cInner = 0;
    if (m_spInner)
        m_spInner->GetCount(&cInner);

    DWORD cExtUnique = 0;
    if (m_spExtension)
    {
        DWORD cExt = 0;
        if (SUCCEEDED(m_spExtension->GetCount(&cExt)))
        {
            for (DWORD i = 0; i < cExt; ++i)
            {
                PROPERTYKEY key;
                if (SUCCEEDED(m_spExtension->GetAt(i, &key)) &&
                    !HasKeyInStore(m_spInner, key))
                {
                    ++cExtUnique;
                }
            }
        }
    }

    ::LeaveCriticalSection(&m_cs);

    *pcProps = cInner + cExtUnique;
    return S_OK;
}

STDMETHODIMP CSafePropertyStore::GetAt(DWORD iProp, PROPERTYKEY* pkey)
{
    if (!pkey)
        return E_POINTER;

    ::EnterCriticalSection(&m_cs);

    DWORD cInner = 0;
    if (m_spInner)
        m_spInner->GetCount(&cInner);

    if (m_spInner && iProp < cInner)
    {
        const HRESULT hr = m_spInner->GetAt(iProp, pkey);
        ::LeaveCriticalSection(&m_cs);
        return hr;
    }

    if (m_spExtension)
    {
        DWORD cExt = 0;
        m_spExtension->GetCount(&cExt);
        DWORD nSkip = iProp - cInner;

        for (DWORD i = 0; i < cExt; ++i)
        {
            PROPERTYKEY key;
            if (FAILED(m_spExtension->GetAt(i, &key)))
                continue;
            if (HasKeyInStore(m_spInner, key))
                continue;
            if (nSkip == 0)
            {
                *pkey = key;
                ::LeaveCriticalSection(&m_cs);
                return S_OK;
            }
            --nSkip;
        }
    }

    ::LeaveCriticalSection(&m_cs);
    return E_INVALIDARG;
}

STDMETHODIMP CSafePropertyStore::GetValue(REFPROPERTYKEY key, PROPVARIANT* pv)
{
    if (!pv)
        return E_POINTER;
    PropVariantInit(pv);

    ::EnterCriticalSection(&m_cs);

    HRESULT hr = S_OK;
    if (m_spInner)
    {
        hr = m_spInner->GetValue(key, pv);
        if (FAILED(hr))
        {
            PropVariantClear(pv);
            ::LeaveCriticalSection(&m_cs);
            return hr;
        }
    }

    // Fall back to the extension store only for keys the inner handler does
    // not expose (it reports those as VT_EMPTY).
    if (pv->vt == VT_EMPTY && m_spExtension)
    {
        PROPVARIANT pvExt;
        PropVariantInit(&pvExt);
        const HRESULT hrExt = m_spExtension->GetValue(key, &pvExt);
        if (SUCCEEDED(hrExt) && pvExt.vt != VT_EMPTY)
        {
            PropVariantClear(pv);
            *pv = pvExt; // ownership transferred
        }
        else
        {
            PropVariantClear(&pvExt);
        }
    }

    // Normalize to the key's canonical type, as the original did.
    if (pv->vt != VT_EMPTY)
    {
        if (FAILED(PSCoerceToCanonicalValue(key, pv)))
            PropVariantClear(pv);
    }

    ::LeaveCriticalSection(&m_cs);
    return S_OK;
}

STDMETHODIMP CSafePropertyStore::SetValue(REFPROPERTYKEY key, REFPROPVARIANT propvar)
{
    ::EnterCriticalSection(&m_cs);

    if (!m_fReadWrite || !m_spInner)
    {
        ::LeaveCriticalSection(&m_cs);
        return STG_E_ACCESSDENIED;
    }

    const HRESULT hr = m_spInner->SetValue(key, propvar);
    ::LeaveCriticalSection(&m_cs);
    return hr;
}

STDMETHODIMP CSafePropertyStore::Commit()
{
    ::EnterCriticalSection(&m_cs);

    HRESULT hr = S_OK;
    if (m_spInner)
    {
        if (!m_fReadWrite)
            hr = STG_E_ACCESSDENIED;
        else
            hr = m_spInner->Commit();
    }

    ::LeaveCriticalSection(&m_cs);
    return hr;
}

STDMETHODIMP CSafePropertyStore::IsPropertyWritable(REFPROPERTYKEY key)
{
    ::EnterCriticalSection(&m_cs);

    HRESULT hr = S_FALSE; // default: not writable through this store
    if (m_fReadWrite && m_spInner)
    {
        CComPtr<IPropertyStoreCapabilities> spCaps;
        if (SUCCEEDED(m_spInner->QueryInterface(
                __uuidof(IPropertyStoreCapabilities), (void**)&spCaps)))
        {
            hr = spCaps->IsPropertyWritable(key);
        }
    }

    ::LeaveCriticalSection(&m_cs);
    return hr;
}

// ============================================================================
// WLXPSGetItemPropertyHandler implementation
// ============================================================================
namespace
{

HRESULT WLXPSGetItemPropertyHandlerImpl(IUnknown* pItem, DWORD dwAccessMode,
                                        REFIID riid, void** ppv)
{
    *ppv = NULL;

    // A NULL item is the documented out-of-scope case. The original binary
    // reported E_NOTIMPL with a NULL out-parameter (pinned by the test
    // suites), not E_POINTER.
    if (!pItem)
        return E_NOTIMPL;

    const BOOL fReadWrite = (dwAccessMode != 0);

    // ---- Resolve the item to an IShellItem and, when possible, a path. ----
    CComPtr<IShellItem> spShellItem;
    pItem->QueryInterface(__uuidof(IShellItem), (void**)&spShellItem);

    std::wstring strPath;
    if (spShellItem)
    {
        LPWSTR pszPath = NULL;
        if (SUCCEEDED(spShellItem->GetDisplayName(SIGDN_FILESYSPATH, &pszPath)) &&
            pszPath)
        {
            strPath.assign(pszPath);
            CoTaskMemFree(pszPath);
        }
    }
    else
    {
        // Accept any object that hands us a file path (e.g. IPersistFile),
        // then promote it to a shell item so the property system is used.
        CComPtr<IPersistFile> spPersistFile;
        pItem->QueryInterface(__uuidof(IPersistFile), (void**)&spPersistFile);
        if (spPersistFile)
        {
            LPOLESTR pszPath = NULL;
            if (SUCCEEDED(spPersistFile->GetCurFile(&pszPath)) && pszPath)
            {
                strPath.assign(pszPath);
                CoTaskMemFree(pszPath);
            }
            if (!strPath.empty())
            {
                // Ignore failure; the extension store can stand alone.
                SHCreateItemFromParsingName(strPath.c_str(), NULL,
                                            IID_PPV_ARGS(&spShellItem));
            }
        }
    }

    if (!spShellItem && strPath.empty())
        return E_INVALIDARG;

    // ---- Primary store: the shell-registered property handler. ------------
    CComPtr<IPropertyStore> spInner;
    if (spShellItem)
    {
        HRESULT hrHandler = PSGetItemPropertyHandler(
            spShellItem.p, fReadWrite, IID_PPV_ARGS(&spInner));
        if (FAILED(hrHandler) && fReadWrite)
        {
            // A writable handler was requested but is unavailable; fall back
            // to a read-only one so metadata can still be read.
            spInner.Release();
            PSGetItemPropertyHandler(spShellItem.p, FALSE, IID_PPV_ARGS(&spInner));
        }
    }

    // ---- Extension store: file / media attributes derived locally. --------
    CComPtr<IPropertyStore> spExtension;
    if (!strPath.empty())
    {
        CMediaMetadataStore* pMedia = NULL;
        if (SUCCEEDED(CMediaMetadataStore::Create(strPath.c_str(), &pMedia)) &&
            pMedia)
        {
            spExtension = pMedia; // AddRef
            pMedia->Release();    // drop the creation reference
        }
    }

    // ---- Wrap in the thread-safe store and hand out the requested IID. ----
    CSafePropertyStore* pStore = NULL;
    HRESULT hr = CSafePropertyStore::Create(spInner, fReadWrite,
                                            spExtension, &pStore);
    if (FAILED(hr))
        return hr;

    hr = pStore->QueryInterface(riid, ppv);
    pStore->Release();
    return hr;
}

} // namespace

// Ord 1: HRESULT WLXPSGetItemPropertyHandler(IUnknown*, DWORD, REFIID, void**)
// Returns an IPropertyStore for the given shell item. dwAccessMode: 0 = read-
// only, nonzero = read-write. A NULL item yields E_NOTIMPL with a NULL
// out-parameter (matching the original binary); a NULL ppv fails with
// E_POINTER.
extern "C" __declspec(dllexport) HRESULT __stdcall WLXPSGetItemPropertyHandler(
    IUnknown* pItem, DWORD dwAccessMode, REFIID riid, void** ppv)
{
    if (!ppv)
        return E_POINTER;

    try
    {
        return WLXPSGetItemPropertyHandlerImpl(pItem, dwAccessMode, riid, ppv);
    }
    catch (...)
    {
        *ppv = NULL;
        return E_FAIL;
    }
}


