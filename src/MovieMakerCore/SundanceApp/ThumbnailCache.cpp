#include "pch.h"
#include "ThumbnailCache.h"

ThumbnailCache::ThumbnailCache() {}
ThumbnailCache::~ThumbnailCache() { Clear(); }

Gdiplus::Bitmap* ThumbnailCache::GetThumbnail(LPCWSTR pszPath)
{
    if (!pszPath || !pszPath[0])
        return NULL;

    std::wstring key(pszPath);
    std::map<std::wstring, Gdiplus::Bitmap*>::iterator it = m_cache.find(key);
    if (it != m_cache.end())
        return it->second;

    return NULL;
}

HRESULT ThumbnailCache::AddThumbnail(LPCWSTR pszPath, Gdiplus::Bitmap* pBitmap)
{
    if (!pszPath || !pszPath[0])
        return E_INVALIDARG;

    std::wstring key(pszPath);

    std::map<std::wstring, Gdiplus::Bitmap*>::iterator it = m_cache.find(key);
    if (it != m_cache.end())
    {
        delete it->second;
        it->second = pBitmap;
    }
    else
    {
        m_cache[key] = pBitmap;
    }

    return S_OK;
}

HRESULT ThumbnailCache::RemoveThumbnail(LPCWSTR pszPath)
{
    if (!pszPath || !pszPath[0])
        return E_INVALIDARG;

    std::wstring key(pszPath);
    std::map<std::wstring, Gdiplus::Bitmap*>::iterator it = m_cache.find(key);
    if (it == m_cache.end())
        return S_FALSE;

    delete it->second;
    m_cache.erase(it);
    return S_OK;
}

void ThumbnailCache::Clear()
{
    for (std::map<std::wstring, Gdiplus::Bitmap*>::iterator it = m_cache.begin();
         it != m_cache.end(); ++it)
    {
        delete it->second;
    }
    m_cache.clear();
}

void ThumbnailCache::InvalidateAll()
{
    Clear();
}

size_t ThumbnailCache::GetCount() const throw()
{
    return m_cache.size();
}
