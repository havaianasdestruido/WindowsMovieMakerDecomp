// TextureInterop.cpp - GPU texture sharing implementation

#include "pch.h"
#include "TextureInterop.h"
#include <algorithm>
#include <d3d11.h>

namespace HMRAVSource
{

// ============================================================================
// TextureInterOp (base)
// ============================================================================

TextureInterOp::TextureInterOp()
    : m_fInitialized(false)
    , m_hSharedHandle(nullptr)
{
}

TextureInterOp::~TextureInterOp()
{
    Shutdown();
}

HRESULT TextureInterOp::Initialize(const TextureInteropDesc& desc)
{
    m_desc = desc;
    m_fInitialized = true;
    return S_OK;
}

HRESULT TextureInterOp::Shutdown()
{
    m_spTexture = nullptr;
    m_spSurface = nullptr;
    m_fInitialized = false;
    return S_OK;
}

HRESULT TextureInterOp::CreateTexture(UINT uWidth, UINT uHeight, D3DFORMAT format)
{
    m_desc.uWidth = uWidth;
    m_desc.uHeight = uHeight;
    m_desc.d3dFormat = format;
    return CreateTextureInternal();
}

HRESULT TextureInterOp::LockRect(UINT uSubresource, void** ppData, UINT* pPitch, DWORD dwFlags)
{
    UNREFERENCED_PARAMETER(uSubresource);
    UNREFERENCED_PARAMETER(dwFlags);

    if (!ppData)
        return E_POINTER;

    *ppData = nullptr;
    if (pPitch) *pPitch = 0;

    if (m_spTexture)
    {
        D3DLOCKED_RECT lockedRect = {};
        HRESULT hr = m_spTexture->LockRect(uSubresource, &lockedRect, nullptr, dwFlags);
        if (SUCCEEDED(hr))
        {
            *ppData = lockedRect.pBits;
            if (pPitch) *pPitch = lockedRect.Pitch;
        }
        return hr;
    }

    return E_UNEXPECTED;
}

HRESULT TextureInterOp::UnlockRect(UINT uSubresource)
{
    if (m_spTexture)
        return m_spTexture->UnlockRect(uSubresource);
    return E_UNEXPECTED;
}

HRESULT TextureInterOp::GetSurface(IDirect3DSurface9** ppSurface)
{
    if (!ppSurface)
        return E_POINTER;

    *ppSurface = nullptr;

    if (m_spSurface)
    {
        *ppSurface = m_spSurface;
        (*ppSurface)->AddRef();
        return S_OK;
    }

    if (m_spTexture)
    {
        HRESULT hr = m_spTexture->GetSurfaceLevel(0, ppSurface);
        return hr;
    }

    return E_UNEXPECTED;
}

HRESULT TextureInterOp::CopyFromSample(IMFSample* pSample)
{
    if (!pSample)
        return E_POINTER;

    CComPtr<IMFMediaBuffer> spBuffer;
    HRESULT hr = pSample->GetBufferByIndex(0, &spBuffer);
    if (FAILED(hr))
        return hr;

    BYTE* pSrcData = nullptr;
    DWORD cbMaxLength = 0;
    DWORD cbCurrentLength = 0;
    hr = spBuffer->Lock(&pSrcData, &cbMaxLength, &cbCurrentLength);
    if (FAILED(hr))
        return hr;

    // Copy from sample buffer to texture
    if (m_spTexture && pSrcData && cbCurrentLength > 0)
    {
        D3DLOCKED_RECT lockedRect = {};
        if (SUCCEEDED(m_spTexture->LockRect(0, &lockedRect, nullptr, 0)))
        {
            UINT copyH = std::min(m_desc.uHeight, cbCurrentLength / lockedRect.Pitch);
            for (UINT y = 0; y < copyH; ++y)
                memcpy((BYTE*)lockedRect.pBits + y * lockedRect.Pitch,
                       pSrcData + y * (cbCurrentLength / m_desc.uHeight),
                       std::min(lockedRect.Pitch, cbCurrentLength / m_desc.uHeight));
            m_spTexture->UnlockRect(0);
        }
    }

    spBuffer->Unlock();
    return hr;
}

HRESULT TextureInterOp::CopyToSample(IMFSample* pSample)
{
    if (!pSample)
        return E_POINTER;

    CComPtr<IMFMediaBuffer> spBuffer;
    HRESULT hr = pSample->GetBufferByIndex(0, &spBuffer);
    if (FAILED(hr))
        return hr;

    BYTE* pDestData = nullptr;
    DWORD cbMaxLength = 0;
    hr = spBuffer->Lock(&pDestData, &cbMaxLength, nullptr);
    if (FAILED(hr))
        return hr;

    // Copy from texture to sample buffer
    if (m_spTexture && pDestData && cbMaxLength > 0)
    {
        D3DLOCKED_RECT lockedRect = {};
        if (SUCCEEDED(m_spTexture->LockRect(0, &lockedRect, nullptr, D3DLOCK_READONLY)))
        {
            UINT copyH = std::min(m_desc.uHeight, cbMaxLength / lockedRect.Pitch);
            for (UINT y = 0; y < copyH; ++y)
                memcpy(pDestData + y * (cbMaxLength / m_desc.uHeight),
                       (BYTE*)lockedRect.pBits + y * lockedRect.Pitch,
                       std::min(lockedRect.Pitch, cbMaxLength / m_desc.uHeight));
            m_spTexture->UnlockRect(0);
        }
    }

    spBuffer->Unlock();
    return hr;
}

bool TextureInterOp::IsInitialized() const throw()
{
    return m_fInitialized;
}

UINT TextureInterOp::GetWidth() const throw()
{
    return m_desc.uWidth;
}

UINT TextureInterOp::GetHeight() const throw()
{
    return m_desc.uHeight;
}

D3DFORMAT TextureInterOp::GetFormat() const throw()
{
    return m_desc.d3dFormat;
}

HRESULT TextureInterOp::GetSharedHandle(HANDLE* phShared)
{
    if (!phShared)
        return E_POINTER;
    *phShared = m_hSharedHandle;
    return m_hSharedHandle ? S_OK : E_NOTIMPL;
}

HRESULT TextureInterOp::CreateTextureInternal()
{
    return E_NOTIMPL;
}

HRESULT TextureInterOp::CreateSurfaceInternal()
{
    return E_NOTIMPL;
}

// ============================================================================
// TextureInterOpDX9
// ============================================================================

TextureInterOpDX9::TextureInterOpDX9()
    : m_pDevice(nullptr)
    , m_hSharedTexture(nullptr)
{
}

TextureInterOpDX9::~TextureInterOpDX9()
{
    Shutdown();
}

HRESULT TextureInterOpDX9::Initialize(const TextureInteropDesc& desc, IDirect3DDevice9* pDevice)
{
    if (!pDevice)
        return E_POINTER;

    m_pDevice = pDevice;

    HRESULT hr = TextureInterOp::Initialize(desc);
    if (FAILED(hr))
        return hr;

    hr = CreateTextureFromDevice(pDevice);
    return hr;
}

HRESULT TextureInterOpDX9::Shutdown()
{
    m_pDevice = nullptr;
    m_hSharedTexture = nullptr;
    return TextureInterOp::Shutdown();
}

HRESULT TextureInterOpDX9::CreateTextureFromDevice(IDirect3DDevice9* pDevice)
{
    if (!pDevice)
        return E_POINTER;

    HRESULT hr = pDevice->CreateTexture(
        m_desc.uWidth,
        m_desc.uHeight,
        1,
        m_desc.uUsage ? m_desc.uUsage : D3DUSAGE_RENDERTARGET,
        m_desc.d3dFormat,
        D3DPOOL_DEFAULT,
        &m_spTexture,
        nullptr);

    if (SUCCEEDED(hr) && m_spTexture)
    {
        hr = m_spTexture->GetSurfaceLevel(0, &m_spSurface);
    }

    return hr;
}

HRESULT TextureInterOpDX9::CopyFromSurface(IDirect3DSurface9* pSource)
{
    if (!pSource || !m_spSurface || !m_pDevice)
        return E_POINTER;

    HRESULT hr = m_pDevice->StretchRect(pSource, nullptr, m_spSurface, nullptr, D3DTEXF_NONE);
    return hr;
}

HRESULT TextureInterOpDX9::CopyToSurface(IDirect3DSurface9* pDest)
{
    if (!pDest || !m_spSurface || !m_pDevice)
        return E_POINTER;

    HRESULT hr = m_pDevice->StretchRect(m_spSurface, nullptr, pDest, nullptr, D3DTEXF_NONE);
    return hr;
}

IDirect3DDevice9* TextureInterOpDX9::GetDevice() const
{
    return m_pDevice;
}

HRESULT TextureInterOpDX9::SetDevice(IDirect3DDevice9* pDevice)
{
    m_pDevice = pDevice;
    return S_OK;
}

HRESULT TextureInterOpDX9::CreateSharedTexture(IDirect3DDevice9* pDevice, HANDLE* phShared)
{
    if (!pDevice || !phShared)
        return E_POINTER;

    *phShared = nullptr;

    HRESULT hr = pDevice->CreateTexture(
        m_desc.uWidth, m_desc.uHeight, 1,
        D3DUSAGE_RENDERTARGET,
        m_desc.d3dFormat,
        D3DPOOL_DEFAULT,
        &m_spTexture,
        phShared);

    if (SUCCEEDED(hr) && m_spTexture)
    {
        m_spTexture->GetSurfaceLevel(0, &m_spSurface);
        m_hSharedHandle = *phShared;
    }

    return hr;
}

HRESULT TextureInterOpDX9::OpenSharedTexture(IDirect3DDevice9* pDevice, HANDLE hShared)
{
    if (!pDevice || !hShared)
        return E_POINTER;

    HRESULT hr = pDevice->CreateTexture(
        m_desc.uWidth, m_desc.uHeight, 1,
        D3DUSAGE_RENDERTARGET,
        m_desc.d3dFormat,
        D3DPOOL_DEFAULT,
        &m_spTexture,
        &hShared);

    if (SUCCEEDED(hr) && m_spTexture)
        m_spTexture->GetSurfaceLevel(0, &m_spSurface);

    return hr;
}

HRESULT TextureInterOpDX9::CreateTextureInternal()
{
    if (!m_pDevice)
        return E_UNEXPECTED;

    return CreateTextureFromDevice(m_pDevice);
}

HRESULT TextureInterOpDX9::CreateSurfaceInternal()
{
    if (!m_pDevice)
        return E_UNEXPECTED;

    return m_pDevice->CreateRenderTarget(
        m_desc.uWidth,
        m_desc.uHeight,
        m_desc.d3dFormat,
        D3DMULTISAMPLE_NONE,
        0,
        FALSE,
        &m_spSurface,
        nullptr);
}

// ============================================================================
// TextureInterOpDX11
// ============================================================================

TextureInterOpDX11::TextureInterOpDX11()
    : m_pDevice(nullptr)
    , m_pContext(nullptr)
    , m_hSharedTexture(nullptr)
{
}

TextureInterOpDX11::~TextureInterOpDX11()
{
    Shutdown();
}

HRESULT TextureInterOpDX11::Initialize(const TextureInteropDesc& desc, ID3D11Device* pDevice)
{
    if (!pDevice)
        return E_POINTER;

    m_pDevice = pDevice;
    m_pDevice->GetImmediateContext(&m_pContext);

    HRESULT hr = TextureInterOp::Initialize(desc);
    if (FAILED(hr))
        return hr;

    hr = CreateTextureFromDevice(pDevice);
    return hr;
}

HRESULT TextureInterOpDX11::Shutdown()
{
    if (m_spDX11Texture)
    {
        m_spDX11Texture = nullptr;
    }

    if (m_pContext)
    {
        m_pContext->Release();
        m_pContext = nullptr;
    }

    m_pDevice = nullptr;
    m_hSharedTexture = nullptr;

    return TextureInterOp::Shutdown();
}

HRESULT TextureInterOpDX11::CreateTextureFromDevice(ID3D11Device* pDevice)
{
    if (!pDevice)
        return E_POINTER;

    return CreateDX11Texture(pDevice);
}

HRESULT TextureInterOpDX11::CopyFromTexture(ID3D11Texture2D* pSource)
{
    if (!pSource || !m_spDX11Texture)
        return E_POINTER;

    if (!m_pContext)
        return E_UNEXPECTED;

    m_pContext->CopyResource(m_spDX11Texture, pSource);
    return S_OK;
}

HRESULT TextureInterOpDX11::CopyToTexture(ID3D11Texture2D* pDest)
{
    if (!pDest || !m_spDX11Texture)
        return E_POINTER;

    if (!m_pContext)
        return E_UNEXPECTED;

    m_pContext->CopyResource(pDest, m_spDX11Texture);
    return S_OK;
}

HRESULT TextureInterOpDX11::CreateSharedTexture(ID3D11Device* pDevice, HANDLE* phShared)
{
    if (!pDevice || !phShared)
        return E_POINTER;

    *phShared = nullptr;

    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = m_desc.uWidth;
    texDesc.Height = m_desc.uHeight;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.MiscFlags = D3D11_RESOURCE_MISC_SHARED;

    CComPtr<ID3D11Texture2D> spTexture;
    HRESULT hr = pDevice->CreateTexture2D(&texDesc, nullptr, &spTexture);
    if (FAILED(hr))
        return hr;

    CComPtr<IDXGIResource> spDXGIResource;
    hr = spTexture->QueryInterface(IID_PPV_ARGS(&spDXGIResource));
    if (FAILED(hr))
        return hr;

    hr = spDXGIResource->GetSharedHandle(phShared);
    if (SUCCEEDED(hr))
    {
        m_hSharedTexture = *phShared;
        m_hSharedHandle = *phShared;
        m_spDX11Texture = spTexture;
    }

    return hr;
}

HRESULT TextureInterOpDX11::OpenSharedTexture(ID3D11Device* pDevice, HANDLE hShared)
{
    if (!pDevice || !hShared)
        return E_POINTER;

    return pDevice->OpenSharedResource(hShared, IID_PPV_ARGS(&m_spDX11Texture));
}

ID3D11Device* TextureInterOpDX11::GetDevice() const
{
    return m_pDevice;
}

ID3D11DeviceContext* TextureInterOpDX11::GetContext() const
{
    return m_pContext;
}

HRESULT TextureInterOpDX11::SetDevice(ID3D11Device* pDevice)
{
    m_pDevice = pDevice;
    if (m_pContext)
    {
        m_pContext->Release();
        m_pContext = nullptr;
    }
    if (pDevice)
        pDevice->GetImmediateContext(&m_pContext);
    return S_OK;
}

HRESULT TextureInterOpDX11::Map(D3D11_MAP mapType, void** ppData, UINT* pRowPitch)
{
    if (!ppData || !m_spDX11Texture)
        return E_POINTER;

    if (!m_pContext)
        return E_UNEXPECTED;

    D3D11_MAPPED_SUBRESOURCE mapped = {};
    HRESULT hr = m_pContext->Map(m_spDX11Texture, 0, mapType, 0, &mapped);
    if (SUCCEEDED(hr))
    {
        *ppData = mapped.pData;
        if (pRowPitch) *pRowPitch = mapped.RowPitch;
    }

    return hr;
}

HRESULT TextureInterOpDX11::Unmap()
{
    if (!m_spDX11Texture)
        return E_UNEXPECTED;

    if (!m_pContext)
        return E_UNEXPECTED;

    m_pContext->Unmap(m_spDX11Texture, 0);
    return S_OK;
}

D3D11_TEXTURE2D_DESC TextureInterOpDX11::GetTextureDesc() const
{
    D3D11_TEXTURE2D_DESC desc = {};
    if (m_spDX11Texture)
        m_spDX11Texture->GetDesc(&desc);
    return desc;
}

HRESULT TextureInterOpDX11::CreateTextureInternal()
{
    if (!m_pDevice)
        return E_UNEXPECTED;

    return CreateDX11Texture(m_pDevice);
}

HRESULT TextureInterOpDX11::CreateSurfaceInternal()
{
    return CreateTextureInternal();
}

HRESULT TextureInterOpDX11::CreateDX11Texture(ID3D11Device* pDevice)
{
    if (!pDevice)
        return E_POINTER;

    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = m_desc.uWidth;
    texDesc.Height = m_desc.uHeight;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.CPUAccessFlags = 0;

    if (m_desc.fShared)
        texDesc.MiscFlags = D3D11_RESOURCE_MISC_SHARED;

    return pDevice->CreateTexture2D(&texDesc, nullptr, &m_spDX11Texture);
}

} // namespace HMRAVSource
