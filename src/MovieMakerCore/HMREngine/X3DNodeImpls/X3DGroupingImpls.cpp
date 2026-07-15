#include "pch.h"
// X3DGroupingImpls.cpp - Grouping/shape/geometry bridge implementation

#include "X3DGroupingImpls.h"

namespace HMREngine
{

// ============================================================================
// X3DGroupingNodeImpl
// ============================================================================
X3DGroupingNodeImpl::X3DGroupingNodeImpl() = default;
X3DGroupingNodeImpl::~X3DGroupingNodeImpl() { Shutdown(); }

HRESULT X3DGroupingNodeImpl::Initialize(X3DGroupingNode* node)
{
    m_groupingNode = node;
    return X3DChildNodeImpl::Initialize(node);
}

void X3DGroupingNodeImpl::Shutdown()
{
    m_groupingNode = nullptr;
    X3DChildNodeImpl::Shutdown();
}

void X3DGroupingNodeImpl::AddGroupChild(X3DChildNodeImpl* child)
{
    AddChild(child);
}

void X3DGroupingNodeImpl::RemoveGroupChild(X3DChildNodeImpl* child)
{
    RemoveChild(child);
}

UINT X3DGroupingNodeImpl::GetGroupChildCount() const
{
    return GetNumChildren();
}

X3DChildNodeImpl* X3DGroupingNodeImpl::GetGroupChild(UINT index) const
{
    return GetChild(index);
}

HRESULT X3DGroupingNodeImpl::SetupFields()
{
    return X3DChildNodeImpl::SetupFields();
}

// ============================================================================
// X3DShapeNodeImpl
// ============================================================================
X3DShapeNodeImpl::X3DShapeNodeImpl() = default;
X3DShapeNodeImpl::~X3DShapeNodeImpl() { Shutdown(); }

HRESULT X3DShapeNodeImpl::Initialize(X3DShapeNode* node)
{
    m_shapeNode = node;
    return X3DChildNodeImpl::Initialize(node);
}

void X3DShapeNodeImpl::Shutdown()
{
    m_geometry = nullptr;
    m_appearance = nullptr;
    m_shapeNode = nullptr;
    X3DChildNodeImpl::Shutdown();
}

void X3DShapeNodeImpl::SetGeometry(X3DGeometryNodeImpl* geo)
{
    m_geometry = geo;
    MarkDirty();
}

void X3DShapeNodeImpl::SetAppearance(X3DAppearanceNodeImpl* app)
{
    m_appearance = app;
    MarkDirty();
}

HRESULT X3DShapeNodeImpl::SetupFields()
{
    return X3DChildNodeImpl::SetupFields();
}

// ============================================================================
// X3DGeometryNodeImpl
// ============================================================================
X3DGeometryNodeImpl::X3DGeometryNodeImpl() = default;
X3DGeometryNodeImpl::~X3DGeometryNodeImpl() { Shutdown(); }

HRESULT X3DGeometryNodeImpl::Initialize(X3DGeometryNode* node)
{
    m_geometryNode = node;
    return X3DChildNodeImpl::Initialize(node);
}

void X3DGeometryNodeImpl::Shutdown()
{
    m_geometryNode = nullptr;
    X3DChildNodeImpl::Shutdown();
}

HRESULT X3DGeometryNodeImpl::BuildVertexBuffer(ID3D11Device* dev, ID3D11Buffer** ppVB)
{
    if (!m_geometryNode) return E_FAIL;
    return m_geometryNode->BuildVertexBuffer(dev, ppVB);
}

HRESULT X3DGeometryNodeImpl::BuildIndexBuffer(ID3D11Device* dev, ID3D11Buffer** ppIB)
{
    if (!m_geometryNode) return E_FAIL;
    return m_geometryNode->BuildIndexBuffer(dev, ppIB);
}

UINT X3DGeometryNodeImpl::GetVertexCount() const
{
    return m_geometryNode ? m_geometryNode->GetVertexCount() : 0;
}

UINT X3DGeometryNodeImpl::GetIndexCount() const
{
    return m_geometryNode ? m_geometryNode->GetIndexCount() : 0;
}

HRESULT X3DGeometryNodeImpl::SetupFields()
{
    return X3DChildNodeImpl::SetupFields();
}

// ============================================================================
// X3DComposedGeometryNodeImpl
// ============================================================================
X3DComposedGeometryNodeImpl::X3DComposedGeometryNodeImpl() = default;
X3DComposedGeometryNodeImpl::~X3DComposedGeometryNodeImpl() { Shutdown(); }

HRESULT X3DComposedGeometryNodeImpl::Initialize(X3DGeometryNode* node)
{
    return X3DGeometryNodeImpl::Initialize(node);
}

void X3DComposedGeometryNodeImpl::Shutdown()
{
    X3DGeometryNodeImpl::Shutdown();
}

HRESULT X3DComposedGeometryNodeImpl::BuildVertexBuffer(ID3D11Device* dev, ID3D11Buffer** ppVB)
{
    return X3DGeometryNodeImpl::BuildVertexBuffer(dev, ppVB);
}

HRESULT X3DComposedGeometryNodeImpl::BuildIndexBuffer(ID3D11Device* dev, ID3D11Buffer** ppIB)
{
    return X3DGeometryNodeImpl::BuildIndexBuffer(dev, ppIB);
}

UINT X3DComposedGeometryNodeImpl::GetVertexCount() const
{
    return X3DGeometryNodeImpl::GetVertexCount();
}

UINT X3DComposedGeometryNodeImpl::GetIndexCount() const
{
    return X3DGeometryNodeImpl::GetIndexCount();
}

HRESULT X3DComposedGeometryNodeImpl::SetupFields()
{
    return X3DGeometryNodeImpl::SetupFields();
}

} // namespace HMREngine
