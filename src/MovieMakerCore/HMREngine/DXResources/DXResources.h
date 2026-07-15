#pragma once
// DXResources.h - Root header for all DX resource types in HMREngine::DX namespace

#include "../HMREngine.h"
#include "../X3DMath.h"
#include "../Shaders.h"
#include "../ResourceCache.h"
#include "../X3DNodes.h"
#include <d3d11.h>
#include "../d3dx11compat.h"
#include <dxgi.h>
#include <atlbase.h>
#include <atlcom.h>
#include <d2d1.h>
#include <dwrite.h>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <wrl/client.h>

namespace HMREngine
{
    namespace DX
    {
        // --- Forward declarations ---
        class EngineDX;
        class PreviewDX;
        class DefaultPreviewDX;
        class EncodeDX;
        class DeviceClientDXImpl;

        class SnapShotDX;
        class RenderLoopDX;
        class ResourceCacheDX;

        class MovieThumbnail;
        class MovieThumbnailDX;

        class TextureResourceDX;
        class TextureResourceFromResourceDX;
        class MotionTextureResourceDX;

        class AVResourceClock;
        class AVResourceDX;

        class MeshResourceDX;
        class ComposedGeometryResourceDX;
        class GridResourceDX;
        class ShatterGridResourceDX;
        class PageCurlGridResourceDX;
        class ScrollingTextResourceDX;

        class TextResourceBaseDX;
        class TextResourceDX;
        class EffectResourceDX;
        class CommonEffectResourceDX;
        class SimpleEffectResourceBase;
        class BlurEffectResource;
        class BrightnessEffectResource;
        class ChannelMixerEffectResource;
        class DissolveEffectResource;
        class AnimatedEffectResourceBase;
        class EdgeDetectionEffectResource;
        class FadeEffectResource;
        class GridEffectResource;
        class HueEffectResource;
        class PosterizeEffectResource;
        class PixelateEffectResource;
        class RippleEffectResource;
        class ScrollingTextEffectResourceDX;
        class TextEffectResourceDX;
        class WipeEffectResource;
        class DefaultEffectResourceDX;

        class SharedSwapChainDX;
        class BackBufferDX;
        class PartialTextRendererDX;
        class FrameBufferImplDX;
        class WipeMeshResourceDX;
        class PreprocessMesh;

        // --- Device feature flags ---
        struct DeviceFeatures
        {
            bool computeShader = false;
            bool doublePrecision = false;
            bool tessellation = false;
            bool multiRenderTargets = false;
            bool instancing = false;
            bool streamOutput = false;
            bool bc1Compression = false;
            bool bc3Compression = false;
            bool bc5Compression = false;
            bool extendedFormats = false;
            float maxAnisotropy = 1.0f;
            D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_10_0;
            UINT maxTextureSize = 8192;
            UINT maxRenderTargets = 1;
        };

        // --- Adapter enumeration ---
        struct AdapterInfo
        {
            std::wstring description;
            UINT vendorId = 0;
            UINT deviceId = 0;
            SIZE_T dedicatedVideoMemory = 0;
            SIZE_T dedicatedSystemMemory = 0;
            SIZE_T sharedSystemMemory = 0;
            DXGI_FORMAT bestDisplayFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
        };

        HRESULT EnumerateAdapters(std::vector<AdapterInfo>& adapters);
        HRESULT GetDeviceFeatures(ID3D11Device* dev, DeviceFeatures& features);

    } // namespace DX
} // namespace HMREngine
