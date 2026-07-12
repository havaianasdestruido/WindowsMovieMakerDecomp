// DShowSource.cpp - DirectShow source wrapper implementation

#include "pch.h"
#include "DShowSource.h"

#pragma comment(lib, "strmiids.lib")
#pragma comment(lib, "quartz.lib")

namespace HMRAVSource
{

// ============================================================================
// Construction / Destruction
// ============================================================================

DShowSource::DShowSource()
    : m_hVideoWnd(nullptr)
    , m_fGraphBuilt(false)
{
    m_type = AVSourceTypeFile;
}

DShowSource::~DShowSource()
{
    Close();
}

// ============================================================================
// Open / Close
// ============================================================================

HRESULT DShowSource::Open(const AVSourceDesc& desc)
{
    if (IsOpen())
        Close();

    HRESULT hr = AVSource::Open(desc);
    if (FAILED(hr))
        return hr;

    hr = BuildFilterGraph(desc.strFilePath.GetString());
    if (FAILED(hr))
    {
        SetState(AVSourceStateError);
        m_hrLastResult = hr;
        return hr;
    }

    SetState(AVSourceStateOpen);
    m_hrLastResult = S_OK;
    return S_OK;
}

HRESULT DShowSource::Close()
{
    if (m_spControl)
    {
        m_spControl->Stop();
        m_spControl = nullptr;
    }

    m_spGraph = nullptr;
    m_spSeeking = nullptr;
    m_spEvent = nullptr;
    m_spVideoWindow = nullptr;
    m_spBasicVideo = nullptr;
    m_spBasicAudio = nullptr;
    m_spSampleGrabber = nullptr;
    m_spGrabberFilter = nullptr;

    m_fGraphBuilt = false;

    return AVSource::Close();
}

// ============================================================================
// Position
// ============================================================================

HRESULT DShowSource::SetPositionHns(LONGLONG llPosition)
{
    HRESULT hr = AVSource::SetPositionHns(llPosition);
    if (FAILED(hr))
        return hr;

    if (!m_spSeeking)
        return E_UNEXPECTED;

    // Convert hundred-nanoseconds to DirectShow reference time (100ns units)
    LONGLONG llSeekPos = llPosition;
    hr = m_spSeeking->SetPositions(
        &llSeekPos,
        AM_SEEKING_AbsolutePositioning,
        nullptr,
        AM_SEEKING_NoPositioning);

    return hr;
}

// ============================================================================
// Start / Stop / Pause
// ============================================================================

HRESULT DShowSource::Start()
{
    if (!m_spControl)
        return E_UNEXPECTED;

    HRESULT hr = m_spControl->Run();
    if (SUCCEEDED(hr))
    {
        SetState(AVSourceStateStarted);
    }
    m_hrLastResult = hr;
    return hr;
}

HRESULT DShowSource::Stop()
{
    if (!m_spControl)
        return E_UNEXPECTED;

    HRESULT hr = m_spControl->Stop();
    if (SUCCEEDED(hr))
    {
        SetState(AVSourceStateStopped);
    }
    m_hrLastResult = hr;
    return hr;
}

HRESULT DShowSource::Pause()
{
    if (!m_spControl)
        return E_UNEXPECTED;

    HRESULT hr = m_spControl->Pause();
    if (SUCCEEDED(hr))
    {
        SetState(AVSourceStatePaused);
    }
    m_hrLastResult = hr;
    return hr;
}

HRESULT DShowSource::Resume()
{
    if (!m_spControl)
        return E_UNEXPECTED;

    HRESULT hr = m_spControl->Run();
    if (SUCCEEDED(hr))
    {
        SetState(AVSourceStateStarted);
    }
    m_hrLastResult = hr;
    return hr;
}

// ============================================================================
// Sample reading
// ============================================================================

HRESULT DShowSource::ReadSample(IMFSample** ppSample, DWORD dwStreamIndex)
{
    if (!ppSample)
        return E_POINTER;
    *ppSample = nullptr;

    if (!m_spSampleGrabber || !m_fGraphBuilt)
        return E_UNEXPECTED;

    // Use ISampleGrabber to capture current frame
    long cbBuffer = 0;
    HRESULT hr = m_spSampleGrabber->GetCurrentBuffer(&cbBuffer, nullptr);
    if (FAILED(hr) || cbBuffer == 0)
        return MF_E_END_OFSTREAM;

    ATL::CAtlArray<BYTE> arrBuffer;
    arrBuffer.SetCount(cbBuffer);
    hr = m_spSampleGrabber->GetCurrentBuffer(&cbBuffer, arrBuffer.GetData());
    if (FAILED(hr))
        return hr;

    // Wrap in MF sample
    CComPtr<IMFSample> spSample;
    hr = MFCreateSample(&spSample);
    if (FAILED(hr))
        return hr;

    CComPtr<IMFMediaBuffer> spBuffer;
    hr = MFCreateMemoryBuffer(cbBuffer, &spBuffer);
    if (FAILED(hr))
        return hr;

    BYTE* pBuffer = nullptr;
    hr = spBuffer->Lock(&pBuffer, nullptr, nullptr);
    if (SUCCEEDED(hr))
    {
        CopyMemory(pBuffer, arrBuffer.GetData(), cbBuffer);
        spBuffer->Unlock();
    }

    spSample->AddBuffer(spBuffer);
    *ppSample = spSample.Detach();

    return S_OK;
}

HRESULT DShowSource::Flush()
{
    if (m_spSampleGrabber)
        m_spSampleGrabber->SetOneShot(FALSE);

    return AVSource::Flush();
}

// ============================================================================
// DShow-specific queries
// ============================================================================

HRESULT DShowSource::GetGraphBuilder(IGraphBuilder** ppGraph)
{
    if (!ppGraph)
        return E_POINTER;
    if (!m_spGraph)
        return E_UNEXPECTED;

    *ppGraph = m_spGraph;
    (*ppGraph)->AddRef();
    return S_OK;
}

HRESULT DShowSource::GetMediaControl(IMediaControl** ppControl)
{
    if (!ppControl)
        return E_POINTER;
    if (!m_spControl)
        return E_UNEXPECTED;

    *ppControl = m_spControl;
    (*ppControl)->AddRef();
    return S_OK;
}

HRESULT DShowSource::GetMediaSeeking(IMediaSeeking** ppSeeking)
{
    if (!ppSeeking)
        return E_POINTER;
    if (!m_spSeeking)
        return E_UNEXPECTED;

    *ppSeeking = m_spSeeking;
    (*ppSeeking)->AddRef();
    return S_OK;
}

HRESULT DShowSource::GetMediaEvent(IMediaEventEx** ppEvent)
{
    if (!ppEvent)
        return E_POINTER;
    if (!m_spEvent)
        return E_UNEXPECTED;

    *ppEvent = m_spEvent;
    (*ppEvent)->AddRef();
    return S_OK;
}

HRESULT DShowSource::BuildFilterGraph(LPCWSTR pszFilePath)
{
    if (!pszFilePath || !pszFilePath[0])
        return E_INVALIDARG;

    // Create the filter graph manager
    HRESULT hr = CoCreateInstance(
        CLSID_FilterGraph, nullptr, CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&m_spGraph));
    if (FAILED(hr))
        return hr;

    // Query interfaces
    hr = m_spGraph->QueryInterface(IID_PPV_ARGS(&m_spControl));
    if (FAILED(hr))
        return hr;

    hr = m_spGraph->QueryInterface(IID_PPV_ARGS(&m_spSeeking));
    if (FAILED(hr))
        return hr;

    hr = m_spGraph->QueryInterface(IID_PPV_ARGS(&m_spEvent));
    if (FAILED(hr))
        return hr;

    hr = m_spGraph->QueryInterface(IID_PPV_ARGS(&m_spVideoWindow));
    if (FAILED(hr))
        return hr;

    hr = m_spGraph->QueryInterface(IID_PPV_ARGS(&m_spBasicVideo));
    if (FAILED(hr))
        return hr;

    hr = m_spGraph->QueryInterface(IID_PPV_ARGS(&m_spBasicAudio));
    if (FAILED(hr))
        return hr;

    // Create and insert sample grabber
    hr = CreateSampleGrabber();
    if (FAILED(hr))
        return hr;

    // Render the file
    hr = RenderFile(pszFilePath);
    if (FAILED(hr))
        return hr;

    // Query duration
    if (m_spSeeking)
    {
        LONGLONG llDuration = 0;
        hr = m_spSeeking->GetDuration(&llDuration);
        if (SUCCEEDED(hr))
            m_llDurationHns = llDuration;
    }

    m_fGraphBuilt = true;
    return S_OK;
}

HRESULT DShowSource::RenderFile(LPCWSTR pszFilePath)
{
    if (!m_spGraph)
        return E_UNEXPECTED;

    return m_spGraph->RenderFile(pszFilePath, nullptr);
}

HRESULT DShowSource::GetVideoWindowHandle(HWND* phWnd)
{
    if (!phWnd)
        return E_POINTER;

    *phWnd = m_hVideoWnd;
    return S_OK;
}

HRESULT DShowSource::SetVideoWindowHandle(HWND hWnd)
{
    m_hVideoWnd = hWnd;

    if (m_spVideoWindow)
    {
        m_spVideoWindow->put_Owner((OAHWND)hWnd);
        m_spVideoWindow->put_WindowStyle(WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

        if (hWnd)
        {
            RECT rc;
            GetClientRect(hWnd, &rc);
            m_spVideoWindow->SetWindowPosition(0, 0, rc.right, rc.bottom);
        }
    }

    return S_OK;
}

HRESULT DShowSource::CheckDShowEvent(long* pEventCode, LONG_PTR* pParam1, LONG_PTR* pParam2)
{
    if (!m_spEvent)
        return E_UNEXPECTED;

    return m_spEvent->GetEvent(pEventCode, pParam1, pParam2, 0);
}

// ============================================================================
// Private helpers
// ============================================================================

HRESULT DShowSource::CreateSampleGrabber()
{
    HRESULT hr = CoCreateInstance(
        CLSID_SampleGrabber, nullptr, CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&m_spGrabberFilter));
    if (FAILED(hr))
        return hr;

    hr = m_spGrabberFilter->QueryInterface(IID_PPV_ARGS(&m_spSampleGrabber));
    if (FAILED(hr))
        return hr;

    // Set media type to RGB32 for grabbing
    AM_MEDIA_TYPE mt;
    ZeroMemory(&mt, sizeof(mt));
    mt.majortype = MEDIATYPE_Video;
    mt.subtype = MEDIASUBTYPE_RGB32;

    m_spSampleGrabber->SetMediaType(&mt);
    m_spSampleGrabber->SetOneShot(FALSE);
    m_spSampleGrabber->SetBufferSamples(TRUE);

    return S_OK;
}

HRESULT DShowSource::ConnectGrabberToGraph()
{
    if (!m_spGraph || !m_spGrabberFilter)
        return E_UNEXPECTED;

    return m_spGraph->AddFilter(m_spGrabberFilter, L"SampleGrabber");
}

HRESULT DShowSource::QueryVideoDimensions()
{
    if (!m_spBasicVideo)
        return E_UNEXPECTED;

    long width = 0, height = 0;
    HRESULT hr = m_spBasicVideo->GetVideoSize(&width, &height);
    if (SUCCEEDED(hr))
    {
        m_info.dwWidth = static_cast<DWORD>(width);
        m_info.dwHeight = static_cast<DWORD>(height);
        m_info.fHasVideo = true;
    }

    return hr;
}

HRESULT DShowSource::ConfigureDefaultAudio()
{
    m_info.fHasAudio = true;
    m_info.dwAudioSampleRate = 44100;
    m_info.dwAudioChannels = 2;
    return S_OK;
}

} // namespace HMRAVSource
