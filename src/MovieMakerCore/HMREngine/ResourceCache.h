#pragma once
// ResourceCache.h - HMREngine::ResourceCache

#include "HMREngine.h"
#include <d3d11.h>
#include "d3dx11compat.h"
#include <atlbase.h>
#include <string>
#include <unordered_map>
#include <memory>
#include <mutex>

namespace HMREngine
{
    struct EffectResource
    {
        std::string path;
        CComPtr<ID3DX11Effect> effect;
        CComPtr<ID3DX11EffectTechnique> technique;
        size_t refCount = 1;
    };

    struct AVResource
    {
        std::string path;
        void* decoder = nullptr;
        UINT width = 0;
        UINT height = 0;
        double duration = 0.0;
        size_t refCount = 1;
    };

    struct TextureResource
    {
        std::string path;
        CComPtr<ID3D11Texture2D> texture;
        CComPtr<ID3D11ShaderResourceView> srv;
        UINT width = 0;
        UINT height = 0;
        DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
        size_t refCount = 1;
    };

    struct MeshResource
    {
        std::string name;
        CComPtr<ID3D11Buffer> vertexBuffer;
        CComPtr<ID3D11Buffer> indexBuffer;
        UINT vertexCount = 0;
        UINT indexCount = 0;
        UINT vertexStride = 0;
        size_t refCount = 1;
    };

    struct MotionTextureResource
    {
        std::string path;
        CComPtr<ID3D11Texture2D> texture;
        CComPtr<ID3D11ShaderResourceView> srv;
        UINT width = 0;
        UINT height = 0;
        UINT frameCount = 0;
        UINT fps = 30;
        size_t refCount = 1;
    };

    class ResourceCache
    {
    public:
        ResourceCache();
        ~ResourceCache();

        void SetDevice(ID3D11Device* dev) { m_device = dev; }

        // Texture resources
        TextureResource* GetTexture(const std::string& path);
        void ReleaseTexture(const std::string& path);
        void PreloadTexture(const std::string& path);
        void ReloadTexture(const std::string& path);

        // Effect resources
        EffectResource* GetEffect(const std::string& path);
        void ReleaseEffect(const std::string& path);

        // Mesh resources
        MeshResource* GetMesh(const std::string& name);
        void AddMesh(const std::string& name, MeshResource* mesh);
        void ReleaseMesh(const std::string& name);

        // AV resources
        AVResource* GetAVResource(const std::string& path);
        void ReleaseAVResource(const std::string& path);

        // Motion texture resources
        MotionTextureResource* GetMotionTexture(const std::string& path);
        void ReleaseMotionTexture(const std::string& path);

        // Cache management
        void Purge();
        void PurgeTextures();
        void PurgeEffects();
        void PurgeMeshes();
        void PurgeAll();

        // Stats
        size_t GetTextureCount() const { return m_textures.size(); }
        size_t GetEffectCount() const { return m_effects.size(); }
        size_t GetMeshCount() const { return m_meshes.size(); }
        size_t GetAVResourceCount() const { return m_avResources.size(); }

        // Memory
        size_t GetTextureMemory() const;

    private:
        ID3D11Device* m_device = nullptr;
        mutable std::mutex m_mutex;

        std::unordered_map<std::string, std::unique_ptr<TextureResource>> m_textures;
        std::unordered_map<std::string, std::unique_ptr<EffectResource>> m_effects;
        std::unordered_map<std::string, std::unique_ptr<MeshResource>> m_meshes;
        std::unordered_map<std::string, std::unique_ptr<AVResource>> m_avResources;
        std::unordered_map<std::string, std::unique_ptr<MotionTextureResource>> m_motionTextures;

        HRESULT LoadTextureFromFile(const std::string& path, TextureResource* res);
        HRESULT LoadEffectFromFile(const std::string& path, EffectResource* res);
        std::string NormalizePath(const std::string& path);
    };

} // namespace HMREngine
