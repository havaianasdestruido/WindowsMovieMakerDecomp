#include "pch.h"

// ResourceCache.cpp - HMREngine::ResourceCache implementation

#include "ResourceCache.h"
#include <algorithm>

namespace HMREngine
{
    ResourceCache::ResourceCache()
    {
    }

    ResourceCache::~ResourceCache()
    {
        PurgeAll();
    }

    std::string ResourceCache::NormalizePath(const std::string& path)
    {
        std::string result = path;
        std::replace(result.begin(), result.end(), '\\', '/');
        // Lowercase for case-insensitive lookup
        std::transform(result.begin(), result.end(), result.begin(), ::tolower);
        return result;
    }

    TextureResource* ResourceCache::GetTexture(const std::string& path)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::string key = NormalizePath(path);

        auto it = m_textures.find(key);
        if (it != m_textures.end())
        {
            it->second->refCount++;
            return it->second.get();
        }

        if (!m_device) return nullptr;

        auto res = std::make_unique<TextureResource>();
        res->path = path;

        if (SUCCEEDED(LoadTextureFromFile(path, res.get())))
        {
            TextureResource* ptr = res.get();
            m_textures[key] = std::move(res);
            return ptr;
        }

        return nullptr;
    }

    HRESULT ResourceCache::LoadTextureFromFile(const std::string& path, TextureResource* res)
    {
        if (!m_device || !res) return E_POINTER;

        CComPtr<ID3DX11ShaderResourceView> srv;
        HRESULT hr = D3DX11CreateShaderResourceViewFromFileA(
            m_device, path.c_str(), nullptr, nullptr, &srv, nullptr);

        if (FAILED(hr)) return hr;

        CComPtr<ID3D11Resource> texRes;
        srv->GetResource(&texRes);

        CComPtr<ID3D11Texture2D> tex2D;
        hr = texRes->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&tex2D);
        if (SUCCEEDED(hr))
        {
            D3D11_TEXTURE2D_DESC td;
            tex2D->GetDesc(&td);
            res->width = td.Width;
            res->height = td.Height;
            res->format = td.Format;
            res->texture = tex2D;
        }

        res->srv = srv;
        return S_OK;
    }

    void ResourceCache::ReleaseTexture(const std::string& path)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::string key = NormalizePath(path);

        auto it = m_textures.find(key);
        if (it != m_textures.end())
        {
            it->second->refCount--;
            if (it->second->refCount == 0)
            {
                m_textures.erase(it);
            }
        }
    }

    void ResourceCache::PreloadTexture(const std::string& path)
    {
        GetTexture(path);
    }

    void ResourceCache::ReloadTexture(const std::string& path)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::string key = NormalizePath(path);
        m_textures.erase(key);
    }

    EffectResource* ResourceCache::GetEffect(const std::string& path)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::string key = NormalizePath(path);

        auto it = m_effects.find(key);
        if (it != m_effects.end())
        {
            it->second->refCount++;
            return it->second.get();
        }

        if (!m_device) return nullptr;

        auto res = std::make_unique<EffectResource>();
        res->path = path;

        if (SUCCEEDED(LoadEffectFromFile(path, res.get())))
        {
            EffectResource* ptr = res.get();
            m_effects[key] = std::move(res);
            return ptr;
        }

        return nullptr;
    }

    HRESULT ResourceCache::LoadEffectFromFile(const std::string& path, EffectResource* res)
    {
        if (!m_device || !res) return E_POINTER;

        DWORD flags = 0;
#ifdef _DEBUG
        flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

        CComPtr<ID3DX11Effect> effect;
        CComPtr<ID3DX11EffectPooling> pooling;

        HRESULT hr = D3DX11CompileEffectFromFileA(
            path.c_str(), nullptr, nullptr,
            flags, 0, m_device, &effect, nullptr);

        if (FAILED(hr)) return hr;

        res->effect = effect;

        // Try to get default technique
        D3DX11_EFFECT_DESC ed;
        if (SUCCEEDED(effect->GetDesc(&ed)) && ed.Techniques > 0)
        {
            res->technique = effect->GetTechniqueByIndex(0);
        }

        return S_OK;
    }

    void ResourceCache::ReleaseEffect(const std::string& path)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::string key = NormalizePath(path);

        auto it = m_effects.find(key);
        if (it != m_effects.end())
        {
            it->second->refCount--;
            if (it->second->refCount == 0)
            {
                m_effects.erase(it);
            }
        }
    }

    MeshResource* ResourceCache::GetMesh(const std::string& name)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::string key = NormalizePath(name);

        auto it = m_meshes.find(key);
        if (it != m_meshes.end())
        {
            it->second->refCount++;
            return it->second.get();
        }
        return nullptr;
    }

    void ResourceCache::AddMesh(const std::string& name, MeshResource* mesh)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::string key = NormalizePath(name);
        m_meshes[key].reset(mesh);
    }

    void ResourceCache::ReleaseMesh(const std::string& name)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::string key = NormalizePath(name);

        auto it = m_meshes.find(key);
        if (it != m_meshes.end())
        {
            it->second->refCount--;
            if (it->second->refCount == 0) m_meshes.erase(it);
        }
    }

    AVResource* ResourceCache::GetAVResource(const std::string& path)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::string key = NormalizePath(path);

        auto it = m_avResources.find(key);
        if (it != m_avResources.end())
        {
            it->second->refCount++;
            return it->second.get();
        }

        auto res = std::make_unique<AVResource>();
        res->path = path;
        AVResource* ptr = res.get();
        m_avResources[key] = std::move(res);
        return ptr;
    }

    void ResourceCache::ReleaseAVResource(const std::string& path)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::string key = NormalizePath(path);

        auto it = m_avResources.find(key);
        if (it != m_avResources.end())
        {
            it->second->refCount--;
            if (it->second->refCount == 0) m_avResources.erase(it);
        }
    }

    MotionTextureResource* ResourceCache::GetMotionTexture(const std::string& path)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::string key = NormalizePath(path);

        auto it = m_motionTextures.find(key);
        if (it != m_motionTextures.end())
        {
            it->second->refCount++;
            return it->second.get();
        }
        return nullptr;
    }

    void ResourceCache::ReleaseMotionTexture(const std::string& path)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::string key = NormalizePath(path);

        auto it = m_motionTextures.find(key);
        if (it != m_motionTextures.end())
        {
            it->second->refCount--;
            if (it->second->refCount == 0) m_motionTextures.erase(it);
        }
    }

    void ResourceCache::Purge() { PurgeAll(); }

    void ResourceCache::PurgeTextures()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_textures.clear();
    }

    void ResourceCache::PurgeEffects()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_effects.clear();
    }

    void ResourceCache::PurgeMeshes()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_meshes.clear();
    }

    void ResourceCache::PurgeAll()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_textures.clear();
        m_effects.clear();
        m_meshes.clear();
        m_avResources.clear();
        m_motionTextures.clear();
    }

    size_t ResourceCache::GetTextureMemory() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        size_t total = 0;
        for (auto& kv : m_textures)
        {
            if (kv.second->texture)
            {
                D3D11_TEXTURE2D_DESC desc;
                kv.second->texture->GetDesc(&desc);
                total += desc.Width * desc.Height * 4; // Assume 32bpp
            }
        }
        return total;
    }

} // namespace HMREngine
