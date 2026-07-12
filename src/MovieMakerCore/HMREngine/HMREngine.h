#pragma once
// HMREngine.h - Homer Media Rendering Engine root header
// DirectX 11-based rendering engine for MovieMakerCore

#include <atlbase.h>
#include <atlcom.h>
#include <atlstr.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <dxgi.h>
#include <DirectXMath.h>
#include <xstring>
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
#pragma comment(lib, "d3dx11.lib")
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

} // namespace HMREngine
