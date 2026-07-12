// VideoProc.cpp - Video processing pipeline implementation

#include "pch.h"
#include "VideoProc.h"

namespace HMRAVSource
{

// ============================================================================
// XVideoProc
// ============================================================================

XVideoProc::XVideoProc()
    : m_fInitialized(false)
    , m_dwFramesProcessed(0)
{
}

XVideoProc::~XVideoProc()
{
    Shutdown();
}

HRESULT XVideoProc::Initialize(const VideoProcDesc& desc)
{
    m_desc = desc;

    HRESULT hr = CreateColorConverter();
    if (FAILED(hr))
        return hr;

    if (m_desc.uOutputWidth != m_desc.uInputWidth ||
        m_desc.uOutputHeight != m_desc.uInputHeight)
    {
        hr = CreateScaler();
        if (FAILED(hr))
            return hr;
    }

    hr = ConfigureTransformChain();
    if (FAILED(hr))
        return hr;

    m_fInitialized = true;
    return S_OK;
}

HRESULT XVideoProc::Shutdown()
{
    m_spColorConverter = nullptr;
    m_spScaler = nullptr;
    m_extraTransforms.RemoveAll();
    m_fInitialized = false;
    m_dwFramesProcessed = 0;
    return S_OK;
}

bool XVideoProc::IsInitialized() const throw()
{
    return m_fInitialized;
}

HRESULT XVideoProc::ProcessFrame(IMFSample* pInputSample, IMFSample** ppOutputSample)
{
    if (!pInputSample || !ppOutputSample)
        return E_POINTER;

    *ppOutputSample = nullptr;

    if (!m_fInitialized)
        return E_UNEXPECTED;

    HRESULT hr = ProcessThroughChain(pInputSample, ppOutputSample);
    if (SUCCEEDED(hr))
        m_dwFramesProcessed++;

    return hr;
}

HRESULT XVideoProc::ProcessFrameToSurface(IMFSample* pInputSample, IDirect3DSurface9* pOutputSurface)
{
    if (!pInputSample || !pOutputSurface)
        return E_POINTER;

    // Software processing to a D3D9 surface
    CComPtr<IMFSample> spOutput;
    HRESULT hr = ProcessFrame(pInputSample, &spOutput);
    return hr;
}

HRESULT XVideoProc::SetOutputFormat(UINT uWidth, UINT uHeight, const GUID* pSubtype)
{
    if (!pSubtype)
        return E_POINTER;

    m_desc.uOutputWidth = uWidth;
    m_desc.uOutputHeight = uHeight;
    m_desc.guidOutputSubtype = *pSubtype;

    return ConfigureTransformChain();
}

HRESULT XVideoProc::GetOutputFormat(UINT* puWidth, UINT* puHeight, GUID* pSubtype)
{
    if (puWidth) *puWidth = m_desc.uOutputWidth;
    if (puHeight) *puHeight = m_desc.uOutputHeight;
    if (pSubtype) *pSubtype = m_desc.guidOutputSubtype;
    return S_OK;
}

HRESULT XVideoProc::SetBrightness(float flBrightness)
{
    if (flBrightness < -1.0f) flBrightness = -1.0f;
    if (flBrightness > 1.0f) flBrightness = 1.0f;
    m_desc.flBrightness = flBrightness;
    m_desc.fBrightness = (flBrightness != 0.0f);
    return S_OK;
}

HRESULT XVideoProc::SetContrast(float flContrast)
{
    if (flContrast < 0.0f) flContrast = 0.0f;
    if (flContrast > 2.0f) flContrast = 2.0f;
    m_desc.flContrast = flContrast;
    return S_OK;
}

HRESULT XVideoProc::SetSaturation(float flSaturation)
{
    if (flSaturation < 0.0f) flSaturation = 0.0f;
    if (flSaturation > 3.0f) flSaturation = 3.0f;
    m_desc.flSaturation = flSaturation;
    return S_OK;
}

HRESULT XVideoProc::SetDeinterlace(bool fEnable)
{
    m_desc.fDeinterlace = fEnable;
    return S_OK;
}

HRESULT XVideoProc::SetDenoise(bool fEnable)
{
    m_desc.fDenoise = fEnable;
    return S_OK;
}

HRESULT XVideoProc::AddTransform(REFGUID guidTransformClsid)
{
    CComPtr<IMFTransform> spTransform;
    HRESULT hr = CoCreateInstance(
        guidTransformClsid,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&spTransform));

    if (FAILED(hr))
        return hr;

    m_extraTransforms.Add(spTransform);
    return S_OK;
}

HRESULT XVideoProc::RemoveTransform(DWORD dwIndex)
{
    if (dwIndex >= m_extraTransforms.GetCount())
        return E_INVALIDARG;

    m_extraTransforms.RemoveAt(dwIndex);
    return S_OK;
}

DWORD XVideoProc::GetTransformCount() const throw()
{
    return static_cast<DWORD>(m_extraTransforms.GetCount());
}

DWORD XVideoProc::GetFramesProcessed() const throw()
{
    return m_dwFramesProcessed;
}

// ============================================================================
// XVideoProc - Private helpers
// ============================================================================

HRESULT XVideoProc::CreateColorConverter()
{
    return CoCreateInstance(
        CLSID_VideoProcessorMFT,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&m_spColorConverter));
}

HRESULT XVideoProc::CreateScaler()
{
    // The MFT Video Processor handles both color conversion and scaling
    return S_OK;
}

HRESULT XVideoProc::ConfigureTransformChain()
{
    return S_OK;
}

HRESULT XVideoProc::ProcessThroughChain(IMFSample* pInput, IMFSample** ppOutput)
{
    if (!pInput || !ppOutput)
        return E_POINTER;

    *ppOutput = nullptr;

    // Pass through without processing for now
    *ppOutput = pInput;
    (*ppOutput)->AddRef();
    return S_OK;
}

// ============================================================================
// DXVA2VideoProc
// ============================================================================

DXVA2VideoProc::DXVA2VideoProc()
    : m_pDevice(nullptr)
    , m_fH264Deinterlace(false)
{
    ZeroMemory(&m_vpcaps, sizeof(m_vpcaps));
    ZeroMemory(&m_d3dpp, sizeof(m_d3dpp));
}

DXVA2VideoProc::~DXVA2VideoProc()
{
    Shutdown();
}

HRESULT DXVA2VideoProc::Initialize(const VideoProcDesc& desc, IDirect3DDevice9* pDevice)
{
    if (!pDevice)
        return E_POINTER;

    m_pDevice = static_cast<IDirect3DDevice9Ex*>(pDevice);

    HRESULT hr = XVideoProc::Initialize(desc);
    if (FAILED(hr))
        return hr;

    hr = CreateDXVA2Processor();
    if (FAILED(hr))
        return hr;

    hr = CreateTempSurface();
    if (FAILED(hr))
        return hr;

    return S_OK;
}

HRESULT DXVA2VideoProc::Shutdown()
{
    m_spVideoProcessor = nullptr;
    m_spEnumerator = nullptr;
    m_spRenderTarget = nullptr;
    m_spTempSurface = nullptr;
    m_pDevice = nullptr;

    return XVideoProc::Shutdown();
}

HRESULT DXVA2VideoProc::ProcessFrame(IMFSample* pInputSample, IMFSample** ppOutputSample)
{
    if (!pInputSample || !ppOutputSample)
        return E_POINTER;

    *ppOutputSample = nullptr;

    if (!m_fInitialized || !m_spVideoProcessor)
        return E_UNEXPECTED;

    return ProcessSampleDXVA2(pInputSample, ppOutputSample);
}

HRESULT DXVA2VideoProc::ProcessFrameToSurface(IMFSample* pInputSample, IDirect3DSurface9* pOutputSurface)
{
    if (!pInputSample || !pOutputSurface)
        return E_POINTER;

    if (!m_spVideoProcessor)
        return E_UNEXPECTED;

    // Convert input to DXVA2 surface and process
    return ConvertSampleToSurface(pInputSample, pOutputSurface);
}

HRESULT DXVA2VideoProc::GetVideoProcessorCaps(DXVA2_VideoProcessorCaps* pCaps)
{
    if (!pCaps)
        return E_POINTER;

    *pCaps = m_vpcaps;
    return S_OK;
}

HRESULT DXVA2VideoProc::SetRenderTarget(IDirect3DSurface9* pRenderTarget)
{
    m_spRenderTarget = pRenderTarget;
    return S_OK;
}

HRESULT DXVA2VideoProc::EnableH264Deinterlace(bool fEnable)
{
    m_fH264Deinterlace = fEnable;
    return S_OK;
}

HRESULT DXVA2VideoProc::SetDevice(IDirect3DDevice9* pDevice)
{
    if (!pDevice)
        return E_POINTER;

    m_pDevice = static_cast<IDirect3DDevice9Ex*>(pDevice);
    return S_OK;
}

IDirect3DDevice9* DXVA2VideoProc::GetDevice() const
{
    return m_pDevice;
}

bool DXVA2VideoProc::IsDXVA2Available()
{
    return true;
}

bool DXVA2VideoProc::IsHardwareDeinterlaceSupported(IDirect3DDevice9* /*pDevice*/)
{
    return true;
}

bool DXVA2VideoProc::IsHardwareScalingSupported(IDirect3DDevice9* /*pDevice*/)
{
    return true;
}

// ============================================================================
// DXVA2VideoProc - Private helpers
// ============================================================================

HRESULT DXVA2VideoProc::CreateDXVA2Processor()
{
    if (!m_pDevice)
        return E_UNEXPECTED;

    HRESULT hr = DXVA2CreateVideoProcessorEnumerator(
        nullptr,  // TODO: fill DXVA2_VideoProcessorDesc
        &m_spEnumerator);

    if (FAILED(hr))
        return hr;

    hr = m_spEnumerator->GetVideoProcessorCaps(&m_vpcaps);
    if (FAILED(hr))
        return hr;

    hr = DXVA2CreateVideoProcessor(
        m_pDevice,
        m_spEnumerator,
        &m_vpcaps,
        &m_spVideoProcessor);

    return hr;
}

HRESULT DXVA2VideoProc::CreateTempSurface()
{
    if (!m_pDevice)
        return E_UNEXPECTED;

    D3DSURFACE_DESC desc = {};
    desc.Width = m_desc.uOutputWidth;
    desc.Height = m_desc.uOutputHeight;
    desc.Format = D3DFMT_A8R8G8B8;
    desc.Type = D3DRTYPE_SURFACE;
    desc.Usage = D3DUSAGE_RENDERTARGET;
    desc.Pool = D3DPOOL_DEFAULT;

    return m_pDevice->CreateRenderTarget(
        desc.Width,
        desc.Height,
        desc.Format,
        desc.MultiSampleType,
        desc.MultiSampleQuality,
        FALSE,
        &m_spTempSurface,
        nullptr);
}

HRESULT DXVA2VideoProc::ProcessSampleDXVA2(IMFSample* pInput, IMFSample** ppOutput)
{
    if (!pInput || !ppOutput)
        return E_POINTER;

    // Simplified DXVA2 processing path
    *ppOutput = nullptr;
    pInput->AddRef();
    *ppOutput = pInput;
    m_dwFramesProcessed++;
    return S_OK;
}

HRESULT DXVA2VideoProc::ConvertSampleToSurface(IMFSample* pSample, IDirect3DSurface9* pSurface)
{
    if (!pSample || !pSurface)
        return E_POINTER;

    // Would lock the surface, copy sample data, and unlock
    return S_OK;
}

HRESULT DXVA2VideoProc::ConvertSurfaceToSample(IDirect3DSurface9* pSurface, IMFSample** ppSample)
{
    if (!pSurface || !ppSample)
        return E_POINTER;

    *ppSample = nullptr;

    D3DSURFACE_DESC desc = {};
    HRESULT hr = pSurface->GetDesc(&desc);
    if (FAILED(hr))
        return hr;

    CComPtr<IMFSample> spSample;
    hr = MFCreateSample(&spSample);
    if (FAILED(hr))
        return hr;

    DWORD cbSize = desc.Width * desc.Height * 4;
    CComPtr<IMFMediaBuffer> spBuffer;
    hr = MFCreateMemoryBuffer(cbSize, &spBuffer);
    if (FAILED(hr))
        return hr;

    hr = spSample->AddBuffer(spBuffer);
    if (FAILED(hr))
        return hr;

    *ppSample = spSample.Detach();
    return S_OK;
}

} // namespace HMRAVSource
