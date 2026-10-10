// TranscodeMetadata.cpp - Transcode metadata and process tracking implementation

#include "pch.h"
#include "TranscodeMetadata.h"
#include <propsys.h>
#include <propvarutil.h>
#include <olectl.h>
#include <oleauto.h>

namespace HMRAVSource
{

// ============================================================================
// File-local helpers
// ============================================================================

// Assigns a string PROPVARIANT to a CString. Returns MF_E_ATTRIBUTENOTFOUND for
// non-string variants.
static HRESULT AssignStringVariant(const PROPVARIANT& var, ATL::CString* pstrValue)
{
    switch (var.vt)
    {
    case VT_LPWSTR:
        if (var.pwszVal)
            *pstrValue = var.pwszVal;
        break;
    case VT_LPSTR:
        if (var.pszVal)
            *pstrValue = CString(var.pszVal);
        break;
    case VT_BSTR:
        if (var.bstrVal)
            *pstrValue = var.bstrVal;
        break;
    default:
        return MF_E_ATTRIBUTENOTFOUND;
    }

    return S_OK;
}

// Candidate WIC metadata query paths for the common shell property keys, in
// priority order. XMP paths come first (codec-independent); the EXIF/TIFF tag
// paths cover JPEG and TIFF containers.
static const WCHAR* const* GetWicPathsForKey(REFPROPERTYKEY key)
{
    static const WCHAR* const kTitlePaths[] = {
        L"/xmp/dc:title", L"/ifd/{ushort=270}", L"/app1/ifd/{ushort=270}", nullptr
    };
    static const WCHAR* const kAuthorPaths[] = {
        L"/xmp/dc:creator", L"/ifd/{ushort=315}", L"/app1/ifd/{ushort=315}", nullptr
    };
    static const WCHAR* const kCommentPaths[] = {
        L"/xmp/dc:description", L"/ifd/{ushort=270}", L"/app1/ifd/{ushort=270}", nullptr
    };
    static const WCHAR* const kCopyrightPaths[] = {
        L"/xmp/dc:rights", L"/ifd/{ushort=33432}", L"/app1/ifd/{ushort=33432}", nullptr
    };
    static const WCHAR* const kDateTakenPaths[] = {
        L"/xmp/exif:DateTimeOriginal", L"/app1/ifd/exif/{ulong=36867}",
        L"/ifd/exif/{ulong=36867}", L"/app1/ifd/{ushort=306}", L"/ifd/{ushort=306}", nullptr
    };
    static const WCHAR* const kRatingPaths[] = {
        L"/xmp/xmp:Rating", L"/app1/ifd/{ushort=18248}", L"/ifd/{ushort=18248}", nullptr
    };

    if (IsEqualPropertyKey(key, PKEY_Title))
        return kTitlePaths;
    if (IsEqualPropertyKey(key, PKEY_Author))
        return kAuthorPaths;
    if (IsEqualPropertyKey(key, PKEY_Comment))
        return kCommentPaths;
    if (IsEqualPropertyKey(key, PKEY_Copyright))
        return kCopyrightPaths;
    if (IsEqualPropertyKey(key, PKEY_DateTaken))
        return kDateTakenPaths;
    if (IsEqualPropertyKey(key, PKEY_Rating))
        return kRatingPaths;

    return nullptr;
}

// WIC metadata fallback for formats without an OLE property store (JPEG, PNG,
// MP4, ...). Queries the frame-level and container-level metadata readers with
// the candidate paths for the requested property key.
static HRESULT GetPropertyStringViaWic(LPCWSTR pszFilePath, REFPROPERTYKEY key, ATL::CString* pstrValue)
{
    *pstrValue = CString();

    const WCHAR* const* pPaths = GetWicPathsForKey(key);
    if (!pPaths)
        return MF_E_ATTRIBUTENOTFOUND;

    CComPtr<IWICImagingFactory> spWicFactory;
    HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&spWicFactory));
    if (FAILED(hr))
        return hr;

    CComPtr<IWICBitmapDecoder> spDecoder;
    hr = spWicFactory->CreateDecoderFromFilename(pszFilePath, nullptr, GENERIC_READ,
        WICDecodeMetadataCacheOnLoad, &spDecoder);
    if (FAILED(hr))
        return hr;

    CComPtr<IWICBitmapFrameDecode> spFrame;
    hr = spDecoder->GetFrame(0, &spFrame);
    if (FAILED(hr))
        return hr;

    // The frame-level reader is required; the container-level reader is
    // optional (some codecs expose metadata such as XMP only there).
    CComPtr<IWICMetadataQueryReader> spFrameReader;
    hr = spFrame->GetMetadataQueryReader(&spFrameReader);
    if (FAILED(hr))
        return hr;

    CComPtr<IWICMetadataQueryReader> spContainerReader;
    spDecoder->GetMetadataQueryReader(&spContainerReader);

    CComPtr<IWICMetadataQueryReader>* readers[2] = { &spFrameReader, &spContainerReader };
    for (int r = 0; r < 2; ++r)
    {
        if (!readers[r] || !*readers[r])
            continue;

        for (int p = 0; pPaths[p] != nullptr; ++p)
        {
            PROPVARIANT var;
            PropVariantInit(&var);
            hr = (*readers[r])->GetMetadataByName(pPaths[p], &var);
            if (SUCCEEDED(hr))
            {
                hr = AssignStringVariant(var, pstrValue);
                PropVariantClear(&var);
                if (SUCCEEDED(hr))
                    return S_OK;
            }
            else
            {
                PropVariantClear(&var);
            }
        }
    }

    return MF_E_ATTRIBUTENOTFOUND;
}

// Opens the shell property store for a media file for writing. The shell
// property store routes each property key to the file's matching property
// handler (OLE property sets for WMV/ASF/AVI, codec metadata for WMA/MP3,
// EXIF/XMP for images), creating property sets on demand.
static HRESULT OpenWritablePropertyStore(LPCWSTR pszFilePath, IPropertyStore** ppStore)
{
    if (!pszFilePath || !ppStore)
        return E_POINTER;

    *ppStore = nullptr;

    CComPtr<IShellItem2> spItem;
    HRESULT hr = SHCreateItemFromParsingName(pszFilePath, nullptr, IID_PPV_ARGS(&spItem));
    if (FAILED(hr))
        return hr;

    return spItem->GetPropertyStore(GPS_DEFAULT, IID_PPV_ARGS(ppStore));
}

// Writes one string property to the store. Empty values are skipped.
static HRESULT SetStringProperty(IPropertyStore* pStore, REFPROPERTYKEY key, const ATL::CString& strValue)
{
    if (strValue.IsEmpty())
        return S_OK;

    PROPVARIANT var;
    HRESULT hr = InitPropVariantFromString(static_cast<LPCWSTR>(strValue), &var);
    if (FAILED(hr))
        return hr;

    hr = pStore->SetValue(key, var);
    PropVariantClear(&var);
    return hr;
}


// ============================================================================
// TranscodeMetadataParser
// ============================================================================

TranscodeMetadataParser::TranscodeMetadataParser()
    : m_fInitialized(false)
{
}

TranscodeMetadataParser::~TranscodeMetadataParser()
{
    Shutdown();
}

HRESULT TranscodeMetadataParser::Initialize()
{
    m_fInitialized = true;
    return S_OK;
}

HRESULT TranscodeMetadataParser::Shutdown()
{
    m_fInitialized = false;
    return S_OK;
}

HRESULT TranscodeMetadataParser::ParseMetadata(LPCWSTR pszFilePath, MediaMetadata* pMetadata)
{
    if (!pszFilePath || !pMetadata)
        return E_POINTER;

    if (!m_fInitialized)
        return E_UNEXPECTED;

    *pMetadata = MediaMetadata();

    return ParseFromPropertyStore(pszFilePath, pMetadata);
}

HRESULT TranscodeMetadataParser::ParseMetadataFromSource(IMFSourceReader* pReader, MediaMetadata* pMetadata)
{
    if (!pReader || !pMetadata)
        return E_POINTER;

    *pMetadata = MediaMetadata();

    CComPtr<IMFAttributes> spAttributes;
    HRESULT hr = pReader->QueryInterface(IID_PPV_ARGS(&spAttributes));
    if (FAILED(hr))
        return hr;

    return ParseFromMFAttributes(spAttributes, pMetadata);
}

HRESULT TranscodeMetadataParser::ParseMetadataFromSample(IMFSample* pSample, MediaMetadata* pMetadata)
{
    if (!pSample || !pMetadata)
        return E_POINTER;

    *pMetadata = MediaMetadata();

    LONGLONG llTimestamp = 0;
    if (SUCCEEDED(pSample->GetSampleTime(&llTimestamp)))
    {
        pMetadata->llDurationHns = llTimestamp;
    }

    return S_OK;
}

HRESULT TranscodeMetadataParser::GetTitle(LPCWSTR pszFilePath, ATL::CString* pstrTitle)
{
    if (!pstrTitle)
        return E_POINTER;

    return GetPropertyString(pszFilePath, PKEY_Title, pstrTitle);
}

HRESULT TranscodeMetadataParser::GetArtist(LPCWSTR pszFilePath, ATL::CString* pstrArtist)
{
    if (!pstrArtist)
        return E_POINTER;

    return GetPropertyString(pszFilePath, PKEY_Author, pstrArtist);
}

HRESULT TranscodeMetadataParser::GetDuration(LPCWSTR pszFilePath, LONGLONG* pllDurationHns)
{
    if (!pllDurationHns)
        return E_POINTER;

    *pllDurationHns = 0;

    CComPtr<IMFSourceReader> spReader;
    HRESULT hr = MFCreateSourceReaderFromURL(pszFilePath, nullptr, &spReader);
    if (FAILED(hr))
        return hr;

    PROPVARIANT var;
    PropVariantInit(&var);
    hr = spReader->GetPresentationAttribute(
        MF_SOURCE_READER_MEDIASOURCE,
        MF_PD_DURATION,
        &var);

    if (SUCCEEDED(hr))
    {
        if (var.vt == VT_UI8)
        {
            *pllDurationHns = static_cast<LONGLONG>(var.uhVal.QuadPart);
        }
        else
        {
            hr = MF_E_ATTRIBUTENOTFOUND;
        }
    }

    PropVariantClear(&var);
    return hr;
}

HRESULT TranscodeMetadataParser::GetResolution(LPCWSTR pszFilePath, DWORD* pdwWidth, DWORD* pdwHeight)
{
    if (!pdwWidth || !pdwHeight)
        return E_POINTER;

    *pdwWidth = 0;
    *pdwHeight = 0;

    CComPtr<IMFSourceReader> spReader;
    HRESULT hr = MFCreateSourceReaderFromURL(pszFilePath, nullptr, &spReader);
    if (FAILED(hr))
        return hr;

    CComPtr<IMFMediaType> spType;
    hr = spReader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, &spType);
    if (FAILED(hr))
        return hr;

    UINT32 uWidth = 0, uHeight = 0;
    hr = MFGetAttributeSize(spType, MF_MT_FRAME_SIZE, &uWidth, &uHeight);
    if (SUCCEEDED(hr))
    {
        *pdwWidth = uWidth;
        *pdwHeight = uHeight;
    }

    return hr;
}

HRESULT TranscodeMetadataParser::WriteMetadata(LPCWSTR pszFilePath, const MediaMetadata& metadata)
{
    if (!pszFilePath)
        return E_POINTER;

    // Persist the writable metadata through the shell property-store path:
    // the store routes each property key to the file's matching property
    // handler, exactly like the reference implementation.
    CComPtr<IPropertyStore> spStore;
    HRESULT hr = OpenWritablePropertyStore(pszFilePath, &spStore);
    if (FAILED(hr))
        return hr;

    hr = SetStringProperty(spStore, PKEY_Title, metadata.strTitle);
    if (FAILED(hr))
        return hr;
    hr = SetStringProperty(spStore, PKEY_Author, metadata.strArtist);
    if (FAILED(hr))
        return hr;
    hr = SetStringProperty(spStore, PKEY_Music_AlbumTitle, metadata.strAlbum);
    if (FAILED(hr))
        return hr;
    hr = SetStringProperty(spStore, PKEY_Music_Genre, metadata.strGenre);
    if (FAILED(hr))
        return hr;
    hr = SetStringProperty(spStore, PKEY_Comment, metadata.strDescription);
    if (FAILED(hr))
        return hr;
    hr = SetStringProperty(spStore, PKEY_Copyright, metadata.strCopyright);
    if (FAILED(hr))
        return hr;
    hr = SetStringProperty(spStore, PKEY_ApplicationName, metadata.strSoftware);
    if (FAILED(hr))
        return hr;

    // The rating is stored as a star count (VT_UI4), not a string.
    if (!metadata.strRating.IsEmpty())
    {
        PROPVARIANT var;
        hr = InitPropVariantFromUInt32(
            static_cast<UINT32>(_ttoi(metadata.strRating)), &var);
        if (FAILED(hr))
            return hr;

        hr = spStore->SetValue(PKEY_Rating, var);
        PropVariantClear(&var);
        if (FAILED(hr))
            return hr;
    }

    // The date-taken is stored as a string; the property system coerces it to
    // the canonical FILETIME type for System.Photo.DateTaken.
    hr = SetStringProperty(spStore, PKEY_DateTaken, metadata.strDateTaken);
    if (FAILED(hr))
        return hr;

    return spStore->Commit();
}

HRESULT TranscodeMetadataParser::SetMetadataAttribute(LPCWSTR pszFilePath, REFGUID guidKey, LPCWSTR pszValue)
{
    if (!pszFilePath)
        return E_POINTER;

    // Map the attribute GUID to its writable property-store key. The GUID is
    // the property set's FMTID; the property ID is the first user-defined
    // property of the set (PID_FIRST_USABLE), the standard convention for
    // custom metadata attributes.
    PROPERTYKEY key;
    key.fmtid = guidKey;
    key.pid = 2; // PID_FIRST_USABLE

    CComPtr<IPropertyStore> spStore;
    HRESULT hr = OpenWritablePropertyStore(pszFilePath, &spStore);
    if (FAILED(hr))
        return hr;

    if (!pszValue)
        return E_POINTER;

    PROPVARIANT var;
    hr = InitPropVariantFromString(pszValue, &var);
    if (FAILED(hr))
        return hr;

    hr = spStore->SetValue(key, var);
    PropVariantClear(&var);
    if (FAILED(hr))
        return hr;

    return spStore->Commit();
}

HRESULT TranscodeMetadataParser::ExtractThumbnail(LPCWSTR pszFilePath, BYTE** ppData, DWORD* pcbData, GUID* pFormat)
{
    if (ppData) *ppData = nullptr;
    if (pcbData) *pcbData = 0;
    if (pFormat) *pFormat = GUID_NULL;

    if (!pszFilePath || !ppData || !pcbData)
        return E_POINTER;

    // Extract the first video frame through the MF source reader (asking for
    // RGB32 so the reader converts for us), then encode it as a JPEG via WIC.
    CComPtr<IMFSourceReader> spReader;
    HRESULT hr = MFCreateSourceReaderFromURL(pszFilePath, nullptr, &spReader);
    if (FAILED(hr))
        return hr;

    // Decode only the first video stream.
    (void)spReader->SetStreamSelection(MF_SOURCE_READER_ALL_STREAMS, FALSE);
    hr = spReader->SetStreamSelection(MF_SOURCE_READER_FIRST_VIDEO_STREAM, TRUE);
    if (FAILED(hr))
        return hr;

    // Negotiate RGB32 output.
    CComPtr<IMFMediaType> spType;
    hr = MFCreateMediaType(&spType);
    if (FAILED(hr))
        return hr;
    hr = spType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    if (FAILED(hr))
        return hr;
    hr = spType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32);
    if (FAILED(hr))
        return hr;
    hr = spReader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, nullptr, spType);
    if (FAILED(hr))
        return hr;

    DWORD dwStreamIndex = 0;
    DWORD dwStreamFlags = 0;
    LONGLONG llTimestamp = 0;
    CComPtr<IMFSample> spSample;
    hr = spReader->ReadSample(MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0,
        &dwStreamIndex, &dwStreamFlags, &llTimestamp, &spSample);
    if (FAILED(hr))
        return hr;
    if (dwStreamFlags & MF_SOURCE_READERF_ERROR)
        return E_FAIL;
    if (!spSample)
        return E_FAIL;

    // Frame size from the negotiated media type.
    UINT32 uWidth = 0;
    UINT32 uHeight = 0;
    hr = spReader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, &spType);
    if (FAILED(hr))
        return hr;
    hr = MFGetAttributeSize(spType, MF_MT_FRAME_SIZE, &uWidth, &uHeight);
    if (FAILED(hr))
        return hr;

    // Bound the decode: reject empty or absurdly large frames.
    if (uWidth == 0 || uHeight == 0 || uWidth > 8192 || uHeight > 8192 ||
        static_cast<UINT64>(uWidth) * static_cast<UINT64>(uHeight) > 64ull * 1024ull * 1024ull)
        return E_FAIL;

    CComPtr<IMFMediaBuffer> spBuffer;
    hr = spSample->GetBufferByIndex(0, &spBuffer);
    if (FAILED(hr))
        return hr;

    BYTE* pSrcData = nullptr;
    DWORD cbMaxLength = 0;
    DWORD cbCurrentLength = 0;
    hr = spBuffer->Lock(&pSrcData, &cbMaxLength, &cbCurrentLength);
    if (FAILED(hr))
        return hr;

    // Copy the frame into a tightly packed RGB32 buffer. The sample stride may
    // differ from the row size, so prefer the 2D buffer interface when present.
    const DWORD cbRowBytes = uWidth * 4;
    std::vector<BYTE> pixels(static_cast<size_t>(cbRowBytes) * uHeight);

    CComPtr<IMF2DBuffer> sp2DBuffer;
    if (SUCCEEDED(spBuffer->QueryInterface(IID_PPV_ARGS(&sp2DBuffer))) && sp2DBuffer)
    {
        BYTE* pScan0 = nullptr;
        LONG lStride = 0;
        hr = sp2DBuffer->Lock2D(&pScan0, &lStride);
        if (SUCCEEDED(hr))
        {
            // A negative stride means the buffer is stored bottom-up.
            LONG lAbsStride = (lStride < 0) ? -lStride : lStride;
            if (lAbsStride >= static_cast<LONG>(cbRowBytes))
            {
                for (UINT32 y = 0; y < uHeight; ++y)
                {
                    UINT32 uSrcRow = (lStride < 0) ? (uHeight - 1 - y) : y;
                    memcpy(&pixels[static_cast<size_t>(y) * cbRowBytes],
                           pScan0 + static_cast<size_t>(uSrcRow) * static_cast<size_t>(lAbsStride),
                           cbRowBytes);
                }
            }
            else
            {
                hr = E_FAIL;
            }
            sp2DBuffer->Unlock2D();
        }
        spBuffer->Unlock();
        if (FAILED(hr))
            return hr;
    }
    else
    {
        // Tightly packed fallback for 1D buffers.
        if (static_cast<UINT64>(cbCurrentLength) < static_cast<UINT64>(cbRowBytes) * uHeight)
        {
            spBuffer->Unlock();
            return E_FAIL;
        }
        for (UINT32 y = 0; y < uHeight; ++y)
            memcpy(&pixels[static_cast<size_t>(y) * cbRowBytes],
                   pSrcData + static_cast<size_t>(y) * cbRowBytes, cbRowBytes);
        spBuffer->Unlock();
    }

    // MFVideoFormat_RGB32 is BGRX on little-endian, matching
    // GUID_WICPixelFormat32bppBGR.
    CComPtr<IWICImagingFactory> spWicFactory;
    hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&spWicFactory));
    if (FAILED(hr))
        return hr;

    CComPtr<IWICBitmap> spWicBitmap;
    hr = spWicFactory->CreateBitmapFromMemory(uWidth, uHeight, GUID_WICPixelFormat32bppBGR,
        cbRowBytes, static_cast<UINT>(pixels.size()), pixels.data(), &spWicBitmap);
    if (FAILED(hr))
        return hr;

    // The JPEG encoder takes 24bppBGR; convert.
    CComPtr<IWICFormatConverter> spConverter;
    hr = spWicFactory->CreateFormatConverter(&spConverter);
    if (SUCCEEDED(hr))
        hr = spConverter->Initialize(spWicBitmap, GUID_WICPixelFormat24bppBGR,
            WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom);
    if (FAILED(hr))
        return hr;

    CComPtr<IStream> spStream;
    hr = CreateStreamOnHGlobal(nullptr, TRUE, &spStream);
    if (FAILED(hr))
        return hr;

    CComPtr<IWICBitmapEncoder> spEncoder;
    hr = spWicFactory->CreateEncoder(GUID_ContainerFormatJpeg, nullptr, &spEncoder);
    if (FAILED(hr))
        return hr;
    hr = spEncoder->Initialize(spStream, WICBitmapEncoderNoCache);
    if (FAILED(hr))
        return hr;

    CComPtr<IWICBitmapFrameEncode> spFrame;
    CComPtr<IPropertyBag2> spProps;
    hr = spEncoder->CreateNewFrame(&spFrame, &spProps);
    if (FAILED(hr))
        return hr;
    hr = spFrame->Initialize(spProps);
    if (FAILED(hr))
        return hr;
    hr = spFrame->SetSize(uWidth, uHeight);
    if (FAILED(hr))
        return hr;

    WICPixelFormatGUID guidPixelFormat = GUID_WICPixelFormat24bppBGR;
    hr = spFrame->SetPixelFormat(&guidPixelFormat);
    if (FAILED(hr))
        return hr;
    if (guidPixelFormat != GUID_WICPixelFormat24bppBGR)
        return E_FAIL; // the encoder cannot take the converted format

    hr = spFrame->WriteSource(spConverter, nullptr);
    if (FAILED(hr))
        return hr;
    hr = spFrame->Commit();
    if (FAILED(hr))
        return hr;
    hr = spEncoder->Commit();
    if (FAILED(hr))
        return hr;

    // Copy the encoded bytes out. The caller frees the buffer with
    // CoTaskMemFree.
    STATSTG stat = {};
    hr = spStream->Stat(&stat, STATFLAG_NONAME);
    if (FAILED(hr))
        return hr;
    if (stat.cbSize.HighPart != 0 || stat.cbSize.LowPart == 0)
        return E_FAIL;

    DWORD cbData = stat.cbSize.LowPart;
    BYTE* pData = static_cast<BYTE*>(CoTaskMemAlloc(cbData));
    if (!pData)
        return E_OUTOFMEMORY;

    LARGE_INTEGER liZero = {};
    spStream->Seek(liZero, STREAM_SEEK_SET, nullptr);
    hr = spStream->Read(pData, cbData, nullptr);
    if (FAILED(hr))
    {
        CoTaskMemFree(pData);
        return hr;
    }

    *ppData = pData;
    *pcbData = cbData;
    if (pFormat)
        *pFormat = GUID_ContainerFormatJpeg;

    return S_OK;
}

HRESULT TranscodeMetadataParser::GetPropertyString(LPCWSTR pszFilePath, REFPROPERTYKEY key, ATL::CString* pstrValue)
{
    if (!pszFilePath || !pstrValue)
        return E_POINTER;

    *pstrValue = CString();

    CComPtr<IPropertySetStorage> spPropSetStorage;
    HRESULT hr = StgOpenStorageEx(
        pszFilePath,
        STGM_READ | STGM_SHARE_DENY_WRITE,
        STGFMT_FILE,
        0,
        nullptr,
        nullptr,
        IID_IPropertySetStorage,
        reinterpret_cast<void**>(&spPropSetStorage));

    CComPtr<IPropertyStore> spStore;
    if (SUCCEEDED(hr))
    {
        CComPtr<IPropertyStorage> spPropStorage;
        hr = spPropSetStorage->Open(FMTID_SummaryInformation, STGM_READ | STGM_SHARE_EXCLUSIVE, &spPropStorage);
        if (SUCCEEDED(hr))
            hr = spPropStorage->QueryInterface(IID_PPV_ARGS(&spStore));
    }

    if (FAILED(hr))
    {
        // Formats without an OLE property store (JPEG, PNG, MP4, ...) fall
        // back to the WIC metadata query readers.
        return GetPropertyStringViaWic(pszFilePath, key, pstrValue);
    }

    PROPVARIANT var;
    PropVariantInit(&var);
    hr = spStore->GetValue(key, &var);
    if (SUCCEEDED(hr))
        hr = AssignStringVariant(var, pstrValue);

    PropVariantClear(&var);
    return hr;
}

HRESULT TranscodeMetadataParser::GetPropertyInt(LPCWSTR pszFilePath, REFPROPERTYKEY key, INT* piValue)
{
    if (!pszFilePath || !piValue)
        return E_POINTER;

    *piValue = 0;

    ATL::CString strValue;
    HRESULT hr = GetPropertyString(pszFilePath, key, &strValue);
    if (SUCCEEDED(hr) && !strValue.IsEmpty())
        *piValue = _ttoi(strValue.GetString());

    return hr;
}

HRESULT TranscodeMetadataParser::GetPropertyDateTime(LPCWSTR pszFilePath, REFPROPERTYKEY key, SYSTEMTIME* pSystemTime)
{
    if (!pSystemTime)
        return E_POINTER;
    if (!pszFilePath || !pszFilePath[0])
        return E_INVALIDARG;

    ZeroMemory(pSystemTime, sizeof(SYSTEMTIME));

    // The original binary reads date/time properties through the shell
    // property store, where they are exposed as VT_FILETIME values.
    // SHGetPropertyStoreFromParsingName covers filesystem paths and shell
    // namespace items alike, unlike StgOpenStorageEx which only works for
    // formats with an OLE property storage.
    CComPtr<IPropertyStore> spStore;
    HRESULT hr = SHGetPropertyStoreFromParsingName(
        pszFilePath, nullptr, GPS_READWRITE, IID_PPV_ARGS(&spStore));
    if (SUCCEEDED(hr))
    {
        PROPVARIANT var;
        PropVariantInit(&var);
        hr = spStore->GetValue(key, &var);
        if (SUCCEEDED(hr))
        {
            switch (var.vt)
            {
            case VT_FILETIME:
                hr = FileTimeToSystemTime(&var.filetime, pSystemTime)
                         ? S_OK
                         : HRESULT_FROM_WIN32(GetLastError());
                break;
            case VT_DATE:
                hr = (VariantTimeToSystemTime(var.date, pSystemTime) != 0)
                         ? S_OK
                         : E_FAIL;
                break;
            default:
                hr = E_FAIL; // fall through to the string parse below
                break;
            }
        }
        PropVariantClear(&var);
    }

    if (SUCCEEDED(hr))
        return hr;

    // Fallback: read the formatted string form and parse the leading
    // ISO-8601 date ("2026-10-08T14:30:00" or "2026-10-08 14:30:00").
    ATL::CString strValue;
    if (SUCCEEDED(GetPropertyString(pszFilePath, key, &strValue)) && !strValue.IsEmpty())
    {
        SYSTEMTIME st = {};
        if (strValue.GetLength() >= 10 &&
            swscanf_s(strValue.GetString(), L"%4hd-%2hd-%2hd",
                      &st.wYear, &st.wMonth, &st.wDay) == 3 &&
            st.wYear >= 1601 && st.wMonth >= 1 && st.wMonth <= 12 &&
            st.wDay >= 1 && st.wDay <= 31)
        {
            if (strValue.GetLength() >= 19)
            {
                swscanf_s(strValue.GetString() + 11, L"%2hd:%2hd:%2hd",
                          &st.wHour, &st.wMinute, &st.wSecond);
            }
            *pSystemTime = st;
            return S_OK;
        }
        return E_FAIL;
    }

    return hr;
}

HRESULT TranscodeMetadataParser::ParseFromPropertyStore(LPCWSTR pszFilePath, MediaMetadata* pMetadata)
{
    if (!pszFilePath || !pMetadata)
        return E_POINTER;

    // Parse basic properties
    GetPropertyString(pszFilePath, PKEY_Title, &pMetadata->strTitle);
    GetPropertyString(pszFilePath, PKEY_Author, &pMetadata->strArtist);
    GetPropertyString(pszFilePath, PKEY_Music_AlbumTitle, &pMetadata->strAlbum);
    GetPropertyString(pszFilePath, PKEY_Music_Genre, &pMetadata->strGenre);
    GetPropertyString(pszFilePath, PKEY_Comment, &pMetadata->strDescription);
    GetPropertyString(pszFilePath, PKEY_Copyright, &pMetadata->strCopyright);

    // Get duration via MF
    GetDuration(pszFilePath, &pMetadata->llDurationHns);

    // Get resolution
    DWORD dwWidth = 0, dwHeight = 0;
    if (SUCCEEDED(GetResolution(pszFilePath, &dwWidth, &dwHeight)))
    {
        pMetadata->dwWidth = dwWidth;
        pMetadata->dwHeight = dwHeight;
    }

    return S_OK;
}

HRESULT TranscodeMetadataParser::ParseFromMFAttributes(IMFAttributes* pAttributes, MediaMetadata* pMetadata)
{
    if (!pAttributes || !pMetadata)
        return E_POINTER;

    // Read duration
    UINT64 ullDuration = 0;
    if (SUCCEEDED(pAttributes->GetUINT64(MF_PD_DURATION, &ullDuration)))
        pMetadata->llDurationHns = static_cast<LONGLONG>(ullDuration);

    // Read frame rate
    UINT32 uNum = 0, uDen = 0;
    if (SUCCEEDED(MFGetAttributeRatio(pAttributes, MF_MT_FRAME_RATE, &uNum, &uDen)) && uDen > 0)
        pMetadata->dblFrameRate = static_cast<double>(uNum) / static_cast<double>(uDen);

    // Read resolution
    UINT32 uWidth = 0, uHeight = 0;
    if (SUCCEEDED(MFGetAttributeSize(pAttributes, MF_MT_FRAME_SIZE, &uWidth, &uHeight)))
    {
        pMetadata->dwWidth = uWidth;
        pMetadata->dwHeight = uHeight;
    }

    return S_OK;
}

// ============================================================================
// TranscodeProcess
// ============================================================================

TranscodeProcess::TranscodeProcess()
    : m_state(TranscodeProcessIdle)
    , m_fCancellationRequested(false)
    , m_llStartTimeHns(0)
    , m_llPauseTimeHns(0)
    , m_llTotalPauseDurationHns(0)
{
}

TranscodeProcess::~TranscodeProcess()
{
    Shutdown();
}

HRESULT TranscodeProcess::Initialize(LPCWSTR pszInputPath, LPCWSTR pszOutputPath)
{
    if (!pszInputPath || !pszOutputPath)
        return E_POINTER;

    m_strInputPath = pszInputPath;
    m_strOutputPath = pszOutputPath;
    m_state = TranscodeProcessIdle;
    m_progress = TranscodeProcessProgress();
    m_fCancellationRequested = false;

    return S_OK;
}

HRESULT TranscodeProcess::Shutdown()
{
    m_state = TranscodeProcessIdle;
    m_progress = TranscodeProcessProgress();
    m_fCancellationRequested = false;
    return S_OK;
}

// ============================================================================
// State management
// ============================================================================

HRESULT TranscodeProcess::BeginTranscode()
{
    if (m_state != TranscodeProcessIdle)
        return E_UNEXPECTED;

    StartTimer();
    SetState(TranscodeProcessPreparing);
    SetState(TranscodeProcessEncoding);

    return S_OK;
}

HRESULT TranscodeProcess::CompleteTranscode()
{
    if (m_state != TranscodeProcessEncoding && m_state != TranscodeProcessFinalizing)
        return E_UNEXPECTED;

    m_progress.fComplete = true;
    m_progress.fPercentComplete = 100.0f;
    SetState(TranscodeProcessComplete);

    FireProgress();
    return S_OK;
}

HRESULT TranscodeProcess::FailTranscode(HRESULT hrError)
{
    m_progress.hrLastError = hrError;
    SetState(TranscodeProcessFailed);
    return S_OK;
}

HRESULT TranscodeProcess::CancelTranscode()
{
    m_fCancellationRequested = true;
    SetState(TranscodeProcessCancelled);
    return S_OK;
}

HRESULT TranscodeProcess::PauseTranscode()
{
    if (m_state != TranscodeProcessEncoding)
        return E_UNEXPECTED;

    m_llPauseTimeHns = GetCurrentTimeHns();
    SetState(TranscodeProcessPaused);
    return S_OK;
}

HRESULT TranscodeProcess::ResumeTranscode()
{
    if (m_state != TranscodeProcessPaused)
        return E_UNEXPECTED;

    if (m_llPauseTimeHns > 0)
    {
        m_llTotalPauseDurationHns += GetCurrentTimeHns() - m_llPauseTimeHns;
        m_llPauseTimeHns = 0;
    }

    SetState(TranscodeProcessEncoding);
    return S_OK;
}

// ============================================================================
// State query
// ============================================================================

TranscodeProcessState TranscodeProcess::GetState() const throw()
{
    return m_state;
}

bool TranscodeProcess::IsComplete() const throw()
{
    return m_state == TranscodeProcessComplete;
}

bool TranscodeProcess::IsFailed() const throw()
{
    return m_state == TranscodeProcessFailed;
}

bool TranscodeProcess::IsCancelled() const throw()
{
    return m_state == TranscodeProcessCancelled;
}

bool TranscodeProcess::IsPaused() const throw()
{
    return m_state == TranscodeProcessPaused;
}

bool TranscodeProcess::IsEncoding() const throw()
{
    return m_state == TranscodeProcessEncoding;
}

// ============================================================================
// Progress
// ============================================================================

TranscodeProcessProgress TranscodeProcess::GetProgress() const
{
    return m_progress;
}

HRESULT TranscodeProcess::UpdateProgress(LONGLONG llCurrentPositionHns)
{
    m_progress.llCurrentPositionHns = llCurrentPositionHns;

    if (m_progress.llTotalDurationHns > 0)
    {
        double dblRatio = static_cast<double>(llCurrentPositionHns) /
                          static_cast<double>(m_progress.llTotalDurationHns);
        if (dblRatio < 0.0) dblRatio = 0.0;
        if (dblRatio > 1.0) dblRatio = 1.0;
        m_progress.fPercentComplete = static_cast<float>(dblRatio * 100.0);
    }

    m_progress.dblElapsedSeconds = GetElapsedSeconds();
    m_progress.dblEstimatedRemaining = GetEstimatedRemaining();

    FireProgress();
    return S_OK;
}

HRESULT TranscodeProcess::SetTotalDuration(LONGLONG llTotalDurationHns)
{
    m_progress.llTotalDurationHns = llTotalDurationHns;
    return S_OK;
}

HRESULT TranscodeProcess::SetTotalFrameCount(DWORD dwTotalFrames)
{
    m_progress.dwTotalFrames = dwTotalFrames;
    return S_OK;
}

HRESULT TranscodeProcess::SetOutputSize(LONGLONG llSizeBytes)
{
    m_progress.llOutputSizeBytes = llSizeBytes;
    return S_OK;
}

HRESULT TranscodeProcess::IncrementFramesEncoded()
{
    m_progress.dwFramesEncoded++;
    return S_OK;
}

HRESULT TranscodeProcess::SetEncodingFps(double dblFps)
{
    m_progress.dblEncodingFps = dblFps;
    return S_OK;
}

// ============================================================================
// Timing
// ============================================================================

HRESULT TranscodeProcess::StartTimer()
{
    m_llStartTimeHns = GetCurrentTimeHns();
    m_llPauseTimeHns = 0;
    m_llTotalPauseDurationHns = 0;
    return S_OK;
}

double TranscodeProcess::GetElapsedSeconds() const throw()
{
    if (m_llStartTimeHns == 0)
        return 0.0;

    LONGLONG llElapsed = GetCurrentTimeHns() - m_llStartTimeHns - m_llTotalPauseDurationHns;
    return static_cast<double>(llElapsed) / 10000000.0;
}

double TranscodeProcess::GetEstimatedRemaining() const throw()
{
    if (m_progress.fPercentComplete <= 0.0f || m_progress.fPercentComplete >= 100.0f)
        return 0.0;

    double dblElapsed = GetElapsedSeconds();
    if (dblElapsed <= 0.0)
        return 0.0;

    double dblTotalEstimated = dblElapsed / (static_cast<double>(m_progress.fPercentComplete) / 100.0);
    return dblTotalEstimated - dblElapsed;
}

// ============================================================================
// Paths
// ============================================================================

ATL::CString TranscodeProcess::GetInputPath() const
{
    return m_strInputPath;
}

ATL::CString TranscodeProcess::GetOutputPath() const
{
    return m_strOutputPath;
}

// ============================================================================
// Cancel request check
// ============================================================================

bool TranscodeProcess::IsCancellationRequested() const throw()
{
    return m_fCancellationRequested;
}

// ============================================================================
// Private helpers
// ============================================================================

void TranscodeProcess::SetState(TranscodeProcessState newState)
{
    TranscodeProcessState oldState = m_state;
    m_state = newState;
    FireStateChange(oldState, newState);
}

void TranscodeProcess::FireProgress()
{
    if (m_progressCb)
        m_progressCb(m_progress);
}

void TranscodeProcess::FireStateChange(TranscodeProcessState oldState, TranscodeProcessState newState)
{
    if (m_stateChangeCb)
        m_stateChangeCb(oldState, newState);
}

LONGLONG TranscodeProcess::GetCurrentTimeHns() const
{
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    return (static_cast<LONGLONG>(ft.dwHighDateTime) << 32) | ft.dwLowDateTime;
}

} // namespace HMRAVSource
