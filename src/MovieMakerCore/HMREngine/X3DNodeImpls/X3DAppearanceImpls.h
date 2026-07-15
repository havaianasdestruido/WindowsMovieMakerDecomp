#pragma once
// X3DAppearanceImpls.h - Appearance, texture, material, shader bridges

#include "X3DNodeImplBase.h"
#include "../X3DNodes.h"

namespace HMREngine
{
    // Forward declarations
    class TextureTransformContainerImpl;
    class X3DShaderNodeImpl;

    // --- X3DTextureNodeImpl: Texture node bridge ---
    class X3DTextureNodeImpl : public X3DChildNodeImpl
    {
    public:
        X3DTextureNodeImpl();
        virtual ~X3DTextureNodeImpl();

        HRESULT Initialize(X3DTextureNode* node);
        void Shutdown() override;

        X3DTextureNode* GetTextureNode() const { return m_textureNode; }
        virtual HRESULT GetTexture(ID3D11Device* dev, ID3D11ShaderResourceView** ppSRV);
        virtual void Update(double time);

    protected:
        X3DTextureNode* m_textureNode = nullptr;

        HRESULT SetupFields() override;
    };

    // --- X3DTexture2DNodeImpl: 2D texture bridge ---
    class X3DTexture2DNodeImpl : public X3DTextureNodeImpl
    {
    public:
        X3DTexture2DNodeImpl();
        virtual ~X3DTexture2DNodeImpl();

        HRESULT Initialize(X3DTextureNode* node);
        void Shutdown() override;

        HRESULT GetTexture(ID3D11Device* dev, ID3D11ShaderResourceView** ppSRV) override;

        UINT GetWidth() const { return m_width; }
        UINT GetHeight() const { return m_height; }

    protected:
        UINT m_width = 0;
        UINT m_height = 0;

        HRESULT SetupFields() override;
    };

    // --- X3DUrlObjectImpl: URL object bridge ---
    class X3DUrlObjectImpl : public X3DNodeImpl
    {
    public:
        X3DUrlObjectImpl();
        virtual ~X3DUrlObjectImpl();

        HRESULT Initialize(X3DNode* node);
        void Shutdown() override;

        const std::string& GetUrl() const { return m_url; }
        void SetUrl(const std::string& url) { m_url = url; MarkDirty(); }

        HRESULT ResolveUrl(std::wstring& outPath) const;

    protected:
        std::string m_url;

        HRESULT SetupFields() override;
    };

    // --- X3DMaterialNodeImpl: Material node bridge ---
    class X3DMaterialNodeImpl : public X3DChildNodeImpl
    {
    public:
        X3DMaterialNodeImpl();
        virtual ~X3DMaterialNodeImpl();

        HRESULT Initialize(X3DMaterialNode* node);
        void Shutdown() override;

        X3DMaterialNode* GetMaterialNode() const { return m_materialNode; }

        Rgb GetDiffuseColor() const { return m_diffuseColor; }
        Rgb GetSpecularColor() const { return m_specularColor; }
        Rgb GetEmissiveColor() const { return m_emissiveColor; }
        Rgb GetAmbientColor() const { return m_ambientColor; }
        float GetShininess() const { return m_shininess; }
        float GetTransparency() const { return m_transparency; }

        void SetDiffuseColor(const Rgb& c) { m_diffuseColor = c; MarkDirty(); }
        void SetSpecularColor(const Rgb& c) { m_specularColor = c; MarkDirty(); }
        void SetEmissiveColor(const Rgb& c) { m_emissiveColor = c; MarkDirty(); }
        void SetAmbientColor(const Rgb& c) { m_ambientColor = c; MarkDirty(); }
        void SetShininess(float s) { m_shininess = s; MarkDirty(); }
        void SetTransparency(float t) { m_transparency = t; MarkDirty(); }

    protected:
        X3DMaterialNode* m_materialNode = nullptr;
        Rgb m_diffuseColor = Rgb(0.8f, 0.8f, 0.8f);
        Rgb m_specularColor = Rgb(0.0f, 0.0f, 0.0f);
        Rgb m_emissiveColor = Rgb(0.0f, 0.0f, 0.0f);
        Rgb m_ambientColor = Rgb(0.2f, 0.2f, 0.2f);
        float m_shininess = 0.2f;
        float m_transparency = 0.0f;

        HRESULT SetupFields() override;
    };

    // --- X3DAppearanceNodeImpl: Appearance node bridge ---
    class X3DAppearanceNodeImpl : public X3DChildNodeImpl
    {
    public:
        X3DAppearanceNodeImpl();
        virtual ~X3DAppearanceNodeImpl();

        HRESULT Initialize(X3DChildNode* node);
        void Shutdown() override;

        void SetTexture(X3DTextureNodeImpl* tex);
        void SetMaterial(X3DMaterialNodeImpl* mat);
        void SetTextureTransform(TextureTransformContainerImpl* tt);

        X3DTextureNodeImpl* GetTexture() const { return m_texture; }
        X3DMaterialNodeImpl* GetMaterial() const { return m_material; }
        TextureTransformContainerImpl* GetTextureTransform() const { return m_textureTransform; }

    protected:
        X3DTextureNodeImpl* m_texture = nullptr;
        X3DMaterialNodeImpl* m_material = nullptr;
        TextureTransformContainerImpl* m_textureTransform = nullptr;

        HRESULT SetupFields() override;
    };

    // --- TextureTransformContainerImpl: Texture transform ---
    class TextureTransformContainerImpl : public X3DNodeImpl
    {
    public:
        TextureTransformContainerImpl();
        virtual ~TextureTransformContainerImpl();

        HRESULT Initialize(X3DChildNode* node);
        void Shutdown() override;

        Vec2 GetTranslation() const { return m_translation; }
        float GetRotation() const { return m_rotation; }
        Vec2 GetScale() const { return m_scale; }

        void SetTranslation(const Vec2& t) { m_translation = t; MarkDirty(); }
        void SetRotation(float r) { m_rotation = r; MarkDirty(); }
        void SetScale(const Vec2& s) { m_scale = s; MarkDirty(); }

        Matrix4f GetMatrix() const;

    protected:
        Vec2 m_translation;
        float m_rotation = 0.0f;
        Vec2 m_scale = Vec2(1, 1);
    };

    // --- MotionTextureNodeImpl: Motion texture bridge ---
    class MotionTextureNodeImpl : public X3DTextureNodeImpl
    {
    public:
        MotionTextureNodeImpl();
        virtual ~MotionTextureNodeImpl();

        HRESULT Initialize(X3DTextureNode* node);
        void Shutdown() override;

        void SetOffset(float offset) { m_offset = offset; }
        float GetOffset() const { return m_offset; }
        void SetScale(float scale) { m_scale = scale; }
        float GetScale() const { return m_scale; }

    protected:
        float m_offset = 0.0f;
        float m_scale = 1.0f;

        HRESULT SetupFields() override;
    };

    // --- ShaderSetNodeImpl: Shader set bridge ---
    class ShaderSetNodeImpl : public X3DNodeImpl
    {
    public:
        ShaderSetNodeImpl();
        virtual ~ShaderSetNodeImpl();

        HRESULT Initialize(X3DNode* node);
        void Shutdown() override;

        UINT GetShaderCount() const { return static_cast<UINT>(m_shaders.size()); }
        void AddShader(X3DShaderNodeImpl* shader);
        X3DShaderNodeImpl* GetShader(UINT index) const;

    protected:
        std::vector<X3DShaderNodeImpl*> m_shaders;
    };

    // --- TexturePropertiesNodeImpl: Texture properties bridge ---
    class TexturePropertiesNodeImpl : public X3DNodeImpl
    {
    public:
        TexturePropertiesNodeImpl();
        virtual ~TexturePropertiesNodeImpl();

        HRESULT Initialize(X3DChildNode* node);
        void Shutdown() override;

        bool GenerateMipMaps() const { return m_generateMipMaps; }
        std::string GetMagFilter() const { return m_magFilter; }
        std::string GetMinFilter() const { return m_minFilter; }

        D3D11_FILTER GetD3DFilter() const;
        D3D11_TEXTURE_ADDRESS_MODE GetAddressMode() const;

    protected:
        bool m_generateMipMaps = true;
        std::string m_magFilter = "LINEAR";
        std::string m_minFilter = "LINEAR";
    };

    // --- X3DShaderNodeImpl: Shader bridge ---
    class X3DShaderNodeImpl : public X3DNodeImpl
    {
    public:
        X3DShaderNodeImpl();
        virtual ~X3DShaderNodeImpl();

        HRESULT Initialize(X3DNode* node);
        void Shutdown() override;

        const std::string& GetLanguage() const { return m_language; }
        void SetLanguage(const std::string& lang) { m_language = lang; }

    protected:
        std::string m_language = "GLSL";

        HRESULT SetupFields() override;
    };

    // --- X3DAnimatedShaderNodeImpl: Animated shader bridge ---
    class X3DAnimatedShaderNodeImpl : public X3DShaderNodeImpl
    {
    public:
        X3DAnimatedShaderNodeImpl();
        virtual ~X3DAnimatedShaderNodeImpl();

        HRESULT Initialize(X3DNode* node);
        void Shutdown() override;

        double GetFraction() const { return m_fraction; }
        void SetFraction(double f) { m_fraction = f; }

    protected:
        double m_fraction = 0.0;

        HRESULT SetupFields() override;
    };

} // namespace HMREngine
