#pragma once
// X3DNodes.h - All X3D node types for HMREngine

#include "HMREngine.h"
#include "X3DFieldTypes.h"
#include "X3DMath.h"
#include <atlbase.h>
#include <atlcom.h>
#include <map>
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
            if (this != &o) { if (m_p) m_p->Release(); m_p = o.m_p; if (m_p) m_p->AddRef(); }
            return *this;
        }
        SAIPtr& operator=(SAIPtr&& o) noexcept
        {
            if (this != &o) { if (m_p) m_p->Release(); m_p = o.m_p; o.m_p = nullptr; }
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
    struct IX3DNode : public IUnknown
    {
        virtual const char* GetNodeName() const = 0;
        virtual const char* GetNodeTypeName() const = 0;
    };

    // -------------------------------------------------------------------
    // X3DNode - Base class for all scene graph nodes
    // -------------------------------------------------------------------
    class ATL_NO_VTABLE X3DNode :
        public CComObjectRootEx<CComSingleThreadModel>,
        public CComCoClass<X3DNode>,
        public IX3DNode
    {
    public:
        std::string m_name;
        std::map<std::string, CComPtr<X3DFieldNode>> m_fields;

        virtual ~X3DNode() = default;

        DECLARE_REGISTRY_RESOURCEID(IDR_X3DNODE)
        DECLARE_NOT_AGGREGATABLE(X3DNode)

        BEGIN_COM_MAP(X3DNode)
            COM_INTERFACE_ENTRY(IX3DNode)
            COM_INTERFACE_ENTRY(IUnknown)
        END_COM_MAP()

        const char* GetNodeName() const override { return m_name.c_str(); }
        const char* GetNodeTypeName() const override { return "X3DNode"; }

        void SetField(const std::string& name, X3DFieldNode* field)
        {
            CComPtr<X3DFieldNode> f(field);
            m_fields[name] = f;
        }

        X3DFieldNode* GetField(const std::string& name) const
        {
            auto it = m_fields.find(name);
            return it != m_fields.end() ? it->second.m_p : nullptr;
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
    class ATL_NO_VTABLE X3DChildNode :
        public CComObjectRootEx<CComSingleThreadModel>,
        public IX3DNode
    {
    public:
        std::string m_name;
        std::map<std::string, CComPtr<X3DFieldNode>> m_fields;
        std::vector<SAIPtr<X3DChildNode>> m_children;
        X3DChildNode* m_parent = nullptr;
        bool m_visible = true;

        virtual ~X3DChildNode() = default;

        BEGIN_COM_MAP(X3DChildNode)
            COM_INTERFACE_ENTRY(IX3DNode)
            COM_INTERFACE_ENTRY(IUnknown)
        END_COM_MAP()

        const char* GetNodeName() const override { return m_name.c_str(); }
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

        virtual void SetField(const std::string& name, X3DFieldNode* field)
        {
            CComPtr<X3DFieldNode> f(field);
            m_fields[name] = f;
        }

        X3DFieldNode* GetField(const std::string& name) const
        {
            auto it = m_fields.find(name);
            return it != m_fields.end() ? it->second.m_p : nullptr;
        }
    };

    // -------------------------------------------------------------------
    // X3DGroupingNode - Grouping nodes that hold child lists
    // -------------------------------------------------------------------
    class ATL_NO_VTABLE X3DGroupingNode :
        public CComObjectRootEx<CComSingleThreadModel>,
        public X3DChildNode
    {
    public:
        BEGIN_COM_MAP(X3DGroupingNode)
            COM_INTERFACE_ENTRY(IX3DNode)
            COM_INTERFACE_ENTRY(IUnknown)
        END_COM_MAP()

        const char* GetNodeTypeName() const override { return "X3DGroupingNode"; }
    };

    // -------------------------------------------------------------------
    // X3DShapeNode - Nodes that define visual appearance
    // -------------------------------------------------------------------
    class ATL_NO_VTABLE X3DShapeNode :
        public CComObjectRootEx<CComSingleThreadModel>,
        public X3DChildNode
    {
    public:
        BEGIN_COM_MAP(X3DShapeNode)
            COM_INTERFACE_ENTRY(IX3DNode)
            COM_INTERFACE_ENTRY(IUnknown)
        END_COM_MAP()

        const char* GetNodeTypeName() const override { return "X3DShapeNode"; }
    };

    // -------------------------------------------------------------------
    // X3DGeometryNode - Geometry definitions
    // -------------------------------------------------------------------
    class ATL_NO_VTABLE X3DGeometryNode :
        public CComObjectRootEx<CComSingleThreadModel>,
        public X3DChildNode
    {
    public:
        BEGIN_COM_MAP(X3DGeometryNode)
            COM_INTERFACE_ENTRY(IX3DNode)
            COM_INTERFACE_ENTRY(IUnknown)
        END_COM_MAP()

        const char* GetNodeTypeName() const override { return "X3DGeometryNode"; }

        virtual HRESULT BuildVertexBuffer(ID3D11Device* pDev, ID3D11Buffer** ppVB) { return E_NOTIMPL; }
        virtual HRESULT BuildIndexBuffer(ID3D11Device* pDev, ID3D11Buffer** ppIB) { return E_NOTIMPL; }
        virtual UINT GetVertexCount() const { return 0; }
        virtual UINT GetIndexCount() const { return 0; }
    };

    // -------------------------------------------------------------------
    // X3DTextureNode - Texture definitions
    // -------------------------------------------------------------------
    class ATL_NO_VTABLE X3DTextureNode :
        public CComObjectRootEx<CComSingleThreadModel>,
        public X3DChildNode
    {
    public:
        BEGIN_COM_MAP(X3DTextureNode)
            COM_INTERFACE_ENTRY(IX3DNode)
            COM_INTERFACE_ENTRY(IUnknown)
        END_COM_MAP()

        const char* GetNodeTypeName() const override { return "X3DTextureNode"; }

        virtual HRESULT GetTexture(ID3D11Device* pDev, ID3D11ShaderResourceView** ppSRV) { return E_NOTIMPL; }
        virtual void Update(double time) {}
    };

    // -------------------------------------------------------------------
    // X3DMaterialNode - Material definitions
    // -------------------------------------------------------------------
    class ATL_NO_VTABLE X3DMaterialNode :
        public CComObjectRootEx<CComSingleThreadModel>,
        public X3DChildNode
    {
    public:
        BEGIN_COM_MAP(X3DMaterialNode)
            COM_INTERFACE_ENTRY(IX3DNode)
            COM_INTERFACE_ENTRY(IUnknown)
        END_COM_MAP()

        const char* GetNodeTypeName() const override { return "X3DMaterialNode"; }
    };

    // -------------------------------------------------------------------
    // Concrete X3D Node Implementations
    // -------------------------------------------------------------------

    // -- Group --
    class ATL_NO_VTABLE GroupNode :
        public CComObjectRootEx<CComSingleThreadModel>,
        public X3DGroupingNode
    {
    public:
        DECLARE_REGISTRY_RESOURCEID(IDR_GROUPNODE)
        DECLARE_NOT_AGGREGATABLE(GroupNode)
        BEGIN_COM_MAP(GroupNode)
            COM_INTERFACE_ENTRY(IX3DNode)
            COM_INTERFACE_ENTRY(IUnknown)
        END_COM_MAP()
        const char* GetNodeTypeName() const override { return "Group"; }
    };

    // -- Transform --
    class ATL_NO_VTABLE TransformNode :
        public CComObjectRootEx<CComSingleThreadModel>,
        public X3DGroupingNode
    {
    public:
        Vec3 m_translation;
        Rotation4f m_rotation;
        Vec3 m_scale = Vec3(1,1,1);
        Vec3 m_scaleOrientation;
        Vec3 m_center;
        Matrix4f m_matrix;

        DECLARE_REGISTRY_RESOURCEID(IDR_TRANSFORMNODE)
        DECLARE_NOT_AGGREGATABLE(TransformNode)
        BEGIN_COM_MAP(TransformNode)
            COM_INTERFACE_ENTRY(IX3DNode)
            COM_INTERFACE_ENTRY(IUnknown)
        END_COM_MAP()

        const char* GetNodeTypeName() const override { return "Transform"; }

        void UpdateMatrix()
        {
            m_matrix = Matrix4f::FromTranslationRotationScale(m_translation, m_rotation, m_scale);
        }
    };

    // -- Switch --
    class ATL_NO_VTABLE SwitchNode :
        public CComObjectRootEx<CComSingleThreadModel>,
        public X3DGroupingNode
    {
    public:
        int m_choice = 0;
        bool m_enabled = true;

        DECLARE_REGISTRY_RESOURCEID(IDR_SWITCHNODE)
        DECLARE_NOT_AGGREGATABLE(SwitchNode)
        BEGIN_COM_MAP(SwitchNode)
            COM_INTERFACE_ENTRY(IX3DNode)
            COM_INTERFACE_ENTRY(IUnknown)
        END_COM_MAP()

        const char* GetNodeTypeName() const override { return "Switch"; }
    };

    // -- Shape --
    class ATL_NO_VTABLE ShapeNode :
        public CComObjectRootEx<CComSingleThreadModel>,
        public X3DShapeNode
    {
    public:
        SAIPtr<X3DGeometryNode> m_geometry;
        SAIPtr<X3DChildNode> m_appearance;
        Matrix4f m_worldMatrix;

        DECLARE_REGISTRY_RESOURCEID(IDR_SHAPENODE)
        DECLARE_NOT_AGGREGATABLE(ShapeNode)
        BEGIN_COM_MAP(ShapeNode)
            COM_INTERFACE_ENTRY(IX3DNode)
            COM_INTERFACE_ENTRY(IUnknown)
        END_COM_MAP()

        const char* GetNodeTypeName() const override { return "Shape"; }
    };

    // -- Appearance --
    class ATL_NO_VTABLE AppearanceNode :
        public CComObjectRootEx<CComSingleThreadModel>,
        public X3DChildNode
    {
    public:
        SAIPtr<X3DTextureNode> m_texture;
        SAIPtr<X3DMaterialNode> m_material;
        CComPtr<X3DFieldNode> m_shaders;

        DECLARE_REGISTRY_RESOURCEID(IDR_APPEARANCENODE)
        DECLARE_NOT_AGGREGATABLE(AppearanceNode)
        BEGIN_COM_MAP(AppearanceNode)
            COM_INTERFACE_ENTRY(IX3DNode)
            COM_INTERFACE_ENTRY(IUnknown)
        END_COM_MAP()

        const char* GetNodeTypeName() const override { return "Appearance"; }
    };

    // -- ImageTexture --
    class ATL_NO_VTABLE ImageTextureNode :
        public CComObjectRootEx<CComSingleThreadModel>,
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

        DECLARE_REGISTRY_RESOURCEID(IDR_IMAGETEXTURENODE)
        DECLARE_NOT_AGGREGATABLE(ImageTextureNode)
        BEGIN_COM_MAP(ImageTextureNode)
            COM_INTERFACE_ENTRY(IX3DNode)
            COM_INTERFACE_ENTRY(IUnknown)
        END_COM_MAP()

        const char* GetNodeTypeName() const override { return "ImageTexture"; }
        HRESULT GetTexture(ID3D11Device* pDev, ID3D11ShaderResourceView** ppSRV) override;
    };

    // -- MovieTexture --
    class ATL_NO_VTABLE MovieTextureNode :
        public CComObjectRootEx<CComSingleThreadModel>,
        public X3DTextureNode
    {
    public:
        std::string m_url;
        bool m_loop = false;
        double m_speed = 1.0;
        bool m_play = false;
        double m_fraction = 0.0;

        DECLARE_REGISTRY_RESOURCEID(IDR_MOVIETEXTURENODE)
        DECLARE_NOT_AGGREGATABLE(MovieTextureNode)
        BEGIN_COM_MAP(MovieTextureNode)
            COM_INTERFACE_ENTRY(IX3DNode)
            COM_INTERFACE_ENTRY(IUnknown)
        END_COM_MAP()

        const char* GetNodeTypeName() const override { return "MovieTexture"; }
        void Update(double time) override;
    };

    // -- MotionTexture (custom) --
    class ATL_NO_VTABLE MotionTextureNode :
        public CComObjectRootEx<CComSingleThreadModel>,
        public X3DTextureNode
    {
    public:
        std::string m_url;
        float m_offset = 0.0f;
        float m_scale = 1.0f;

        DECLARE_REGISTRY_RESOURCEID(IDR_MOTIONTEXTURENODE)
        DECLARE_NOT_AGGREGATABLE(MotionTextureNode)
        BEGIN_COM_MAP(MotionTextureNode)
            COM_INTERFACE_ENTRY(IX3DNode)
            COM_INTERFACE_ENTRY(IUnknown)
        END_COM_MAP()

        const char* GetNodeTypeName() const override { return "MotionTexture"; }
    };

    // -- TextureTransform --
    class ATL_NO_VTABLE TextureTransformNode :
        public CComObjectRootEx<CComSingleThreadModel>,
        public X3DChildNode
    {
    public:
        Vec2 m_translation;
        float m_rotation = 0.0f;
        Vec2 m_scale = Vec2(1, 1);
        Matrix4f m_matrix;

        DECLARE_REGISTRY_RESOURCEID(IDR_TEXTURETRANSFORMNODE)
        DECLARE_NOT_AGGREGATABLE(TextureTransformNode)
        BEGIN_COM_MAP(TextureTransformNode)
            COM_INTERFACE_ENTRY(IX3DNode)
            COM_INTERFACE_ENTRY(IUnknown)
        END_COM_MAP()

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
    class ATL_NO_VTABLE TexturePropertiesNode :
        public CComObjectRootEx<CComSingleThreadModel>,
        public X3DChildNode
    {
    public:
        bool m_generateMipMaps = true;
        std::string m_magFilter = "LINEAR";
        std::string m_minFilter = "LINEAR";

        DECLARE_REGISTRY_RESOURCEID(IDR_TEXTUREPROPERTIESNODE)
        DECLARE_NOT_AGGREGATABLE(TexturePropertiesNode)
        BEGIN_COM_MAP(TexturePropertiesNode)
            COM_INTERFACE_ENTRY(IX3DNode)
            COM_INTERFACE_ENTRY(IUnknown)
        END_COM_MAP()

        const char* GetNodeTypeName() const override { return "TextureProperties"; }
    };

    // -- Material --
    class ATL_NO_VTABLE MaterialNode :
        public CComObjectRootEx<CComSingleThreadModel>,
        public X3DMaterialNode
    {
    public:
        Rgb m_diffuseColor = Rgb(0.8f, 0.8f, 0.8f);
        Rgb m_specularColor = Rgb(0.0f, 0.0f, 0.0f);
        Rgb m_emissiveColor = Rgb(0.0f, 0.0f, 0.0f);
        Rgb m_ambientColor = Rgb(0.2f, 0.2f, 0.2f);
        float m_shininess = 0.2f;
        float m_transparency = 0.0f;

        DECLARE_REGISTRY_RESOURCEID(IDR_MATERIALNODE)
        DECLARE_NOT_AGGREGATABLE(MaterialNode)
        BEGIN_COM_MAP(MaterialNode)
            COM_INTERFACE_ENTRY(IX3DNode)
            COM_INTERFACE_ENTRY(IUnknown)
        END_COM_MAP()

        const char* GetNodeTypeName() const override { return "Material"; }
    };

    // -- LineProperties --
    class ATL_NO_VTABLE LinePropertiesNode :
        public CComObjectRootEx<CComSingleThreadModel>,
        public X3DChildNode
    {
    public:
        bool m_applied = true;
        int m_linetype = 1;
        float m_linewidthScaleFactor = 1.0f;

        DECLARE_REGISTRY_RESOURCEID(IDR_LINEPROPERTIESNODE)
        DECLARE_NOT_AGGREGATABLE(LinePropertiesNode)
        BEGIN_COM_MAP(LinePropertiesNode)
            COM_INTERFACE_ENTRY(IX3DNode)
            COM_INTERFACE_ENTRY(IUnknown)
        END_COM_MAP()

        const char* GetNodeTypeName() const override { return "LineProperties"; }
    };

    // -- FillProperties --
    class ATL_NO_VTABLE FillPropertiesNode :
        public CComObjectRootEx<CComSingleThreadModel>,
        public X3DChildNode
    {
    public:
        bool m_applied = true;
        bool m_filled = true;

        DECLARE_REGISTRY_RESOURCEID(IDR_FILLPROPERTIESNODE)
        DECLARE_NOT_AGGREGATABLE(FillPropertiesNode)
        BEGIN_COM_MAP(FillPropertiesNode)
            COM_INTERFACE_ENTRY(IX3DNode)
            COM_INTERFACE_ENTRY(IUnknown)
        END_COM_MAP()

        const char* GetNodeTypeName() const override { return "FillProperties"; }
    };

    // -- TimeSensor --
    class ATL_NO_VTABLE TimeSensorNode :
        public CComObjectRootEx<CComSingleThreadModel>,
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
        double m.currentTime = 0.0;
        bool m_isActive = false;

        std::function<void(double)> m_onFractionChanged;
        std::function<void(bool)> m_onActiveChanged;

        DECLARE_REGISTRY_RESOURCEID(IDR_TIMESENSORNODE)
        DECLARE_NOT_AGGREGATABLE(TimeSensorNode)
        BEGIN_COM_MAP(TimeSensorNode)
            COM_INTERFACE_ENTRY(IX3DNode)
            COM_INTERFACE_ENTRY(IUnknown)
        END_COM_MAP()

        const char* GetNodeTypeName() const override { return "TimeSensor"; }

        void Evaluate(double globalTime);
    };

    // -- TimeTrigger --
    class ATL_NO_VTABLE TimeTriggerNode :
        public CComObjectRootEx<CComSingleThreadModel>,
        public X3DChildNode
    {
    public:
        bool m_enabled = true;
        double m_triggerTime = 0.0;

        DECLARE_REGISTRY_RESOURCEID(IDR_TIMETRIGGERNODE)
        DECLARE_NOT_AGGREGATABLE(TimeTriggerNode)
        BEGIN_COM_MAP(TimeTriggerNode)
            COM_INTERFACE_ENTRY(IX3DNode)
            COM_INTERFACE_ENTRY(IUnknown)
        END_COM_MAP()

        const char* GetNodeTypeName() const override { return "TimeTrigger"; }
    };

    // -- Coordinate --
    class ATL_NO_VTABLE CoordinateNode :
        public CComObjectRootEx<CComSingleThreadModel>,
        public X3DChildNode
    {
    public:
        std::vector<Vec3> m_point;

        DECLARE_REGISTRY_RESOURCEID(IDR_COORDINATENODE)
        DECLARE_NOT_AGGREGATABLE(CoordinateNode)
        BEGIN_COM_MAP(CoordinateNode)
            COM_INTERFACE_ENTRY(IX3DNode)
            COM_INTERFACE_ENTRY(IUnknown)
        END_COM_MAP()

        const char* GetNodeTypeName() const override { return "Coordinate"; }
    };

    // -- Normal --
    class ATL_NO_VTABLE NormalNode :
        public CComObjectRootEx<CComSingleThreadModel>,
        public X3DChildNode
    {
    public:
        std::vector<Vec3> m_vector;

        DECLARE_REGISTRY_RESOURCEID(IDR_NORMALNODE)
        DECLARE_NOT_AGGREGATABLE(NormalNode)
        BEGIN_COM_MAP(NormalNode)
            COM_INTERFACE_ENTRY(IX3DNode)
            COM_INTERFACE_ENTRY(IUnknown)
        END_COM_MAP()

        const char* GetNodeTypeName() const override { return "Normal"; }
    };

    // -- Color --
    class ATL_NO_VTABLE ColorNode :
        public CComObjectRootEx<CComSingleThreadModel>,
        public X3DChildNode
    {
    public:
        std::vector<Rgb> m_color;

        DECLARE_REGISTRY_RESOURCEID(IDR_COLORNODE)
        DECLARE_NOT_AGGREGATABLE(ColorNode)
        BEGIN_COM_MAP(ColorNode)
            COM_INTERFACE_ENTRY(IX3DNode)
            COM_INTERFACE_ENTRY(IUnknown)
        END_COM_MAP()

        const char* GetNodeTypeName() const override { return "Color"; }
    };

    // -- ColorRGBA --
    class ATL_NO_VTABLE ColorRGBANode :
        public CComObjectRootEx<CComSingleThreadModel>,
        public X3DChildNode
    {
    public:
        std::vector<Rgba> m_color;

        DECLARE_REGISTRY_RESOURCEID(IDR_COLORRGBANODE)
        DECLARE_NOT_AGGREGATABLE(ColorRGBANode)
        BEGIN_COM_MAP(ColorRGBANode)
            COM_INTERFACE_ENTRY(IX3DNode)
            COM_INTERFACE_ENTRY(IUnknown)
        END_COM_MAP()

        const char* GetNodeTypeName() const override { return "ColorRGBA"; }
    };

    // -- TextureCoordinate --
    class ATL_NO_VTABLE TextureCoordinateNode :
        public CComObjectRootEx<CComSingleThreadModel>,
        public X3DChildNode
    {
    public:
        std::vector<Vec2> m_point;

        DECLARE_REGISTRY_RESOURCEID(IDR_TEXTURECOORDINATENODE)
        DECLARE_NOT_AGGREGATABLE(TextureCoordinateNode)
        BEGIN_COM_MAP(TextureCoordinateNode)
            COM_INTERFACE_ENTRY(IX3DNode)
            COM_INTERFACE_ENTRY(IUnknown)
        END_COM_MAP()

        const char* GetNodeTypeName() const override { return "TextureCoordinate"; }
    };

    // -- IndexedFaceSet --
    class ATL_NO_VTABLE IndexedFaceSetNode :
        public CComObjectRootEx<CComSingleThreadModel>,
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

        DECLARE_REGISTRY_RESOURCEID(IDR_INDEXEDFACESETNODE)
        DECLARE_NOT_AGGREGATABLE(IndexedFaceSetNode)
        BEGIN_COM_MAP(IndexedFaceSetNode)
            COM_INTERFACE_ENTRY(IX3DNode)
            COM_INTERFACE_ENTRY(IUnknown)
        END_COM_MAP()

        const char* GetNodeTypeName() const override { return "IndexedFaceSet"; }

        HRESULT BuildVertexBuffer(ID3D11Device* pDev, ID3D11Buffer** ppVB) override;
        HRESULT BuildIndexBuffer(ID3D11Device* pDev, ID3D11Buffer** ppIB) override;
        UINT GetVertexCount() const override { return m_vertexCount; }
        UINT GetIndexCount() const override { return m_indexCount; }
    };

    // -- FontStyle --
    class ATL_NO_VTABLE FontStyleNode :
        public CComObjectRootEx<CComSingleThreadModel>,
        public X3DChildNode
    {
    public:
        std::string m_family = "SANS";
        std::string m_style = "PLAIN";
        float m_size = 1.0f;
        bool m_bold = false;
        bool m_italic = false;
        bool m_language = false;

        DECLARE_REGISTRY_RESOURCEID(IDR_FONTSTYLENODE)
        DECLARE_NOT_AGGREGATABLE(FontStyleNode)
        BEGIN_COM_MAP(FontStyleNode)
            COM_INTERFACE_ENTRY(IX3DNode)
            COM_INTERFACE_ENTRY(IUnknown)
        END_COM_MAP()

        const char* GetNodeTypeName() const override { return "FontStyle"; }
    };

    // -- Text --
    class ATL_NO_VTABLE TextNode :
        public CComObjectRootEx<CComSingleThreadModel>,
        public X3DGeometryNode
    {
    public:
        MFString m_string;
        MFVec2f m_length;
        MFVec2f m_maxExtent;
        SAIPtr<FontStyleNode> m_fontStyle;

        DECLARE_REGISTRY_RESOURCEID(IDR_TEXTNODE)
        DECLARE_NOT_AGGREGATABLE(TextNode)
        BEGIN_COM_MAP(TextNode)
            COM_INTERFACE_ENTRY(IX3DNode)
            COM_INTERFACE_ENTRY(IUnknown)
        END_COM_MAP()

        const char* GetNodeTypeName() const override { return "Text"; }
    };

    // -- Grid (custom node) --
    class ATL_NO_VTABLE GridNode :
        public CComObjectRootEx<CComSingleThreadModel>,
        public X3DGeometryNode
    {
    public:
        float m_width = 1.0f;
        float m_height = 1.0f;
        int m_divisions = 4;
        Rgb m_lineColor = Rgb(0.5f, 0.5f, 0.5f);
        float m_lineWidth = 1.0f;
        bool m_showOrigin = true;

        DECLARE_REGISTRY_RESOURCEID(IDR_GRIDNODE)
        DECLARE_NOT_AGGREGATABLE(GridNode)
        BEGIN_COM_MAP(GridNode)
            COM_INTERFACE_ENTRY(IX3DNode)
            COM_INTERFACE_ENTRY(IUnknown)
        END_COM_MAP()

        const char* GetNodeTypeName() const override { return "Grid"; }
    };

    // -- Viewport --
    class ATL_NO_VTABLE ViewportNode :
        public CComObjectRootEx<CComSingleThreadModel>,
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

        DECLARE_REGISTRY_RESOURCEID(IDR_VIEWPORTNODE)
        DECLARE_NOT_AGGREGATABLE(ViewportNode)
        BEGIN_COM_MAP(ViewportNode)
            COM_INTERFACE_ENTRY(IX3DNode)
            COM_INTERFACE_ENTRY(IUnknown)
        END_COM_MAP()

        const char* GetNodeTypeName() const override { return "Viewport"; }
    };

    // -- Layer --
    class ATL_NO_VTABLE LayerNode :
        public CComObjectRootEx<CComSingleThreadModel>,
        public X3DGroupingNode
    {
    public:
        bool m_pickable = true;
        bool m_visible = true;

        DECLARE_REGISTRY_RESOURCEID(IDR_LAYERNODE)
        DECLARE_NOT_AGGREGATABLE(LayerNode)
        BEGIN_COM_MAP(LayerNode)
            COM_INTERFACE_ENTRY(IX3DNode)
            COM_INTERFACE_ENTRY(IUnknown)
        END_COM_MAP()

        const char* GetNodeTypeName() const override { return "Layer"; }
    };

    // -- LayerSet --
    class ATL_NO_VTABLE LayerSetNode :
        public CComObjectRootEx<CComSingleThreadModel>,
        public X3DChildNode
    {
    public:
        SAIPtr<LayerNode> m_activeLayer;
        int m_ordering = 0; // 0=TRAV_ALL, 1=TRAV_LOCAL

        DECLARE_REGISTRY_RESOURCEID(IDR_LAYERSETNODE)
        DECLARE_NOT_AGGREGATABLE(LayerSetNode)
        BEGIN_COM_MAP(LayerSetNode)
            COM_INTERFACE_ENTRY(IX3DNode)
            COM_INTERFACE_ENTRY(IUnknown)
        END_COM_MAP()

        const char* GetNodeTypeName() const override { return "LayerSet"; }
    };

    // -------------------------------------------------------------------
    // Interpolator Nodes
    // -------------------------------------------------------------------

    class ATL_NO_VTABLE PositionInterpolatorNode :
        public CComObjectRootEx<CComSingleThreadModel>,
        public X3DChildNode
    {
    public:
        MFFloat m_key;
        MFVec3f m_value;
        bool m_enabled = true;

        DECLARE_REGISTRY_RESOURCEID(IDR_POSITIONINTERPOLATORNODE)
        DECLARE_NOT_AGGREGATABLE(PositionInterpolatorNode)
        BEGIN_COM_MAP(PositionInterpolatorNode)
            COM_INTERFACE_ENTRY(IX3DNode)
            COM_INTERFACE_ENTRY(IUnknown)
        END_COM_MAP()

        const char* GetNodeTypeName() const override { return "PositionInterpolator"; }
        Vec3 Interpolate(float fraction) const;
    };

    class ATL_NO_VTABLE OrientationInterpolatorNode :
        public CComObjectRootEx<CComSingleThreadModel>,
        public X3DChildNode
    {
    public:
        MFFloat m_key;
        MFRotation m_value;
        bool m_enabled = true;

        DECLARE_REGISTRY_RESOURCEID(IDR_ORIENTATIONINTERPOLATORNODE)
        DECLARE_NOT_AGGREGATABLE(OrientationInterpolatorNode)
        BEGIN_COM_MAP(OrientationInterpolatorNode)
            COM_INTERFACE_ENTRY(IX3DNode)
            COM_INTERFACE_ENTRY(IUnknown)
        END_COM_MAP()

        const char* GetNodeTypeName() const override { return "OrientationInterpolator"; }
        Rotation4f Interpolate(float fraction) const;
    };

    class ATL_NO_VTABLE ScalarInterpolatorNode :
        public CComObjectRootEx<CComSingleThreadModel>,
        public X3DChildNode
    {
    public:
        MFFloat m_key;
        MFFloat m_value;
        bool m_enabled = true;

        DECLARE_REGISTRY_RESOURCEID(IDR_SCALARINTERPOLATORNODE)
        DECLARE_NOT_AGGREGATABLE(ScalarInterpolatorNode)
        BEGIN_COM_MAP(ScalarInterpolatorNode)
            COM_INTERFACE_ENTRY(IX3DNode)
            COM_INTERFACE_ENTRY(IUnknown)
        END_COM_MAP()

        const char* GetNodeTypeName() const override { return "ScalarInterpolator"; }
        float Interpolate(float fraction) const;
    };

    class ATL_NO_VTABLE ColorInterpolatorNode :
        public CComObjectRootEx<CComSingleThreadModel>,
        public X3DChildNode
    {
    public:
        MFFloat m_key;
        MFColor m_value;
        bool m_enabled = true;

        DECLARE_REGISTRY_RESOURCEID(IDR_COLORINTERPOLATORNODE)
        DECLARE_NOT_AGGREGATABLE(ColorInterpolatorNode)
        BEGIN_COM_MAP(ColorInterpolatorNode)
            COM_INTERFACE_ENTRY(IX3DNode)
            COM_INTERFACE_ENTRY(IUnknown)
        END_COM_MAP()

        const char* GetNodeTypeName() const override { return "ColorInterpolator"; }
        Rgb Interpolate(float fraction) const;
    };

    class ATL_NO_VTABLE CoordinateInterpolatorNode :
        public CComObjectRootEx<CComSingleThreadModel>,
        public X3DChildNode
    {
    public:
        MFFloat m_key;
        MFVec3f m_value;
        bool m_enabled = true;

        DECLARE_REGISTRY_RESOURCEID(IDR_COORDINATEINTERPOLATORNODE)
        DECLARE_NOT_AGGREGATABLE(CoordinateInterpolatorNode)
        BEGIN_COM_MAP(CoordinateInterpolatorNode)
            COM_INTERFACE_ENTRY(IX3DNode)
            COM_INTERFACE_ENTRY(IUnknown)
        END_COM_MAP()

        const char* GetNodeTypeName() const override { return "CoordinateInterpolator"; }
    };

    // -------------------------------------------------------------------
    // Sequencer Nodes
    // -------------------------------------------------------------------

    class ATL_NO_VTABLE IntegerSequencerNode :
        public CComObjectRootEx<CComSingleThreadModel>,
        public X3DChildNode
    {
    public:
        MFFloat m_key;
        MFInt32 m_value;
        bool m_enabled = true;

        DECLARE_REGISTRY_RESOURCEID(IDR_INTEGERSEQUENCERNODE)
        DECLARE_NOT_AGGREGATABLE(IntegerSequencerNode)
        BEGIN_COM_MAP(IntegerSequencerNode)
            COM_INTERFACE_ENTRY(IX3DNode)
            COM_INTERFACE_ENTRY(IUnknown)
        END_COM_MAP()

        const char* GetNodeTypeName() const override { return "IntegerSequencer"; }
        int Evaluate(float fraction) const;
    };

    class ATL_NO_VTABLE BooleanSequencerNode :
        public CComObjectRootEx<CComSingleThreadModel>,
        public X3DChildNode
    {
    public:
        MFFloat m_key;
        MFBool m_value;
        bool m_enabled = true;

        DECLARE_REGISTRY_RESOURCEID(IDR_BOOLEANSEQUENCERNODE)
        DECLARE_NOT_AGGREGATABLE(BooleanSequencerNode)
        BEGIN_COM_MAP(BooleanSequencerNode)
            COM_INTERFACE_ENTRY(IX3DNode)
            COM_INTERFACE_ENTRY(IUnknown)
        END_COM_MAP()

        const char* GetNodeTypeName() const override { return "BooleanSequencer"; }
        bool Evaluate(float fraction) const;
    };

    // -------------------------------------------------------------------
    // Metadata Nodes
    // -------------------------------------------------------------------

    template<typename T>
    class ATL_NO_VTABLE MetadataTNode :
        public CComObjectRootEx<CComSingleThreadModel>,
        public X3DChildNode
    {
    public:
        std::string m_name;
        std::string m_reference;
        T m_value{};

        const char* GetNodeTypeName() const override { return "MetadataT"; }
    };

    class ATL_NO_VTABLE MetadataStringNode :
        public CComObjectRootEx<CComSingleThreadModel>,
        public X3DChildNode
    {
    public:
        std::string m_name;
        std::string m_reference;
        std::string m_value;
        MFString m_values;

        DECLARE_REGISTRY_RESOURCEID(IDR_METADATASTRINGNODE)
        DECLARE_NOT_AGGREGATABLE(MetadataStringNode)
        BEGIN_COM_MAP(MetadataStringNode)
            COM_INTERFACE_ENTRY(IX3DNode)
            COM_INTERFACE_ENTRY(IUnknown)
        END_COM_MAP()

        const char* GetNodeTypeName() const override { return "MetadataString"; }
    };

    class ATL_NO_VTABLE MetadataDoubleNode :
        public CComObjectRootEx<CComSingleThreadModel>,
        public X3DChildNode
    {
    public:
        std::string m_name;
        std::string m_reference;
        double m_value = 0.0;
        MFDouble m_values;

        DECLARE_REGISTRY_RESOURCEID(IDR_METADATADOUBLENODE)
        DECLARE_NOT_AGGREGATABLE(MetadataDoubleNode)
        BEGIN_COM_MAP(MetadataDoubleNode)
            COM_INTERFACE_ENTRY(IX3DNode)
            COM_INTERFACE_ENTRY(IUnknown)
        END_COM_MAP()

        const char* GetNodeTypeName() const override { return "MetadataDouble"; }
    };

    class ATL_NO_VTABLE MetadataFloatNode :
        public CComObjectRootEx<CComSingleThreadModel>,
        public X3DChildNode
    {
    public:
        std::string m_name;
        std::string m_reference;
        float m_value = 0.0f;
        MFFloat m_values;

        DECLARE_REGISTRY_RESOURCEID(IDR_METADATAFLOATNODE)
        DECLARE_NOT_AGGREGATABLE(MetadataFloatNode)
        BEGIN_COM_MAP(MetadataFloatNode)
            COM_INTERFACE_ENTRY(IX3DNode)
            COM_INTERFACE_ENTRY(IUnknown)
        END_COM_MAP()

        const char* GetNodeTypeName() const override { return "MetadataFloat"; }
    };

    class ATL_NO_VTABLE MetadataIntegerNode :
        public CComObjectRootEx<CComSingleThreadModel>,
        public X3DChildNode
    {
    public:
        std::string m_name;
        std::string m_reference;
        int m_value = 0;
        MFInt32 m_values;

        DECLARE_REGISTRY_RESOURCEID(IDR_METADATAINTEGERNODE)
        DECLARE_NOT_AGGREGATABLE(MetadataIntegerNode)
        BEGIN_COM_MAP(MetadataIntegerNode)
            COM_INTERFACE_ENTRY(IX3DNode)
            COM_INTERFACE_ENTRY(IUnknown)
        END_COM_MAP()

        const char* GetNodeTypeName() const override { return "MetadataInteger"; }
    };

    class ATL_NO_VTABLE MetadataBoolNode :
        public CComObjectRootEx<CComSingleThreadModel>,
        public X3DChildNode
    {
    public:
        std::string m_name;
        std::string m_reference;
        bool m_value = false;
        MFBool m_values;

        DECLARE_REGISTRY_RESOURCEID(IDR_METADATABOOLNODE)
        DECLARE_NOT_AGGREGATABLE(MetadataBoolNode)
        BEGIN_COM_MAP(MetadataBoolNode)
            COM_INTERFACE_ENTRY(IX3DNode)
            COM_INTERFACE_ENTRY(IUnknown)
        END_COM_MAP()

        const char* GetNodeTypeName() const override { return "MetadataBool"; }
    };

    class ATL_NO_VTABLE MetadataSetNode :
        public CComObjectRootEx<CComSingleThreadModel>,
        public X3DChildNode
    {
    public:
        std::string m_name;
        std::string m_reference;
        std::vector<SAIPtr<X3DChildNode>> m_metadata;

        DECLARE_REGISTRY_RESOURCEID(IDR_METADATASETNODE)
        DECLARE_NOT_AGGREGATABLE(MetadataSetNode)
        BEGIN_COM_MAP(MetadataSetNode)
            COM_INTERFACE_ENTRY(IX3DNode)
            COM_INTERFACE_ENTRY(IUnknown)
        END_COM_MAP()

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
