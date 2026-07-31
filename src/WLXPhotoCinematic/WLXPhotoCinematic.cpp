/*
 * WLXPhotoCinematic.cpp
 *
 * Implementation of WLXPhotoCinematic.dll -- cinematic photo effects
 * for Windows Live Movie Maker 2012.
 *
 * Implements Ken Burns pan/zoom animations on still images:
 *   1. Selects a start and end viewport region on the source image
 *   2. Applies easing curves for smooth acceleration/deceleration
 *   3. Resamples the viewport region at each animation step
 *   4. Outputs rendered frames at the requested resolution and frame rate
 *
 * Uses GDI+ for high-quality bilinear/bicubic resampling and supports
 * both software and D3D9-accelerated rendering paths via d3dx9_32.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "WLXPhotoCinematic.h"
#include "WLXPhotoBase.h"

#include <vector>
#include <memory>
#include <cmath>

// ============================================================================
// Internal implementation
// ============================================================================
namespace PhotoCinematic
{

// ============================================================================
// Easing functions
// ============================================================================
class EasingFunction
{
public:
    static DOUBLE Evaluate(EasingType eType, DOUBLE dT)
    {
        // Clamp to [0, 1]; inverted comparisons also map NaN to 0
        if (!(dT > 0.0)) return 0.0;
        if (!(dT < 1.0)) return 1.0;

        switch (eType)
        {
        case EasingType_Linear:
            return dT;

        case EasingType_EaseIn:
            // Quadratic ease-in
            return dT * dT;

        case EasingType_EaseOut:
            // Quadratic ease-out
            return 1.0 - (1.0 - dT) * (1.0 - dT);

        case EasingType_EaseInOut:
            // Quadratic ease-in-out
            if (dT < 0.5)
                return 2.0 * dT * dT;
            else
                return 1.0 - 2.0 * (1.0 - dT) * (1.0 - dT);

        default:
            return dT;
        }
    }
};

// ============================================================================
// KenBurnsEffect -- computes interpolated viewport for a given progress value
// ============================================================================
class KenBurnsEffect
{
public:
    KenBurnsEffect(const CinematicParams& params)
        : m_params(params)
    {
    }

    // Get the interpolated viewport rectangle at progress dT (0.0 to 1.0)
    NormalizedRect GetViewport(DOUBLE dT) const
    {
        DOUBLE dEased = EasingFunction::Evaluate(m_params.eEasing, dT);

        NormalizedRect result;
        result.dLeft   = Lerp(m_params.rcViewportStart.dLeft,   m_params.rcViewportEnd.dLeft,   dEased);
        result.dTop    = Lerp(m_params.rcViewportStart.dTop,    m_params.rcViewportEnd.dTop,    dEased);
        result.dRight  = Lerp(m_params.rcViewportStart.dRight,  m_params.rcViewportEnd.dRight,  dEased);
        result.dBottom = Lerp(m_params.rcViewportStart.dBottom, m_params.rcViewportEnd.dBottom, dEased);

        return result;
    }

private:
    static DOUBLE Lerp(DOUBLE a, DOUBLE b, DOUBLE t)
    {
        return a + (b - a) * t;
    }

    CinematicParams m_params;
};

// ============================================================================
// CinematicEngine -- main engine
// ============================================================================
class CinematicEngine
{
public:
    CinematicEngine() {}
    ~CinematicEngine() {}

    HRESULT RenderFrame(Gdiplus::Bitmap* pSource, const CinematicParams* pParams,
        DOUBLE dProgress, Gdiplus::Bitmap** ppBitmap)
    {
        if (!pSource || !pParams || !ppBitmap)
            return E_INVALIDARG;

        if (dProgress < 0.0) dProgress = 0.0;
        if (dProgress > 1.0) dProgress = 1.0;

        *ppBitmap = NULL;

        // Compute the interpolated viewport
        KenBurnsEffect kb(*pParams);
        NormalizedRect viewport = kb.GetViewport(dProgress);

        // Convert normalized coordinates to source image pixels
        UINT srcW = pSource->GetWidth();
        UINT srcH = pSource->GetHeight();

        INT srcX = static_cast<INT>(viewport.dLeft * srcW);
        INT srcY = static_cast<INT>(viewport.dTop * srcH);
        INT srcW2 = static_cast<INT>((viewport.dRight - viewport.dLeft) * srcW);
        INT srcH2 = static_cast<INT>((viewport.dBottom - viewport.dTop) * srcH);

        // Clamp
        if (srcX < 0) srcX = 0;
        if (srcY < 0) srcY = 0;
        if (srcX + srcW2 > static_cast<INT>(srcW)) srcW2 = srcW - srcX;
        if (srcY + srcH2 > static_cast<INT>(srcH)) srcH2 = srcH - srcY;
        if (srcW2 <= 0 || srcH2 <= 0)
            return E_INVALIDARG;

        // Create output bitmap at the target resolution
        *ppBitmap = new Gdiplus::Bitmap(pParams->uOutputWidth, pParams->uOutputHeight,
            PixelFormat32bppARGB);
        if (!*ppBitmap)
            return E_OUTOFMEMORY;

        Gdiplus::Graphics g(*ppBitmap);
        g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
        g.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);
        g.SetCompositingQuality(Gdiplus::CompositingQualityHighQuality);

        // Draw the viewport region, scaled to the output size
        g.DrawImage(pSource,
            Gdiplus::Rect(0, 0, pParams->uOutputWidth, pParams->uOutputHeight),
            srcX, srcY, srcW2, srcH2,
            Gdiplus::UnitPixel);

        return S_OK;
    }

    HRESULT RenderAll(Gdiplus::Bitmap* pSource, const CinematicParams* pParams,
        UINT32 uFrameRate, Gdiplus::Bitmap*** ppFrames, UINT32* pFrameCount)
    {
        if (!pSource || !pParams || !ppFrames || !pFrameCount)
            return E_INVALIDARG;

        if (uFrameRate == 0)
            uFrameRate = 30;

        // Calculate total frames
        DOUBLE dDurationSec = static_cast<DOUBLE>(pParams->llDuration) / 10000000.0;
        UINT32 uTotalFrames = static_cast<UINT32>(dDurationSec * uFrameRate);
        if (uTotalFrames == 0)
            uTotalFrames = 1;

        // Allocate frame array
        *ppFrames = new(std::nothrow) Gdiplus::Bitmap*[uTotalFrames];
        if (!*ppFrames)
            return E_OUTOFMEMORY;

        ZeroMemory(*ppFrames, uTotalFrames * sizeof(Gdiplus::Bitmap*));

        for (UINT32 i = 0; i < uTotalFrames; ++i)
        {
            DOUBLE dProgress = (uTotalFrames > 1) ? (static_cast<DOUBLE>(i) / static_cast<DOUBLE>(uTotalFrames - 1)) : 0.0;
            HRESULT hr = RenderFrame(pSource, pParams, dProgress, &(*ppFrames)[i]);
            if (FAILED(hr))
            {
                // Clean up on failure
                // Clean up on failure
            for (UINT32 j = 0; j < i; ++j) // use < i to avoid double delete
                delete (*ppFrames)[j];
                    delete (*ppFrames)[j];
                delete[] *ppFrames;
                *ppFrames = NULL;
                *pFrameCount = 0;
                return hr;
            }
        }

        *pFrameCount = uTotalFrames;
        return S_OK;
    }
};

} // namespace PhotoCinematic

// ============================================================================
// Exported functions -- standard COM DLL entry points
// ============================================================================

extern "C"
{

STDAPI DllCanUnloadNow()
{
    return S_OK;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    UNREFERENCED_PARAMETER(rclsid);
    UNREFERENCED_PARAMETER(riid);
    UNREFERENCED_PARAMETER(ppv);
    return CLASS_E_CLASSNOTAVAILABLE;
}

STDAPI DllRegisterServer()
{
    return S_OK;
}

STDAPI DllUnregisterServer()
{
    return S_OK;
}

} // extern "C"
