#include "pch.h"
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
        llPerPhoto = std::max(m_llMinPhotoDurationHns, llPerPhoto);
        llPerPhoto = std::min(m_llMaxPhotoDurationHns, llPerPhoto);

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
    if (m_vPhotoDurations.empty())
        return E_UNEXPECTED;

    if (m_dwPhotoCount == 0)
        return E_INVALIDARG;

    size_t cDurations = m_vPhotoDurations.size();
    LONGLONG llTotalApplied = 0;

    for (size_t i = 0; i < cDurations; i++)
    {
        LONGLONG llDuration = m_vPhotoDurations[i];

        if (llDuration < m_llMinPhotoDurationHns)
            llDuration = m_llMinPhotoDurationHns;
        if (llDuration > m_llMaxPhotoDurationHns)
            llDuration = m_llMaxPhotoDurationHns;

        llTotalApplied += llDuration;
    }

    UNREFERENCED_PARAMETER(llTotalApplied);

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

    HRESULT hr = MFStartup(MF_VERSION);
    if (FAILED(hr))
        return hr;

    CComPtr<IMFAttributes> spAttributes;
    hr = MFCreateAttributes(&spAttributes, 2);
    if (FAILED(hr))
    {
        MFShutdown();
        return hr;
    }

    CComPtr<IMFSourceReader> spReader;
    hr = MFCreateSourceReaderFromURL(m_strMusicFilePath, spAttributes, &spReader);
    if (FAILED(hr))
    {
        MFShutdown();
        return hr;
    }

    CComPtr<IMFMediaType> spAudioType;
    hr = spReader->GetNativeMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, &spAudioType);
    if (FAILED(hr))
    {
        MFShutdown();
        return hr;
    }

    UINT32 unSampleRate = 44100;
    UINT32 unChannels = 2;
    spAudioType->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &unSampleRate);
    spAudioType->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &unChannels);

    DWORD dwBlockSize = 4096;
    DWORD dwHopSize = dwBlockSize / 2;
    std::vector<float> vPrevEnergy(dwBlockSize / 2, 0.0f);
    std::vector<float> vCurrentEnergy(dwBlockSize / 2, 0.0f);

    float flThreshold = 0.0f;
    std::vector<float> vEnergyHistory;

    LONGLONG llDurationHns = 0;
    {
        PROPVARIANT var;
        PropVariantInit(&var);
        hr = spReader->GetPresentationAttribute(
            MF_SOURCE_READER_MEDIASOURCE, MF_PD_DURATION, &var);
        if (SUCCEEDED(hr))
        {
            llDurationHns = static_cast<LONGLONG>(var.uhVal.QuadPart);
            PropVariantClear(&var);
        }
    }

    DWORD dwSamplesPerBlock = dwBlockSize * unChannels;
    std::vector<BYTE> audioBuffer(dwSamplesPerBlock * sizeof(float));
    DWORD dwBlocksRead = 0;
    bool fDone = false;

    m_vBeatPositionsMs.clear();

    while (!fDone)
    {
        DWORD dwStreamIndex = 0;
        DWORD dwStreamFlags = 0;
        LONGLONG llTimestamp = 0;
        CComPtr<IMFSample> spSample;

        hr = spReader->ReadSample(
            MF_SOURCE_READER_FIRST_AUDIO_STREAM,
            0, &dwStreamIndex, &dwStreamFlags, &llTimestamp, &spSample);

        if (FAILED(hr) || (dwStreamFlags & MF_SOURCE_READERF_ENDOFSTREAM))
            break;

        if (!spSample)
            continue;

        CComPtr<IMFMediaBuffer> spBuffer;
        hr = spSample->ConvertToContiguousBuffer(&spBuffer);
        if (FAILED(hr))
            continue;

        BYTE* pbData = NULL;
        DWORD cbData = 0;
        hr = spBuffer->Lock(&pbData, NULL, &cbData);
        if (FAILED(hr))
            continue;

        DWORD dwSamplesInBuffer = cbData / (unChannels * sizeof(float));

        float flEnergy = 0.0f;
        DWORD dwSamplesToProcess = std::min(dwSamplesInBuffer, dwBlockSize * unChannels);
        const float* pSamples = reinterpret_cast<const float*>(pbData);

        for (DWORD s = 0; s < dwSamplesToProcess; s++)
        {
            flEnergy += pSamples[s] * pSamples[s];
        }

        if (dwSamplesToProcess > 0)
            flEnergy /= static_cast<float>(dwSamplesToProcess);

        spBuffer->Unlock();

        flEnergy = sqrtf(flEnergy);
        vEnergyHistory.push_back(flEnergy);

        dwBlocksRead++;
    }

    if (vEnergyHistory.size() > 2)
    {
        float flSum = 0.0f;
        for (float e : vEnergyHistory)
            flSum += e;
        float flMean = flSum / static_cast<float>(vEnergyHistory.size());

        float flVariance = 0.0f;
        for (float e : vEnergyHistory)
        {
            float flDiff = e - flMean;
            flVariance += flDiff * flDiff;
        }
        flVariance /= static_cast<float>(vEnergyHistory.size());
        float flStdDev = sqrtf(flVariance);

        flThreshold = flMean + flStdDev * 0.75f;

        DWORD dwBlockDurationMs = (dwBlockSize * 1000) /
                                  (unSampleRate > 0 ? unSampleRate : 44100);

        LONGLONG llLastBeatMs = -1000;
        LONGLONG llMinInterBeatMs = 250;

        for (size_t i = 1; i < vEnergyHistory.size(); i++)
        {
            float flDelta = vEnergyHistory[i] - vEnergyHistory[i - 1];

            if (flDelta > flThreshold && flVariance > 0.0f)
            {
                LONGLONG llBeatMs = static_cast<LONGLONG>(i) * dwBlockDurationMs;

                if (llBeatMs - llLastBeatMs >= llMinInterBeatMs)
                {
                    m_vBeatPositionsMs.push_back(llBeatMs);
                    llLastBeatMs = llBeatMs;
                }
            }
        }

        if (m_vBeatPositionsMs.empty() && vEnergyHistory.size() > 1)
        {
            LONGLONG llTotalMs = llDurationHns / 10000;
            if (llTotalMs <= 0)
                llTotalMs = 60000;

            LONGLONG llDefaultBeatMs = 500;
            for (LONGLONG t = llDefaultBeatMs; t < llTotalMs; t += llDefaultBeatMs)
            {
                m_vBeatPositionsMs.push_back(t);
            }
        }
    }

    MFShutdown();
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
    m_flMinRegionWeight = std::max(0.0f, flWeight);
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
    if (!pBitmap)
        return E_POINTER;

    vFaces.clear();

    INT nWidth = pBitmap->GetWidth();
    INT nHeight = pBitmap->GetHeight();

    if (nWidth < 20 || nHeight < 20)
        return E_INVALIDARG;

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
    INT nStride = bmpData.Stride;

    DWORD dwBlockSize = 20;
    DWORD dwGridW = nWidth / dwBlockSize;
    DWORD dwGridH = nHeight / dwBlockSize;

    std::vector<float> vSkinScore(static_cast<size_t>(dwGridW) * dwGridH, 0.0f);

    for (DWORD gy = 0; gy < dwGridH; gy++)
    {
        for (DWORD gx = 0; gx < dwGridW; gx++)
        {
            float flTotalScore = 0.0f;
            DWORD dwPixelCount = 0;

            for (DWORD py = 0; py < dwBlockSize; py++)
            {
                for (DWORD px = 0; px < dwBlockSize; px++)
                {
                    INT x = gx * dwBlockSize + px;
                    INT y = gy * dwBlockSize + py;
                    if (x >= nWidth || y >= nHeight)
                        continue;

                    const BYTE* pPixel = pPixels + y * nStride + x * 4;
                    BYTE b = pPixel[0];
                    BYTE g = pPixel[1];
                    BYTE r = pPixel[2];

                    float flR = static_cast<float>(r) / 255.0f;
                    float flG = static_cast<float>(g) / 255.0f;
                    float flB = static_cast<float>(b) / 255.0f;

                    float flMax = (flR > flG) ? flR : flG;
                    flMax = (flMax > flB) ? flMax : flB;
                    float flMin = (flR < flG) ? flR : flG;
                    flMin = (flMin < flB) ? flMin : flB;

                    float flSaturation = (flMax > 0.001f) ? ((flMax - flMin) / flMax) : 0.0f;
                    float flValue = flMax;

                    bool fSkinLike = (r > 60 && g > 40 && b > 20) &&
                                     (r > g) && (r > b) &&
                                     (flSaturation > 0.1f && flSaturation < 0.7f) &&
                                     (flValue > 0.2f);

                    if (fSkinLike)
                        flTotalScore += 1.0f;

                    dwPixelCount++;
                }
            }

            float flScore = (dwPixelCount > 0) ?
                flTotalScore / static_cast<float>(dwPixelCount) : 0.0f;

            vSkinScore[static_cast<size_t>(gy) * dwGridW + gx] = flScore;
        }
    }

    std::vector<bool> vVisited(dwGridW * dwGridH, false);

    for (DWORD gy = 0; gy < dwGridH; gy++)
    {
        for (DWORD gx = 0; gx < dwGridW; gx++)
        {
            size_t idx = static_cast<size_t>(gy) * dwGridW + gx;
            if (vVisited[idx] || vSkinScore[idx] < 0.3f)
                continue;

            DWORD dwMinX = gx, dwMinY = gy;
            DWORD dwMaxX = gx, dwMaxY = gy;
            float flSumScore = 0.0f;
            DWORD dwRegionPixels = 0;

            std::vector<std::pair<DWORD, DWORD>> stack;
            stack.push_back(std::make_pair(gx, gy));
            vVisited[idx] = true;

            while (!stack.empty())
            {
                auto cur = stack.back();
                stack.pop_back();

                DWORD cx = cur.first;
                DWORD cy = cur.second;
                size_t ci = static_cast<size_t>(cy) * dwGridW + cx;

                flSumScore += vSkinScore[ci];
                dwRegionPixels++;

                if (cx < dwMinX) dwMinX = cx;
                if (cy < dwMinY) dwMinY = cy;
                if (cx > dwMaxX) dwMaxX = cx;
                if (cy > dwMaxY) dwMaxY = cy;

                const int dx[] = { -1, 1, 0, 0 };
                const int dy[] = { 0, 0, -1, 1 };

                for (int d = 0; d < 4; d++)
                {
                    int nx = static_cast<int>(cx) + dx[d];
                    int ny = static_cast<int>(cy) + dy[d];
                    if (nx >= 0 && ny >= 0 &&
                        nx < static_cast<int>(dwGridW) &&
                        ny < static_cast<int>(dwGridH))
                    {
                        size_t ni = static_cast<size_t>(ny) * dwGridW + nx;
                        if (!vVisited[ni] && vSkinScore[ni] >= 0.3f)
                        {
                            vVisited[ni] = true;
                            stack.push_back(std::make_pair(static_cast<DWORD>(nx), static_cast<DWORD>(ny)));
                        }
                    }
                }
            }

            if (dwRegionPixels >= 2)
            {
                RECT rc;
                rc.left = static_cast<LONG>(dwMinX * dwBlockSize);
                rc.top = static_cast<LONG>(dwMinY * dwBlockSize);
                rc.right = static_cast<LONG>((dwMaxX + 1) * dwBlockSize);
                rc.bottom = static_cast<LONG>((dwMaxY + 1) * dwBlockSize);

                if (rc.right > nWidth) rc.right = nWidth;
                if (rc.bottom > nHeight) rc.bottom = nHeight;

                vFaces.push_back(rc);
            }
        }
    }

    pBitmap->UnlockBits(&bmpData);
    return S_OK;
}

HRESULT AreaOfInterestGenerator::AnalyzeSaliency(
    INT nWidth, INT nHeight, const BYTE* pPixels, std::vector<float>& vSaliency)
{
    if (!pPixels || nWidth <= 0 || nHeight <= 0)
        return E_INVALIDARG;

    size_t cPixels = static_cast<size_t>(nWidth) * nHeight;
    vSaliency.resize(cPixels, 0.0f);

    double dblMeanR = 0.0, dblMeanG = 0.0, dblMeanB = 0.0;
    double dblMeanLabA = 0.0, dblMeanLabB = 0.0;

    for (INT y = 0; y < nHeight; y++)
    {
        const BYTE* pRow = pPixels + y * nWidth * 4;
        for (INT x = 0; x < nWidth; x++)
        {
            BYTE b = pRow[x * 4 + 0];
            BYTE g = pRow[x * 4 + 1];
            BYTE r = pRow[x * 4 + 2];

            double lr = static_cast<double>(r) / 255.0;
            double lg = static_cast<double>(g) / 255.0;
            double lb = static_cast<double>(b) / 255.0;

            double l = 0.4124564 * lr + 0.3575761 * lg + 0.1804375 * lb;
            double m = 0.2126729 * lr + 0.7151522 * lg + 0.0721750 * lb;
            double s = 0.0193339 * lr + 0.1191920 * lg + 0.9503041 * lb;

            l = (l > 0.008856) ? pow(l, 1.0 / 3.0) : (7.787 * l + 16.0 / 116.0);
            m = (m > 0.008856) ? pow(m, 1.0 / 3.0) : (7.787 * m + 16.0 / 116.0);
            s = (s > 0.008856) ? pow(s, 1.0 / 3.0) : (7.787 * s + 16.0 / 116.0);

            double labA = 500.0 * (l - m);
            double labB = 200.0 * (m - s);

            dblMeanR += lr;
            dblMeanG += lg;
            dblMeanB += lb;
            dblMeanLabA += labA;
            dblMeanLabB += labB;
        }
    }

    double dblInv = 1.0 / cPixels;
    dblMeanR *= dblInv;
    dblMeanG *= dblInv;
    dblMeanB *= dblInv;
    dblMeanLabA *= dblInv;
    dblMeanLabB *= dblInv;

    double dblVarR = 0.0, dblVarG = 0.0, dblVarB = 0.0;
    for (INT y = 0; y < nHeight; y++)
    {
        const BYTE* pRow = pPixels + y * nWidth * 4;
        for (INT x = 0; x < nWidth; x++)
        {
            BYTE b = pRow[x * 4 + 0];
            BYTE g = pRow[x * 4 + 1];
            BYTE r = pRow[x * 4 + 2];

            double lr = static_cast<double>(r) / 255.0 - dblMeanR;
            double lg = static_cast<double>(g) / 255.0 - dblMeanG;
            double lb = static_cast<double>(b) / 255.0 - dblMeanB;

            dblVarR += lr * lr;
            dblVarG += lg * lg;
            dblVarB += lb * lb;
        }
    }

    double dblScaleR = (dblVarR > 0.0) ? (1.0 / sqrt(dblVarR * dblInv)) : 0.0;
    double dblScaleG = (dblVarG > 0.0) ? (1.0 / sqrt(dblVarG * dblInv)) : 0.0;
    double dblScaleB = (dblVarB > 0.0) ? (1.0 / sqrt(dblVarB * dblInv)) : 0.0;

    for (INT y = 0; y < nHeight; y++)
    {
        const BYTE* pRow = pPixels + y * nWidth * 4;
        for (INT x = 0; x < nWidth; x++)
        {
            BYTE b = pRow[x * 4 + 0];
            BYTE g = pRow[x * 4 + 1];
            BYTE r = pRow[x * 4 + 2];

            double dr = (static_cast<double>(r) / 255.0 - dblMeanR) * dblScaleR;
            double dg = (static_cast<double>(g) / 255.0 - dblMeanG) * dblScaleG;
            double db = (static_cast<double>(b) / 255.0 - dblMeanB) * dblScaleB;

            double dblDist = sqrt(dr * dr + dg * dg + db * db);

            size_t idx = static_cast<size_t>(y) * nWidth + x;
            vSaliency[idx] = static_cast<float>(dblDist);
        }
    }

    float flMax = 0.0f;
    for (size_t i = 0; i < cPixels; i++)
    {
        if (vSaliency[i] > flMax)
            flMax = vSaliency[i];
    }

    if (flMax > 0.0f)
    {
        float flInvMax = 1.0f / flMax;
        for (size_t i = 0; i < cPixels; i++)
            vSaliency[i] *= flInvMax;
    }

    return S_OK;
}

HRESULT AreaOfInterestGenerator::ExtractRegions(
    const std::vector<float>& vSaliency, INT nWidth, INT nHeight,
    std::vector<AreaOfInterest>& vRegions)
{
    if (vSaliency.empty() || nWidth <= 0 || nHeight <= 0)
        return E_INVALIDARG;

    vRegions.clear();

    float flThreshold = 0.3f;
    float flBlockSize = 16.0f;

    INT nBlocksX = static_cast<INT>(ceilf(static_cast<float>(nWidth) / flBlockSize));
    INT nBlocksY = static_cast<INT>(ceilf(static_cast<float>(nHeight) / flBlockSize));

    std::vector<bool> vVisited(static_cast<size_t>(nBlocksX) * nBlocksY, false);
    std::vector<std::vector<std::pair<INT, INT>>> vComponents;

    for (INT by = 0; by < nBlocksY; by++)
    {
        for (INT bx = 0; bx < nBlocksX; bx++)
        {
            size_t idx = static_cast<size_t>(by) * nBlocksX + bx;
            if (vVisited[idx])
                continue;

            INT x0 = static_cast<INT>(bx * flBlockSize);
            INT y0 = static_cast<INT>(by * flBlockSize);
            x0 = (x0 < nWidth) ? x0 : nWidth - 1;
            y0 = (y0 < nHeight) ? y0 : nHeight - 1;

            if (vSaliency[static_cast<size_t>(y0) * nWidth + x0] < flThreshold)
            {
                vVisited[idx] = true;
                continue;
            }

            std::vector<std::pair<INT, INT>> component;
            std::vector<std::pair<INT, INT>> stack;
            stack.push_back(std::make_pair(bx, by));
            vVisited[idx] = true;

            while (!stack.empty())
            {
                auto cur = stack.back();
                stack.pop_back();
                component.push_back(cur);

                const int dx[] = { -1, 1, 0, 0, -1, -1, 1, 1 };
                const int dy[] = { 0, 0, -1, 1, -1, 1, -1, 1 };

                for (int d = 0; d < 8; d++)
                {
                    int nx = cur.first + dx[d];
                    int ny = cur.second + dy[d];
                    if (nx >= 0 && ny >= 0 && nx < nBlocksX && ny < nBlocksY)
                    {
                        size_t ni = static_cast<size_t>(ny) * nBlocksX + nx;
                        if (!vVisited[ni])
                        {
                            INT px = nx * static_cast<INT>(flBlockSize);
                            INT py = ny * static_cast<INT>(flBlockSize);
                            px = (px < nWidth) ? px : nWidth - 1;
                            py = (py < nHeight) ? py : nHeight - 1;

                            float flVal = vSaliency[static_cast<size_t>(py) * nWidth + px];
                            if (flVal >= flThreshold)
                            {
                                vVisited[ni] = true;
                                stack.push_back(std::make_pair(nx, ny));
                            }
                            else
                            {
                                vVisited[ni] = true;
                            }
                        }
                    }
                }
            }

            if (component.size() >= 2)
            {
                vComponents.push_back(component);
            }
        }
    }

    std::sort(vComponents.begin(), vComponents.end(),
        [](const std::vector<std::pair<INT, INT>>& a,
           const std::vector<std::pair<INT, INT>>& b) { return a.size() > b.size(); });

    DWORD dwMaxRegions = m_dwMaxRegions;
    for (size_t c = 0; c < vComponents.size() && vRegions.size() < dwMaxRegions; c++)
    {
        const auto& comp = vComponents[c];

        INT nMinBX = comp[0].first, nMaxBX = comp[0].first;
        INT nMinBY = comp[0].second, nMaxBY = comp[0].second;
        float flTotalWeight = 0.0f;

        for (const auto& pt : comp)
        {
            if (pt.first < nMinBX) nMinBX = pt.first;
            if (pt.first > nMaxBX) nMaxBX = pt.first;
            if (pt.second < nMinBY) nMinBY = pt.second;
            if (pt.second > nMaxBY) nMaxBY = pt.second;

            INT px = pt.first * static_cast<INT>(flBlockSize);
            INT py = pt.second * static_cast<INT>(flBlockSize);
            px = (px < nWidth) ? px : nWidth - 1;
            py = (py < nHeight) ? py : nHeight - 1;
            flTotalWeight += vSaliency[static_cast<size_t>(py) * nWidth + px];
        }

        float flAvgWeight = flTotalWeight / static_cast<float>(comp.size());

        if (flAvgWeight < m_flMinRegionWeight)
            continue;

        RECT rcBounds;
        rcBounds.left = nMinBX * static_cast<LONG>(flBlockSize);
        rcBounds.top = nMinBY * static_cast<LONG>(flBlockSize);
        rcBounds.right = (nMaxBX + 1) * static_cast<LONG>(flBlockSize);
        rcBounds.bottom = (nMaxBY + 1) * static_cast<LONG>(flBlockSize);

        if (rcBounds.left < 0) rcBounds.left = 0;
        if (rcBounds.top < 0) rcBounds.top = 0;
        if (rcBounds.right > nWidth) rcBounds.right = nWidth;
        if (rcBounds.bottom > nHeight) rcBounds.bottom = nHeight;

        AreaOfInterest aoi;
        aoi.rcBounds = rcBounds;
        aoi.flWeight = std::min(1.0f, flAvgWeight);
        aoi.dwType = 1;
        aoi.bPrimary = (vRegions.empty());

        vRegions.push_back(aoi);
    }

    return S_OK;
}
