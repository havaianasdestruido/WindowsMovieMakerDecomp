/*
 * SelectorExtentRefContainer.h
 *
 * Extent reference containers for selectors. Manages the mapping between
 * template selectors and the extents they apply to, supporting lookup,
 * validation, and iteration over extent-selector bindings.
 *
 * RTTI: ?AVSelectorExtentRefContainer@@, ?AVSelectorFinder@@, ?AVExtentSelectorValidator@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef SELECTOREXTENTREFCONTAINER_H
#define SELECTOREXTENTREFCONTAINER_H

#include "../StoryboardManager.h"

namespace StoryboardManager
{

// ============================================================================
// SelectorExtentRef
// ============================================================================
// A single binding between a template selector name and an extent ID.
//
struct SelectorExtentRef
{
    ATL::CString strSelectorName;
    DWORD        dwExtentId;

    SelectorExtentRef()
        : dwExtentId(0)
    {
    }

    SelectorExtentRef(LPCWSTR pszName, DWORD dwId)
        : strSelectorName(pszName ? pszName : L"")
        , dwExtentId(dwId)
    {
    }
};

// ============================================================================
// SelectorExtentRefContainer
// ============================================================================
// Container managing the set of selector-to-extent bindings for a project.
// Provides lookup by selector name, extent ID, or both.
//
class STORYBOARD_API SelectorExtentRefContainer
{
public:
    SelectorExtentRefContainer();
    ~SelectorExtentRefContainer();

    // -- Copy semantics --
    SelectorExtentRefContainer(const SelectorExtentRefContainer& other);
    SelectorExtentRefContainer& operator=(const SelectorExtentRefContainer& other);

    // -- Add binding --
    void AddRef(LPCWSTR pszSelectorName, DWORD dwExtentId);
    void AddRef(const SelectorExtentRef& ref);

    // -- Remove binding --
    void RemoveRef(LPCWSTR pszSelectorName, DWORD dwExtentId);
    void RemoveAllRefs();
    void RemoveRefsForExtent(DWORD dwExtentId);
    void RemoveRefsForSelector(LPCWSTR pszSelectorName);

    // -- Lookup --
    bool Contains(LPCWSTR pszSelectorName, DWORD dwExtentId) const;
    bool HasSelector(LPCWSTR pszSelectorName) const;
    bool HasExtent(DWORD dwExtentId) const;

    // -- Count --
    size_t GetCount() const throw();
    size_t GetCountForSelector(LPCWSTR pszSelectorName) const;
    size_t GetCountForExtent(DWORD dwExtentId) const;

    // -- Access --
    const SelectorExtentRef& GetAt(size_t nIndex) const;

    // -- Find --
    int Find(LPCWSTR pszSelectorName, DWORD dwExtentId) const;
    bool FindFirstBySelector(LPCWSTR pszSelectorName, SelectorExtentRef* pRef) const;
    bool FindFirstByExtent(DWORD dwExtentId, SelectorExtentRef* pRef) const;

private:
    ATL::CAtlArray<SelectorExtentRef> m_arrRefs;
};

// ============================================================================
// SelectorFinder
// ============================================================================
// Utility class for searching for selectors matching specific criteria
// (name pattern, extent membership, etc.).
//
class STORYBOARD_API SelectorFinder
{
public:
    SelectorFinder();
    ~SelectorFinder();

    // -- Search by name pattern --
    void FindByNamePattern(LPCWSTR pszPattern,
                           const SelectorExtentRefContainer& container,
                           SelectorExtentRefContainer& results);

    // -- Search by extent ID range --
    void FindByExtentRange(DWORD dwMinExtentId, DWORD dwMaxExtentId,
                           const SelectorExtentRefContainer& container,
                           SelectorExtentRefContainer& results);

    // -- Get unique selector names --
    void GetUniqueSelectors(const SelectorExtentRefContainer& container,
                            ATL::CAtlArray<ATL::CString>& arrNames);
};

// ============================================================================
// ExtentSelectorValidator
// ============================================================================
// Validates that extent-selector bindings are consistent with the project
// model (i.e., all referenced extents exist, selectors are defined).
//
class STORYBOARD_API ExtentSelectorValidator
{
public:
    ExtentSelectorValidator();
    ~ExtentSelectorValidator();

    // -- Validation --
    bool Validate(const SelectorExtentRefContainer& container,
                  const ExtentCollection& extents);

    // -- Error info --
    HRESULT GetLastHResult() const throw();
    ATL::CString GetErrorDetails() const;

    // -- Get invalid refs --
    size_t GetInvalidRefCount() const throw();
    const SelectorExtentRef& GetInvalidRefAt(size_t nIndex) const;

private:
    HRESULT    m_hrLast;
    ATL::CString m_strErrorDetails;
    ATL::CAtlArray<SelectorExtentRef> m_arrInvalidRefs;
};

} // namespace StoryboardManager

#endif // SELECTOREXTENTREFCONTAINER_H
