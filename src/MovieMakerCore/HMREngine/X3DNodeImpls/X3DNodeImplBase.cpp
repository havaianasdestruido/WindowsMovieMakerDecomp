#include "pch.h"
// X3DNodeImplBase.cpp - Base bridge implementation

#include "X3DNodeImplBase.h"

namespace HMREngine
{

// ============================================================================
// FieldImpl
// ============================================================================
FieldImpl::FieldImpl() = default;
FieldImpl::~FieldImpl() = default;

HRESULT FieldImpl::Initialize(const std::string& name, FieldType type, X3DFieldNode* field)
{
    m_name = name;
    m_type = type;
    m_field = field;
    m_modified = false;
    return S_OK;
}

void FieldImpl::Shutdown()
{
    m_field = nullptr;
    m_changeCallback = nullptr;
}

HRESULT FieldImpl::SetValue(X3DFieldNode* value)
{
    if (!value || !m_field) return E_POINTER;
    if (value->GetFieldType() != m_type) return E_INVALIDARG;

    m_field->CopyFrom(*value);
    m_modified = true;

    if (m_changeCallback)
        m_changeCallback(this);

    return S_OK;
}

HRESULT FieldImpl::GetValue(X3DFieldNode** ppValue) const
{
    if (!ppValue) return E_POINTER;
    if (!m_field) return E_FAIL;

    return m_field->Clone(ppValue);
}

// ============================================================================
// FieldCollection
// ============================================================================
FieldCollection::FieldCollection() = default;
FieldCollection::~FieldCollection() { Clear(); }

HRESULT FieldCollection::AddField(const std::string& name, FieldType type, X3DFieldNode* field)
{
    if (HasField(name)) return E_FAIL;

    auto impl = std::make_unique<FieldImpl>();
    HRESULT hr = impl->Initialize(name, type, field);
    if (FAILED(hr)) return hr;

    UINT index = static_cast<UINT>(m_fields.size());
    m_fieldIndex[name] = index;
    m_fields.push_back(std::move(impl));

    if (m_changedCallback)
        m_changedCallback(name, true);

    return S_OK;
}

HRESULT FieldCollection::RemoveField(const std::string& name)
{
    auto it = m_fieldIndex.find(name);
    if (it == m_fieldIndex.end()) return E_FAIL;

    m_fields.erase(m_fields.begin() + it->second);
    m_fieldIndex.erase(it);

    m_fieldIndex.clear();
    for (UINT i = 0; i < static_cast<UINT>(m_fields.size()); ++i)
        m_fieldIndex[m_fields[i]->GetName()] = i;

    if (m_changedCallback)
        m_changedCallback(name, false);

    return S_OK;
}

FieldImpl* FieldCollection::GetField(const std::string& name) const
{
    auto it = m_fieldIndex.find(name);
    if (it == m_fieldIndex.end()) return nullptr;
    return m_fields[it->second].get();
}

HRESULT FieldCollection::GetFieldByIndex(UINT index, FieldImpl** ppField) const
{
    if (!ppField) return E_POINTER;
    if (index >= m_fields.size()) return E_INVALIDARG;

    *ppField = m_fields[index].get();
    return S_OK;
}

bool FieldCollection::HasField(const std::string& name) const
{
    return m_fieldIndex.find(name) != m_fieldIndex.end();
}

void FieldCollection::Clear()
{
    m_fields.clear();
    m_fieldIndex.clear();
}

// ============================================================================
// IsActiveBase
// ============================================================================
IsActiveBase::IsActiveBase() = default;
IsActiveBase::~IsActiveBase() = default;

void IsActiveBase::SetActive(bool active)
{
    if (m_isActive == active) return;
    m_isActive = active;
    OnActiveChanged(active);
    if (m_activeChangedCallback)
        m_activeChangedCallback(active);
}

// ============================================================================
// TransparentObject
// ============================================================================
TransparentObject::TransparentObject() = default;
TransparentObject::~TransparentObject() = default;

// ============================================================================
// X3DNodeImpl
// ============================================================================
X3DNodeImpl::X3DNodeImpl() = default;
X3DNodeImpl::~X3DNodeImpl() { Shutdown(); }

HRESULT X3DNodeImpl::Initialize(X3DNode* node)
{
    if (!node) return E_POINTER;

    m_node = node;
    m_nodeName = node->GetNodeName();
    m_nodeTypeName = node->GetNodeTypeName();

    return SetupFields();
}

void X3DNodeImpl::Shutdown()
{
    m_fields.Clear();
    m_node = nullptr;
    m_userData = nullptr;
    m_dirty = false;
}

HRESULT X3DNodeImpl::OnFieldChanged(const std::string& fieldName, X3DFieldNode* value)
{
    FieldImpl* field = m_fields.GetField(fieldName);
    if (field && value)
    {
        field->SetValue(value);
        m_dirty = true;
    }
    return S_OK;
}

HRESULT X3DNodeImpl::OnPropertyChanged(const std::string& propertyName)
{
    m_dirty = true;
    return S_OK;
}

HRESULT X3DNodeImpl::SetupFields()
{
    if (!m_node) return E_FAIL;
    return S_OK;
}

// ============================================================================
// X3DChildNodeImpl
// ============================================================================
X3DChildNodeImpl::X3DChildNodeImpl() = default;
X3DChildNodeImpl::~X3DChildNodeImpl() { Shutdown(); }

HRESULT X3DChildNodeImpl::Initialize(X3DChildNode* node)
{
    m_childNode = node;
    return X3DNodeImpl::Initialize(node);
}

void X3DChildNodeImpl::Shutdown()
{
    m_children.clear();
    m_parent = nullptr;
    m_childNode = nullptr;
    X3DNodeImpl::Shutdown();
}

X3DChildNodeImpl* X3DChildNodeImpl::GetChild(UINT index) const
{
    if (index >= m_children.size()) return nullptr;
    return m_children[index];
}

void X3DChildNodeImpl::AddChild(X3DChildNodeImpl* child)
{
    if (!child) return;
    child->SetParent(this);
    m_children.push_back(child);
}

void X3DChildNodeImpl::RemoveChild(X3DChildNodeImpl* child)
{
    for (auto it = m_children.begin(); it != m_children.end(); ++it)
    {
        if (*it == child)
        {
            child->SetParent(nullptr);
            m_children.erase(it);
            return;
        }
    }
}

HRESULT X3DChildNodeImpl::SetupFields()
{
    return X3DNodeImpl::SetupFields();
}

// ============================================================================
// X3DChildObjectImpl
// ============================================================================
X3DChildObjectImpl::X3DChildObjectImpl() = default;
X3DChildObjectImpl::~X3DChildObjectImpl() { Shutdown(); }

HRESULT X3DChildObjectImpl::Initialize(X3DChildNode* node)
{
    m_childObject = node;
    return X3DNodeImpl::Initialize(node);
}

void X3DChildObjectImpl::Shutdown()
{
    m_childObject = nullptr;
    X3DNodeImpl::Shutdown();
}

// ============================================================================
// X3DBoundedObjectImpl
// ============================================================================
X3DBoundedObjectImpl::X3DBoundedObjectImpl() = default;
X3DBoundedObjectImpl::~X3DBoundedObjectImpl() = default;

void X3DBoundedObjectImpl::SetBoundingBox(const Vec3& center, const Vec3& extents)
{
    m_bboxCenter = center;
    m_bboxExtents = extents;
    m_centerValid = true;
}

void X3DBoundedObjectImpl::GetBoundingBox(Vec3& center, Vec3& extents) const
{
    center = m_bboxCenter;
    extents = m_bboxExtents;
}

} // namespace HMREngine
