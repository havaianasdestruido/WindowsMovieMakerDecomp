/*
 * BackgroundRequests.h
 *
 * Abstract base class and concrete implementations for background processing
 * requests within MovieMakerCore. Supports priority queuing, cancellation
 * tokens, and progress reporting through the BackgroundOrchestrator.
 *
 * RTTI classes:
 *   ?AVBaseBackgroundRequest@@
 *   ?AVTranscodeBackgroundRequest@@
 *   ?AVMediaLoadBackgroundRequest@@
 *   ?AVSceneMergeBackgroundRequest@@
 *   ?AVSceneMergeContext@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once
#ifndef BACKGROUNDREQUESTS_H
#define BACKGROUNDREQUESTS_H

#include "../../pch.h"

// ============================================================================
// RequestType enum
// ============================================================================
enum RequestType
{
    RequestTypeUnknown           = 0,
    RequestTypeTranscode         = 1,
    RequestTypeMediaLoad         = 2,
    RequestTypeSceneMerge        = 3,
    RequestTypeAutofit           = 4,
    RequestTypeThumbnail         = 5,
    RequestTypeAreaOfInterest    = 6
};

// ============================================================================
// RequestPriority enum
// ============================================================================
enum RequestPriority
{
    RequestPriorityLowest        = 0,
    RequestPriorityLow           = 1,
    RequestPriorityDefault       = 2,
    RequestPriorityHigh          = 3,
    RequestPriorityHighest       = 4,
    RequestPriorityCritical      = 5
};

// ============================================================================
// RequestStatus enum
// ============================================================================
enum RequestStatus
{
    RequestStatusNone            = 0,
    RequestStatusPending         = 1,
    RequestStatusInProgress      = 2,
    RequestStatusCompleted       = 3,
    RequestStatusFailed          = 4,
    RequestStatusCancelled       = 5
};

// ============================================================================
// BaseBackgroundRequest
// ============================================================================
// Abstract base class for all background requests processed by the
// BackgroundOrchestrator. Provides priority, status, cancellation token,
// and progress callback infrastructure.
//
class BaseBackgroundRequest
{
public:
    BaseBackgroundRequest();
    virtual ~BaseBackgroundRequest();

    // Execute the request (called by worker thread)
    virtual HRESULT Execute() = 0;

    // Cancel the request
    virtual void Cancel();

    // Notification called after request completes
    virtual void OnCompleted(HRESULT hr);

    // Progress reporting (0.0 to 1.0)
    void SetProgress(float flProgress);
    float GetProgress() const throw();

    // Status
    RequestStatus GetStatus() const throw();
    bool IsCancelled() const throw();
    bool IsCompleted() const throw();

    // Identity
    DWORD GetRequestId() const throw();
    RequestType GetRequestType() const throw();

protected:
    friend class BackgroundOrchestrator;

    DWORD           m_dwRequestId;
    RequestType     m_requestType;
    RequestPriority m_priority;
    RequestStatus   m_status;
    bool            m_bCancelled;
    bool            m_bPersistent;
    float           m_flProgress;
    HRESULT         m_hrResult;

private:
    BaseBackgroundRequest(const BaseBackgroundRequest&);
    BaseBackgroundRequest& operator=(const BaseBackgroundRequest&);
};

// ============================================================================
// TranscodeBackgroundRequest
// ============================================================================
// Background request to transcode a media extent into a proxy video format
// suitable for timeline playback. Runs the Media Foundation transcode pipeline
// on a background thread.
//
class TranscodeBackgroundRequest : public BaseBackgroundRequest
{
public:
    TranscodeBackgroundRequest(DWORD dwExtentId, DWORD dwMediaId);
    ~TranscodeBackgroundRequest();

    HRESULT Execute() override;

    // Transcode configuration
    void SetOutputPath(LPCWSTR pszPath);
    ATL::CString GetOutputPath() const;

    void SetOutputWidth(UINT uWidth);
    UINT GetOutputWidth() const throw();

    void SetOutputHeight(UINT uHeight);
    UINT GetOutputHeight() const throw();

    void SetFrameRate(DWORD dwFrameRate);
    DWORD GetFrameRate() const throw();

    void SetBitRate(DWORD dwBitRate);
    DWORD GetBitRate() const throw();

    // Source settings
    DWORD GetExtentId() const throw();
    DWORD GetMediaId() const throw();

private:
    DWORD        m_dwExtentId;
    DWORD        m_dwMediaId;
    ATL::CString m_strOutputPath;
    UINT         m_uOutputWidth;
    UINT         m_uOutputHeight;
    DWORD        m_dwFrameRate;
    DWORD        m_dwBitRate;
};

// ============================================================================
// MediaLoadBackgroundRequest
// ============================================================================
// Background request to load media metadata (duration, dimensions, codec info)
// and generate a thumbnail frame. Used when adding media items to the project.
//
class MediaLoadBackgroundRequest : public BaseBackgroundRequest
{
public:
    MediaLoadBackgroundRequest(LPCWSTR pszFilePath);
    ~MediaLoadBackgroundRequest();

    HRESULT Execute() override;

    // Results
    ATL::CString GetFilePath() const;

    LONGLONG GetDurationHns() const throw();
    UINT     GetWidth() const throw();
    UINT     GetHeight() const throw();
    DWORD    GetFrameRate() const throw();
    DWORD    GetMediaType() const throw();

    Gdiplus::Bitmap* GetThumbnail() const;
    void SetThumbnail(Gdiplus::Bitmap* pBitmap);

private:
    ATL::CString     m_strFilePath;
    LONGLONG         m_llDurationHns;
    UINT             m_uWidth;
    UINT             m_uHeight;
    DWORD            m_dwFrameRate;
    DWORD            m_dwMediaType;

    // Thumbnail (owned)
    Gdiplus::Bitmap* m_pThumbnail;
};

// ============================================================================
// SceneMergeContext
// ============================================================================
// Context state for a scene merge operation. Stores the set of extent IDs
// that are being merged, the merge type, and any pending undo information.
//
class SceneMergeContext
{
public:
    SceneMergeContext();
    ~SceneMergeContext();

    // Merge type identifiers
    void SetMergeType(DWORD dwType);
    DWORD GetMergeType() const throw();

    // Extent IDs involved in the merge
    void AddExtentId(DWORD dwExtentId);
    void RemoveExtentId(DWORD dwExtentId);
    void ClearExtentIds();
    size_t GetExtentIdCount() const throw();
    DWORD GetExtentIdAt(size_t nIndex) const;

    // Merge parameters
    void SetMergeParam(LPCWSTR pszKey, LPCWSTR pszValue);
    ATL::CString GetMergeParam(LPCWSTR pszKey) const;

    // Undo snapshot (serialized state before merge)
    void SetUndoSnapshot(IStream* pStream);
    IStream* GetUndoSnapshot() const;

    // Result
    HRESULT GetResult() const throw();
    void SetResult(HRESULT hr) throw();

private:
    DWORD                        m_dwMergeType;
    std::vector<DWORD>           m_vExtentIds;
    std::map<ATL::CString, ATL::CString> m_mergeParams;
    ATL::CComPtr<IStream>        m_spUndoSnapshot;
    HRESULT                      m_hrResult;
};

// ============================================================================
// SceneMergeBackgroundRequest
// ============================================================================
// Background request to merge a set of scene graph changes into the project.
// Used when applying timeline edits that need to update the X3D scene graph
// without blocking the UI thread.
//
class SceneMergeBackgroundRequest : public BaseBackgroundRequest
{
public:
    SceneMergeBackgroundRequest(SceneMergeContext* pContext);
    ~SceneMergeBackgroundRequest();

    HRESULT Execute() override;

    SceneMergeContext* GetContext() const;

private:
    SceneMergeContext* m_pContext; // owned
};

#endif // BACKGROUNDREQUESTS_H
