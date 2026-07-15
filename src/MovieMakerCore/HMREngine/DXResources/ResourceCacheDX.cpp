#include "pch.h"
// ResourceCacheDX.cpp - D3D11 resource cache implementation

#include "ResourceCacheDX.h"

namespace HMREngine
{
namespace DX
{

// ============================================================================
// MovieThumbnailDX
// ============================================================================
MovieThumbnailDX::MovieThumbnailDX() = default;
MovieThumbnailDX::~MovieThumbnailDX() = default;

HRESULT MovieThumbnailDX::GenerateThumbnail(const std::wstring& videoPath,
    double timestamp, UINT thumbWidth, UINT thumbHeight, MovieThumbnail* outThumb)
{
    if (!outThumb || !m_device) return E_POINTER;

    outThumb->filePath = videoPath;
    outThumb->width = thumbWidth;
    outThumb->height = thumbHeight;
    outThumb->timestamp = timestamp;

    D3D11_TEXTURE2D_DESC td = {};
    td.Width = thumbWidth;
    td.Height = thumbHeight;
    td.MipLevels = 1;
    td.ArraySize = 1;
    td.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    td.SampleDesc.Count = 1;
    td.Usage = D3D11_USAGE_DEFAULT;
    td.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    HRESULT hr = m_device->CreateTexture2D(&td, nullptr, &outThumb->texture);
    if (FAILED(hr)) return hr;

    hr = m_device->CreateShaderResourceView(outThumb->texture, nullptr, &outThumb->srv);
    return hr;
}

HRESULT MovieThumbnailDX::GenerateThumbnailFromTexture(ID3D11Texture2D* source,
    UINT thumbWidth, UINT thumbHeight, MovieThumbnail* outThumb)
{
    if (!source || !outThumb || !m_device) return E_POINTER;

    outThumb->width = thumbWidth;
    outThumb->height = thumbHeight;

    D3D11_TEXTURE2D_DESC srcDesc;
    source->GetDesc(&srcDesc);

    D3D11_TEXTURE2D_DESC td = {};
    td.Width = thumbWidth;
    td.Height = thumbHeight;
    td.MipLevels = 1;
    td.ArraySize = 1;
    td.Format = srcDesc.Format;
    td.SampleDesc.Count = 1;
    td.Usage = D3D11_USAGE_DEFAULT;
    td.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    HRESULT hr = m_device->CreateTexture2D(&td, nullptr, &outThumb->texture);
    if (FAILED(hr)) return hr;

    m_context->CopySubresourceRegion(outThumb->texture, 0, 0, 0, 0,
        source, 0, nullptr);

    hr = m_device->CreateShaderResourceView(outThumb->texture, nullptr, &outThumb->srv);
    return hr;
}

void MovieThumbnailDX::ReleaseThumbnail(MovieThumbnail* thumb)
{
    if (!thumb) return;
    thumb->srv.Release();
    thumb->texture.Release();
    thumb->width = 0;
    thumb->height = 0;
}

// ============================================================================
// AVResourceClock
// ============================================================================
AVResourceClock::AVResourceClock()
{
    QueryPerformanceFrequency(&m_frequency);
    Reset();
}

AVResourceClock::~AVResourceClock() = default;

void AVResourceClock::Start()
{
    QueryPerformanceCounter(&m_startCounter);
    m_pauseOffset = 0.0;
    m_running = true;
    m_paused = false;
}

void AVResourceClock::Stop()
{
    m_running = false;
    m_paused = false;
}

void AVResourceClock::Pause()
{
    if (m_running && !m_paused)
    {
        m_pauseOffset = GetCurrentTime();
        m_paused = true;
    }
}

void AVResourceClock::Resume()
{
    if (m_paused)
    {
        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);
        m_startCounter.QuadPart = now.QuadPart -
            static_cast<LONGLONG>(m_pauseOffset * m_frequency.QuadPart);
        m_paused = false;
    }
}

void AVResourceClock::Reset()
{
    QueryPerformanceCounter(&m_startCounter);
    m_pauseOffset = 0.0;
    m_running = false;
    m_paused = false;
}

double AVResourceClock::GetCurrentTime() const
{
    if (!m_running) return 0.0;
    if (m_paused) return m_pauseOffset;

    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    double elapsed = static_cast<double>(now.QuadPart - m_startCounter.QuadPart) / m_frequency.QuadPart;
    return elapsed * m_playbackRate;
}

double AVResourceClock::GetWallClockTime() const
{
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    return static_cast<double>(now.QuadPart) / m_frequency.QuadPart;
}

// ============================================================================
// AVResourceDX
// ============================================================================
AVResourceDX::AVResourceDX() = default;
AVResourceDX::~AVResourceDX() { Release(); }

HRESULT AVResourceDX::Initialize(ID3D11Device* dev, ID3D11DeviceContext* ctx)
{
    HRESULT hr = DeviceClientDXImpl::InitializeDevice(dev, ctx);
    if (SUCCEEDED(hr)) m_clock.Start();
    return hr;
}

void AVResourceDX::Release()
{
    m_clock.Stop();
    while (!m_frameQueue.empty()) m_frameQueue.pop();
    m_nextFrameTime = 0.0;
}

HRESULT AVResourceDX::PresentFrame(double presentationTime)
{
    m_nextFrameTime = presentationTime + m_frameDuration;
    return S_OK;
}

double AVResourceDX::GetNextFrameTime() const
{
    return m_nextFrameTime;
}

bool AVResourceDX::IsDone() const
{
    if (m_duration <= 0.0) return false;
    return m_clock.GetCurrentTime() >= m_duration;
}

// ============================================================================
// ResourceCacheDX
// ============================================================================
ResourceCacheDX::ResourceCacheDX() = default;
ResourceCacheDX::~ResourceCacheDX() { Release(); }

HRESULT ResourceCacheDX::Initialize(ID3D11Device* dev, ID3D11DeviceContext* ctx)
{
    HRESULT hr = DeviceClientDXImpl::InitializeDevice(dev, ctx);
    if (FAILED(hr)) return hr;

    m_thumbnailManager = std::make_unique<MovieThumbnailDX>();
    return m_thumbnailManager->InitializeDevice(dev, ctx);
}

void ResourceCacheDX::Release()
{
    PurgeAll();
    m_thumbnailManager.reset();
    DeviceClientDXImpl::ReleaseDevice();
}

TextureResourceDX* ResourceCacheDX::GetTexture(const std::wstring& path)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_textures.find(path);
    if (it != m_textures.end()) return it->second.get();

    auto tex = std::make_unique<TextureResourceDX>();
    tex->InitializeDevice(m_device, m_context);

    HRESULT hr = tex->LoadFromFile(path);
    if (FAILED(hr)) return nullptr;

    TextureResourceDX* result = tex.get();
    m_textures[path] = std::move(tex);
    return result;
}

TextureResourceDX* ResourceCacheDX::GetTextureFromResource(HMODULE hModule, UINT resourceId)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto tex = std::make_unique<TextureResourceFromResourceDX>();
    tex->InitializeDevice(m_device, m_context);

    HRESULT hr = tex->LoadFromResource(hModule, resourceId);
    if (FAILED(hr)) return nullptr;

    TextureResourceDX* result = tex.get();
    std::wstring key = L"res:" + std::to_wstring(resourceId);
    m_textures[key] = std::move(tex);
    return result;
}

void ResourceCacheDX::ReleaseTexture(const std::wstring& path)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_textures.erase(path);
}

EffectResourceDX* ResourceCacheDX::GetEffect(const std::wstring& path)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_effects.find(path);
    if (it != m_effects.end()) return it->second.get();

    auto effect = std::make_unique<EffectResourceDX>();
    effect->InitializeDevice(m_device, m_context);

    HRESULT hr = effect->LoadFromFile(path);
    if (FAILED(hr)) return nullptr;

    EffectResourceDX* result = effect.get();
    m_effects[path] = std::move(effect);
    return result;
}

void ResourceCacheDX::ReleaseEffect(const std::wstring& path)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_effects.erase(path);
}

MeshResourceDX* ResourceCacheDX::GetMesh(const std::string& name)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_meshes.find(name);
    if (it != m_meshes.end()) return it->second.get();

    return nullptr;
}

void ResourceCacheDX::AddMesh(const std::string& name, std::unique_ptr<MeshResourceDX> mesh)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_meshes[name] = std::move(mesh);
}

void ResourceCacheDX::ReleaseMesh(const std::string& name)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_meshes.erase(name);
}

MotionTextureResourceDX* ResourceCacheDX::GetMotionTexture(const std::wstring& path)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_motionTextures.find(path);
    if (it != m_motionTextures.end()) return it->second.get();
    return nullptr;
}

void ResourceCacheDX::ReleaseMotionTexture(const std::wstring& path)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_motionTextures.erase(path);
}

AVResourceDX* ResourceCacheDX::GetAVResource(const std::wstring& path)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_avResources.find(path);
    if (it != m_avResources.end()) return it->second.get();

    auto av = std::make_unique<AVResourceDX>();
    av->InitializeDevice(m_device, m_context);

    AVResourceDX* result = av.get();
    m_avResources[path] = std::move(av);
    return result;
}

void ResourceCacheDX::ReleaseAVResource(const std::wstring& path)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_avResources.erase(path);
}

void ResourceCacheDX::Purge()
{
    PurgeAll();
}

void ResourceCacheDX::PurgeTextures()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_textures.clear();
    m_motionTextures.clear();
}

void ResourceCacheDX::PurgeEffects()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_effects.clear();
}

void ResourceCacheDX::PurgeMeshes()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_meshes.clear();
}

void ResourceCacheDX::PurgeAll()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_avResources.clear();
    m_motionTextures.clear();
    m_textures.clear();
    m_effects.clear();
    m_meshes.clear();
}

size_t ResourceCacheDX::GetTextureMemory() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    size_t total = 0;
    for (const auto& kv : m_textures)
    {
        if (kv.second)
            total += kv.second->GetWidth() * kv.second->GetHeight() * 4;
    }
    return total;
}

} // namespace DX
} // namespace HMREngine
