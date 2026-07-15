#pragma once
// TextureCodecs.h - D3DX11 texture codec RTTI classes
// Original Windows Live Movie Maker 2012 has ~118 D3DX11 texture codec
// classes split between D3DX11Texture and D3DXOldTexture families.

#include "DXResources.h"
#include "DeviceClientDX.h"

namespace HMREngine
{
namespace DX
{

// ============================================================================
// ITextureCodec - Base interface for all texture codecs
// ============================================================================
class ITextureCodec
{
public:
    virtual ~ITextureCodec() = default;

    virtual HRESULT STDMETHODCALLTYPE Encode(
        const BYTE* pData, UINT cbData,
        UINT width, UINT height, DXGI_FORMAT format,
        ID3D11Texture2D** ppTexture) = 0;

    virtual HRESULT STDMETHODCALLTYPE Decode(
        ID3D11Texture2D* pTexture,
        BYTE** ppData, UINT* pcbData,
        UINT* pWidth, UINT* pHeight, DXGI_FORMAT* pFormat) = 0;

    virtual UINT STDMETHODCALLTYPE GetBitsPerPixel() const = 0;
    virtual DXGI_FORMAT STDMETHODCALLTYPE GetDXGIFormat() const = 0;
    virtual BOOL STDMETHODCALLTYPE IsCompressed() const = 0;
    virtual BOOL STDMETHODCALLTYPE IsSupported(ID3D11Device* pDevice) const = 0;
};

// ============================================================================
// CTextureCodecBase - Common implementation for all codec stubs
// ============================================================================
class CTextureCodecBase : public ITextureCodec
{
public:
    CTextureCodecBase() = default;
    ~CTextureCodecBase() override = default;

    HRESULT STDMETHODCALLTYPE Encode(
        const BYTE* pData, UINT cbData,
        UINT width, UINT height, DXGI_FORMAT format,
        ID3D11Texture2D** ppTexture) override;

    HRESULT STDMETHODCALLTYPE Decode(
        ID3D11Texture2D* pTexture,
        BYTE** ppData, UINT* pcbData,
        UINT* pWidth, UINT* pHeight, DXGI_FORMAT* pFormat) override;

    UINT STDMETHODCALLTYPE GetBitsPerPixel() const override;
    DXGI_FORMAT STDMETHODCALLTYPE GetDXGIFormat() const override;
    BOOL STDMETHODCALLTYPE IsCompressed() const override;
    BOOL STDMETHODCALLTYPE IsSupported(ID3D11Device* pDevice) const override;

protected:
    DXGI_FORMAT m_format = DXGI_FORMAT_UNKNOWN;
    UINT m_bitsPerPixel = 0;
    BOOL m_compressed = FALSE;
};

// ============================================================================
// D3DX11Texture codec family - New D3DX11 texture formats
// Each class gets its own RTTI via a unique virtual destructor.
//
// Generated via DECLARE_D3DX11_CODEC / DECLARE_D3DX11_CODEC_EX macros.
// ============================================================================

// Macro: declares a codec class in namespace D3DX11Texture with the given
// format name, DXGI format, bits-per-pixel, and compressed flag.
#define DECLARE_D3DX11_CODEC(NAME, DXGI_FMT, BPP, COMPRESSED) \
namespace D3DX11Texture { \
class CCodec_##NAME : public ::HMREngine::DX::CTextureCodecBase { \
public: \
    CCodec_##NAME() { m_format = (DXGI_FMT); m_bitsPerPixel = (BPP); m_compressed = (COMPRESSED); } \
    ~CCodec_##NAME() override {} \
    UINT STDMETHODCALLTYPE GetBitsPerPixel() const override { return m_bitsPerPixel; } \
    DXGI_FORMAT STDMETHODCALLTYPE GetDXGIFormat() const override { return m_format; } \
    BOOL STDMETHODCALLTYPE IsCompressed() const override { return m_compressed; } \
}; \
}

// Macro: declares a codec class with a custom DXGI format expression
#define DECLARE_D3DX11_CODEC_EX(NAME, DXGI_EXPR, BPP, COMPRESSED) \
namespace D3DX11Texture { \
class CCodec_##NAME : public ::HMREngine::DX::CTextureCodecBase { \
public: \
    CCodec_##NAME() { m_format = (DXGI_EXPR); m_bitsPerPixel = (BPP); m_compressed = (COMPRESSED); } \
    ~CCodec_##NAME() override {} \
    UINT STDMETHODCALLTYPE GetBitsPerPixel() const override { return m_bitsPerPixel; } \
    DXGI_FORMAT STDMETHODCALLTYPE GetDXGIFormat() const override { return m_format; } \
    BOOL STDMETHODCALLTYPE IsCompressed() const override { return m_compressed; } \
}; \
}

// --- Standard uncompressed formats (D3DX11Texture) ---
DECLARE_D3DX11_CODEC(B8G8R8A8,         DXGI_FORMAT_B8G8R8A8_UNORM,              32, FALSE)
DECLARE_D3DX11_CODEC(R8G8B8A8,         DXGI_FORMAT_R8G8B8A8_UNORM,              32, FALSE)
DECLARE_D3DX11_CODEC(R8G8B8,           DXGI_FORMAT_B8G8R8X8_UNORM,              24, FALSE)
DECLARE_D3DX11_CODEC(B5G6R5,           DXGI_FORMAT_B5G6R5_UNORM,                16, FALSE)
DECLARE_D3DX11_CODEC(B5G5R5A1,         DXGI_FORMAT_B5G5R5A1_UNORM,              16, FALSE)
DECLARE_D3DX11_CODEC(B4G4R4A4,         DXGI_FORMAT_B4G4R4A4_UNORM,              16, FALSE)
DECLARE_D3DX11_CODEC(A8,               DXGI_FORMAT_A8_UNORM,                     8, FALSE)
DECLARE_D3DX11_CODEC(R8,               DXGI_FORMAT_R8_UNORM,                     8, FALSE)
DECLARE_D3DX11_CODEC(R16,              DXGI_FORMAT_R16_UNORM,                   16, FALSE)
DECLARE_D3DX11_CODEC(R16G16,           DXGI_FORMAT_R16G16_UNORM,                32, FALSE)
DECLARE_D3DX11_CODEC(R16G16B16A16,     DXGI_FORMAT_R16G16B16A16_UNORM,          64, FALSE)
DECLARE_D3DX11_CODEC(R16G16B16A16F,    DXGI_FORMAT_R16G16B16A16_FLOAT,          64, FALSE)
DECLARE_D3DX11_CODEC(R32,              DXGI_FORMAT_R32_FLOAT,                   32, FALSE)
DECLARE_D3DX11_CODEC(R32G32,           DXGI_FORMAT_R32G32_FLOAT,                64, FALSE)
DECLARE_D3DX11_CODEC(R32G32B32,        DXGI_FORMAT_R32G32B32_FLOAT,             96, FALSE)
DECLARE_D3DX11_CODEC(R32G32B32A32,     DXGI_FORMAT_R32G32B32A32_FLOAT,         128, FALSE)
DECLARE_D3DX11_CODEC(R8G8X8,           DXGI_FORMAT_B8G8R8X8_UNORM,              32, FALSE)
DECLARE_D3DX11_CODEC(R8G8,             DXGI_FORMAT_R8G8_UNORM,                  16, FALSE)
DECLARE_D3DX11_CODEC(R16F,             DXGI_FORMAT_R16_FLOAT,                   16, FALSE)
DECLARE_D3DX11_CODEC(R16G16F,          DXGI_FORMAT_R16G16_FLOAT,                32, FALSE)
DECLARE_D3DX11_CODEC(R32F,             DXGI_FORMAT_R32_FLOAT,                   32, FALSE)
DECLARE_D3DX11_CODEC(R32G32F,          DXGI_FORMAT_R32G32_FLOAT,                64, FALSE)
DECLARE_D3DX11_CODEC(R32G32B32F,       DXGI_FORMAT_R32G32B32_FLOAT,             96, FALSE)
DECLARE_D3DX11_CODEC(R10G10B10A2,      DXGI_FORMAT_R10G10B10A2_UNORM,           32, FALSE)
DECLARE_D3DX11_CODEC(R11G11B10F,       DXGI_FORMAT_R11G11B10_FLOAT,             32, FALSE)
DECLARE_D3DX11_CODEC(R9G9B9E5,         DXGI_FORMAT_R9G9B9E5_SHAREDEXP,          32, FALSE)
DECLARE_D3DX11_CODEC(A16B16G16R16F,    DXGI_FORMAT_R16G16B16A16_FLOAT,          64, FALSE)
DECLARE_D3DX11_CODEC(R8G8B8A8_SRGB,    DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,         32, FALSE)
DECLARE_D3DX11_CODEC(B8G8R8A8_SRGB,    DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,         32, FALSE)

// --- BC compressed formats (D3DX11Texture) ---
DECLARE_D3DX11_CODEC(BC1,              DXGI_FORMAT_BC1_UNORM,                    4, TRUE)
DECLARE_D3DX11_CODEC(BC1_SRGB,         DXGI_FORMAT_BC1_UNORM_SRGB,               4, TRUE)
DECLARE_D3DX11_CODEC(BC2,              DXGI_FORMAT_BC2_UNORM,                    8, TRUE)
DECLARE_D3DX11_CODEC(BC2_SRGB,         DXGI_FORMAT_BC2_UNORM_SRGB,               8, TRUE)
DECLARE_D3DX11_CODEC(BC3,              DXGI_FORMAT_BC3_UNORM,                    8, TRUE)
DECLARE_D3DX11_CODEC(BC3_SRGB,         DXGI_FORMAT_BC3_UNORM_SRGB,               8, TRUE)
DECLARE_D3DX11_CODEC(BC4,              DXGI_FORMAT_BC4_UNORM,                    4, TRUE)
DECLARE_D3DX11_CODEC(BC4_SNORM,        DXGI_FORMAT_BC4_SNORM,                    4, TRUE)
DECLARE_D3DX11_CODEC(BC5,              DXGI_FORMAT_BC5_UNORM,                    8, TRUE)
DECLARE_D3DX11_CODEC(BC5_SNORM,        DXGI_FORMAT_BC5_SNORM,                    8, TRUE)
DECLARE_D3DX11_CODEC(BC6H,             DXGI_FORMAT_BC6H_UF16,                    8, TRUE)
DECLARE_D3DX11_CODEC(BC6H_SF16,        DXGI_FORMAT_BC6H_SF16,                    8, TRUE)
DECLARE_D3DX11_CODEC(BC7,              DXGI_FORMAT_BC7_UNORM,                    8, TRUE)
DECLARE_D3DX11_CODEC(BC7_SRGB,         DXGI_FORMAT_BC7_UNORM_SRGB,               8, TRUE)

// --- Legacy DXT names (D3DX11Texture) ---
DECLARE_D3DX11_CODEC(DXT1,             DXGI_FORMAT_BC1_UNORM,                    4, TRUE)
DECLARE_D3DX11_CODEC(DXT2,             DXGI_FORMAT_BC2_UNORM,                    8, TRUE)
DECLARE_D3DX11_CODEC(DXT3,             DXGI_FORMAT_BC2_UNORM,                    8, TRUE)
DECLARE_D3DX11_CODEC(DXT4,             DXGI_FORMAT_BC3_UNORM,                    8, TRUE)
DECLARE_D3DX11_CODEC(DXT5,             DXGI_FORMAT_BC3_UNORM,                    8, TRUE)

// --- Mobile / legacy formats (D3DX11Texture) ---
DECLARE_D3DX11_CODEC(ATI1N,            DXGI_FORMAT_BC4_UNORM,                    4, TRUE)
DECLARE_D3DX11_CODEC(ATI2N,            DXGI_FORMAT_BC5_UNORM,                    8, TRUE)
DECLARE_D3DX11_CODEC(ATC,              DXGI_FORMAT_BC1_UNORM,                    4, TRUE)
DECLARE_D3DX11_CODEC(ATC_EXPLICIT,     DXGI_FORMAT_BC1_UNORM,                    4, TRUE)
DECLARE_D3DX11_CODEC(ATC_INTERPOLATED, DXGI_FORMAT_BC3_UNORM,                    8, TRUE)
DECLARE_D3DX11_CODEC(PVRTC,            DXGI_FORMAT_BC1_UNORM,                    4, TRUE)
DECLARE_D3DX11_CODEC(PVRTC_2BPP,       DXGI_FORMAT_BC1_UNORM,                    2, TRUE)
DECLARE_D3DX11_CODEC(ETC1,             DXGI_FORMAT_BC1_UNORM,                    4, TRUE)
DECLARE_D3DX11_CODEC(ETC2,             DXGI_FORMAT_BC7_UNORM,                    8, TRUE)
DECLARE_D3DX11_CODEC(ASTC_4x4,         DXGI_FORMAT_BC7_UNORM,                    8, TRUE)
DECLARE_D3DX11_CODEC(ASTC_5x5,         DXGI_FORMAT_BC7_UNORM,                    8, TRUE)
DECLARE_D3DX11_CODEC(ASTC_6x6,         DXGI_FORMAT_BC7_UNORM,                    8, TRUE)

// --- YUV / video formats (D3DX11Texture) ---
DECLARE_D3DX11_CODEC_EX(AYUV,          DXGI_FORMAT_UNKNOWN,                     32, FALSE)
DECLARE_D3DX11_CODEC_EX(YUY2,          DXGI_FORMAT_UNKNOWN,                     16, FALSE)
DECLARE_D3DX11_CODEC_EX(NV12,          DXGI_FORMAT_UNKNOWN,                     12, FALSE)
DECLARE_D3DX11_CODEC_EX(P010,          DXGI_FORMAT_UNKNOWN,                     15, FALSE)
DECLARE_D3DX11_CODEC_EX(P016,          DXGI_FORMAT_UNKNOWN,                     24, FALSE)
DECLARE_D3DX11_CODEC_EX(Y410,          DXGI_FORMAT_UNKNOWN,                     32, FALSE)
DECLARE_D3DX11_CODEC_EX(Y416,          DXGI_FORMAT_UNKNOWN,                     64, FALSE)
DECLARE_D3DX11_CODEC_EX(YV12,          DXGI_FORMAT_UNKNOWN,                     12, FALSE)
DECLARE_D3DX11_CODEC_EX(I420,          DXGI_FORMAT_UNKNOWN,                     12, FALSE)
DECLARE_D3DX11_CODEC_EX(NV11,          DXGI_FORMAT_UNKNOWN,                     12, FALSE)
DECLARE_D3DX11_CODEC_EX(420_OPAQUE,    DXGI_FORMAT_UNKNOWN,                     12, FALSE)

// ============================================================================
// D3DXOldTexture codec family - Legacy/D3DX9 texture formats
// ============================================================================

#define DECLARE_D3DXOLD_CODEC(NAME, DXGI_FMT, BPP, COMPRESSED) \
namespace D3DXOldTexture { \
class CCodec_##NAME : public ::HMREngine::DX::CTextureCodecBase { \
public: \
    CCodec_##NAME() { m_format = (DXGI_FMT); m_bitsPerPixel = (BPP); m_compressed = (COMPRESSED); } \
    ~CCodec_##NAME() override {} \
    UINT STDMETHODCALLTYPE GetBitsPerPixel() const override { return m_bitsPerPixel; } \
    DXGI_FORMAT STDMETHODCALLTYPE GetDXGIFormat() const override { return m_format; } \
    BOOL STDMETHODCALLTYPE IsCompressed() const override { return m_compressed; } \
}; \
}

#define DECLARE_D3DXOLD_CODEC_EX(NAME, DXGI_EXPR, BPP, COMPRESSED) \
namespace D3DXOldTexture { \
class CCodec_##NAME : public ::HMREngine::DX::CTextureCodecBase { \
public: \
    CCodec_##NAME() { m_format = (DXGI_EXPR); m_bitsPerPixel = (BPP); m_compressed = (COMPRESSED); } \
    ~CCodec_##NAME() override {} \
    UINT STDMETHODCALLTYPE GetBitsPerPixel() const override { return m_bitsPerPixel; } \
    DXGI_FORMAT STDMETHODCALLTYPE GetDXGIFormat() const override { return m_format; } \
    BOOL STDMETHODCALLTYPE IsCompressed() const override { return m_compressed; } \
}; \
}

// --- Standard uncompressed formats (D3DXOldTexture) ---
DECLARE_D3DXOLD_CODEC(B8G8R8A8,         DXGI_FORMAT_B8G8R8A8_UNORM,              32, FALSE)
DECLARE_D3DXOLD_CODEC(R8G8B8A8,         DXGI_FORMAT_R8G8B8A8_UNORM,              32, FALSE)
DECLARE_D3DXOLD_CODEC(R8G8B8,           DXGI_FORMAT_B8G8R8X8_UNORM,              24, FALSE)
DECLARE_D3DXOLD_CODEC(B5G6R5,           DXGI_FORMAT_B5G6R5_UNORM,                16, FALSE)
DECLARE_D3DXOLD_CODEC(B5G5R5A1,         DXGI_FORMAT_B5G5R5A1_UNORM,              16, FALSE)
DECLARE_D3DXOLD_CODEC(B4G4R4A4,         DXGI_FORMAT_B4G4R4A4_UNORM,              16, FALSE)
DECLARE_D3DXOLD_CODEC(A8,               DXGI_FORMAT_A8_UNORM,                     8, FALSE)
DECLARE_D3DXOLD_CODEC(R8,               DXGI_FORMAT_R8_UNORM,                     8, FALSE)
DECLARE_D3DXOLD_CODEC(R16,              DXGI_FORMAT_R16_UNORM,                   16, FALSE)
DECLARE_D3DXOLD_CODEC(R16G16,           DXGI_FORMAT_R16G16_UNORM,                32, FALSE)
DECLARE_D3DXOLD_CODEC(R16G16B16A16,     DXGI_FORMAT_R16G16B16A16_UNORM,          64, FALSE)
DECLARE_D3DXOLD_CODEC(R16G16B16A16F,    DXGI_FORMAT_R16G16B16A16_FLOAT,          64, FALSE)
DECLARE_D3DXOLD_CODEC(R32,              DXGI_FORMAT_R32_FLOAT,                   32, FALSE)
DECLARE_D3DXOLD_CODEC(R32G32,           DXGI_FORMAT_R32G32_FLOAT,                64, FALSE)
DECLARE_D3DXOLD_CODEC(R32G32B32,        DXGI_FORMAT_R32G32B32_FLOAT,             96, FALSE)
DECLARE_D3DXOLD_CODEC(R32G32B32A32,     DXGI_FORMAT_R32G32B32A32_FLOAT,         128, FALSE)
DECLARE_D3DXOLD_CODEC(B8G8R8X8,         DXGI_FORMAT_B8G8R8X8_UNORM,              32, FALSE)
DECLARE_D3DXOLD_CODEC(R8G8,             DXGI_FORMAT_R8G8_UNORM,                  16, FALSE)
DECLARE_D3DXOLD_CODEC(R16F,             DXGI_FORMAT_R16_FLOAT,                   16, FALSE)
DECLARE_D3DXOLD_CODEC(R16G16F,          DXGI_FORMAT_R16G16_FLOAT,                32, FALSE)
DECLARE_D3DXOLD_CODEC(R32F,             DXGI_FORMAT_R32_FLOAT,                   32, FALSE)
DECLARE_D3DXOLD_CODEC(R32G32F,          DXGI_FORMAT_R32G32_FLOAT,                64, FALSE)
DECLARE_D3DXOLD_CODEC(R32G32B32F,       DXGI_FORMAT_R32G32B32_FLOAT,             96, FALSE)
DECLARE_D3DXOLD_CODEC(R10G10B10A2,      DXGI_FORMAT_R10G10B10A2_UNORM,           32, FALSE)
DECLARE_D3DXOLD_CODEC(R11G11B10F,       DXGI_FORMAT_R11G11B10_FLOAT,             32, FALSE)
DECLARE_D3DXOLD_CODEC(R9G9B9E5,         DXGI_FORMAT_R9G9B9E5_SHAREDEXP,          32, FALSE)
DECLARE_D3DXOLD_CODEC(A16B16G16R16F,    DXGI_FORMAT_R16G16B16A16_FLOAT,          64, FALSE)
DECLARE_D3DXOLD_CODEC(R8G8B8A8_SRGB,    DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,         32, FALSE)
DECLARE_D3DXOLD_CODEC(B8G8R8A8_SRGB,    DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,         32, FALSE)

// --- BC compressed formats (D3DXOldTexture) ---
DECLARE_D3DXOLD_CODEC(BC1,              DXGI_FORMAT_BC1_UNORM,                    4, TRUE)
DECLARE_D3DXOLD_CODEC(BC1_SRGB,         DXGI_FORMAT_BC1_UNORM_SRGB,               4, TRUE)
DECLARE_D3DXOLD_CODEC(BC2,              DXGI_FORMAT_BC2_UNORM,                    8, TRUE)
DECLARE_D3DXOLD_CODEC(BC2_SRGB,         DXGI_FORMAT_BC2_UNORM_SRGB,               8, TRUE)
DECLARE_D3DXOLD_CODEC(BC3,              DXGI_FORMAT_BC3_UNORM,                    8, TRUE)
DECLARE_D3DXOLD_CODEC(BC3_SRGB,         DXGI_FORMAT_BC3_UNORM_SRGB,               8, TRUE)
DECLARE_D3DXOLD_CODEC(BC4,              DXGI_FORMAT_BC4_UNORM,                    4, TRUE)
DECLARE_D3DXOLD_CODEC(BC4_SNORM,        DXGI_FORMAT_BC4_SNORM,                    4, TRUE)
DECLARE_D3DXOLD_CODEC(BC5,              DXGI_FORMAT_BC5_UNORM,                    8, TRUE)
DECLARE_D3DXOLD_CODEC(BC5_SNORM,        DXGI_FORMAT_BC5_SNORM,                    8, TRUE)
DECLARE_D3DXOLD_CODEC(BC6H,             DXGI_FORMAT_BC6H_UF16,                    8, TRUE)
DECLARE_D3DXOLD_CODEC(BC6H_SF16,        DXGI_FORMAT_BC6H_SF16,                    8, TRUE)
DECLARE_D3DXOLD_CODEC(BC7,              DXGI_FORMAT_BC7_UNORM,                    8, TRUE)
DECLARE_D3DXOLD_CODEC(BC7_SRGB,         DXGI_FORMAT_BC7_UNORM_SRGB,               8, TRUE)

// --- Legacy DXT names (D3DXOldTexture) ---
DECLARE_D3DXOLD_CODEC(DXT1,             DXGI_FORMAT_BC1_UNORM,                    4, TRUE)
DECLARE_D3DXOLD_CODEC(DXT2,             DXGI_FORMAT_BC2_UNORM,                    8, TRUE)
DECLARE_D3DXOLD_CODEC(DXT3,             DXGI_FORMAT_BC2_UNORM,                    8, TRUE)
DECLARE_D3DXOLD_CODEC(DXT4,             DXGI_FORMAT_BC3_UNORM,                    8, TRUE)
DECLARE_D3DXOLD_CODEC(DXT5,             DXGI_FORMAT_BC3_UNORM,                    8, TRUE)

// --- Mobile / legacy formats (D3DXOldTexture) ---
DECLARE_D3DXOLD_CODEC(ATI1N,            DXGI_FORMAT_BC4_UNORM,                    4, TRUE)
DECLARE_D3DXOLD_CODEC(ATI2N,            DXGI_FORMAT_BC5_UNORM,                    8, TRUE)
DECLARE_D3DXOLD_CODEC(ATC,              DXGI_FORMAT_BC1_UNORM,                    4, TRUE)
DECLARE_D3DXOLD_CODEC(ATC_EXPLICIT,     DXGI_FORMAT_BC1_UNORM,                    4, TRUE)
DECLARE_D3DXOLD_CODEC(ATC_INTERPOLATED, DXGI_FORMAT_BC3_UNORM,                    8, TRUE)
DECLARE_D3DXOLD_CODEC(PVRTC,            DXGI_FORMAT_BC1_UNORM,                    4, TRUE)
DECLARE_D3DXOLD_CODEC(PVRTC_2BPP,       DXGI_FORMAT_BC1_UNORM,                    2, TRUE)
DECLARE_D3DXOLD_CODEC(ETC1,             DXGI_FORMAT_BC1_UNORM,                    4, TRUE)
DECLARE_D3DXOLD_CODEC(ETC2,             DXGI_FORMAT_BC7_UNORM,                    8, TRUE)
DECLARE_D3DXOLD_CODEC(ASTC_4x4,         DXGI_FORMAT_BC7_UNORM,                    8, TRUE)
DECLARE_D3DXOLD_CODEC(ASTC_5x5,         DXGI_FORMAT_BC7_UNORM,                    8, TRUE)
DECLARE_D3DXOLD_CODEC(ASTC_6x6,         DXGI_FORMAT_BC7_UNORM,                    8, TRUE)

// --- YUV / video formats (D3DXOldTexture) ---
DECLARE_D3DXOLD_CODEC_EX(AYUV,          DXGI_FORMAT_UNKNOWN,                     32, FALSE)
DECLARE_D3DXOLD_CODEC_EX(YUY2,          DXGI_FORMAT_UNKNOWN,                     16, FALSE)
DECLARE_D3DXOLD_CODEC_EX(NV12,          DXGI_FORMAT_UNKNOWN,                     12, FALSE)
DECLARE_D3DXOLD_CODEC_EX(P010,          DXGI_FORMAT_UNKNOWN,                     15, FALSE)
DECLARE_D3DXOLD_CODEC_EX(P016,          DXGI_FORMAT_UNKNOWN,                     24, FALSE)
DECLARE_D3DXOLD_CODEC_EX(Y410,          DXGI_FORMAT_UNKNOWN,                     32, FALSE)
DECLARE_D3DXOLD_CODEC_EX(Y416,          DXGI_FORMAT_UNKNOWN,                     64, FALSE)
DECLARE_D3DXOLD_CODEC_EX(YV12,          DXGI_FORMAT_UNKNOWN,                     12, FALSE)
DECLARE_D3DXOLD_CODEC_EX(I420,          DXGI_FORMAT_UNKNOWN,                     12, FALSE)
DECLARE_D3DXOLD_CODEC_EX(NV11,          DXGI_FORMAT_UNKNOWN,                     12, FALSE)
DECLARE_D3DXOLD_CODEC_EX(420_OPAQUE,    DXGI_FORMAT_UNKNOWN,                     12, FALSE)

// ============================================================================
// Factory function - creates a codec instance by family and format name
// ============================================================================
ITextureCodec* CreateTextureCodec(const wchar_t* pFamily, const char* pFormatName);

} // namespace DX
} // namespace HMREngine
