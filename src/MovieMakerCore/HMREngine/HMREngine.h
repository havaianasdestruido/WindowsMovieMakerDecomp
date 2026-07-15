#pragma once
// HMREngine.h - Homer Media Rendering Engine root header
// DirectX 11-based rendering engine for MovieMakerCore

#include <atlbase.h>
#include <atlcom.h>
#include <atlstr.h>
#include <d3d11.h>
#include <dxgi.h>
#include <DirectXMath.h>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <memory>
#include <functional>
#include <mutex>
#include <atomic>
#include <queue>
#include <set>
#include <algorithm>
#include <cmath>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

namespace HMREngine
{
    class Engine;
    class Scene;
    class Browser;
    class ExecutionContext;
    class RenderLoop;
    class ScenePreview;
    class SceneEncode;
    class ErrHandler;
    class X3DReader;
    class ResourceCache;
    class RenderingList;

    // Smart pointer types
    template<typename T> class SAIPtr;
    template<typename T> class ImplObject;

    // Forward declarations for X3D nodes
    class X3DNode;
    class X3DChildNode;
    class X3DGroupingNode;
    class X3DShapeNode;
    class X3DGeometryNode;
    class X3DTextureNode;
    class X3DMaterialNode;
    class X3DFieldNode;

    // Forward declarations for implementation objects
    class ImageTextureImpl;
    class MovieTextureImpl;
    class MotionTextureImpl;
    class MaterialImpl;
    class AppearanceImpl;
    class ShapeImpl;
    class TransformImpl;
    class GroupImpl;
    class SwitchImpl;
    class TimeSensorImpl;
    class GridImpl;
    class TextImpl;
    class IndexedFaceSetImpl;
    class LayerImpl;
    class LayerSetImpl;
    class ViewportImpl;

    // Forward declarations for shaders
    class GridShader;
    class FadeShader;
    class DissolveShader;
    class ChannelMixerShader;
    class BrightnessShader;
    class BlurShader;
    class PosterizeShader;
    class PixelateShader;
    class RippleShader;
    class HueShader;
    class EdgeDetectionShader;
    class TextShader;
    class ScrollingTextShader;
    class WipeShader;

    // Forward declarations for interpolators/sequencers
    class PositionInterpolatorImpl;
    class OrientationInterpolatorImpl;
    class ScalarInterpolatorImpl;
    class ColorInterpolatorImpl;
    class CoordinateInterpolatorImpl;
    class IntegerSequencerImpl;
    class BooleanSequencerImpl;

    // Forward declarations for fields
    class SingleField;
    class MultipleField;
    template<typename T> class X3DArrayField;

    // Resource handle types
    struct EffectResource;
    struct AVResource;
    struct TextureResource;
    struct MeshResource;
    struct MotionTextureResource;

    // Return codes
    // Temporarily undef Windows SDK macros that conflict with our enum values
#undef S_OK
#undef E_FAIL
#undef E_OUTOFMEMORY
#undef E_INVALIDARG
#undef E_NOTIMPL
#undef E_POINTER
#undef E_UNEXPECTED
#undef E_ACCESSDENIED
#undef E_ABORT
#undef DXGI_ERROR_DEVICE_REMOVED
#undef DXGI_ERROR_DEVICE_RESET
#undef D3DERR_DEVICELOST
#undef D3DERR_DEVICENOTRESET

    enum class HMRError : int
    {
        S_OK = 0,
        E_FAIL = 1,
        E_OUTOFMEMORY = 2,
        E_INVALIDARG = 3,
        E_NOTIMPL = 4,
        E_POINTER = 5,
        E_UNEXPECTED = 6,
        E_ACCESSDENIED = 7,
        E_ABORT = 8,
        DXGI_ERROR_DEVICE_REMOVED = 9,
        DXGI_ERROR_DEVICE_RESET = 10,
        D3DERR_DEVICELOST = 11,
        D3DERR_DEVICENOTRESET = 12,
    };

    // Restore the original Windows SDK macros
#ifndef S_OK
#define S_OK ((HRESULT)0L)
#endif
#ifndef E_FAIL
#define E_FAIL ((HRESULT)0x80004005L)
#endif
#ifndef E_OUTOFMEMORY
#define E_OUTOFMEMORY ((HRESULT)0x8007000EL)
#endif
#ifndef E_INVALIDARG
#define E_INVALIDARG ((HRESULT)0x80070057L)
#endif
#ifndef E_NOTIMPL
#define E_NOTIMPL ((HRESULT)0x80004001L)
#endif
#ifndef E_POINTER
#define E_POINTER ((HRESULT)0x80004003L)
#endif
#ifndef E_UNEXPECTED
#define E_UNEXPECTED ((HRESULT)0x8000FFFFL)
#endif
#ifndef E_ACCESSDENIED
#define E_ACCESSDENIED ((HRESULT)0x80070005L)
#endif
#ifndef E_ABORT
#define E_ABORT ((HRESULT)0x40001L)
#endif
#ifndef DXGI_ERROR_DEVICE_REMOVED
#define DXGI_ERROR_DEVICE_REMOVED ((HRESULT)0x887A0005L)
#endif
#ifndef DXGI_ERROR_DEVICE_RESET
#define DXGI_ERROR_DEVICE_RESET ((HRESULT)0x887A0006L)
#endif
#ifndef D3DERR_DEVICELOST
#define D3DERR_DEVICELOST ((HRESULT)0x887608C8L)
#endif
#ifndef D3DERR_DEVICENOTRESET
#define D3DERR_DEVICENOTRESET ((HRESULT)0x887608C9L)
#endif

} // namespace HMREngine
