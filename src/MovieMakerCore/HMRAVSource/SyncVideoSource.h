/*
 * SyncVideoSource.h
 *
 * SyncVideoSource - Synchronized video sample source for frame-accurate
 * playback. Provides synchronized access to video samples from an MF
 * source reader with frame-level timing and presentation.
 *
 * RTTI: ?AVSyncVideoSampleSource@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef SYNCVIDEOSAMPLESOURCE_H
#define SYNCVIDEOSAMPLESOURCE_H

#include "HMRAVSource.h"
#include "AVSource.h"

namespace HMRAVSource
{

// ============================================================================
// SyncVideoSourceDesc
// ============================================================================
struct SyncVideoSourceDesc
{
    AVSource*   pSource;
    DWORD       dwVideoStreamIndex;
    double      dblTargetFrameRate;
    bool        fDropDuplicateFrames;
    bool        fMaintainTimestampOrder;

    SyncVideoSourceDesc()
        : pSource(nullptr)
        , dwVideoStreamIndex(0)
        , dblTargetFrameRate(30.0)
        , fDropDuplicateFrames(true)
        , fMaintainTimestampOrder(true)
    {
    }
};

// ============================================================================
// SyncVideoSampleSource
// ============================================================================
// Provides frame-accurate synchronized access to video samples from a
// source. Manages frame timing, duplicate frame detection, and ensures
// frames are delivered in the correct presentation order with proper
// timestamp tracking.
//
class AVSOURCE_API SyncVideoSampleSource
{
public:
    SyncVideoSampleSource();
    ~SyncVideoSampleSource();

    // Lifecycle
    HRESULT Initialize(const SyncVideoSourceDesc& desc);
    HRESULT Shutdown();
    bool IsInitialized() const throw();

    // Sample access
    HRESULT GetNextSample(IMFSample** ppSample);
    HRESULT GetSampleAtPosition(LONGLONG llPositionHns, IMFSample** ppSample);
    HRESULT Flush();

    // Seeking
    HRESULT Seek(LONGLONG llPositionHns);
    HRESULT SeekToFrame(DWORD dwFrameIndex);
    LONGLONG GetCurrentPosition() const throw();

    // Frame info
    DWORD GetFrameCount() const throw();
    DWORD GetCurrentFrameIndex() const throw();
    LONGLONG GetFrameTimestamp(DWORD dwFrameIndex) const;
    LONGLONG GetFrameDuration() const throw();
    double GetFrameRate() const throw();

    // State
    bool IsEndOfStream() const throw();
    bool HasMoreSamples() const throw();

    // Statistics
    DWORD GetFramesDelivered() const throw();
    DWORD GetFramesSkipped() const throw();

    // Configuration
    HRESULT SetTargetFrameRate(double dblFrameRate);
    void SetDropDuplicateFrames(bool fDrop);
    void SetMaintainTimestampOrder(bool fMaintain);

private:
    SyncVideoSourceDesc m_desc;
    bool                m_fInitialized;
    bool                m_fEndOfStream;
    DWORD               m_dwFrameCount;
    DWORD               m_dwCurrentFrameIndex;
    DWORD               m_dwFramesDelivered;
    DWORD               m_dwFramesSkipped;
    LONGLONG            m_llCurrentPositionHns;
    LONGLONG            m_llFrameDurationHns;
    LONGLONG            m_llLastTimestampHns;
    double              m_dblFrameRate;

    CComPtr<IMFSample>  m_spPendingSample;

    HRESULT ReadNextFrame(IMFSample** ppSample);
    HRESULT CheckTimestampOrder(IMFSample* pSample);
    bool IsDuplicateFrame(IMFSample* pSample) const;
    void UpdateFrameCount(IMFSample* pSample);
};

} // namespace HMRAVSource

#endif // SYNCVIDEOSAMPLESOURCE_H
