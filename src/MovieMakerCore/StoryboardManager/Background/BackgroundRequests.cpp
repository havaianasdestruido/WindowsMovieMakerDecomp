#include "pch.h"
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
    m_flProgress = std::max(0.0f, std::min(1.0f, flProgress));
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

    HRESULT hr = MFStartup(MF_VERSION);
    if (FAILED(hr))
        return hr;

    CComPtr<IMFSinkWriter> spSinkWriter;
    CComPtr<IMFAttributes> spSinkAttributes;
    hr = MFCreateAttributes(&spSinkAttributes, 2);
    if (FAILED(hr))
        {
        MFShutdown();
        return hr;
        }

    hr = MFCreateSinkWriterFromURL(m_strOutputPath, NULL, spSinkAttributes, &spSinkWriter);
    if (FAILED(hr))
    {
        MFShutdown();
        return hr;
    }

    CComPtr<IMFAttributes> spSourceAttributes;
    hr = MFCreateAttributes(&spSourceAttributes, 2);
    if (FAILED(hr))
    {
        MFShutdown();
        return hr;
    }

    CComPtr<IMFSourceReader> spSourceReader;
    hr = MFCreateSourceReaderFromURL(m_strOutputPath, spSourceAttributes, &spSourceReader);
    if (FAILED(hr))
    {
        MFShutdown();
        return hr;
    }

    DWORD dwVideoStreamIndex = 0;
    DWORD dwAudioStreamIndex = 1;
    bool fHasVideo = false;
    bool fHasAudio = false;

    CComPtr<IMFMediaType> spNativeVideoType;
    hr = spSourceReader->GetNativeMediaType(
        MF_SOURCE_READER_FIRST_VIDEO_STREAM,
        0, &spNativeVideoType);
    fHasVideo = SUCCEEDED(hr);

    CComPtr<IMFMediaType> spNativeAudioType;
    hr = spSourceReader->GetNativeMediaType(
        MF_SOURCE_READER_FIRST_AUDIO_STREAM,
        0, &spNativeAudioType);
    fHasAudio = SUCCEEDED(hr);

    if (!fHasVideo && !fHasAudio)
    {
        MFShutdown();
        return E_FAIL;
    }

    DWORD dwVideoOutIndex = 0;
    DWORD dwAudioOutIndex = 0;

    if (fHasVideo)
    {
        CComPtr<IMFMediaType> spVideoOutType;
        hr = MFCreateMediaType(&spVideoOutType);
        if (SUCCEEDED(hr))
        {
            spVideoOutType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
            spVideoOutType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_H264);
            spVideoOutType->SetUINT32(MF_MT_AVG_BITRATE, m_dwBitRate);
            MFSetAttributeSize(spVideoOutType, MF_MT_FRAME_SIZE, m_uOutputWidth, m_uOutputHeight);
            MFSetAttributeRatio(spVideoOutType, MF_MT_FRAME_RATE, m_dwFrameRate, 1);
            spVideoOutType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
            spVideoOutType->SetUINT32(MF_MT_MPEG2_PROFILE, 100);

            hr = spSinkWriter->AddStream(spVideoOutType, &dwVideoOutIndex);
        }

        if (SUCCEEDED(hr))
        {
            hr = spSinkWriter->SetInputMediaType(dwVideoOutIndex, spNativeVideoType, NULL);
        }
    }

    if (fHasAudio && SUCCEEDED(hr))
    {
        CComPtr<IMFMediaType> spAudioOutType;
        hr = MFCreateMediaType(&spAudioOutType);
        if (SUCCEEDED(hr))
        {
            spAudioOutType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
            spAudioOutType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_AAC);
            spAudioOutType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, 48000);
            spAudioOutType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, 2);
            spAudioOutType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, 16);
            spAudioOutType->SetUINT32(MF_MT_AVG_BITRATE, 192000);

            hr = spSinkWriter->AddStream(spAudioOutType, &dwAudioOutIndex);
        }

        if (SUCCEEDED(hr))
        {
            hr = spSinkWriter->SetInputMediaType(dwAudioOutIndex, spNativeAudioType, NULL);
        }
    }

    if (FAILED(hr))
    {
        MFShutdown();
        return hr;
    }

    hr = spSinkWriter->BeginWriting();
    if (FAILED(hr))
    {
        MFShutdown();
        return hr;
    }

    LONGLONG llTotalDuration = 0;

    CComPtr<IMFAttributes> spSrcAttribs;
    if (SUCCEEDED(spSourceReader->GetServiceForStream(
            MF_SOURCE_READER_MEDIASOURCE,
            GUID_NULL, IID_PPV_ARGS(&spSrcAttribs))))
    {
        UINT64 cbDuration = 0;
        if (SUCCEEDED(spSrcAttribs->GetUINT64(MF_PD_DURATION, &cbDuration)))
            llTotalDuration = static_cast<LONGLONG>(cbDuration);
    }

    DWORD dwFrameCount = 0;
    bool fDone = false;

    while (!fDone && !m_bCancelled)
    {
        DWORD dwStreamIndex = 0;
        DWORD dwStreamFlags = 0;
        LONGLONG llTimestamp = 0;
        CComPtr<IMFSample> spSample;

        hr = spSourceReader->ReadSample(
            MF_SOURCE_READER_ALL_STREAMS,
            0,
            &dwStreamIndex,
            &dwStreamFlags,
            &llTimestamp,
            &spSample);

        if (FAILED(hr))
            break;

        if (dwStreamFlags & MF_SOURCE_READERF_ENDOFSTREAM)
        {
            fDone = true;
            break;
        }

        if (!spSample)
            continue;

        DWORD dwWriterStreamIndex = dwStreamIndex;
        if (dwStreamIndex == MF_SOURCE_READER_FIRST_VIDEO_STREAM)
            dwWriterStreamIndex = dwVideoOutIndex;
        else if (dwStreamIndex == MF_SOURCE_READER_FIRST_AUDIO_STREAM)
            dwWriterStreamIndex = dwAudioOutIndex;

        hr = spSinkWriter->WriteSample(dwWriterStreamIndex, spSample);
        if (FAILED(hr))
            break;

        if (llTotalDuration > 0 && llTimestamp > 0)
        {
            float flProgress = static_cast<float>(llTimestamp) /
                               static_cast<float>(llTotalDuration);
            SetProgress(std::min(1.0f, flProgress));
        }

        if (dwStreamIndex == MF_SOURCE_READER_FIRST_VIDEO_STREAM)
            dwFrameCount++;
    }

    if (!m_bCancelled)
    {
        hr = spSinkWriter->Finalize();
    }
    else
    {
        hr = E_ABORT;
    }

    MFShutdown();

    if (SUCCEEDED(hr))
        m_flProgress = 1.0f;

    return hr;
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

    HRESULT hr = MFStartup(MF_VERSION);
    if (FAILED(hr))
        return hr;

    CComPtr<IMFAttributes> spAttributes;
    hr = MFCreateAttributes(&spAttributes, 2);
    if (FAILED(hr))
    {
        MFShutdown();
        return hr;
    }

    CComPtr<IMFSourceReader> spReader;
    hr = MFCreateSourceReaderFromURL(m_strFilePath, spAttributes, &spReader);
    if (FAILED(hr))
    {
        MFShutdown();
        return hr;
    }

    SetProgress(0.2f);
    if (m_bCancelled)
    {
        MFShutdown();
        return E_ABORT;
    }

    CComPtr<IMFMediaType> spNativeType;
    hr = spReader->GetNativeMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, &spNativeType);
    if (SUCCEEDED(hr))
    {
        MFGetAttributeSize(spNativeType, MF_MT_FRAME_SIZE, &m_uWidth, &m_uHeight);

        UINT32 unum = 0, uden = 1;
        if (SUCCEEDED(MFGetAttributeRatio(spNativeType, MF_MT_FRAME_RATE, &unum, &uden)))
        {
            if (uden > 0)
                m_dwFrameRate = unum / uden;
        }

        m_dwMediaType = 1;
    }
    else
    {
        CComPtr<IMFMediaType> spAudioType;
        hr = spReader->GetNativeMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, &spAudioType);
        if (SUCCEEDED(hr))
        {
            m_dwMediaType = 2;
            hr = S_OK;
        }
    }

    SetProgress(0.5f);
    if (m_bCancelled)
    {
        MFShutdown();
        return E_ABORT;
    }

    UINT64 ullDuration = 0;
    CComPtr<IMFSourceReader> spReaderForDuration;
    hr = MFCreateSourceReaderFromURL(m_strFilePath, spAttributes, &spReaderForDuration);
    if (SUCCEEDED(hr))
    {
        hr = spReaderForDuration->GetPresentationAttribute(
            MF_SOURCE_READER_MEDIASOURCE,
            MF_PD_DURATION,
            NULL);
        if (SUCCEEDED(hr))
        {
            PROPVARIANT varDuration;
            PropVariantInit(&varDuration);
            hr = spReaderForDuration->GetPresentationAttribute(
                MF_SOURCE_READER_MEDIASOURCE,
                MF_PD_DURATION,
                &varDuration);
            if (SUCCEEDED(hr))
            {
                ullDuration = varDuration.uhVal.QuadPart;
                PropVariantClear(&varDuration);
            }
        }
    }
    m_llDurationHns = static_cast<LONGLONG>(ullDuration);

    SetProgress(0.7f);
    if (m_bCancelled)
    {
        MFShutdown();
        return E_ABORT;
    }

    if (m_dwMediaType == 1 && m_uWidth > 0 && m_uHeight > 0)
    {
        LONGLONG llThumbPosition = m_llDurationHns / 10;
        if (llThumbPosition < 0)
            llThumbPosition = 0;

        PROPVARIANT varPosition;
        PropVariantInit(&varPosition);
        varPosition.vt = VT_I8;
        varPosition.hVal.QuadPart = llThumbPosition;
        hr = spReader->SetCurrentPosition(GUID_NULL, varPosition);
        PropVariantClear(&varPosition);

        if (SUCCEEDED(hr))
        {
            DWORD dwStreamIndex = 0;
            DWORD dwStreamFlags = 0;
            LONGLONG llTimestamp = 0;
            CComPtr<IMFSample> spVideoSample;

            hr = spReader->ReadSample(
                MF_SOURCE_READER_FIRST_VIDEO_STREAM,
                0,
                &dwStreamIndex,
                &dwStreamFlags,
                &llTimestamp,
                &spVideoSample);

            if (SUCCEEDED(hr) && spVideoSample)
            {
                CComPtr<IMFMediaBuffer> spBuffer;
                hr = spVideoSample->ConvertToContiguousBuffer(&spBuffer);
                if (SUCCEEDED(hr))
                {
                    BYTE* pbData = NULL;
                    DWORD cbData = 0;
                    hr = spBuffer->Lock(&pbData, NULL, &cbData);
                    if (SUCCEEDED(hr))
                    {
                        m_pThumbnail = new Gdiplus::Bitmap(m_uWidth, m_uHeight, PixelFormat32bppARGB);

                        Gdiplus::BitmapData bmpData;
                        Gdiplus::Rect rcLock(0, 0, m_uWidth, m_uHeight);
                        Gdiplus::Status gs = m_pThumbnail->LockBits(
                            &rcLock,
                            Gdiplus::ImageLockModeWrite,
                            PixelFormat32bppARGB,
                            &bmpData);

                        if (gs == Gdiplus::Ok)
                        {
                            DWORD cbRow = m_uWidth * 4;
                            INT cbStride = bmpData.Stride;
                            if (cbStride < 0) cbStride = -cbStride;
                            DWORD cbMinRow = (cbRow < static_cast<DWORD>(cbStride)) ?
                                cbRow : static_cast<DWORD>(cbStride);
                            const BYTE* pSrc = pbData;
                            BYTE* pDst = static_cast<BYTE*>(bmpData.Scan0);

                            for (UINT row = 0; row < m_uHeight; row++)
                            {
                                CopyMemory(pDst, pSrc, cbMinRow);
                                pSrc += cbRow;
                                pDst += bmpData.Stride;
                            }

                            m_pThumbnail->UnlockBits(&bmpData);
                        }

                        spBuffer->Unlock();
                    }
                }
            }
        }
    }

    MFShutdown();

    SetProgress(1.0f);
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

    size_t cExtents = m_pContext->GetExtentIdCount();
    if (cExtents == 0)
    {
        m_pContext->SetResult(S_FALSE);
        m_flProgress = 1.0f;
        return S_FALSE;
    }

    DWORD dwMergeType = m_pContext->GetMergeType();
    bool fSuccess = true;

    SetProgress(0.0f);

    for (size_t i = 0; i < cExtents; i++)
    {
        if (m_bCancelled)
        {
            m_pContext->SetResult(E_ABORT);
            return E_ABORT;
        }

        DWORD dwExtentId = m_pContext->GetExtentIdAt(i);

        WCHAR szKey[64];
        StringCchPrintfW(szKey, _countof(szKey), L"extent_%u_position", dwExtentId);
        CString strPosition = m_pContext->GetMergeParam(szKey);

        StringCchPrintfW(szKey, _countof(szKey), L"extent_%u_transition", dwExtentId);
        CString strTransition = m_pContext->GetMergeParam(szKey);

        UNREFERENCED_PARAMETER(dwMergeType);
        UNREFERENCED_PARAMETER(strPosition);
        UNREFERENCED_PARAMETER(strTransition);

        float flProgress = static_cast<float>(i + 1) /
                           static_cast<float>(cExtents);
        SetProgress(flProgress);
    }

    if (fSuccess)
    {
        IStream* pUndoStream = m_pContext->GetUndoSnapshot();
        UNREFERENCED_PARAMETER(pUndoStream);
    }

    SetProgress(1.0f);

    HRESULT hrResult = fSuccess ? S_OK : E_FAIL;
    m_pContext->SetResult(hrResult);
    return hrResult;
}

SceneMergeContext* SceneMergeBackgroundRequest::GetContext() const
{
    return m_pContext;
}
