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
    delete m_geometry;
    m_geometry = nullptr;
    delete m_appearance;
    m_appearance = nullptr;
    m_shapeNode = nullptr;
    X3DChildNodeImpl::Shutdown();
}

void X3DShapeNodeImpl::SetGeometry(X3DGeometryNodeImpl* geo)
{
    if (m_geometry == geo) return;
    delete m_geometry;
    m_geometry = geo;
    MarkDirty();
}

void X3DShapeNodeImpl::SetAppearance(X3DAppearanceNodeImpl* app)
{
    if (m_appearance == app) return;
    delete m_appearance;
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
// TransformNodeImpl
// ============================================================================
TransformNodeImpl::TransformNodeImpl() = default;
TransformNodeImpl::~TransformNodeImpl() { Shutdown(); }

HRESULT TransformNodeImpl::Initialize(TransformNode* node)
{
    if (!node) return E_POINTER;
    m_transformNode = node;
    m_translation = node->m_translation;
    m_rotation = node->m_rotation;
    m_scale = node->m_scale;
    m_center = node->m_center;
    RebuildMatrix();
    return X3DGroupingNodeImpl::Initialize(node);
}

void TransformNodeImpl::Shutdown()
{
    m_transformNode = nullptr;
    X3DGroupingNodeImpl::Shutdown();
}

void TransformNodeImpl::SetTranslation(const Vec3& t)
{
    m_translation = t;
    if (m_transformNode) m_transformNode->m_translation = t;
    m_matrixDirty = true;
    MarkDirty();
}

void TransformNodeImpl::SetRotation(const Rotation4f& r)
{
    m_rotation = r;
    if (m_transformNode) m_transformNode->m_rotation = r;
    m_matrixDirty = true;
    MarkDirty();
}

void TransformNodeImpl::SetScale(const Vec3& s)
{
    m_scale = s;
    if (m_transformNode) m_transformNode->m_scale = s;
    m_matrixDirty = true;
    MarkDirty();
}

void TransformNodeImpl::SetCenter(const Vec3& c)
{
    m_center = c;
    if (m_transformNode) m_transformNode->m_center = c;
    m_matrixDirty = true;
    MarkDirty();
}

Matrix4f TransformNodeImpl::GetLocalMatrix() const
{
    return Matrix4f::FromTranslationRotationScale(m_translation, m_rotation, m_scale);
}

Matrix4f TransformNodeImpl::GetWorldMatrix() const
{
    return m_worldMatrix;
}

void TransformNodeImpl::RebuildMatrix()
{
    if (m_center.LengthSq() > 0.0f)
    {
        Matrix4f toCenter = Matrix4f::FromTranslation(-m_center);
        Matrix4f fromCenter = Matrix4f::FromTranslation(m_center);
        Matrix4f local = Matrix4f::FromTranslationRotationScale(m_translation, m_rotation, m_scale);
        m_worldMatrix = toCenter * local * fromCenter;
    }
    else
    {
        m_worldMatrix = GetLocalMatrix();
    }

    if (m_transformNode)
        m_transformNode->m_matrix = m_worldMatrix;

    m_matrixDirty = false;
}

HRESULT TransformNodeImpl::SetupFields()
{
    return X3DGroupingNodeImpl::SetupFields();
}

// ============================================================================
// GroupNodeImpl
// ============================================================================
GroupNodeImpl::GroupNodeImpl() = default;
GroupNodeImpl::~GroupNodeImpl() { Shutdown(); }

HRESULT GroupNodeImpl::Initialize(GroupNode* node)
{
    if (!node) return E_POINTER;
    m_groupNode = node;
    return X3DGroupingNodeImpl::Initialize(node);
}

void GroupNodeImpl::Shutdown()
{
    m_groupNode = nullptr;
    X3DGroupingNodeImpl::Shutdown();
}

void GroupNodeImpl::Traverse(std::function<void(X3DChildNodeImpl*, int)> visitor, int depth)
{
    visitor(this, depth);
    UINT count = GetGroupChildCount();
    for (UINT i = 0; i < count; ++i)
    {
        X3DChildNodeImpl* child = GetGroupChild(i);
        if (child && child->IsVisible())
        {
            child->Traverse(visitor, depth + 1);
        }
    }
}

HRESULT GroupNodeImpl::SetupFields()
{
    return X3DGroupingNodeImpl::SetupFields();
}

// ============================================================================
// SwitchNodeImpl
// ============================================================================
SwitchNodeImpl::SwitchNodeImpl() = default;
SwitchNodeImpl::~SwitchNodeImpl() { Shutdown(); }

HRESULT SwitchNodeImpl::Initialize(SwitchNode* node)
{
    if (!node) return E_POINTER;
    m_switchNode = node;
    m_choice = node->m_choice;
    m_switchEnabled = node->m_enabled;
    return X3DGroupingNodeImpl::Initialize(node);
}

void SwitchNodeImpl::Shutdown()
{
    m_switchNode = nullptr;
    X3DGroupingNodeImpl::Shutdown();
}

void SwitchNodeImpl::SetChoice(int choice)
{
    m_choice = choice;
    if (m_switchNode) m_switchNode->m_choice = choice;
    MarkDirty();
}

X3DChildNodeImpl* SwitchNodeImpl::GetActiveChild() const
{
    if (!m_switchEnabled) return nullptr;
    if (m_choice < 0) return nullptr;
    return GetGroupChild(static_cast<UINT>(m_choice));
}

void SwitchNodeImpl::TraverseActive(std::function<void(X3DChildNodeImpl*, int)> visitor, int depth)
{
    X3DChildNodeImpl* active = GetActiveChild();
    if (active)
    {
        visitor(active, depth);
        UINT count = active->GetNumChildren();
        for (UINT i = 0; i < count; ++i)
        {
            X3DChildNodeImpl* child = active->GetChild(i);
            if (child) child->Traverse(visitor, depth + 1);
        }
    }
}

HRESULT SwitchNodeImpl::SetupFields()
{
    return X3DGroupingNodeImpl::SetupFields();
}

// ============================================================================
// BillboardNodeImpl
// ============================================================================
BillboardNodeImpl::BillboardNodeImpl() = default;
BillboardNodeImpl::~BillboardNodeImpl() { Shutdown(); }

HRESULT BillboardNodeImpl::Initialize(X3DGroupingNode* node)
{
    m_billboardDirty = true;
    return X3DGroupingNodeImpl::Initialize(node);
}

void BillboardNodeImpl::Shutdown()
{
    X3DGroupingNodeImpl::Shutdown();
}

void BillboardNodeImpl::UpdateBillboard(const Vec3& eyePosition)
{
    if (!m_billboardDirty) return;

    Matrix4f viewInv;
    Vec3 fwd = eyePosition;
    if (fwd.LengthSq() > 0.0001f)
        fwd = fwd.Normalized();
    else
        fwd = Vec3(0, 0, -1);

    Vec3 up = m_axisOfRotation;
    if (up.LengthSq() < 0.0001f)
        up = Vec3(0, 1, 0);

    Vec3 right = up.Cross(fwd);
    if (right.LengthSq() < 0.0001f)
    {
        right = Vec3(1, 0, 0);
    }
    else
    {
        right = right.Normalized();
    }

    up = fwd.Cross(right);
    if (up.LengthSq() < 0.0001f)
        up = Vec3(0, 1, 0);
    else
        up = up.Normalized();

    viewInv._11 = right.x; viewInv._12 = right.y; viewInv._13 = right.z;
    viewInv._21 = up.x;    viewInv._22 = up.y;    viewInv._23 = up.z;
    viewInv._31 = fwd.x;   viewInv._32 = fwd.y;   viewInv._33 = fwd.z;

    m_billboardMatrix = viewInv;
    m_billboardDirty = false;
}

Matrix4f BillboardNodeImpl::GetBillboardMatrix() const
{
    return m_billboardMatrix;
}

HRESULT BillboardNodeImpl::SetupFields()
{
    return X3DGroupingNodeImpl::SetupFields();
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
