// TranscodeMetadata.cpp - Transcode metadata and process tracking implementation

#include "pch.h"
#include "TranscodeMetadata.h"
#include <propsys.h>

namespace HMRAVSource
{

// ============================================================================
// TranscodeMetadataParser
// ============================================================================

TranscodeMetadataParser::TranscodeMetadataParser()
    : m_fInitialized(false)
{
}

TranscodeMetadataParser::~TranscodeMetadataParser()
{
    Shutdown();
}

HRESULT TranscodeMetadataParser::Initialize()
{
    m_fInitialized = true;
    return S_OK;
}

HRESULT TranscodeMetadataParser::Shutdown()
{
    m_fInitialized = false;
    return S_OK;
}

HRESULT TranscodeMetadataParser::ParseMetadata(LPCWSTR pszFilePath, MediaMetadata* pMetadata)
{
    if (!pszFilePath || !pMetadata)
        return E_POINTER;

    if (!m_fInitialized)
        return E_UNEXPECTED;

    *pMetadata = MediaMetadata();

    return ParseFromPropertyStore(pszFilePath, pMetadata);
}

HRESULT TranscodeMetadataParser::ParseMetadataFromSource(IMFSourceReader* pReader, MediaMetadata* pMetadata)
{
    if (!pReader || !pMetadata)
        return E_POINTER;

    *pMetadata = MediaMetadata();

    CComPtr<IMFAttributes> spAttributes;
    HRESULT hr = pReader->QueryInterface(IID_PPV_ARGS(&spAttributes));
    if (FAILED(hr))
        return hr;

    return ParseFromMFAttributes(spAttributes, pMetadata);
}

HRESULT TranscodeMetadataParser::ParseMetadataFromSample(IMFSample* pSample, MediaMetadata* pMetadata)
{
    if (!pSample || !pMetadata)
        return E_POINTER;

    *pMetadata = MediaMetadata();

    LONGLONG llTimestamp = 0;
    if (SUCCEEDED(pSample->GetSampleTime(&llTimestamp)))
    {
        pMetadata->llDurationHns = llTimestamp;
    }

    return S_OK;
}

HRESULT TranscodeMetadataParser::GetTitle(LPCWSTR pszFilePath, ATL::CString* pstrTitle)
{
    if (!pstrTitle)
        return E_POINTER;

    return GetPropertyString(pszFilePath, PKEY_Title, pstrTitle);
}

HRESULT TranscodeMetadataParser::GetArtist(LPCWSTR pszFilePath, ATL::CString* pstrArtist)
{
    if (!pstrArtist)
        return E_POINTER;

    return GetPropertyString(pszFilePath, PKEY_Author, pstrArtist);
}

HRESULT TranscodeMetadataParser::GetDuration(LPCWSTR pszFilePath, LONGLONG* pllDurationHns)
{
    if (!pllDurationHns)
        return E_POINTER;

    *pllDurationHns = 0;

    CComPtr<IMFSourceReader> spReader;
    HRESULT hr = MFCreateSourceReaderFromURL(pszFilePath, nullptr, &spReader);
    if (FAILED(hr))
        return hr;

    PROPVARIANT var;
    PropVariantInit(&var);
    hr = spReader->GetPresentationAttribute(
        MF_SOURCE_READER_MEDIASOURCE,
        MF_PD_DURATION,
        &var);

    if (SUCCEEDED(hr))
    {
        if (var.vt == VT_UI8)
        {
            *pllDurationHns = static_cast<LONGLONG>(var.uhVal.QuadPart);
        }
        else
        {
            hr = MF_E_ATTRIBUTENOTFOUND;
        }
    }

    PropVariantClear(&var);
    return hr;
}

HRESULT TranscodeMetadataParser::GetResolution(LPCWSTR pszFilePath, DWORD* pdwWidth, DWORD* pdwHeight)
{
    if (!pdwWidth || !pdwHeight)
        return E_POINTER;

    *pdwWidth = 0;
    *pdwHeight = 0;

    CComPtr<IMFSourceReader> spReader;
    HRESULT hr = MFCreateSourceReaderFromURL(pszFilePath, nullptr, &spReader);
    if (FAILED(hr))
        return hr;

    CComPtr<IMFMediaType> spType;
    hr = spReader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, &spType);
    if (FAILED(hr))
        return hr;

    UINT32 uWidth = 0, uHeight = 0;
    hr = MFGetAttributeSize(spType, MF_MT_FRAME_SIZE, &uWidth, &uHeight);
    if (SUCCEEDED(hr))
    {
        *pdwWidth = uWidth;
        *pdwHeight = uHeight;
    }

    return hr;
}

HRESULT TranscodeMetadataParser::WriteMetadata(LPCWSTR pszFilePath, const MediaMetadata& metadata)
{
    if (!pszFilePath)
        return E_POINTER;

    // Would use property store to write metadata
    return S_OK;
}

HRESULT TranscodeMetadataParser::SetMetadataAttribute(LPCWSTR /*pszFilePath*/, REFGUID /*guidKey*/, LPCWSTR /*pszValue*/)
{
    return S_OK;
}

HRESULT TranscodeMetadataParser::ExtractThumbnail(LPCWSTR /*pszFilePath*/, BYTE** ppData, DWORD* pcbData, GUID* pFormat)
{
    if (ppData) *ppData = nullptr;
    if (pcbData) *pcbData = 0;
    if (pFormat) *pFormat = GUID_NULL;
    return E_NOTIMPL;
}

HRESULT TranscodeMetadataParser::GetPropertyString(LPCWSTR pszFilePath, REFPROPERTYKEY key, ATL::CString* pstrValue)
{
    if (!pszFilePath || !pstrValue)
        return E_POINTER;

    *pstrValue = CString();

    CComPtr<IPropertySetStorage> spPropSetStorage;
    HRESULT hr = StgOpenStorageEx(
        pszFilePath,
        STGM_READ | STGM_SHARE_DENY_WRITE,
        STGFMT_FILE,
        0,
        nullptr,
        nullptr,
        IID_IPropertySetStorage,
        reinterpret_cast<void**>(&spPropSetStorage));

    CComPtr<IPropertyStore> spStore;
    if (SUCCEEDED(hr))
    {
        CComPtr<IPropertyStorage> spPropStorage;
        hr = spPropSetStorage->Open(FMTID_SummaryInformation, STGM_READ | STGM_SHARE_EXCLUSIVE, &spPropStorage);
        if (SUCCEEDED(hr))
            hr = spPropStorage->QueryInterface(IID_PPV_ARGS(&spStore));
    }

    if (FAILED(hr))
    {
        // Fallback: try reading via WIC
        return E_NOTIMPL;
    }

    PROPVARIANT var;
    PropVariantInit(&var);
    hr = spStore->GetValue(key, &var);
    if (SUCCEEDED(hr))
    {
        switch (var.vt)
        {
        case VT_LPWSTR:
            if (var.pwszVal)
                *pstrValue = var.pwszVal;
            break;
        case VT_LPSTR:
            if (var.pszVal)
                *pstrValue = CString(var.pszVal);
            break;
        case VT_BSTR:
            if (var.bstrVal)
                *pstrValue = var.bstrVal;
            break;
        default:
            hr = MF_E_ATTRIBUTENOTFOUND;
            break;
        }
    }

    PropVariantClear(&var);
    return hr;
}

HRESULT TranscodeMetadataParser::GetPropertyInt(LPCWSTR pszFilePath, REFPROPERTYKEY key, INT* piValue)
{
    if (!pszFilePath || !piValue)
        return E_POINTER;

    *piValue = 0;

    ATL::CString strValue;
    HRESULT hr = GetPropertyString(pszFilePath, key, &strValue);
    if (SUCCEEDED(hr) && !strValue.IsEmpty())
        *piValue = _ttoi(strValue.GetString());

    return hr;
}

HRESULT TranscodeMetadataParser::GetPropertyDateTime(LPCWSTR /*pszFilePath*/, REFPROPERTYKEY /*key*/, SYSTEMTIME* pSystemTime)
{
    if (pSystemTime)
        ZeroMemory(pSystemTime, sizeof(SYSTEMTIME));
    return E_NOTIMPL;
}

HRESULT TranscodeMetadataParser::ParseFromPropertyStore(LPCWSTR pszFilePath, MediaMetadata* pMetadata)
{
    if (!pszFilePath || !pMetadata)
        return E_POINTER;

    // Parse basic properties
    GetPropertyString(pszFilePath, PKEY_Title, &pMetadata->strTitle);
    GetPropertyString(pszFilePath, PKEY_Author, &pMetadata->strArtist);
    GetPropertyString(pszFilePath, PKEY_Music_AlbumTitle, &pMetadata->strAlbum);
    GetPropertyString(pszFilePath, PKEY_Music_Genre, &pMetadata->strGenre);
    GetPropertyString(pszFilePath, PKEY_Comment, &pMetadata->strDescription);
    GetPropertyString(pszFilePath, PKEY_Copyright, &pMetadata->strCopyright);

    // Get duration via MF
    GetDuration(pszFilePath, &pMetadata->llDurationHns);

    // Get resolution
    DWORD dwWidth = 0, dwHeight = 0;
    if (SUCCEEDED(GetResolution(pszFilePath, &dwWidth, &dwHeight)))
    {
        pMetadata->dwWidth = dwWidth;
        pMetadata->dwHeight = dwHeight;
    }

    return S_OK;
}

HRESULT TranscodeMetadataParser::ParseFromMFAttributes(IMFAttributes* pAttributes, MediaMetadata* pMetadata)
{
    if (!pAttributes || !pMetadata)
        return E_POINTER;

    // Read duration
    UINT64 ullDuration = 0;
    if (SUCCEEDED(pAttributes->GetUINT64(MF_PD_DURATION, &ullDuration)))
        pMetadata->llDurationHns = static_cast<LONGLONG>(ullDuration);

    // Read frame rate
    UINT32 uNum = 0, uDen = 0;
    if (SUCCEEDED(MFGetAttributeRatio(pAttributes, MF_MT_FRAME_RATE, &uNum, &uDen)) && uDen > 0)
        pMetadata->dblFrameRate = static_cast<double>(uNum) / static_cast<double>(uDen);

    // Read resolution
    UINT32 uWidth = 0, uHeight = 0;
    if (SUCCEEDED(MFGetAttributeSize(pAttributes, MF_MT_FRAME_SIZE, &uWidth, &uHeight)))
    {
        pMetadata->dwWidth = uWidth;
        pMetadata->dwHeight = uHeight;
    }

    return S_OK;
}

// ============================================================================
// TranscodeProcess
// ============================================================================

TranscodeProcess::TranscodeProcess()
    : m_state(TranscodeProcessIdle)
    , m_fCancellationRequested(false)
    , m_llStartTimeHns(0)
    , m_llPauseTimeHns(0)
    , m_llTotalPauseDurationHns(0)
{
}

TranscodeProcess::~TranscodeProcess()
{
    Shutdown();
}

HRESULT TranscodeProcess::Initialize(LPCWSTR pszInputPath, LPCWSTR pszOutputPath)
{
    if (!pszInputPath || !pszOutputPath)
        return E_POINTER;

    m_strInputPath = pszInputPath;
    m_strOutputPath = pszOutputPath;
    m_state = TranscodeProcessIdle;
    m_progress = TranscodeProcessProgress();
    m_fCancellationRequested = false;

    return S_OK;
}

HRESULT TranscodeProcess::Shutdown()
{
    m_state = TranscodeProcessIdle;
    m_progress = TranscodeProcessProgress();
    m_fCancellationRequested = false;
    return S_OK;
}

// ============================================================================
// State management
// ============================================================================

HRESULT TranscodeProcess::BeginTranscode()
{
    if (m_state != TranscodeProcessIdle)
        return E_UNEXPECTED;

    StartTimer();
    SetState(TranscodeProcessPreparing);
    SetState(TranscodeProcessEncoding);

    return S_OK;
}

HRESULT TranscodeProcess::CompleteTranscode()
{
    if (m_state != TranscodeProcessEncoding && m_state != TranscodeProcessFinalizing)
        return E_UNEXPECTED;

    m_progress.fComplete = true;
    m_progress.fPercentComplete = 100.0f;
    SetState(TranscodeProcessComplete);

    FireProgress();
    return S_OK;
}

HRESULT TranscodeProcess::FailTranscode(HRESULT hrError)
{
    m_progress.hrLastError = hrError;
    SetState(TranscodeProcessFailed);
    return S_OK;
}

HRESULT TranscodeProcess::CancelTranscode()
{
    m_fCancellationRequested = true;
    SetState(TranscodeProcessCancelled);
    return S_OK;
}

HRESULT TranscodeProcess::PauseTranscode()
{
    if (m_state != TranscodeProcessEncoding)
        return E_UNEXPECTED;

    m_llPauseTimeHns = GetCurrentTimeHns();
    SetState(TranscodeProcessPaused);
    return S_OK;
}

HRESULT TranscodeProcess::ResumeTranscode()
{
    if (m_state != TranscodeProcessPaused)
        return E_UNEXPECTED;

    if (m_llPauseTimeHns > 0)
    {
        m_llTotalPauseDurationHns += GetCurrentTimeHns() - m_llPauseTimeHns;
        m_llPauseTimeHns = 0;
    }

    SetState(TranscodeProcessEncoding);
    return S_OK;
}

// ============================================================================
// State query
// ============================================================================

TranscodeProcessState TranscodeProcess::GetState() const throw()
{
    return m_state;
}

bool TranscodeProcess::IsComplete() const throw()
{
    return m_state == TranscodeProcessComplete;
}

bool TranscodeProcess::IsFailed() const throw()
{
    return m_state == TranscodeProcessFailed;
}

bool TranscodeProcess::IsCancelled() const throw()
{
    return m_state == TranscodeProcessCancelled;
}

bool TranscodeProcess::IsPaused() const throw()
{
    return m_state == TranscodeProcessPaused;
}

bool TranscodeProcess::IsEncoding() const throw()
{
    return m_state == TranscodeProcessEncoding;
}

// ============================================================================
// Progress
// ============================================================================

TranscodeProcessProgress TranscodeProcess::GetProgress() const
{
    return m_progress;
}

HRESULT TranscodeProcess::UpdateProgress(LONGLONG llCurrentPositionHns)
{
    m_progress.llCurrentPositionHns = llCurrentPositionHns;

    if (m_progress.llTotalDurationHns > 0)
    {
        double dblRatio = static_cast<double>(llCurrentPositionHns) /
                          static_cast<double>(m_progress.llTotalDurationHns);
        if (dblRatio < 0.0) dblRatio = 0.0;
        if (dblRatio > 1.0) dblRatio = 1.0;
        m_progress.fPercentComplete = static_cast<float>(dblRatio * 100.0);
    }

    m_progress.dblElapsedSeconds = GetElapsedSeconds();
    m_progress.dblEstimatedRemaining = GetEstimatedRemaining();

    FireProgress();
    return S_OK;
}

HRESULT TranscodeProcess::SetTotalDuration(LONGLONG llTotalDurationHns)
{
    m_progress.llTotalDurationHns = llTotalDurationHns;
    return S_OK;
}

HRESULT TranscodeProcess::SetTotalFrameCount(DWORD dwTotalFrames)
{
    m_progress.dwTotalFrames = dwTotalFrames;
    return S_OK;
}

HRESULT TranscodeProcess::SetOutputSize(LONGLONG llSizeBytes)
{
    m_progress.llOutputSizeBytes = llSizeBytes;
    return S_OK;
}

HRESULT TranscodeProcess::IncrementFramesEncoded()
{
    m_progress.dwFramesEncoded++;
    return S_OK;
}

HRESULT TranscodeProcess::SetEncodingFps(double dblFps)
{
    m_progress.dblEncodingFps = dblFps;
    return S_OK;
}

// ============================================================================
// Timing
// ============================================================================

HRESULT TranscodeProcess::StartTimer()
{
    m_llStartTimeHns = GetCurrentTimeHns();
    m_llPauseTimeHns = 0;
    m_llTotalPauseDurationHns = 0;
    return S_OK;
}

double TranscodeProcess::GetElapsedSeconds() const throw()
{
    if (m_llStartTimeHns == 0)
        return 0.0;

    LONGLONG llElapsed = GetCurrentTimeHns() - m_llStartTimeHns - m_llTotalPauseDurationHns;
    return static_cast<double>(llElapsed) / 10000000.0;
}

double TranscodeProcess::GetEstimatedRemaining() const throw()
{
    if (m_progress.fPercentComplete <= 0.0f || m_progress.fPercentComplete >= 100.0f)
        return 0.0;

    double dblElapsed = GetElapsedSeconds();
    if (dblElapsed <= 0.0)
        return 0.0;

    double dblTotalEstimated = dblElapsed / (static_cast<double>(m_progress.fPercentComplete) / 100.0);
    return dblTotalEstimated - dblElapsed;
}

// ============================================================================
// Paths
// ============================================================================

ATL::CString TranscodeProcess::GetInputPath() const
{
    return m_strInputPath;
}

ATL::CString TranscodeProcess::GetOutputPath() const
{
    return m_strOutputPath;
}

// ============================================================================
// Cancel request check
// ============================================================================

bool TranscodeProcess::IsCancellationRequested() const throw()
{
    return m_fCancellationRequested;
}

// ============================================================================
// Private helpers
// ============================================================================

void TranscodeProcess::SetState(TranscodeProcessState newState)
{
    TranscodeProcessState oldState = m_state;
    m_state = newState;
    FireStateChange(oldState, newState);
}

void TranscodeProcess::FireProgress()
{
    if (m_progressCb)
        m_progressCb(m_progress);
}

void TranscodeProcess::FireStateChange(TranscodeProcessState oldState, TranscodeProcessState newState)
{
    if (m_stateChangeCb)
        m_stateChangeCb(oldState, newState);
}

LONGLONG TranscodeProcess::GetCurrentTimeHns() const
{
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    return (static_cast<LONGLONG>(ft.dwHighDateTime) << 32) | ft.dwLowDateTime;
}

} // namespace HMRAVSource
