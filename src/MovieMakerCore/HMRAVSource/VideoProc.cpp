// VideoProc.cpp - Video processing pipeline implementation

#include "pch.h"
#include "VideoProc.h"
#include <algorithm>

namespace
{

// Row pitch (in bytes) for the luma/RGB plane of the given MF video
// subtype at the given width. Packed RGB formats use bytes-per-pixel
// times the width; YUV planar / semi-planar formats (NV12, YV12, IYUV,
// and other YUV420 variants) use a 16-byte aligned luma row. This keeps
// the CPU copy paths from over-reading past the sample buffer and lets
// them handle RGB32 input as well as YUV input.
UINT ComputeRowPitchForSubtype(const GUID& guidSubtype, UINT width)
{
    if (guidSubtype == MFVideoFormat_RGB32 || guidSubtype == MFVideoFormat_ARGB32)
        return width * 4;
    if (guidSubtype == MFVideoFormat_RGB24)
        return width * 3;
    if (guidSubtype == MFVideoFormat_RGB555 || guidSubtype == MFVideoFormat_RGB565)
        return width * 2;
    return (width + 15) & ~15u;
}

} // namespace

namespace HMRAVSource
{

// ============================================================================
// XVideoProc
// ============================================================================

XVideoProc::XVideoProc()
    : m_fInitialized(false)
    , m_dwFramesProcessed(0)
    , m_effectType(VideoEffectNone)
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

    CComPtr<IMFSample> spOutput;
    HRESULT hr = ProcessFrame(pInputSample, &spOutput);
    if (FAILED(hr))
        return hr;

    if (!spOutput)
        return E_UNEXPECTED;

    CComPtr<IMFMediaBuffer> spBuffer;
    hr = spOutput->GetBufferByIndex(0, &spBuffer);
    if (FAILED(hr))
        return hr;

    BYTE* pData = nullptr;
    DWORD cbLength = 0;
    hr = spBuffer->Lock(&pData, nullptr, &cbLength);
    if (FAILED(hr))
        return hr;

    D3DLOCKED_RECT lockedRect;
    hr = pOutputSurface->LockRect(&lockedRect, nullptr, 0);
    if (SUCCEEDED(hr))
    {
        D3DSURFACE_DESC desc;
        pOutputSurface->GetDesc(&desc);

        UINT srcWidth = m_desc.uOutputWidth;
        UINT srcHeight = m_desc.uOutputHeight;
        UINT srcPitch = ComputeRowPitchForSubtype(m_desc.guidOutputSubtype, srcWidth);

        // Bound the number of rows by what is actually present in the
        // sample buffer so we never read past the end of the data.
        UINT srcRows = srcHeight;
        if (srcPitch > 0)
        {
            UINT rowsInBuffer = cbLength / srcPitch;
            if (rowsInBuffer < srcRows)
                srcRows = rowsInBuffer;
        }

        for (UINT y = 0; y < srcRows && y < desc.Height; ++y)
        {
            BYTE* pSrc = pData + y * srcPitch;
            BYTE* pDst = static_cast<BYTE*>(lockedRect.pBits) + y * lockedRect.Pitch;
            memcpy(pDst, pSrc, std::min(srcPitch, static_cast<UINT>(lockedRect.Pitch)));
        }

        pOutputSurface->UnlockRect();
    }

    spBuffer->Unlock();
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

HRESULT XVideoProc::SetEffect(VideoEffectType effectType)
{
    m_effectType = effectType;
    return S_OK;
}

XVideoProc::VideoEffectType XVideoProc::GetEffect() const throw()
{
    return m_effectType;
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
    if (!m_spColorConverter)
        return E_UNEXPECTED;

    HRESULT hr = S_OK;

    CComPtr<IMFMediaType> spInputType;
    hr = MFCreateMediaType(&spInputType);
    if (FAILED(hr))
        return hr;

    spInputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    spInputType->SetGUID(MF_MT_SUBTYPE, m_desc.guidInputSubtype);
    spInputType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
    MFSetAttributeSize(spInputType, MF_MT_FRAME_SIZE, m_desc.uInputWidth, m_desc.uInputHeight);

    hr = m_spColorConverter->SetInputType(0, spInputType, 0);
    if (FAILED(hr))
        return hr;

    CComPtr<IMFMediaType> spOutputType;
    hr = MFCreateMediaType(&spOutputType);
    if (FAILED(hr))
        return hr;

    spOutputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    spOutputType->SetGUID(MF_MT_SUBTYPE, m_desc.guidOutputSubtype);
    spOutputType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
    MFSetAttributeSize(spOutputType, MF_MT_FRAME_SIZE, m_desc.uOutputWidth, m_desc.uOutputHeight);

    hr = m_spColorConverter->SetOutputType(0, spOutputType, 0);
    if (FAILED(hr))
        return hr;

    hr = m_spColorConverter->ProcessMessage(MFT_MESSAGE_COMMAND_FLUSH, 0);
    if (FAILED(hr))
        return hr;

    hr = m_spColorConverter->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, 0);
    return hr;
}

HRESULT XVideoProc::ProcessThroughChain(IMFSample* pInput, IMFSample** ppOutput)
{
    if (!pInput || !ppOutput)
        return E_POINTER;

    *ppOutput = nullptr;

    if (!m_spColorConverter)
    {
        *ppOutput = pInput;
        (*ppOutput)->AddRef();
        return S_OK;
    }

    HRESULT hr = m_spColorConverter->ProcessInput(0, pInput, 0);
    if (hr == MF_E_NOTACCEPTING)
    {
        *ppOutput = pInput;
        (*ppOutput)->AddRef();
        return S_OK;
    }
    if (FAILED(hr))
        return hr;

    MFT_OUTPUT_STREAM_INFO streamInfo = {};
    hr = m_spColorConverter->GetOutputStreamInfo(0, &streamInfo);
    if (FAILED(hr))
        return hr;

    CComPtr<IMFSample> spOutputSample;
    hr = MFCreateSample(&spOutputSample);
    if (FAILED(hr))
        return hr;

    CComPtr<IMFMediaBuffer> spOutputBuffer;
    hr = MFCreateMemoryBuffer(streamInfo.cbSize, &spOutputBuffer);
    if (FAILED(hr))
        return hr;

    hr = spOutputSample->AddBuffer(spOutputBuffer);
    if (FAILED(hr))
        return hr;

    MFT_OUTPUT_DATA_BUFFER outputData = {};
    outputData.dwStreamID = 0;
    outputData.pSample = spOutputSample;

    DWORD dwStatus = 0;
    hr = m_spColorConverter->ProcessOutput(0, 1, &outputData, &dwStatus);
    if (hr == MF_E_TRANSFORM_NEED_MORE_INPUT)
    {
        *ppOutput = pInput;
        (*ppOutput)->AddRef();
        return S_OK;
    }
    if (FAILED(hr))
        return hr;

    *ppOutput = outputData.pSample;
    return S_OK;
}

// ============================================================================
// DXVA2VideoProc
// ============================================================================

DXVA2VideoProc::DXVA2VideoProc()
    : m_pDevice(nullptr)
    , m_pVideoProcessor(nullptr)
    , m_spEnumerator(NULL)
    , m_spRenderTarget(NULL)
    , m_spTempSurface(NULL)
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

    // Release any resources from a previous initialization so the device
    // and processor are not leaked when Initialize is called again.
    Shutdown();

    m_pDevice = static_cast<IDirect3DDevice9Ex*>(pDevice);

    HRESULT hr = XVideoProc::Initialize(desc);
    if (FAILED(hr))
        return hr;

    hr = CreateDXVA2Processor();
    if (FAILED(hr))
    {
        Shutdown();
        return hr;
    }

    hr = CreateTempSurface();
    if (FAILED(hr))
    {
        Shutdown();
        return hr;
    }

    return S_OK;
}

HRESULT DXVA2VideoProc::Shutdown()
{
    if (m_pVideoProcessor)
    {
        m_pVideoProcessor->Release();
        m_pVideoProcessor = nullptr;
    }
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

    if (!m_fInitialized || !m_pVideoProcessor)
        return E_UNEXPECTED;

    return ProcessSampleDXVA2(pInputSample, ppOutputSample);
}

HRESULT DXVA2VideoProc::ProcessFrameToSurface(IMFSample* pInputSample, IDirect3DSurface9* pOutputSurface)
{
    if (!pInputSample || !pOutputSurface)
        return E_POINTER;

    if (!m_pVideoProcessor)
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

    DXVA2_VideoProcessorDesc vpDesc = {};
    vpDesc.DeviceFrameWidth = m_desc.uInputWidth;
    vpDesc.DeviceFrameHeight = m_desc.uInputHeight;
    vpDesc.TargetFrameWidth = m_desc.uOutputWidth;
    vpDesc.TargetFrameHeight = m_desc.uOutputHeight;
    vpDesc.TargetMinWidth = 0;
    vpDesc.TargetMinHeight = 0;
    vpDesc.TargetMaxWidth = m_desc.uOutputWidth;
    vpDesc.TargetMaxHeight = m_desc.uOutputHeight;
    vpDesc.Usage = DXVA2_VPDev_HardwareDeinterlaceOrScaling;
    vpDesc.FrameRate.Numerator = 30;
    vpDesc.FrameRate.Denominator = 1;
    vpDesc.Uid = 0;

    HRESULT hr = DXVA2CreateVideoProcessorEnumerator(
        &vpDesc,
        &m_spEnumerator);

    if (FAILED(hr))
        return hr;

    if (!m_spEnumerator)
        return E_UNEXPECTED;

    hr = m_spEnumerator.p->GetVideoProcessorCaps(&m_vpcaps);
    if (FAILED(hr))
        return hr;

    // Zero the output pointer before creation so a failed creation can
    // never leave a stale or indeterminate processor behind.
    m_pVideoProcessor = nullptr;
    hr = DXVA2CreateVideoProcessor(
        m_pDevice,
        m_spEnumerator,
        &m_vpcaps,
        &m_pVideoProcessor);
    if (SUCCEEDED(hr) && !m_pVideoProcessor)
        return E_UNEXPECTED;

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

    *ppOutput = nullptr;

    if (!m_pVideoProcessor)
        return E_UNEXPECTED;

    CComPtr<IMFMediaBuffer> spInputBuffer;
    HRESULT hr = pInput->GetBufferByIndex(0, &spInputBuffer);
    if (FAILED(hr))
        return hr;

    BYTE* pInputData = nullptr;
    DWORD cbInputLength = 0;
    hr = spInputBuffer->Lock(&pInputData, nullptr, &cbInputLength);
    if (FAILED(hr))
        return hr;

    // Copy input data to the temp surface
    if (m_spTempSurface && pInputData && cbInputLength > 0)
    {
        D3DLOCKED_RECT lockedRect = {};
        if (SUCCEEDED(m_spTempSurface->LockRect(&lockedRect, nullptr, 0)))
        {
            UINT srcWidth = m_desc.uInputWidth;
            UINT srcHeight = m_desc.uInputHeight;
            UINT srcPitch = ComputeRowPitchForSubtype(m_desc.guidInputSubtype, srcWidth);

            // Bound the number of rows by the input buffer length.
            UINT srcRows = srcHeight;
            if (srcPitch > 0)
            {
                UINT rowsInBuffer = cbInputLength / srcPitch;
                if (rowsInBuffer < srcRows)
                    srcRows = rowsInBuffer;
            }

            for (UINT y = 0; y < srcRows; ++y)
            {
                BYTE* pSrc = pInputData + y * srcPitch;
                BYTE* pDst = static_cast<BYTE*>(lockedRect.pBits) + y * lockedRect.Pitch;
                memcpy(pDst, pSrc, std::min(srcPitch, static_cast<UINT>(lockedRect.Pitch)));
            }

            m_spTempSurface->UnlockRect();
        }
    }

    spInputBuffer->Unlock();

    // Use DXVA2 processor to blit from temp surface to render target
    IDirect3DSurface9* pRenderTarget = m_spRenderTarget ? m_spRenderTarget.p : m_spTempSurface.p;
    if (pRenderTarget && m_spTempSurface)
    {
        DXVA2_VideoProcessBltParameters blt = {};
        blt.TargetRect = { 0, 0, static_cast<LONG>(m_desc.uOutputWidth), static_cast<LONG>(m_desc.uOutputHeight) };
        blt.SourceRect = { 0, 0, static_cast<LONG>(m_desc.uInputWidth), static_cast<LONG>(m_desc.uInputHeight) };
        blt.TargetFrame = DXVA2_Fixed32FromDouble(0.0);
        blt.BackgroundColor = 0;
        blt.StreamRect = blt.TargetRect;
        blt.Alpha = DXVA2_Fixed32Opaque();

        m_pVideoProcessor->ProcessBlt(pRenderTarget, &blt, nullptr, 0, nullptr, nullptr);
    }

    // Create output sample from the processed surface
    DWORD cbOutputSize = m_desc.uOutputWidth * m_desc.uOutputHeight * 4;
    CComPtr<IMFSample> spOutputSample;
    hr = MFCreateSample(&spOutputSample);
    if (FAILED(hr))
        return hr;

    CComPtr<IMFMediaBuffer> spOutputBuffer;
    hr = MFCreateMemoryBuffer(cbOutputSize, &spOutputBuffer);
    if (FAILED(hr))
        return hr;

    hr = spOutputSample->AddBuffer(spOutputBuffer);
    if (FAILED(hr))
        return hr;

    // Copy processed surface data to output sample
    if (pRenderTarget)
    {
        D3DLOCKED_RECT lockedRect = {};
        if (SUCCEEDED(pRenderTarget->LockRect(&lockedRect, nullptr, D3DLOCK_READONLY)))
        {
            BYTE* pDstData = nullptr;
            DWORD cbDstMax = 0;
            spOutputBuffer->Lock(&pDstData, &cbDstMax, nullptr);
            if (pDstData)
            {
                UINT copyH = m_desc.uOutputHeight;
                for (UINT y = 0; y < copyH; ++y)
                {
                    memcpy(pDstData + y * m_desc.uOutputWidth * 4,
                           static_cast<BYTE*>(lockedRect.pBits) + y * lockedRect.Pitch,
                           std::min(static_cast<UINT>(m_desc.uOutputWidth * 4), static_cast<UINT>(lockedRect.Pitch)));
                }
                spOutputBuffer->Unlock();
            }
            spOutputBuffer->SetCurrentLength(m_desc.uOutputWidth * m_desc.uOutputHeight * 4);
            pRenderTarget->UnlockRect();
        }
    }

    LONGLONG llTimestamp = 0;
    pInput->GetSampleTime(&llTimestamp);
    spOutputSample->SetSampleTime(llTimestamp);

    LONGLONG llDuration = 0;
    pInput->GetSampleDuration(&llDuration);
    if (llDuration > 0)
        spOutputSample->SetSampleDuration(llDuration);

    m_dwFramesProcessed++;
    *ppOutput = spOutputSample.Detach();
    return S_OK;
}

HRESULT DXVA2VideoProc::ConvertSampleToSurface(IMFSample* pSample, IDirect3DSurface9* pSurface)
{
    if (!pSample || !pSurface)
        return E_POINTER;

    CComPtr<IMFMediaBuffer> spBuffer;
    HRESULT hr = pSample->GetBufferByIndex(0, &spBuffer);
    if (FAILED(hr))
        return hr;

    BYTE* pData = nullptr;
    DWORD cbLength = 0;
    hr = spBuffer->Lock(&pData, nullptr, &cbLength);
    if (FAILED(hr))
        return hr;

    D3DLOCKED_RECT lockedRect;
    hr = pSurface->LockRect(&lockedRect, nullptr, 0);
    if (SUCCEEDED(hr))
    {
        D3DSURFACE_DESC desc;
        pSurface->GetDesc(&desc);

        UINT srcWidth = m_desc.uInputWidth;
        UINT srcHeight = m_desc.uInputHeight;
        UINT dstPitch = lockedRect.Pitch;
        UINT srcPitch = ComputeRowPitchForSubtype(m_desc.guidInputSubtype, srcWidth);

        // Bound the number of rows by the input buffer length.
        UINT srcRows = srcHeight;
        if (srcPitch > 0)
        {
            UINT rowsInBuffer = cbLength / srcPitch;
            if (rowsInBuffer < srcRows)
                srcRows = rowsInBuffer;
        }

        for (UINT y = 0; y < srcRows && y < desc.Height; ++y)
        {
            BYTE* pSrc = pData + y * srcPitch;
            BYTE* pDst = static_cast<BYTE*>(lockedRect.pBits) + y * dstPitch;
            memcpy(pDst, pSrc, std::min(srcPitch, static_cast<UINT>(dstPitch)));
        }

        pSurface->UnlockRect();
    }

    spBuffer->Unlock();
    return hr;
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

    D3DLOCKED_RECT lockedRect = {};
    hr = pSurface->LockRect(&lockedRect, nullptr, D3DLOCK_READONLY);
    if (SUCCEEDED(hr))
    {
        BYTE* pDstData = nullptr;
        DWORD cbDstMax = 0;
        hr = spBuffer->Lock(&pDstData, &cbDstMax, nullptr);
        if (SUCCEEDED(hr) && pDstData)
        {
            for (UINT y = 0; y < desc.Height; ++y)
            {
                memcpy(pDstData + y * desc.Width * 4,
                       static_cast<BYTE*>(lockedRect.pBits) + y * lockedRect.Pitch,
                       std::min(static_cast<UINT>(desc.Width * 4), static_cast<UINT>(lockedRect.Pitch)));
            }
            spBuffer->Unlock();
        }
        spBuffer->SetCurrentLength(cbSize);
        pSurface->UnlockRect();
    }

    *ppSample = spSample.Detach();
    return S_OK;
}

} // namespace HMRAVSource
