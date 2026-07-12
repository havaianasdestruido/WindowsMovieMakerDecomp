// X3DLayerImpls.cpp - Layer/viewport/bindable/text/vertex/metadata bridge implementation

#include "X3DLayerImpls.h"

namespace HMREngine
{

// ============================================================================
// X3DLayerNodeImpl
// ============================================================================
X3DLayerNodeImpl::X3DLayerNodeImpl() = default;
X3DLayerNodeImpl::~X3DLayerNodeImpl() { Shutdown(); }

HRESULT X3DLayerNodeImpl::Initialize(X3DLayerNode* node)
{
    m_layerNode = node;
    if (node)
    {
        m_pickable = node->m_pickable;
        m_visible = node->m_visible;
    }
    return X3DGroupingNodeImpl::Initialize(node);
}

void X3DLayerNodeImpl::Shutdown()
{
    m_layerNode = nullptr;
    X3DGroupingNodeImpl::Shutdown();
}

HRESULT X3DLayerNodeImpl::SetupFields()
{
    return X3DGroupingNodeImpl::SetupFields();
}

// ============================================================================
// X3DViewportNodeImpl
// ============================================================================
X3DViewportNodeImpl::X3DViewportNodeImpl() = default;
X3DViewportNodeImpl::~X3DViewportNodeImpl() { Shutdown(); }

HRESULT X3DViewportNodeImpl::Initialize(X3DChildNode* node)
{
    return X3DChildNodeImpl::Initialize(node);
}

void X3DViewportNodeImpl::Shutdown()
{
    X3DChildNodeImpl::Shutdown();
}

void X3DViewportNodeImpl::SetViewport(float x, float y, float w, float h)
{
    m_viewport[0] = x;
    m_viewport[1] = y;
    m_viewport[2] = w;
    m_viewport[3] = h;
    MarkDirty();
}

void X3DViewportNodeImpl::Apply(ID3D11DeviceContext* ctx, UINT rtWidth, UINT rtHeight)
{
    if (!ctx) return;

    D3D11_VIEWPORT vp = {};
    vp.TopLeftX = m_viewport[0] * rtWidth;
    vp.TopLeftY = m_viewport[1] * rtHeight;
    vp.Width = m_viewport[2] * rtWidth;
    vp.Height = m_viewport[3] * rtHeight;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;

    ctx->RSSetViewports(1, &vp);
}

HRESULT X3DViewportNodeImpl::SetupFields()
{
    return X3DChildNodeImpl::SetupFields();
}

// ============================================================================
// X3DBindableNodeImpl
// ============================================================================
X3DBindableNodeImpl::X3DBindableNodeImpl() = default;
X3DBindableNodeImpl::~X3DBindableNodeImpl() { Shutdown(); }

HRESULT X3DBindableNodeImpl::Initialize(X3DNode* node)
{
    return X3DNodeImpl::Initialize(node);
}

void X3DBindableNodeImpl::Shutdown()
{
    m_bindCallback = nullptr;
    X3DNodeImpl::Shutdown();
}

void X3DBindableNodeImpl::SetBound(bool bound)
{
    m_bound = bound;
    if (m_bindCallback) m_bindCallback(bound);
}

// ============================================================================
// X3DTextNodeImpl
// ============================================================================
X3DTextNodeImpl::X3DTextNodeImpl() = default;
X3DTextNodeImpl::~X3DTextNodeImpl() { Shutdown(); }

HRESULT X3DTextNodeImpl::Initialize(X3DGeometryNode* node)
{
    if (node)
    {
        TextNode* tn = static_cast<TextNode*>(node);
        m_strings = tn->m_string.m_values;
        if (tn->m_fontStyle)
        {
            m_fontFamily = tn->m_fontStyle->m_family;
            m_fontSize = tn->m_fontStyle->m_size;
        }
    }
    return X3DGeometryNodeImpl::Initialize(node);
}

void X3DTextNodeImpl::Shutdown()
{
    m_strings.clear();
    X3DGeometryNodeImpl::Shutdown();
}

HRESULT X3DTextNodeImpl::SetupFields()
{
    return X3DGeometryNodeImpl::SetupFields();
}

// ============================================================================
// X3DVertexAttributeNodeImpl
// ============================================================================
X3DVertexAttributeNodeImpl::X3DVertexAttributeNodeImpl() = default;
X3DVertexAttributeNodeImpl::~X3DVertexAttributeNodeImpl() { Shutdown(); }

HRESULT X3DVertexAttributeNodeImpl::Initialize(X3DNode* node)
{
    return X3DNodeImpl::Initialize(node);
}

void X3DVertexAttributeNodeImpl::Shutdown()
{
    X3DNodeImpl::Shutdown();
}

// ============================================================================
// X3DMetadataObjectImpl
// ============================================================================
X3DMetadataObjectImpl::X3DMetadataObjectImpl() = default;
X3DMetadataObjectImpl::~X3DMetadataObjectImpl() { Shutdown(); }

HRESULT X3DMetadataObjectImpl::Initialize(X3DNode* node)
{
    if (node)
    {
        if (auto* ms = dynamic_cast<MetadataStringNode*>(node))
        {
            m_metadataName = ms->m_name;
            m_reference = ms->m_reference;
            m_stringValue = ms->m_value;
        }
        else if (auto* md = dynamic_cast<MetadataDoubleNode*>(node))
        {
            m_metadataName = md->m_name;
            m_reference = md->m_reference;
            m_doubleValue = md->m_value;
        }
        else if (auto* mf = dynamic_cast<MetadataFloatNode*>(node))
        {
            m_metadataName = mf->m_name;
            m_reference = mf->m_reference;
            m_floatValue = mf->m_value;
        }
        else if (auto* mi = dynamic_cast<MetadataIntegerNode*>(node))
        {
            m_metadataName = mi->m_name;
            m_reference = mi->m_reference;
            m_intValue = mi->m_value;
        }
        else if (auto* mb = dynamic_cast<MetadataBoolNode*>(node))
        {
            m_metadataName = mb->m_name;
            m_reference = mb->m_reference;
            m_boolValue = mb->m_value;
        }
    }
    return X3DNodeImpl::Initialize(node);
}

void X3DMetadataObjectImpl::Shutdown()
{
    X3DNodeImpl::Shutdown();
}

HRESULT X3DMetadataObjectImpl::SetValueString(const std::string& value)
{
    m_stringValue = value;
    MarkDirty();
    return S_OK;
}

HRESULT X3DMetadataObjectImpl::SetValueDouble(double value)
{
    m_doubleValue = value;
    MarkDirty();
    return S_OK;
}

HRESULT X3DMetadataObjectImpl::SetValueFloat(float value)
{
    m_floatValue = value;
    MarkDirty();
    return S_OK;
}

HRESULT X3DMetadataObjectImpl::SetValueInt(int value)
{
    m_intValue = value;
    MarkDirty();
    return S_OK;
}

HRESULT X3DMetadataObjectImpl::SetValueBool(bool value)
{
    m_boolValue = value;
    MarkDirty();
    return S_OK;
}

} // namespace HMREngine
