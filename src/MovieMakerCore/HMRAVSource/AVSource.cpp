// AVSource.cpp - Base audio/video source implementation

#include "pch.h"
#include "AVSource.h"

namespace HMRAVSource
{

// ============================================================================
// Static members
// ============================================================================
DWORD AVSource::s_dwNextSourceId = 1;

// ============================================================================
// Construction / Destruction
// ============================================================================

AVSource::AVSource()
    : m_type(AVSourceTypeUnknown)
    , m_state(AVSourceStateIdle)
    , m_hrLastResult(S_OK)
    , m_dwSourceId(InterlockedIncrement(reinterpret_cast<LONG*>(&s_dwNextSourceId)))
    , m_llDurationHns(0)
    , m_llPositionHns(0)
    , m_cRef(1)
{
    ResetInfo();
}

AVSource::~AVSource()
{
    if (IsOpen())
        Close();
}

// ============================================================================
// Identity
// ============================================================================

AVSourceType AVSource::GetType() const throw()
{
    return m_type;
}

DWORD AVSource::GetSourceId() const throw()
{
    return m_dwSourceId;
}

// ============================================================================
// Open / Close
// ============================================================================

HRESULT AVSource::Open(const AVSourceDesc& desc)
{
    if (m_state != AVSourceStateIdle && m_state != AVSourceStateClosed)
    {
        m_hrLastResult = E_UNEXPECTED;
        return E_UNEXPECTED;
    }

    SetState(AVSourceStateOpening);
    m_hrLastResult = S_OK;

    // Base implementation just stores the descriptor state.
    // Subclasses override to perform actual source creation.
    return S_OK;
}

HRESULT AVSource::Close()
{
    if (m_state == AVSourceStateClosed || m_state == AVSourceStateIdle)
        return S_OK;

    m_llPositionHns = 0;
    m_llDurationHns = 0;
    ResetInfo();
    SetState(AVSourceStateClosed);

    return S_OK;
}

bool AVSource::IsOpen() const throw()
{
    return m_state != AVSourceStateIdle &&
           m_state != AVSourceStateClosed &&
           m_state != AVSourceStateError;
}

// ============================================================================
// State
// ============================================================================

AVSourceState AVSource::GetState() const throw()
{
    return m_state;
}

HRESULT AVSource::GetLastResult() const throw()
{
    return m_hrLastResult;
}

const AVSourceInfo& AVSource::GetInfo() const
{
    return m_info;
}

LONGLONG AVSource::GetDurationHns() const throw()
{
    return m_llDurationHns;
}

LONGLONG AVSource::GetPositionHns() const throw()
{
    return m_llPositionHns;
}

HRESULT AVSource::SetPositionHns(LONGLONG llPosition)
{
    if (!IsOpen())
        return E_UNEXPECTED;

    if (llPosition < 0)
        llPosition = 0;
    if (llPosition > m_llDurationHns)
        llPosition = m_llDurationHns;

    m_llPositionHns = llPosition;
    m_hrLastResult = S_OK;
    return S_OK;
}

// ============================================================================
// Start / Stop / Pause
// ============================================================================

HRESULT AVSource::Start()
{
    if (!IsOpen())
        return E_UNEXPECTED;

    SetState(AVSourceStateStarted);
    m_hrLastResult = S_OK;
    return S_OK;
}

HRESULT AVSource::Stop()
{
    if (!IsOpen())
        return E_UNEXPECTED;

    SetState(AVSourceStateStopped);
    m_hrLastResult = S_OK;
    return S_OK;
}

HRESULT AVSource::Pause()
{
    if (m_state != AVSourceStateStarted)
        return E_UNEXPECTED;

    SetState(AVSourceStatePaused);
    m_hrLastResult = S_OK;
    return S_OK;
}

HRESULT AVSource::Resume()
{
    if (m_state != AVSourceStatePaused)
        return E_UNEXPECTED;

    SetState(AVSourceStateStarted);
    m_hrLastResult = S_OK;
    return S_OK;
}

// ============================================================================
// Media type queries
// ============================================================================

HRESULT AVSource::GetNativeVideoType(GUID* pSubtype, UINT* pWidth, UINT* pHeight)
{
    if (!pSubtype || !pWidth || !pHeight)
        return E_POINTER;

    if (!m_info.fHasVideo)
    {
        *pSubtype = GUID_NULL;
        *pWidth = 0;
        *pHeight = 0;
        return MF_E_INVALIDMEDIATYPE;
    }

    *pSubtype = MFVideoFormat_NV12; // default guess
    *pWidth = m_info.dwWidth;
    *pHeight = m_info.dwHeight;
    return S_OK;
}

HRESULT AVSource::GetNativeAudioType(GUID* pSubtype, DWORD* pSampleRate, DWORD* pChannels)
{
    if (!pSubtype || !pSampleRate || !pChannels)
        return E_POINTER;

    if (!m_info.fHasAudio)
    {
        *pSubtype = GUID_NULL;
        *pSampleRate = 0;
        *pChannels = 0;
        return MF_E_INVALIDMEDIATYPE;
    }

    *pSubtype = MFAudioFormat_AAC;
    *pSampleRate = m_info.dwAudioSampleRate;
    *pChannels = m_info.dwAudioChannels;
    return S_OK;
}

// ============================================================================
// Sample reading
// ============================================================================

HRESULT AVSource::ReadSample(IMFSample** ppSample, DWORD dwStreamIndex)
{
    if (!ppSample)
        return E_POINTER;

    *ppSample = nullptr;

    if (!IsOpen())
        return E_UNEXPECTED;

    // Base implementation returns no sample. Subclasses provide real data.
    return MF_E_END_OF_STREAM;
}

HRESULT AVSource::Flush()
{
    m_llPositionHns = 0;
    return S_OK;
}

// ============================================================================
// Event handling
// ============================================================================

HRESULT AVSource::GetEvent(IMFMediaEvent** ppEvent)
{
    if (!ppEvent)
        return E_POINTER;
    *ppEvent = nullptr;
    return E_NOTIMPL;
}

HRESULT AVSource::BeginGetEvent(IMFAsyncCallback* pCallback, IUnknown* punkState)
{
    return E_NOTIMPL;
}

HRESULT AVSource::EndGetEvent(IMFMediaEvent* pEvent, IMFMediaEvent** ppNextEvent)
{
    return E_NOTIMPL;
}

// ============================================================================
// Error info
// ============================================================================

ATL::CString AVSource::GetLastErrorDescription() const
{
    if (SUCCEEDED(m_hrLastResult))
        return CString();

    ATL::CString strError;
    strError.Format(L"HRESULT: 0x%08X", m_hrLastResult);
    return strError;
}

// ============================================================================
// Reference counting
// ============================================================================

void AVSource::AddRef()
{
    InterlockedIncrement(&m_cRef);
}

void AVSource::Release()
{
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (cRef == 0)
        delete this;
}

// ============================================================================
// Protected helpers
// ============================================================================

void AVSource::SetState(AVSourceState state)
{
    m_state = state;
}

void AVSource::SetLastError(HRESULT hr)
{
    m_hrLastResult = hr;
    if (FAILED(hr))
        m_state = AVSourceStateError;
}

void AVSource::ResetInfo()
{
    m_info = AVSourceInfo();
}

} // namespace HMRAVSource
