#include "pch.h"
// TextureCodecs.cpp - D3DX11 texture codec RTTI class implementations
//
// Original Windows Live Movie Maker 2012 ships ~118 D3DX11 texture codec
// classes.  They exist solely to generate unique RTTI entries (one vtable
// per codec).  The Encode / Decode stubs return E_NOTIMPL because the
// actual pixel-format conversion was provided by D3DX11 runtime helpers;
// this project reconstructs the class hierarchy for binary compatibility.

#include "TextureCodecs.h"

namespace HMREngine
{
namespace DX
{

// ============================================================================
// CTextureCodecBase - shared stub implementation
// ============================================================================
HRESULT STDMETHODCALLTYPE CTextureCodecBase::Encode(
    const BYTE* pData, UINT cbData,
    UINT width, UINT height, DXGI_FORMAT format,
    ID3D11Texture2D** ppTexture)
{
    if (!pData || !ppTexture || width == 0 || height == 0) return E_INVALIDARG;
    if (format == DXGI_FORMAT_UNKNOWN) return E_INVALIDARG;

    D3D11_TEXTURE2D_DESC td = {};
    td.Width = width;
    td.Height = height;
    td.MipLevels = 1;
    td.ArraySize = 1;
    td.Format = format;
    td.SampleDesc.Count = 1;
    td.Usage = D3D11_USAGE_DEFAULT;
    td.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    UINT rowPitch = (width * m_bitsPerPixel) / 8;
    if (m_compressed)
    {
        UINT blockSize = (format == DXGI_FORMAT_BC1_UNORM || format == DXGI_FORMAT_BC1_UNORM_SRGB ||
                          format == DXGI_FORMAT_BC4_UNORM || format == DXGI_FORMAT_BC4_SNORM) ? 8 : 16;
        rowPitch = ((width + 3) / 4) * blockSize;
    }

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = pData;
    initData.SysMemPitch = rowPitch;

    return D3D11CreateTexture2D(ppTexture, &td, &initData, nullptr);
}

HRESULT STDMETHODCALLTYPE CTextureCodecBase::Decode(
    ID3D11Texture2D* pTexture,
    BYTE** ppData, UINT* pcbData,
    UINT* pWidth, UINT* pHeight, DXGI_FORMAT* pFormat)
{
    if (!pTexture || !ppData || !pcbData) return E_INVALIDARG;

    D3D11_TEXTURE2D_DESC td;
    pTexture->GetDesc(&td);

    if (pWidth) *pWidth = td.Width;
    if (pHeight) *pHeight = td.Height;
    if (pFormat) *pFormat = td.Format;

    UINT rowPitch = (td.Width * m_bitsPerPixel) / 8;
    if (m_compressed)
    {
        UINT blockSize = (td.Format == DXGI_FORMAT_BC1_UNORM || td.Format == DXGI_FORMAT_BC1_UNORM_SRGB ||
                          td.Format == DXGI_FORMAT_BC4_UNORM || td.Format == DXGI_FORMAT_BC4_SNORM) ? 8 : 16;
        rowPitch = ((td.Width + 3) / 4) * blockSize;
    }

    UINT totalSize = rowPitch * td.Height;
    *ppData = static_cast<BYTE*>(CoTaskMemAlloc(totalSize));
    if (!*ppData) return E_OUTOFMEMORY;
    *pcbData = totalSize;

    // Create a staging texture to read back
    ID3D11Device* dev = nullptr;
    ID3D11DeviceContext* ctx = nullptr;
    pTexture->GetDevice(&dev);
    if (dev) dev->GetImmediateContext(&ctx);

    if (!dev || !ctx)
    {
        CoTaskMemFree(*ppData);
        *ppData = nullptr;
        return E_FAIL;
    }

    D3D11_TEXTURE2D_DESC stagingDesc = td;
    stagingDesc.Usage = D3D11_USAGE_STAGING;
    stagingDesc.BindFlags = 0;
    stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    stagingDesc.MiscFlags = 0;

    CComPtr<ID3D11Texture2D> staging;
    HRESULT hr = dev->CreateTexture2D(&stagingDesc, nullptr, &staging);
    if (FAILED(hr))
    {
        CoTaskMemFree(*ppData);
        *ppData = nullptr;
        ctx->Release();
        dev->Release();
        return hr;
    }

    ctx->CopyResource(staging, pTexture);

    D3D11_MAPPED_SUBRESOURCE mapped;
    hr = ctx->Map(staging, 0, D3D11_MAP_READ, 0, &mapped);
    if (SUCCEEDED(hr))
    {
        for (UINT y = 0; y < td.Height; ++y)
            memcpy((*ppData) + y * rowPitch, (BYTE*)mapped.pData + y * mapped.RowPitch, rowPitch);
        ctx->Unmap(staging, 0);
    }
    else
    {
        CoTaskMemFree(*ppData);
        *ppData = nullptr;
    }

    ctx->Release();
    dev->Release();
    return hr;
}

UINT STDMETHODCALLTYPE CTextureCodecBase::GetBitsPerPixel() const
{
    return m_bitsPerPixel;
}

DXGI_FORMAT STDMETHODCALLTYPE CTextureCodecBase::GetDXGIFormat() const
{
    return m_format;
}

BOOL STDMETHODCALLTYPE CTextureCodecBase::IsCompressed() const
{
    return m_compressed;
}

BOOL STDMETHODCALLTYPE CTextureCodecBase::IsSupported(ID3D11Device* pDevice) const
{
    if (!pDevice || m_format == DXGI_FORMAT_UNKNOWN) return FALSE;

    D3D11_FEATURE_DATA_FORMAT_SUPPORT fmtSupport = {};
    fmtSupport.InFormat = m_format;
    HRESULT hr = pDevice->CheckFeatureSupport(
        D3D11_FEATURE_FORMAT_SUPPORT, &fmtSupport, sizeof(fmtSupport));
    if (FAILED(hr)) return FALSE;

    return (fmtSupport.OutFormatSupport & D3D11_FORMAT_SUPPORT_TEXTURE2D) != 0;
}

// ============================================================================
// Factory: look up codec by RTTI family name + format name
// ============================================================================
// All D3DX11Texture codec names (must match the DECLARE_ macros above)
struct CodecEntry
{
    const char*     name;
    ITextureCodec*  (*create)();
};

template<typename T>
static ITextureCodec* CreateInstance() { return new T(); }

#define ENTRY(FAMILY, NAME) \
    { #NAME, CreateInstance<FAMILY::CCodec_##NAME> }

static const CodecEntry s_d3dx11Codecs[] =
{
    ENTRY(D3DX11Texture, B8G8R8A8),
    ENTRY(D3DX11Texture, R8G8B8A8),
    ENTRY(D3DX11Texture, R8G8B8),
    ENTRY(D3DX11Texture, B5G6R5),
    ENTRY(D3DX11Texture, B5G5R5A1),
    ENTRY(D3DX11Texture, B4G4R4A4),
    ENTRY(D3DX11Texture, A8),
    ENTRY(D3DX11Texture, R8),
    ENTRY(D3DX11Texture, R16),
    ENTRY(D3DX11Texture, R16G16),
    ENTRY(D3DX11Texture, R16G16B16A16),
    ENTRY(D3DX11Texture, R16G16B16A16F),
    ENTRY(D3DX11Texture, R32),
    ENTRY(D3DX11Texture, R32G32),
    ENTRY(D3DX11Texture, R32G32B32),
    ENTRY(D3DX11Texture, R32G32B32A32),
    ENTRY(D3DX11Texture, R8G8X8),
    ENTRY(D3DX11Texture, R8G8),
    ENTRY(D3DX11Texture, R16F),
    ENTRY(D3DX11Texture, R16G16F),
    ENTRY(D3DX11Texture, R32F),
    ENTRY(D3DX11Texture, R32G32F),
    ENTRY(D3DX11Texture, R32G32B32F),
    ENTRY(D3DX11Texture, R10G10B10A2),
    ENTRY(D3DX11Texture, R11G11B10F),
    ENTRY(D3DX11Texture, R9G9B9E5),
    ENTRY(D3DX11Texture, A16B16G16R16F),
    ENTRY(D3DX11Texture, R8G8B8A8_SRGB),
    ENTRY(D3DX11Texture, B8G8R8A8_SRGB),
    ENTRY(D3DX11Texture, BC1),
    ENTRY(D3DX11Texture, BC1_SRGB),
    ENTRY(D3DX11Texture, BC2),
    ENTRY(D3DX11Texture, BC2_SRGB),
    ENTRY(D3DX11Texture, BC3),
    ENTRY(D3DX11Texture, BC3_SRGB),
    ENTRY(D3DX11Texture, BC4),
    ENTRY(D3DX11Texture, BC4_SNORM),
    ENTRY(D3DX11Texture, BC5),
    ENTRY(D3DX11Texture, BC5_SNORM),
    ENTRY(D3DX11Texture, BC6H),
    ENTRY(D3DX11Texture, BC6H_SF16),
    ENTRY(D3DX11Texture, BC7),
    ENTRY(D3DX11Texture, BC7_SRGB),
    ENTRY(D3DX11Texture, DXT1),
    ENTRY(D3DX11Texture, DXT2),
    ENTRY(D3DX11Texture, DXT3),
    ENTRY(D3DX11Texture, DXT4),
    ENTRY(D3DX11Texture, DXT5),
    ENTRY(D3DX11Texture, ATI1N),
    ENTRY(D3DX11Texture, ATI2N),
    ENTRY(D3DX11Texture, ATC),
    ENTRY(D3DX11Texture, ATC_EXPLICIT),
    ENTRY(D3DX11Texture, ATC_INTERPOLATED),
    ENTRY(D3DX11Texture, PVRTC),
    ENTRY(D3DX11Texture, PVRTC_2BPP),
    ENTRY(D3DX11Texture, ETC1),
    ENTRY(D3DX11Texture, ETC2),
    ENTRY(D3DX11Texture, ASTC_4x4),
    ENTRY(D3DX11Texture, ASTC_5x5),
    ENTRY(D3DX11Texture, ASTC_6x6),
    ENTRY(D3DX11Texture, AYUV),
    ENTRY(D3DX11Texture, YUY2),
    ENTRY(D3DX11Texture, NV12),
    ENTRY(D3DX11Texture, P010),
    ENTRY(D3DX11Texture, P016),
    ENTRY(D3DX11Texture, Y410),
    ENTRY(D3DX11Texture, Y416),
    ENTRY(D3DX11Texture, YV12),
    ENTRY(D3DX11Texture, I420),
    ENTRY(D3DX11Texture, NV11),
    ENTRY(D3DX11Texture, 420_OPAQUE),
};

static const CodecEntry s_d3dxOldCodecs[] =
{
    ENTRY(D3DXOldTexture, B8G8R8A8),
    ENTRY(D3DXOldTexture, R8G8B8A8),
    ENTRY(D3DXOldTexture, R8G8B8),
    ENTRY(D3DXOldTexture, B5G6R5),
    ENTRY(D3DXOldTexture, B5G5R5A1),
    ENTRY(D3DXOldTexture, B4G4R4A4),
    ENTRY(D3DXOldTexture, A8),
    ENTRY(D3DXOldTexture, R8),
    ENTRY(D3DXOldTexture, R16),
    ENTRY(D3DXOldTexture, R16G16),
    ENTRY(D3DXOldTexture, R16G16B16A16),
    ENTRY(D3DXOldTexture, R16G16B16A16F),
    ENTRY(D3DXOldTexture, R32),
    ENTRY(D3DXOldTexture, R32G32),
    ENTRY(D3DXOldTexture, R32G32B32),
    ENTRY(D3DXOldTexture, R32G32B32A32),
    ENTRY(D3DXOldTexture, B8G8R8X8),
    ENTRY(D3DXOldTexture, R8G8),
    ENTRY(D3DXOldTexture, R16F),
    ENTRY(D3DXOldTexture, R16G16F),
    ENTRY(D3DXOldTexture, R32F),
    ENTRY(D3DXOldTexture, R32G32F),
    ENTRY(D3DXOldTexture, R32G32B32F),
    ENTRY(D3DXOldTexture, R10G10B10A2),
    ENTRY(D3DXOldTexture, R11G11B10F),
    ENTRY(D3DXOldTexture, R9G9B9E5),
    ENTRY(D3DXOldTexture, A16B16G16R16F),
    ENTRY(D3DXOldTexture, R8G8B8A8_SRGB),
    ENTRY(D3DXOldTexture, B8G8R8A8_SRGB),
    ENTRY(D3DXOldTexture, BC1),
    ENTRY(D3DXOldTexture, BC1_SRGB),
    ENTRY(D3DXOldTexture, BC2),
    ENTRY(D3DXOldTexture, BC2_SRGB),
    ENTRY(D3DXOldTexture, BC3),
    ENTRY(D3DXOldTexture, BC3_SRGB),
    ENTRY(D3DXOldTexture, BC4),
    ENTRY(D3DXOldTexture, BC4_SNORM),
    ENTRY(D3DXOldTexture, BC5),
    ENTRY(D3DXOldTexture, BC5_SNORM),
    ENTRY(D3DXOldTexture, BC6H),
    ENTRY(D3DXOldTexture, BC6H_SF16),
    ENTRY(D3DXOldTexture, BC7),
    ENTRY(D3DXOldTexture, BC7_SRGB),
    ENTRY(D3DXOldTexture, DXT1),
    ENTRY(D3DXOldTexture, DXT2),
    ENTRY(D3DXOldTexture, DXT3),
    ENTRY(D3DXOldTexture, DXT4),
    ENTRY(D3DXOldTexture, DXT5),
    ENTRY(D3DXOldTexture, ATI1N),
    ENTRY(D3DXOldTexture, ATI2N),
    ENTRY(D3DXOldTexture, ATC),
    ENTRY(D3DXOldTexture, ATC_EXPLICIT),
    ENTRY(D3DXOldTexture, ATC_INTERPOLATED),
    ENTRY(D3DXOldTexture, PVRTC),
    ENTRY(D3DXOldTexture, PVRTC_2BPP),
    ENTRY(D3DXOldTexture, ETC1),
    ENTRY(D3DXOldTexture, ETC2),
    ENTRY(D3DXOldTexture, ASTC_4x4),
    ENTRY(D3DXOldTexture, ASTC_5x5),
    ENTRY(D3DXOldTexture, ASTC_6x6),
    ENTRY(D3DXOldTexture, AYUV),
    ENTRY(D3DXOldTexture, YUY2),
    ENTRY(D3DXOldTexture, NV12),
    ENTRY(D3DXOldTexture, P010),
    ENTRY(D3DXOldTexture, P016),
    ENTRY(D3DXOldTexture, Y410),
    ENTRY(D3DXOldTexture, Y416),
    ENTRY(D3DXOldTexture, YV12),
    ENTRY(D3DXOldTexture, I420),
    ENTRY(D3DXOldTexture, NV11),
    ENTRY(D3DXOldTexture, 420_OPAQUE),
};

static bool StrEqA(const char* a, const char* b)
{
    if (!a || !b) return false;
    while (*a && *b)
    {
        if (*a != *b) return false;
        ++a; ++b;
    }
    return *a == *b;
}

ITextureCodec* CreateTextureCodec(const wchar_t* pFamily, const char* pFormatName)
{
    if (!pFamily || !pFormatName) return nullptr;

    const CodecEntry* table = nullptr;
    size_t count = 0;

    if (StrEqA(reinterpret_cast<const char*>(pFamily), "D3DX11Texture"))
    {
        table = s_d3dx11Codecs;
        count = sizeof(s_d3dx11Codecs) / sizeof(s_d3dx11Codecs[0]);
    }
    else if (StrEqA(reinterpret_cast<const char*>(pFamily), "D3DXOldTexture"))
    {
        table = s_d3dxOldCodecs;
        count = sizeof(s_d3dxOldCodecs) / sizeof(s_d3dxOldCodecs[0]);
    }

    if (!table) return nullptr;

    for (size_t i = 0; i < count; ++i)
    {
        if (StrEqA(table[i].name, pFormatName))
            return table[i].create();
    }

    return nullptr;
}

} // namespace DX
} // namespace HMREngine
