#pragma once
// Shaders.h - Shader types for HMREngine

#include "HMREngine.h"
#include "X3DMath.h"
#include <d3d11.h>
#include <d3dx11.h>
#include <string>

namespace HMREngine
{
    struct ShaderConstants
    {
        Matrix4f world;
        Matrix4f view;
        Matrix4f projection;
        Matrix4f worldViewProj;
        float time;
        float deltaTime;
        Vec2 resolution;
        Vec4 userParams[8];
    };

    struct GridConstants
    {
        Vec2 gridMin;
        Vec2 gridMax;
        Vec2 divisions;
        Rgb lineColor;
        float lineWidth;
        float showOrigin;
        Rgb originColor;
        float padding;
    };

    struct FadeConstants
    {
        float alpha;
        float fadeDirection; // 0=none, 1=in, -1=out
        float padding[2];
    };

    struct DissolveConstants
    {
        float threshold;
        float edgeWidth;
        Rgb edgeColor;
    };

    struct ChannelMixerConstants
    {
        Vec3 redVector;
        float pad0;
        Vec3 greenVector;
        float pad1;
        Vec3 blueVector;
        float pad2;
    };

    struct BrightnessConstants
    {
        float brightness;
        float contrast;
        float gamma;
        float saturation;
    };

    struct BlurConstants
    {
        Vec2 direction;
        Vec2 texelSize;
        float blurRadius;
        float padding[3];
    };

    struct PosterizeConstants
    {
        float numLevels;
        float padding[3];
    };

    struct PixelateConstants
    {
        Vec2 pixelSize;
        float padding[2];
    };

    struct RippleConstants
    {
        float amplitude;
        float frequency;
        float speed;
        float time;
    };

    struct HueConstants
    {
        float hueShift;
        float padding[3];
    };

    struct EdgeDetectionConstants
    {
        Vec2 texelSize;
        float threshold;
        float padding;
        Rgb edgeColor;
        float pad2;
    };

    struct TextConstants
    {
        Vec2 position;
        Vec2 textSize;
        Rgba textColor;
        float rotation;
        float padding[3];
    };

    struct ScrollingTextConstants
    {
        Vec2 position;
        Vec2 textSize;
        Rgba textColor;
        float scrollOffset;
        float scrollSpeed;
        float padding;
    };

    struct WipeConstants
    {
        float progress;
        float direction; // 0=horizontal, 1=vertical, 2=radial
        float softness;
        float padding;
    };

    // --- Base Shader ---
    class ShaderBase
    {
    protected:
        CComPtr<ID3DX11Effect> m_effect;
        CComPtr<ID3DX11EffectTechnique> m_technique;
        CComPtr<ID3DX11EffectPass> m_pass;
        std::string m_effectPath;

        HRESULT LoadEffect(ID3D11Device* pDev, const char* path);
        void SetTechnique(const char* name);

    public:
        virtual ~ShaderBase() = default;
        virtual HRESULT Initialize(ID3D11Device* pDev, const char* effectPath);
        virtual void Begin(ID3D11DeviceContext* pCtx);
        virtual void End(ID3D11DeviceContext* pCtx);
        ID3DX11Effect* GetEffect() { return m_effect; }
    };

    // --- Grid Shader ---
    class GridShader : public ShaderBase
    {
    public:
        HRESULT Initialize(ID3D11Device* pDev) { return ShaderBase::Initialize(pDev, "Grid.fx"); }
        void SetConstants(ID3D11DeviceContext* pCtx, const GridConstants& c);
        void Draw(ID3D11DeviceContext* pCtx, ID3D11Buffer* vb, UINT vertexCount);
    };

    // --- Fade Shader ---
    class FadeShader : public ShaderBase
    {
    public:
        HRESULT Initialize(ID3D11Device* pDev) { return ShaderBase::Initialize(pDev, "Fade.fx"); }
        void SetTexture(ID3D11DeviceContext* pCtx, ID3D11ShaderResourceView* srv);
        void SetConstants(ID3D11DeviceContext* pCtx, const FadeConstants& c);
        void Draw(ID3D11DeviceContext* pCtx, ID3D11Buffer* vb, UINT vertexCount);
    };

    // --- Dissolve Shader ---
    class DissolveShader : public ShaderBase
    {
    public:
        HRESULT Initialize(ID3D11Device* pDev) { return ShaderBase::Initialize(pDev, "Dissolve.fx"); }
        void SetTexture(ID3D11DeviceContext* pCtx, ID3D11ShaderResourceView* srv);
        void SetNoiseTexture(ID3D11DeviceContext* pCtx, ID3D11ShaderResourceView* srv);
        void SetConstants(ID3D11DeviceContext* pCtx, const DissolveConstants& c);
        void Draw(ID3D11DeviceContext* pCtx, ID3D11Buffer* vb, UINT vertexCount);
    };

    // --- Channel Mixer Shader ---
    class ChannelMixerShader : public ShaderBase
    {
    public:
        HRESULT Initialize(ID3D11Device* pDev) { return ShaderBase::Initialize(pDev, "ChannelMixer.fx"); }
        void SetTexture(ID3D11DeviceContext* pCtx, ID3D11ShaderResourceView* srv);
        void SetConstants(ID3D11DeviceContext* pCtx, const ChannelMixerConstants& c);
        void Draw(ID3D11DeviceContext* pCtx, ID3D11Buffer* vb, UINT vertexCount);
    };

    // --- Brightness/Contrast Shader ---
    class BrightnessShader : public ShaderBase
    {
    public:
        HRESULT Initialize(ID3D11Device* pDev) { return ShaderBase::Initialize(pDev, "Brightness.fx"); }
        void SetTexture(ID3D11DeviceContext* pCtx, ID3D11ShaderResourceView* srv);
        void SetConstants(ID3D11DeviceContext* pCtx, const BrightnessConstants& c);
        void Draw(ID3D11DeviceContext* pCtx, ID3D11Buffer* vb, UINT vertexCount);
    };

    // --- Blur Shader ---
    class BlurShader : public ShaderBase
    {
    public:
        HRESULT Initialize(ID3D11Device* pDev) { return ShaderBase::Initialize(pDev, "Blur.fx"); }
        void SetTexture(ID3D11DeviceContext* pCtx, ID3D11ShaderResourceView* srv);
        void SetConstants(ID3D11DeviceContext* pCtx, const BlurConstants& c);
        void DrawHorizontal(ID3D11DeviceContext* pCtx, ID3D11Buffer* vb, UINT vertexCount);
        void DrawVertical(ID3D11DeviceContext* pCtx, ID3D11Buffer* vb, UINT vertexCount);
    };

    // --- Posterize Shader ---
    class PosterizeShader : public ShaderBase
    {
    public:
        HRESULT Initialize(ID3D11Device* pDev) { return ShaderBase::Initialize(pDev, "Posterize.fx"); }
        void SetTexture(ID3D11DeviceContext* pCtx, ID3D11ShaderResourceView* srv);
        void SetConstants(ID3D11DeviceContext* pCtx, const PosterizeConstants& c);
        void Draw(ID3D11DeviceContext* pCtx, ID3D11Buffer* vb, UINT vertexCount);
    };

    // --- Pixelate Shader ---
    class PixelateShader : public ShaderBase
    {
    public:
        HRESULT Initialize(ID3D11Device* pDev) { return ShaderBase::Initialize(pDev, "Pixelate.fx"); }
        void SetTexture(ID3D11DeviceContext* pCtx, ID3D11ShaderResourceView* srv);
        void SetConstants(ID3D11DeviceContext* pCtx, const PixelateConstants& c);
        void Draw(ID3D11DeviceContext* pCtx, ID3D11Buffer* vb, UINT vertexCount);
    };

    // --- Ripple Shader ---
    class RippleShader : public ShaderBase
    {
    public:
        HRESULT Initialize(ID3D11Device* pDev) { return ShaderBase::Initialize(pDev, "Ripple.fx"); }
        void SetTexture(ID3D11DeviceContext* pCtx, ID3D11ShaderResourceView* srv);
        void SetConstants(ID3D11DeviceContext* pCtx, const RippleConstants& c);
        void Draw(ID3D11DeviceContext* pCtx, ID3D11Buffer* vb, UINT vertexCount);
    };

    // --- Hue Shift Shader ---
    class HueShader : public ShaderBase
    {
    public:
        HRESULT Initialize(ID3D11Device* pDev) { return ShaderBase::Initialize(pDev, "HueShift.fx"); }
        void SetTexture(ID3D11DeviceContext* pCtx, ID3D11ShaderResourceView* srv);
        void SetConstants(ID3D11DeviceContext* pCtx, const HueConstants& c);
        void Draw(ID3D11DeviceContext* pCtx, ID3D11Buffer* vb, UINT vertexCount);
    };

    // --- Edge Detection Shader ---
    class EdgeDetectionShader : public ShaderBase
    {
    public:
        HRESULT Initialize(ID3D11Device* pDev) { return ShaderBase::Initialize(pDev, "EdgeDetection.fx"); }
        void SetTexture(ID3D11DeviceContext* pCtx, ID3D11ShaderResourceView* srv);
        void SetConstants(ID3D11DeviceContext* pCtx, const EdgeDetectionConstants& c);
        void Draw(ID3D11DeviceContext* pCtx, ID3D11Buffer* vb, UINT vertexCount);
    };

    // --- Text Shader ---
    class TextShader : public ShaderBase
    {
    public:
        HRESULT Initialize(ID3D11Device* pDev) { return ShaderBase::Initialize(pDev, "Text.fx"); }
        void SetFontTexture(ID3D11DeviceContext* pCtx, ID3D11ShaderResourceView* srv);
        void SetConstants(ID3D11DeviceContext* pCtx, const TextConstants& c);
        void Draw(ID3D11DeviceContext* pCtx, ID3D11Buffer* vb, UINT vertexCount);
    };

    // --- Scrolling Text Shader ---
    class ScrollingTextShader : public ShaderBase
    {
    public:
        HRESULT Initialize(ID3D11Device* pDev) { return ShaderBase::Initialize(pDev, "ScrollingText.fx"); }
        void SetFontTexture(ID3D11DeviceContext* pCtx, ID3D11ShaderResourceView* srv);
        void SetConstants(ID3D11DeviceContext* pCtx, const ScrollingTextConstants& c);
        void Draw(ID3D11DeviceContext* pCtx, ID3D11Buffer* vb, UINT vertexCount);
    };

    // --- Wipe Shader ---
    class WipeShader : public ShaderBase
    {
    public:
        HRESULT Initialize(ID3D11Device* pDev) { return ShaderBase::Initialize(pDev, "Wipe.fx"); }
        void SetTextureFrom(ID3D11DeviceContext* pCtx, ID3D11ShaderResourceView* srv);
        void SetTextureTo(ID3D11DeviceContext* pCtx, ID3D11ShaderResourceView* srv);
        void SetConstants(ID3D11DeviceContext* pCtx, const WipeConstants& c);
        void Draw(ID3D11DeviceContext* pCtx, ID3D11Buffer* vb, UINT vertexCount);
    };

} // namespace HMREngine
