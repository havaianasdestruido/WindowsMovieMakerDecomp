/*
 * TranscodeMetadata.h
 *
 * TranscodeMetadata - Metadata parsing and transcode progress tracking
 * for the export pipeline.
 *
 * TranscodeMetadataParser - Parse metadata from media files during
 * transcode operations (title, artist, date, duration, etc.).
 *
 * TranscodeProcess - Track transcode progress, state, and provide
 * cancellation and pause/resume support.
 *
 * RTTI: ?AVTranscodeMetadataParser@@, ?AVTranscodeProcess@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef TRANSCODEMETADATA_H
#define TRANSCODEMETADATA_H

#include "HMRAVSource.h"

namespace HMRAVSource
{

// ============================================================================
// MediaMetadata
// ============================================================================
struct MediaMetadata
{
    ATL::CString    strTitle;
    ATL::CString    strArtist;
    ATL::CString    strAlbum;
    ATL::CString    strGenre;
    ATL::CString    strDescription;
    ATL::CString    strCopyright;
    ATL::CString    strRating;
    ATL::CString    strDateTaken;
    ATL::CString    strSoftware;
    LONGLONG        llDurationHns;
    DWORD           dwWidth;
    DWORD           dwHeight;
    double          dblFrameRate;

    MediaMetadata()
        : llDurationHns(0)
        , dwWidth(0)
        , dwHeight(0)
        , dblFrameRate(0.0)
    {
    }
};

// ============================================================================
// TranscodeProcessState
// ============================================================================
enum TranscodeProcessState
{
    TranscodeProcessIdle        = 0,
    TranscodeProcessPreparing   = 1,
    TranscodeProcessEncoding    = 2,
    TranscodeProcessFinalizing  = 3,
    TranscodeProcessComplete    = 4,
    TranscodeProcessFailed      = 5,
    TranscodeProcessCancelled   = 6,
    TranscodeProcessPaused      = 7
};

// ============================================================================
// TranscodeProcessProgress
// ============================================================================
struct TranscodeProcessProgress
{
    TranscodeProcessState   state;
    float                   fPercentComplete;
    LONGLONG                llCurrentPositionHns;
    LONGLONG                llTotalDurationHns;
    DWORD                   dwFramesEncoded;
    DWORD                   dwTotalFrames;
    double                  dblEncodingFps;
    double                  dblElapsedSeconds;
    double                  dblEstimatedRemaining;
    LONGLONG                llOutputSizeBytes;
    HRESULT                 hrLastError;
    bool                    fComplete;

    TranscodeProcessProgress()
        : state(TranscodeProcessIdle)
        , fPercentComplete(0.0f)
        , llCurrentPositionHns(0)
        , llTotalDurationHns(0)
        , dwFramesEncoded(0)
        , dwTotalFrames(0)
        , dblEncodingFps(0.0)
        , dblElapsedSeconds(0.0)
        , dblEstimatedRemaining(0.0)
        , llOutputSizeBytes(0)
        , hrLastError(S_OK)
        , fComplete(false)
    {
    }
};

// ============================================================================
// TranscodeMetadataParser
// ============================================================================
// Parses metadata from media files during transcode operations. Extracts
// properties like title, artist, duration, and resolution from the source
// file and can embed them into the output file.
//
class AVSOURCE_API TranscodeMetadataParser
{
public:
    TranscodeMetadataParser();
    ~TranscodeMetadataParser();

    // Lifecycle
    HRESULT Initialize();
    HRESULT Shutdown();

    // Metadata parsing
    HRESULT ParseMetadata(LPCWSTR pszFilePath, MediaMetadata* pMetadata);
    HRESULT ParseMetadataFromSource(IMFSourceReader* pReader, MediaMetadata* pMetadata);
    HRESULT ParseMetadataFromSample(IMFSample* pSample, MediaMetadata* pMetadata);

    // Individual property queries
    HRESULT GetTitle(LPCWSTR pszFilePath, ATL::CString* pstrTitle);
    HRESULT GetArtist(LPCWSTR pszFilePath, ATL::CString* pstrArtist);
    HRESULT GetDuration(LPCWSTR pszFilePath, LONGLONG* pllDurationHns);
    HRESULT GetResolution(LPCWSTR pszFilePath, DWORD* pdwWidth, DWORD* pdwHeight);

    // Metadata writing (for output files)
    HRESULT WriteMetadata(LPCWSTR pszFilePath, const MediaMetadata& metadata);
    HRESULT SetMetadataAttribute(LPCWSTR pszFilePath, REFGUID guidKey, LPCWSTR pszValue);

    // Thumbnail extraction
    HRESULT ExtractThumbnail(LPCWSTR pszFilePath, BYTE** ppData, DWORD* pcbData, GUID* pFormat);

    // Property system access
    static HRESULT GetPropertyString(LPCWSTR pszFilePath, REFPROPERTYKEY key, ATL::CString* pstrValue);
    static HRESULT GetPropertyInt(LPCWSTR pszFilePath, REFPROPERTYKEY key, INT* piValue);
    static HRESULT GetPropertyDateTime(LPCWSTR pszFilePath, REFPROPERTYKEY key, SYSTEMTIME* pSystemTime);

private:
    bool    m_fInitialized;

    HRESULT ParseFromPropertyStore(LPCWSTR pszFilePath, MediaMetadata* pMetadata);
    HRESULT ParseFromMFAttributes(IMFAttributes* pAttributes, MediaMetadata* pMetadata);
};

// ============================================================================
// TranscodeProcess
// ============================================================================
// Tracks the progress and state of a transcode operation. Provides
// progress reporting, cancellation support, and state management for
// the transcode pipeline.
//
class AVSOURCE_API TranscodeProcess
{
public:
    TranscodeProcess();
    ~TranscodeProcess();

    // Lifecycle
    HRESULT Initialize(LPCWSTR pszInputPath, LPCWSTR pszOutputPath);
    HRESULT Shutdown();

    // State management
    HRESULT BeginTranscode();
    HRESULT CompleteTranscode();
    HRESULT FailTranscode(HRESULT hrError);
    HRESULT CancelTranscode();
    HRESULT PauseTranscode();
    HRESULT ResumeTranscode();

    // State query
    TranscodeProcessState GetState() const throw();
    bool IsComplete() const throw();
    bool IsFailed() const throw();
    bool IsCancelled() const throw();
    bool IsPaused() const throw();
    bool IsEncoding() const throw();

    // Progress
    TranscodeProcessProgress GetProgress() const;
    HRESULT UpdateProgress(LONGLONG llCurrentPositionHns);
    HRESULT SetTotalDuration(LONGLONG llTotalDurationHns);
    HRESULT SetTotalFrameCount(DWORD dwTotalFrames);
    HRESULT SetOutputSize(LONGLONG llSizeBytes);

    // Frame tracking
    HRESULT IncrementFramesEncoded();
    HRESULT SetEncodingFps(double dblFps);

    // Timing
    HRESULT StartTimer();
    double GetElapsedSeconds() const throw();
    double GetEstimatedRemaining() const throw();

    // Input/output paths
    ATL::CString GetInputPath() const;
    ATL::CString GetOutputPath() const;

    // Callbacks
    using ProgressCallback = std::function<void(const TranscodeProcessProgress&)>;
    using StateChangeCallback = std::function<void(TranscodeProcessState, TranscodeProcessState)>;
    using ErrorCallback = std::function<void(HRESULT, LPCWSTR)>;

    void SetProgressCallback(ProgressCallback cb) { m_progressCb = cb; }
    void SetStateChangeCallback(StateChangeCallback cb) { m_stateChangeCb = cb; }
    void SetErrorCallback(ErrorCallback cb) { m_errorCb = cb; }

    // Cancel request check
    bool IsCancellationRequested() const throw();

private:
    TranscodeProcessState       m_state;
    TranscodeProcessProgress    m_progress;
    ATL::CString                m_strInputPath;
    ATL::CString                m_strOutputPath;
    bool                        m_fCancellationRequested;

    LONGLONG                    m_llStartTimeHns;
    LONGLONG                    m_llPauseTimeHns;
    LONGLONG                    m_llTotalPauseDurationHns;

    ProgressCallback            m_progressCb;
    StateChangeCallback         m_stateChangeCb;
    ErrorCallback               m_errorCb;

    void SetState(TranscodeProcessState newState);
    void FireProgress();
    void FireStateChange(TranscodeProcessState oldState, TranscodeProcessState newState);
    LONGLONG GetCurrentTimeHns() const;
};

} // namespace HMRAVSource

#endif // TRANSCODEMETADATA_H
