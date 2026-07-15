#include "pch.h"
#include "ValueVariantRef.h"

namespace StoryboardManager
{

ValueVariantRef::ValueVariantRef()
    : m_type(VariantTypeNone)
    , nValue(0)
{
}

ValueVariantRef::ValueVariantRef(LPCWSTR pszValue)
    : m_type(VariantTypeString)
    , nValue(0)
{
    m_strValue = pszValue ? pszValue : L"";
}

ValueVariantRef::ValueVariantRef(int nValue)
    : m_type(VariantTypeInt)
    , nValue(nValue)
{
}

ValueVariantRef::ValueVariantRef(LONGLONG llValue)
    : m_type(VariantTypeLongLong)
    , llValue(llValue)
{
}

ValueVariantRef::ValueVariantRef(double dblValue)
    : m_type(VariantTypeDouble)
    , dblValue(dblValue)
{
}

ValueVariantRef::ValueVariantRef(bool fValue)
    : m_type(VariantTypeBool)
    , fValue(fValue)
{
}

ValueVariantRef::~ValueVariantRef()
{
}

ValueVariantRef::VariantType ValueVariantRef::GetType() const throw()
{
    return m_type;
}

ATL::CString ValueVariantRef::AsString() const
{
    switch (m_type)
    {
    case VariantTypeString:
        return m_strValue;
    case VariantTypeInt:
    {
        ATL::CString s;
        s.Format(L"%d", nValue);
        return s;
    }
    case VariantTypeLongLong:
    {
        ATL::CString s;
        s.Format(L"%lld", llValue);
        return s;
    }
    case VariantTypeDouble:
    {
        ATL::CString s;
        s.Format(L"%g", dblValue);
        return s;
    }
    case VariantTypeBool:
        return fValue ? L"true" : L"false";
    default:
        return L"";
    }
}

int ValueVariantRef::AsInt() const
{
    if (m_type == VariantTypeInt) return nValue;
    if (m_type == VariantTypeString) return _wtoi(m_strValue);
    if (m_type == VariantTypeBool) return fValue ? 1 : 0;
    if (m_type == VariantTypeDouble) return (int)dblValue;
    return 0;
}

LONGLONG ValueVariantRef::AsLongLong() const
{
    if (m_type == VariantTypeLongLong) return llValue;
    if (m_type == VariantTypeInt) return nValue;
    if (m_type == VariantTypeString) return _wtoi64(m_strValue);
    return 0;
}

double ValueVariantRef::AsDouble() const
{
    if (m_type == VariantTypeDouble) return dblValue;
    if (m_type == VariantTypeInt) return (double)nValue;
    if (m_type == VariantTypeLongLong) return (double)llValue;
    if (m_type == VariantTypeString) return _wtof(m_strValue);
    return 0.0;
}

bool ValueVariantRef::AsBool() const
{
    if (m_type == VariantTypeBool) return fValue;
    if (m_type == VariantTypeInt) return nValue != 0;
    if (m_type == VariantTypeString)
        return m_strValue.CompareNoCase(L"true") == 0 || m_strValue.CompareNoCase(L"1") == 0;
    return false;
}

void ValueVariantRef::SetString(LPCWSTR pszValue)
{
    m_type = VariantTypeString;
    m_strValue = pszValue ? pszValue : L"";
}

void ValueVariantRef::SetInt(int nValue)
{
    m_type = VariantTypeInt;
    this->nValue = nValue;
}

void ValueVariantRef::SetLongLong(LONGLONG llValue)
{
    m_type = VariantTypeLongLong;
    this->llValue = llValue;
}

void ValueVariantRef::SetDouble(double dblValue)
{
    m_type = VariantTypeDouble;
    this->dblValue = dblValue;
}

void ValueVariantRef::SetBool(bool fValue)
{
    m_type = VariantTypeBool;
    this->fValue = fValue;
}

LPCWSTR ValueVariantRef::GetOwnerElementName() const
{
    return m_strOwnerElement;
}

void ValueVariantRef::SetOwnerElementName(LPCWSTR pszName)
{
    m_strOwnerElement = pszName ? pszName : L"";
}

bool ValueVariantRef::IsEmpty() const throw()
{
    return m_type == VariantTypeNone;
}

bool ValueVariantRef::operator==(const ValueVariantRef& other) const
{
    if (m_type != other.m_type) return false;
    switch (m_type)
    {
    case VariantTypeNone: return true;
    case VariantTypeString: return m_strValue == other.m_strValue;
    case VariantTypeInt: return nValue == other.nValue;
    case VariantTypeLongLong: return llValue == other.llValue;
    case VariantTypeDouble: return dblValue == other.dblValue;
    case VariantTypeBool: return fValue == other.fValue;
    default: return false;
    }
}

bool ValueVariantRef::operator!=(const ValueVariantRef& other) const
{
    return !(*this == other);
}

// ============================================================================
// ValueVariantRefArray
// ============================================================================

ValueVariantRefArray::ValueVariantRefArray()
{
}

ValueVariantRefArray::~ValueVariantRefArray()
{
}

size_t ValueVariantRefArray::Add(const ValueVariantRef& var)
{
    size_t idx = m_arrVariants.GetCount();
    m_arrVariants.Add(var);
    return idx;
}

void ValueVariantRefArray::AddString(LPCWSTR pszName, LPCWSTR pszValue)
{
    ValueVariantRef var(pszValue);
    var.SetOwnerElementName(pszName);
    Add(var);
}

void ValueVariantRefArray::AddInt(LPCWSTR pszName, int nValue)
{
    ValueVariantRef var(nValue);
    var.SetOwnerElementName(pszName);
    Add(var);
}

void ValueVariantRefArray::AddLongLong(LPCWSTR pszName, LONGLONG llValue)
{
    ValueVariantRef var(llValue);
    var.SetOwnerElementName(pszName);
    Add(var);
}

void ValueVariantRefArray::AddDouble(LPCWSTR pszName, double dblValue)
{
    ValueVariantRef var(dblValue);
    var.SetOwnerElementName(pszName);
    Add(var);
}

void ValueVariantRefArray::AddBool(LPCWSTR pszName, bool fValue)
{
    ValueVariantRef var(fValue);
    var.SetOwnerElementName(pszName);
    Add(var);
}

size_t ValueVariantRefArray::GetCount() const throw()
{
    return m_arrVariants.GetCount();
}

const ValueVariantRef& ValueVariantRefArray::GetAt(size_t nIndex) const
{
    return m_arrVariants.GetAt(nIndex);
}

ValueVariantRef& ValueVariantRefArray::GetAt(size_t nIndex)
{
    return m_arrVariants.GetAt(nIndex);
}

int ValueVariantRefArray::FindByName(LPCWSTR pszName) const
{
    if (!pszName) return -1;
    for (size_t i = 0; i < m_arrVariants.GetCount(); ++i)
    {
        if (m_arrVariants.GetAt(i).GetOwnerElementName() &&
            wcscmp(m_arrVariants.GetAt(i).GetOwnerElementName(), pszName) == 0)
        {
            return (int)i;
        }
    }
    return -1;
}

void ValueVariantRefArray::RemoveAll()
{
    m_arrVariants.RemoveAll();
}

} // namespace StoryboardManager
