/*
 * WLXSlideshow.cpp
 *
 * Implementation of WLXSlideshow.dll -- slideshow generation for
 * Windows Live Movie Maker 2012.
 *
 * Manages photo sequencing, transition placement, audio synchronization,
 * and template-based slideshow creation. Templates define the look and
 * feel (timing, transitions, pan/zoom patterns) for auto-generated slideshows.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "WLXSlideshow.h"
#include "WLXPhotoBase.h"

#include <vector>
#include <string>
#include <map>
#include <memory>
#include <algorithm>
#include <cmath>

// ============================================================================
// Internal classes
// ============================================================================
namespace Slideshow
{

// ============================================================================
// Template definitions
// ============================================================================
struct TemplateDef
{
    WCHAR   wszId[64] = {};
    WCHAR   wszName[128] = {};
    UINT32  uDefaultSlideDurationMs = 5000;
    UINT32  uDefaultTransitionMs = 1000;
    WCHAR   wszDefaultTransitionId[64] = {};
    BOOL    bSupportsPanZoom = TRUE;
};

// Built-in template registry
static const TemplateDef s_builtinTemplates[] = {
    { L"Noel",          L"Noel",            5000, 1000, L"CrossFade",  TRUE },
    { L"Contemporary",  L"Contemporary",    4000, 1500, L"DiagonalWipe", TRUE },
    { L"Classic",       L"Classic",         6000, 800,  L"FadeToBlack", FALSE },
    { L"KenBurns",      L"Ken Burns",       5000, 1200, L"Dissolve",   TRUE },
    { L"Playful",       L"Playful",         3500, 1000, L"CircleWipe", TRUE },
    { L"Simple",        L"Simple",          5000, 500,  L"CrossFade",  FALSE },
};
static const UINT32 s_uBuiltinTemplateCount = ARRAYSIZE(s_builtinTemplates);

// ============================================================================
// SlideSequence -- ordered list of slides with computed timings
// ============================================================================
class SlideSequence
{
public:
    SlideSequence() {}
    ~SlideSequence() {}

    struct SlideEntry
    {
        SlideshowSlideInfo  info;
        LONGLONG            llStartTime;
        LONGLONG            llEndTime;
    };

    HRESULT Build(const SlideshowSlideInfo* pSlides, UINT32 uCount,
        const SlideshowConfig* pConfig)
    {
        if (!pSlides || uCount == 0 || !pConfig)
            return E_INVALIDARG;

        m_slides.clear();
        m_slides.reserve(uCount);

        LONGLONG llCurrentTime = 0;

        for (UINT32 i = 0; i < uCount; ++i)
        {
            SlideEntry entry;
            entry.info = pSlides[i];
            entry.llStartTime = llCurrentTime;

            // Apply default duration from config if not specified
            if (entry.info.llDuration <= 0)
                entry.info.llDuration = static_cast<LONGLONG>(pConfig->uSlideDurationMs) * 10000;

            entry.llEndTime = entry.llStartTime + entry.info.llDuration;
            m_slides.push_back(entry);

            llCurrentTime = entry.llEndTime;
        }

        m_llTotalDuration = llCurrentTime;
        return S_OK;
    }

    LONGLONG GetTotalDuration() const { return m_llTotalDuration; }
    UINT32 GetSlideCount() const { return static_cast<UINT32>(m_slides.size()); }

    const SlideEntry& GetSlide(UINT32 uIndex) const
    {
        return m_slides[uIndex];
    }

private:
    std::vector<SlideEntry> m_slides;
    LONGLONG                m_llTotalDuration;
};

// ============================================================================
// TransitionPlacer -- inserts transitions between slides
// ============================================================================
class TransitionPlacer
{
public:
    TransitionPlacer() {}
    ~TransitionPlacer() {}

    HRESULT PlaceTransitions(SlideSequence& sequence, const SlideshowConfig* pConfig)
    {
        if (!pConfig)
            return E_INVALIDARG;

        // Determine which transition to use between each pair of slides
        UINT32 uCount = sequence.GetSlideCount();
        m_transitions.resize(uCount > 0 ? uCount - 1 : 0);

        for (UINT32 i = 0; i < m_transitions.size(); ++i)
        {
            // Use configured transition or default
            // In the full implementation, this selects from the template's
            // transition set, optionally randomizing
            wcsncpy_s(m_transitions[i].wszTransitionId, 64,
                pConfig->bRandomizeTransitions ? L"CrossFade" : L"CrossFade",
                _TRUNCATE);
            m_transitions[i].uDurationMs = pConfig->uTransitionDurationMs;
        }

        return S_OK;
    }

private:
    struct TransitionEntry
    {
        WCHAR   wszTransitionId[64];
        UINT32  uDurationMs;
    };

    std::vector<TransitionEntry> m_transitions;
};

// ============================================================================
// SlideshowEngine -- main engine coordinating slideshow generation
// ============================================================================
class SlideshowEngine
{
public:
    SlideshowEngine()
        : m_bInitialized(false)
    {
    }

    ~SlideshowEngine()
    {
    }

    HRESULT Generate(const SlideshowSlideInfo* pSlides, UINT32 uSlideCount,
        const SlideshowConfig* pConfig, LONGLONG* pTotalDuration)
    {
        if (!pSlides || uSlideCount == 0 || !pConfig)
            return E_INVALIDARG;

        // Build the slide sequence
        SlideSequence sequence;
        HRESULT hr = sequence.Build(pSlides, uSlideCount, pConfig);
        if (FAILED(hr))
            return hr;

        // Place transitions
        TransitionPlacer placer;
        hr = placer.PlaceTransitions(sequence, pConfig);
        if (FAILED(hr))
            return hr;

        // Compute total duration accounting for transitions
        LONGLONG llTotal = sequence.GetTotalDuration();
        if (uSlideCount > 1)
        {
            // Subtract overlap from transitions
            llTotal -= static_cast<LONGLONG>(pConfig->uTransitionDurationMs) * 10000 * (uSlideCount - 1);
            if (llTotal < 0)
                llTotal = 0;
        }

        if (pTotalDuration)
            *pTotalDuration = llTotal;

        return S_OK;
    }

    HRESULT EnumerateTemplates(WCHAR* pTemplateIds, UINT32* pCount)
    {
        if (!pCount)
            return E_INVALIDARG;

        UINT32 uCopy = std::min(*pCount, s_uBuiltinTemplateCount);

        if (pTemplateIds)
        {
            for (UINT32 i = 0; i < uCopy; ++i)
            {
                wcsncpy_s(pTemplateIds + i * 64, 64,
                    s_builtinTemplates[i].wszId, _TRUNCATE);
            }
        }

        *pCount = s_uBuiltinTemplateCount;
        return S_OK;
    }

private:
    bool m_bInitialized;
};

} // namespace Slideshow

// ============================================================================
// Exported functions (4 exports)
// ============================================================================

extern "C"
{

WLXSLD_API HANDLE __stdcall Slideshow_Create()
{
    Slideshow::SlideshowEngine* pEngine = new(std::nothrow) Slideshow::SlideshowEngine();
    return static_cast<HANDLE>(pEngine);
}

WLXSLD_API void __stdcall Slideshow_Destroy(HANDLE hSlideshow)
{
    delete static_cast<Slideshow::SlideshowEngine*>(hSlideshow);
}

WLXSLD_API HRESULT __stdcall Slideshow_Generate(HANDLE hSlideshow,
    const SlideshowSlideInfo* pSlides, UINT32 uSlideCount,
    const Slideshow::SlideshowConfig* pConfig, LONGLONG* pTotalDuration)
{
    if (!hSlideshow) return E_INVALIDARG;
    return static_cast<Slideshow::SlideshowEngine*>(hSlideshow)->Generate(
        pSlides, uSlideCount, pConfig, pTotalDuration);
}

WLXSLD_API HRESULT __stdcall Slideshow_EnumerateTemplates(WCHAR* pTemplateIds, UINT32* pCount)
{
    Slideshow::SlideshowEngine engine;
    return engine.EnumerateTemplates(pTemplateIds, pCount);
}

} // extern "C"
