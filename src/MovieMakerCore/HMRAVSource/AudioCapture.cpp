// AudioCapture.cpp - Audio capture (narration/microphone) implementation

#include "pch.h"
#include "AudioCapture.h"

namespace HMRAVSource
{

// ============================================================================
// Construction / Destruction
// ============================================================================

AudioCapture::AudioCapture()
    : m_state(CaptureStateIdle)
    , m_flVolume(1.0f)
    , m_fMuted(false)
{
}

AudioCapture::~AudioCapture()
{
    Shutdown();
}

// ============================================================================
// Device enumeration
// ============================================================================

HRESULT AudioCapture::EnumDevices(ATL::CAtlArray<AudioCaptureDeviceInfo>& devices)
{
    devices.RemoveAll();

    CComPtr<IMFAttributes> spAttributes;
    HRESULT hr = MFCreateAttributes(&spAttributes, 1);
    if (FAILED(hr))
        return hr;

    hr = spAttributes->SetGUID(
        MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
        MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID);
    if (FAILED(hr))
        return hr;

    UINT32 cDevices = 0;
    hr = MFEnumDeviceSources(spAttributes, nullptr, &cDevices);
    if (FAILED(hr) && hr != MF_E_NOT_FOUND)
        return hr;

    if (cDevices == 0)
        return S_OK;

    ATL::CAtlArray<IMFActivate*> arrActivates;
    arrActivates.SetCount(cDevices);
    hr = MFEnumDeviceSources(spAttributes, arrActivates.GetData(), &cDevices);
    if (FAILED(hr))
        return hr;

    for (UINT32 i = 0; i < cDevices; ++i)
    {
        AudioCaptureDeviceInfo info;

        WCHAR szFriendlyName[256] = {};
        UINT32 cchName = 256;
        arrActivates[i]->GetString(
            MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, szFriendlyName, cchName, &cchName);
        info.strDeviceName = szFriendlyName;

        WCHAR szDeviceId[512] = {};
        UINT32 cchId = 512;
        arrActivates[i]->GetString(
            MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_SYMBOLIC_LINK,
            szDeviceId, cchId, &cchId);
        info.strDeviceId = szDeviceId;

        info.dwMaxChannels = 2;
        info.dwMaxSampleRate = 48000;
        info.dwDefaultSampleRate = 44100;
        info.dwDefaultChannels = 1;

        devices.Add(info);

        arrActivates[i]->Release();
    }

    return S_OK;
}

HRESULT AudioCapture::GetDefaultDevice(AudioCaptureDeviceInfo* pInfo)
{
    if (!pInfo)
        return E_POINTER;

    ATL::CAtlArray<AudioCaptureDeviceInfo> devices;
    HRESULT hr = EnumDevices(devices);
    if (FAILED(hr))
        return hr;

    if (devices.GetCount() == 0)
        return MF_E_NOT_FOUND;

    *pInfo = devices[0];
    return S_OK;
}

// ============================================================================
// Lifecycle
// ============================================================================

HRESULT AudioCapture::Initialize(const AudioCaptureDeviceInfo& deviceInfo)
{
    m_deviceInfo = deviceInfo;

    HRESULT hr = CreateAudioSource();
    if (FAILED(hr))
        return hr;

    hr = ConfigureReader();
    if (FAILED(hr))
        return hr;

    m_state = CaptureStateIdle;
    return S_OK;
}

HRESULT AudioCapture::Shutdown()
{
    if (m_state == CaptureStateCapturing || m_state == CaptureStatePaused)
        StopCapture();

    if (m_spReader)
    {
        m_spReader->Flush(MF_SOURCE_READER_ALL_STREAMS);
        m_spReader = nullptr;
    }

    m_spMediaSource = nullptr;
    m_state = CaptureStateIdle;

    return S_OK;
}

// ============================================================================
// Capture control
// ============================================================================

HRESULT AudioCapture::StartCapture()
{
    if (m_state == CaptureStateCapturing)
        return S_FALSE;

    if (!m_spReader)
        return E_UNEXPECTED;

    m_state = CaptureStateCapturing;
    return S_OK;
}

HRESULT AudioCapture::StopCapture()
{
    if (m_state != CaptureStateCapturing && m_state != CaptureStatePaused)
        return S_FALSE;

    m_state = CaptureStateIdle;
    return S_OK;
}

HRESULT AudioCapture::IsCapturing() const throw()
{
    return (m_state == CaptureStateCapturing) ? S_OK : S_FALSE;
}

HRESULT AudioCapture::PauseCapture()
{
    if (m_state != CaptureStateCapturing)
        return E_UNEXPECTED;

    m_state = CaptureStatePaused;
    return S_OK;
}

HRESULT AudioCapture::ResumeCapture()
{
    if (m_state != CaptureStatePaused)
        return E_UNEXPECTED;

    m_state = CaptureStateCapturing;
    return S_OK;
}

// ============================================================================
// Configuration
// ============================================================================

HRESULT AudioCapture::SetConfig(const AudioCaptureConfig& config)
{
    m_config = config;
    return S_OK;
}

AudioCaptureConfig AudioCapture::GetConfig() const throw()
{
    return m_config;
}

// ============================================================================
// State
// ============================================================================

CaptureState AudioCapture::GetState() const throw()
{
    return m_state;
}

// ============================================================================
// Sample reading
// ============================================================================

HRESULT AudioCapture::ReadSample(IMFSample** ppSample)
{
    if (!ppSample)
        return E_POINTER;
    *ppSample = nullptr;

    if (!m_spReader || m_state != CaptureStateCapturing)
        return E_UNEXPECTED;

    DWORD dwStreamIndex = 0;
    DWORD dwStreamFlags = 0;
    LONGLONG llTimestamp = 0;
    CComPtr<IMFSample> spSample;

    HRESULT hr = m_spReader->ReadSample(
        MF_SOURCE_READER_FIRST_AUDIO_STREAM,
        0,
        nullptr,
        &dwStreamFlags,
        &llTimestamp,
        &spSample);

    if (FAILED(hr))
        return hr;

    if (dwStreamFlags & MF_SOURCE_READERF_ENDOFSTREAM)
        return MF_E_END_OFSTREAM;

    if (spSample)
        *ppSample = spSample.Detach();

    return S_OK;
}

// ============================================================================
// Volume / Mute
// ============================================================================

HRESULT AudioCapture::SetVolume(float flVolume)
{
    if (flVolume < 0.0f) flVolume = 0.0f;
    if (flVolume > 1.0f) flVolume = 1.0f;
    m_flVolume = flVolume;
    return S_OK;
}

HRESULT AudioCapture::GetVolume(float* pflVolume)
{
    if (!pflVolume)
        return E_POINTER;

    *pflVolume = m_flVolume;
    return S_OK;
}

HRESULT AudioCapture::SetMute(bool fMute)
{
    m_fMuted = fMute;
    return S_OK;
}

bool AudioCapture::IsMuted() const throw()
{
    return m_fMuted;
}

// ============================================================================
// Device info
// ============================================================================

AudioCaptureDeviceInfo AudioCapture::GetDeviceInfo() const
{
    return m_deviceInfo;
}

// ============================================================================
// Private helpers
// ============================================================================

HRESULT AudioCapture::CreateAudioSource()
{
    CComPtr<IMFAttributes> spAttributes;
    HRESULT hr = MFCreateAttributes(&spAttributes, 2);
    if (FAILED(hr))
        return hr;

    hr = spAttributes->SetGUID(
        MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
        MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID);
    if (FAILED(hr))
        return hr;

    if (!m_deviceInfo.strDeviceId.IsEmpty())
    {
        hr = spAttributes->SetString(
            MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_SYMBOLIC_LINK,
            m_deviceInfo.strDeviceId.GetString());
        if (FAILED(hr))
            return hr;
    }

    hr = MFCreateDeviceSource(spAttributes, &m_spMediaSource);
    if (FAILED(hr))
        return hr;

    hr = MFCreateSourceReaderFromMediaSource(
        m_spMediaSource,
        nullptr,
        &m_spReader);

    return hr;
}

HRESULT AudioCapture::ConfigureReader()
{
    if (!m_spReader)
        return E_UNEXPECTED;

    // Set output format to PCM
    CComPtr<IMFMediaType> spOutputType;
    HRESULT hr = MFCreateMediaType(&spOutputType);
    if (FAILED(hr))
        return hr;

    spOutputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
    spOutputType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
    spOutputType->SetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, m_config.dwSampleRate);
    spOutputType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, m_config.dwChannels);
    spOutputType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, m_config.dwBitsPerSample);
    spOutputType->SetUINT32(MF_MT_BLOCK_ALIGNMENT,
        m_config.dwChannels * (m_config.dwBitsPerSample / 8));

    hr = m_spReader->SetCurrentMediaType(
        MF_SOURCE_READER_FIRST_AUDIO_STREAM,
        nullptr,
        spOutputType);

    return hr;
}

} // namespace HMRAVSource
