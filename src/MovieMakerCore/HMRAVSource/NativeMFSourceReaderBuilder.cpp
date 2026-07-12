// NativeMFSourceReaderBuilder.cpp - Native MF source reader builder implementation

#include "pch.h"
#include "NativeMFSourceReaderBuilder.h"

namespace HMRAVSource
{

// ============================================================================
// Construction / Destruction
// ============================================================================

NativeMFSourceReaderBuilder::NativeMFSourceReaderBuilder()
    : m_fInitialized(false)
{
}

NativeMFSourceReaderBuilder::~NativeMFSourceReaderBuilder()
{
    Shutdown();
}

// ============================================================================
// Lifecycle
// ============================================================================

HRESULT NativeMFSourceReaderBuilder::Initialize()
{
    m_fInitialized = true;
    return S_OK;
}

HRESULT NativeMFSourceReaderBuilder::Shutdown()
{
    m_fInitialized = false;
    return S_OK;
}

// ============================================================================
// Source reader creation
// ============================================================================

HRESULT NativeMFSourceReaderBuilder::CreateSourceReader(LPCWSTR pszFilePath, IMFSourceReader** ppReader)
{
    if (!pszFilePath || !ppReader)
        return E_POINTER;

    *ppReader = nullptr;

    if (!m_fInitialized)
        return E_UNEXPECTED;

    CComPtr<IMFAttributes> spAttributes;
    HRESULT hr = CreateDefaultAttributes(&spAttributes);
    if (FAILED(hr))
        return hr;

    hr = MFCreateSourceReaderFromURL(pszFilePath, spAttributes, ppReader);
    return hr;
}

HRESULT NativeMFSourceReaderBuilder::CreateSourceReaderWithAttributes(
    LPCWSTR pszFilePath,
    IMFAttributes* pAttributes,
    IMFSourceReader** ppReader)
{
    if (!pszFilePath || !ppReader)
        return E_POINTER;

    *ppReader = nullptr;

    return MFCreateSourceReaderFromURL(pszFilePath, pAttributes, ppReader);
}

HRESULT NativeMFSourceReaderBuilder::CreateSourceReaderFromByteStream(
    IMFByteStream* pByteStream,
    LPCWSTR pszUrl,
    IMFSourceReader** ppReader)
{
    if (!pByteStream || !ppReader)
        return E_POINTER;

    *ppReader = nullptr;

    CComPtr<IMFAttributes> spAttributes;
    HRESULT hr = CreateDefaultAttributes(&spAttributes);
    if (FAILED(hr))
        return hr;

    hr = MFCreateSourceReaderFromByteStream(pByteStream, spAttributes, ppReader);
    return hr;
}

// ============================================================================
// Format support
// ============================================================================

bool NativeMFSourceReaderBuilder::IsNativeFormatSupported(LPCWSTR pszFilePath)
{
    if (!pszFilePath)
        return false;

    LPCWSTR pszExtension = PathFindExtensionW(pszFilePath);
    if (!pszExtension || *pszExtension == L'\0')
        return false;

    return IsStandardExtension(pszExtension);
}

bool NativeMFSourceReaderBuilder::CanOpenWithNativeReader(LPCWSTR pszFilePath)
{
    return IsNativeFormatSupported(pszFilePath);
}

// ============================================================================
// Media type configuration
// ============================================================================

HRESULT NativeMFSourceReaderBuilder::SetVideoOutputType(
    IMFSourceReader* pReader,
    IMFMediaType* pType)
{
    if (!pReader || !pType)
        return E_POINTER;

    return pReader->SetCurrentMediaType(
        MF_SOURCE_READER_FIRST_VIDEO_STREAM,
        nullptr,
        pType);
}

HRESULT NativeMFSourceReaderBuilder::SetAudioOutputType(
    IMFSourceReader* pReader,
    IMFMediaType* pType)
{
    if (!pReader || !pType)
        return E_POINTER;

    return pReader->SetCurrentMediaType(
        MF_SOURCE_READER_FIRST_AUDIO_STREAM,
        nullptr,
        pType);
}

HRESULT NativeMFSourceReaderBuilder::SetDefaultVideoOutputType(
    IMFSourceReader* pReader,
    UINT uWidth,
    UINT uHeight)
{
    if (!pReader)
        return E_POINTER;

    CComPtr<IMFMediaType> spType;
    HRESULT hr = MFCreateMediaType(&spType);
    if (FAILED(hr))
        return hr;

    spType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    spType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_NV12);
    spType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
    MFSetAttributeSize(spType, MF_MT_FRAME_SIZE, uWidth, uHeight);

    return SetVideoOutputType(pReader, spType);
}

HRESULT NativeMFSourceReaderBuilder::SetDefaultAudioOutputType(
    IMFSourceReader* pReader,
    DWORD dwSampleRate,
    DWORD dwChannels)
{
    if (!pReader)
        return E_POINTER;

    CComPtr<IMFMediaType> spType;
    HRESULT hr = MFCreateMediaType(&spType);
    if (FAILED(hr))
        return hr;

    spType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
    spType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
    spType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, dwSampleRate);
    spType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, dwChannels);
    spType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, 16);
    spType->SetUINT32(MF_MT_BLOCK_ALIGNMENT, dwChannels * 2);

    return SetAudioOutputType(pReader, spType);
}

// ============================================================================
// Source information
// ============================================================================

HRESULT NativeMFSourceReaderBuilder::GetSourceDuration(IMFSourceReader* pReader, LONGLONG* pllDuration)
{
    if (!pReader || !pllDuration)
        return E_POINTER;

    *pllDuration = 0;

    PROPVARIANT var;
    PropVariantInit(&var);
    HRESULT hr = pReader->GetPresentationAttribute(
        MF_SOURCE_READER_MEDIASOURCE,
        MF_PD_DURATION,
        &var);

    if (SUCCEEDED(hr) && var.vt == VT_UI8)
    {
        *pllDuration = static_cast<LONGLONG>(var.uhVal.QuadPart);
    }

    PropVariantClear(&var);
    return hr;
}

HRESULT NativeMFSourceReaderBuilder::GetStreamCount(IMFSourceReader* pReader, DWORD* pdwCount)
{
    if (!pReader || !pdwCount)
        return E_POINTER;

    *pdwCount = 0;

    DWORD dwCount = 0;
    for (;;)
    {
        CComPtr<IMFMediaType> spType;
        HRESULT hr = pReader->GetNativeMediaType(dwCount, 0, &spType);
        if (FAILED(hr))
            break;
        dwCount++;
    }

    *pdwCount = dwCount;
    return S_OK;
}

HRESULT NativeMFSourceReaderBuilder::GetNativeMediaType(
    IMFSourceReader* pReader,
    DWORD dwStreamIndex,
    IMFMediaType** ppType)
{
    if (!pReader || !ppType)
        return E_POINTER;

    return pReader->GetNativeMediaType(dwStreamIndex, 0, ppType);
}

// ============================================================================
// Private helpers
// ============================================================================

bool NativeMFSourceReaderBuilder::IsStandardExtension(LPCWSTR pszExtension)
{
    if (!pszExtension)
        return false;

    if (_wcsicmp(pszExtension, L".mp4") == 0 ||
        _wcsicmp(pszExtension, L".mov") == 0 ||
        _wcsicmp(pszExtension, L".m4v") == 0 ||
        _wcsicmp(pszExtension, L".mp3") == 0 ||
        _wcsicmp(pszExtension, L".m4a") == 0 ||
        _wcsicmp(pszExtension, L".wav") == 0 ||
        _wcsicmp(pszExtension, L".aac") == 0 ||
        _wcsicmp(pszExtension, L".wma") == 0 ||
        _wcsicmp(pszExtension, L".wmv") == 0 ||
        _wcsicmp(pszExtension, L".avi") == 0 ||
        _wcsicmp(pszExtension, L".asf") == 0 ||
        _wcsicmp(pszExtension, L".3gp") == 0 ||
        _wcsicmp(pszExtension, L".3g2") == 0 ||
        _wcsicmp(pszExtension, L".flv") == 0 ||
        _wcsicmp(pszExtension, L".mkv") == 0 ||
        _wcsicmp(pszExtension, L".webm") == 0 ||
        _wcsicmp(pszExtension, L".mpg") == 0 ||
        _wcsicmp(pszExtension, L".mpeg") == 0)
    {
        return true;
    }

    return false;
}

HRESULT NativeMFSourceReaderBuilder::CreateDefaultAttributes(IMFAttributes** ppAttributes)
{
    if (!ppAttributes)
        return E_POINTER;

    HRESULT hr = MFCreateAttributes(ppAttributes, 1);
    if (FAILED(hr))
        return hr;

    // Enable hardware transforms when available
    (*ppAttributes)->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, TRUE);

    return S_OK;
}

} // namespace HMRAVSource
