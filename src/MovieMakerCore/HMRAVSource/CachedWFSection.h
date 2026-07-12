/*
 * CachedWFSection.h
 *
 * CachedWFSection - Cached wave format section. Caches WAVEFORMATEX
 * structures and related media type information for audio streams,
 * avoiding repeated queries from the source reader.
 *
 * RTTI: ?AVCachedWFSection@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef CACHEDWFSECTION_H
#define CACHEDWFSECTION_H

#include "HMRAVSource.h"

namespace HMRAVSource
{

// ============================================================================
// CachedWFEntry
// ============================================================================
// A single cached wave format entry, associating a stream index with its
// WAVEFORMATEX and derived values.
//
struct CachedWFEntry
{
    DWORD           dwStreamIndex = 0;
    WAVEFORMATEX    wfx = {};
    GUID            subtype = GUID_NULL;
    DWORD           dwAverageBytesPerSecond = 0;
    bool            fValid = false;
};

// ============================================================================
// CachedWFSection
// ============================================================================
// Caches WAVEFORMATEX information for audio streams to avoid repeated
// Media Foundation type queries. Stores format data per stream index
// and provides fast lookup.
//
// From RTTI: ?AVCachedWFSection@@
//
class AVSOURCE_API CachedWFSection
{
public:
    CachedWFSection();
    ~CachedWFSection();

    // Cache management
    HRESULT CacheWaveFormat(DWORD dwStreamIndex, IMFMediaType* pMediaType);
    HRESULT CacheWaveFormatDirect(DWORD dwStreamIndex, const WAVEFORMATEX* pWfx, GUID subtype);

    // Lookup
    bool FindByStreamIndex(DWORD dwStreamIndex, CachedWFEntry** ppEntry);
    bool FindByStreamIndex(DWORD dwStreamIndex, const CachedWFEntry** ppEntry) const;

    // Enumeration
    DWORD GetCount() const throw();
    bool GetAt(DWORD nIndex, CachedWFEntry** ppEntry);

    // Removal
    void Remove(DWORD dwStreamIndex);
    void Clear();

    // Validation
    bool IsCached(DWORD dwStreamIndex) const throw();

    // Utility
    HRESULT GetDefaultFormat(WAVEFORMATEX* pWfx) const;
    bool HasAnyEntry() const throw();

private:
    ATL::CAtlArray<CachedWFEntry> m_arrEntries;

    int FindIndex(DWORD dwStreamIndex) const;
};

} // namespace HMRAVSource

#endif // CACHEDWFSECTION_H
