/*
 * WLMFReadWrite.cpp
 *
 * Implementation of WLMFReadWrite.dll -- Media Foundation read/write for
 * Windows Live Movie Maker 2012.
 *
 * Provides convenience wrappers around MFSourceReader and MFSinkWriter
 * for reading video files, querying media properties, extracting frames,
 * and writing encoded output.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "WLMFReadWrite.h"
#include "WLXPhotoBase.h"

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include <mfobjects.h>
#include <shlwapi.h>
#include <vector>
#include <algorithm>

// ============================================================================
// Internal classes
// ============================================================================
namespace MFReadWrite
{

// ----------------------------------------------------------------------------
// Default codec selection
//
// MFReader/MFWriter receive caller-specified subtypes via WLMediaProperties.
// When the caller leaves a subtype at GUID_NULL (the WLMediaProperties default),
// pick an encoder that the container implied by the output extension is known
// to accept. H.264/AAC cover MP4/AVI; WMV3/WMA cover ASF/WMV.
// ----------------------------------------------------------------------------
static GUID DefaultVideoSubtypeFromPath(LPCWSTR pszOutputPath)
{
    LPCWSTR pExt = PathFindExtensionW(pszOutputPath);
    if (pExt && *pExt)
    {
        if (lstrcmpiW(pExt, L".wmv") == 0 || lstrcmpiW(pExt, L".asf") == 0)
            return MFVideoFormat_WMV3;
    }
    return MFVideoFormat_H264;
}

static GUID DefaultAudioSubtypeFromPath(LPCWSTR pszOutputPath)
{
    LPCWSTR pExt = PathFindExtensionW(pszOutputPath);
    if (pExt && *pExt)
    {
        if (lstrcmpiW(pExt, L".wma") == 0 || lstrcmpiW(pExt, L".asf") == 0)
            return MFAudioFormat_WMAudioV8;
    }
    return MFAudioFormat_AAC;
}

class ReaderManager
{
public:
    ReaderManager()
        : m_pReader(NULL)
        , m_bHasVideoStream(false)
        , m_bVideoStreamConfigured(false)
    {
    }

    ~ReaderManager()
    {
        Close();
    }

    HRESULT Open(LPCWSTR pszFilePath)
    {
        Close();

        if (!pszFilePath || !pszFilePath[0])
            return E_INVALIDARG;

        HRESULT hr = MFCreateSourceReaderFromURL(pszFilePath, NULL, &m_pReader);
        if (FAILED(hr))
            return hr;

        // Detect a video stream up front so ReadFrame can fail fast with a
        // meaningful error for audio-only files. The native (encoded) type
        // is still used by GetProperties -- no decoding occurs there.
        IMFMediaType* pType = NULL;
        HRESULT hrVideo = m_pReader->GetNativeMediaType(
            (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, &pType);
        m_bHasVideoStream = SUCCEEDED(hrVideo);
        if (pType)
            pType->Release();

        return S_OK;
    }

    void Close()
    {
        if (m_pReader)
        {
            m_pReader->Release();
            m_pReader = NULL;
        }
        m_bVideoStreamConfigured = false;
        m_bHasVideoStream = false;
    }

    HRESULT GetProperties(WLMediaProperties* pProps)
    {
        if (!m_pReader || !pProps)
            return E_UNEXPECTED;

        HRESULT hrOverall = E_FAIL;

        // Duration (100ns units) reported by the media source
        PROPVARIANT var;
        PropVariantInit(&var);
        if (SUCCEEDED(m_pReader->GetPresentationAttribute(
                (DWORD)MF_SOURCE_READER_MEDIASOURCE, MF_PD_DURATION, &var))
            && var.vt == VT_UI8)
        {
            pProps->llDuration = var.uhVal.QuadPart;
            hrOverall = S_OK;
        }
        PropVariantClear(&var);

        // Best-effort file type (MIME type reported by the media source)
        PropVariantInit(&var);
        if (SUCCEEDED(m_pReader->GetPresentationAttribute(
                (DWORD)MF_SOURCE_READER_MEDIASOURCE, MF_PD_MIME_TYPE, &var))
            && var.vt == VT_LPWSTR && var.pwszVal)
        {
            lstrcpynW(pProps->wszFileType, var.pwszVal,
                sizeof(pProps->wszFileType) / sizeof(pProps->wszFileType[0]));
            hrOverall = S_OK;
        }
        PropVariantClear(&var);

        IMFMediaType* pVideoType = NULL;
        HRESULT hr = m_pReader->GetNativeMediaType(
            (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, &pVideoType);

        if (SUCCEEDED(hr))
        {
            pProps->bHasVideo = TRUE;
            hrOverall = S_OK;

            UINT32 uW = 0, uH = 0;
            // Correctly retrieve width/height using MFGetAttributeSize
            HRESULT sizeHr = MFGetAttributeSize(pVideoType, MF_MT_FRAME_SIZE, &uW, &uH);
            if (SUCCEEDED(sizeHr)) {
                pProps->uVideoWidth = uW;
                pProps->uVideoHeight = uH;
            }

            MFGetAttributeRatio(pVideoType, MF_MT_FRAME_RATE,
                &pProps->uFrameRateNumerator, &pProps->uFrameRateDenominator);

            pVideoType->GetUINT32(MF_MT_AVG_BITRATE, &pProps->uVideoBitrate);

            GUID guidSubtype = GUID_NULL;
            pVideoType->GetGUID(MF_MT_SUBTYPE, &guidSubtype);
            pProps->guidVideoSubtype = guidSubtype;

            pVideoType->Release();
        }

        IMFMediaType* pAudioType = NULL;
        hr = m_pReader->GetNativeMediaType(
            (DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, &pAudioType);

        if (SUCCEEDED(hr))
        {
            pProps->bHasAudio = TRUE;
            hrOverall = S_OK;
            pAudioType->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &pProps->uAudioSampleRate);
            pAudioType->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &pProps->uAudioChannels);
            pAudioType->GetUINT32(MF_MT_AVG_BITRATE, &pProps->uAudioBitrate);

            GUID guidSubtype = GUID_NULL;
            pAudioType->GetGUID(MF_MT_SUBTYPE, &guidSubtype);
            pProps->guidAudioSubtype = guidSubtype;

            pAudioType->Release();
        }

        return SUCCEEDED(hrOverall) ? S_OK : hrOverall;
    }

    HRESULT ReadFrame(LONGLONG llTimeMs, BYTE* pBuffer, UINT32 cbBuffer, UINT32* pcbRead)
    {
        if (!m_pReader)
            return E_UNEXPECTED;
        if (!pBuffer || cbBuffer == 0)
            return E_INVALIDARG;
        if (!m_bHasVideoStream)
            return MF_E_INVALIDSTREAMNUMBER;

        // Configure a decoded (uncompressed) output once so callers receive
        // raw pixel data instead of the encoded source samples.
        HRESULT hr = ConfigureVideoStream();
        if (FAILED(hr))
            return hr;

        // Seek to the requested time
        LONGLONG llHns = llTimeMs * 10000; // ms to 100ns units
        PROPVARIANT varPosition;
        PropVariantInit(&varPosition);
        varPosition.vt = VT_I8;
        varPosition.uhVal.QuadPart = llHns;

        hr = m_pReader->SetCurrentPosition(GUID_NULL, varPosition);
        PropVariantClear(&varPosition);
        if (FAILED(hr))
            return hr;

        // Read sample
        DWORD dwFlags = 0;
        IMFSample* pSample = NULL;
        LONGLONG llPosition = 0;

        hr = m_pReader->ReadSample(
            (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,
            0, NULL, &dwFlags, &llPosition, &pSample);

        if (FAILED(hr))
            return hr;

        if (!pSample || (dwFlags & MF_SOURCE_READERF_ENDOFSTREAM))
        {
            if (pSample) pSample->Release();
            if (pcbRead) *pcbRead = 0;
            return S_FALSE;
        }

        // Convert sample to buffer
        IMFMediaBuffer* pBufferObj = NULL;
        hr = pSample->ConvertToContiguousBuffer(&pBufferObj);
        if (SUCCEEDED(hr))
        {
            BYTE* pData = NULL;
            DWORD cbData = 0;
            hr = pBufferObj->Lock(&pData, NULL, &cbData);

            if (SUCCEEDED(hr))
            {
                UINT32 cbCopy = (std::min)(static_cast<DWORD>(cbBuffer), cbData);
                CopyMemory(pBuffer, pData, cbCopy);
                if (pcbRead) *pcbRead = cbCopy;

                pBufferObj->Unlock();
            }

            pBufferObj->Release();
        }

        pSample->Release();
        return hr;
    }

private:
    // Sets the video stream output type to decoded RGB32 (fallback NV12) so
    // ReadFrame returns uncompressed frames. The source reader inserts a color
    // converter as needed; RGB32 gives callers a simple bottom-up BGRA buffer.
    HRESULT ConfigureVideoStream()
    {
        if (m_bVideoStreamConfigured)
            return S_OK;

        static const GUID rgSubtypes[] = { MFVideoFormat_RGB32, MFVideoFormat_NV12 };

        HRESULT hr = E_FAIL;
        for (size_t i = 0; i < sizeof(rgSubtypes) / sizeof(rgSubtypes[0]) && FAILED(hr); i++)
        {
            IMFMediaType* pType = NULL;
            hr = MFCreateMediaType(&pType);
            if (FAILED(hr))
                break;

            pType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
            pType->SetGUID(MF_MT_SUBTYPE, rgSubtypes[i]);

            hr = m_pReader->SetCurrentMediaType(
                (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, NULL, pType);
            pType->Release();
        }

        m_bVideoStreamConfigured = SUCCEEDED(hr);
        return hr;
    }

    IMFSourceReader* m_pReader;
    bool            m_bHasVideoStream;
    bool            m_bVideoStreamConfigured;
};

class WriterManager
{
public:
    WriterManager()
        : m_pWriter(NULL)
        , m_bFinalized(false)
        , m_bIsVideo(false)
        , m_dwStreamIndex((DWORD)MF_SINK_WRITER_MEDIASINK)
        , m_uAudioSampleRate(0)
        , m_uAudioChannels(0)
        , m_llVideoFrameDuration(333333)
    {
    }

    ~WriterManager()
    {
        if (!m_bFinalized && m_pWriter)
            Finalize();
        Release();
    }

    HRESULT Create(LPCWSTR pszOutputPath, const WLMediaProperties* pProps)
    {
        if (!pszOutputPath || !pProps)
            return E_INVALIDARG;

        Release();
        m_bFinalized = false;

        HRESULT hr = MFCreateSinkWriterFromURL(pszOutputPath, NULL, NULL, &m_pWriter);
        if (FAILED(hr))
            return hr;

        // Only a single write stream is exposed (MFWriter_WriteFrame), so the
        // writer is configured for whichever stream the caller describes:
        // video frames (WriteFrame = decoded frame) or, for audio-only files,
        // audio samples (WriteFrame = 16-bit PCM bytes).
        if (pProps->bHasVideo)
        {
            if (pProps->uVideoWidth == 0 || pProps->uVideoHeight == 0)
                hr = MF_E_INVALIDMEDIATYPE;
            else
                hr = AddVideoStream(pszOutputPath, pProps);
        }
        else if (pProps->bHasAudio)
        {
            if (pProps->uAudioSampleRate == 0)
                hr = MF_E_INVALIDMEDIATYPE;
            else
                hr = AddAudioStream(pszOutputPath, pProps);
        }
        else
        {
            hr = MF_E_INVALIDMEDIATYPE;
        }

        if (FAILED(hr))
        {
            Release();
            return hr;
        }

        return S_OK;
    }

    HRESULT WriteFrame(const BYTE* pData, UINT32 cbData, LONGLONG llTimeMs)
    {
        if (!m_pWriter)
            return E_UNEXPECTED;
        if (!pData || cbData == 0)
            return E_INVALIDARG;

        // Wrap the caller's bytes in an IMFSample and deliver to the sink
        // writer, which encodes them to the configured output media type.
        IMFSample* pSample = NULL;
        HRESULT hr = MFCreateSample(&pSample);
        if (FAILED(hr))
            return hr;

        IMFMediaBuffer* pBuffer = NULL;
        hr = MFCreateMemoryBuffer(cbData, &pBuffer);
        if (SUCCEEDED(hr))
        {
            BYTE* pDst = NULL;
            hr = pBuffer->Lock(&pDst, NULL, NULL);
            if (SUCCEEDED(hr))
            {
                CopyMemory(pDst, pData, cbData);
                hr = pBuffer->Unlock();
            }
            if (SUCCEEDED(hr))
                hr = pBuffer->SetCurrentLength(cbData);
            if (SUCCEEDED(hr))
                hr = pSample->AddBuffer(pBuffer);
            pBuffer->Release();
        }

        if (SUCCEEDED(hr))
            hr = pSample->SetSampleTime(llTimeMs * 10000); // ms to 100ns units
        if (SUCCEEDED(hr))
            hr = pSample->SetSampleDuration(GetSampleDuration(cbData));
        if (SUCCEEDED(hr))
            hr = m_pWriter->WriteSample(m_dwStreamIndex, pSample);

        pSample->Release();
        return hr;
    }

    HRESULT Finalize()
    {
        if (!m_pWriter)
            return E_UNEXPECTED;

        HRESULT hr = m_pWriter->Finalize();
        m_bFinalized = SUCCEEDED(hr);
        return hr;
    }

    void Release()
    {
        if (m_pWriter)
        {
            m_pWriter->Release();
            m_pWriter = NULL;
        }
    }

private:
    HRESULT AddVideoStream(LPCWSTR pszOutputPath, const WLMediaProperties* pProps)
    {
        // Output type: compressed. The encoder is chosen from the caller's
        // subtype, or a default matched to the container (file extension).
        GUID guidSubtype = pProps->guidVideoSubtype;
        if (guidSubtype == GUID_NULL)
            guidSubtype = DefaultVideoSubtypeFromPath(pszOutputPath);

        UINT32 uFrameRateNum = pProps->uFrameRateNumerator;
        UINT32 uFrameRateDen = pProps->uFrameRateDenominator;
        if (uFrameRateNum == 0 || uFrameRateDen == 0)
        {
            uFrameRateNum = 30;
            uFrameRateDen = 1;
        }

        IMFMediaType* pOutputType = NULL;
        HRESULT hr = MFCreateMediaType(&pOutputType);
        if (FAILED(hr))
            return hr;

        pOutputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
        pOutputType->SetGUID(MF_MT_SUBTYPE, guidSubtype);
        MFSetAttributeSize(pOutputType, MF_MT_FRAME_SIZE,
            pProps->uVideoWidth, pProps->uVideoHeight);
        MFSetAttributeRatio(pOutputType, MF_MT_FRAME_RATE,
            uFrameRateNum, uFrameRateDen);
        MFSetAttributeRatio(pOutputType, MF_MT_PIXEL_ASPECT_RATIO, 1, 1);
        pOutputType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
        if (pProps->uVideoBitrate > 0)
            pOutputType->SetUINT32(MF_MT_AVG_BITRATE, pProps->uVideoBitrate);

        hr = m_pWriter->AddStream(pOutputType, &m_dwStreamIndex);
        pOutputType->Release();
        if (FAILED(hr))
            return hr;

        m_llVideoFrameDuration = 10000000LL * uFrameRateDen / uFrameRateNum;
        m_bIsVideo = true;

        // Input type: uncompressed RGB32 -- the sink writer inserts a color
        // converter to the encoder's native format when needed. Fall back to
        // NV12 (the common MF native format) if RGB32 is rejected.
        static const GUID rgInputSubtypes[] = { MFVideoFormat_RGB32, MFVideoFormat_NV12 };

        HRESULT hrInput = E_FAIL;
        for (size_t i = 0; i < sizeof(rgInputSubtypes) / sizeof(rgInputSubtypes[0]) && FAILED(hrInput); i++)
        {
            IMFMediaType* pInputType = NULL;
            hrInput = MFCreateMediaType(&pInputType);
            if (FAILED(hrInput))
                break;

            pInputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
            pInputType->SetGUID(MF_MT_SUBTYPE, rgInputSubtypes[i]);
            MFSetAttributeSize(pInputType, MF_MT_FRAME_SIZE,
                pProps->uVideoWidth, pProps->uVideoHeight);
            MFSetAttributeRatio(pInputType, MF_MT_FRAME_RATE,
                uFrameRateNum, uFrameRateDen);
            MFSetAttributeRatio(pInputType, MF_MT_PIXEL_ASPECT_RATIO, 1, 1);
            pInputType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);

            hrInput = m_pWriter->SetInputMediaType(m_dwStreamIndex, pInputType, NULL);
            pInputType->Release();
        }

        return hrInput;
    }

    HRESULT AddAudioStream(LPCWSTR pszOutputPath, const WLMediaProperties* pProps)
    {
        GUID guidSubtype = pProps->guidAudioSubtype;
        if (guidSubtype == GUID_NULL)
            guidSubtype = DefaultAudioSubtypeFromPath(pszOutputPath);

        IMFMediaType* pOutputType = NULL;
        HRESULT hr = MFCreateMediaType(&pOutputType);
        if (FAILED(hr))
            return hr;

        pOutputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
        pOutputType->SetGUID(MF_MT_SUBTYPE, guidSubtype);
        pOutputType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, pProps->uAudioSampleRate);
        pOutputType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, pProps->uAudioChannels);
        if (pProps->uAudioBitrate > 0)
        {
            pOutputType->SetUINT32(MF_MT_AVG_BITRATE, pProps->uAudioBitrate);
            pOutputType->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, pProps->uAudioBitrate / 8);
        }

        hr = m_pWriter->AddStream(pOutputType, &m_dwStreamIndex);
        pOutputType->Release();
        if (FAILED(hr))
            return hr;

        // Input: 16-bit PCM; the sink writer inserts an encoder/resampler to
        // match the configured output media type.
        IMFMediaType* pInputType = NULL;
        hr = MFCreateMediaType(&pInputType);
        if (FAILED(hr))
            return hr;

        pInputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
        pInputType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
        pInputType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, pProps->uAudioSampleRate);
        pInputType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, pProps->uAudioChannels);
        pInputType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, 16);

        hr = m_pWriter->SetInputMediaType(m_dwStreamIndex, pInputType, NULL);
        pInputType->Release();

        m_bIsVideo = false;
        m_uAudioSampleRate = pProps->uAudioSampleRate;
        m_uAudioChannels = pProps->uAudioChannels;

        return hr;
    }

    // Returns the 100ns duration of one written unit. For video that is one
    // frame (1/frame rate); for audio it is derived from the PCM byte count.
    LONGLONG GetSampleDuration(UINT32 cbData) const
    {
        if (m_bIsVideo)
            return m_llVideoFrameDuration;

        UINT32 cbPerSecond = m_uAudioSampleRate * m_uAudioChannels * sizeof(SHORT);
        if (cbPerSecond == 0)
            return 10000000LL;
        return (LONGLONG)cbData * 10000000LL / cbPerSecond;
    }

    IMFSinkWriter*  m_pWriter;
    bool            m_bFinalized;
    bool            m_bIsVideo;
    DWORD           m_dwStreamIndex;
    UINT32          m_uAudioSampleRate;
    UINT32          m_uAudioChannels;
    LONGLONG        m_llVideoFrameDuration;
};

} // namespace MFReadWrite

// ============================================================================
// Exported functions -- COM exports + MF reader/writer functions
// ============================================================================

extern "C"
{

STDAPI DllCanUnloadNow()
{
    // No outstanding COM objects are ever handed out (DllGetClassObject
    // returns CLASS_E_CLASSNOTAVAILABLE), so the DLL is always safe to
    // unload.  S_OK matches the reference binary; the earlier S_FALSE was
    // a leftover stub value.
    return S_OK;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    UNREFERENCED_PARAMETER(rclsid);
    UNREFERENCED_PARAMETER(riid);
    UNREFERENCED_PARAMETER(ppv);
    return CLASS_E_CLASSNOTAVAILABLE;
}

WLMFRW_API HANDLE __stdcall MFReader_Open(LPCWSTR pszFilePath)
{
    MFReadWrite::ReaderManager* pReader = new(std::nothrow) MFReadWrite::ReaderManager();
    if (!pReader) return NULL;

    HRESULT hr = pReader->Open(pszFilePath);
    if (FAILED(hr))
    {
        delete pReader;
        return NULL;
    }

    return static_cast<HANDLE>(pReader);
}

WLMFRW_API void __stdcall MFReader_Close(HANDLE hReader)
{
    if (hReader)
    {
        MFReadWrite::ReaderManager* p = static_cast<MFReadWrite::ReaderManager*>(hReader);
        p->Close();
        delete p;
    }
}

WLMFRW_API HRESULT __stdcall MFReader_GetProperties(HANDLE hReader, WLMediaProperties* pProps)
{
    if (!hReader || !pProps)
        return E_INVALIDARG;
    return static_cast<MFReadWrite::ReaderManager*>(hReader)->GetProperties(pProps);
}

WLMFRW_API HRESULT __stdcall MFReader_ReadFrame(HANDLE hReader, LONGLONG llTimeMs,
    BYTE* pBuffer, UINT32 cbBuffer, UINT32* pcbRead)
{
    if (!hReader)
        return E_INVALIDARG;
    return static_cast<MFReadWrite::ReaderManager*>(hReader)->ReadFrame(
        llTimeMs, pBuffer, cbBuffer, pcbRead);
}

WLMFRW_API HANDLE __stdcall MFWriter_Create(LPCWSTR pszOutputPath, const WLMediaProperties* pProps)
{
    MFReadWrite::WriterManager* pWriter = new(std::nothrow) MFReadWrite::WriterManager();
    if (!pWriter) return NULL;

    HRESULT hr = pWriter->Create(pszOutputPath, pProps);
    if (FAILED(hr))
    {
        delete pWriter;
        return NULL;
    }

    return static_cast<HANDLE>(pWriter);
}

WLMFRW_API HRESULT __stdcall MFWriter_WriteFrame(HANDLE hWriter, const BYTE* pData, UINT32 cbData, LONGLONG llTimeMs)
{
    if (!hWriter) return E_INVALIDARG;
    return static_cast<MFReadWrite::WriterManager*>(hWriter)->WriteFrame(pData, cbData, llTimeMs);
}

WLMFRW_API HRESULT __stdcall MFWriter_Finalize(HANDLE hWriter)
{
    if (!hWriter) return E_INVALIDARG;
    return static_cast<MFReadWrite::WriterManager*>(hWriter)->Finalize();
}

} // extern "C"
