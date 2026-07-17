#include "pch.h"
#include <functiondiscoverykeys_devpkey.h>
/*
 * AudioOutput.cpp
 *
 * Implementation of the AudioOutput class for WASAPI audio rendering.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "AudioOutput.h"

namespace HMRAVSource
{

AudioOutput::AudioOutput()
    : m_state(StateStopped)
    , m_flVolume(1.0f)
    , m_fMute(false)
    , m_llPositionHns(0)
    , m_dwBufferFrameCount(0)
    , m_fInitialized(false)
{
}

AudioOutput::~AudioOutput()
{
    Shutdown();
}

HRESULT AudioOutput::Initialize(const AudioOutputFormat& format)
{
    if (m_fInitialized)
        return S_FALSE;

    m_format = format;

    HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                                 IID_PPV_ARGS(&m_spDeviceEnumerator));
    if (FAILED(hr))
        return hr;

    hr = CreateAudioClient();
    if (FAILED(hr))
        return hr;

    m_fInitialized = true;
    return S_OK;
}

HRESULT AudioOutput::Shutdown()
{
    if (!m_fInitialized)
        return S_FALSE;

    StopSession();

    m_spRenderClient.Release();
    m_spAudioClient.Release();
    m_spDevice.Release();
    m_spDeviceEnumerator.Release();
    m_fInitialized = false;
    return S_OK;
}

bool AudioOutput::IsInitialized() const throw() { return m_fInitialized; }

HRESULT AudioOutput::SetDefaultDevice()
{
    if (!m_spDeviceEnumerator) return E_FAIL;

    m_spDevice.Release();
    HRESULT hr = m_spDeviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &m_spDevice);
    if (SUCCEEDED(hr))
    {
        hr = CreateAudioClient();
    }
    return hr;
}

HRESULT AudioOutput::SetDevice(LPCWSTR pszDeviceId)
{
    if (!m_spDeviceEnumerator)
        return E_FAIL;

    if (!pszDeviceId || pszDeviceId[0] == L'\0')
        return SetDefaultDevice();

    m_spDevice.Release();
    CComPtr<IMMDevice> spDevice;
    HRESULT hr = m_spDeviceEnumerator->GetDevice(pszDeviceId, &spDevice);
    if (FAILED(hr))
        return hr;

    m_spDevice = spDevice;
    return CreateAudioClient();
}

ATL::CString AudioOutput::GetDeviceName() const
{
    if (!m_spDevice) return ATL::CString(L"Default");

    CComPtr<IPropertyStore> spProps;
    HRESULT hr = m_spDevice->OpenPropertyStore(STGM_READ, &spProps);
    if (FAILED(hr)) return ATL::CString(L"Unknown");

    PROPVARIANT varName;
    PropVariantInit(&varName);
    hr = spProps->GetValue(PKEY_Device_FriendlyName, &varName);
    if (SUCCEEDED(hr) && varName.vt == VT_LPWSTR)
    {
        ATL::CString strName(varName.pwszVal);
        PropVariantClear(&varName);
        return strName;
    }

    PropVariantClear(&varName);
    return ATL::CString(L"Unknown");
}

ATL::CString AudioOutput::GetDeviceId() const
{
    if (!m_spDevice)
        return ATL::CString();

    LPWSTR pwszDeviceId = nullptr;
    HRESULT hr = m_spDevice->GetId(&pwszDeviceId);
    if (FAILED(hr) || !pwszDeviceId)
        return ATL::CString();

    ATL::CString strDeviceId(pwszDeviceId);
    CoTaskMemFree(pwszDeviceId);
    return strDeviceId;
}

HRESULT AudioOutput::Play()
{
    if (m_state == StatePlaying)
        return S_FALSE;

    HRESULT hr = StartSession();
    if (SUCCEEDED(hr))
        m_state = StatePlaying;
    return hr;
}

HRESULT AudioOutput::Stop()
{
    if (m_state == StateStopped)
        return S_FALSE;

    HRESULT hr = StopSession();
    m_state = StateStopped;
    m_llPositionHns = 0;
    return hr;
}

HRESULT AudioOutput::Pause()
{
    if (m_state != StatePlaying) return E_FAIL;
    if (m_spAudioClient) m_spAudioClient->Stop();
    m_state = StatePaused;
    return S_OK;
}

HRESULT AudioOutput::Resume()
{
    if (m_state != StatePaused) return E_FAIL;
    return Play();
}

HRESULT AudioOutput::Flush()
{
    if (m_spAudioClient && m_state == StatePlaying)
    {
        m_spAudioClient->Stop();
        m_spAudioClient->Reset();
    }
    m_llPositionHns = 0;
    return S_OK;
}

HRESULT AudioOutput::WriteSample(const BYTE* pData, DWORD cbData)
{
    if (!m_fInitialized || !m_spRenderClient || !pData)
        return E_FAIL;

    UINT32 padding = 0;
    HRESULT hr = m_spAudioClient->GetCurrentPadding(&padding);
    if (FAILED(hr)) return hr;

    UINT32 availableFrames = m_dwBufferFrameCount - padding;
    UINT32 requestedFrames = cbData / m_format.wfx.nBlockAlign;
    UINT32 framesToWrite = std::min(availableFrames, requestedFrames);

    BYTE* pBuffer = nullptr;
    hr = m_spRenderClient->GetBuffer(framesToWrite, &pBuffer);
    if (FAILED(hr)) return hr;

    DWORD cbToWrite = framesToWrite * m_format.wfx.nBlockAlign;
    memcpy(pBuffer, pData, cbToWrite);

    hr = m_spRenderClient->ReleaseBuffer(framesToWrite, 0);
    return hr;
}

HRESULT AudioOutput::WriteSampleFromMF(IMFSample* pSample)
{
    if (!pSample) return E_INVALIDARG;

    CComPtr<IMFMediaBuffer> spBuffer;
    HRESULT hr = pSample->ConvertToContiguousBuffer(&spBuffer);
    if (FAILED(hr)) return hr;

    BYTE* pData = nullptr;
    DWORD cbData = 0;
    hr = spBuffer->Lock(&pData, nullptr, &cbData);
    if (FAILED(hr)) return hr;

    hr = WriteSample(pData, cbData);
    spBuffer->Unlock();
    return hr;
}

HRESULT AudioOutput::SetVolume(float flVolume)
{
    if (flVolume < 0.0f) flVolume = 0.0f;
    if (flVolume > 1.0f) flVolume = 1.0f;
    m_flVolume = flVolume;
    return S_OK;
}

float AudioOutput::GetVolume() const throw() { return m_flVolume; }

HRESULT AudioOutput::SetMute(bool fMute) { m_fMute = fMute; return S_OK; }
bool AudioOutput::IsMuted() const throw() { return m_fMute; }

HRESULT AudioOutput::SetPosition(LONGLONG llPositionHns) { m_llPositionHns = llPositionHns; return S_OK; }
LONGLONG AudioOutput::GetPosition() const throw() { return m_llPositionHns; }

AudioOutput::AudioOutputState AudioOutput::GetState() const throw() { return m_state; }

const AudioOutputFormat& AudioOutput::GetFormat() const { return m_format; }

HRESULT AudioOutput::SetFormat(const AudioOutputFormat& format)
{
    m_format = format;
    if (m_fInitialized)
        return CreateAudioClient();
    return S_OK;
}

DWORD AudioOutput::GetBufferDurationMs() const throw() { return m_format.dwBufferSizeMs; }

DWORD AudioOutput::GetBufferPaddingMs() const throw()
{
    if (!m_spAudioClient || m_dwBufferFrameCount == 0) return 0;
    UINT32 padding = 0;
    m_spAudioClient->GetCurrentPadding(&padding);
    return static_cast<DWORD>((static_cast<double>(padding) / m_format.wfx.nSamplesPerSec) * 1000.0);
}

HRESULT AudioOutput::ExecuteCommand(const AudioOutputCommandParam& param)
{
    switch (param.command)
    {
    case AudioOutputCommandParam::CmdPlay:      return Play();
    case AudioOutputCommandParam::CmdStop:      return Stop();
    case AudioOutputCommandParam::CmdPause:     return Pause();
    case AudioOutputCommandParam::CmdResume:    return Resume();
    case AudioOutputCommandParam::CmdFlush:     return Flush();
    case AudioOutputCommandParam::CmdSetVolume: return SetVolume(param.flVolume);
    case AudioOutputCommandParam::CmdSetMute:   return SetMute(param.fMute);
    case AudioOutputCommandParam::CmdSetPosition:return SetPosition(param.llPositionHns);
    default: return E_INVALIDARG;
    }
}

HRESULT AudioOutput::CreateAudioClient()
{
    if (!m_spDevice) return E_FAIL;

    m_spAudioClient.Release();
    m_spRenderClient.Release();

    WAVEFORMATEX* pwfx = &m_format.wfx;
    REFERENCE_TIME hnsBufferDuration = static_cast<REFERENCE_TIME>(m_format.dwBufferSizeMs) * 10000;
    REFERENCE_TIME hnsPeriodicity = 0;

    HRESULT hr = m_spDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr,
                                      reinterpret_cast<void**>(&m_spAudioClient));
    if (FAILED(hr)) return hr;

    hr = m_spAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, hnsBufferDuration,
                                      hnsPeriodicity, pwfx, nullptr);
    if (FAILED(hr)) return hr;

    hr = m_spAudioClient->GetBufferSize(&m_dwBufferFrameCount);
    if (FAILED(hr)) return hr;

    hr = m_spAudioClient->GetService(IID_PPV_ARGS(&m_spRenderClient));
    return hr;
}

HRESULT AudioOutput::StartSession()
{
    if (!m_spAudioClient) return E_FAIL;
    return m_spAudioClient->Start();
}

HRESULT AudioOutput::StopSession()
{
    if (!m_spAudioClient) return S_OK;
    return m_spAudioClient->Stop();
}

} // namespace HMRAVSource
