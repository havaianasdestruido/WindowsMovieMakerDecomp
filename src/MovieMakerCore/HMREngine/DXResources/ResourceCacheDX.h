#pragma once
// ResourceCacheDX.h - D3D11 resource cache, thumbnails, A/V sync resources

#include "DXResources.h"
#include "TextureResourceDX.h"
#include "MeshResourceDX.h"
#include "EffectResourceDX.h"

namespace HMREngine
{
    namespace DX
    {
        // --- MovieThumbnail: Thumbnail data ---
        struct MovieThumbnail
        {
            std::wstring filePath;
            CComPtr<ID3D11Texture2D> texture;
            CComPtr<ID3D11ShaderResourceView> srv;
            UINT width = 0;
            UINT height = 0;
            double timestamp = 0.0;
            size_t refCount = 1;
        };

        // --- MovieThumbnailDX: Thumbnail rendering ---
        class MovieThumbnailDX : public DeviceClientDXImpl
        {
        public:
            MovieThumbnailDX();
            virtual ~MovieThumbnailDX();

            HRESULT GenerateThumbnail(const std::wstring& videoPath, double timestamp,
                UINT thumbWidth, UINT thumbHeight, MovieThumbnail* outThumb);
            HRESULT GenerateThumbnailFromTexture(ID3D11Texture2D* source,
                UINT thumbWidth, UINT thumbHeight, MovieThumbnail* outThumb);
            void ReleaseThumbnail(MovieThumbnail* thumb);

        protected:
            CComPtr<ID3D11Texture2D> m_stagingTexture;
            CComPtr<ID3D11Texture2D> m_resizeTexture;
            CComPtr<ID3D11RenderTargetView> m_resizeRTV;
        };

        // --- AVResourceClock: A/V synchronization clock ---
        class AVResourceClock
        {
        public:
            AVResourceClock();
            ~AVResourceClock();

            void Start();
            void Stop();
            void Pause();
            void Resume();
            void Reset();

            double GetCurrentTime() const;
            void SetPlaybackRate(double rate) { m_playbackRate = rate; }
            double GetPlaybackRate() const { return m_playbackRate; }
            bool IsRunning() const { return m_running; }

            void SetAudioSyncOffset(double offset) { m_audioSyncOffset = offset; }
            double GetAudioSyncOffset() const { return m_audioSyncOffset; }

            double GetWallClockTime() const;

        protected:
            LARGE_INTEGER m_frequency{};
            LARGE_INTEGER m_startCounter{};
            double m_pauseOffset = 0.0;
            double m_playbackRate = 1.0;
            double m_audioSyncOffset = 0.0;
            bool m_running = false;
            bool m_paused = false;
        };

        // --- AVResourceDX: A/V synchronization resource ---
        class AVResourceDX : public DeviceClientDXImpl
        {
        public:
            AVResourceDX();
            virtual ~AVResourceDX();

            HRESULT Initialize(ID3D11Device* dev, ID3D11DeviceContext* ctx);
            void Release();

            AVResourceClock* GetClock() { return &m_clock; }

            HRESULT PresentFrame(double presentationTime);
            double GetNextFrameTime() const;

            void SetFrameRate(double fps) { m_frameRate = fps; }
            double GetFrameRate() const { return m_frameRate; }

            void SetDuration(double duration) { m_duration = duration; }
            double GetDuration() const { return m_duration; }

            bool IsDone() const;

        protected:
            AVResourceClock m_clock;
            double m_frameRate = 30.0;
            double m_duration = 0.0;
            double m_frameDuration = 1.0 / 30.0;
            double m_nextFrameTime = 0.0;
            std::queue<double> m_frameQueue;
        };

        // --- ResourceCacheDX: D3D11 resource cache ---
        class ResourceCacheDX : public DeviceClientDXImpl
        {
        public:
            ResourceCacheDX();
            virtual ~ResourceCacheDX();

            HRESULT Initialize(ID3D11Device* dev, ID3D11DeviceContext* ctx);
            void Release();

            // Texture resources
            TextureResourceDX* GetTexture(const std::wstring& path);
            TextureResourceDX* GetTextureFromResource(HMODULE hModule, UINT resourceId);
            void ReleaseTexture(const std::wstring& path);

            // Effect resources
            EffectResourceDX* GetEffect(const std::wstring& path);
            void ReleaseEffect(const std::wstring& path);

            // Mesh resources
            MeshResourceDX* GetMesh(const std::string& name);
            void AddMesh(const std::string& name, std::unique_ptr<MeshResourceDX> mesh);
            void ReleaseMesh(const std::string& name);

            // Motion texture resources
            MotionTextureResourceDX* GetMotionTexture(const std::wstring& path);
            void ReleaseMotionTexture(const std::wstring& path);

            // Thumbnails
            MovieThumbnailDX* GetThumbnailManager() { return m_thumbnailManager.get(); }

            // AV resources
            AVResourceDX* GetAVResource(const std::wstring& path);
            void ReleaseAVResource(const std::wstring& path);

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
            size_t GetTextureMemory() const;

            // LRU cache management
            void SetMaxTextureMemory(size_t maxBytes) { m_maxTextureMemory = maxBytes; }
            size_t GetMaxTextureMemory() const { return m_maxTextureMemory; }

        protected:
            mutable std::mutex m_mutex;

            std::unordered_map<std::wstring, std::unique_ptr<TextureResourceDX>> m_textures;
            std::unordered_map<std::wstring, std::unique_ptr<EffectResourceDX>> m_effects;
            std::unordered_map<std::string, std::unique_ptr<MeshResourceDX>> m_meshes;
            std::unordered_map<std::wstring, std::unique_ptr<MotionTextureResourceDX>> m_motionTextures;
            std::unordered_map<std::wstring, std::unique_ptr<AVResourceDX>> m_avResources;
            std::unique_ptr<MovieThumbnailDX> m_thumbnailManager;

            std::unordered_map<std::wstring, uint64_t> m_textureAccessOrder;
            uint64_t m_accessCounter = 0;
            size_t m_maxTextureMemory = 256 * 1024 * 1024;

            void TouchTexture(const std::wstring& path);
            void EvictLeastUsedTextures();
        };

    } // namespace DX
} // namespace HMREngine
