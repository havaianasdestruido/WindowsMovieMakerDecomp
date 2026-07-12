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

    class X3DFieldNode : public CComObjectRootEx<CComSingleThreadModel>
    {
    public:
        virtual ~X3DFieldNode() = default;

        virtual FieldType GetFieldType() const = 0;
        virtual bool IsArray() const = 0;
        virtual void CopyFrom(const X3DFieldNode& other) = 0;
        virtual HRESULT Clone(X3DFieldNode** ppOut) const = 0;

        BEGIN_COM_MAP(X3DFieldNode)
        END_COM_MAP()
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
            CComObject<SFBool>* p; CComObject<SFBool>::CreateInstance(&p); p->m_value = m_value;
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
            CComObject<SFInt32>* p; CComObject<SFInt32>::CreateInstance(&p); p->m_value = m_value;
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
            CComObject<SFFloat>* p; CComObject<SFFloat>::CreateInstance(&p); p->m_value = m_value;
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
            CComObject<SFDouble>* p; CComObject<SFDouble>::CreateInstance(&p); p->m_value = m_value;
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
            CComObject<SFString>* p; CComObject<SFString>::CreateInstance(&p); p->m_value = m_value;
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
            CComObject<SFTime>* p; CComObject<SFTime>::CreateInstance(&p); p->m_value = m_value;
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
            CComObject<SFVec2f>* p; CComObject<SFVec2f>::CreateInstance(&p); p->m_value = m_value;
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
            CComObject<SFVec3f>* p; CComObject<SFVec3f>::CreateInstance(&p); p->m_value = m_value;
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
            CComObject<SFVec4f>* p; CComObject<SFVec4f>::CreateInstance(&p); p->m_value = m_value;
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
            CComObject<SFRotation>* p; CComObject<SFRotation>::CreateInstance(&p); p->m_value = m_value;
            *pp = p; return S_OK;
        }
    };

    class SFColor : public SingleFieldBase<Rgb>
    {
    public:
        FieldType GetFieldType() const override { return FieldType::SFColor; }
        void CopyFrom(const X3DFieldNode& other) override { m_value = static_cast<const SFColor&>(other).m_value; }
        HRESULT Clone(X3DFieldNode** pp) const override
        {
            CComObject<SFColor>* p; CComObject<SFColor>::CreateInstance(&p); p->m_value = m_value;
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
            CComObject<SFColorRGBA>* p; CComObject<SFColorRGBA>::CreateInstance(&p); p->m_value = m_value;
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
            CComObject<SFMatrix3f>* p; CComObject<SFMatrix3f>::CreateInstance(&p); p->m_value = m_value;
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
            CComObject<SFMatrix4f>* p; CComObject<SFMatrix4f>::CreateInstance(&p); p->m_value = m_value;
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
            CComObject<SFNode>* p; CComObject<SFNode>::CreateInstance(&p); p->m_value = m_value;
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
            CComObject<MFBool>* p; CComObject<MFBool>::CreateInstance(&p); p->m_values = m_values;
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
            CComObject<MFInt32>* p; CComObject<MFInt32>::CreateInstance(&p); p->m_values = m_values;
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
            CComObject<MFFloat>* p; CComObject<MFFloat>::CreateInstance(&p); p->m_values = m_values;
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
            CComObject<MFDouble>* p; CComObject<MFDouble>::CreateInstance(&p); p->m_values = m_values;
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
            CComObject<MFString>* p; CComObject<MFString>::CreateInstance(&p); p->m_values = m_values;
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
            CComObject<MFTime>* p; CComObject<MFTime>::CreateInstance(&p); p->m_values = m_values;
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
            CComObject<MFVec2f>* p; CComObject<MFVec2f>::CreateInstance(&p); p->m_values = m_values;
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
            CComObject<MFVec3f>* p; CComObject<MFVec3f>::CreateInstance(&p); p->m_values = m_values;
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
            CComObject<MFVec4f>* p; CComObject<MFVec4f>::CreateInstance(&p); p->m_values = m_values;
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
            CComObject<MFRotation>* p; CComObject<MFRotation>::CreateInstance(&p); p->m_values = m_values;
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
            CComObject<MFColor>* p; CComObject<MFColor>::CreateInstance(&p); p->m_values = m_values;
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
            CComObject<MFColorRGBA>* p; CComObject<MFColorRGBA>::CreateInstance(&p); p->m_values = m_values;
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
            CComObject<MFMatrix3f>* p; CComObject<MFMatrix3f>::CreateInstance(&p); p->m_values = m_values;
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
            CComObject<MFMatrix4f>* p; CComObject<MFMatrix4f>::CreateInstance(&p); p->m_values = m_values;
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
            CComObject<MFNode>* p; CComObject<MFNode>::CreateInstance(&p); p->m_values = m_values;
            *pp = p; return S_OK;
        }
    };

} // namespace HMREngine
