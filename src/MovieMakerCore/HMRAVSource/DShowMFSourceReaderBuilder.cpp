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

    HRESULT hr = CreateFilterGraph(pszFilePath);
    if (FAILED(hr))
        return hr;

    // In a full implementation, this would create a DShow-to-MF bridge
    // using IMFSourceReader with a custom media source that wraps the
    // DShow filter graph. For now, return E_NOTIMPL to indicate that
    // DShow fallback would need the DShowSource path instead.
    return E_NOTIMPL;
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

    // Would query via IBasicVideo from the DShow graph
    return E_NOTIMPL;
}

HRESULT DShowMFSourceReaderBuilder::GetFrameRate(LPCWSTR pszFilePath, double* pdblFrameRate)
{
    if (!pszFilePath || !pdblFrameRate)
        return E_POINTER;

    *pdblFrameRate = 0.0;
    return E_NOTIMPL;
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
