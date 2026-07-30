#include "pch.h"
#include "ThumbnailCache.h"

extern IWICImagingFactory* MovieCore_GetWICFactory(void);

ThumbnailCache::ThumbnailCache()
{
    InitializeCriticalSection(&m_cs);
}

ThumbnailCache::~ThumbnailCache()
{
    Clear();
    DeleteCriticalSection(&m_cs);
}

HBITMAP ThumbnailCache::GetThumbnail(LPCWSTR pszPath)
{
    if (!pszPath || !pszPath[0])
        return NULL;

    std::wstring key(pszPath);

    EnterCriticalSection(&m_cs);

    std::map<std::wstring, CacheEntry>::iterator it = m_cache.find(key);
    if (it != m_cache.end())
    {
        if (!IsFileStale(pszPath, it->second.lastWriteTime))
        {
            m_lruOrder.erase(it->second.lruIter);
            m_lruOrder.push_front(key);
            it->second.lruIter = m_lruOrder.begin();
            HBITMAP hBitmap = it->second.hBitmap;
            LeaveCriticalSection(&m_cs);
            return hBitmap;
        }
        DeleteObject(it->second.hBitmap);
        m_lruOrder.erase(it->second.lruIter);
        m_cache.erase(it);
    }

    LeaveCriticalSection(&m_cs);

    HBITMAP hBitmap = GenerateThumbnail(pszPath);
    if (!hBitmap)
        return NULL;

    FILETIME ftWrite = {};
    WIN32_FILE_ATTRIBUTE_DATA fad;
    if (GetFileAttributesExW(pszPath, GetFileExInfoStandard, &fad))
        ftWrite = fad.ftLastWriteTime;

    EnterCriticalSection(&m_cs);

    it = m_cache.find(key);
    if (it != m_cache.end())
    {
        DeleteObject(hBitmap);
        HBITMAP cached = it->second.hBitmap;
        m_lruOrder.erase(it->second.lruIter);
        m_lruOrder.push_front(key);
        it->second.lruIter = m_lruOrder.begin();
        LeaveCriticalSection(&m_cs);
        return cached;
    }

    CacheEntry entry;
    entry.hBitmap = hBitmap;
    entry.lastWriteTime = ftWrite;
    m_lruOrder.push_front(key);
    entry.lruIter = m_lruOrder.begin();
    m_cache[key] = entry;
    EvictOldest();

    LeaveCriticalSection(&m_cs);

    return hBitmap;
}

HRESULT ThumbnailCache::AddThumbnail(LPCWSTR pszPath, HBITMAP hBitmap)
{
    if (!pszPath || !pszPath[0])
        return E_INVALIDARG;

    std::wstring key(pszPath);

    FILETIME ftWrite = {};
    WIN32_FILE_ATTRIBUTE_DATA fad;
    if (GetFileAttributesExW(pszPath, GetFileExInfoStandard, &fad))
        ftWrite = fad.ftLastWriteTime;

    EnterCriticalSection(&m_cs);

    std::map<std::wstring, CacheEntry>::iterator it = m_cache.find(key);
    if (it != m_cache.end())
    {
        if (it->second.hBitmap)
            DeleteObject(it->second.hBitmap);
        it->second.hBitmap = hBitmap;
        it->second.lastWriteTime = ftWrite;
        m_lruOrder.erase(it->second.lruIter);
        m_lruOrder.push_front(key);
        it->second.lruIter = m_lruOrder.begin();
    }
    else
    {
        CacheEntry entry;
        entry.hBitmap = hBitmap;
        entry.lastWriteTime = ftWrite;
        m_lruOrder.push_front(key);
        entry.lruIter = m_lruOrder.begin();
        m_cache[key] = entry;
        EvictOldest();
    }

    LeaveCriticalSection(&m_cs);

    return S_OK;
}

HRESULT ThumbnailCache::RemoveThumbnail(LPCWSTR pszPath)
{
    if (!pszPath || !pszPath[0])
        return E_INVALIDARG;

    std::wstring key(pszPath);

    EnterCriticalSection(&m_cs);

    std::map<std::wstring, CacheEntry>::iterator it = m_cache.find(key);
    if (it == m_cache.end())
    {
        LeaveCriticalSection(&m_cs);
        return S_FALSE;
    }

    if (it->second.hBitmap)
        DeleteObject(it->second.hBitmap);
    m_lruOrder.erase(it->second.lruIter);
    m_cache.erase(it);

    LeaveCriticalSection(&m_cs);

    return S_OK;
}

void ThumbnailCache::Clear()
{
    EnterCriticalSection(&m_cs);

    for (std::map<std::wstring, CacheEntry>::iterator it = m_cache.begin();
         it != m_cache.end(); ++it)
    {
        if (it->second.hBitmap)
            DeleteObject(it->second.hBitmap);
    }
    m_cache.clear();
    m_lruOrder.clear();

    LeaveCriticalSection(&m_cs);
}

void ThumbnailCache::InvalidateAll()
{
    Clear();
}

size_t ThumbnailCache::GetCount() const throw()
{
    EnterCriticalSection(&m_cs);
    size_t count = m_cache.size();
    LeaveCriticalSection(&m_cs);
    return count;
}

void ThumbnailCache::EvictOldest()
{
    while (m_cache.size() > MAX_CACHE_SIZE && !m_lruOrder.empty())
    {
        std::wstring oldest = m_lruOrder.back();
        m_lruOrder.pop_back();
        std::map<std::wstring, CacheEntry>::iterator it = m_cache.find(oldest);
        if (it != m_cache.end())
        {
            if (it->second.hBitmap)
                DeleteObject(it->second.hBitmap);
            m_cache.erase(it);
        }
    }
}

bool ThumbnailCache::IsFileStale(LPCWSTR pszPath, const FILETIME& cachedTime) const
{
    if (cachedTime.dwLowDateTime == 0 && cachedTime.dwHighDateTime == 0)
        return false;

    WIN32_FILE_ATTRIBUTE_DATA fad;
    if (!GetFileAttributesExW(pszPath, GetFileExInfoStandard, &fad))
        return true;

    return CompareFileTime(&fad.ftLastWriteTime, &cachedTime) > 0;
}

HBITMAP ThumbnailCache::GenerateThumbnail(LPCWSTR pszPath)
{
    if (!pszPath || !pszPath[0])
        return NULL;

    HRESULT hr = MFStartup(MF_VERSION);
    if (FAILED(hr))
        return NULL;

    CComPtr<IMFSourceReader> spReader;
    hr = MFCreateSourceReaderFromURL(pszPath, nullptr, &spReader);
    if (FAILED(hr))
    {
        MFShutdown();
        return NULL;
    }

    UINT uWidth = 0;
    UINT uHeight = 0;
    bool fHasVideo = false;

    CComPtr<IMFMediaType> spNativeType;
    hr = spReader->GetNativeMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, &spNativeType);
    if (SUCCEEDED(hr))
    {
        MFGetAttributeSize(spNativeType, MF_MT_FRAME_SIZE, &uWidth, &uHeight);
        fHasVideo = (uWidth > 0 && uHeight > 0);
    }

    if (!fHasVideo)
    {
        MFShutdown();
        return NULL;
    }

    UINT64 ullDuration = 0;
    PROPVARIANT varAttr;
    PropVariantInit(&varAttr);
    hr = spReader->GetPresentationAttribute(
        MF_SOURCE_READER_MEDIASOURCE,
        MF_PD_DURATION,
        &varAttr);
    if (SUCCEEDED(hr))
    {
        ullDuration = varAttr.uhVal.QuadPart;
        PropVariantClear(&varAttr);
    }

    LONGLONG llThumbPosition = static_cast<LONGLONG>(ullDuration / 10);
    if (llThumbPosition < 0)
        llThumbPosition = 0;

    PROPVARIANT varPosition;
    PropVariantInit(&varPosition);
    varPosition.vt = VT_I8;
    varPosition.hVal.QuadPart = llThumbPosition;
    hr = spReader->SetCurrentPosition(GUID_NULL, varPosition);
    PropVariantClear(&varPosition);

    if (FAILED(hr))
    {
        MFShutdown();
        return NULL;
    }

    DWORD dwStreamIndex = 0;
    DWORD dwStreamFlags = 0;
    LONGLONG llTimestamp = 0;
    CComPtr<IMFSample> spVideoSample;

    hr = spReader->ReadSample(
        MF_SOURCE_READER_FIRST_VIDEO_STREAM,
        0,
        &dwStreamIndex,
        &dwStreamFlags,
        &llTimestamp,
        &spVideoSample);

    if (FAILED(hr) || !spVideoSample)
    {
        MFShutdown();
        return NULL;
    }

    CComPtr<IMFMediaBuffer> spBuffer;
    hr = spVideoSample->ConvertToContiguousBuffer(&spBuffer);
    if (FAILED(hr))
    {
        MFShutdown();
        return NULL;
    }

    BYTE* pbData = NULL;
    DWORD cbData = 0;
    hr = spBuffer->Lock(&pbData, NULL, &cbData);
    if (FAILED(hr))
    {
        MFShutdown();
        return NULL;
    }

    HBITMAP hBitmap = CreateHBitmapFromFrame(pbData, uWidth, uHeight);

    spBuffer->Unlock();

    MFShutdown();

    return hBitmap;
}

HBITMAP ThumbnailCache::CreateHBitmapFromFrame(BYTE* pbFrameData, UINT uWidth, UINT uHeight)
{
    if (!pbFrameData || uWidth == 0 || uHeight == 0)
        return NULL;

    IWICImagingFactory* pWicFactory = MovieCore_GetWICFactory();
    if (!pWicFactory)
    {
        HRESULT hr = CoCreateInstance(
            CLSID_WICImagingFactory,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(&pWicFactory));
        if (FAILED(hr) || !pWicFactory)
            return NULL;
    }
    else
    {
        pWicFactory->AddRef();
    }

    CComPtr<IWICBitmap> spWicBitmap;
    HRESULT hr = pWicFactory->CreateBitmapFromMemory(
        uWidth,
        uHeight,
        GUID_WICPixelFormat32bppBGRA,
        uWidth * 4,
        uWidth * uHeight * 4,
        pbFrameData,
        &spWicBitmap);

    if (FAILED(hr))
    {
        pWicFactory->Release();
        return NULL;
    }

    CComPtr<IWICFormatConverter> spConverter;
    hr = pWicFactory->CreateFormatConverter(&spConverter);
    if (FAILED(hr))
    {
        pWicFactory->Release();
        return NULL;
    }

    hr = spConverter->Initialize(
        spWicBitmap,
        GUID_WICPixelFormat32bppBGRA,
        WICBitmapDitherTypeNone,
        NULL,
        0.0,
        WICBitmapPaletteTypeCustom);

    if (FAILED(hr))
    {
        pWicFactory->Release();
        return NULL;
    }

    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = static_cast<LONG>(uWidth);
    bmi.bmiHeader.biHeight = -static_cast<LONG>(uHeight);
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    BYTE* pPixels = NULL;
    HBITMAP hBitmap = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, (void**)&pPixels, NULL, 0);
    if (!hBitmap || !pPixels)
    {
        pWicFactory->Release();
        return NULL;
    }

    hr = spConverter->CopyPixels(
        NULL,
        uWidth * 4,
        uWidth * uHeight * 4,
        pPixels);

    pWicFactory->Release();

    if (FAILED(hr))
    {
        DeleteObject(hBitmap);
        return NULL;
    }

    return hBitmap;
}
