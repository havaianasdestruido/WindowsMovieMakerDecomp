// SyncVideoSource.cpp - Synchronized video sample source implementation

#include "pch.h"
#include "SyncVideoSource.h"

namespace HMRAVSource
{

// ============================================================================
// Construction / Destruction
// ============================================================================

SyncVideoSampleSource::SyncVideoSampleSource()
    : m_fInitialized(false)
    , m_fEndOfStream(false)
    , m_dwFrameCount(0)
    , m_dwCurrentFrameIndex(0)
    , m_dwFramesDelivered(0)
    , m_dwFramesSkipped(0)
    , m_llCurrentPositionHns(0)
    , m_llFrameDurationHns(0)
    , m_llLastTimestampHns(-1)
    , m_dblFrameRate(30.0)
{
}

SyncVideoSampleSource::~SyncVideoSampleSource()
{
    Shutdown();
}

// ============================================================================
// Lifecycle
// ============================================================================

HRESULT SyncVideoSampleSource::Initialize(const SyncVideoSourceDesc& desc)
{
    if (!desc.pSource)
        return E_POINTER;

    if (m_fInitialized)
        return E_UNEXPECTED;

    m_desc = desc;
    m_dblFrameRate = desc.dblTargetFrameRate;

    if (m_dblFrameRate > 0.0)
        m_llFrameDurationHns = static_cast<LONGLONG>(10000000.0 / m_dblFrameRate);

    m_fInitialized = true;
    return S_OK;
}

HRESULT SyncVideoSampleSource::Shutdown()
{
    if (!m_fInitialized)
        return S_OK;

    m_spPendingSample = nullptr;
    m_fInitialized = false;
    m_fEndOfStream = false;
    m_dwFrameCount = 0;
    m_dwCurrentFrameIndex = 0;
    m_dwFramesDelivered = 0;
    m_dwFramesSkipped = 0;
    m_llCurrentPositionHns = 0;
    m_llLastTimestampHns = -1;

    return S_OK;
}

bool SyncVideoSampleSource::IsInitialized() const throw()
{
    return m_fInitialized;
}

// ============================================================================
// Sample access
// ============================================================================

HRESULT SyncVideoSampleSource::GetNextSample(IMFSample** ppSample)
{
    if (!ppSample)
        return E_POINTER;

    *ppSample = nullptr;

    if (!m_fInitialized)
        return E_UNEXPECTED;

    if (m_fEndOfStream)
        return MF_E_END_OF_STREAM;

    CComPtr<IMFSample> spSample;
    HRESULT hr = ReadNextFrame(&spSample);

    if (hr == MF_E_END_OF_STREAM)
    {
        m_fEndOfStream = true;
        return MF_E_END_OF_STREAM;
    }

    if (FAILED(hr))
        return hr;

    if (m_desc.fDropDuplicateFrames && IsDuplicateFrame(spSample))
    {
        m_dwFramesSkipped++;
        return GetNextSample(ppSample);
    }

    CheckTimestampOrder(spSample);
    UpdateFrameCount(spSample);

    *ppSample = spSample.Detach();
    m_dwFramesDelivered++;

    return S_OK;
}

HRESULT SyncVideoSampleSource::GetSampleAtPosition(LONGLONG llPositionHns, IMFSample** ppSample)
{
    if (!ppSample)
        return E_POINTER;

    *ppSample = nullptr;

    if (!m_fInitialized)
        return E_UNEXPECTED;

    if (!m_desc.pSource)
        return E_UNEXPECTED;

    HRESULT hr = m_desc.pSource->SetPositionHns(llPositionHns);
    if (FAILED(hr))
        return hr;

    hr = m_desc.pSource->ReadSample(ppSample, m_desc.dwVideoStreamIndex);
    if (SUCCEEDED(hr) && *ppSample)
    {
        m_llCurrentPositionHns = llPositionHns;
        m_llLastTimestampHns = llPositionHns;
    }

    return hr;
}

HRESULT SyncVideoSampleSource::Flush()
{
    if (!m_fInitialized)
        return E_UNEXPECTED;

    m_spPendingSample = nullptr;
    m_fEndOfStream = false;
    m_dwCurrentFrameIndex = 0;
    m_llLastTimestampHns = -1;

    if (m_desc.pSource)
        return m_desc.pSource->Flush();

    return S_OK;
}

// ============================================================================
// Seeking
// ============================================================================

HRESULT SyncVideoSampleSource::Seek(LONGLONG llPositionHns)
{
    if (!m_fInitialized)
        return E_UNEXPECTED;

    m_llCurrentPositionHns = llPositionHns;
    m_spPendingSample = nullptr;
    m_fEndOfStream = false;
    m_llLastTimestampHns = -1;

    if (m_desc.pSource)
    {
        HRESULT hr = m_desc.pSource->SetPositionHns(llPositionHns);
        if (FAILED(hr))
            return hr;

        // Calculate frame index from position
        if (m_llFrameDurationHns > 0)
            m_dwCurrentFrameIndex = static_cast<DWORD>(llPositionHns / m_llFrameDurationHns);
    }

    return S_OK;
}

HRESULT SyncVideoSampleSource::SeekToFrame(DWORD dwFrameIndex)
{
    LONGLONG llPosition = static_cast<LONGLONG>(dwFrameIndex) * m_llFrameDurationHns;
    return Seek(llPosition);
}

LONGLONG SyncVideoSampleSource::GetCurrentPosition() const throw()
{
    return m_llCurrentPositionHns;
}

// ============================================================================
// Frame info
// ============================================================================

DWORD SyncVideoSampleSource::GetFrameCount() const throw()
{
    return m_dwFrameCount;
}

DWORD SyncVideoSampleSource::GetCurrentFrameIndex() const throw()
{
    return m_dwCurrentFrameIndex;
}

LONGLONG SyncVideoSampleSource::GetFrameTimestamp(DWORD /*dwFrameIndex*/) const
{
    return 0;
}

LONGLONG SyncVideoSampleSource::GetFrameDuration() const throw()
{
    return m_llFrameDurationHns;
}

double SyncVideoSampleSource::GetFrameRate() const throw()
{
    return m_dblFrameRate;
}

// ============================================================================
// State
// ============================================================================

bool SyncVideoSampleSource::IsEndOfStream() const throw()
{
    return m_fEndOfStream;
}

bool SyncVideoSampleSource::HasMoreSamples() const throw()
{
    return !m_fEndOfStream;
}

// ============================================================================
// Statistics
// ============================================================================

DWORD SyncVideoSampleSource::GetFramesDelivered() const throw()
{
    return m_dwFramesDelivered;
}

DWORD SyncVideoSampleSource::GetFramesSkipped() const throw()
{
    return m_dwFramesSkipped;
}

// ============================================================================
// Configuration
// ============================================================================

HRESULT SyncVideoSampleSource::SetTargetFrameRate(double dblFrameRate)
{
    if (dblFrameRate <= 0.0)
        return E_INVALIDARG;

    m_dblFrameRate = dblFrameRate;
    m_llFrameDurationHns = static_cast<LONGLONG>(10000000.0 / m_dblFrameRate);
    return S_OK;
}

void SyncVideoSampleSource::SetDropDuplicateFrames(bool fDrop)
{
    m_desc.fDropDuplicateFrames = fDrop;
}

void SyncVideoSampleSource::SetMaintainTimestampOrder(bool fMaintain)
{
    m_desc.fMaintainTimestampOrder = fMaintain;
}

// ============================================================================
// Private helpers
// ============================================================================

HRESULT SyncVideoSampleSource::ReadNextFrame(IMFSample** ppSample)
{
    if (!ppSample)
        return E_POINTER;

    *ppSample = nullptr;

    // Check for pending sample first
    if (m_spPendingSample)
    {
        *ppSample = m_spPendingSample.Detach();
        return S_OK;
    }

    if (!m_desc.pSource)
        return E_UNEXPECTED;

    return m_desc.pSource->ReadSample(ppSample, m_desc.dwVideoStreamIndex);
}

HRESULT SyncVideoSampleSource::CheckTimestampOrder(IMFSample* pSample)
{
    if (!pSample || !m_desc.fMaintainTimestampOrder)
        return S_OK;

    LONGLONG llTimestamp = 0;
    HRESULT hr = pSample->GetSampleTime(&llTimestamp);
    if (SUCCEEDED(hr))
    {
        if (m_llLastTimestampHns >= 0 && llTimestamp < m_llLastTimestampHns)
        {
            // Out-of-order timestamp detected - reorder
        }
        m_llLastTimestampHns = llTimestamp;
    }

    return S_OK;
}

bool SyncVideoSampleSource::IsDuplicateFrame(IMFSample* pSample) const
{
    if (!pSample)
        return false;

    LONGLONG llTimestamp = 0;
    HRESULT hr = pSample->GetSampleTime(&llTimestamp);
    if (FAILED(hr))
        return false;

    return (llTimestamp == m_llLastTimestampHns);
}

void SyncVideoSampleSource::UpdateFrameCount(IMFSample* pSample)
{
    if (!pSample)
        return;

    LONGLONG llTimestamp = 0;
    if (SUCCEEDED(pSample->GetSampleTime(&llTimestamp)))
    {
        m_llCurrentPositionHns = llTimestamp;

        if (m_llFrameDurationHns > 0)
            m_dwCurrentFrameIndex = static_cast<DWORD>(llTimestamp / m_llFrameDurationHns);
    }

    m_dwFrameCount++;
}

} // namespace HMRAVSource
