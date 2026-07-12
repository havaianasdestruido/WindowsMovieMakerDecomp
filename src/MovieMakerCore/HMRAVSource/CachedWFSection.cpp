// CachedWFSection.cpp - Cached wave format section implementation

#include "pch.h"
#include "CachedWFSection.h"

namespace HMRAVSource
{

// ============================================================================
// Construction / Destruction
// ============================================================================

CachedWFSection::CachedWFSection()
{
}

CachedWFSection::~CachedWFSection()
{
    Clear();
}

// ============================================================================
// Cache management
// ============================================================================

HRESULT CachedWFSection::CacheWaveFormat(DWORD dwStreamIndex, IMFMediaType* pMediaType)
{
    if (!pMediaType)
        return E_POINTER;

    WAVEFORMATEX wfx = {};
    GUID subtype = GUID_NULL;

    HRESULT hr = pMediaType->GetGUID(MF_MT_SUBTYPE, &subtype);
    if (FAILED(hr))
        return hr;

    UINT32 cbFormat = 0;
    hr = pMediaType->GetBlobSize(MF_MT_USER_DATA, &cbFormat);

    // Try to extract WAVEFORMATEX from the media type
    UINT32 sampleRate = 0;
    pMediaType->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &sampleRate);
    wfx.nSamplesPerSec = sampleRate;

    UINT32 channels = 0;
    pMediaType->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &channels);
    wfx.nChannels = static_cast<WORD>(channels);

    UINT32 bitsPerSample = 0;
    pMediaType->GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, &bitsPerSample);
    wfx.wBitsPerSample = static_cast<WORD>(bitsPerSample);

    UINT32 blockAlign = 0;
    hr = pMediaType->GetUINT32(MF_MT_BLOCK_ALIGNMENT, &blockAlign);
    if (SUCCEEDED(hr))
        wfx.nBlockAlign = static_cast<WORD>(blockAlign);
    else
        wfx.nBlockAlign = wfx.nChannels * (wfx.wBitsPerSample / 8);

    UINT32 avgBytesPerSec = 0;
    hr = pMediaType->GetUINT32(MF_MT_AUDIO_AVG_BYTES_PER_SECTION, &avgBytesPerSec);
    if (SUCCEEDED(hr))
        wfx.nAvgBytesPerSec = avgBytesPerSec;
    else
        wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;

    wfx.wFormatTag = WAVE_FORMAT_PCM;

    return CacheWaveFormatDirect(dwStreamIndex, &wfx, subtype);
}

HRESULT CachedWFSection::CacheWaveFormatDirect(
    DWORD dwStreamIndex,
    const WAVEFORMATEX* pWfx,
    GUID subtype)
{
    if (!pWfx)
        return E_POINTER;

    // Check if already cached - update if so
    int nIndex = FindIndex(dwStreamIndex);
    if (nIndex >= 0)
    {
        m_arrEntries[nIndex].wfx = *pWfx;
        m_arrEntries[nIndex].subtype = subtype;
        m_arrEntries[nIndex].dwAverageBytesPerSecond = pWfx->nAvgBytesPerSec;
        m_arrEntries[nIndex].fValid = true;
        return S_OK;
    }

    // Add new entry
    CachedWFEntry entry;
    entry.dwStreamIndex = dwStreamIndex;
    entry.wfx = *pWfx;
    entry.subtype = subtype;
    entry.dwAverageBytesPerSecond = pWfx->nAvgBytesPerSec;
    entry.fValid = true;

    m_arrEntries.Add(entry);
    return S_OK;
}

// ============================================================================
// Lookup
// ============================================================================

bool CachedWFSection::FindByStreamIndex(DWORD dwStreamIndex, CachedWFEntry** ppEntry)
{
    if (!ppEntry)
        return false;

    int nIndex = FindIndex(dwStreamIndex);
    if (nIndex < 0)
        return false;

    *ppEntry = &m_arrEntries[nIndex];
    return true;
}

bool CachedWFSection::FindByStreamIndex(DWORD dwStreamIndex, const CachedWFEntry** ppEntry) const
{
    if (!ppEntry)
        return false;

    int nIndex = FindIndex(dwStreamIndex);
    if (nIndex < 0)
        return false;

    *ppEntry = &m_arrEntries[nIndex];
    return true;
}

// ============================================================================
// Enumeration
// ============================================================================

DWORD CachedWFSection::GetCount() const throw()
{
    return static_cast<DWORD>(m_arrEntries.GetCount());
}

bool CachedWFSection::GetAt(DWORD nIndex, CachedWFEntry** ppEntry)
{
    if (!ppEntry || nIndex >= m_arrEntries.GetCount())
        return false;

    *ppEntry = &m_arrEntries[nIndex];
    return true;
}

// ============================================================================
// Removal
// ============================================================================

void CachedWFSection::Remove(DWORD dwStreamIndex)
{
    int nIndex = FindIndex(dwStreamIndex);
    if (nIndex >= 0)
        m_arrEntries.RemoveAt(nIndex);
}

void CachedWFSection::Clear()
{
    m_arrEntries.RemoveAll();
}

// ============================================================================
// Validation
// ============================================================================

bool CachedWFSection::IsCached(DWORD dwStreamIndex) const throw()
{
    return FindIndex(dwStreamIndex) >= 0;
}

// ============================================================================
// Utility
// ============================================================================

HRESULT CachedWFSection::GetDefaultFormat(WAVEFORMATEX* pWfx) const
{
    if (!pWfx)
        return E_POINTER;

    if (m_arrEntries.GetCount() == 0)
    {
        // Return a sensible default
        ZeroMemory(pWfx, sizeof(WAVEFORMATEX));
        pWfx->wFormatTag = WAVE_FORMAT_PCM;
        pWfx->nChannels = 2;
        pWfx->nSamplesPerSec = 44100;
        pWfx->wBitsPerSample = 16;
        pWfx->nBlockAlign = 4;
        pWfx->nAvgBytesPerSec = 176400;
        return S_OK;
    }

    // Return the first cached format
    *pWfx = m_arrEntries[0].wfx;
    return S_OK;
}

bool CachedWFSection::HasAnyEntry() const throw()
{
    return m_arrEntries.GetCount() > 0;
}

// ============================================================================
// Private helpers
// ============================================================================

int CachedWFSection::FindIndex(DWORD dwStreamIndex) const
{
    for (size_t i = 0; i < m_arrEntries.GetCount(); ++i)
    {
        if (m_arrEntries[i].dwStreamIndex == dwStreamIndex)
            return static_cast<int>(i);
    }
    return -1;
}

} // namespace HMRAVSource
