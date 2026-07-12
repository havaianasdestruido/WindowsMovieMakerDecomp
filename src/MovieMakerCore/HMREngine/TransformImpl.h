#pragma once
// TransformImpl.h - Transform, X3DBoundedObject

#include "HMREngine.h"
#include "X3DNodes.h"
#include "X3DMath.h"

namespace HMREngine
{
    class TransformImpl : public ImplObject<TransformImpl>
    {
    public:
        TransformImpl();
        virtual ~TransformImpl();

        HRESULT Initialize(TransformNode* node);
        void Shutdown();

        void UpdateMatrix();

        Matrix4f GetLocalMatrix() const { return m_localMatrix; }
        Matrix4f GetWorldMatrix() const { return m_worldMatrix; }
        void SetWorldMatrix(const Matrix4f& m) { m_worldMatrix = m; }

        Vec3 GetTranslation() const { return m_translation; }
        Rotation4f GetRotation() const { return m_rotation; }
        Vec3 GetScale() const { return m_scale; }

        void SetTranslation(const Vec3& t) { m_translation = t; m_dirty = true; }
        void SetRotation(const Rotation4f& r) { m_rotation = r; m_dirty = true; }
        void SetScale(const Vec3& s) { m_scale = s; m_dirty = true; }

        void Translate(const Vec3& delta);
        void Rotate(const Rotation4f& delta);

        bool IsDirty() const { return m_dirty; }
        void ClearDirty() { m_dirty = false; }

    private:
        Vec3 m_translation;
        Rotation4f m_rotation;
        Vec3 m_scale = Vec3(1, 1, 1);
        Vec3 m_center;
        Vec3 m_scaleOrientation;

        Matrix4f m_localMatrix;
        Matrix4f m_worldMatrix;
        bool m_dirty = true;
    };

    class BoundedObjectImpl : public ImplObject<BoundedObjectImpl>
    {
    public:
        HRESULT Initialize(const Vec3& bboxMin, const Vec3& bboxMax);
        void Shutdown();

        bool IsInFrustum(const Frustum& frustum) const;
        Vec3 GetCenter() const { return m_center; }
        Vec3 GetExtents() const { return m_extents; }

        void SetBBox(const Vec3& min, const Vec3& max);

    private:
        Vec3 m_bboxMin;
        Vec3 m_bboxMax;
        Vec3 m_center;
        Vec3 m_extents;
        bool m_dirty = true;

        void UpdateBounds();
    };

} // namespace HMREngine
