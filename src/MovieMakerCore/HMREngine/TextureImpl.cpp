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
        if (!node->m_url.empty())
        {
            m_resource = cache->GetAVResource(node->m_url);
        }
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
        if (!m_cache || !dev || !ppSRV) return E_POINTER;

        if (!m_resource)
        {
            // Try to load from the first URL
            if (!m_cache) return E_FAIL;
            const std::string& path = m_resource ? m_resource->path : "";
            if (path.empty()) return E_FAIL;
            m_resource = m_cache->GetAVResource(path);
        }

        if (!m_resource || !m_resource->decoder) return E_FAIL;

        // The AVResource holds decoded video data; create a texture for the current frame.
        // If no decoder, return failure.
        D3D11_TEXTURE2D_DESC td{};
        td.Width = m_resource->width > 0 ? m_resource->width : 1;
        td.Height = m_resource->height > 0 ? m_resource->height : 1;
        td.MipLevels = 1;
        td.ArraySize = 1;
        td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        td.SampleDesc.Count = 1;
        td.Usage = D3D11_USAGE_DEFAULT;
        td.BindFlags = D3D11_BIND_SHADER_RESOURCE;

        CComPtr<ID3D11Texture2D> tex;
        HRESULT hr = dev->CreateTexture2D(&td, nullptr, &tex);
        if (FAILED(hr)) return hr;

        hr = dev->CreateShaderResourceView(tex, nullptr, ppSRV);
        return hr;
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
        if (!node->m_url.empty())
        {
            m_resource = cache->GetMotionTexture(node->m_url);
        }
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
        if (!m_cache || !dev || !ppSRV) return E_POINTER;
        if (!m_resource) return E_FAIL;

        if (m_resource->srv)
        {
            *ppSRV = m_resource->srv;
            (*ppSRV)->AddRef();
            return S_OK;
        }

        if (!m_resource->path.empty())
        {
            MotionTextureResource* mt = m_cache->GetMotionTexture(m_resource->path);
            if (mt && mt->srv)
            {
                m_resource = mt;
                *ppSRV = mt->srv;
                (*ppSRV)->AddRef();
                return S_OK;
            }
        }

        return E_FAIL;
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
