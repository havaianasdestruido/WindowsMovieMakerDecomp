// SyncVideoSource.cpp - Synchronized video sample source implementation

#include "pch.h"
#include "SyncVideoSource.h"

namespace
{

// Maximum frame duration (100 ns units) that can be stored without the
// frame-index calculations overflowing 64-bit math. Cap is ~214 seconds
// per frame (a minimum frame rate of about 0.0046 fps).
const LONGLONG c_llMaxFrameDurationHns = 0x7FFFFFFF;
const LONGLONG c_llMaxDwordValue = 0xFFFFFFFF;

// Convert a target frame rate to a frame duration in 100 ns units,
// clamping to the representable range. Returns 0 for invalid rates.
LONGLONG ComputeFrameDurationHns(double dblFrameRate)
{
    if (!(dblFrameRate > 0.0))
        return 0;

    double dblDuration = 10000000.0 / dblFrameRate;

    // Clamp NaN/Inf/oversized results so the cast to LONGLONG is well
    // defined and subsequent multiplication/division cannot overflow.
    if (!(dblDuration >= 1.0))
        return 1;
    if (dblDuration > static_cast<double>(c_llMaxFrameDurationHns))
        return c_llMaxFrameDurationHns;

    return static_cast<LONGLONG>(dblDuration);
}

// Convert a timestamp (100 ns units) to a frame index, saturating at the
// DWORD range and treating negative timestamps as frame zero.
DWORD FrameIndexFromTimestampHns(LONGLONG llTimestampHns, LONGLONG llFrameDurationHns)
{
    if (llTimestampHns <= 0 || llFrameDurationHns <= 0)
        return 0;

    LONGLONG llFrameIndex = llTimestampHns / llFrameDurationHns;
    if (llFrameIndex > c_llMaxDwordValue)
        return static_cast<DWORD>(c_llMaxDwordValue);

    return static_cast<DWORD>(llFrameIndex);
}

} // namespace

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
    m_llFrameDurationHns = ComputeFrameDurationHns(m_dblFrameRate);

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

    // Iterate instead of recursing so long runs of duplicate frames
    // (still video, VFR content, etc.) cannot overflow the stack.
    for (;;)
    {
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
            spSample = nullptr;
            continue;
        }

        break;
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
        // Track the actual sample timestamp (not the requested position)
        // so the frame delivered here is not mistaken for a duplicate of
        // the next one after the seek.
        LONGLONG llSampleTime = 0;
        if (SUCCEEDED((*ppSample)->GetSampleTime(&llSampleTime)) && llSampleTime >= 0)
        {
            m_llCurrentPositionHns = llSampleTime;
            m_llLastTimestampHns = llSampleTime;
        }
        else
        {
            m_llCurrentPositionHns = llPositionHns;
            m_llLastTimestampHns = llPositionHns;
        }
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
        m_dwCurrentFrameIndex = FrameIndexFromTimestampHns(llPositionHns, m_llFrameDurationHns);
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
    m_llFrameDurationHns = ComputeFrameDurationHns(m_dblFrameRate);
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
        m_dwCurrentFrameIndex = FrameIndexFromTimestampHns(llTimestamp, m_llFrameDurationHns);
    }

    m_dwFrameCount++;
}

} // namespace HMRAVSource
