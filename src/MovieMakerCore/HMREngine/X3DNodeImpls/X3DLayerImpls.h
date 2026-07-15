#pragma once
// X3DLayerImpls.h - Layer, viewport, bindable, text, vertex attribute, metadata, URL bridges

#include "X3DGroupingImpls.h"
#include "../X3DNodes.h"

namespace HMREngine
{
    // --- X3DLayerNodeImpl: Layer bridge ---
    class X3DLayerNodeImpl : public X3DGroupingNodeImpl
    {
    public:
        X3DLayerNodeImpl();
        virtual ~X3DLayerNodeImpl();

        HRESULT Initialize(LayerNode* node);
        void Shutdown() override;

        LayerNode* GetLayerNode() const { return m_layerNode; }

        bool IsPickable() const { return m_pickable; }
        void SetPickable(bool pickable) { m_pickable = pickable; }

        bool IsLayerVisible() const { return m_visible; }
        void SetLayerVisible(bool visible) { m_visible = visible; }

    protected:
        LayerNode* m_layerNode = nullptr;
        bool m_pickable = true;
        bool m_visible = true;

        HRESULT SetupFields() override;
    };

    // --- X3DViewportNodeImpl: Viewport bridge ---
    class X3DViewportNodeImpl : public X3DChildNodeImpl
    {
    public:
        X3DViewportNodeImpl();
        virtual ~X3DViewportNodeImpl();

        HRESULT Initialize(X3DChildNode* node);
        void Shutdown() override;

        float GetViewportX() const { return m_viewport[0]; }
        float GetViewportY() const { return m_viewport[1]; }
        float GetViewportW() const { return m_viewport[2]; }
        float GetViewportH() const { return m_viewport[3]; }

        void SetViewport(float x, float y, float w, float h);
        void Apply(ID3D11DeviceContext* ctx, UINT rtWidth, UINT rtHeight);

    protected:
        float m_viewport[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
        Matrix4f m_view;
        Matrix4f m_projection;

        HRESULT SetupFields() override;
    };

    // --- X3DBindableNodeImpl: Bindable node bridge ---
    class X3DBindableNodeImpl : public X3DNodeImpl
    {
    public:
        X3DBindableNodeImpl();
        virtual ~X3DBindableNodeImpl();

        HRESULT Initialize(X3DNode* node);
        void Shutdown() override;

        bool IsBound() const { return m_bound; }
        void SetBound(bool bound);

        bool SetBind() const { return m_setBind; }

        using BindCallback = std::function<void(bool)>;
        void SetBindCallback(BindCallback cb) { m_bindCallback = cb; }

    protected:
        bool m_bound = false;
        bool m_setBind = false;
        BindCallback m_bindCallback;
    };

    // --- X3DTextNodeImpl: Text node bridge ---
    class X3DTextNodeImpl : public X3DGeometryNodeImpl
    {
    public:
        X3DTextNodeImpl();
        virtual ~X3DTextNodeImpl();

        HRESULT Initialize(X3DGeometryNode* node);
        void Shutdown() override;

        const std::vector<std::string>& GetStrings() const { return m_strings; }
        void SetStrings(const std::vector<std::string>& strings) { m_strings = strings; MarkDirty(); }

        float GetFontSize() const { return m_fontSize; }
        void SetFontSize(float size) { m_fontSize = size; MarkDirty(); }

        const std::string& GetFontFamily() const { return m_fontFamily; }
        void SetFontFamily(const std::string& family) { m_fontFamily = family; MarkDirty(); }

    protected:
        std::vector<std::string> m_strings;
        float m_fontSize = 1.0f;
        std::string m_fontFamily = "SANS";

        HRESULT SetupFields() override;
    };

    // --- X3DVertexAttributeNodeImpl: Vertex attribute bridge ---
    class X3DVertexAttributeNodeImpl : public X3DNodeImpl
    {
    public:
        X3DVertexAttributeNodeImpl();
        virtual ~X3DVertexAttributeNodeImpl();

        HRESULT Initialize(X3DNode* node);
        void Shutdown() override;

        const std::string& GetName() const { return m_attribName; }
        void SetName(const std::string& name) { m_attribName = name; }

        UINT GetNumCoordinates() const { return m_numCoordinates; }
        void SetNumCoordinates(UINT n) { m_numCoordinates = n; }

    protected:
        std::string m_attribName;
        UINT m_numCoordinates = 4;
    };

    // --- X3DMetadataObjectImpl: Metadata bridge ---
    class X3DMetadataObjectImpl : public X3DNodeImpl
    {
    public:
        X3DMetadataObjectImpl();
        virtual ~X3DMetadataObjectImpl();

        HRESULT Initialize(X3DNode* node);
        void Shutdown() override;

        const std::string& GetName() const { return m_metadataName; }
        const std::string& GetReference() const { return m_reference; }

        void SetName(const std::string& name) { m_metadataName = name; }
        void SetReference(const std::string& ref) { m_reference = ref; }

        HRESULT SetValueString(const std::string& value);
        HRESULT SetValueDouble(double value);
        HRESULT SetValueFloat(float value);
        HRESULT SetValueInt(int value);
        HRESULT SetValueBool(bool value);

    protected:
        std::string m_metadataName;
        std::string m_reference;
        std::string m_stringValue;
        double m_doubleValue = 0.0;
        float m_floatValue = 0.0f;
        int m_intValue = 0;
        bool m_boolValue = false;
    };

} // namespace HMREngine
