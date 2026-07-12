// TransformImpl.cpp - Transform/BoundedObject implementations

#include "TransformImpl.h"

namespace HMREngine
{
    // TransformImpl
    TransformImpl::TransformImpl() {}
    TransformImpl::~TransformImpl() { Shutdown(); }

    HRESULT TransformImpl::Initialize(TransformNode* node)
    {
        if (!node) return E_POINTER;

        m_translation = node->m_translation;
        m_rotation = node->m_rotation;
        m_scale = node->m_scale;
        m_center = node->m_center;
        m_scaleOrientation = node->m_scaleOrientation;

        UpdateMatrix();
        return S_OK;
    }

    void TransformImpl::Shutdown() {}

    void TransformImpl::UpdateMatrix()
    {
        if (!m_dirty) return;

        m_localMatrix = Matrix4f::FromTranslationRotationScale(m_translation, m_rotation, m_scale);
        m_worldMatrix = m_localMatrix;
        m_dirty = false;
    }

    void TransformImpl::Translate(const Vec3& delta)
    {
        m_translation = m_translation + delta;
        m_dirty = true;
    }

    void TransformImpl::Rotate(const Rotation4f& delta)
    {
        m_rotation = delta * m_rotation;
        m_dirty = true;
    }

    // BoundedObjectImpl
    HRESULT BoundedObjectImpl::Initialize(const Vec3& bboxMin, const Vec3& bboxMax)
    {
        m_bboxMin = bboxMin;
        m_bboxMax = bboxMax;
        UpdateBounds();
        return S_OK;
    }

    void BoundedObjectImpl::Shutdown() {}

    bool BoundedObjectImpl::IsInFrustum(const Frustum& frustum) const
    {
        if (m_dirty) const_cast<BoundedObjectImpl*>(this)->UpdateBounds();
        return frustum.IsBoxInside(m_center, m_extents);
    }

    void BoundedObjectImpl::SetBBox(const Vec3& min, const Vec3& max)
    {
        m_bboxMin = min;
        m_bboxMax = max;
        m_dirty = true;
    }

    void BoundedObjectImpl::UpdateBounds()
    {
        m_center = (m_bboxMin + m_bboxMax) * 0.5f;
        m_extents = (m_bboxMax - m_bboxMin) * 0.5f;
        m_dirty = false;
    }

} // namespace HMREngine
