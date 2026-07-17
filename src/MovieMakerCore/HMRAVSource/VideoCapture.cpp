// VideoCapture.cpp - Video capture (webcam) implementation

#include "pch.h"
#include "VideoCapture.h"

namespace HMRAVSource
{

// ============================================================================
// Construction / Destruction
// ============================================================================

VideoCapture::VideoCapture()
    : m_state(CaptureStateIdle)
    , m_hPreviewWnd(nullptr)
    , m_fPreviewing(false)
    , m_fCapturing(false)
{
}

VideoCapture::~VideoCapture()
{
    Shutdown();
}

// ============================================================================
// Device enumeration
// ============================================================================

HRESULT VideoCapture::EnumDevices(ATL::CAtlArray<VideoCaptureDeviceInfo>& devices)
{
    devices.RemoveAll();

    CComPtr<IMFAttributes> spAttributes;
    HRESULT hr = MFCreateAttributes(&spAttributes, 1);
    if (FAILED(hr))
        return hr;

    hr = spAttributes->SetGUID(
        MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
        MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
    if (FAILED(hr))
        return hr;

    UINT32 cDevices = 0;
    hr = MFEnumDeviceSources(spAttributes, nullptr, &cDevices);
    if (FAILED(hr) && hr != MF_E_NOT_FOUND)
        return hr;

    if (cDevices == 0)
        return S_OK;

    IMFActivate** ppActivates = nullptr;
    hr = MFEnumDeviceSources(spAttributes, &ppActivates, &cDevices);
    if (FAILED(hr))
        return hr;

    for (UINT32 i = 0; i < cDevices; ++i)
    {
        VideoCaptureDeviceInfo info;

        WCHAR szFriendlyName[256] = {};
        UINT32 cchName = 256;
        ppActivates[i]->GetString(
            MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, szFriendlyName, cchName, &cchName);
        info.strDeviceName = szFriendlyName;

        WCHAR szDevicePath[512] = {};
        UINT32 cchPath = 512;
        ppActivates[i]->GetString(
            MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK,
            szDevicePath, cchPath, &cchPath);
        info.strDevicePath = szDevicePath;

        info.fHasPreview = true;
        info.fHasCapture = true;
        info.uMaxWidth = 1920;
        info.uMaxHeight = 1080;
        info.dblMaxFrameRate = 30.0;

        devices.Add(info);

        ppActivates[i]->Release();
    }
    CoTaskMemFree(ppActivates);

    return S_OK;
}

HRESULT VideoCapture::GetDefaultDevice(VideoCaptureDeviceInfo* pInfo)
{
    if (!pInfo)
        return E_POINTER;

    ATL::CAtlArray<VideoCaptureDeviceInfo> devices;
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

HRESULT VideoCapture::Initialize(const VideoCaptureDeviceInfo& deviceInfo)
{
    m_deviceInfo = deviceInfo;

    HRESULT hr = CreateCaptureEngine();
    if (FAILED(hr))
        return hr;

    hr = ConfigureCaptureEngine();
    if (FAILED(hr))
        return hr;

    m_state = CaptureStateIdle;
    return S_OK;
}

HRESULT VideoCapture::Shutdown()
{
    if (m_fPreviewing)
        StopPreview();

    if (m_fCapturing)
        StopCapture();

    if (m_spCaptureEngine)
    {
        m_spCaptureEngine.p->StopPreview();
        m_spCaptureEngine.p->StopRecord(TRUE, TRUE);
        m_spCaptureEngine = nullptr;
    }

    m_spMediaSource = nullptr;
    m_spPreviewSink = nullptr;
    m_spRecordSink = nullptr;
    m_state = CaptureStateIdle;

    return S_OK;
}

// ============================================================================
// Preview
// ============================================================================

HRESULT VideoCapture::StartPreview(HWND hWnd)
{
    if (m_fPreviewing)
        return S_FALSE;

    m_hPreviewWnd = hWnd;
    m_fPreviewing = true;
    m_state = CaptureStatePreviewing;
    return S_OK;
}

HRESULT VideoCapture::StopPreview()
{
    if (!m_fPreviewing)
        return S_FALSE;

    m_fPreviewing = false;
    m_hPreviewWnd = nullptr;

    if (!m_fCapturing)
        m_state = CaptureStateIdle;

    return S_OK;
}

HRESULT VideoCapture::IsPreviewing() const throw()
{
    return m_fPreviewing ? S_OK : S_FALSE;
}

// ============================================================================
// Capture
// ============================================================================

HRESULT VideoCapture::StartCapture(LPCWSTR pszOutputPath)
{
    if (m_fCapturing)
        return S_FALSE;

    if (!pszOutputPath)
        return E_POINTER;

    m_fCapturing = true;
    m_state = CaptureStateCapturing;
    return S_OK;
}

HRESULT VideoCapture::StopCapture()
{
    if (!m_fCapturing)
        return S_FALSE;

    m_fCapturing = false;

    if (!m_fPreviewing)
        m_state = CaptureStateIdle;
    else
        m_state = CaptureStatePreviewing;

    return S_OK;
}

HRESULT VideoCapture::IsCapturing() const throw()
{
    return m_fCapturing ? S_OK : S_FALSE;
}

// ============================================================================
// Single frame capture
// ============================================================================

HRESULT VideoCapture::CaptureFrame(IMFSample** ppSample)
{
    if (!ppSample)
        return E_POINTER;
    *ppSample = nullptr;

    if (m_state == CaptureStateIdle)
        return E_UNEXPECTED;

    return S_OK;
}

// ============================================================================
// Configuration
// ============================================================================

HRESULT VideoCapture::SetConfig(const VideoCaptureConfig& config)
{
    m_config = config;
    return S_OK;
}

VideoCaptureConfig VideoCapture::GetConfig() const throw()
{
    return m_config;
}

// ============================================================================
// State / Device info
// ============================================================================

CaptureState VideoCapture::GetState() const throw()
{
    return m_state;
}

VideoCaptureDeviceInfo VideoCapture::GetDeviceInfo() const
{
    return m_deviceInfo;
}

HRESULT VideoCapture::SetCaptureResolution(UINT uWidth, UINT uHeight)
{
    m_config.uRequestedWidth = uWidth;
    m_config.uRequestedHeight = uHeight;
    return S_OK;
}

HRESULT VideoCapture::GetCaptureResolution(UINT* puWidth, UINT* puHeight)
{
    if (!puWidth || !puHeight)
        return E_POINTER;

    *puWidth = m_config.uRequestedWidth;
    *puHeight = m_config.uRequestedHeight;
    return S_OK;
}

// ============================================================================
// Private helpers
// ============================================================================

HRESULT VideoCapture::CreateCaptureEngine()
{
    HRESULT hr = CoCreateInstance(
        CLSID_CaptureEngineManager, nullptr, CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&m_spCaptureEngine));

    // If CaptureEngine is not available, create a media source from the device
    if (FAILED(hr))
    {
        CComPtr<IMFAttributes> spAttributes;
        hr = MFCreateAttributes(&spAttributes, 2);
        if (FAILED(hr))
            return hr;

        hr = spAttributes->SetGUID(
            MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
            MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
        if (FAILED(hr))
            return hr;

        hr = spAttributes->SetString(
            MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK,
            m_deviceInfo.strDevicePath.GetString());
        if (FAILED(hr))
            return hr;

        hr = MFCreateDeviceSource(spAttributes, &m_spMediaSource);
    }

    return hr;
}

HRESULT VideoCapture::ConfigureCaptureEngine()
{
    if (!m_spCaptureEngine && !m_spMediaSource)
        return E_UNEXPECTED;

    CComPtr<IMFMediaType> spMediaType;
    HRESULT hr = FindBestMediaType(spMediaType);
    if (FAILED(hr))
        return hr;

    if (spMediaType && m_spMediaSource)
    {
        CComPtr<IMFSourceReader> spReader;
        hr = MFCreateSourceReaderFromMediaSource(m_spMediaSource, nullptr, &spReader);
        if (SUCCEEDED(hr))
        {
            hr = spReader->SetCurrentMediaType(
                MF_SOURCE_READER_FIRST_VIDEO_STREAM, nullptr, spMediaType);
        }
    }

    return S_OK;
}

HRESULT VideoCapture::FindBestMediaType(CComPtr<IMFMediaType>& spType)
{
    spType = nullptr;

    if (!m_spMediaSource)
        return E_UNEXPECTED;

    CComPtr<IMFSourceReader> spReader;
    HRESULT hr = MFCreateSourceReaderFromMediaSource(m_spMediaSource, nullptr, &spReader);
    if (FAILED(hr))
        return hr;

    DWORD dwBestScore = 0;

    for (DWORD dwTypeIndex = 0; ; ++dwTypeIndex)
    {
        CComPtr<IMFMediaType> spCandidate;
        hr = spReader->GetNativeMediaType(
            MF_SOURCE_READER_FIRST_VIDEO_STREAM, dwTypeIndex, &spCandidate);
        if (hr == MF_E_NO_MORE_TYPES)
            break;
        if (FAILED(hr))
            continue;

        UINT32 uWidth = 0, uHeight = 0;
        MFGetAttributeSize(spCandidate, MF_MT_FRAME_SIZE, &uWidth, &uHeight);

        double dblFrameRate = 0.0;
        UINT32 uNumerator = 0, uDenominator = 1;
        if (SUCCEEDED(MFGetAttributeRatio(spCandidate, MF_MT_FRAME_RATE, &uNumerator, &uDenominator))
            && uDenominator > 0)
        {
            dblFrameRate = static_cast<double>(uNumerator) / static_cast<double>(uDenominator);
        }

        DWORD dwScore = 0;
        if (uWidth == m_config.uRequestedWidth && uHeight == m_config.uRequestedHeight)
            dwScore += 1000;
        else
            dwScore += abs(static_cast<int>(uWidth) - static_cast<int>(m_config.uRequestedWidth)) +
                       abs(static_cast<int>(uHeight) - static_cast<int>(m_config.uRequestedHeight));

        double dblDiff = fabs(dblFrameRate - m_config.dblRequestedFrameRate);
        if (dblDiff < 1.0)
            dwScore += 500;
        else if (dblDiff < 5.0)
            dwScore += 200;

        if (dwScore > dwBestScore)
        {
            dwBestScore = dwScore;
            spType = spCandidate;
        }
    }

    if (!spType)
    {
        hr = spReader->GetNativeMediaType(
            MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, &spType);
    }

    return spType ? S_OK : MF_E_NO_MORE_TYPES;
}

} // namespace HMRAVSource
