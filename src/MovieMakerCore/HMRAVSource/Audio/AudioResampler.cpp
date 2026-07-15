#include "pch.h"
#include <mferror.h>
/*
 * AudioResampler.cpp
 *
 * Implementation of the AudioResamplerHelper using Media Foundation
 * audio resampler MFT.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "AudioResampler.h"

namespace HMRAVSource
{

AudioResamplerHelper::AudioResamplerHelper()
    : m_dwInputSampleRate(0)
    , m_dwInputChannels(0)
    , m_dwOutputSampleRate(0)
    , m_dwOutputChannels(0)
    , m_dwLatencyFrames(0)
    , m_fInitialized(false)
{
}

AudioResamplerHelper::~AudioResamplerHelper()
{
    Shutdown();
}

HRESULT AudioResamplerHelper::Initialize(DWORD dwInputSampleRate, DWORD dwInputChannels,
                                         DWORD dwOutputSampleRate, DWORD dwOutputChannels)
{
    if (m_fInitialized) return S_FALSE;

    m_dwInputSampleRate = dwInputSampleRate;
    m_dwInputChannels = dwInputChannels;
    m_dwOutputSampleRate = dwOutputSampleRate;
    m_dwOutputChannels = dwOutputChannels;

    HRESULT hr = CreateResamplerMft();
    if (FAILED(hr)) return hr;

    hr = ConfigureMediaType();
    if (FAILED(hr)) return hr;

    m_fInitialized = true;
    return S_OK;
}

HRESULT AudioResamplerHelper::Shutdown()
{
    if (!m_fInitialized) return S_FALSE;

    if (m_spResampler)
        m_spResampler->ProcessMessage(MFT_MESSAGE_COMMAND_FLUSH, 0);

    m_spResampler.Release();
    m_spEvent.Release();
    m_fInitialized = false;
    return S_OK;
}

bool AudioResamplerHelper::IsInitialized() const throw() { return m_fInitialized; }

HRESULT AudioResamplerHelper::ProcessInput(const BYTE* pData, DWORD cbData, LONGLONG llTimestampHns)
{
    if (!m_fInitialized || !m_spResampler || !pData)
        return E_INVALIDARG;

    // Create input sample
    CComPtr<IMFSample> spSample;
    HRESULT hr = MFCreateSample(&spSample);
    if (FAILED(hr)) return hr;

    CComPtr<IMFMediaBuffer> spBuffer;
    hr = MFCreateMemoryBuffer(cbData, &spBuffer);
    if (FAILED(hr)) return hr;

    BYTE* pBuffer = nullptr;
    hr = spBuffer->Lock(&pBuffer, nullptr, nullptr);
    if (FAILED(hr)) return hr;

    memcpy(pBuffer, pData, cbData);
    spBuffer->Unlock();
    spBuffer->SetCurrentLength(cbData);

    spSample->AddBuffer(spBuffer);
    spSample->SetSampleTime(llTimestampHns);

    hr = m_spResampler->ProcessInput(0, spSample, 0);
    return hr;
}

HRESULT AudioResamplerHelper::ProcessOutput(BYTE* pData, DWORD cbMaxData, DWORD* pcbWritten, LONGLONG* pllTimestampHns)
{
    if (!m_fInitialized || !m_spResampler || !pData)
        return E_INVALIDARG;

    MFT_OUTPUT_STREAM_INFO streamInfo = {};
    HRESULT hr = m_spResampler->GetOutputStreamInfo(0, &streamInfo);
    if (FAILED(hr)) return hr;

    CComPtr<IMFSample> spOutputSample;
    hr = MFCreateSample(&spOutputSample);
    if (FAILED(hr)) return hr;

    CComPtr<IMFMediaBuffer> spBuffer;
    hr = MFCreateMemoryBuffer(streamInfo.cbSize, &spBuffer);
    if (FAILED(hr)) return hr;

    spOutputSample->AddBuffer(spBuffer);

    MFT_OUTPUT_DATA_BUFFER outputData = {};
    outputData.dwStreamID = 0;
    outputData.pSample = spOutputSample;

    DWORD dwStatus = 0;
    hr = m_spResampler->ProcessOutput(0, 1, &outputData, &dwStatus);

    if (hr == S_OK || hr == MF_S_TRANSFORM_STREAM_CHANGE)
    {
        spBuffer = nullptr;
        spOutputSample->GetBufferByIndex(0, &spBuffer);
        if (spBuffer)
        {
            BYTE* pSrc = nullptr;
            DWORD cbSrc = 0;
            spBuffer->Lock(&pSrc, nullptr, &cbSrc);

            DWORD cbToCopy = std::min(cbSrc, cbMaxData);
            memcpy(pData, pSrc, cbToCopy);

            if (pcbWritten) *pcbWritten = cbToCopy;

            LONGLONG llTime = 0;
            spOutputSample->GetSampleTime(&llTime);
            if (pllTimestampHns) *pllTimestampHns = llTime;

            spBuffer->Unlock();
        }
    }

    return hr;
}

HRESULT AudioResamplerHelper::Flush()
{
    if (!m_fInitialized || !m_spResampler) return E_FAIL;
    return m_spResampler->ProcessMessage(MFT_MESSAGE_COMMAND_FLUSH, 0);
}

DWORD AudioResamplerHelper::GetInputSampleRate() const throw() { return m_dwInputSampleRate; }
DWORD AudioResamplerHelper::GetInputChannels() const throw() { return m_dwInputChannels; }
DWORD AudioResamplerHelper::GetOutputSampleRate() const throw() { return m_dwOutputSampleRate; }
DWORD AudioResamplerHelper::GetOutputChannels() const throw() { return m_dwOutputChannels; }

DWORD AudioResamplerHelper::GetLatencyFrames() const throw() { return m_dwLatencyFrames; }

HRESULT AudioResamplerHelper::Reset()
{
    if (!m_fInitialized || !m_spResampler) return E_FAIL;
    HRESULT hr = m_spResampler->ProcessMessage(MFT_MESSAGE_COMMAND_FLUSH, 0);
    if (SUCCEEDED(hr))
        hr = m_spResampler->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, 0);
    return hr;
}

HRESULT AudioResamplerHelper::CreateResamplerMft()
{
    HRESULT hr = CoCreateInstance(__uuidof(CResamplerMediaObject), nullptr, CLSCTX_ALL,
                                 IID_PPV_ARGS(&m_spResampler));
    return hr;
}

HRESULT AudioResamplerHelper::ConfigureMediaType()
{
    if (!m_spResampler) return E_FAIL;

    // Create input media type
    CComPtr<IMFMediaType> spInputType;
    HRESULT hr = MFCreateMediaType(&spInputType);
    if (FAILED(hr)) return hr;

    spInputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
    spInputType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
    spInputType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, m_dwInputChannels);
    spInputType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, m_dwInputSampleRate);
    spInputType->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, m_dwInputChannels * sizeof(short));
    spInputType->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, m_dwInputSampleRate * m_dwInputChannels * sizeof(short));
    spInputType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, 16);
    spInputType->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE);

    hr = m_spResampler->SetInputType(0, spInputType, 0);
    if (FAILED(hr)) return hr;

    // Create output media type
    CComPtr<IMFMediaType> spOutputType;
    hr = MFCreateMediaType(&spOutputType);
    if (FAILED(hr)) return hr;

    spOutputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
    spOutputType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
    spOutputType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, m_dwOutputChannels);
    spOutputType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, m_dwOutputSampleRate);
    spOutputType->SetUINT32(MF_MT_AUDIO_BLOCK_ALIGNMENT, m_dwOutputChannels * sizeof(short));
    spOutputType->SetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECOND, m_dwOutputSampleRate * m_dwOutputChannels * sizeof(short));
    spOutputType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, 16);
    spOutputType->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE);

    hr = m_spResampler->SetOutputType(0, spOutputType, 0);
    if (FAILED(hr)) return hr;

    // Begin streaming
    hr = m_spResampler->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, 0);

    m_dwLatencyFrames = 1024; // typical resampler latency

    return hr;
}

} // namespace HMRAVSource
