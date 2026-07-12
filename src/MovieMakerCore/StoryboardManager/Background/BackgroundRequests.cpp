/*
 * BackgroundRequests.cpp
 *
 * Implementation of BaseBackgroundRequest and derived concrete request types.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "BackgroundRequests.h"
#include <shlwapi.h>

// ============================================================================
// BaseBackgroundRequest implementation
// ============================================================================

BaseBackgroundRequest::BaseBackgroundRequest()
    : m_dwRequestId(0)
    , m_requestType(RequestTypeUnknown)
    , m_priority(RequestPriorityDefault)
    , m_status(RequestStatusPending)
    , m_bCancelled(false)
    , m_bPersistent(false)
    , m_flProgress(0.0f)
    , m_hrResult(S_OK)
{
}

BaseBackgroundRequest::~BaseBackgroundRequest()
{
}

void BaseBackgroundRequest::Cancel()
{
    m_bCancelled = true;
    m_status = RequestStatusCancelled;
}

void BaseBackgroundRequest::OnCompleted(HRESULT hr)
{
    UNREFERENCED_PARAMETER(hr);
}

void BaseBackgroundRequest::SetProgress(float flProgress)
{
    m_flProgress = max(0.0f, min(1.0f, flProgress));
}

float BaseBackgroundRequest::GetProgress() const throw()
{
    return m_flProgress;
}

RequestStatus BaseBackgroundRequest::GetStatus() const throw()
{
    return m_status;
}

bool BaseBackgroundRequest::IsCancelled() const throw()
{
    return m_bCancelled;
}

bool BaseBackgroundRequest::IsCompleted() const throw()
{
    return (m_status == RequestStatusCompleted ||
            m_status == RequestStatusFailed ||
            m_status == RequestStatusCancelled);
}

DWORD BaseBackgroundRequest::GetRequestId() const throw()
{
    return m_dwRequestId;
}

RequestType BaseBackgroundRequest::GetRequestType() const throw()
{
    return m_requestType;
}

// ============================================================================
// TranscodeBackgroundRequest implementation
// ============================================================================

TranscodeBackgroundRequest::TranscodeBackgroundRequest(DWORD dwExtentId, DWORD dwMediaId)
    : BaseBackgroundRequest()
    , m_dwExtentId(dwExtentId)
    , m_dwMediaId(dwMediaId)
    , m_uOutputWidth(1920)
    , m_uOutputHeight(1080)
    , m_dwFrameRate(30000)
    , m_dwBitRate(8000000)
{
    m_requestType = RequestTypeTranscode;
    m_priority = RequestPriorityHigh;
}

TranscodeBackgroundRequest::~TranscodeBackgroundRequest()
{
}

HRESULT TranscodeBackgroundRequest::Execute()
{
    if (m_bCancelled)
        return E_ABORT;

    if (m_strOutputPath.IsEmpty())
        return E_INVALIDARG;

    // Prepare the Media Foundation transcode profile
    // and execute the transcode pipeline.
    //
    // In the full implementation, this would:
    //  1. Create an MFTranscodeProfile
    //  2. Configure audio/video encoding parameters
    //  3. Open the source media using IMFSourceReader
    //  4. Create a sink writer to the output path
    //  5. Process samples in a loop, checking cancellation
    //  6. Report progress via SetProgress()
    //
    // For now, return S_OK as a stub.

    m_flProgress = 1.0f;
    return S_OK;
}

void TranscodeBackgroundRequest::SetOutputPath(LPCWSTR pszPath)
{
    m_strOutputPath = pszPath ? pszPath : L"";
}

ATL::CString TranscodeBackgroundRequest::GetOutputPath() const
{
    return m_strOutputPath;
}

void TranscodeBackgroundRequest::SetOutputWidth(UINT uWidth)
{
    m_uOutputWidth = uWidth;
}

UINT TranscodeBackgroundRequest::GetOutputWidth() const throw()
{
    return m_uOutputWidth;
}

void TranscodeBackgroundRequest::SetOutputHeight(UINT uHeight)
{
    m_uOutputHeight = uHeight;
}

UINT TranscodeBackgroundRequest::GetOutputHeight() const throw()
{
    return m_uOutputHeight;
}

void TranscodeBackgroundRequest::SetFrameRate(DWORD dwFrameRate)
{
    m_dwFrameRate = dwFrameRate;
}

DWORD TranscodeBackgroundRequest::GetFrameRate() const throw()
{
    return m_dwFrameRate;
}

void TranscodeBackgroundRequest::SetBitRate(DWORD dwBitRate)
{
    m_dwBitRate = dwBitRate;
}

DWORD TranscodeBackgroundRequest::GetBitRate() const throw()
{
    return m_dwBitRate;
}

DWORD TranscodeBackgroundRequest::GetExtentId() const throw()
{
    return m_dwExtentId;
}

DWORD TranscodeBackgroundRequest::GetMediaId() const throw()
{
    return m_dwMediaId;
}

// ============================================================================
// MediaLoadBackgroundRequest implementation
// ============================================================================

MediaLoadBackgroundRequest::MediaLoadBackgroundRequest(LPCWSTR pszFilePath)
    : BaseBackgroundRequest()
    , m_llDurationHns(0)
    , m_uWidth(0)
    , m_uHeight(0)
    , m_dwFrameRate(0)
    , m_dwMediaType(0)
    , m_pThumbnail(nullptr)
{
    m_requestType = RequestTypeMediaLoad;
    m_priority = RequestPriorityDefault;
    m_strFilePath = pszFilePath ? pszFilePath : L"";
}

MediaLoadBackgroundRequest::~MediaLoadBackgroundRequest()
{
    delete m_pThumbnail;
    m_pThumbnail = nullptr;
}

HRESULT MediaLoadBackgroundRequest::Execute()
{
    if (m_bCancelled)
        return E_ABORT;

    if (m_strFilePath.IsEmpty())
        return E_INVALIDARG;

    // In the full implementation, this would:
    //  1. Create an IMFSourceReader from the file path
    //  2. Read media type attributes (duration, dimensions, frame rate)
    //  3. Generate a thumbnail frame using IMFMediaSource
    //  4. Store results and report progress
    //
    // For now, return S_OK as a stub.

    m_flProgress = 1.0f;
    return S_OK;
}

ATL::CString MediaLoadBackgroundRequest::GetFilePath() const
{
    return m_strFilePath;
}

LONGLONG MediaLoadBackgroundRequest::GetDurationHns() const throw()
{
    return m_llDurationHns;
}

UINT MediaLoadBackgroundRequest::GetWidth() const throw()
{
    return m_uWidth;
}

UINT MediaLoadBackgroundRequest::GetHeight() const throw()
{
    return m_uHeight;
}

DWORD MediaLoadBackgroundRequest::GetFrameRate() const throw()
{
    return m_dwFrameRate;
}

DWORD MediaLoadBackgroundRequest::GetMediaType() const throw()
{
    return m_dwMediaType;
}

Gdiplus::Bitmap* MediaLoadBackgroundRequest::GetThumbnail() const
{
    return m_pThumbnail;
}

void MediaLoadBackgroundRequest::SetThumbnail(Gdiplus::Bitmap* pBitmap)
{
    delete m_pThumbnail;
    m_pThumbnail = pBitmap;
}

// ============================================================================
// SceneMergeContext implementation
// ============================================================================

SceneMergeContext::SceneMergeContext()
    : m_dwMergeType(0)
    , m_hrResult(S_OK)
{
}

SceneMergeContext::~SceneMergeContext()
{
}

void SceneMergeContext::SetMergeType(DWORD dwType)
{
    m_dwMergeType = dwType;
}

DWORD SceneMergeContext::GetMergeType() const throw()
{
    return m_dwMergeType;
}

void SceneMergeContext::AddExtentId(DWORD dwExtentId)
{
    m_vExtentIds.push_back(dwExtentId);
}

void SceneMergeContext::RemoveExtentId(DWORD dwExtentId)
{
    auto it = std::find(m_vExtentIds.begin(), m_vExtentIds.end(), dwExtentId);
    if (it != m_vExtentIds.end())
        m_vExtentIds.erase(it);
}

void SceneMergeContext::ClearExtentIds()
{
    m_vExtentIds.clear();
}

size_t SceneMergeContext::GetExtentIdCount() const throw()
{
    return m_vExtentIds.size();
}

DWORD SceneMergeContext::GetExtentIdAt(size_t nIndex) const
{
    if (nIndex < m_vExtentIds.size())
        return m_vExtentIds[nIndex];
    return 0;
}

void SceneMergeContext::SetMergeParam(LPCWSTR pszKey, LPCWSTR pszValue)
{
    if (pszKey)
        m_mergeParams[pszKey] = pszValue ? pszValue : L"";
}

ATL::CString SceneMergeContext::GetMergeParam(LPCWSTR pszKey) const
{
    auto it = m_mergeParams.find(pszKey);
    if (it != m_mergeParams.end())
        return it->second;
    return L"";
}

void SceneMergeContext::SetUndoSnapshot(IStream* pStream)
{
    m_spUndoSnapshot = pStream;
}

IStream* SceneMergeContext::GetUndoSnapshot() const
{
    return m_spUndoSnapshot;
}

HRESULT SceneMergeContext::GetResult() const throw()
{
    return m_hrResult;
}

void SceneMergeContext::SetResult(HRESULT hr) throw()
{
    m_hrResult = hr;
}

// ============================================================================
// SceneMergeBackgroundRequest implementation
// ============================================================================

SceneMergeBackgroundRequest::SceneMergeBackgroundRequest(SceneMergeContext* pContext)
    : BaseBackgroundRequest()
    , m_pContext(pContext)
{
    m_requestType = RequestTypeSceneMerge;
    m_priority = RequestPriorityDefault;
}

SceneMergeBackgroundRequest::~SceneMergeBackgroundRequest()
{
    delete m_pContext;
    m_pContext = nullptr;
}

HRESULT SceneMergeBackgroundRequest::Execute()
{
    if (m_bCancelled)
        return E_ABORT;

    if (!m_pContext)
        return E_POINTER;

    // Execute the scene merge operation:
    //  1. Apply accumulated scene graph changes
    //  2. Update extent positions in the timeline
    //  3. Adjust transitions and effects as needed
    //  4. Store the result in the context
    //
    // For now, return S_OK as a stub.

    m_flProgress = 1.0f;
    return S_OK;
}

SceneMergeContext* SceneMergeBackgroundRequest::GetContext() const
{
    return m_pContext;
}
