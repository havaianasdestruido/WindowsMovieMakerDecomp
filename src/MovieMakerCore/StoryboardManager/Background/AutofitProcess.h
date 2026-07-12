/*
 * AutofitProcess.h
 *
 * AutofitProcess: auto-fit photo duration to music beat/tempo.
 * AreaOfInterestGenerator: generate pan/zoom areas of interest for
 * still photos based on computed saliency and face detection.
 *
 * RTTI: ?AVAutofitProcess@@, ?AVAreaOfInterestGenerator@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once
#ifndef AUTOFITPROCESS_H
#define AUTOFITPROCESS_H

#include "../../pch.h"

// ============================================================================
// AreaOfInterest
// ============================================================================
// Describes a pan/zoom region of interest for a still photo. Used by the
// Ken Burns effect to animate the camera across the photo during playback.
//
struct AreaOfInterest
{
    RECT    rcBounds;     // bounding rectangle (normalized 0..1000)
    float   flWeight;     // importance weight (0.0 to 1.0)
    DWORD   dwType;       // 0=face, 1=salient region, 2=center
    bool    bPrimary;     // primary focus point

    AreaOfInterest()
        : rcBounds({ 0, 0, 1000, 1000 })
        , flWeight(1.0f)
        , dwType(0)
        , bPrimary(true)
    {
    }
};

// ============================================================================
// AutofitProcess
// ============================================================================
// Analyzes the project timeline and music track to compute optimal photo
// durations that synchronize with the music tempo/beat structure.
// Adjusts extent durations to match detected beats and measures.
//
class AutofitProcess
{
public:
    AutofitProcess();
    ~AutofitProcess();

    // -- Configuration --
    void SetMusicFilePath(LPCWSTR pszPath);
    ATL::CString GetMusicFilePath() const;

    void SetPhotoCount(DWORD dwCount);
    DWORD GetPhotoCount() const throw();

    void SetTotalDurationHns(LONGLONG llDuration);
    LONGLONG GetTotalDurationHns() const throw();

    void SetMinPhotoDurationHns(LONGLONG llDuration);
    LONGLONG GetMinPhotoDurationHns() const throw();

    void SetMaxPhotoDurationHns(LONGLONG llDuration);
    LONGLONG GetMaxPhotoDurationHns() const throw();

    // -- Execution --
    HRESULT Analyze();
    HRESULT ApplyToProject();

    // -- Results --
    size_t GetDurationCount() const throw();
    LONGLONG GetPhotoDurationAt(size_t nIndex) const;

    // -- Beat detection --
    bool HasBeatData() const throw();
    DWORD GetBeatCount() const throw();
    LONGLONG GetBeatPositionMs(DWORD dwIndex) const throw();

private:
    // Audio beat detection
    HRESULT AnalyzeAudio();

    ATL::CString         m_strMusicFilePath;
    DWORD                m_dwPhotoCount;
    LONGLONG             m_llTotalDurationHns;
    LONGLONG             m_llMinPhotoDurationHns;
    LONGLONG             m_llMaxPhotoDurationHns;

    // Computed durations per photo
    std::vector<LONGLONG> m_vPhotoDurations;

    // Detected beat positions (milliseconds)
    std::vector<LONGLONG> m_vBeatPositionsMs;
};

// ============================================================================
// AreaOfInterestGenerator
// ============================================================================
// Generates pan/zoom areas of interest for still photos by analyzing image
// content. Uses saliency maps, face detection, and composition heuristics
// to compute primary and secondary focus regions for the Ken Burns effect.
//
class AreaOfInterestGenerator
{
public:
    AreaOfInterestGenerator();
    ~AreaOfInterestGenerator();

    // -- Generation --
    HRESULT Generate(Gdiplus::Bitmap* pBitmap, std::vector<AreaOfInterest>& vResults);

    // -- Configuration --
    void SetMaxRegions(DWORD dwMaxRegions);
    DWORD GetMaxRegions() const throw();

    void SetMinRegionWeight(float flWeight);
    float GetMinRegionWeight() const throw();

    void SetAspectRatio(float flAspectRatio);
    float GetAspectRatio() const throw();

    // -- Helpers --
    static HRESULT ComputeCenterOfInterest(Gdiplus::Bitmap* pBitmap, POINT* ptCenter);
    static HRESULT ComputeFaceRectangles(Gdiplus::Bitmap* pBitmap, std::vector<RECT>& vFaces);

private:
    // Internal analysis methods
    HRESULT AnalyzeSaliency(INT nWidth, INT nHeight, const BYTE* pPixels, std::vector<float>& vSaliency);
    HRESULT ExtractRegions(const std::vector<float>& vSaliency, INT nWidth, INT nHeight, std::vector<AreaOfInterest>& vRegions);

    DWORD  m_dwMaxRegions;
    float  m_flMinRegionWeight;
    float  m_flAspectRatio;
};

#endif // AUTOFITPROCESS_H
