#include "pch.h"

// MetadataImpl.cpp - Metadata implementations

#include "MetadataImpl.h"

namespace HMREngine
{
    // MetadataStringImpl
    HRESULT MetadataStringImpl::Initialize(MetadataStringNode* node)
    {
        if (!node) return E_POINTER;
        m_name = node->m_name;
        m_reference = node->m_reference;
        m_value = node->m_value;
        m_values = node->m_values;
        return S_OK;
    }
    void MetadataStringImpl::Shutdown() {}

    // MetadataDoubleImpl
    HRESULT MetadataDoubleImpl::Initialize(MetadataDoubleNode* node)
    {
        if (!node) return E_POINTER;
        m_name = node->m_name;
        m_reference = node->m_reference;
        m_value = node->m_value;
        m_values = node->m_values;
        return S_OK;
    }
    void MetadataDoubleImpl::Shutdown() {}

    // MetadataFloatImpl
    HRESULT MetadataFloatImpl::Initialize(MetadataFloatNode* node)
    {
        if (!node) return E_POINTER;
        m_name = node->m_name;
        m_reference = node->m_reference;
        m_value = node->m_value;
        m_values = node->m_values;
        return S_OK;
    }
    void MetadataFloatImpl::Shutdown() {}

    // MetadataIntegerImpl
    HRESULT MetadataIntegerImpl::Initialize(MetadataIntegerNode* node)
    {
        if (!node) return E_POINTER;
        m_name = node->m_name;
        m_reference = node->m_reference;
        m_value = node->m_value;
        m_values = node->m_values;
        return S_OK;
    }
    void MetadataIntegerImpl::Shutdown() {}

    // MetadataBoolImpl
    HRESULT MetadataBoolImpl::Initialize(MetadataBoolNode* node)
    {
        if (!node) return E_POINTER;
        m_name = node->m_name;
        m_reference = node->m_reference;
        m_value = node->m_value;
        m_values = node->m_values;
        return S_OK;
    }
    void MetadataBoolImpl::Shutdown() {}

    // MetadataSetImpl
    MetadataSetImpl::MetadataSetImpl() {}
    MetadataSetImpl::~MetadataSetImpl() { Shutdown(); }

    HRESULT MetadataSetImpl::Initialize(MetadataSetNode* node)
    {
        if (!node) return E_POINTER;
        m_name = node->m_name;
        m_reference = node->m_reference;

        for (auto& child : node->m_metadata)
        {
            if (child) m_metadata.push_back(child);
        }
        return S_OK;
    }

    void MetadataSetImpl::Shutdown()
    {
        m_metadata.clear();
    }

    X3DChildNode* MetadataSetImpl::GetMetadata(size_t index) const
    {
        if (index < m_metadata.size()) return m_metadata[index].Get();
        return nullptr;
    }

} // namespace HMREngine
