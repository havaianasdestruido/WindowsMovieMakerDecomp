// MFSource.cpp - Media Foundation source wrapper implementation

#include "pch.h"
#include "MFSource.h"

namespace HMRAVSource
{

// ============================================================================
// MFSourceReaderCallback
// ============================================================================

MFSourceReaderCallback::MFSourceReaderCallback()
    : m_cRef(1)
    , m_hEvent(nullptr)
    , m_dwLastStreamIndex(0)
    , m_dwLastStreamFlags(0)
    , m_llLastTimestamp(0)
    , m_hrLastStatus(S_OK)
{
    m_hEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
}

MFSourceReaderCallback::~MFSourceReaderCallback()
{
    if (m_hEvent)
        CloseHandle(m_hEvent);
}

STDMETHODIMP MFSourceReaderCallback::QueryInterface(REFIID riid, void** ppvObject)
{
    if (!ppvObject)
        return E_POINTER;

    *ppvObject = nullptr;

    if (IsEqualIID(riid, IID_IUnknown))
    {
        *ppvObject = static_cast<IMFSourceReaderCallback*>(this);
        AddRef();
        return S_OK;
    }
    if (IsEqualIID(riid, IID_IMFSourceReaderCallback))
    {
        *ppvObject = static_cast<IMFSourceReaderCallback*>(this);
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) MFSourceReaderCallback::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) MFSourceReaderCallback::Release()
{
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if (cRef == 0)
        delete this;
    return cRef;
}

STDMETHODIMP MFSourceReaderCallback::OnReadSample(
    HRESULT hrStatus,
    DWORD dwStreamIndex,
    DWORD dwStreamFlags,
    LONGLONG llTimestamp,
    IMFSample* pSample)
{
    m_hrLastStatus = hrStatus;
    m_dwLastStreamIndex = dwStreamIndex;
    m_dwLastStreamFlags = dwStreamFlags;
    m_llLastTimestamp = llTimestamp;

    if (pSample)
        m_spLastSample = pSample;

    if (m_hEvent)
        SetEvent(m_hEvent);

    return S_OK;
}

STDMETHODIMP MFSourceReaderCallback::OnFlush(DWORD /*dwStreamIndex*/)
{
    if (m_hEvent)
        SetEvent(m_hEvent);

    return S_OK;
}

STDMETHODIMP MFSourceReaderCallback::OnEvent(DWORD /*dwStreamIndex*/, IMFMediaEvent* /*pEvent*/)
{
    return S_OK;
}

HRESULT MFSourceReaderCallback::WaitSample(DWORD dwTimeoutMs)
{
    if (!m_hEvent)
        return E_UNEXPECTED;

    DWORD dwResult = WaitForSingleObject(m_hEvent, dwTimeoutMs);
    if (dwResult == WAIT_TIMEOUT)
        return MF_E_TIMEOUT;

    ResetEvent(m_hEvent);
    return m_hrLastStatus;
}

HRESULT MFSourceReaderCallback::GetLastSample(
    IMFSample** ppSample,
    DWORD* pdwStreamIndex,
    DWORD* pdwStreamFlags,
    LONGLONG* pllTimestamp)
{
    if (ppSample)
    {
        *ppSample = m_spLastSample;
        if (*ppSample)
            (*ppSample)->AddRef();
    }
    if (pdwStreamIndex)
        *pdwStreamIndex = m_dwLastStreamIndex;
    if (pdwStreamFlags)
        *pdwStreamFlags = m_dwLastStreamFlags;
    if (pllTimestamp)
        *pllTimestamp = m_llLastTimestamp;

    return m_hrLastStatus;
}

void MFSourceReaderCallback::Reset()
{
    m_spLastSample = nullptr;
    m_hrLastStatus = S_OK;
    m_dwLastStreamIndex = 0;
    m_dwLastStreamFlags = 0;
    m_llLastTimestamp = 0;

    if (m_hEvent)
        ResetEvent(m_hEvent);
}

// ============================================================================
// MFSource
// ============================================================================

MFSource::MFSource()
    : m_dwVideoStreamIndex(0)
    , m_dwAudioStreamIndex(0)
    , m_dwStreamCount(0)
    , m_fAsyncMode(false)
{
    m_type = AVSourceTypeFile;
}

MFSource::~MFSource()
{
    Close();
}

// ============================================================================
// Open / Close
// ============================================================================

HRESULT MFSource::Open(const AVSourceDesc& desc)
{
    if (IsOpen())
        Close();

    HRESULT hr = AVSource::Open(desc);
    if (FAILED(hr))
        return hr;

    hr = CreateSourceReader(desc);
    if (FAILED(hr))
    {
        SetState(AVSourceStateError);
        m_hrLastResult = hr;
        return hr;
    }

    hr = EnumerateStreams();
    if (FAILED(hr))
    {
        SetState(AVSourceStateError);
        m_hrLastResult = hr;
        return hr;
    }

    // Query duration
    PROPVARIANT varDuration;
    PropVariantInit(&varDuration);
    hr = m_spReader->GetPresentationAttribute(
        MF_SOURCE_READER_MEDIASOURCE,
        MF_PD_DURATION,
        &varDuration);
    if (SUCCEEDED(hr) && varDuration.vt == VT_UI8)
    {
        m_llDurationHns = static_cast<LONGLONG>(varDuration.uhVal.QuadPart);
        PropVariantClear(&varDuration);
    }

    SetState(AVSourceStateOpen);
    m_hrLastResult = S_OK;
    return S_OK;
}

HRESULT MFSource::Close()
{
    if (m_spReader)
    {
        m_spReader->Flush(MF_SOURCE_READER_ALL_STREAMS);
        m_spReader = nullptr;
    }

    m_spCallback = nullptr;
    m_dwVideoStreamIndex = 0;
    m_dwAudioStreamIndex = 0;
    m_dwStreamCount = 0;
    m_fAsyncMode = false;

    return AVSource::Close();
}

// ============================================================================
// Position
// ============================================================================

HRESULT MFSource::SetPositionHns(LONGLONG llPosition)
{
    HRESULT hr = AVSource::SetPositionHns(llPosition);
    if (FAILED(hr))
        return hr;

    return SeekToPosition(llPosition);
}

// ============================================================================
// Start / Stop
// ============================================================================

HRESULT MFSource::Start()
{
    HRESULT hr = AVSource::Start();
    if (FAILED(hr))
        return hr;

    if (m_spReader)
        m_spReader->SetStreamSelection(MF_SOURCE_READER_ALL_STREAMS, TRUE);

    m_hrLastResult = S_OK;
    return S_OK;
}

HRESULT MFSource::Stop()
{
    if (m_spReader)
        m_spReader->SetStreamSelection(MF_SOURCE_READER_ALL_STREAMS, FALSE);

    return AVSource::Stop();
}

// ============================================================================
// Sample reading
// ============================================================================

HRESULT MFSource::ReadSample(IMFSample** ppSample, DWORD dwStreamIndex)
{
    if (!ppSample)
        return E_POINTER;
    *ppSample = nullptr;

    if (!m_spReader || !IsOpen())
        return E_UNEXPECTED;

    DWORD dwActualStreamIndex = 0;
    DWORD dwStreamFlags = 0;
    LONGLONG llTimestamp = 0;

    if (m_fAsyncMode && m_spCallback)
    {
        m_spCallback->Reset();

        HRESULT hr = m_spCallback->WaitSample(5000);
        if (FAILED(hr))
            return hr;

        return m_spCallback->GetLastSample(ppSample, &dwActualStreamIndex, &dwStreamFlags, &llTimestamp);
    }
    else
    {
        CComPtr<IMFSample> spSample;
        HRESULT hr = m_spReader->ReadSample(
            dwStreamIndex == DWORD_MAX ? MF_SOURCE_READER_FIRST_VIDEO_STREAM : dwStreamIndex,
            0,
            nullptr,
            &dwStreamFlags,
            &llTimestamp,
            &spSample);

        if (FAILED(hr))
            return hr;

        if (dwStreamFlags & MF_SOURCE_READERF_ENDOFSTREAM)
            return MF_E_END_OF_STREAM;

        if (spSample)
        {
            *ppSample = spSample.Detach();
            m_llPositionHns = llTimestamp;
        }

        return S_OK;
    }
}

HRESULT MFSource::Flush()
{
    if (m_spReader)
        m_spReader->Flush(MF_SOURCE_READER_ALL_STREAMS);

    return AVSource::Flush();
}

// ============================================================================
// Events
// ============================================================================

HRESULT MFSource::GetEvent(IMFMediaEvent** ppEvent)
{
    if (!ppEvent)
        return E_POINTER;
    *ppEvent = nullptr;

    if (!m_spReader)
        return E_UNEXPECTED;

    CComPtr<IMFMediaEventGenerator> spEventGen;
    HRESULT hr = m_spReader->QueryInterface(IID_PPV_ARGS(&spEventGen));
    if (FAILED(hr))
        return hr;

    return spEventGen->GetEvent(0, ppEvent);
}

HRESULT MFSource::BeginGetEvent(IMFAsyncCallback* pCallback, IUnknown* punkState)
{
    if (!pCallback)
        return E_POINTER;

    if (!m_spReader)
        return E_UNEXPECTED;

    CComPtr<IMFMediaEventGenerator> spEventGen;
    HRESULT hr = m_spReader->QueryInterface(IID_PPV_ARGS(&spEventGen));
    if (FAILED(hr))
        return hr;

    return spEventGen->BeginGetEvent(pCallback, punkState);
}

HRESULT MFSource::EndGetEvent(IMFMediaEvent* pEvent, IMFMediaEvent** ppNextEvent)
{
    UNREFERENCED_PARAMETER(pEvent);
    if (ppNextEvent)
        *ppNextEvent = nullptr;
    return S_OK;
}

// ============================================================================
// MF-specific queries
// ============================================================================

HRESULT MFSource::GetSourceReader(IMFSourceReader** ppReader)
{
    if (!ppReader)
        return E_POINTER;

    if (!m_spReader)
        return E_UNEXPECTED;

    *ppReader = m_spReader;
    (*ppReader)->AddRef();
    return S_OK;
}

DWORD MFSource::GetStreamCount() const throw()
{
    return m_dwStreamCount;
}

HRESULT MFSource::GetStreamMediaType(DWORD dwStreamIndex, IMFMediaType** ppMediaType)
{
    if (!ppMediaType)
        return E_POINTER;

    if (!m_spReader)
        return E_UNEXPECTED;

    return m_spReader->GetNativeMediaType(dwStreamIndex, 0, ppMediaType);
}

HRESULT MFSource::GetVideoFrameSize(UINT* pWidth, UINT* pHeight)
{
    if (!pWidth || !pHeight)
        return E_POINTER;

    *pWidth = m_info.dwWidth;
    *pHeight = m_info.dwHeight;
    return S_OK;
}

HRESULT MFSource::SetVideoFrameSize(UINT /*width*/, UINT /*height*/)
{
    // Frame size is typically negotiated via the output type
    return S_OK;
}

HRESULT MFSource::GetAudioFormat(WAVEFORMATEX* pWfx)
{
    if (!pWfx)
        return E_POINTER;

    ZeroMemory(pWfx, sizeof(WAVEFORMATEX));
    pWfx->wFormatTag = WAVE_FORMAT_PCM;
    pWfx->nChannels = static_cast<WORD>(m_info.dwAudioChannels);
    pWfx->nSamplesPerSec = m_info.dwAudioSampleRate;
    pWfx->wBitsPerSample = static_cast<WORD>(m_info.dwAudioBitsPerSample);
    pWfx->nBlockAlign = (pWfx->nChannels * pWfx->wBitsPerSample) / 8;
    pWfx->nAvgBytesPerSec = pWfx->nSamplesPerSec * pWfx->nBlockAlign;

    return S_OK;
}

HRESULT MFSource::SetAudioFormat(const WAVEFORMATEX* /*pWfx*/)
{
    return S_OK;
}

HRESULT MFSource::GetSelectedMediaType(DWORD dwStreamIndex, IMFMediaType** ppType)
{
    if (!ppType)
        return E_POINTER;

    if (!m_spReader)
        return E_UNEXPECTED;

    return m_spReader->GetCurrentMediaType(dwStreamIndex, ppType);
}

HRESULT MFSource::SetSelectedMediaType(DWORD dwStreamIndex, IMFMediaType* pType)
{
    if (!pType)
        return E_POINTER;

    if (!m_spReader)
        return E_UNEXPECTED;

    return m_spReader->SetCurrentMediaType(dwStreamIndex, nullptr, pType);
}

// ============================================================================
// Private helpers
// ============================================================================

HRESULT MFSource::CreateSourceReader(const AVSourceDesc& desc)
{
    if (desc.strFilePath.IsEmpty())
        return E_INVALIDARG;

    HRESULT hr = S_OK;

    // Create the source reader with callback for async mode
    m_spCallback = new (std::nothrow) MFSourceReaderCallback();
    if (!m_spCallback)
        return E_OUTOFMEMORY;

    hr = MFCreateSourceReaderFromURL(
        desc.strFilePath.GetString(),
        nullptr,
        &m_spReader);

    if (FAILED(hr))
    {
        m_spCallback = nullptr;
        return hr;
    }

    return S_OK;
}

HRESULT MFSource::EnumerateStreams()
{
    if (!m_spReader)
        return E_UNEXPECTED;

    m_dwStreamCount = 0;
    m_dwVideoStreamIndex = DWORD_MAX;
    m_dwAudioStreamIndex = DWORD_MAX;

    for (DWORD dwIndex = 0; ; ++dwIndex)
    {
        CComPtr<IMFMediaType> spMediaType;
        HRESULT hr = m_spReader->GetNativeMediaType(dwIndex, 0, &spMediaType);
        if (FAILED(hr))
            break;

        GUID majorType = GUID_NULL;
        hr = spMediaType->GetGUID(MF_MT_MAJOR_TYPE, &majorType);
        if (FAILED(hr))
            break;

        if (majorType == MFMediaType_Video && m_dwVideoStreamIndex == DWORD_MAX)
        {
            m_dwVideoStreamIndex = dwIndex;
            m_info.fHasVideo = true;

            UINT32 width = 0, height = 0;
            MFGetAttributeSize(spMediaType, MF_MT_FRAME_SIZE, &width, &height);
            m_info.dwWidth = width;
            m_info.dwHeight = height;

            UINT32 numerator = 0, denominator = 0;
            MFGetAttributeRatio(spMediaType, MF_MT_FRAME_RATE, &numerator, &denominator);
            if (denominator > 0)
                m_info.dblFrameRate = static_cast<double>(numerator) / static_cast<double>(denominator);
        }
        else if (majorType == MFMediaType_Audio && m_dwAudioStreamIndex == DWORD_MAX)
        {
            m_dwAudioStreamIndex = dwIndex;
            m_info.fHasAudio = true;

            UINT32 sampleRate = 0;
            spMediaType->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &sampleRate);
            m_info.dwAudioSampleRate = sampleRate;

            UINT32 channels = 0;
            spMediaType->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &channels);
            m_info.dwAudioChannels = channels;

            UINT32 bitsPerSample = 0;
            spMediaType->GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, &bitsPerSample);
            m_info.dwAudioBitsPerSample = bitsPerSample;
        }

        m_dwStreamCount++;
    }

    if (m_info.fHasVideo || m_info.fHasAudio)
        return S_OK;

    return MF_E_INVALIDMEDIATYPE;
}

HRESULT MFSource::ConfigureVideoStream(UINT /*width*/, UINT /*height*/)
{
    return S_OK;
}

HRESULT MFSource::ConfigureAudioStream()
{
    return S_OK;
}

HRESULT MFSource::SeekToPosition(LONGLONG llPosition)
{
    if (!m_spReader)
        return E_UNEXPECTED;

    PROPVARIANT varPosition;
    PropVariantInit(&varPosition);
    varPosition.vt = VT_I8;
    varPosition.uhVal.QuadPart = llPosition;

    HRESULT hr = m_spReader->SetCurrentPosition(GUID_NULL, varPosition);
    PropVariantClear(&varPosition);
    return hr;
}

} // namespace HMRAVSource
