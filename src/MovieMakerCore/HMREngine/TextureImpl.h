#pragma once
// TextureImpl.h - ImageTexture, MovieTexture, MotionTexture, TextureTransform, TextureProperties

#include "HMREngine.h"
#include "X3DNodes.h"
#include "ResourceCache.h"
#include <d3d11.h>
#include <atlbase.h>
#include <string>

namespace HMREngine
{
    class ImageTextureImpl : public ImplObject<ImageTextureImpl>
    {
    public:
        ImageTextureImpl();
        virtual ~ImageTextureImpl();

        HRESULT Initialize(ImageTextureNode* node, ID3D11Device* dev, ResourceCache* cache);
        void Shutdown();
        void Update(double time);

        HRESULT GetTexture(ID3D11Device* dev, ID3D11ShaderResourceView** ppSRV);

        const std::string& GetURL() const { return m_url; }
        UINT GetWidth() const { return m_width; }
        UINT GetHeight() const { return m_height; }

    private:
        std::string m_url;
        TextureResource* m_resource = nullptr;
        ResourceCache* m_cache = nullptr;
        ID3D11Device* m_device = nullptr;
        UINT m_width = 0;
        UINT m_height = 0;
        bool m_loaded = false;
    };

    class MovieTextureImpl : public ImplObject<MovieTextureImpl>
    {
    public:
        MovieTextureImpl();
        virtual ~MovieTextureImpl();

        HRESULT Initialize(MovieTextureNode* node, ID3D11Device* dev, ResourceCache* cache);
        void Shutdown();
        void Update(double time);

        HRESULT GetTexture(ID3D11Device* dev, ID3D11ShaderResourceView** ppSRV);

        void Play() { m_playing = true; }
        void Pause() { m_playing = false; }
        void Stop() { m_playing = false; m_currentTime = 0.0; }
        bool IsPlaying() const { return m_playing; }
        double GetFraction() const { return m_fraction; }

    private:
        AVResource* m_resource = nullptr;
        ResourceCache* m_cache = nullptr;
        ID3D11Device* m_device = nullptr;
        bool m_playing = false;
        double m_currentTime = 0.0;
        double m_fraction = 0.0;
        bool m_loop = false;
        double m_speed = 1.0;
    };

    class MotionTextureImpl : public ImplObject<MotionTextureImpl>
    {
    public:
        MotionTextureImpl();
        virtual ~MotionTextureImpl();

        HRESULT Initialize(MotionTextureNode* node, ID3D11Device* dev, ResourceCache* cache);
        void Shutdown();
        void Update(double time);

        HRESULT GetTexture(ID3D11Device* dev, ID3D11ShaderResourceView** ppSRV);

        void SetOffset(float offset) { m_offset = offset; }
        float GetOffset() const { return m_offset; }

    private:
        MotionTextureResource* m_resource = nullptr;
        ResourceCache* m_cache = nullptr;
        ID3D11Device* m_device = nullptr;
        float m_offset = 0.0f;
        float m_scale = 1.0f;
    };

    class TextureTransformImpl : public ImplObject<TextureTransformImpl>
    {
    public:
        TextureTransformImpl();
        virtual ~TextureTransformImpl();

        HRESULT Initialize(TextureTransformNode* node);
        void Shutdown();

        Matrix4f GetMatrix() const;

    private:
        Vec2 m_translation;
        float m_rotation = 0.0f;
        Vec2 m_scale = Vec2(1, 1);
    };

    class TexturePropertiesImpl : public ImplObject<TexturePropertiesImpl>
    {
    public:
        TexturePropertiesImpl();
        virtual ~TexturePropertiesImpl();

        HRESULT Initialize(TexturePropertiesNode* node);
        void Shutdown();

        D3D11_FILTER GetFilter() const;
        D3D11_TEXTURE_ADDRESS_MODE GetAddressMode() const;
        bool GenerateMipMaps() const { return m_generateMipMaps; }

    private:
        bool m_generateMipMaps = true;
        std::string m_magFilter = "LINEAR";
        std::string m_minFilter = "LINEAR";
    };

} // namespace HMREngine
