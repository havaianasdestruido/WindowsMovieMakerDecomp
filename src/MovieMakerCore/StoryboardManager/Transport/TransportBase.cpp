#include "pch.h"
/*
 * TransportBase.cpp
 *
 * Implementation of TransportBase, MovieTransport, and RenderTransport.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "TransportBase.h"

#ifndef MEPosition
#define MEPosition ((MediaEventType)31)
#endif

// ============================================================================
// TransportBase implementation
// ============================================================================

TransportBase::TransportBase()
    : m_state(TransportStateStopped)
    , m_dblRate(1.0)
    , m_flVolume(1.0f)
    , m_llCurrentPositionHns(0)
    , m_llDurationHns(0)
{
}

TransportBase::~TransportBase()
{
}

HRESULT TransportBase::SetRate(double dblRate)
{
    m_dblRate = std::max(0.0, dblRate);
    return S_OK;
}

double TransportBase::GetRate() const throw()
{
    return m_dblRate;
}

HRESULT TransportBase::GetCurrentPosition(LONGLONG* pllPositionHns) const
{
    if (!pllPositionHns)
        return E_POINTER;
    *pllPositionHns = m_llCurrentPositionHns;
    return S_OK;
}

HRESULT TransportBase::GetDuration(LONGLONG* pllDurationHns) const
{
    if (!pllDurationHns)
        return E_POINTER;
    *pllDurationHns = m_llDurationHns;
    return S_OK;
}

TransportState TransportBase::GetState() const throw()
{
    return m_state;
}

bool TransportBase::IsPlaying() const throw()
{
    return m_state == TransportStatePlaying || m_state == TransportStateTransitioning;
}

bool TransportBase::IsPaused() const throw()
{
    return m_state == TransportStatePaused;
}

bool TransportBase::IsStopped() const throw()
{
    return m_state == TransportStateStopped;
}

HRESULT TransportBase::SetVolume(float flVolume)
{
    m_flVolume = std::max(0.0f, std::min(1.0f, flVolume));
    return S_OK;
}

float TransportBase::GetVolume() const throw()
{
    return m_flVolume;
}

void TransportBase::SetStateChangeCallback(std::function<void(TransportState, TransportState)> fn)
{
    m_fnStateChange = std::move(fn);
}

void TransportBase::SetPositionChangeCallback(std::function<void(LONGLONG)> fn)
{
    m_fnPositionChange = std::move(fn);
}

void TransportBase::SetErrorCallback(std::function<void(HRESULT)> fn)
{
    m_fnError = std::move(fn);
}

void TransportBase::SetState(TransportState state)
{
    FireStateChange(m_state, state);
    m_state = state;
}

void TransportBase::FireStateChange(TransportState oldState, TransportState newState)
{
    if (m_fnStateChange)
        m_fnStateChange(oldState, newState);
}

void TransportBase::FirePositionChange(LONGLONG llPosition)
{
    if (m_fnPositionChange)
        m_fnPositionChange(llPosition);
}

void TransportBase::FireError(HRESULT hr)
{
    if (m_fnError)
        m_fnError(hr);
}

// ============================================================================
// MovieTransport implementation
// ============================================================================

MovieTransport::MovieTransport()
    : TransportBase()
    , m_bMediaOpen(false)
    , m_bLoopEnabled(false)
    , m_llLoopStartHns(0)
    , m_llLoopEndHns(0)
    , m_llFrameStepHns(10000000) // 1 second default
{
}

MovieTransport::~MovieTransport()
{
    CloseMedia();
}

HRESULT MovieTransport::Play()
{
    if (!m_bMediaOpen)
        return E_UNEXPECTED;

    if (m_spSession)
    {
        HRESULT hr = m_spSession->Start(nullptr, nullptr);
        if (FAILED(hr))
        {
            FireError(hr);
            return hr;
        }
    }

    SetState(TransportStatePlaying);
    return S_OK;
}

HRESULT MovieTransport::Pause()
{
    if (!m_bMediaOpen)
        return E_UNEXPECTED;

    if (m_spSession)
    {
        HRESULT hr = m_spSession->Pause();
        if (FAILED(hr))
        {
            FireError(hr);
            return hr;
        }
    }

    SetState(TransportStatePaused);
    return S_OK;
}

HRESULT MovieTransport::Stop()
{
    if (m_spSession)
    {
        HRESULT hr = m_spSession->Stop();
        if (FAILED(hr))
            FireError(hr);
    }

    SetState(TransportStateStopped);
    m_llCurrentPositionHns = 0;
    FirePositionChange(m_llCurrentPositionHns);
    return S_OK;
}

HRESULT MovieTransport::Seek(LONGLONG llPositionHns, DWORD dwFlags)
{
    if (!m_bMediaOpen)
        return E_UNEXPECTED;

    TransportState prevState = m_state;
    if (m_state == TransportStatePlaying)
        SetState(TransportStateSeeking);

    if (dwFlags & TransportSeekFlagRelative)
        m_llCurrentPositionHns += llPositionHns;
    else
        m_llCurrentPositionHns = llPositionHns;

    m_llCurrentPositionHns = std::max<LONGLONG>(0, std::min(m_llCurrentPositionHns, m_llDurationHns));

    if (m_spSession && (dwFlags & TransportSeekFlagKeyFrame))
    {
        PROPVARIANT varStart;
        PropVariantInit(&varStart);
        varStart.vt = VT_I8;
        varStart.hVal.QuadPart = m_llCurrentPositionHns;

        IMFPresentationClock* pClock = nullptr;
        CComPtr<IMFClock> spSessionClock;
        if (SUCCEEDED(m_spSession->GetClock(&spSessionClock)) && spSessionClock)
        {
            spSessionClock->QueryInterface(IID_PPV_ARGS(&pClock));
        }
        if (pClock)
        {
            IMFPresentationTimeSource* pTimeSource = nullptr;
            if (SUCCEEDED(pClock->GetTimeSource(&pTimeSource)))
            {
                IMFMediaEventGenerator* pEventGen = nullptr;
                if (SUCCEEDED(pTimeSource->QueryInterface(IID_IMFMediaEventGenerator, (void**)&pEventGen)))
                {
                    pEventGen->QueueEvent(MEPosition, GUID_NULL, S_OK, &varStart);
                    pEventGen->Release();
                }
                pTimeSource->Release();
            }
            pClock->Release();
        }

        PropVariantClear(&varStart);
    }

    FirePositionChange(m_llCurrentPositionHns);

    if (m_state == TransportStateSeeking)
        SetState(prevState);

    return S_OK;
}

HRESULT MovieTransport::SetRate(double dblRate)
{
    if (dblRate <= 0.0 || dblRate > 8.0)
        return E_INVALIDARG;

    return TransportBase::SetRate(dblRate);
}

void MovieTransport::SetLoopRegion(LONGLONG llStartHns, LONGLONG llEndHns)
{
    m_llLoopStartHns = llStartHns;
    m_llLoopEndHns = llEndHns;
    m_bLoopEnabled = true;
}

void MovieTransport::ClearLoopRegion()
{
    m_bLoopEnabled = false;
    m_llLoopStartHns = 0;
    m_llLoopEndHns = 0;
}

bool MovieTransport::HasLoopRegion() const throw()
{
    return m_bLoopEnabled;
}

LONGLONG MovieTransport::GetLoopStartHns() const throw()
{
    return m_llLoopStartHns;
}

LONGLONG MovieTransport::GetLoopEndHns() const throw()
{
    return m_llLoopEndHns;
}

HRESULT MovieTransport::StepForward()
{
    LONGLONG llNewPos = m_llCurrentPositionHns + m_llFrameStepHns;
    return Seek(llNewPos, TransportSeekFlagAbsolute);
}

HRESULT MovieTransport::StepBackward()
{
    LONGLONG llNewPos = m_llCurrentPositionHns - m_llFrameStepHns;
    return Seek(std::max(0LL, llNewPos), TransportSeekFlagAbsolute);
}

void MovieTransport::SetFrameStepSize(LONGLONG llStepHns)
{
    m_llFrameStepHns = std::max(1000000LL, llStepHns); // minimum 100ms
}

LONGLONG MovieTransport::GetFrameStepSize() const throw()
{
    return m_llFrameStepHns;
}

HRESULT MovieTransport::OpenMedia(LPCWSTR pszFilePath)
{
    if (!pszFilePath)
        return E_POINTER;

    CloseMedia();

    HRESULT hr = MFStartup(MF_VERSION, MFSTARTUP_LITE);
    if (FAILED(hr))
        return hr;

    IMFMediaSession* pSession = nullptr;
    hr = MFCreateMediaSession(nullptr, &pSession);
    if (FAILED(hr))
        return hr;

    m_spSession = pSession;
    pSession->Release();

    IMFSourceResolver* pSourceResolver = nullptr;
    hr = MFCreateSourceResolver(&pSourceResolver);
    if (FAILED(hr))
        return hr;

    MF_OBJECT_TYPE objectType = MF_OBJECT_UNKNOWN;
    IUnknown* pSourceUnk = nullptr;
    hr = pSourceResolver->CreateObjectFromURL(
        pszFilePath, MF_RESOLUTION_MEDIASOURCE, nullptr,
        &objectType, &pSourceUnk);
    pSourceResolver->Release();

    if (FAILED(hr))
        return hr;

    IMFMediaSource* pSource = nullptr;
    hr = pSourceUnk->QueryInterface(IID_IMFMediaSource, (void**)&pSource);
    pSourceUnk->Release();

    if (FAILED(hr))
        return hr;

    m_spSource = pSource;
    pSource->Release();

    IMFTopology* pTopology = nullptr;
    hr = MFCreateTopology(&pTopology);
    if (FAILED(hr))
        return hr;

    m_spTopology = pTopology;
    pTopology->Release();

    m_bMediaOpen = true;
    return S_OK;
}

HRESULT MovieTransport::CloseMedia()
{
    if (m_spSession)
    {
        m_spSession->Close();
        m_spSession.Release();
    }
    m_spTopology.Release();
    m_spSource.Release();
    m_bMediaOpen = false;
    m_llCurrentPositionHns = 0;
    MFShutdown();
    return S_OK;
}

bool MovieTransport::IsMediaOpen() const throw()
{
    return m_bMediaOpen;
}

// ============================================================================
// RenderTransport implementation
// ============================================================================

RenderTransport::RenderTransport()
    : TransportBase()
    , m_dwProfileIndex(0)
    , m_uOutputWidth(1920)
    , m_uOutputHeight(1080)
    , m_dwFrameRateNumerator(30)
    , m_dwFrameRateDenominator(1)
    , m_dwBitRateKbps(8000)
    , m_flRenderProgress(0.0f)
    , m_bAbortRequested(false)
    , m_hrRenderResult(S_OK)
{
}

RenderTransport::~RenderTransport()
{
}

HRESULT RenderTransport::Play()
{
    if (m_strOutputPath.IsEmpty())
        return E_UNEXPECTED;

    if (m_bAbortRequested)
    {
        FireError(E_ABORT);
        return E_ABORT;
    }

    if (m_state == TransportStatePlaying)
        return S_FALSE;

    SetState(TransportStatePlaying);
    return S_OK;
}

HRESULT RenderTransport::Pause()
{
    if (m_state == TransportStatePlaying)
    {
        SetState(TransportStatePaused);
    }
    else if (m_state != TransportStatePaused)
    {
        return E_UNEXPECTED;
    }
    return S_OK;
}

HRESULT RenderTransport::Stop()
{
    if (m_state == TransportStateStopped)
        return S_FALSE;

    SetState(TransportStateStopped);
    m_llCurrentPositionHns = 0;
    m_flRenderProgress = 0.0f;
    m_bAbortRequested = false;
    FirePositionChange(m_llCurrentPositionHns);
    return S_OK;
}

HRESULT RenderTransport::Seek(LONGLONG llPositionHns, DWORD dwFlags)
{
    if (m_state != TransportStateStopped && m_state != TransportStatePaused)
    {
        if (m_state == TransportStatePlaying)
        {
            SetState(TransportStateSeeking);
        }
        else if (m_state == TransportStateSeeking)
        {
            // Already seeking, update position
        }
        else
        {
            return E_UNEXPECTED;
        }
    }

    if (dwFlags & TransportSeekFlagRelative)
        m_llCurrentPositionHns += llPositionHns;
    else
        m_llCurrentPositionHns = llPositionHns;

    // Clamp to valid range
    if (m_llDurationHns > 0)
        m_llCurrentPositionHns = std::max<LONGLONG>(0, std::min(m_llCurrentPositionHns, m_llDurationHns));
    else
        m_llCurrentPositionHns = std::max<LONGLONG>(0, m_llCurrentPositionHns);

    // Update render progress if we have a target duration
    if (m_llDurationHns > 0)
    {
        m_flRenderProgress = static_cast<float>(m_llCurrentPositionHns) /
                             static_cast<float>(m_llDurationHns);
        m_flRenderProgress = std::max(0.0f, std::min(1.0f, m_flRenderProgress));

        if (m_fnProgress)
            m_fnProgress(m_flRenderProgress);
    }

    FirePositionChange(m_llCurrentPositionHns);

    if (m_state == TransportStateSeeking)
        SetState(TransportStatePlaying);

    return S_OK;
}

HRESULT RenderTransport::SetOutputPath(LPCWSTR pszPath)
{
    m_strOutputPath = pszPath ? pszPath : L"";
    return S_OK;
}

ATL::CString RenderTransport::GetOutputPath() const
{
    return m_strOutputPath;
}

HRESULT RenderTransport::SetProfile(DWORD dwProfileIndex)
{
    m_dwProfileIndex = dwProfileIndex;
    return S_OK;
}

DWORD RenderTransport::GetProfile() const throw()
{
    return m_dwProfileIndex;
}

void RenderTransport::SetOutputWidth(UINT uWidth)
{
    m_uOutputWidth = uWidth;
}

UINT RenderTransport::GetOutputWidth() const throw()
{
    return m_uOutputWidth;
}

void RenderTransport::SetOutputHeight(UINT uHeight)
{
    m_uOutputHeight = uHeight;
}

UINT RenderTransport::GetOutputHeight() const throw()
{
    return m_uOutputHeight;
}

void RenderTransport::SetFrameRate(DWORD dwFrameRateNumerator, DWORD dwFrameRateDenominator)
{
    m_dwFrameRateNumerator = dwFrameRateNumerator;
    m_dwFrameRateDenominator = dwFrameRateDenominator > 0 ? dwFrameRateDenominator : 1;
}

DWORD RenderTransport::GetFrameRateNumerator() const throw()
{
    return m_dwFrameRateNumerator;
}

DWORD RenderTransport::GetFrameRateDenominator() const throw()
{
    return m_dwFrameRateDenominator;
}

void RenderTransport::SetBitRate(DWORD dwBitRateKbps)
{
    m_dwBitRateKbps = dwBitRateKbps;
}

DWORD RenderTransport::GetBitRate() const throw()
{
    return m_dwBitRateKbps;
}

void RenderTransport::SetProgressCallback(std::function<void(float)> fn)
{
    m_fnProgress = std::move(fn);
}

float RenderTransport::GetRenderProgress() const throw()
{
    return m_flRenderProgress;
}

void RenderTransport::RequestAbort()
{
    m_bAbortRequested = true;
}

bool RenderTransport::IsAbortRequested() const throw()
{
    return m_bAbortRequested;
}

HRESULT RenderTransport::GetRenderResult() const throw()
{
    return m_hrRenderResult;
}
