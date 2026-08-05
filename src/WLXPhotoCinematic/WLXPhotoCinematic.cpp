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
 * Camera model
 * ------------
 * The original binary animated a 3D camera (D3DXMatrixLookAtLH /
 * D3DXMatrixPerspectiveFovLH) over a full-screen textured quad. That camera
 * motion is mathematically equivalent to a 2D viewport interpolation:
 *
 *     scale     = 1 / viewport size       (magnification of the crop)
 *     translate = viewport center - 0.5   (camera position offset)
 *
 * This recreation computes the same camera transform per frame (see
 * FrameOutput / CinematicEngine::ComputeFrameTransform) and renders the
 * crop with GDI+ high-quality bicubic resampling, which is the software
 * equivalent of the GPU path. The GDI+ renderer was chosen because this
 * project links gdiplus and does not depend on the obsolete d3dx9_32.dll
 * that the original imported.
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
#include <new>
#include <wchar.h>

// ============================================================================
// Internal implementation
// ============================================================================

namespace
{
    // Constant active-object counter shared by the COM object hosting.
    // DllCanUnloadNow() returns S_OK only while this is zero.
    volatile LONG g_cActiveObjects = 0;

    const DOUBLE kEpsilon = 1.0e-9;
}

namespace PhotoCinematic
{

// ============================================================================
// Easing functions
// ============================================================================
class EasingFunction
{
public:
    // Maps linear progress dT in [0,1] through the easing curve.
    // Clamp to [0, 1]; inverted comparisons also map NaN to 0.
    static DOUBLE Evaluate(EasingType eType, DOUBLE dT)
    {
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
            // Quadratic ease-in-out (symmetric, C1 continuous at the midpoint)
            if (dT < 0.5)
                return 2.0 * dT * dT;
            else
                return 1.0 - 2.0 * (1.0 - dT) * (1.0 - dT);

        default:
            return dT;
        }
    }

    // First derivative (velocity) of the easing curve. Continuous for all
    // curves above; useful for frame-rate independent interpolation and for
    // driving pan/zoom velocity consumers.
    static DOUBLE GetDerivative(EasingType eType, DOUBLE dT)
    {
        if (!(dT > 0.0)) dT = 0.0;
        if (!(dT < 1.0)) dT = 1.0;

        switch (eType)
        {
        case EasingType_Linear:
            return 1.0;

        case EasingType_EaseIn:
            return 2.0 * dT;

        case EasingType_EaseOut:
            return 2.0 * (1.0 - dT);

        case EasingType_EaseInOut:
            if (dT < 0.5)
                return 4.0 * dT;
            else
                return 4.0 * (1.0 - dT);

        default:
            return 1.0;
        }
    }
};

// ============================================================================
// FrameOutput -- per-frame camera transform decomposition
// ============================================================================
// A single animation step fully described by its viewport, the normalized
// camera scale/translate (equivalent to the D3DX9 camera position of the
// original), and the timing values used to derive it.
struct FrameOutput
{
    DOUBLE          dProgress;          // linear progress in [0,1]
    DOUBLE          dEased;             // eased progress in [0,1]
    DOUBLE          dEasedVelocity;     // d(eased)/d(progress) at this frame
    NormalizedRect  rcViewport;         // normalized source crop region
    DOUBLE          dScaleX;            // 1.0 = full image width visible
    DOUBLE          dScaleY;            // 1.0 = full image height visible
    DOUBLE          dTranslateX;        // viewport center x offset from image center
    DOUBLE          dTranslateY;        // viewport center y offset from image center
    LONGLONG        llElapsed100ns;     // elapsed animation time in 100ns units

    FrameOutput()
        : dProgress(0.0)
        , dEased(0.0)
        , dEasedVelocity(0.0)
        , dScaleX(1.0)
        , dScaleY(1.0)
        , dTranslateX(0.0)
        , dTranslateY(0.0)
        , llElapsed100ns(0)
    {
    }
};

// ============================================================================
// KenBurnsEffect -- computes the interpolated camera viewport for a progress
// value, plus the normalized scale/translate decomposition of that viewport.
// ============================================================================
class KenBurnsEffect
{
public:
    KenBurnsEffect(const CinematicParams& params)
        : m_params(params)
        , m_start(SanitizeRect(params.rcViewportStart))
        , m_end(SanitizeRect(params.rcViewportEnd))
    {
    }

    // Get the interpolated viewport rectangle at progress dT (0.0 to 1.0)
    NormalizedRect GetViewport(DOUBLE dT) const
    {
        DOUBLE dEased = EasingFunction::Evaluate(m_params.eEasing, dT);

        NormalizedRect result;
        result.dLeft   = Lerp(m_start.dLeft,   m_end.dLeft,   dEased);
        result.dTop    = Lerp(m_start.dTop,    m_end.dTop,    dEased);
        result.dRight  = Lerp(m_start.dRight,  m_end.dRight,  dEased);
        result.dBottom = Lerp(m_start.dBottom, m_end.dBottom, dEased);

        return result;
    }

    // Full per-frame transform: eased progress, viewport, camera scale and
    // translation, and elapsed time.
    void GetFrameTransform(DOUBLE dT, FrameOutput& output) const
    {
        output.dProgress = Clamp01(dT);
        output.dEased = EasingFunction::Evaluate(m_params.eEasing, output.dProgress);
        output.dEasedVelocity = EasingFunction::GetDerivative(m_params.eEasing, output.dProgress);
        output.rcViewport = GetViewport(output.dProgress);
        output.llElapsed100ns =
            static_cast<LONGLONG>(output.dProgress * static_cast<DOUBLE>(m_params.llDuration));

        DOUBLE dVpW = output.rcViewport.dRight - output.rcViewport.dLeft;
        DOUBLE dVpH = output.rcViewport.dBottom - output.rcViewport.dTop;

        output.dScaleX = (dVpW > kEpsilon) ? 1.0 / dVpW : 1.0;
        output.dScaleY = (dVpH > kEpsilon) ? 1.0 / dVpH : 1.0;

        output.dTranslateX =
            (output.rcViewport.dLeft + output.rcViewport.dRight) * 0.5 - 0.5;
        output.dTranslateY =
            (output.rcViewport.dTop + output.rcViewport.dBottom) * 0.5 - 0.5;
    }

    // Maps the interpolated viewport to a source-pixel crop. The crop is
    // expanded with a uniform "cover" scale so the rendered frame fills the
    // requested output dimensions without distortion (matching the original
    // full-screen quad behavior). The crop is clamped to the image bounds;
    // the original let the quad slide slightly off-screen, this recreation
    // keeps the crop fully inside the source.
    bool GetSourceRect(const Gdiplus::Bitmap* pSource, DOUBLE dT,
        INT& iX, INT& iY, INT& iW, INT& iH) const
    {
        if (!pSource)
            return false;

        UINT srcW = pSource->GetWidth();
        UINT srcH = pSource->GetHeight();
        if (srcW == 0 || srcH == 0)
            return false;

        DOUBLE dOutW = static_cast<DOUBLE>(m_params.uOutputWidth);
        DOUBLE dOutH = static_cast<DOUBLE>(m_params.uOutputHeight);
        if (!(dOutW > 0.0) || !(dOutH > 0.0))
            return false;

        NormalizedRect vp = GetViewport(dT);
        if (!IsValidViewport(vp))
            vp = NormalizedRect();

        DOUBLE dVpW = vp.dRight - vp.dLeft;
        DOUBLE dVpH = vp.dBottom - vp.dTop;
        if (!(dVpW > 0.0) || !(dVpH > 0.0))
            return false;

        // Uniform "cover" scale: the crop must cover the full output frame.
        DOUBLE dScaleX = dOutW / (dVpW * srcW);
        DOUBLE dScaleY = dOutH / (dVpH * srcH);
        DOUBLE dScale  = (dScaleX > dScaleY) ? dScaleX : dScaleY;

        // Visible source region in pixels (aspect matches the output).
        DOUBLE dVisW = dOutW / dScale;
        DOUBLE dVisH = dOutH / dScale;

        DOUBLE dCx = (vp.dLeft + vp.dRight) * 0.5 * srcW;
        DOUBLE dCy = (vp.dTop + vp.dBottom) * 0.5 * srcH;

        INT x = static_cast<INT>(dCx - dVisW * 0.5 + 0.5);
        INT y = static_cast<INT>(dCy - dVisH * 0.5 + 0.5);
        INT w = static_cast<INT>(dVisW + 0.5);
        INT h = static_cast<INT>(dVisH + 0.5);

        // Clamp into image bounds, shifting the crop back when it overhangs.
        if (x < 0) x = 0;
        if (y < 0) y = 0;
        if (x + w > static_cast<INT>(srcW)) x = static_cast<INT>(srcW) - w;
        if (y + h > static_cast<INT>(srcH)) y = static_cast<INT>(srcH) - h;
        if (x < 0) x = 0;
        if (y < 0) y = 0;
        if (w > static_cast<INT>(srcW)) w = static_cast<INT>(srcW);
        if (h > static_cast<INT>(srcH)) h = static_cast<INT>(srcH);
        if (w <= 0 || h <= 0)
            return false;

        iX = x; iY = y; iW = w; iH = h;
        return true;
    }

private:
    static DOUBLE Lerp(DOUBLE a, DOUBLE b, DOUBLE t)
    {
        return a + (b - a) * t;
    }

    static DOUBLE Clamp01(DOUBLE dT)
    {
        if (!(dT > 0.0)) return 0.0;
        if (!(dT < 1.0)) return 1.0;
        return dT;
    }

    static bool IsValidViewport(const NormalizedRect& rc)
    {
        return (rc.dRight > rc.dLeft) && (rc.dBottom > rc.dTop);
    }

    // Normalizes a user-supplied viewport: fixes inverted coordinates,
    // collapses degenerate rects to the full frame, and clamps into [0,1]
    // while preserving size wherever possible.
    static NormalizedRect SanitizeRect(const NormalizedRect& rc)
    {
        NormalizedRect r = rc;

        if (!(r.dRight > r.dLeft)) { r.dLeft = 0.0; r.dRight = 1.0; }
        if (!(r.dBottom > r.dTop)) { r.dTop = 0.0; r.dBottom = 1.0; }

        DOUBLE w = r.dRight - r.dLeft;
        DOUBLE h = r.dBottom - r.dTop;

        if (w > 1.0) { r.dLeft = 0.0; r.dRight = 1.0; }
        else if (r.dLeft < 0.0) { r.dRight -= r.dLeft; r.dLeft = 0.0; }
        else if (r.dRight > 1.0) { r.dLeft -= r.dRight - 1.0; r.dRight = 1.0; }

        if (h > 1.0) { r.dTop = 0.0; r.dBottom = 1.0; }
        else if (r.dTop < 0.0) { r.dBottom -= r.dTop; r.dTop = 0.0; }
        else if (r.dBottom > 1.0) { r.dTop -= r.dBottom - 1.0; r.dBottom = 1.0; }

        return r;
    }

    CinematicParams  m_params;
    NormalizedRect   m_start;
    NormalizedRect   m_end;
};

// ============================================================================
// Pan/Zoom path generation
// ============================================================================
// Builds a classic Ken Burns camera path from the same parameters the
// original binary embedded in its PanZoomTheme XML resource (Zoom and
// PanZoom amounts, EffectLength): the camera starts on the full frame and
// drifts toward a magnified window offset by the pan amount (or reverses
// for a zoom-out).
void GeneratePanZoomPath(NormalizedRect* pStart, NormalizedRect* pEnd,
    DOUBLE dZoom, DOUBLE dPan, bool fPanHorizontal, bool fZoomIn)
{
    if (!pStart || !pEnd)
        return;

    if (!(dZoom > 0.0)) dZoom = 0.0;
    if (dZoom > 0.9)    dZoom = 0.9;
    if (dPan < 0.0)     dPan = -dPan;
    if (dPan > 0.45)    dPan = 0.45;

    // Magnified window size: the crop shrinks by the zoom delta, so the
    // visible magnification grows from 1.0 to (1.0 + dZoom).
    DOUBLE dWinW = 1.0 - dZoom;
    DOUBLE dWinH = 1.0 - dZoom;
    if (dWinW <= 0.0) dWinW = 0.1;
    if (dWinH <= 0.0) dWinH = 0.1;

    DOUBLE dCx = 0.5;
    DOUBLE dCy = 0.5;
    if (fPanHorizontal) dCx += dPan;
    else                dCy += dPan;

    // Keep the window inside the frame.
    if (dCx - dWinW * 0.5 < 0.0) dCx = dWinW * 0.5;
    if (dCx + dWinW * 0.5 > 1.0) dCx = 1.0 - dWinW * 0.5;
    if (dCy - dWinH * 0.5 < 0.0) dCy = dWinH * 0.5;
    if (dCy + dWinH * 0.5 > 1.0) dCy = 1.0 - dWinH * 0.5;

    NormalizedRect rcFull(0.0, 0.0, 1.0, 1.0);
    NormalizedRect rcWin(dCx - dWinW * 0.5, dCy - dWinH * 0.5,
                         dCx + dWinW * 0.5, dCy + dWinH * 0.5);

    if (fZoomIn)
    {
        *pStart = rcFull;
        *pEnd   = rcWin;
    }
    else
    {
        *pStart = rcWin;
        *pEnd   = rcFull;
    }
}

// Default zoom delta by image aspect ratio, taken verbatim from the
// PanZoomTheme XML embedded in the original binary:
//   XPortrait/Portrait  (aspect < 1.0)  -> 0.20
//   Landscape           (1.0 .. 2.0)    -> 0.10
//   Panorama/XPanorama  (aspect > 2.0)  -> 0.20
DOUBLE GetDefaultZoomForAspect(DOUBLE dAspect)
{
    if (!(dAspect > 0.0))
        return 0.2;
    if (dAspect <= 2.0)
        return (dAspect < 1.0) ? 0.2 : 0.1;
    return 0.2;
}

// Convenience: fills a CinematicParams with a generated Ken Burns path.
// The default pan amount (0.05) matches the PanZoom value used by every
// image class in the original theme XML.
void ConfigurePanZoom(CinematicParams* pParams, DOUBLE dZoom,
    DOUBLE dPan, bool fPanHorizontal, bool fZoomIn)
{
    if (!pParams)
        return;

    GeneratePanZoomPath(&pParams->rcViewportStart, &pParams->rcViewportEnd,
        dZoom, dPan, fPanHorizontal, fZoomIn);
}

// ============================================================================
// CinematicEngine -- main engine
// ============================================================================
class CinematicEngine
{
public:
    CinematicEngine() {}
    ~CinematicEngine() {}

    // Pure per-frame transform computation (no rendering). Equivalent to the
    // camera-position animation of the original D3DX9 path.
    HRESULT ComputeFrameTransform(const CinematicParams* pParams,
        DOUBLE dProgress, FrameOutput* pOutput) const
    {
        if (!pParams || !pOutput)
            return E_INVALIDARG;

        KenBurnsEffect kb(*pParams);
        kb.GetFrameTransform(dProgress, *pOutput);
        return S_OK;
    }

    // Renders a single animation frame as a new ARGB bitmap at the requested
    // output resolution.
    HRESULT RenderFrame(Gdiplus::Bitmap* pSource, const CinematicParams* pParams,
        DOUBLE dProgress, Gdiplus::Bitmap** ppBitmap)
    {
        if (!pSource || !pParams || !ppBitmap)
            return E_INVALIDARG;

        *ppBitmap = NULL;

        KenBurnsEffect kb(*pParams);

        INT srcX, srcY, srcW, srcH;
        if (!kb.GetSourceRect(pSource, dProgress, srcX, srcY, srcW, srcH))
            return E_INVALIDARG;

        Gdiplus::Bitmap* pBitmap = new(std::nothrow) Gdiplus::Bitmap(
            pParams->uOutputWidth, pParams->uOutputHeight, PixelFormat32bppARGB);
        if (!pBitmap)
            return E_OUTOFMEMORY;

        Gdiplus::Graphics g(pBitmap);
        g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
        g.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);
        g.SetCompositingQuality(Gdiplus::CompositingQualityHighQuality);
        g.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);

        Gdiplus::Status status = g.DrawImage(pSource,
            Gdiplus::Rect(0, 0, pParams->uOutputWidth, pParams->uOutputHeight),
            srcX, srcY, srcW, srcH,
            Gdiplus::UnitPixel);

        if (status != Gdiplus::Ok)
        {
            delete pBitmap;
            return Base::GdiplusStatusToHresult(status);
        }

        *ppBitmap = pBitmap;
        return S_OK;
    }

    // Applies the animation frame directly onto a caller-owned GDI+ surface,
    // covering the full output rectangle described by pParams.
    HRESULT RenderFrameToGraphics(Gdiplus::Bitmap* pSource,
        const CinematicParams* pParams, DOUBLE dProgress, Gdiplus::Graphics* pGraphics)
    {
        if (!pSource || !pParams || !pGraphics)
            return E_INVALIDARG;

        KenBurnsEffect kb(*pParams);

        INT srcX, srcY, srcW, srcH;
        if (!kb.GetSourceRect(pSource, dProgress, srcX, srcY, srcW, srcH))
            return E_INVALIDARG;

        Gdiplus::Status status = pGraphics->DrawImage(pSource,
            Gdiplus::Rect(0, 0, pParams->uOutputWidth, pParams->uOutputHeight),
            srcX, srcY, srcW, srcH,
            Gdiplus::UnitPixel);

        return Base::GdiplusStatusToHresult(status);
    }

    // Renders the full animation into an array of frames. Each frame is a
    // newly allocated bitmap; the caller frees them with ReleaseFrames().
    HRESULT RenderAll(Gdiplus::Bitmap* pSource, const CinematicParams* pParams,
        UINT32 uFrameRate, Gdiplus::Bitmap*** ppFrames, UINT32* pFrameCount)
    {
        if (!pSource || !pParams || !ppFrames || !pFrameCount)
            return E_INVALIDARG;

        if (pParams->llDuration <= 0)
            return E_INVALIDARG;

        if (uFrameRate == 0)
            uFrameRate = 30;

        UINT32 uTotalFrames = GetFrameCount(*pParams, uFrameRate);

        Gdiplus::Bitmap** pFrames = new(std::nothrow) Gdiplus::Bitmap*[uTotalFrames];
        if (!pFrames)
        {
            *ppFrames = NULL;
            *pFrameCount = 0;
            return E_OUTOFMEMORY;
        }

        ZeroMemory(pFrames, uTotalFrames * sizeof(Gdiplus::Bitmap*));

        UINT32 i;
        HRESULT hr = S_OK;
        for (i = 0; i < uTotalFrames; ++i)
        {
            DOUBLE dProgress = (uTotalFrames > 1)
                ? static_cast<DOUBLE>(i) / static_cast<DOUBLE>(uTotalFrames - 1)
                : 0.0;
            hr = RenderFrame(pSource, pParams, dProgress, &pFrames[i]);
            if (FAILED(hr))
                break;
        }

        if (FAILED(hr))
        {
            // Free the frames rendered so far (indices [0, i-1]).
            ReleaseFrames(pFrames, i);
            *ppFrames = NULL;
            *pFrameCount = 0;
            return hr;
        }

        *ppFrames = pFrames;
        *pFrameCount = uTotalFrames;
        return S_OK;
    }

    // Frees a frame array produced by RenderAll().
    static void ReleaseFrames(Gdiplus::Bitmap** ppFrames, UINT32 cFrames)
    {
        if (!ppFrames)
            return;

        for (UINT32 i = 0; i < cFrames; ++i)
        {
            delete ppFrames[i];
            ppFrames[i] = NULL;
        }

        delete[] ppFrames;
    }

    // Frame count for a duration/frame-rate pair. Always at least one frame.
    static UINT32 GetFrameCount(const CinematicParams& params, UINT32 uFrameRate)
    {
        UINT32 uFps = (uFrameRate == 0) ? 30 : uFrameRate;

        DOUBLE dDurationSec = static_cast<DOUBLE>(params.llDuration) / 10000000.0;
        UINT32 uTotalFrames = static_cast<UINT32>(ceil(dDurationSec * uFps));
        if (uTotalFrames == 0)
            uTotalFrames = 1;
        if (uTotalFrames > 100000)
            uTotalFrames = 100000;

        return uTotalFrames;
    }
};

// ============================================================================
// COM object hosting
// ============================================================================
// The original binary hosts two COM objects (object map CLSIDs extracted from
// the binary; the RGS resource used different, unserved CLSIDs -- a quirk of
// the original that is not reproduced here):
//
//   {557B4CD8-C1EA-4A46-84EE-BA1AF9AA67D4}  CinematicFullScreen
//   {0B1A232A-4A09-4A43-A7B3-E367D1C3B4B7}  PanZoomTransform
//
// Both objects are IUnknown-only on their primary interface (they answer the
// identity IIDs below). The domain-specific method sets (IMediaNode-style
// methods on PanZoomTransform, the 9 custom slots on CinematicFullScreen) are
// NOT declared in the public header, so their contracts are unknown and they
// are out of scope for this recreation. The objects exist so the effect
// engine can be hosted and instantiated through the standard COM entry points.

const CLSID CLSID_CinematicFullScreen =
    { 0x557B4CD8, 0xC1EA, 0x4A46, { 0x84, 0xEE, 0xBA, 0x1A, 0xF9, 0xAA, 0x67, 0xD4 } };
const CLSID CLSID_PanZoomTransform =
    { 0x0B1A232A, 0x4A09, 0x4A43, { 0xA7, 0xB3, 0xE3, 0x67, 0xD1, 0xC3, 0xB4, 0xB7 } };

const IID IID_IdentityCinematicFullScreen =
    { 0x70E8E77F, 0x8721, 0x46B6, { 0xB7, 0x46, 0x33, 0x5E, 0xBF, 0x85, 0x77, 0x04 } };
const IID IID_IdentityPanZoomTransform =
    { 0xC2DB9835, 0x1146, 0x4B10, { 0xA7, 0xBB, 0x73, 0x61, 0x91, 0x25, 0xB0, 0x26 } };

// Common reference-counted base. Each live object (including class
// factories) contributes to g_cActiveObjects so DllCanUnloadNow() can
// report whether the DLL is safely unloadable.
class CComObjectBase
{
public:
    CComObjectBase() : m_cRef(1)
    {
        InterlockedIncrement(&g_cActiveObjects);
    }

    virtual ~CComObjectBase()
    {
        InterlockedDecrement(&g_cActiveObjects);
    }

    ULONG AddRef()
    {
        return static_cast<ULONG>(InterlockedIncrement(&m_cRef));
    }

    ULONG Release()
    {
        ULONG cRef = static_cast<ULONG>(InterlockedDecrement(&m_cRef));
        if (cRef == 0)
            delete this;
        return cRef;
    }

protected:
    volatile LONG m_cRef;
};

class CinematicFullScreenObject : public CComObjectBase
{
public:
    HRESULT QueryInterface(REFIID riid, void** ppv)
    {
        if (!ppv)
            return E_POINTER;
        *ppv = NULL;

        if (IsEqualIID(riid, IID_IUnknown) ||
            IsEqualIID(riid, IID_IdentityCinematicFullScreen))
        {
            *ppv = static_cast<IUnknown*>(this);
            AddRef();
            return S_OK;
        }

        return E_NOINTERFACE;
    }

    CinematicEngine m_engine;
};

class PanZoomTransformObject : public CComObjectBase
{
public:
    HRESULT QueryInterface(REFIID riid, void** ppv)
    {
        if (!ppv)
            return E_POINTER;
        *ppv = NULL;

        if (IsEqualIID(riid, IID_IUnknown) ||
            IsEqualIID(riid, IID_IdentityPanZoomTransform))
        {
            *ppv = static_cast<IUnknown*>(this);
            AddRef();
            return S_OK;
        }

        // IServiceProvider sub-interface ({6D5140C1-...}) at this+4 exists in
        // the original binary but is not reproduced: its 5 custom methods are
        // undocumented and out of scope.
        return E_NOINTERFACE;
    }

    CinematicEngine m_engine;
};

// Minimal class factory bound to one of the servable CLSIDs.
class CClassFactory : public IClassFactory
{
public:
    CClassFactory(REFCLSID clsid)
        : m_clsid(clsid)
        , m_cRef(1)
    {
        InterlockedIncrement(&g_cActiveObjects);
    }

    virtual ~CClassFactory()
    {
        InterlockedDecrement(&g_cActiveObjects);
    }

    STDMETHODIMP QueryInterface(REFIID riid, void** ppv) override
    {
        if (!ppv)
            return E_POINTER;
        *ppv = NULL;

        if (IsEqualIID(riid, IID_IUnknown) ||
            IsEqualIID(riid, IID_IClassFactory))
        {
            *ppv = static_cast<IClassFactory*>(this);
            AddRef();
            return S_OK;
        }

        return E_NOINTERFACE;
    }

    STDMETHODIMP_(ULONG) AddRef() override
    {
        return static_cast<ULONG>(InterlockedIncrement(&m_cRef));
    }

    STDMETHODIMP_(ULONG) Release() override
    {
        ULONG cRef = static_cast<ULONG>(InterlockedDecrement(&m_cRef));
        if (cRef == 0)
            delete this;
        return cRef;
    }

    STDMETHODIMP CreateInstance(LPUNKNOWN pUnkOuter, REFIID riid, void** ppv) override
    {
        if (!ppv)
            return E_POINTER;
        *ppv = NULL;

        if (pUnkOuter)
            return CLASS_E_NOAGGREGATION;

        IUnknown* pObj = NULL;
        if (IsEqualCLSID(m_clsid, CLSID_CinematicFullScreen))
        {
            pObj = new(std::nothrow) CinematicFullScreenObject();
        }
        else if (IsEqualCLSID(m_clsid, CLSID_PanZoomTransform))
        {
            pObj = new(std::nothrow) PanZoomTransformObject();
        }
        else
        {
            return CLASS_E_CLASSNOTAVAILABLE;
        }

        if (!pObj)
            return E_OUTOFMEMORY;

        HRESULT hr = pObj->QueryInterface(riid, ppv);
        pObj->Release();
        return hr;
    }

    STDMETHODIMP LockServer(BOOL fLock) override
    {
        if (fLock)
            InterlockedIncrement(&g_cActiveObjects);
        else
            InterlockedDecrement(&g_cActiveObjects);
        return S_OK;
    }

private:
    CLSID m_clsid;
    volatile LONG m_cRef;
};

// Registers the servable objects under the Slideshow Themes key, matching the
// original binary's .rgs location. Returns E_ACCESSDENIED when the process
// lacks rights to write HKLM (the original returned the same without admin).
HRESULT RegisterServer()
{
    HMODULE hMod = NULL;
    if (!GetModuleHandleExW(
            GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
            GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            reinterpret_cast<LPCWSTR>(&RegisterServer), &hMod))
        return HRESULT_FROM_WIN32(GetLastError());

    WCHAR szPath[MAX_PATH];
    if (GetModuleFileNameW(hMod, szPath, MAX_PATH) == 0)
        return HRESULT_FROM_WIN32(GetLastError());

    const CLSID* apClsids[2] = { &CLSID_CinematicFullScreen, &CLSID_PanZoomTransform };

    for (int i = 0; i < 2; ++i)
    {
        WCHAR szClsid[128];
        StringFromGUID2(*apClsids[i], szClsid, 128);

        WCHAR szKey[512];
        wsprintfW(szKey,
            L"Software\\Microsoft\\Windows Photo Gallery\\Slideshow\\Themes\\%s",
            szClsid);

        HKEY hKey = NULL;
        LONG lr = RegCreateKeyExW(HKEY_LOCAL_MACHINE, szKey, 0, NULL,
            REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL);
        if (lr != ERROR_SUCCESS || !hKey)
            return HRESULT_FROM_WIN32((lr == ERROR_SUCCESS) ? GetLastError() : lr);

        LPCWSTR wszThreading = L"Apartment";
        RegSetValueExW(hKey, L"InprocServer32", 0, REG_SZ,
            reinterpret_cast<const BYTE*>(szPath),
            static_cast<DWORD>((wcslen(szPath) + 1) * sizeof(WCHAR)));
        RegSetValueExW(hKey, L"ThreadingModel", 0, REG_SZ,
            reinterpret_cast<const BYTE*>(wszThreading),
            static_cast<DWORD>((wcslen(wszThreading) + 1) * sizeof(WCHAR)));
        RegCloseKey(hKey);
    }

    return S_OK;
}

// Removes the registration keys written by RegisterServer().
HRESULT UnregisterServer()
{
    const CLSID* apClsids[2] = { &CLSID_CinematicFullScreen, &CLSID_PanZoomTransform };

    for (int i = 0; i < 2; ++i)
    {
        WCHAR szClsid[128];
        StringFromGUID2(*apClsids[i], szClsid, 128);

        WCHAR szKey[512];
        wsprintfW(szKey,
            L"Software\\Microsoft\\Windows Photo Gallery\\Slideshow\\Themes\\%s",
            szClsid);

        RegDeleteKeyW(HKEY_LOCAL_MACHINE, szKey);
    }

    return S_OK;
}

} // namespace PhotoCinematic

// ============================================================================
// Exported functions -- standard COM DLL entry points
// ============================================================================

extern "C"
{

STDAPI DllCanUnloadNow()
{
    return (g_cActiveObjects == 0) ? S_OK : S_FALSE;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    if (!ppv)
        return E_POINTER;
    *ppv = NULL;

    if (!(IsEqualCLSID(rclsid, PhotoCinematic::CLSID_CinematicFullScreen) ||
          IsEqualCLSID(rclsid, PhotoCinematic::CLSID_PanZoomTransform)))
        return CLASS_E_CLASSNOTAVAILABLE;

    PhotoCinematic::CClassFactory* pFactory =
        new(std::nothrow) PhotoCinematic::CClassFactory(rclsid);
    if (!pFactory)
        return E_OUTOFMEMORY;

    HRESULT hr = pFactory->QueryInterface(riid, ppv);
    pFactory->Release();
    return hr;
}

STDAPI DllRegisterServer()
{
    return PhotoCinematic::RegisterServer();
}

STDAPI DllUnregisterServer()
{
    return PhotoCinematic::UnregisterServer();
}

} // extern "C"
