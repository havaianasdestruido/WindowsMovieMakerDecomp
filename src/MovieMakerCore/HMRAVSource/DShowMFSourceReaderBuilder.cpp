// DShowMFSourceReaderBuilder.cpp - DirectShow/MF bridge implementation

#include "pch.h"
#include "DShowMFSourceReaderBuilder.h"

namespace HMRAVSource
{

// ============================================================================
// Construction / Destruction
// ============================================================================

DShowMFSourceReaderBuilder::DShowMFSourceReaderBuilder()
    : m_fInitialized(false)
    , m_hVideoWnd(nullptr)
    , m_fDeinterlace(true)
{
    m_guidPreferredSubtype = MFVideoFormat_NV12;
}

DShowMFSourceReaderBuilder::~DShowMFSourceReaderBuilder()
{
    Shutdown();
}

// ============================================================================
// Lifecycle
// ============================================================================

HRESULT DShowMFSourceReaderBuilder::Initialize()
{
    m_fInitialized = true;
    return S_OK;
}

HRESULT DShowMFSourceReaderBuilder::Shutdown()
{
    if (m_spControl)
    {
        OAFilterState state = State_Stopped;
        m_spControl->Stop();
    }

    m_spGraph = nullptr;
    m_spControl = nullptr;
    m_spEvent = nullptr;
    m_spSeeking = nullptr;
    m_fInitialized = false;

    return S_OK;
}

// ============================================================================
// Source reader creation
// ============================================================================

HRESULT DShowMFSourceReaderBuilder::CreateSourceReader(LPCWSTR pszFilePath, IMFSourceReader** ppReader)
{
    if (!pszFilePath || !ppReader)
        return E_POINTER;

    *ppReader = nullptr;

    if (!m_fInitialized)
        return E_UNEXPECTED;

    HRESULT hr = MFCreateSourceReaderFromURL(pszFilePath, nullptr, ppReader);
    if (SUCCEEDED(hr))
        return hr;

    hr = CreateFilterGraph(pszFilePath);
    if (FAILED(hr))
        return hr;

    if (!m_spGraph)
        return E_FAIL;

    return MFCreateSourceReaderFromURL(pszFilePath, nullptr, ppReader);
}

HRESULT DShowMFSourceReaderBuilder::CreateSourceReaderWithCallback(
    LPCWSTR pszFilePath,
    IMFSourceReaderCallback* pCallback,
    IMFSourceReader** ppReader)
{
    UNREFERENCED_PARAMETER(pCallback);
    return CreateSourceReader(pszFilePath, ppReader);
}

// ============================================================================
// Format support
// ============================================================================

bool DShowMFSourceReaderBuilder::IsDShowRequired(LPCWSTR pszFilePath)
{
    return IsDShowOnlyFormat(pszFilePath);
}

bool DShowMFSourceReaderBuilder::IsDShowOnlyFormat(LPCWSTR pszFilePath)
{
    if (!pszFilePath)
        return false;

    LPCWSTR pszExtension = PathFindExtensionW(pszFilePath);
    return IsLegacyExtension(pszExtension);
}

// ============================================================================
// DShow-specific configuration
// ============================================================================

HRESULT DShowMFSourceReaderBuilder::SetVideoWindow(HWND hWnd)
{
    m_hVideoWnd = hWnd;
    return S_OK;
}

HRESULT DShowMFSourceReaderBuilder::EnableDeinterlace(bool fEnable)
{
    m_fDeinterlace = fEnable;
    return S_OK;
}

HRESULT DShowMFSourceReaderBuilder::SetPreferredVideoFormat(const GUID* pSubtype)
{
    if (pSubtype)
        m_guidPreferredSubtype = *pSubtype;
    return S_OK;
}

// ============================================================================
// Source info
// ============================================================================

HRESULT DShowMFSourceReaderBuilder::GetVideoDimensions(LPCWSTR pszFilePath, DWORD* pdwWidth, DWORD* pdwHeight)
{
    if (!pszFilePath || !pdwWidth || !pdwHeight)
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

HRESULT DShowMFSourceReaderBuilder::GetFrameRate(LPCWSTR pszFilePath, double* pdblFrameRate)
{
    if (!pszFilePath || !pdblFrameRate)
        return E_POINTER;

    *pdblFrameRate = 0.0;

    CComPtr<IMFSourceReader> spReader;
    HRESULT hr = MFCreateSourceReaderFromURL(pszFilePath, nullptr, &spReader);
    if (FAILED(hr))
        return hr;

    CComPtr<IMFMediaType> spType;
    hr = spReader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, &spType);
    if (FAILED(hr))
        return hr;

    UINT32 uNumerator = 0, uDenominator = 1;
    hr = MFGetAttributeRatio(spType, MF_MT_FRAME_RATE, &uNumerator, &uDenominator);
    if (SUCCEEDED(hr) && uDenominator > 0)
        *pdblFrameRate = static_cast<double>(uNumerator) / static_cast<double>(uDenominator);

    return hr;
}

HRESULT DShowMFSourceReaderBuilder::GetDuration(LPCWSTR pszFilePath, LONGLONG* pllDurationHns)
{
    if (!pszFilePath || !pllDurationHns)
        return E_POINTER;

    *pllDurationHns = 0;

    HRESULT hr = CreateFilterGraph(pszFilePath);
    if (FAILED(hr))
        return hr;

    if (m_spSeeking)
    {
        LONGLONG llDuration = 0;
        hr = m_spSeeking->GetDuration(&llDuration);
        if (SUCCEEDED(hr))
            *pllDurationHns = llDuration;
    }

    return hr;
}

// ============================================================================
// Filter graph access
// ============================================================================

HRESULT DShowMFSourceReaderBuilder::GetFilterGraph(IGraphBuilder** ppGraph)
{
    if (!ppGraph)
        return E_POINTER;

    *ppGraph = m_spGraph;
    if (*ppGraph)
        (*ppGraph)->AddRef();
    return S_OK;
}

HRESULT DShowMFSourceReaderBuilder::GetMediaControl(IMediaControl** ppControl)
{
    if (!ppControl)
        return E_POINTER;

    *ppControl = m_spControl;
    if (*ppControl)
        (*ppControl)->AddRef();
    return S_OK;
}

HRESULT DShowMFSourceReaderBuilder::GetMediaEvent(IMediaEventEx** ppEvent)
{
    if (!ppEvent)
        return E_POINTER;

    *ppEvent = m_spEvent;
    if (*ppEvent)
        (*ppEvent)->AddRef();
    return S_OK;
}

// ============================================================================
// Private helpers
// ============================================================================

HRESULT DShowMFSourceReaderBuilder::CreateFilterGraph(LPCWSTR pszFilePath)
{
    if (!pszFilePath)
        return E_POINTER;

    // Release existing graph
    m_spGraph = nullptr;
    m_spControl = nullptr;
    m_spEvent = nullptr;
    m_spSeeking = nullptr;

    HRESULT hr = CoCreateInstance(
        CLSID_FilterGraph,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&m_spGraph));

    if (FAILED(hr))
        return hr;

    hr = m_spGraph->QueryInterface(IID_PPV_ARGS(&m_spControl));
    if (FAILED(hr))
        return hr;

    hr = m_spGraph->QueryInterface(IID_PPV_ARGS(&m_spEvent));
    if (FAILED(hr))
        return hr;

    hr = m_spGraph->QueryInterface(IID_PPV_ARGS(&m_spSeeking));
    if (FAILED(hr))
        return hr;

    // Render the file
    hr = m_spGraph->RenderFile(pszFilePath, nullptr);
    return hr;
}

HRESULT DShowMFSourceReaderBuilder::AddVideoRenderer(IGraphBuilder* pGraph)
{
    if (!pGraph)
        return E_POINTER;

    CComPtr<IBaseFilter> spRenderer;
    HRESULT hr = CoCreateInstance(
        CLSID_VideoMixingRenderer9,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&spRenderer));

    if (SUCCEEDED(hr))
        hr = pGraph->AddFilter(spRenderer, L"Video Renderer");

    return hr;
}

HRESULT DShowMFSourceReaderBuilder::AddSampleGrabber(IGraphBuilder* pGraph)
{
    if (!pGraph)
        return E_POINTER;

    CComPtr<IBaseFilter> spGrabber;
    HRESULT hr = CoCreateInstance(
        CLSID_SampleGrabber,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&spGrabber));

    if (SUCCEEDED(hr))
        hr = pGraph->AddFilter(spGrabber, L"Sample Grabber");

    return hr;
}

HRESULT DShowMFSourceReaderBuilder::ConnectFilters()
{
    if (!m_spGraph)
        return E_UNEXPECTED;

    CComPtr<IEnumFilters> spEnum;
    HRESULT hr = m_spGraph->EnumFilters(&spEnum);
    if (FAILED(hr))
        return hr;

    CAtlArray<CComPtr<IBaseFilter>> filters;
    CComPtr<IBaseFilter> spFilter;
    while (spEnum->Next(1, &spFilter, nullptr) == S_OK)
    {
        filters.Add(spFilter);
        spFilter.Release();
    }

    for (SIZE_T i = 0; i + 1 < filters.GetCount(); ++i)
    {
        CComPtr<IEnumPins> spEnumPins;
        hr = filters[i]->EnumPins(&spEnumPins);
        if (FAILED(hr))
            continue;

        CComPtr<IPin> spPin;
        if (spEnumPins->Next(1, &spPin, nullptr) != S_OK)
            continue;

        PIN_INFO pinInfo = {};
        spPin->QueryPinInfo(&pinInfo);
        if (pinInfo.dir != PINDIR_OUTPUT)
        {
            if (pinInfo.pFilter) pinInfo.pFilter->Release();
            spPin.Release();
            continue;
        }
        if (pinInfo.pFilter) pinInfo.pFilter->Release();

        CComPtr<IEnumPins> spEnumPinsDown;
        hr = filters[i + 1]->EnumPins(&spEnumPinsDown);
        if (FAILED(hr))
            continue;

        CComPtr<IPin> spDownPin;
        while (spEnumPinsDown->Next(1, &spDownPin, nullptr) == S_OK)
        {
            PIN_INFO downPinInfo = {};
            spDownPin->QueryPinInfo(&downPinInfo);
            bool fIsInput = (downPinInfo.dir == PINDIR_INPUT);
            if (downPinInfo.pFilter) downPinInfo.pFilter->Release();

            if (fIsInput)
            {
                hr = m_spGraph->Connect(spPin, spDownPin);
                if (SUCCEEDED(hr))
                    break;
            }
            spDownPin.Release();
        }
    }

    return S_OK;
}

bool DShowMFSourceReaderBuilder::IsLegacyExtension(LPCWSTR pszExtension)
{
    if (!pszExtension)
        return false;

    if (_wcsicmp(pszExtension, L".mpg") == 0 ||
        _wcsicmp(pszExtension, L".mpeg") == 0 ||
        _wcsicmp(pszExtension, L".mpe") == 0 ||
        _wcsicmp(pszExtension, L".vob") == 0 ||
        _wcsicmp(pszExtension, L".ogv") == 0 ||
        _wcsicmp(pszExtension, L".ogm") == 0 ||
        _wcsicmp(pszExtension, L".mkv") == 0 ||
        _wcsicmp(pszExtension, L".webm") == 0 ||
        _wcsicmp(pszExtension, L".flv") == 0 ||
        _wcsicmp(pszExtension, L".ts") == 0 ||
        _wcsicmp(pszExtension, L".m2ts") == 0 ||
        _wcsicmp(pszExtension, L".divx") == 0 ||
        _wcsicmp(pszExtension, L".xvid") == 0 ||
        _wcsicmp(pszExtension, L".flac") == 0 ||
        _wcsicmp(pszExtension, L".ogg") == 0)
    {
        return true;
    }

    return false;
}

} // namespace HMRAVSource
