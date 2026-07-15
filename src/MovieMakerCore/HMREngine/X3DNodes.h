#pragma once
// X3DNodes.h - All X3D node types for HMREngine

#include "HMREngine.h"
#include "X3DFieldTypes.h"
#include "X3DMath.h"
#include <atlbase.h>
#include <atlcom.h>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <functional>

namespace HMREngine
{
    // SAIPtr - Smart Auto-Increment Pointer
    template<typename T>
    class SAIPtr
    {
    public:
        T* m_p = nullptr;

        SAIPtr() = default;
        SAIPtr(T* p) : m_p(p) { if (m_p) m_p->AddRef(); }
        SAIPtr(const SAIPtr& o) : m_p(o.m_p) { if (m_p) m_p->AddRef(); }
        SAIPtr(SAIPtr&& o) noexcept : m_p(o.m_p) { o.m_p = nullptr; }
        ~SAIPtr() { if (m_p) m_p->Release(); }

        SAIPtr& operator=(const SAIPtr& o)
        {
            if (this != std::addressof(o)) { if (m_p) m_p->Release(); m_p = o.m_p; if (m_p) m_p->AddRef(); }
            return *this;
        }
        SAIPtr& operator=(SAIPtr&& o) noexcept
        {
            if (this != std::addressof(o)) { if (m_p) m_p->Release(); m_p = o.m_p; o.m_p = nullptr; }
            return *this;
        }

        T* operator->() const { return m_p; }
        T& operator*() const { return *m_p; }
        T** operator&() { return &m_p; }
        operator T*() const { return m_p; }
        operator bool() const { return m_p != nullptr; }

        T* Get() const { return m_p; }
        void Release() { if (m_p) { m_p->Release(); m_p = nullptr; } }
        void Attach(T* p) { if (m_p) m_p->Release(); m_p = p; }
        T* Detach() { T* p = m_p; m_p = nullptr; return p; }
    };

    // ImplObject<T> - Reference-counted implementation wrapper
    template<typename T>
    class ImplObject
    {
    protected:
        LONG m_refCount = 1;

    public:
        virtual ~ImplObject() = default;

        ULONG AddRef() { return InterlockedIncrement(&m_refCount); }
        ULONG Release()
        {
            ULONG c = InterlockedDecrement(&m_refCount);
            if (c == 0) { delete this; }
            return c;
        }
    };

    // Base X3D node interface
    struct __declspec(uuid("E5D73B50-3F7E-4B1E-8D3A-1A2F4C6E8B90")) IX3DNode : public IUnknown
    {
        virtual const char* GetNodeName() const = 0;
        virtual const char* GetNodeTypeName() const = 0;
    };

    // -------------------------------------------------------------------
    // X3DNode - Base class for all scene graph nodes
    // -------------------------------------------------------------------
    class X3DNode :

        public IX3DNode
    {
    public:
        std::string m_name;
        std::map<std::string, std::unique_ptr<X3DFieldNode>> m_fields;

        virtual ~X3DNode() = default;

        const char* GetNodeName() const override { return m_name.c_str(); }
        const char* GetNodeTypeName() const override { return "X3DNode"; }

        void SetField(const std::string& name, X3DFieldNode* field)
        {
            m_fields[name] = std::unique_ptr<X3DFieldNode>(field);
        }

        X3DFieldNode* GetField(const std::string& name) const
        {
            auto it = m_fields.find(name);
            return it != m_fields.end() ? it->second.get() : nullptr;
        }

        template<typename T>
        T* GetFieldAs(const std::string& name) const
        {
            return dynamic_cast<T*>(GetField(name));
        }
    };

    // -------------------------------------------------------------------
    // X3DChildNode - Nodes that can be children in the scene graph
    // -------------------------------------------------------------------
    class X3DChildNode :
        public X3DNode
    {
    public:
        std::vector<SAIPtr<X3DChildNode>> m_children;
        X3DChildNode* m_parent = nullptr;
        bool m_visible = true;

        virtual ~X3DChildNode() = default;

        const char* GetNodeTypeName() const override { return "X3DChildNode"; }

        void AddChild(X3DChildNode* child)
        {
            if (child) { child->AddRef(); child->m_parent = this; m_children.push_back(SAIPtr<X3DChildNode>(child)); }
        }

        void RemoveChild(X3DChildNode* child)
        {
            for (auto it = m_children.begin(); it != m_children.end(); ++it)
            {
                if (it->Get() == child) { child->m_parent = nullptr; m_children.erase(it); return; }
            }
        }

        size_t GetNumChildren() const { return m_children.size(); }
        X3DChildNode* GetChild(size_t i) { return m_children[i].Get(); }
    };

    // -------------------------------------------------------------------
    // X3DGroupingNode - Grouping nodes that hold child lists
    // -------------------------------------------------------------------
    class X3DGroupingNode :
        public X3DChildNode
    {
    public:

        const char* GetNodeTypeName() const override { return "X3DGroupingNode"; }
    };

    // -------------------------------------------------------------------
    // X3DShapeNode - Nodes that define visual appearance
    // -------------------------------------------------------------------
    class X3DShapeNode :
        public X3DChildNode
    {
    public:

        const char* GetNodeTypeName() const override { return "X3DShapeNode"; }
    };

    // -------------------------------------------------------------------
    // X3DGeometryNode - Geometry definitions
    // -------------------------------------------------------------------
    class X3DGeometryNode :
        public X3DChildNode
    {
    public:

        const char* GetNodeTypeName() const override { return "X3DGeometryNode"; }

        virtual HRESULT BuildVertexBuffer(ID3D11Device* pDev, ID3D11Buffer** ppVB) { return E_NOTIMPL; }
        virtual HRESULT BuildIndexBuffer(ID3D11Device* pDev, ID3D11Buffer** ppIB) { return E_NOTIMPL; }
        virtual UINT GetVertexCount() const { return 0; }
        virtual UINT GetIndexCount() const { return 0; }
    };

    // -------------------------------------------------------------------
    // X3DTextureNode - Texture definitions
    // -------------------------------------------------------------------
    class X3DTextureNode :
        public X3DChildNode
    {
    public:

        const char* GetNodeTypeName() const override { return "X3DTextureNode"; }

        virtual HRESULT GetTexture(ID3D11Device* pDev, ID3D11ShaderResourceView** ppSRV) { return E_NOTIMPL; }
        virtual void Update(double time) {}
    };

    // -------------------------------------------------------------------
    // X3DMaterialNode - Material definitions
    // -------------------------------------------------------------------
    class X3DMaterialNode :
        public X3DChildNode
    {
    public:

        const char* GetNodeTypeName() const override { return "X3DMaterialNode"; }
    };

    // -------------------------------------------------------------------
    // Concrete X3D Node Implementations
    // -------------------------------------------------------------------

    // -- Group --
    class GroupNode :
        public X3DGroupingNode
    {
    public:
        const char* GetNodeTypeName() const override { return "Group"; }
    };

    // -- Transform --
    class TransformNode :
        public X3DGroupingNode
    {
    public:
        Vec3 m_translation;
        Rotation4f m_rotation;
        Vec3 m_scale = Vec3(1,1,1);
        Vec3 m_scaleOrientation;
        Vec3 m_center;
        Matrix4f m_matrix;

        const char* GetNodeTypeName() const override { return "Transform"; }

        void UpdateMatrix()
        {
            m_matrix = Matrix4f::FromTranslationRotationScale(m_translation, m_rotation, m_scale);
        }
    };

    // -- Switch --
    class SwitchNode :
        public X3DGroupingNode
    {
    public:
        int m_choice = 0;
        bool m_enabled = true;

        const char* GetNodeTypeName() const override { return "Switch"; }
    };

    // -- Shape --
    class ShapeNode :
        public X3DShapeNode
    {
    public:
        SAIPtr<X3DGeometryNode> m_geometry;
        SAIPtr<X3DChildNode> m_appearance;
        Matrix4f m_worldMatrix;

        const char* GetNodeTypeName() const override { return "Shape"; }
    };

    // -- Appearance --
    class AppearanceNode :
        public X3DChildNode
    {
    public:
        SAIPtr<X3DTextureNode> m_texture;
        SAIPtr<X3DMaterialNode> m_material;
        std::unique_ptr<X3DFieldNode> m_shaders;

        const char* GetNodeTypeName() const override { return "Appearance"; }
    };

    // -- ImageTexture --
    class ImageTextureNode :
        public X3DTextureNode
    {
    public:
        std::string m_url;
        std::string m_fullPath;
        CComPtr<ID3D11ShaderResourceView> m_srv;
        CComPtr<ID3D11Texture2D> m_texture2D;
        bool m_loaded = false;
        bool m_repeatS = true;
        bool m_repeatT = true;
        std::string m_filterType = "LINEAR";

        const char* GetNodeTypeName() const override { return "ImageTexture"; }
        HRESULT GetTexture(ID3D11Device* pDev, ID3D11ShaderResourceView** ppSRV) override;
    };

    // -- MovieTexture --
    class MovieTextureNode :
        public X3DTextureNode
    {
    public:
        std::string m_url;
        bool m_loop = false;
        double m_speed = 1.0;
        bool m_play = false;
        double m_fraction = 0.0;

        const char* GetNodeTypeName() const override { return "MovieTexture"; }
        void Update(double time) override;
    };

    // -- MotionTexture (custom) --
    class MotionTextureNode :
        public X3DTextureNode
    {
    public:
        std::string m_url;
        float m_offset = 0.0f;
        float m_scale = 1.0f;

        const char* GetNodeTypeName() const override { return "MotionTexture"; }
    };

    // -- TextureTransform --
    class TextureTransformNode :
        public X3DChildNode
    {
    public:
        Vec2 m_translation;
        float m_rotation = 0.0f;
        Vec2 m_scale = Vec2(1, 1);
        Matrix4f m_matrix;

        const char* GetNodeTypeName() const override { return "TextureTransform"; }

        void UpdateMatrix()
        {
            m_matrix = Matrix4f::Identity();
            m_matrix._11 = m_scale.x;
            m_matrix._22 = m_scale.y;
            m_matrix._41 = m_translation.x;
            m_matrix._42 = m_translation.y;
        }
    };

    // -- TextureProperties --
    class TexturePropertiesNode :
        public X3DChildNode
    {
    public:
        bool m_generateMipMaps = true;
        std::string m_magFilter = "LINEAR";
        std::string m_minFilter = "LINEAR";

        const char* GetNodeTypeName() const override { return "TextureProperties"; }
    };

    // -- Material --
    class MaterialNode :
        public X3DMaterialNode
    {
    public:
        Rgb m_diffuseColor = Rgb(0.8f, 0.8f, 0.8f);
        Rgb m_specularColor = Rgb(0.0f, 0.0f, 0.0f);
        Rgb m_emissiveColor = Rgb(0.0f, 0.0f, 0.0f);
        Rgb m_ambientColor = Rgb(0.2f, 0.2f, 0.2f);
        float m_shininess = 0.2f;
        float m_transparency = 0.0f;

        const char* GetNodeTypeName() const override { return "Material"; }
    };

    // -- LineProperties --
    class LinePropertiesNode :
        public X3DChildNode
    {
    public:
        bool m_applied = true;
        int m_linetype = 1;
        float m_linewidthScaleFactor = 1.0f;

        const char* GetNodeTypeName() const override { return "LineProperties"; }
    };

    // -- FillProperties --
    class FillPropertiesNode :
        public X3DChildNode
    {
    public:
        bool m_applied = true;
        bool m_filled = true;

        const char* GetNodeTypeName() const override { return "FillProperties"; }
    };

    // -- TimeSensor --
    class TimeSensorNode :
        public X3DChildNode
    {
    public:
        double m_startTime = 0.0;
        double m_stopTime = 0.0;
        double m_pauseTime = 0.0;
        double m_resumeTime = 0.0;
        double m_cycleInterval = 1.0;
        bool m_loop = false;
        bool m_enabled = true;

        double m_fraction = 0.0;
        double m_elapsedTime = 0.0;
        double m_currentTime = 0.0;
        bool m_isActive = false;

        std::function<void(double)> m_onFractionChanged;
        std::function<void(bool)> m_onActiveChanged;

        const char* GetNodeTypeName() const override { return "TimeSensor"; }

        void Evaluate(double globalTime);
    };

    // -- TimeTrigger --
    class TimeTriggerNode :
        public X3DChildNode
    {
    public:
        bool m_enabled = true;
        double m_triggerTime = 0.0;

        const char* GetNodeTypeName() const override { return "TimeTrigger"; }
    };

    // -- Coordinate --
    class CoordinateNode :
        public X3DChildNode
    {
    public:
        std::vector<Vec3> m_point;

        const char* GetNodeTypeName() const override { return "Coordinate"; }
    };

    // -- Normal --
    class NormalNode :
        public X3DChildNode
    {
    public:
        std::vector<Vec3> m_vector;

        const char* GetNodeTypeName() const override { return "Normal"; }
    };

    // -- Color --
    class ColorNode :
        public X3DChildNode
    {
    public:
        std::vector<Rgb> m_color;

        const char* GetNodeTypeName() const override { return "Color"; }
    };

    // -- ColorRGBA --
    class ColorRGBANode :
        public X3DChildNode
    {
    public:
        std::vector<Rgba> m_color;

        const char* GetNodeTypeName() const override { return "ColorRGBA"; }
    };

    // -- TextureCoordinate --
    class TextureCoordinateNode :
        public X3DChildNode
    {
    public:
        std::vector<Vec2> m_point;

        const char* GetNodeTypeName() const override { return "TextureCoordinate"; }
    };

    // -- IndexedFaceSet --
    class IndexedFaceSetNode :
        public X3DGeometryNode
    {
    public:
        MFInt32 m_coordIndex;
        MFInt32 m_normalIndex;
        MFInt32 m_texCoordIndex;
        MFInt32 m_colorIndex;
        bool m_ccw = true;
        bool m_convex = true;
        bool m_solid = true;
        std::string m_creaseAngle = "0";

        SAIPtr<CoordinateNode> m_coord;
        SAIPtr<NormalNode> m_normal;
        SAIPtr<TextureCoordinateNode> m_texCoord;
        SAIPtr<ColorNode> m_color;
        SAIPtr<ColorRGBANode> m_colorRGBA;

        CComPtr<ID3D11Buffer> m_vertexBuffer;
        CComPtr<ID3D11Buffer> m_indexBuffer;
        UINT m_vertexCount = 0;
        UINT m_indexCount = 0;

        const char* GetNodeTypeName() const override { return "IndexedFaceSet"; }

        HRESULT BuildVertexBuffer(ID3D11Device* pDev, ID3D11Buffer** ppVB) override;
        HRESULT BuildIndexBuffer(ID3D11Device* pDev, ID3D11Buffer** ppIB) override;
        UINT GetVertexCount() const override { return m_vertexCount; }
        UINT GetIndexCount() const override { return m_indexCount; }
    };

    // -- FontStyle --
    class FontStyleNode :
        public X3DChildNode
    {
    public:
        std::string m_family = "SANS";
        std::string m_style = "PLAIN";
        float m_size = 1.0f;
        bool m_bold = false;
        bool m_italic = false;
        bool m_language = false;

        const char* GetNodeTypeName() const override { return "FontStyle"; }
    };

    // -- Text --
    class TextNode :
        public X3DGeometryNode
    {
    public:
        MFString m_string;
        MFVec2f m_length;
        MFVec2f m_maxExtent;
        SAIPtr<FontStyleNode> m_fontStyle;

        const char* GetNodeTypeName() const override { return "Text"; }
    };

    // -- Grid (custom node) --
    class GridNode :
        public X3DGeometryNode
    {
    public:
        float m_width = 1.0f;
        float m_height = 1.0f;
        int m_divisions = 4;
        Rgb m_lineColor = Rgb(0.5f, 0.5f, 0.5f);
        float m_lineWidth = 1.0f;
        bool m_showOrigin = true;

        const char* GetNodeTypeName() const override { return "Grid"; }
    };

    // -- Viewport --
    class ViewportNode :
        public X3DChildNode
    {
    public:
        float m_viewport[4] = { 0.0f, 0.0f, 1.0f, 1.0f }; // x, y, w, h normalized
        float m_viewMatrix[16];
        float m_projMatrix[16];
        Matrix4f m_view;
        Matrix4f m_projection;
        Vec3 m_eyePosition;
        float m_fieldOfView = 0.785398f; // PI/4
        float m_nearClip = 0.1f;
        float m_farClip = 1000.0f;

        const char* GetNodeTypeName() const override { return "Viewport"; }
    };

    // -- Layer --
    class LayerNode :
        public X3DGroupingNode
    {
    public:
        bool m_pickable = true;
        bool m_visible = true;

        const char* GetNodeTypeName() const override { return "Layer"; }
    };

    // -- LayerSet --
    class LayerSetNode :
        public X3DChildNode
    {
    public:
        SAIPtr<LayerNode> m_activeLayer;
        int m_ordering = 0; // 0=TRAV_ALL, 1=TRAV_LOCAL

        const char* GetNodeTypeName() const override { return "LayerSet"; }
    };

    // -------------------------------------------------------------------
    // Interpolator Nodes
    // -------------------------------------------------------------------

    class PositionInterpolatorNode :
        public X3DChildNode
    {
    public:
        MFFloat m_key;
        MFVec3f m_value;
        bool m_enabled = true;

        const char* GetNodeTypeName() const override { return "PositionInterpolator"; }
        Vec3 Interpolate(float fraction) const;
    };

    class OrientationInterpolatorNode :
        public X3DChildNode
    {
    public:
        MFFloat m_key;
        MFRotation m_value;
        bool m_enabled = true;

        const char* GetNodeTypeName() const override { return "OrientationInterpolator"; }
        Rotation4f Interpolate(float fraction) const;
    };

    class ScalarInterpolatorNode :
        public X3DChildNode
    {
    public:
        MFFloat m_key;
        MFFloat m_value;
        bool m_enabled = true;

        const char* GetNodeTypeName() const override { return "ScalarInterpolator"; }
        float Interpolate(float fraction) const;
    };

    class ColorInterpolatorNode :
        public X3DChildNode
    {
    public:
        MFFloat m_key;
        MFColor m_value;
        bool m_enabled = true;

        const char* GetNodeTypeName() const override { return "ColorInterpolator"; }
        Rgb Interpolate(float fraction) const;
    };

    class CoordinateInterpolatorNode :
        public X3DChildNode
    {
    public:
        MFFloat m_key;
        MFVec3f m_value;
        bool m_enabled = true;

        const char* GetNodeTypeName() const override { return "CoordinateInterpolator"; }
    };

    // -------------------------------------------------------------------
    // Sequencer Nodes
    // -------------------------------------------------------------------

    class IntegerSequencerNode :
        public X3DChildNode
    {
    public:
        MFFloat m_key;
        MFInt32 m_value;
        bool m_enabled = true;

        const char* GetNodeTypeName() const override { return "IntegerSequencer"; }
        int Evaluate(float fraction) const;
    };

    class BooleanSequencerNode :
        public X3DChildNode
    {
    public:
        MFFloat m_key;
        MFBool m_value;
        bool m_enabled = true;

        const char* GetNodeTypeName() const override { return "BooleanSequencer"; }
        bool Evaluate(float fraction) const;
    };

    // -------------------------------------------------------------------
    // Metadata Nodes
    // -------------------------------------------------------------------

    template<typename T>
    class MetadataTNode :
        public X3DChildNode
    {
    public:
        std::string m_name;
        std::string m_reference;
        T m_value{};

        const char* GetNodeTypeName() const override { return "MetadataT"; }
    };

    class MetadataStringNode :
        public X3DChildNode
    {
    public:
        std::string m_name;
        std::string m_reference;
        std::string m_value;
        MFString m_values;

        const char* GetNodeTypeName() const override { return "MetadataString"; }
    };

    class MetadataDoubleNode :
        public X3DChildNode
    {
    public:
        std::string m_name;
        std::string m_reference;
        double m_value = 0.0;
        MFDouble m_values;

        const char* GetNodeTypeName() const override { return "MetadataDouble"; }
    };

    class MetadataFloatNode :
        public X3DChildNode
    {
    public:
        std::string m_name;
        std::string m_reference;
        float m_value = 0.0f;
        MFFloat m_values;

        const char* GetNodeTypeName() const override { return "MetadataFloat"; }
    };

    class MetadataIntegerNode :
        public X3DChildNode
    {
    public:
        std::string m_name;
        std::string m_reference;
        int m_value = 0;
        MFInt32 m_values;

        const char* GetNodeTypeName() const override { return "MetadataInteger"; }
    };

    class MetadataBoolNode :
        public X3DChildNode
    {
    public:
        std::string m_name;
        std::string m_reference;
        bool m_value = false;
        MFBool m_values;

        const char* GetNodeTypeName() const override { return "MetadataBool"; }
    };

    class MetadataSetNode :
        public X3DChildNode
    {
    public:
        std::string m_name;
        std::string m_reference;
        std::vector<SAIPtr<X3DChildNode>> m_metadata;

        const char* GetNodeTypeName() const override { return "MetadataSet"; }
    };

    // -------------------------------------------------------------------
    // Event / Route
    // -------------------------------------------------------------------

    class SAIRoute
    {
    public:
        X3DNode* m_sourceNode = nullptr;
        std::string m_sourceField;
        X3DNode* m_destNode = nullptr;
        std::string m_destField;
        bool m_enabled = true;

        void Propagate();
    };

    // -------------------------------------------------------------------
    // TimeDependent base
    // -------------------------------------------------------------------
    class TimeDependentNode :
        public X3DChildNode
    {
    public:
        double m_startTime = 0.0;
        double m_stopTime = 0.0;
        double m_pauseTime = 0.0;
        double m_resumeTime = 0.0;
        bool m_enabled = true;
        bool m_isActive = false;
    };

} // namespace HMREngine
