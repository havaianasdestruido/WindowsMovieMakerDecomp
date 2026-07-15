#include "pch.h"

// CoordinateImpl.cpp - Coordinate/Normal/Color/ColorRGBA/TextureCoordinate implementations

#include "CoordinateImpl.h"

namespace HMREngine
{
    // CoordinateImpl
    HRESULT CoordinateImpl::Initialize(CoordinateNode* node)
    {
        if (!node) return E_POINTER;
        m_points = node->m_point;
        return S_OK;
    }

    void CoordinateImpl::Shutdown() { m_points.clear(); }

    Vec3 CoordinateImpl::GetPoint(size_t index) const
    {
        return index < m_points.size() ? m_points[index] : Vec3();
    }

    // NormalImpl
    HRESULT NormalImpl::Initialize(NormalNode* node)
    {
        if (!node) return E_POINTER;
        m_vectors = node->m_vector;
        return S_OK;
    }

    void NormalImpl::Shutdown() { m_vectors.clear(); }

    Vec3 NormalImpl::GetVector(size_t index) const
    {
        return index < m_vectors.size() ? m_vectors[index] : Vec3(0, 1, 0);
    }

    // ColorImpl
    HRESULT ColorImpl::Initialize(ColorNode* node)
    {
        if (!node) return E_POINTER;
        m_colors = node->m_color;
        return S_OK;
    }

    void ColorImpl::Shutdown() { m_colors.clear(); }

    Rgb ColorImpl::GetColor(size_t index) const
    {
        return index < m_colors.size() ? m_colors[index] : Rgb(1, 1, 1);
    }

    // ColorRGBAImpl
    HRESULT ColorRGBAImpl::Initialize(ColorRGBANode* node)
    {
        if (!node) return E_POINTER;
        m_colors = node->m_color;
        return S_OK;
    }

    void ColorRGBAImpl::Shutdown() { m_colors.clear(); }

    Rgba ColorRGBAImpl::GetColor(size_t index) const
    {
        return index < m_colors.size() ? m_colors[index] : Rgba(1, 1, 1, 1);
    }

    // TextureCoordinateImpl
    HRESULT TextureCoordinateImpl::Initialize(TextureCoordinateNode* node)
    {
        if (!node) return E_POINTER;
        m_points = node->m_point;
        return S_OK;
    }

    void TextureCoordinateImpl::Shutdown() { m_points.clear(); }

    Vec2 TextureCoordinateImpl::GetPoint(size_t index) const
    {
        return index < m_points.size() ? m_points[index] : Vec2();
    }

} // namespace HMREngine
