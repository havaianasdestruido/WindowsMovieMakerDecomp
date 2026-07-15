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
#include <vector>
#include <algorithm>

// ============================================================================
// Internal classes
// ============================================================================
namespace MFReadWrite
{

class ReaderManager
{
public:
    ReaderManager()
        : m_pReader(NULL)
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
        return hr;
    }

    void Close()
    {
        if (m_pReader)
        {
            m_pReader->Release();
            m_pReader = NULL;
        }
    }

    HRESULT GetProperties(WLMediaProperties* pProps)
    {
        if (!m_pReader || !pProps)
            return E_UNEXPECTED;

        IMFMediaType* pVideoType = NULL;
        HRESULT hr = m_pReader->GetNativeMediaType(
            (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, &pVideoType);

        if (SUCCEEDED(hr))
        {
            pProps->bHasVideo = TRUE;

            UINT32 uW = 0, uH = 0;
            pVideoType->GetUINT32(MF_MT_FRAME_SIZE, &uW);
            // MF_MT_FRAME_SIZE encodes width in high word, height in low word
            pProps->uVideoWidth = uW >> 16;
            pProps->uVideoHeight = uW & 0xFFFF;

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
            pAudioType->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &pProps->uAudioSampleRate);
            pAudioType->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &pProps->uAudioChannels);
            pAudioType->GetUINT32(MF_MT_AVG_BITRATE, &pProps->uAudioBitrate);

            GUID guidSubtype = GUID_NULL;
            pAudioType->GetGUID(MF_MT_SUBTYPE, &guidSubtype);
            pProps->guidAudioSubtype = guidSubtype;

            pAudioType->Release();
        }

        // Duration
        PROPVARIANT var;
        PropVariantInit(&var);
        hr = m_pReader->GetPresentationAttribute(
            (DWORD)MF_SOURCE_READER_MEDIASOURCE, MF_PD_DURATION, &var);
        if (SUCCEEDED(hr))
        {
            pProps->llDuration = var.uhVal.QuadPart;
            PropVariantClear(&var);
        }

        return S_OK;
    }

    HRESULT ReadFrame(LONGLONG llTimeMs, BYTE* pBuffer, UINT32 cbBuffer, UINT32* pcbRead)
    {
        if (!m_pReader)
            return E_UNEXPECTED;

        // Seek to the requested time
        LONGLONG llHns = llTimeMs * 10000; // ms to 100ns units
        PROPVARIANT varPosition;
        PropVariantInit(&varPosition);
        varPosition.vt = VT_I8;
        varPosition.uhVal.QuadPart = llHns;

        HRESULT hr = m_pReader->SetCurrentPosition(GUID_NULL, varPosition);
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
    IMFSourceReader* m_pReader;
};

class WriterManager
{
public:
    WriterManager()
        : m_pWriter(NULL)
        , m_bFinalized(false)
    {
    }

    ~WriterManager()
    {
        if (!m_bFinalized && m_pWriter)
            Finalize();
    }

    HRESULT Create(LPCWSTR pszOutputPath, const WLMediaProperties* pProps)
    {
        if (!pszOutputPath || !pProps)
            return E_INVALIDARG;

        HRESULT hr = MFCreateSinkWriterFromURL(pszOutputPath, NULL, NULL, &m_pWriter);
        return hr;
    }

    HRESULT WriteFrame(const BYTE* pData, UINT32 cbData, LONGLONG llTimeMs)
    {
        if (!m_pWriter)
            return E_UNEXPECTED;
        if (!pData || cbData == 0)
            return E_INVALIDARG;

        // In full implementation: wrap data in IMFSample and write
        return E_NOTIMPL;
    }

    HRESULT Finalize()
    {
        if (!m_pWriter)
            return E_UNEXPECTED;

        HRESULT hr = m_pWriter->Finalize();
        m_bFinalized = true;
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
    IMFSinkWriter*  m_pWriter;
    bool            m_bFinalized;
};

} // namespace MFReadWrite

// ============================================================================
// Module state
// ============================================================================
static HINSTANCE g_hModule = NULL;

// ============================================================================
// Exported functions (7 exports)
// ============================================================================

extern "C"
{

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
