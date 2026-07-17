#pragma once
// X3DFieldTypes.h - X3D field types for HMREngine

#include "X3DMath.h"
#include <vector>
#include <string>
#include <functional>
#include <atlbase.h>
#include <atlcom.h>

namespace HMREngine
{
    enum class FieldType
    {
        SFBool, SFInt32, SFFloat, SFDouble, SFString, SFTime,
        SFVec2f, SFVec3f, SFVec4f, SFRotation, SFColor, SFColorRGBA,
        SFMatrix3f, SFMatrix4f, SFNode,
        MFBool, MFInt32, MFFloat, MFDouble, MFString, MFTime,
        MFVec2f, MFVec3f, MFVec4f, MFRotation, MFColor, MFColorRGBA,
        MFMatrix3f, MFMatrix4f, MFNode,
    };

    class X3DFieldNode
    {
    public:
        virtual ~X3DFieldNode() = default;

        virtual FieldType GetFieldType() const = 0;
        virtual bool IsArray() const = 0;
        virtual void CopyFrom(const X3DFieldNode& other) = 0;
        virtual HRESULT Clone(X3DFieldNode** ppOut) const = 0;
    };

    template<typename T>
    class SingleFieldBase : public X3DFieldNode
    {
    public:
        T m_value{};

        bool IsArray() const override { return false; }

        const T& GetValue() const { return m_value; }
        void SetValue(const T& v) { m_value = v; }
    };

    template<typename T>
    class MultipleFieldBase : public X3DFieldNode
    {
    public:
        std::vector<T> m_values;

        bool IsArray() const override { return true; }

        size_t GetSize() const { return m_values.size(); }
        void Resize(size_t n) { m_values.resize(n); }
        const T& GetValue(size_t i) const { return m_values[i]; }
        void SetValue(size_t i, const T& v) { m_values[i] = v; }
        void PushBack(const T& v) { m_values.push_back(v); }
        void Clear() { m_values.clear(); }
    };

    template<typename T>
    class X3DArrayField : public MultipleFieldBase<T>
    {
    };

    // --- SingleField specializations ---

    class SFBool : public SingleFieldBase<bool>
    {
    public:
        FieldType GetFieldType() const override { return FieldType::SFBool; }
        void CopyFrom(const X3DFieldNode& other) override { m_value = static_cast<const SFBool&>(other).m_value; }
        HRESULT Clone(X3DFieldNode** pp) const override
        {
            SFBool* p = new (std::nothrow) SFBool();
            if (!p) return E_OUTOFMEMORY;
            p->m_value = m_value;
            *pp = p; return S_OK;
        }
    };

    class SFInt32 : public SingleFieldBase<int>
    {
    public:
        FieldType GetFieldType() const override { return FieldType::SFInt32; }
        void CopyFrom(const X3DFieldNode& other) override { m_value = static_cast<const SFInt32&>(other).m_value; }
        HRESULT Clone(X3DFieldNode** pp) const override
        {
            SFInt32* p = new (std::nothrow) SFInt32();
            if (!p) return E_OUTOFMEMORY;
            p->m_value = m_value;
            *pp = p; return S_OK;
        }
    };

    class SFFloat : public SingleFieldBase<float>
    {
    public:
        FieldType GetFieldType() const override { return FieldType::SFFloat; }
        void CopyFrom(const X3DFieldNode& other) override { m_value = static_cast<const SFFloat&>(other).m_value; }
        HRESULT Clone(X3DFieldNode** pp) const override
        {
            SFFloat* p = new (std::nothrow) SFFloat();
            if (!p) return E_OUTOFMEMORY;
            p->m_value = m_value;
            *pp = p; return S_OK;
        }
    };

    class SFDouble : public SingleFieldBase<double>
    {
    public:
        FieldType GetFieldType() const override { return FieldType::SFDouble; }
        void CopyFrom(const X3DFieldNode& other) override { m_value = static_cast<const SFDouble&>(other).m_value; }
        HRESULT Clone(X3DFieldNode** pp) const override
        {
            SFDouble* p = new (std::nothrow) SFDouble();
            if (!p) return E_OUTOFMEMORY;
            p->m_value = m_value;
            *pp = p; return S_OK;
        }
    };

    class SFString : public SingleFieldBase<std::string>
    {
    public:
        FieldType GetFieldType() const override { return FieldType::SFString; }
        void CopyFrom(const X3DFieldNode& other) override { m_value = static_cast<const SFString&>(other).m_value; }
        HRESULT Clone(X3DFieldNode** pp) const override
        {
            SFString* p = new (std::nothrow) SFString();
            if (!p) return E_OUTOFMEMORY;
            p->m_value = m_value;
            *pp = p; return S_OK;
        }
    };

    class SFTime : public SingleFieldBase<double>
    {
    public:
        FieldType GetFieldType() const override { return FieldType::SFTime; }
        void CopyFrom(const X3DFieldNode& other) override { m_value = static_cast<const SFTime&>(other).m_value; }
        HRESULT Clone(X3DFieldNode** pp) const override
        {
            SFTime* p = new (std::nothrow) SFTime();
            if (!p) return E_OUTOFMEMORY;
            p->m_value = m_value;
            *pp = p; return S_OK;
        }
    };

    class SFVec2f : public SingleFieldBase<Vec2>
    {
    public:
        FieldType GetFieldType() const override { return FieldType::SFVec2f; }
        void CopyFrom(const X3DFieldNode& other) override { m_value = static_cast<const SFVec2f&>(other).m_value; }
        HRESULT Clone(X3DFieldNode** pp) const override
        {
            SFVec2f* p = new (std::nothrow) SFVec2f();
            if (!p) return E_OUTOFMEMORY;
            p->m_value = m_value;
            *pp = p; return S_OK;
        }
    };

    class SFVec3f : public SingleFieldBase<Vec3>
    {
    public:
        FieldType GetFieldType() const override { return FieldType::SFVec3f; }
        void CopyFrom(const X3DFieldNode& other) override { m_value = static_cast<const SFVec3f&>(other).m_value; }
        HRESULT Clone(X3DFieldNode** pp) const override
        {
            SFVec3f* p = new (std::nothrow) SFVec3f();
            if (!p) return E_OUTOFMEMORY;
            p->m_value = m_value;
            *pp = p; return S_OK;
        }
    };

    class SFVec4f : public SingleFieldBase<Vec4>
    {
    public:
        FieldType GetFieldType() const override { return FieldType::SFVec4f; }
        void CopyFrom(const X3DFieldNode& other) override { m_value = static_cast<const SFVec4f&>(other).m_value; }
        HRESULT Clone(X3DFieldNode** pp) const override
        {
            SFVec4f* p = new (std::nothrow) SFVec4f();
            if (!p) return E_OUTOFMEMORY;
            p->m_value = m_value;
            *pp = p; return S_OK;
        }
    };

    class SFRotation : public SingleFieldBase<Rotation4f>
    {
    public:
        FieldType GetFieldType() const override { return FieldType::SFRotation; }
        void CopyFrom(const X3DFieldNode& other) override { m_value = static_cast<const SFRotation&>(other).m_value; }
        HRESULT Clone(X3DFieldNode** pp) const override
        {
            SFRotation* p = new (std::nothrow) SFRotation();
            if (!p) return E_OUTOFMEMORY;
            p->m_value = m_value;
            *pp = p; return S_OK;
        }

        Rotation4f Slerp(const Rotation4f& target, float fraction) const
        {
            float dot = m_value.x * target.x + m_value.y * target.y + m_value.z * target.z + m_value.w * target.w;
            Rotation4f b2 = target;
            if (dot < 0.0f) { dot = -dot; b2 = Rotation4f(-target.x, -target.y, -target.z, -target.w); }
            if (dot > 0.9995f)
            {
                return Rotation4f(
                    m_value.x + fraction * (b2.x - m_value.x),
                    m_value.y + fraction * (b2.y - m_value.y),
                    m_value.z + fraction * (b2.z - m_value.z),
                    m_value.w + fraction * (b2.w - m_value.w)).Normalized();
            }
            float theta0 = acosf(dot);
            float theta = theta0 * fraction;
            float s0 = cosf(theta) - dot * sinf(theta) / sinf(theta0);
            float s1 = sinf(theta) / sinf(theta0);
            return Rotation4f(
                s0 * m_value.x + s1 * b2.x, s0 * m_value.y + s1 * b2.y,
                s0 * m_value.z + s1 * b2.z, s0 * m_value.w + s1 * b2.w).Normalized();
        }
    };

    class SFColor : public SingleFieldBase<Rgb>
    {
    public:
        FieldType GetFieldType() const override { return FieldType::SFColor; }
        void CopyFrom(const X3DFieldNode& other) override { m_value = static_cast<const SFColor&>(other).m_value; }
        HRESULT Clone(X3DFieldNode** pp) const override
        {
            SFColor* p = new (std::nothrow) SFColor();
            if (!p) return E_OUTOFMEMORY;
            p->m_value = m_value;
            *pp = p; return S_OK;
        }
    };

    class SFColorRGBA : public SingleFieldBase<Rgba>
    {
    public:
        FieldType GetFieldType() const override { return FieldType::SFColorRGBA; }
        void CopyFrom(const X3DFieldNode& other) override { m_value = static_cast<const SFColorRGBA&>(other).m_value; }
        HRESULT Clone(X3DFieldNode** pp) const override
        {
            SFColorRGBA* p = new (std::nothrow) SFColorRGBA();
            if (!p) return E_OUTOFMEMORY;
            p->m_value = m_value;
            *pp = p; return S_OK;
        }
    };

    class SFMatrix3f : public SingleFieldBase<Matrix3f>
    {
    public:
        FieldType GetFieldType() const override { return FieldType::SFMatrix3f; }
        void CopyFrom(const X3DFieldNode& other) override { m_value = static_cast<const SFMatrix3f&>(other).m_value; }
        HRESULT Clone(X3DFieldNode** pp) const override
        {
            SFMatrix3f* p = new (std::nothrow) SFMatrix3f();
            if (!p) return E_OUTOFMEMORY;
            p->m_value = m_value;
            *pp = p; return S_OK;
        }
    };

    class SFMatrix4f : public SingleFieldBase<Matrix4f>
    {
    public:
        FieldType GetFieldType() const override { return FieldType::SFMatrix4f; }
        void CopyFrom(const X3DFieldNode& other) override { m_value = static_cast<const SFMatrix4f&>(other).m_value; }
        HRESULT Clone(X3DFieldNode** pp) const override
        {
            SFMatrix4f* p = new (std::nothrow) SFMatrix4f();
            if (!p) return E_OUTOFMEMORY;
            p->m_value = m_value;
            *pp = p; return S_OK;
        }
    };

    class SFNode : public SingleFieldBase<CComPtr<IUnknown>>
    {
    public:
        FieldType GetFieldType() const override { return FieldType::SFNode; }
        void CopyFrom(const X3DFieldNode& other) override { m_value = static_cast<const SFNode&>(other).m_value; }
        HRESULT Clone(X3DFieldNode** pp) const override
        {
            SFNode* p = new (std::nothrow) SFNode();
            if (!p) return E_OUTOFMEMORY;
            p->m_value = m_value;
            *pp = p; return S_OK;
        }
    };

    // --- MultipleField specializations ---

    class MFBool : public X3DArrayField<bool>
    {
    public:
        FieldType GetFieldType() const override { return FieldType::MFBool; }
        void CopyFrom(const X3DFieldNode& other) override { m_values = static_cast<const MFBool&>(other).m_values; }
        HRESULT Clone(X3DFieldNode** pp) const override
        {
            MFBool* p = new (std::nothrow) MFBool();
            if (!p) return E_OUTOFMEMORY;
            p->m_values = m_values;
            *pp = p; return S_OK;
        }
    };

    class MFInt32 : public X3DArrayField<int>
    {
    public:
        FieldType GetFieldType() const override { return FieldType::MFInt32; }
        void CopyFrom(const X3DFieldNode& other) override { m_values = static_cast<const MFInt32&>(other).m_values; }
        HRESULT Clone(X3DFieldNode** pp) const override
        {
            MFInt32* p = new (std::nothrow) MFInt32();
            if (!p) return E_OUTOFMEMORY;
            p->m_values = m_values;
            *pp = p; return S_OK;
        }
    };

    class MFFloat : public X3DArrayField<float>
    {
    public:
        FieldType GetFieldType() const override { return FieldType::MFFloat; }
        void CopyFrom(const X3DFieldNode& other) override { m_values = static_cast<const MFFloat&>(other).m_values; }
        HRESULT Clone(X3DFieldNode** pp) const override
        {
            MFFloat* p = new (std::nothrow) MFFloat();
            if (!p) return E_OUTOFMEMORY;
            p->m_values = m_values;
            *pp = p; return S_OK;
        }
    };

    class MFDouble : public X3DArrayField<double>
    {
    public:
        FieldType GetFieldType() const override { return FieldType::MFDouble; }
        void CopyFrom(const X3DFieldNode& other) override { m_values = static_cast<const MFDouble&>(other).m_values; }
        HRESULT Clone(X3DFieldNode** pp) const override
        {
            MFDouble* p = new (std::nothrow) MFDouble();
            if (!p) return E_OUTOFMEMORY;
            p->m_values = m_values;
            *pp = p; return S_OK;
        }
    };

    class MFString : public X3DArrayField<std::string>
    {
    public:
        FieldType GetFieldType() const override { return FieldType::MFString; }
        void CopyFrom(const X3DFieldNode& other) override { m_values = static_cast<const MFString&>(other).m_values; }
        HRESULT Clone(X3DFieldNode** pp) const override
        {
            MFString* p = new (std::nothrow) MFString();
            if (!p) return E_OUTOFMEMORY;
            p->m_values = m_values;
            *pp = p; return S_OK;
        }
    };

    class MFTime : public X3DArrayField<double>
    {
    public:
        FieldType GetFieldType() const override { return FieldType::MFTime; }
        void CopyFrom(const X3DFieldNode& other) override { m_values = static_cast<const MFTime&>(other).m_values; }
        HRESULT Clone(X3DFieldNode** pp) const override
        {
            MFTime* p = new (std::nothrow) MFTime();
            if (!p) return E_OUTOFMEMORY;
            p->m_values = m_values;
            *pp = p; return S_OK;
        }
    };

    class MFVec2f : public X3DArrayField<Vec2>
    {
    public:
        FieldType GetFieldType() const override { return FieldType::MFVec2f; }
        void CopyFrom(const X3DFieldNode& other) override { m_values = static_cast<const MFVec2f&>(other).m_values; }
        HRESULT Clone(X3DFieldNode** pp) const override
        {
            MFVec2f* p = new (std::nothrow) MFVec2f();
            if (!p) return E_OUTOFMEMORY;
            p->m_values = m_values;
            *pp = p; return S_OK;
        }
    };

    class MFVec3f : public X3DArrayField<Vec3>
    {
    public:
        FieldType GetFieldType() const override { return FieldType::MFVec3f; }
        void CopyFrom(const X3DFieldNode& other) override { m_values = static_cast<const MFVec3f&>(other).m_values; }
        HRESULT Clone(X3DFieldNode** pp) const override
        {
            MFVec3f* p = new (std::nothrow) MFVec3f();
            if (!p) return E_OUTOFMEMORY;
            p->m_values = m_values;
            *pp = p; return S_OK;
        }
    };

    class MFVec4f : public X3DArrayField<Vec4>
    {
    public:
        FieldType GetFieldType() const override { return FieldType::MFVec4f; }
        void CopyFrom(const X3DFieldNode& other) override { m_values = static_cast<const MFVec4f&>(other).m_values; }
        HRESULT Clone(X3DFieldNode** pp) const override
        {
            MFVec4f* p = new (std::nothrow) MFVec4f();
            if (!p) return E_OUTOFMEMORY;
            p->m_values = m_values;
            *pp = p; return S_OK;
        }
    };

    class MFRotation : public X3DArrayField<Rotation4f>
    {
    public:
        FieldType GetFieldType() const override { return FieldType::MFRotation; }
        void CopyFrom(const X3DFieldNode& other) override { m_values = static_cast<const MFRotation&>(other).m_values; }
        HRESULT Clone(X3DFieldNode** pp) const override
        {
            MFRotation* p = new (std::nothrow) MFRotation();
            if (!p) return E_OUTOFMEMORY;
            p->m_values = m_values;
            *pp = p; return S_OK;
        }
    };

    class MFColor : public X3DArrayField<Rgb>
    {
    public:
        FieldType GetFieldType() const override { return FieldType::MFColor; }
        void CopyFrom(const X3DFieldNode& other) override { m_values = static_cast<const MFColor&>(other).m_values; }
        HRESULT Clone(X3DFieldNode** pp) const override
        {
            MFColor* p = new (std::nothrow) MFColor();
            if (!p) return E_OUTOFMEMORY;
            p->m_values = m_values;
            *pp = p; return S_OK;
        }
    };

    class MFColorRGBA : public X3DArrayField<Rgba>
    {
    public:
        FieldType GetFieldType() const override { return FieldType::MFColorRGBA; }
        void CopyFrom(const X3DFieldNode& other) override { m_values = static_cast<const MFColorRGBA&>(other).m_values; }
        HRESULT Clone(X3DFieldNode** pp) const override
        {
            MFColorRGBA* p = new (std::nothrow) MFColorRGBA();
            if (!p) return E_OUTOFMEMORY;
            p->m_values = m_values;
            *pp = p; return S_OK;
        }
    };

    class MFMatrix3f : public X3DArrayField<Matrix3f>
    {
    public:
        FieldType GetFieldType() const override { return FieldType::MFMatrix3f; }
        void CopyFrom(const X3DFieldNode& other) override { m_values = static_cast<const MFMatrix3f&>(other).m_values; }
        HRESULT Clone(X3DFieldNode** pp) const override
        {
            MFMatrix3f* p = new (std::nothrow) MFMatrix3f();
            if (!p) return E_OUTOFMEMORY;
            p->m_values = m_values;
            *pp = p; return S_OK;
        }
    };

    class MFMatrix4f : public X3DArrayField<Matrix4f>
    {
    public:
        FieldType GetFieldType() const override { return FieldType::MFMatrix4f; }
        void CopyFrom(const X3DFieldNode& other) override { m_values = static_cast<const MFMatrix4f&>(other).m_values; }
        HRESULT Clone(X3DFieldNode** pp) const override
        {
            MFMatrix4f* p = new (std::nothrow) MFMatrix4f();
            if (!p) return E_OUTOFMEMORY;
            p->m_values = m_values;
            *pp = p; return S_OK;
        }
    };

    class MFNode : public X3DArrayField<CComPtr<IUnknown>>
    {
    public:
        FieldType GetFieldType() const override { return FieldType::MFNode; }
        void CopyFrom(const X3DFieldNode& other) override { m_values = static_cast<const MFNode&>(other).m_values; }
        HRESULT Clone(X3DFieldNode** pp) const override
        {
            MFNode* p = new (std::nothrow) MFNode();
            if (!p) return E_OUTOFMEMORY;
            p->m_values = m_values;
            *pp = p; return S_OK;
        }
    };

} // namespace HMREngine
