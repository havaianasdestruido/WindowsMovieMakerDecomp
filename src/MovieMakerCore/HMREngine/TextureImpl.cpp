#include "pch.h"

// TextureImpl.cpp - Texture implementations

#include "TextureImpl.h"

namespace HMREngine
{
    // ImageTextureImpl
    ImageTextureImpl::ImageTextureImpl() {}
    ImageTextureImpl::~ImageTextureImpl() { Shutdown(); }

    HRESULT ImageTextureImpl::Initialize(ImageTextureNode* node, ID3D11Device* dev, ResourceCache* cache)
    {
        if (!node || !dev || !cache) return E_POINTER;
        m_url = node->m_url;
        m_device = dev;
        m_cache = cache;
        return S_OK;
    }

    void ImageTextureImpl::Shutdown()
    {
        if (m_cache && !m_url.empty()) m_cache->ReleaseTexture(m_url);
        m_resource = nullptr;
    }

    void ImageTextureImpl::Update(double time) {}

    HRESULT ImageTextureImpl::GetTexture(ID3D11Device* dev, ID3D11ShaderResourceView** ppSRV)
    {
        if (!m_cache || m_url.empty()) return E_FAIL;

        m_resource = m_cache->GetTexture(m_url);
        if (!m_resource) return E_FAIL;

        *ppSRV = m_resource->srv;
        (*ppSRV)->AddRef();
        m_width = m_resource->width;
        m_height = m_resource->height;
        m_loaded = true;
        return S_OK;
    }

    // MovieTextureImpl
    MovieTextureImpl::MovieTextureImpl() {}
    MovieTextureImpl::~MovieTextureImpl() { Shutdown(); }

    HRESULT MovieTextureImpl::Initialize(MovieTextureNode* node, ID3D11Device* dev, ResourceCache* cache)
    {
        if (!node || !dev || !cache) return E_POINTER;
        m_device = dev;
        m_cache = cache;
        m_loop = node->m_loop;
        m_speed = node->m_speed;
        m_playing = node->m_play;
        return S_OK;
    }

    void MovieTextureImpl::Shutdown()
    {
        if (m_cache && m_resource) m_cache->ReleaseAVResource(m_resource->path);
    }

    void MovieTextureImpl::Update(double time)
    {
        if (!m_playing) return;
        m_currentTime += time * m_speed;
        if (m_resource && m_resource->duration > 0)
        {
            m_fraction = fmod(m_currentTime, m_resource->duration) / m_resource->duration;
            if (!m_loop && m_currentTime >= m_resource->duration)
            {
                m_playing = false;
                m_fraction = 1.0;
            }
        }
    }

    HRESULT MovieTextureImpl::GetTexture(ID3D11Device* dev, ID3D11ShaderResourceView** ppSRV)
    {
        return E_NOTIMPL;
    }

    // MotionTextureImpl
    MotionTextureImpl::MotionTextureImpl() {}
    MotionTextureImpl::~MotionTextureImpl() { Shutdown(); }

    HRESULT MotionTextureImpl::Initialize(MotionTextureNode* node, ID3D11Device* dev, ResourceCache* cache)
    {
        if (!node || !dev || !cache) return E_POINTER;
        m_device = dev;
        m_cache = cache;
        m_offset = node->m_offset;
        m_scale = node->m_scale;
        return S_OK;
    }

    void MotionTextureImpl::Shutdown()
    {
        if (m_cache && m_resource) m_cache->ReleaseMotionTexture(m_resource->path);
    }

    void MotionTextureImpl::Update(double time)
    {
        m_offset += static_cast<float>(time) * m_scale;
    }

    HRESULT MotionTextureImpl::GetTexture(ID3D11Device* dev, ID3D11ShaderResourceView** ppSRV)
    {
        return E_NOTIMPL;
    }

    // TextureTransformImpl
    TextureTransformImpl::TextureTransformImpl() {}
    TextureTransformImpl::~TextureTransformImpl() { Shutdown(); }

    HRESULT TextureTransformImpl::Initialize(TextureTransformNode* node)
    {
        if (!node) return E_POINTER;
        m_translation = node->m_translation;
        m_rotation = node->m_rotation;
        m_scale = node->m_scale;
        return S_OK;
    }

    void TextureTransformImpl::Shutdown() {}

    Matrix4f TextureTransformImpl::GetMatrix() const
    {
        Matrix4f m = Matrix4f::Identity();
        m._11 = m_scale.x;
        m._22 = m_scale.y;
        m._41 = m_translation.x;
        m._42 = m_translation.y;

        float cs = cosf(m_rotation);
        float sn = sinf(m_rotation);
        Matrix4f rot = Matrix4f::Identity();
        rot._11 = cs; rot._12 = -sn;
        rot._21 = sn; rot._22 = cs;

        return m * rot;
    }

    // TexturePropertiesImpl
    TexturePropertiesImpl::TexturePropertiesImpl() {}
    TexturePropertiesImpl::~TexturePropertiesImpl() { Shutdown(); }

    HRESULT TexturePropertiesImpl::Initialize(TexturePropertiesNode* node)
    {
        if (!node) return E_POINTER;
        m_generateMipMaps = node->m_generateMipMaps;
        m_magFilter = node->m_magFilter;
        m_minFilter = node->m_minFilter;
        return S_OK;
    }

    void TexturePropertiesImpl::Shutdown() {}

    D3D11_FILTER TexturePropertiesImpl::GetFilter() const
    {
        if (m_magFilter == "NEAREST" || m_minFilter == "NEAREST")
            return D3D11_FILTER_MIN_MAG_MIP_POINT;
        return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    }

    D3D11_TEXTURE_ADDRESS_MODE TexturePropertiesImpl::GetAddressMode() const
    {
        return D3D11_TEXTURE_ADDRESS_WRAP;
    }

} // namespace HMREngine
