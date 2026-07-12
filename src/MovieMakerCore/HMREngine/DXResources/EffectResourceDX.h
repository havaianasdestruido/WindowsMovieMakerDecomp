#pragma once
// EffectResourceDX.h - HLSL effect wrapper and all effect resource types

#include "DXResources.h"

namespace HMREngine
{
    namespace DX
    {
        // --- EffectResourceDX: Base HLSL effect wrapper ---
        class EffectResourceDX : public DeviceClientDXImpl
        {
        public:
            EffectResourceDX();
            virtual ~EffectResourceDX();

            HRESULT LoadFromFile(const std::wstring& filePath);
            HRESULT LoadFromMemory(const void* data, UINT dataSize);
            HRESULT LoadFromResource(HMODULE hModule, UINT resourceId);
            void Release();

            HRESULT SetTechnique(const char* techniqueName);
            HRESULT BeginPass(UINT passIndex);
            HRESULT EndPass();

            void SetFloat(const char* name, float value);
            void SetFloat2(const char* name, const Vec2& value);
            void SetFloat3(const char* name, const Vec3& value);
            void SetFloat4(const char* name, const Vec4& value);
            void SetMatrix(const char* name, const Matrix4f& value);
            void SetTexture(const char* name, ID3D11ShaderResourceView* srv);
            void SetInt(const char* name, int value);
            void SetBool(const char* name, bool value);

            ID3DX11Effect* GetEffect() const { return m_effect; }
            ID3DX11EffectTechnique* GetTechnique() const { return m_technique; }
            bool IsLoaded() const { return m_loaded; }
            const std::wstring& GetFilePath() const { return m_filePath; }

        protected:
            CComPtr<ID3DX11Effect> m_effect;
            CComPtr<ID3DX11EffectTechnique> m_technique;
            CComPtr<ID3DX11EffectPass> m_currentPass;
            std::wstring m_filePath;
            bool m_loaded = false;
        };

        // --- CommonEffectResourceDX: Shared effect utilities ---
        class CommonEffectResourceDX : public EffectResourceDX
        {
        public:
            CommonEffectResourceDX();
            virtual ~CommonEffectResourceDX();

            HRESULT LoadCommonEffect(ID3D11Device* dev);
            void SetWorldViewProj(const Matrix4f& wvp);
            void SetWorld(const Matrix4f& world);
            void SetView(const Matrix4f& view);
            void SetProjection(const Matrix4f& proj);
            void SetTime(float time);
            void SetResolution(float width, float height);

        protected:
            CComPtr<ID3DX11EffectMatrixVariable> m_worldVar;
            CComPtr<ID3DX11EffectMatrixVariable> m_viewVar;
            CComPtr<ID3DX11EffectMatrixVariable> m_projVar;
            CComPtr<ID3DX11EffectMatrixVariable> m_wvpVar;
            CComPtr<ID3DX11EffectScalarVariable> m_timeVar;
            CComPtr<ID3DX11EffectVectorVariable> m_resolutionVar;
        };

        // --- SimpleEffectResourceBase: Base for simple single-pass effects ---
        class SimpleEffectResourceBase : public EffectResourceDX
        {
        public:
            SimpleEffectResourceBase();
            virtual ~SimpleEffectResourceBase();

            virtual HRESULT Initialize(ID3D11Device* dev, const char* effectFile);
            void Apply(ID3D11DeviceContext* ctx);

        protected:
            CComPtr<ID3DX11EffectTechnique> m_defaultTechnique;
        };

        // --- BlurEffectResource ---
        class BlurEffectResource : public SimpleEffectResourceBase
        {
        public:
            BlurEffectResource();
            virtual ~BlurEffectResource();

            HRESULT Initialize(ID3D11Device* dev) override;
            void SetDirection(const Vec2& dir);
            void SetTexelSize(const Vec2& size);
            void SetBlurRadius(float radius);
            void SetInputTexture(ID3D11ShaderResourceView* srv);

        protected:
            CComPtr<ID3DX11EffectVectorVariable> m_directionVar;
            CComPtr<ID3DX11EffectVectorVariable> m_texelSizeVar;
            CComPtr<ID3DX11EffectScalarVariable> m_blurRadiusVar;
            CComPtr<ID3DX11EffectShaderResourceVariable> m_inputTextureVar;
        };

        // --- BrightnessEffectResource ---
        class BrightnessEffectResource : public SimpleEffectResourceBase
        {
        public:
            BrightnessEffectResource();
            virtual ~BrightnessEffectResource();

            HRESULT Initialize(ID3D11Device* dev) override;
            void SetBrightness(float brightness);
            void SetContrast(float contrast);
            void SetGamma(float gamma);
            void SetSaturation(float saturation);
            void SetInputTexture(ID3D11ShaderResourceView* srv);

        protected:
            CComPtr<ID3DX11EffectScalarVariable> m_brightnessVar;
            CComPtr<ID3DX11EffectScalarVariable> m_contrastVar;
            CComPtr<ID3DX11EffectScalarVariable> m_gammaVar;
            CComPtr<ID3DX11EffectScalarVariable> m_saturationVar;
            CComPtr<ID3DX11EffectShaderResourceVariable> m_inputTextureVar;
        };

        // --- ChannelMixerEffectResource ---
        class ChannelMixerEffectResource : public SimpleEffectResourceBase
        {
        public:
            ChannelMixerEffectResource();
            virtual ~ChannelMixerEffectResource();

            HRESULT Initialize(ID3D11Device* dev) override;
            void SetRedVector(const Vec3& v);
            void SetGreenVector(const Vec3& v);
            void SetBlueVector(const Vec3& v);
            void SetInputTexture(ID3D11ShaderResourceView* srv);

        protected:
            CComPtr<ID3DX11EffectVectorVariable> m_redVectorVar;
            CComPtr<ID3DX11EffectVectorVariable> m_greenVectorVar;
            CComPtr<ID3DX11EffectVectorVariable> m_blueVectorVar;
            CComPtr<ID3DX11EffectShaderResourceVariable> m_inputTextureVar;
        };

        // --- DissolveEffectResource ---
        class DissolveEffectResource : public SimpleEffectResourceBase
        {
        public:
            DissolveEffectResource();
            virtual ~DissolveEffectResource();

            HRESULT Initialize(ID3D11Device* dev) override;
            void SetThreshold(float threshold);
            void SetEdgeWidth(float width);
            void SetEdgeColor(const Rgb& color);
            void SetInputTexture(ID3D11ShaderResourceView* srv);
            void SetNoiseTexture(ID3D11ShaderResourceView* srv);

        protected:
            CComPtr<ID3DX11EffectScalarVariable> m_thresholdVar;
            CComPtr<ID3DX11EffectScalarVariable> m_edgeWidthVar;
            CComPtr<ID3DX11EffectVectorVariable> m_edgeColorVar;
            CComPtr<ID3DX11EffectShaderResourceVariable> m_inputTextureVar;
            CComPtr<ID3DX11EffectShaderResourceVariable> m_noiseTextureVar;
        };

        // --- AnimatedEffectResourceBase: Base for time-dependent effects ---
        class AnimatedEffectResourceBase : public SimpleEffectResourceBase
        {
        public:
            AnimatedEffectResourceBase();
            virtual ~AnimatedEffectResourceBase();

            void SetTime(float time);
            void SetProgress(float progress);

        protected:
            CComPtr<ID3DX11EffectScalarVariable> m_timeVar;
            CComPtr<ID3DX11EffectScalarVariable> m_progressVar;
        };

        // --- EdgeDetectionEffectResource ---
        class EdgeDetectionEffectResource : public SimpleEffectResourceBase
        {
        public:
            EdgeDetectionEffectResource();
            virtual ~EdgeDetectionEffectResource();

            HRESULT Initialize(ID3D11Device* dev) override;
            void SetTexelSize(const Vec2& size);
            void SetThreshold(float threshold);
            void SetEdgeColor(const Rgb& color);
            void SetInputTexture(ID3D11ShaderResourceView* srv);

        protected:
            CComPtr<ID3DX11EffectVectorVariable> m_texelSizeVar;
            CComPtr<ID3DX11EffectScalarVariable> m_thresholdVar;
            CComPtr<ID3DX11EffectVectorVariable> m_edgeColorVar;
            CComPtr<ID3DX11EffectShaderResourceVariable> m_inputTextureVar;
        };

        // --- FadeEffectResource ---
        class FadeEffectResource : public AnimatedEffectResourceBase
        {
        public:
            FadeEffectResource();
            virtual ~FadeEffectResource();

            HRESULT Initialize(ID3D11Device* dev) override;
            void SetAlpha(float alpha);
            void SetFadeDirection(float dir);
            void SetInputTexture(ID3D11ShaderResourceView* srv);

        protected:
            CComPtr<ID3DX11EffectScalarVariable> m_alphaVar;
            CComPtr<ID3DX11EffectScalarVariable> m_fadeDirectionVar;
            CComPtr<ID3DX11EffectShaderResourceVariable> m_inputTextureVar;
        };

        // --- GridEffectResource ---
        class GridEffectResource : public SimpleEffectResourceBase
        {
        public:
            GridEffectResource();
            virtual ~GridEffectResource();

            HRESULT Initialize(ID3D11Device* dev) override;
            void SetGridMin(const Vec2& min);
            void SetGridMax(const Vec2& max);
            void SetDivisions(const Vec2& div);
            void SetLineColor(const Rgb& color);
            void SetLineWidth(float width);
            void SetShowOrigin(bool show);
            void SetOriginColor(const Rgb& color);

        protected:
            CComPtr<ID3DX11EffectVectorVariable> m_gridMinVar;
            CComPtr<ID3DX11EffectVectorVariable> m_gridMaxVar;
            CComPtr<ID3DX11EffectVectorVariable> m_divisionsVar;
            CComPtr<ID3DX11EffectVectorVariable> m_lineColorVar;
            CComPtr<ID3DX11EffectScalarVariable> m_lineWidthVar;
            CComPtr<ID3DX11EffectScalarVariable> m_showOriginVar;
            CComPtr<ID3DX11EffectVectorVariable> m_originColorVar;
        };

        // --- HueEffectResource ---
        class HueEffectResource : public SimpleEffectResourceBase
        {
        public:
            HueEffectResource();
            virtual ~HueEffectResource();

            HRESULT Initialize(ID3D11Device* dev) override;
            void SetHueShift(float shift);
            void SetInputTexture(ID3D11ShaderResourceView* srv);

        protected:
            CComPtr<ID3DX11EffectScalarVariable> m_hueShiftVar;
            CComPtr<ID3DX11EffectShaderResourceVariable> m_inputTextureVar;
        };

        // --- PosterizeEffectResource ---
        class PosterizeEffectResource : public SimpleEffectResourceBase
        {
        public:
            PosterizeEffectResource();
            virtual ~PosterizeEffectResource();

            HRESULT Initialize(ID3D11Device* dev) override;
            void SetNumLevels(float levels);
            void SetInputTexture(ID3D11ShaderResourceView* srv);

        protected:
            CComPtr<ID3DX11EffectScalarVariable> m_numLevelsVar;
            CComPtr<ID3DX11EffectShaderResourceVariable> m_inputTextureVar;
        };

        // --- PixelateEffectResource ---
        class PixelateEffectResource : public SimpleEffectResourceBase
        {
        public:
            PixelateEffectResource();
            virtual ~PixelateEffectResource();

            HRESULT Initialize(ID3D11Device* dev) override;
            void SetPixelSize(const Vec2& size);
            void SetInputTexture(ID3D11ShaderResourceView* srv);

        protected:
            CComPtr<ID3DX11EffectVectorVariable> m_pixelSizeVar;
            CComPtr<ID3DX11EffectShaderResourceVariable> m_inputTextureVar;
        };

        // --- RippleEffectResource ---
        class RippleEffectResource : public AnimatedEffectResourceBase
        {
        public:
            RippleEffectResource();
            virtual ~RippleEffectResource();

            HRESULT Initialize(ID3D11Device* dev) override;
            void SetAmplitude(float amp);
            void SetFrequency(float freq);
            void SetSpeed(float speed);
            void SetInputTexture(ID3D11ShaderResourceView* srv);

        protected:
            CComPtr<ID3DX11EffectScalarVariable> m_amplitudeVar;
            CComPtr<ID3DX11EffectScalarVariable> m_frequencyVar;
            CComPtr<ID3DX11EffectScalarVariable> m_speedVar;
            CComPtr<ID3DX11EffectShaderResourceVariable> m_inputTextureVar;
        };

        // --- ScrollingTextEffectResourceDX ---
        class ScrollingTextEffectResourceDX : public SimpleEffectResourceBase
        {
        public:
            ScrollingTextEffectResourceDX();
            virtual ~ScrollingTextEffectResourceDX();

            HRESULT Initialize(ID3D11Device* dev) override;
            void SetPosition(const Vec2& pos);
            void SetTextSize(const Vec2& size);
            void SetTextColor(const Rgba& color);
            void SetScrollOffset(float offset);
            void SetScrollSpeed(float speed);
            void SetFontTexture(ID3D11ShaderResourceView* srv);

        protected:
            CComPtr<ID3DX11EffectVectorVariable> m_positionVar;
            CComPtr<ID3DX11EffectVectorVariable> m_textSizeVar;
            CComPtr<ID3DX11EffectVectorVariable> m_textColorVar;
            CComPtr<ID3DX11EffectScalarVariable> m_scrollOffsetVar;
            CComPtr<ID3DX11EffectScalarVariable> m_scrollSpeedVar;
            CComPtr<ID3DX11EffectShaderResourceVariable> m_fontTextureVar;
        };

        // --- TextEffectResourceDX ---
        class TextEffectResourceDX : public SimpleEffectResourceBase
        {
        public:
            TextEffectResourceDX();
            virtual ~TextEffectResourceDX();

            HRESULT Initialize(ID3D11Device* dev) override;
            void SetPosition(const Vec2& pos);
            void SetTextSize(const Vec2& size);
            void SetTextColor(const Rgba& color);
            void SetRotation(float rotation);
            void SetFontTexture(ID3D11ShaderResourceView* srv);

        protected:
            CComPtr<ID3DX11EffectVectorVariable> m_positionVar;
            CComPtr<ID3DX11EffectVectorVariable> m_textSizeVar;
            CComPtr<ID3DX11EffectVectorVariable> m_textColorVar;
            CComPtr<ID3DX11EffectScalarVariable> m_rotationVar;
            CComPtr<ID3DX11EffectShaderResourceVariable> m_fontTextureVar;
        };

        // --- WipeEffectResource ---
        class WipeEffectResource : public AnimatedEffectResourceBase
        {
        public:
            WipeEffectResource();
            virtual ~WipeEffectResource();

            HRESULT Initialize(ID3D11Device* dev) override;
            void SetProgress(float progress);
            void SetDirection(float direction);
            void SetSoftness(float softness);
            void SetTextureFrom(ID3D11ShaderResourceView* srv);
            void SetTextureTo(ID3D11ShaderResourceView* srv);

        protected:
            CComPtr<ID3DX11EffectScalarVariable> m_progressVar;
            CComPtr<ID3DX11EffectScalarVariable> m_directionVar;
            CComPtr<ID3DX11EffectScalarVariable> m_softnessVar;
            CComPtr<ID3DX11EffectShaderResourceVariable> m_textureFromVar;
            CComPtr<ID3DX11EffectShaderResourceVariable> m_textureToVar;
        };

        // --- DefaultEffectResourceDX: Default effect fallback ---
        class DefaultEffectResourceDX : public SimpleEffectResourceBase
        {
        public:
            DefaultEffectResourceDX();
            virtual ~DefaultEffectResourceDX();

            HRESULT Initialize(ID3D11Device* dev) override;
            void SetTexture(ID3D11ShaderResourceView* srv);
            void SetTransform(const Matrix4f& transform);

        protected:
            CComPtr<ID3DX11EffectShaderResourceVariable> m_textureVar;
            CComPtr<ID3DX11EffectMatrixVariable> m_transformVar;
        };

    } // namespace DX
} // namespace HMREngine
