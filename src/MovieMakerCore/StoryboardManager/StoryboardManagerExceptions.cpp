/*
 * StoryboardManagerExceptions.cpp
 *
 * Implementation of the StoryboardManager exception classes:
 *   - StoryboardManagerException (base)
 *   - TemplateInitializationException
 *   - ScriptInitializationException
 *   - EncodeInitializationException
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "StoryboardManagerExceptions.h"

namespace StoryboardManager
{

// ============================================================================
// StoryboardManagerException implementation
// ============================================================================

StoryboardManagerException::StoryboardManagerException(HRESULT hr)
    : Base::Exception(hr)
{
}

StoryboardManagerException::StoryboardManagerException(HRESULT hr, LPCWSTR pszMessage)
    : Base::Exception(hr)
    , m_strMessage(pszMessage ? pszMessage : L"")
{
}

StoryboardManagerException::StoryboardManagerException(HRESULT hr, const ATL::CString& strMessage)
    : Base::Exception(hr)
    , m_strMessage(strMessage)
{
}

StoryboardManagerException::StoryboardManagerException(const StoryboardManagerException& other)
    : Base::Exception(other)
    , m_strMessage(other.m_strMessage)
{
}

StoryboardManagerException::~StoryboardManagerException() throw()
{
}

ATL::CString StoryboardManagerException::GetMessage() const
{
    return m_strMessage;
}

void StoryboardManagerException::Throw() const
{
    Base::Throw(GetHResult());
}

StoryboardManagerException& StoryboardManagerException::operator=(const StoryboardManagerException& other)
{
    if (this != &other)
    {
        Base::Exception::operator=(other);
        m_strMessage = other.m_strMessage;
    }
    return *this;
}

// ============================================================================
// TemplateInitializationException implementation
// ============================================================================

TemplateInitializationException::TemplateInitializationException()
    : StoryboardManagerException(E_FAIL)
{
}

TemplateInitializationException::TemplateInitializationException(HRESULT hr)
    : StoryboardManagerException(hr)
{
}

TemplateInitializationException::TemplateInitializationException(HRESULT hr, LPCWSTR pszMessage)
    : StoryboardManagerException(hr, pszMessage)
{
}

TemplateInitializationException::TemplateInitializationException(
    const TemplateInitializationException& other)
    : StoryboardManagerException(other)
    , m_strTemplateName(other.m_strTemplateName)
{
}

TemplateInitializationException::~TemplateInitializationException() throw()
{
}

void TemplateInitializationException::Throw() const
{
    Base::Throw(GetHResult());
}

TemplateInitializationException& TemplateInitializationException::operator=(
    const TemplateInitializationException& other)
{
    if (this != &other)
    {
        StoryboardManagerException::operator=(other);
        m_strTemplateName = other.m_strTemplateName;
    }
    return *this;
}

ATL::CString TemplateInitializationException::GetTemplateName() const
{
    return m_strTemplateName;
}

void TemplateInitializationException::SetTemplateName(LPCWSTR pszTemplateName)
{
    m_strTemplateName = pszTemplateName ? pszTemplateName : L"";
}

// ============================================================================
// ScriptInitializationException implementation
// ============================================================================

ScriptInitializationException::ScriptInitializationException()
    : StoryboardManagerException(E_FAIL)
{
}

ScriptInitializationException::ScriptInitializationException(HRESULT hr)
    : StoryboardManagerException(hr)
{
}

ScriptInitializationException::ScriptInitializationException(HRESULT hr, LPCWSTR pszMessage)
    : StoryboardManagerException(hr, pszMessage)
{
}

ScriptInitializationException::ScriptInitializationException(
    const ScriptInitializationException& other)
    : StoryboardManagerException(other)
    , m_strScriptPath(other.m_strScriptPath)
    , m_strScriptEngineId(other.m_strScriptEngineId)
{
}

ScriptInitializationException::~ScriptInitializationException() throw()
{
}

void ScriptInitializationException::Throw() const
{
    Base::Throw(GetHResult());
}

ScriptInitializationException& ScriptInitializationException::operator=(
    const ScriptInitializationException& other)
{
    if (this != &other)
    {
        StoryboardManagerException::operator=(other);
        m_strScriptPath = other.m_strScriptPath;
        m_strScriptEngineId = other.m_strScriptEngineId;
    }
    return *this;
}

ATL::CString ScriptInitializationException::GetScriptPath() const
{
    return m_strScriptPath;
}

void ScriptInitializationException::SetScriptPath(LPCWSTR pszScriptPath)
{
    m_strScriptPath = pszScriptPath ? pszScriptPath : L"";
}

ATL::CString ScriptInitializationException::GetScriptEngineId() const
{
    return m_strScriptEngineId;
}

void ScriptInitializationException::SetScriptEngineId(LPCWSTR pszEngineId)
{
    m_strScriptEngineId = pszEngineId ? pszEngineId : L"";
}

// ============================================================================
// EncodeInitializationException implementation
// ============================================================================

EncodeInitializationException::EncodeInitializationException()
    : StoryboardManagerException(E_FAIL)
    , m_dwProfileIndex(0)
{
}

EncodeInitializationException::EncodeInitializationException(HRESULT hr)
    : StoryboardManagerException(hr)
    , m_dwProfileIndex(0)
{
}

EncodeInitializationException::EncodeInitializationException(HRESULT hr, LPCWSTR pszMessage)
    : StoryboardManagerException(hr, pszMessage)
    , m_dwProfileIndex(0)
{
}

EncodeInitializationException::EncodeInitializationException(
    const EncodeInitializationException& other)
    : StoryboardManagerException(other)
    , m_strOutputFormat(other.m_strOutputFormat)
    , m_strCodecName(other.m_strCodecName)
    , m_dwProfileIndex(other.m_dwProfileIndex)
{
}

EncodeInitializationException::~EncodeInitializationException() throw()
{
}

void EncodeInitializationException::Throw() const
{
    Base::Throw(GetHResult());
}

EncodeInitializationException& EncodeInitializationException::operator=(
    const EncodeInitializationException& other)
{
    if (this != &other)
    {
        StoryboardManagerException::operator=(other);
        m_strOutputFormat = other.m_strOutputFormat;
        m_strCodecName = other.m_strCodecName;
        m_dwProfileIndex = other.m_dwProfileIndex;
    }
    return *this;
}

ATL::CString EncodeInitializationException::GetOutputFormat() const
{
    return m_strOutputFormat;
}

void EncodeInitializationException::SetOutputFormat(LPCWSTR pszFormat)
{
    m_strOutputFormat = pszFormat ? pszFormat : L"";
}

ATL::CString EncodeInitializationException::GetCodecName() const
{
    return m_strCodecName;
}

void EncodeInitializationException::SetCodecName(LPCWSTR pszCodecName)
{
    m_strCodecName = pszCodecName ? pszCodecName : L"";
}

DWORD EncodeInitializationException::GetProfileIndex() const throw()
{
    return m_dwProfileIndex;
}

void EncodeInitializationException::SetProfileIndex(DWORD dwIndex) throw()
{
    m_dwProfileIndex = dwIndex;
}

} // namespace StoryboardManager
