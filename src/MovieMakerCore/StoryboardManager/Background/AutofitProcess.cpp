/*
 * AutofitProcess.cpp
 *
 * Implementation of AutofitProcess and AreaOfInterestGenerator.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "AutofitProcess.h"
#include <cmath>

// ============================================================================
// AutofitProcess implementation
// ============================================================================

AutofitProcess::AutofitProcess()
    : m_dwPhotoCount(0)
    , m_llTotalDurationHns(0)
    , m_llMinPhotoDurationHns(20000000)    // 2 seconds default
    , m_llMaxPhotoDurationHns(120000000)   // 12 seconds default
{
}

AutofitProcess::~AutofitProcess()
{
}

void AutofitProcess::SetMusicFilePath(LPCWSTR pszPath)
{
    m_strMusicFilePath = pszPath ? pszPath : L"";
}

ATL::CString AutofitProcess::GetMusicFilePath() const
{
    return m_strMusicFilePath;
}

void AutofitProcess::SetPhotoCount(DWORD dwCount)
{
    m_dwPhotoCount = dwCount;
}

DWORD AutofitProcess::GetPhotoCount() const throw()
{
    return m_dwPhotoCount;
}

void AutofitProcess::SetTotalDurationHns(LONGLONG llDuration)
{
    m_llTotalDurationHns = llDuration;
}

LONGLONG AutofitProcess::GetTotalDurationHns() const throw()
{
    return m_llTotalDurationHns;
}

void AutofitProcess::SetMinPhotoDurationHns(LONGLONG llDuration)
{
    m_llMinPhotoDurationHns = llDuration;
}

LONGLONG AutofitProcess::GetMinPhotoDurationHns() const throw()
{
    return m_llMinPhotoDurationHns;
}

void AutofitProcess::SetMaxPhotoDurationHns(LONGLONG llDuration)
{
    m_llMaxPhotoDurationHns = llDuration;
}

LONGLONG AutofitProcess::GetMaxPhotoDurationHns() const throw()
{
    return m_llMaxPhotoDurationHns;
}

HRESULT AutofitProcess::Analyze()
{
    m_vPhotoDurations.clear();
    m_vBeatPositionsMs.clear();

    if (m_dwPhotoCount == 0)
        return S_FALSE;

    // Analyze music for beat detection
    HRESULT hr = AnalyzeAudio();
    if (FAILED(hr))
        return hr;

    // Distribute total duration across photos
    if (m_llTotalDurationHns > 0)
    {
        LONGLONG llPerPhoto = m_llTotalDurationHns / m_dwPhotoCount;

        // Clamp to min/max bounds
        llPerPhoto = max(m_llMinPhotoDurationHns, llPerPhoto);
        llPerPhoto = min(m_llMaxPhotoDurationHns, llPerPhoto);

        for (DWORD i = 0; i < m_dwPhotoCount; i++)
        {
            m_vPhotoDurations.push_back(llPerPhoto);
        }
    }
    else
    {
        // No total duration specified, use default per-photo duration
        for (DWORD i = 0; i < m_dwPhotoCount; i++)
        {
            m_vPhotoDurations.push_back(m_llMinPhotoDurationHns);
        }
    }

    return S_OK;
}

HRESULT AutofitProcess::ApplyToProject()
{
    // In the full implementation, this would iterate through the photo extents
    // on the timeline and adjust their end time to match the computed durations.
    return S_OK;
}

size_t AutofitProcess::GetDurationCount() const throw()
{
    return m_vPhotoDurations.size();
}

LONGLONG AutofitProcess::GetPhotoDurationAt(size_t nIndex) const
{
    if (nIndex < m_vPhotoDurations.size())
        return m_vPhotoDurations[nIndex];
    return 0;
}

bool AutofitProcess::HasBeatData() const throw()
{
    return !m_vBeatPositionsMs.empty();
}

DWORD AutofitProcess::GetBeatCount() const throw()
{
    return static_cast<DWORD>(m_vBeatPositionsMs.size());
}

LONGLONG AutofitProcess::GetBeatPositionMs(DWORD dwIndex) const throw()
{
    if (dwIndex < m_vBeatPositionsMs.size())
        return m_vBeatPositionsMs[dwIndex];
    return 0;
}

HRESULT AutofitProcess::AnalyzeAudio()
{
    if (m_strMusicFilePath.IsEmpty())
        return S_FALSE;

    // In the full implementation, this would:
    //  1. Open the audio file using MFSourceReader
    //  2. Read PCM audio data
    //  3. Apply beat detection algorithm (onset detection, FFT-based)
    //  4. Store beat positions in m_vBeatPositionsMs
    //  5. Report progress via callback

    return S_OK;
}

// ============================================================================
// AreaOfInterestGenerator implementation
// ============================================================================

AreaOfInterestGenerator::AreaOfInterestGenerator()
    : m_dwMaxRegions(5)
    , m_flMinRegionWeight(0.1f)
    , m_flAspectRatio(16.0f / 9.0f)
{
}

AreaOfInterestGenerator::~AreaOfInterestGenerator()
{
}

HRESULT AreaOfInterestGenerator::Generate(Gdiplus::Bitmap* pBitmap, std::vector<AreaOfInterest>& vResults)
{
    vResults.clear();

    if (!pBitmap)
        return E_POINTER;

    INT nWidth = pBitmap->GetWidth();
    INT nHeight = pBitmap->GetHeight();

    if (nWidth == 0 || nHeight == 0)
        return E_INVALIDARG;

    // Lock bitmap bits for pixel access
    Gdiplus::Rect rcLock(0, 0, nWidth, nHeight);
    Gdiplus::BitmapData bmpData;

    Gdiplus::Status status = pBitmap->LockBits(
        &rcLock,
        Gdiplus::ImageLockModeRead,
        PixelFormat32bppARGB,
        &bmpData);

    if (status != Gdiplus::Ok)
        return E_FAIL;

    const BYTE* pPixels = static_cast<const BYTE*>(bmpData.Scan0);

    // Analyze saliency
    std::vector<float> vSaliency(static_cast<size_t>(nWidth) * nHeight, 0.0f);
    HRESULT hr = AnalyzeSaliency(nWidth, nHeight, pPixels, vSaliency);

    pBitmap->UnlockBits(&bmpData);

    if (FAILED(hr))
        return hr;

    // Extract regions from saliency map
    hr = ExtractRegions(vSaliency, nWidth, nHeight, vResults);
    if (FAILED(hr))
        return hr;

    // If no regions found, add default center region
    if (vResults.empty())
    {
        AreaOfInterest aoi;
        aoi.rcBounds.left = nWidth / 4;
        aoi.rcBounds.top = nHeight / 4;
        aoi.rcBounds.right = nWidth * 3 / 4;
        aoi.rcBounds.bottom = nHeight * 3 / 4;
        aoi.flWeight = 1.0f;
        aoi.dwType = 2; // center
        aoi.bPrimary = true;
        vResults.push_back(aoi);
    }

    return S_OK;
}

void AreaOfInterestGenerator::SetMaxRegions(DWORD dwMaxRegions)
{
    m_dwMaxRegions = dwMaxRegions;
}

DWORD AreaOfInterestGenerator::GetMaxRegions() const throw()
{
    return m_dwMaxRegions;
}

void AreaOfInterestGenerator::SetMinRegionWeight(float flWeight)
{
    m_flMinRegionWeight = max(0.0f, flWeight);
}

float AreaOfInterestGenerator::GetMinRegionWeight() const throw()
{
    return m_flMinRegionWeight;
}

void AreaOfInterestGenerator::SetAspectRatio(float flAspectRatio)
{
    m_flAspectRatio = flAspectRatio;
}

float AreaOfInterestGenerator::GetAspectRatio() const throw()
{
    return m_flAspectRatio;
}

HRESULT AreaOfInterestGenerator::ComputeCenterOfInterest(Gdiplus::Bitmap* pBitmap, POINT* ptCenter)
{
    if (!pBitmap || !ptCenter)
        return E_POINTER;

    ptCenter->x = pBitmap->GetWidth() / 2;
    ptCenter->y = pBitmap->GetHeight() / 2;
    return S_OK;
}

HRESULT AreaOfInterestGenerator::ComputeFaceRectangles(Gdiplus::Bitmap* pBitmap, std::vector<RECT>& vFaces)
{
    UNREFERENCED_PARAMETER(pBitmap);
    UNREFERENCED_PARAMETER(vFaces);
    // Face detection via Windows Imaging Component or DirectX
    // In the full implementation, this would use an external face detection
    // library or WIC bitmap analysis. For now, return no faces.
    return S_OK;
}

HRESULT AreaOfInterestGenerator::AnalyzeSaliency(
    INT nWidth, INT nHeight, const BYTE* pPixels, std::vector<float>& vSaliency)
{
    UNREFERENCED_PARAMETER(nWidth);
    UNREFERENCED_PARAMETER(nHeight);
    UNREFERENCED_PARAMETER(pPixels);
    UNREFERENCED_PARAMETER(vSaliency);

    // In the full implementation, this would apply a frequency-tuned
    // saliency detection algorithm (Achanta et al. 2009) using per-pixel
    // color contrast against the mean image color.

    return S_OK;
}

HRESULT AreaOfInterestGenerator::ExtractRegions(
    const std::vector<float>& vSaliency, INT nWidth, INT nHeight,
    std::vector<AreaOfInterest>& vRegions)
{
    UNREFERENCED_PARAMETER(vSaliency);
    UNREFERENCED_PARAMETER(nWidth);
    UNREFERENCED_PARAMETER(nHeight);
    UNREFERENCED_PARAMETER(vRegions);

    // In the full implementation, this would:
    //  1. Threshold the saliency map
    //  2. Find connected components
    //  3. Compute bounding rectangles
    //  4. Filter by weight and count

    return S_OK;
}
