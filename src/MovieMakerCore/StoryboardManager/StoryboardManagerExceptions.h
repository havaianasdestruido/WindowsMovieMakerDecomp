/*
 * StoryboardManagerExceptions.h
 *
 * Exception classes for the StoryboardManager namespace, derived from
 * Base::Exception for consistent error handling across MovieMakerCore.dll.
 *
 * RTTI classes from analysis:
 *   - TemplateInitializationException
 *   - ScriptInitializationException
 *   - EncodeInitializationException
 *
 * These exceptions are thrown when initialization of their respective
 * subsystems fails. They carry an HRESULT and an optional description
 * string for diagnostic purposes.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef STORYBOARDMANAGEREXCEPTIONS_H
#define STORYBOARDMANAGEREXCEPTIONS_H

#include "StoryboardManager.h"

namespace StoryboardManager
{

// ============================================================================
// StoryboardManagerException
// ============================================================================
// Base exception class for StoryboardManager errors. Extends Base::Exception
// with an optional descriptive message string.
//
class STORYBOARD_API StoryboardManagerException : public Base::Exception
{
public:
    StoryboardManagerException(HRESULT hr);
    StoryboardManagerException(HRESULT hr, LPCWSTR pszMessage);
    StoryboardManagerException(HRESULT hr, const ATL::CString& strMessage);
    StoryboardManagerException(const StoryboardManagerException& other);
    virtual ~StoryboardManagerException() throw();

    // Exception message
    ATL::CString GetMessage() const;

    // Re-throw
    virtual void Throw() const;

    StoryboardManagerException& operator=(const StoryboardManagerException& other);

private:
    ATL::CString m_strMessage;
};

// ============================================================================
// TemplateInitializationException
// ============================================================================
// Thrown when the template/theme subsystem fails to initialize. This can
// occur if the template table cannot be populated with built-in themes,
// or if a required template resource is missing.
//
// RTTI class: TemplateInitializationException
//
class STORYBOARD_API TemplateInitializationException : public StoryboardManagerException
{
public:
    TemplateInitializationException();
    TemplateInitializationException(HRESULT hr);
    TemplateInitializationException(HRESULT hr, LPCWSTR pszMessage);
    TemplateInitializationException(const TemplateInitializationException& other);
    virtual ~TemplateInitializationException() throw();

    virtual void Throw() const;

    TemplateInitializationException& operator=(const TemplateInitializationException& other);

    // Template-specific diagnostic info
    ATL::CString GetTemplateName() const;
    void SetTemplateName(LPCWSTR pszTemplateName);

private:
    ATL::CString m_strTemplateName;
};

// ============================================================================
// ScriptInitializationException
// ============================================================================
// Thrown when the scripting/automation subsystem fails to initialize. This
// covers the WLMP script engine used for AutoMovie generation and custom
// title/credits rendering.
//
// RTTI class: ScriptInitializationException
//
class STORYBOARD_API ScriptInitializationException : public StoryboardManagerException
{
public:
    ScriptInitializationException();
    ScriptInitializationException(HRESULT hr);
    ScriptInitializationException(HRESULT hr, LPCWSTR pszMessage);
    ScriptInitializationException(const ScriptInitializationException& other);
    virtual ~ScriptInitializationException() throw();

    virtual void Throw() const;

    ScriptInitializationException& operator=(const ScriptInitializationException& other);

    // Script-specific diagnostic info
    ATL::CString GetScriptPath() const;
    void SetScriptPath(LPCWSTR pszScriptPath);

    ATL::CString GetScriptEngineId() const;
    void SetScriptEngineId(LPCWSTR pszEngineId);

private:
    ATL::CString m_strScriptPath;
    ATL::CString m_strScriptEngineId;
};

// ============================================================================
// EncodeInitializationException
// ============================================================================
// Thrown when the encoding/transcoding subsystem fails to initialize. This
// covers Media Foundation encoder setup, codec enumeration failures, and
// output profile configuration errors.
//
// RTTI class: EncodeInitializationException
//
class STORYBOARD_API EncodeInitializationException : public StoryboardManagerException
{
public:
    EncodeInitializationException();
    EncodeInitializationException(HRESULT hr);
    EncodeInitializationException(HRESULT hr, LPCWSTR pszMessage);
    EncodeInitializationException(const EncodeInitializationException& other);
    virtual ~EncodeInitializationException() throw();

    virtual void Throw() const;

    EncodeInitializationException& operator=(const EncodeInitializationException& other);

    // Encoding-specific diagnostic info
    ATL::CString GetOutputFormat() const;
    void SetOutputFormat(LPCWSTR pszFormat);

    ATL::CString GetCodecName() const;
    void SetCodecName(LPCWSTR pszCodecName);

    DWORD GetProfileIndex() const throw();
    void SetProfileIndex(DWORD dwIndex) throw();

private:
    ATL::CString m_strOutputFormat;
    ATL::CString m_strCodecName;
    DWORD        m_dwProfileIndex;
};

} // namespace StoryboardManager

#endif // STORYBOARDMANAGEREXCEPTIONS_H
