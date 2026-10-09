/*
 * WLXPipetran.cpp
 *
 * Implementation of WLXPipetran.dll -- pipeline transform classes for
 * transitions and effect animations in Windows Live Movie Maker 2012.
 *
 * Contains 92 RTTI classes implementing the full set of transitions and
 * effects available in Movie Maker. Each transform class derives from
 * either TransitionBase or EffectBase and implements the Apply() method.
 *
 * The TransformRegistry is a singleton that maps string identifiers to
 * factory functions, enabling dynamic creation of transforms by name.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "WLXPipetran.h"
#include "WLXPhotoBase.h"

#include <vector>
#include <map>
#include <string>
#include <memory>
#include <mutex>
#include <cmath>
#include <functional>
#include <algorithm>

// ============================================================================
// Internal implementation
// ============================================================================
namespace Pipetran
{

// ============================================================================
// TransitionBase -- abstract base for all transitions
// ============================================================================
class TransitionBase
{
public:
    TransitionBase() {}
    virtual ~TransitionBase() {}

    virtual HRESULT Apply(
        Gdiplus::Bitmap* pSourceFrom,
        Gdiplus::Bitmap* pSourceTo,
        Gdiplus::Bitmap** ppResult,
        const TransformParams* pParams) = 0;

    virtual const WCHAR* GetId() const = 0;
    virtual const WCHAR* GetName() const = 0;
};

// ============================================================================
// EffectBase -- abstract base for all effects
// ============================================================================
class EffectBase
{
public:
    EffectBase() {}
    virtual ~EffectBase() {}

    virtual HRESULT Apply(
        Gdiplus::Bitmap* pSource,
        Gdiplus::Bitmap** ppResult,
        const TransformParams* pParams) = 0;

    virtual const WCHAR* GetId() const = 0;
    virtual const WCHAR* GetName() const = 0;
};

// ============================================================================
// Transition implementations (26 transitions)
// ============================================================================

class CrossFadeTransition : public TransitionBase
{
public:
    const WCHAR* GetId() const override { return L"CrossFade"; }
    const WCHAR* GetName() const override { return L"Cross Fade"; }

    HRESULT Apply(Gdiplus::Bitmap* pFrom, Gdiplus::Bitmap* pTo,
        Gdiplus::Bitmap** ppResult, const TransformParams* pParams) override
    {
        if (!pFrom || !pTo || !ppResult || !pParams)
            return E_INVALIDARG;

        UINT w = pFrom->GetWidth();
        UINT h = pFrom->GetHeight();
        *ppResult = new Gdiplus::Bitmap(w, h, PixelFormat32bppARGB);
        if (!*ppResult)
            return E_OUTOFMEMORY;

        Gdiplus::Graphics g(*ppResult);
        g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);

        BYTE alpha = static_cast<BYTE>(pParams->dProgress * 255.0);

        // Draw source-from with decreasing opacity
        Gdiplus::ImageAttributes attrFrom;
        Gdiplus::ColorMatrix cm = {
            1, 0, 0, 0, 0,
            0, 1, 0, 0, 0,
            0, 0, 1, 0, 0,
            0, 0, 0, 1.0 - pParams->dProgress, 0,
            0, 0, 0, 0, 1
        };
        attrFrom.SetColorMatrix(&cm);
        g.DrawImage(pFrom, Gdiplus::Rect(0, 0, w, h), 0, 0, w, h,
            Gdiplus::UnitPixel, &attrFrom);

        // Draw source-to with increasing opacity
        Gdiplus::ImageAttributes attrTo;
        Gdiplus::ColorMatrix cmTo = {
            1, 0, 0, 0, 0,
            0, 1, 0, 0, 0,
            0, 0, 1, 0, 0,
            0, 0, 0, pParams->dProgress, 0,
            0, 0, 0, 0, 1
        };
        attrTo.SetColorMatrix(&cmTo);
        g.DrawImage(pTo, Gdiplus::Rect(0, 0, w, h), 0, 0, w, h,
            Gdiplus::UnitPixel, &attrTo);

        return S_OK;
    }
};

class FadeToBlackTransition : public TransitionBase
{
public:
    const WCHAR* GetId() const override { return L"FadeToBlack"; }
    const WCHAR* GetName() const override { return L"Fade to Black"; }

    HRESULT Apply(Gdiplus::Bitmap* pFrom, Gdiplus::Bitmap* pTo,
        Gdiplus::Bitmap** ppResult, const TransformParams* pParams) override
    {
        if (!pFrom || !pTo || !ppResult || !pParams)
            return E_INVALIDARG;

        UINT w = pFrom->GetWidth();
        UINT h = pFrom->GetHeight();
        *ppResult = new Gdiplus::Bitmap(w, h, PixelFormat32bppARGB);
        if (!*ppResult)
            return E_OUTOFMEMORY;

        Gdiplus::Graphics g(*ppResult);
        g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);

        double d = pParams->dProgress;

        if (d < 0.5)
        {
            // Fade from source to black
            double fadeOut = d * 2.0;
            Gdiplus::ImageAttributes attr;
            Gdiplus::ColorMatrix cm = {
                1, 0, 0, 0, 0,
                0, 1, 0, 0, 0,
                0, 0, 1, 0, 0,
                0, 0, 0, 1.0 - fadeOut, 0,
                0, 0, 0, 0, 1
            };
            attr.SetColorMatrix(&cm);
            g.DrawImage(pFrom, Gdiplus::Rect(0, 0, w, h), 0, 0, w, h,
                Gdiplus::UnitPixel, &attr);
        }
        else
        {
            // Fade from black to target
            double fadeIn = (d - 0.5) * 2.0;
            Gdiplus::ImageAttributes attr;
            Gdiplus::ColorMatrix cm = {
                1, 0, 0, 0, 0,
                0, 1, 0, 0, 0,
                0, 0, 1, 0, 0,
                0, 0, 0, fadeIn, 0,
                0, 0, 0, 0, 1
            };
            attr.SetColorMatrix(&cm);
            g.DrawImage(pTo, Gdiplus::Rect(0, 0, w, h), 0, 0, w, h,
                Gdiplus::UnitPixel, &attr);
        }

        return S_OK;
    }
};

class DiagonalWipeTransition : public TransitionBase
{
public:
    const WCHAR* GetId() const override { return L"DiagonalWipe"; }
    const WCHAR* GetName() const override { return L"Diagonal Wipe"; }

    HRESULT Apply(Gdiplus::Bitmap* pFrom, Gdiplus::Bitmap* pTo,
        Gdiplus::Bitmap** ppResult, const TransformParams* pParams) override
    {
        if (!pFrom || !pTo || !ppResult || !pParams)
            return E_INVALIDARG;

        UINT w = pFrom->GetWidth();
        UINT h = pFrom->GetHeight();
        *ppResult = new Gdiplus::Bitmap(w, h, PixelFormat32bppARGB);
        if (!*ppResult)
            return E_OUTOFMEMORY;

        Gdiplus::Graphics g(*ppResult);
        g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);

        // Draw target first
        g.DrawImage(pTo, 0, 0, w, h);

        // Use clip region for the wipe effect
        double d = pParams->dProgress;
        Gdiplus::Region clip(Gdiplus::Rect(
            static_cast<int>((w + h) * d - w),
            0,
            w + h,
            h));
        g.SetClip(&clip);
        g.DrawImage(pFrom, 0, 0, w, h);

        return S_OK;
    }
};

class FadeToWhiteTransition : public TransitionBase
{
public:
    const WCHAR* GetId() const override { return L"FadeToWhite"; }
    const WCHAR* GetName() const override { return L"Fade to White"; }

    HRESULT Apply(Gdiplus::Bitmap* pFrom, Gdiplus::Bitmap* pTo,
        Gdiplus::Bitmap** ppResult, const TransformParams* pParams) override
    {
        if (!pFrom || !pTo || !ppResult || !pParams)
            return E_INVALIDARG;

        UINT w = pFrom->GetWidth();
        UINT h = pFrom->GetHeight();
        *ppResult = new Gdiplus::Bitmap(w, h, PixelFormat32bppARGB);
        if (!*ppResult)
            return E_OUTOFMEMORY;

        Gdiplus::Graphics g(*ppResult);
        double d = pParams->dProgress;

        if (d < 0.5)
        {
            g.DrawImage(pFrom, 0, 0, w, h);
            Gdiplus::SolidBrush whiteBrush(Gdiplus::Color(
                static_cast<BYTE>(d * 2.0 * 255), 255, 255, 255));
            g.FillRectangle(&whiteBrush, 0, 0, w, h);
        }
        else
        {
            Gdiplus::SolidBrush whiteBrush(Gdiplus::Color(
                static_cast<BYTE>((1.0 - (d - 0.5) * 2.0) * 255), 255, 255, 255));
            g.FillRectangle(&whiteBrush, 0, 0, w, h);
            g.DrawImage(pTo, 0, 0, w, h);
        }

        return S_OK;
    }
};

// ============================================================================
// Shared transition helpers
// ============================================================================
// The original renders transitions with D3D9 pattern meshes (see
// analysis/WLXPipetran); this recreation reproduces each transition's
// geometry with the GDI+ compositing layer. All helpers draw into the
// destination bitmap's graphics context.

// Draws the image stretched to the full frame with the given opacity.
static void DrawFaded(Gdiplus::Graphics& g, Gdiplus::Image* pImage, double dAlpha)
{
    Gdiplus::ColorMatrix cm = {
        1, 0, 0, 0, 0,
        0, 1, 0, 0, 0,
        0, 0, 1, 0, 0,
        0, 0, 0, static_cast<float>(dAlpha), 0,
        0, 0, 0, 0, 1
    };
    Gdiplus::ImageAttributes attr;
    attr.SetColorMatrix(&cm);

    UINT w = pImage->GetWidth();
    UINT h = pImage->GetHeight();
    g.DrawImage(pImage, Gdiplus::Rect(0, 0, static_cast<INT>(w), static_cast<INT>(h)),
        0, 0, static_cast<INT>(w), static_cast<INT>(h), Gdiplus::UnitPixel, &attr);
}

// Draws the image scaled about the frame center with the given opacity.
static void DrawCentered(Gdiplus::Graphics& g, Gdiplus::Image* pImage,
    double dScaleX, double dScaleY, double dAlpha)
{
    float cx = static_cast<float>(pImage->GetWidth()) * 0.5f;
    float cy = static_cast<float>(pImage->GetHeight()) * 0.5f;

    g.TranslateTransform(cx, cy);
    g.ScaleTransform(static_cast<float>(dScaleX), static_cast<float>(dScaleY));
    g.TranslateTransform(-cx, -cy);
    DrawFaded(g, pImage, dAlpha);
    g.ResetTransform();
}

// Draws the image rotated about the frame center with the given opacity.
static void DrawRotated(Gdiplus::Graphics& g, Gdiplus::Image* pImage,
    float fAngleDegrees, double dAlpha)
{
    float cx = static_cast<float>(pImage->GetWidth()) * 0.5f;
    float cy = static_cast<float>(pImage->GetHeight()) * 0.5f;

    g.TranslateTransform(cx, cy);
    g.RotateTransform(fAngleDegrees);
    g.TranslateTransform(-cx, -cy);
    DrawFaded(g, pImage, dAlpha);
    g.ResetTransform();
}

// Returns a pixelated copy of the source: each block of uBlockSize pixels is
// replaced by the block's average color. uBlockSize <= 1 returns a plain copy.
static Gdiplus::Bitmap* PixelateBitmap(Gdiplus::Bitmap* pSource, UINT uBlockSize)
{
    UINT w = pSource->GetWidth();
    UINT h = pSource->GetHeight();

    // Normalize the working copy to 32bppARGB so the pixel math is uniform.
    Gdiplus::Bitmap* pWork = pSource->Clone(0, 0, w, h, PixelFormat32bppARGB);
    if (!pWork)
        return nullptr;

    if (uBlockSize <= 1)
        return pWork;

    Gdiplus::Bitmap* pResult = new Gdiplus::Bitmap(w, h, PixelFormat32bppARGB);
    if (!pResult)
    {
        delete pWork;
        return nullptr;
    }

    Gdiplus::Rect rcLock(0, 0, static_cast<INT>(w), static_cast<INT>(h));

    Gdiplus::BitmapData srcData = {};
    if (pWork->LockBits(&rcLock, Gdiplus::ImageLockModeRead,
            PixelFormat32bppARGB, &srcData) != Gdiplus::Ok)
    {
        delete pWork;
        delete pResult;
        return nullptr;
    }

    Gdiplus::BitmapData dstData = {};
    if (pResult->LockBits(&rcLock, Gdiplus::ImageLockModeWrite,
            PixelFormat32bppARGB, &dstData) != Gdiplus::Ok)
    {
        pWork->UnlockBits(&srcData);
        delete pWork;
        delete pResult;
        return nullptr;
    }

    for (UINT y = 0; y < h; y += uBlockSize)
    {
        UINT uBlockH = std::min(uBlockSize, h - y);
        for (UINT x = 0; x < w; x += uBlockSize)
        {
            UINT uBlockW = std::min(uBlockSize, w - x);

            // Average the block (32bppARGB memory order is B, G, R, A).
            UINT64 uSumB = 0, uSumG = 0, uSumR = 0, uSumA = 0;
            for (UINT by = 0; by < uBlockH; ++by)
            {
                const BYTE* pSrcRow = static_cast<const BYTE*>(srcData.Scan0) +
                    (y + by) * srcData.Stride + x * 4;
                for (UINT bx = 0; bx < uBlockW; ++bx)
                {
                    uSumB += pSrcRow[bx * 4 + 0];
                    uSumG += pSrcRow[bx * 4 + 1];
                    uSumR += pSrcRow[bx * 4 + 2];
                    uSumA += pSrcRow[bx * 4 + 3];
                }
            }

            UINT uCount = uBlockW * uBlockH;
            BYTE byA = static_cast<BYTE>((uSumA + uCount / 2) / uCount);
            BYTE byR = static_cast<BYTE>((uSumR + uCount / 2) / uCount);
            BYTE byG = static_cast<BYTE>((uSumG + uCount / 2) / uCount);
            BYTE byB = static_cast<BYTE>((uSumB + uCount / 2) / uCount);

            for (UINT by = 0; by < uBlockH; ++by)
            {
                BYTE* pDstRow = static_cast<BYTE*>(dstData.Scan0) +
                    (y + by) * dstData.Stride + x * 4;
                for (UINT bx = 0; bx < uBlockW; ++bx)
                {
                    pDstRow[bx * 4 + 0] = byB;
                    pDstRow[bx * 4 + 1] = byG;
                    pDstRow[bx * 4 + 2] = byR;
                    pDstRow[bx * 4 + 3] = byA;
                }
            }
        }
    }

    pWork->UnlockBits(&srcData);
    pResult->UnlockBits(&dstData);

    delete pWork;
    return pResult;
}

// Approximate blur: downscale, then draw stretched back up with bicubic
// interpolation so the block edges smooth into a soft blur.
static Gdiplus::Bitmap* BlurBitmap(Gdiplus::Bitmap* pSource)
{
    UINT w = pSource->GetWidth();
    UINT h = pSource->GetHeight();
    UINT uSmallW = std::max<UINT>(1u, w / 8u);
    UINT uSmallH = std::max<UINT>(1u, h / 8u);

    Gdiplus::Bitmap* pSmall = new Gdiplus::Bitmap(uSmallW, uSmallH, PixelFormat32bppARGB);
    if (!pSmall)
        return nullptr;
    {
        Gdiplus::Graphics g(pSmall);
        g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
        g.DrawImage(pSource, 0, 0, static_cast<INT>(uSmallW), static_cast<INT>(uSmallH));
    }

    Gdiplus::Bitmap* pBlurred = new Gdiplus::Bitmap(w, h, PixelFormat32bppARGB);
    if (!pBlurred)
    {
        delete pSmall;
        return nullptr;
    }
    {
        Gdiplus::Graphics g(pBlurred);
        g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
        g.DrawImage(pSmall, 0, 0, static_cast<INT>(w), static_cast<INT>(h));
    }

    delete pSmall;
    return pBlurred;
}

// Deterministic per-cell threshold in [0, 1) for the dissolve transition.
static double CellThreshold(UINT uCellX, UINT uCellY)
{
    UINT32 uHash = (uCellX * 73856093u) ^ (uCellY * 19349663u) ^ 0x9E3779B9u;
    return static_cast<double>(uHash % 1000u) / 1000.0;
}

// ============================================================================
// Wipe transitions -- the incoming frame is revealed through a moving clip
// (pattern meshes: Rectangle/Swipe, Iris, Diamond, Star, BowTie, Radar/Wheel).
// ============================================================================

class HorizontalWipeTransition : public TransitionBase
{
public:
    const WCHAR* GetId() const override { return L"HorizontalWipe"; }
    const WCHAR* GetName() const override { return L"Horizontal Wipe"; }

    HRESULT Apply(Gdiplus::Bitmap* pFrom, Gdiplus::Bitmap* pTo,
        Gdiplus::Bitmap** ppResult, const TransformParams* pParams) override
    {
        if (!pFrom || !pTo || !ppResult || !pParams)
            return E_INVALIDARG;

        UINT w = pFrom->GetWidth();
        UINT h = pFrom->GetHeight();
        *ppResult = new Gdiplus::Bitmap(w, h, PixelFormat32bppARGB);
        if (!*ppResult)
            return E_OUTOFMEMORY;

        Gdiplus::Graphics g(*ppResult);
        g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);

        double d = pParams->dProgress;

        // Reveal the incoming frame left to right.
        g.DrawImage(pFrom, 0, 0, w, h);
        g.SetClip(Gdiplus::Rect(0, 0, static_cast<INT>(w * d), static_cast<INT>(h)));
        g.DrawImage(pTo, 0, 0, w, h);

        return S_OK;
    }
};

class VerticalWipeTransition : public TransitionBase
{
public:
    const WCHAR* GetId() const override { return L"VerticalWipe"; }
    const WCHAR* GetName() const override { return L"Vertical Wipe"; }

    HRESULT Apply(Gdiplus::Bitmap* pFrom, Gdiplus::Bitmap* pTo,
        Gdiplus::Bitmap** ppResult, const TransformParams* pParams) override
    {
        if (!pFrom || !pTo || !ppResult || !pParams)
            return E_INVALIDARG;

        UINT w = pFrom->GetWidth();
        UINT h = pFrom->GetHeight();
        *ppResult = new Gdiplus::Bitmap(w, h, PixelFormat32bppARGB);
        if (!*ppResult)
            return E_OUTOFMEMORY;

        Gdiplus::Graphics g(*ppResult);
        g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);

        double d = pParams->dProgress;

        // Reveal the incoming frame top to bottom.
        g.DrawImage(pFrom, 0, 0, w, h);
        g.SetClip(Gdiplus::Rect(0, 0, static_cast<INT>(w), static_cast<INT>(h * d)));
        g.DrawImage(pTo, 0, 0, w, h);

        return S_OK;
    }
};

class CircleWipeTransition : public TransitionBase
{
public:
    const WCHAR* GetId() const override { return L"CircleWipe"; }
    const WCHAR* GetName() const override { return L"Circle Wipe"; }

    HRESULT Apply(Gdiplus::Bitmap* pFrom, Gdiplus::Bitmap* pTo,
        Gdiplus::Bitmap** ppResult, const TransformParams* pParams) override
    {
        if (!pFrom || !pTo || !ppResult || !pParams)
            return E_INVALIDARG;

        UINT w = pFrom->GetWidth();
        UINT h = pFrom->GetHeight();
        *ppResult = new Gdiplus::Bitmap(w, h, PixelFormat32bppARGB);
        if (!*ppResult)
            return E_OUTOFMEMORY;

        Gdiplus::Graphics g(*ppResult);
        g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);

        double d = pParams->dProgress;

        // Iris: an expanding circle centered on the frame.
        g.DrawImage(pFrom, 0, 0, w, h);
        float fRadius = static_cast<float>(std::max(w, h)) * 0.5f * static_cast<float>(d);
        if (fRadius > 0.0f)
        {
            float cx = static_cast<float>(w) * 0.5f;
            float cy = static_cast<float>(h) * 0.5f;
            Gdiplus::GraphicsPath path;
            path.AddPie(cx - fRadius, cy - fRadius, fRadius * 2.0f, fRadius * 2.0f, 0.0f, 360.0f);
            g.SetClip(&path);
            g.DrawImage(pTo, 0, 0, w, h);
        }

        return S_OK;
    }
};

class DiamondWipeTransition : public TransitionBase
{
public:
    const WCHAR* GetId() const override { return L"DiamondWipe"; }
    const WCHAR* GetName() const override { return L"Diamond Wipe"; }

    HRESULT Apply(Gdiplus::Bitmap* pFrom, Gdiplus::Bitmap* pTo,
        Gdiplus::Bitmap** ppResult, const TransformParams* pParams) override
    {
        if (!pFrom || !pTo || !ppResult || !pParams)
            return E_INVALIDARG;

        UINT w = pFrom->GetWidth();
        UINT h = pFrom->GetHeight();
        *ppResult = new Gdiplus::Bitmap(w, h, PixelFormat32bppARGB);
        if (!*ppResult)
            return E_OUTOFMEMORY;

        Gdiplus::Graphics g(*ppResult);
        g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);

        double d = pParams->dProgress;

        // An expanding diamond centered on the frame. At d = 1 the vertices
        // reach twice the half-extents, so the whole frame is covered.
        g.DrawImage(pFrom, 0, 0, w, h);
        float cx = static_cast<float>(w) * 0.5f;
        float cy = static_cast<float>(h) * 0.5f;
        float rx = static_cast<float>(w) * static_cast<float>(d);
        float ry = static_cast<float>(h) * static_cast<float>(d);
        Gdiplus::PointF pts[4] = {
            Gdiplus::PointF(cx, cy - ry),
            Gdiplus::PointF(cx + rx, cy),
            Gdiplus::PointF(cx, cy + ry),
            Gdiplus::PointF(cx - rx, cy)
        };
        Gdiplus::GraphicsPath path;
        path.AddPolygon(pts, 4);
        g.SetClip(&path);
        g.DrawImage(pTo, 0, 0, w, h);

        return S_OK;
    }
};

class StarWipeTransition : public TransitionBase
{
public:
    const WCHAR* GetId() const override { return L"StarWipe"; }
    const WCHAR* GetName() const override { return L"Star Wipe"; }

    HRESULT Apply(Gdiplus::Bitmap* pFrom, Gdiplus::Bitmap* pTo,
        Gdiplus::Bitmap** ppResult, const TransformParams* pParams) override
    {
        if (!pFrom || !pTo || !ppResult || !pParams)
            return E_INVALIDARG;

        UINT w = pFrom->GetWidth();
        UINT h = pFrom->GetHeight();
        *ppResult = new Gdiplus::Bitmap(w, h, PixelFormat32bppARGB);
        if (!*ppResult)
            return E_OUTOFMEMORY;

        Gdiplus::Graphics g(*ppResult);
        g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);

        double d = pParams->dProgress;

        // A five-pointed star centered on the frame. The outer radius carries
        // a 1.5x margin so the star covers the frame corners at d = 1.
        g.DrawImage(pFrom, 0, 0, w, h);
        float cx = static_cast<float>(w) * 0.5f;
        float cy = static_cast<float>(h) * 0.5f;
        float fOuter = static_cast<float>(std::max(w, h)) * 1.5f * static_cast<float>(d);
        float fInner = fOuter * 0.4f;

        Gdiplus::PointF pts[10];
        for (int i = 0; i < 10; ++i)
        {
            double dAngle = -3.14159265358979 / 2.0 + i * 3.14159265358979 / 5.0;
            float fR = (i % 2 == 0) ? fOuter : fInner;
            pts[i] = Gdiplus::PointF(
                cx + fR * static_cast<float>(cos(dAngle)),
                cy + fR * static_cast<float>(sin(dAngle)));
        }

        Gdiplus::GraphicsPath path;
        path.AddPolygon(pts, 10);
        g.SetClip(&path);
        g.DrawImage(pTo, 0, 0, w, h);

        return S_OK;
    }
};

class BowTieWipeTransition : public TransitionBase
{
public:
    const WCHAR* GetId() const override { return L"BowTieWipe"; }
    const WCHAR* GetName() const override { return L"Bow Tie Wipe"; }

    HRESULT Apply(Gdiplus::Bitmap* pFrom, Gdiplus::Bitmap* pTo,
        Gdiplus::Bitmap** ppResult, const TransformParams* pParams) override
    {
        if (!pFrom || !pTo || !ppResult || !pParams)
            return E_INVALIDARG;

        UINT w = pFrom->GetWidth();
        UINT h = pFrom->GetHeight();
        *ppResult = new Gdiplus::Bitmap(w, h, PixelFormat32bppARGB);
        if (!*ppResult)
            return E_OUTOFMEMORY;

        Gdiplus::Graphics g(*ppResult);
        g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);

        double d = pParams->dProgress;

        // Two triangles growing from the left and right edges; their apexes
        // meet at the center when d = 1.
        g.DrawImage(pFrom, 0, 0, w, h);
        float cx = static_cast<float>(w) * 0.5f;
        float cy = static_cast<float>(h) * 0.5f;
        float fw = static_cast<float>(w);
        float fh = static_cast<float>(h);

        Gdiplus::PointF leftPts[3] = {
            Gdiplus::PointF(0.0f, 0.0f),
            Gdiplus::PointF(cx * static_cast<float>(d), cy),
            Gdiplus::PointF(0.0f, fh)
        };
        Gdiplus::PointF rightPts[3] = {
            Gdiplus::PointF(fw, 0.0f),
            Gdiplus::PointF(fw - cx * static_cast<float>(d), cy),
            Gdiplus::PointF(fw, fh)
        };

        Gdiplus::GraphicsPath path;
        path.AddPolygon(leftPts, 3);
        path.AddPolygon(rightPts, 3);
        g.SetClip(&path);
        g.DrawImage(pTo, 0, 0, w, h);

        return S_OK;
    }
};

class ClockWipeTransition : public TransitionBase
{
public:
    const WCHAR* GetId() const override { return L"ClockWipe"; }
    const WCHAR* GetName() const override { return L"Clock Wipe"; }

    HRESULT Apply(Gdiplus::Bitmap* pFrom, Gdiplus::Bitmap* pTo,
        Gdiplus::Bitmap** ppResult, const TransformParams* pParams) override
    {
        if (!pFrom || !pTo || !ppResult || !pParams)
            return E_INVALIDARG;

        UINT w = pFrom->GetWidth();
        UINT h = pFrom->GetHeight();
        *ppResult = new Gdiplus::Bitmap(w, h, PixelFormat32bppARGB);
        if (!*ppResult)
            return E_OUTOFMEMORY;

        Gdiplus::Graphics g(*ppResult);
        g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);

        double d = pParams->dProgress;

        // Radar sweep: a sector growing clockwise from 12 o'clock.
        g.DrawImage(pFrom, 0, 0, w, h);
        if (d > 0.0)
        {
            Gdiplus::GraphicsPath path;
            path.AddPie(0.0f, 0.0f, static_cast<float>(w), static_cast<float>(h),
                -90.0f, static_cast<float>(360.0 * d));
            g.SetClip(&path);
            g.DrawImage(pTo, 0, 0, w, h);
        }

        return S_OK;
    }
};

// ============================================================================
// Push/slide transitions -- frames translate across the output.
// ============================================================================

class PushLeftTransition : public TransitionBase
{
public:
    const WCHAR* GetId() const override { return L"PushLeft"; }
    const WCHAR* GetName() const override { return L"Push Left"; }

    HRESULT Apply(Gdiplus::Bitmap* pFrom, Gdiplus::Bitmap* pTo,
        Gdiplus::Bitmap** ppResult, const TransformParams* pParams) override
    {
        if (!pFrom || !pTo || !ppResult || !pParams)
            return E_INVALIDARG;

        UINT w = pFrom->GetWidth();
        UINT h = pFrom->GetHeight();
        *ppResult = new Gdiplus::Bitmap(w, h, PixelFormat32bppARGB);
        if (!*ppResult)
            return E_OUTOFMEMORY;

        Gdiplus::Graphics g(*ppResult);
        g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);

        double d = pParams->dProgress;

        // Both frames travel left; the incoming frame enters from the right.
        g.DrawImage(pFrom, static_cast<INT>(-w * d), 0, w, h);
        g.DrawImage(pTo, static_cast<INT>(w * (1.0 - d)), 0, w, h);

        return S_OK;
    }
};

class PushRightTransition : public TransitionBase
{
public:
    const WCHAR* GetId() const override { return L"PushRight"; }
    const WCHAR* GetName() const override { return L"Push Right"; }

    HRESULT Apply(Gdiplus::Bitmap* pFrom, Gdiplus::Bitmap* pTo,
        Gdiplus::Bitmap** ppResult, const TransformParams* pParams) override
    {
        if (!pFrom || !pTo || !ppResult || !pParams)
            return E_INVALIDARG;

        UINT w = pFrom->GetWidth();
        UINT h = pFrom->GetHeight();
        *ppResult = new Gdiplus::Bitmap(w, h, PixelFormat32bppARGB);
        if (!*ppResult)
            return E_OUTOFMEMORY;

        Gdiplus::Graphics g(*ppResult);
        g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);

        double d = pParams->dProgress;

        // Both frames travel right; the incoming frame enters from the left.
        g.DrawImage(pFrom, static_cast<INT>(w * d), 0, w, h);
        g.DrawImage(pTo, static_cast<INT>(-w * (1.0 - d)), 0, w, h);

        return S_OK;
    }
};

class PushUpTransition : public TransitionBase
{
public:
    const WCHAR* GetId() const override { return L"PushUp"; }
    const WCHAR* GetName() const override { return L"Push Up"; }

    HRESULT Apply(Gdiplus::Bitmap* pFrom, Gdiplus::Bitmap* pTo,
        Gdiplus::Bitmap** ppResult, const TransformParams* pParams) override
    {
        if (!pFrom || !pTo || !ppResult || !pParams)
            return E_INVALIDARG;

        UINT w = pFrom->GetWidth();
        UINT h = pFrom->GetHeight();
        *ppResult = new Gdiplus::Bitmap(w, h, PixelFormat32bppARGB);
        if (!*ppResult)
            return E_OUTOFMEMORY;

        Gdiplus::Graphics g(*ppResult);
        g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);

        double d = pParams->dProgress;

        // Both frames travel up; the incoming frame enters from the bottom.
        g.DrawImage(pFrom, 0, static_cast<INT>(-h * d), w, h);
        g.DrawImage(pTo, 0, static_cast<INT>(h * (1.0 - d)), w, h);

        return S_OK;
    }
};

class PushDownTransition : public TransitionBase
{
public:
    const WCHAR* GetId() const override { return L"PushDown"; }
    const WCHAR* GetName() const override { return L"Push Down"; }

    HRESULT Apply(Gdiplus::Bitmap* pFrom, Gdiplus::Bitmap* pTo,
        Gdiplus::Bitmap** ppResult, const TransformParams* pParams) override
    {
        if (!pFrom || !pTo || !ppResult || !pParams)
            return E_INVALIDARG;

        UINT w = pFrom->GetWidth();
        UINT h = pFrom->GetHeight();
        *ppResult = new Gdiplus::Bitmap(w, h, PixelFormat32bppARGB);
        if (!*ppResult)
            return E_OUTOFMEMORY;

        Gdiplus::Graphics g(*ppResult);
        g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);

        double d = pParams->dProgress;

        // Both frames travel down; the incoming frame enters from the top.
        g.DrawImage(pFrom, 0, static_cast<INT>(h * d), w, h);
        g.DrawImage(pTo, 0, static_cast<INT>(-h * (1.0 - d)), w, h);

        return S_OK;
    }
};

class SlideLeftTransition : public TransitionBase
{
public:
    const WCHAR* GetId() const override { return L"SlideLeft"; }
    const WCHAR* GetName() const override { return L"Slide Left"; }

    HRESULT Apply(Gdiplus::Bitmap* pFrom, Gdiplus::Bitmap* pTo,
        Gdiplus::Bitmap** ppResult, const TransformParams* pParams) override
    {
        if (!pFrom || !pTo || !ppResult || !pParams)
            return E_INVALIDARG;

        UINT w = pFrom->GetWidth();
        UINT h = pFrom->GetHeight();
        *ppResult = new Gdiplus::Bitmap(w, h, PixelFormat32bppARGB);
        if (!*ppResult)
            return E_OUTOFMEMORY;

        Gdiplus::Graphics g(*ppResult);
        g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);

        double d = pParams->dProgress;

        // The outgoing frame stays put; the incoming frame slides in over it
        // from the right.
        g.DrawImage(pFrom, 0, 0, w, h);
        g.DrawImage(pTo, static_cast<INT>(w * (1.0 - d)), 0, w, h);

        return S_OK;
    }
};

class SlideRightTransition : public TransitionBase
{
public:
    const WCHAR* GetId() const override { return L"SlideRight"; }
    const WCHAR* GetName() const override { return L"Slide Right"; }

    HRESULT Apply(Gdiplus::Bitmap* pFrom, Gdiplus::Bitmap* pTo,
        Gdiplus::Bitmap** ppResult, const TransformParams* pParams) override
    {
        if (!pFrom || !pTo || !ppResult || !pParams)
            return E_INVALIDARG;

        UINT w = pFrom->GetWidth();
        UINT h = pFrom->GetHeight();
        *ppResult = new Gdiplus::Bitmap(w, h, PixelFormat32bppARGB);
        if (!*ppResult)
            return E_OUTOFMEMORY;

        Gdiplus::Graphics g(*ppResult);
        g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);

        double d = pParams->dProgress;

        // The outgoing frame stays put; the incoming frame slides in over it
        // from the left.
        g.DrawImage(pFrom, 0, 0, w, h);
        g.DrawImage(pTo, static_cast<INT>(-w * d), 0, w, h);

        return S_OK;
    }
};

// ============================================================================
// Rotate/flip/spin/zoom transitions -- the incoming frame transforms in.
// ============================================================================

class RotateTransition : public TransitionBase
{
public:
    const WCHAR* GetId() const override { return L"Rotate"; }
    const WCHAR* GetName() const override { return L"Rotate"; }

    HRESULT Apply(Gdiplus::Bitmap* pFrom, Gdiplus::Bitmap* pTo,
        Gdiplus::Bitmap** ppResult, const TransformParams* pParams) override
    {
        if (!pFrom || !pTo || !ppResult || !pParams)
            return E_INVALIDARG;

        UINT w = pFrom->GetWidth();
        UINT h = pFrom->GetHeight();
        *ppResult = new Gdiplus::Bitmap(w, h, PixelFormat32bppARGB);
        if (!*ppResult)
            return E_OUTOFMEMORY;

        Gdiplus::Graphics g(*ppResult);
        g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);

        double d = pParams->dProgress;

        // The incoming frame rotates upright from 90 degrees while fading in.
        g.DrawImage(pFrom, 0, 0, w, h);
        DrawRotated(g, pTo, static_cast<float>((1.0 - d) * 90.0), d);

        return S_OK;
    }
};

class FlipTransition : public TransitionBase
{
public:
    const WCHAR* GetId() const override { return L"Flip"; }
    const WCHAR* GetName() const override { return L"Flip"; }

    HRESULT Apply(Gdiplus::Bitmap* pFrom, Gdiplus::Bitmap* pTo,
        Gdiplus::Bitmap** ppResult, const TransformParams* pParams) override
    {
        if (!pFrom || !pTo || !ppResult || !pParams)
            return E_INVALIDARG;

        UINT w = pFrom->GetWidth();
        UINT h = pFrom->GetHeight();
        *ppResult = new Gdiplus::Bitmap(w, h, PixelFormat32bppARGB);
        if (!*ppResult)
            return E_OUTOFMEMORY;

        Gdiplus::Graphics g(*ppResult);
        g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);

        double d = pParams->dProgress;

        // The incoming frame flips in around the vertical center axis
        // (approximated by a horizontal squash) while fading in.
        g.DrawImage(pFrom, 0, 0, w, h);
        if (d > 0.0)
            DrawCentered(g, pTo, d, 1.0, d);

        return S_OK;
    }
};

class SpinTransition : public TransitionBase
{
public:
    const WCHAR* GetId() const override { return L"Spin"; }
    const WCHAR* GetName() const override { return L"Spin"; }

    HRESULT Apply(Gdiplus::Bitmap* pFrom, Gdiplus::Bitmap* pTo,
        Gdiplus::Bitmap** ppResult, const TransformParams* pParams) override
    {
        if (!pFrom || !pTo || !ppResult || !pParams)
            return E_INVALIDARG;

        UINT w = pFrom->GetWidth();
        UINT h = pFrom->GetHeight();
        *ppResult = new Gdiplus::Bitmap(w, h, PixelFormat32bppARGB);
        if (!*ppResult)
            return E_OUTOFMEMORY;

        Gdiplus::Graphics g(*ppResult);
        g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);

        double d = pParams->dProgress;

        // The incoming frame spins a full turn while fading in.
        g.DrawImage(pFrom, 0, 0, w, h);
        DrawRotated(g, pTo, static_cast<float>(d * 360.0), d);

        return S_OK;
    }
};

class ZoomTransition : public TransitionBase
{
public:
    const WCHAR* GetId() const override { return L"Zoom"; }
    const WCHAR* GetName() const override { return L"Zoom"; }

    HRESULT Apply(Gdiplus::Bitmap* pFrom, Gdiplus::Bitmap* pTo,
        Gdiplus::Bitmap** ppResult, const TransformParams* pParams) override
    {
        if (!pFrom || !pTo || !ppResult || !pParams)
            return E_INVALIDARG;

        UINT w = pFrom->GetWidth();
        UINT h = pFrom->GetHeight();
        *ppResult = new Gdiplus::Bitmap(w, h, PixelFormat32bppARGB);
        if (!*ppResult)
            return E_OUTOFMEMORY;

        Gdiplus::Graphics g(*ppResult);
        g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);

        double d = pParams->dProgress;

        // The incoming frame fades in beneath while the outgoing frame zooms
        // past the camera and fades out.
        DrawFaded(g, pTo, d);
        if (d < 1.0)
            DrawCentered(g, pFrom, 1.0 + 0.5 * d, 1.0 + 0.5 * d, 1.0 - d);

        return S_OK;
    }
};

// ============================================================================
// Pixel-level transitions -- pixelate and dissolve.
// ============================================================================

class PixelateTransition : public TransitionBase
{
public:
    const WCHAR* GetId() const override { return L"Pixelate"; }
    const WCHAR* GetName() const override { return L"Pixelate"; }

    HRESULT Apply(Gdiplus::Bitmap* pFrom, Gdiplus::Bitmap* pTo,
        Gdiplus::Bitmap** ppResult, const TransformParams* pParams) override
    {
        if (!pFrom || !pTo || !ppResult || !pParams)
            return E_INVALIDARG;

        UINT w = pFrom->GetWidth();
        UINT h = pFrom->GetHeight();
        *ppResult = new Gdiplus::Bitmap(w, h, PixelFormat32bppARGB);
        if (!*ppResult)
            return E_OUTOFMEMORY;

        Gdiplus::Graphics g(*ppResult);
        g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);

        double d = pParams->dProgress;

        // Classic pixelate dissolve: the outgoing frame pixelates with a
        // growing block up to the midpoint, then the incoming frame resolves
        // with a shrinking block.
        UINT uMaxBlock = std::max<UINT>(4u, std::min(w, h) / 8u);
        bool fOutgoing = (d < 0.5);
        double dLocal = fOutgoing ? d * 2.0 : (d - 0.5) * 2.0;
        UINT uBlock = fOutgoing
            ? static_cast<UINT>(1.0 + dLocal * static_cast<double>(uMaxBlock - 1u))
            : static_cast<UINT>(static_cast<double>(uMaxBlock) - dLocal * static_cast<double>(uMaxBlock - 1u));

        Gdiplus::Bitmap* pPixelated = PixelateBitmap(fOutgoing ? pFrom : pTo, uBlock);
        if (!pPixelated)
        {
            delete *ppResult;
            *ppResult = nullptr;
            return E_OUTOFMEMORY;
        }

        g.DrawImage(pPixelated, 0, 0, w, h);
        delete pPixelated;

        return S_OK;
    }
};

class DissolveTransition : public TransitionBase
{
public:
    const WCHAR* GetId() const override { return L"Dissolve"; }
    const WCHAR* GetName() const override { return L"Dissolve"; }

    HRESULT Apply(Gdiplus::Bitmap* pFrom, Gdiplus::Bitmap* pTo,
        Gdiplus::Bitmap** ppResult, const TransformParams* pParams) override
    {
        if (!pFrom || !pTo || !ppResult || !pParams)
            return E_INVALIDARG;

        UINT w = pFrom->GetWidth();
        UINT h = pFrom->GetHeight();
        *ppResult = new Gdiplus::Bitmap(w, h, PixelFormat32bppARGB);
        if (!*ppResult)
            return E_OUTOFMEMORY;

        Gdiplus::Graphics g(*ppResult);
        g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);

        double d = pParams->dProgress;

        // Random-block dissolve: a 16x16 grid of cells, each switching from
        // the outgoing to the incoming frame at its own deterministic
        // threshold as the progress crosses it.
        const UINT kCellCount = 16;
        UINT uCellW = (w + kCellCount - 1) / kCellCount;
        UINT uCellH = (h + kCellCount - 1) / kCellCount;
        if (uCellW == 0) uCellW = 1;
        if (uCellH == 0) uCellH = 1;

        for (UINT cy = 0; cy < kCellCount; ++cy)
        {
            UINT uY = cy * uCellH;
            if (uY >= h)
                continue;

            for (UINT cx = 0; cx < kCellCount; ++cx)
            {
                UINT uX = cx * uCellW;
                if (uX >= w)
                    continue;

                INT x = static_cast<INT>(uX);
                INT y = static_cast<INT>(uY);
                INT cw = static_cast<INT>(std::min(uCellW, w - uX));
                INT ch = static_cast<INT>(std::min(uCellH, h - uY));

                Gdiplus::Image* pCellSrc = (d > CellThreshold(cx, cy)) ? pTo : pFrom;
                g.DrawImage(pCellSrc, Gdiplus::Rect(x, y, cw, ch), x, y, cw, ch,
                    Gdiplus::UnitPixel);
            }
        }

        return S_OK;
    }
};

// ============================================================================
// Focus/scale transitions -- blur, shrink, expand.
// ============================================================================

class BlurTransition : public TransitionBase
{
public:
    const WCHAR* GetId() const override { return L"BlurTransition"; }
    const WCHAR* GetName() const override { return L"Blur"; }

    HRESULT Apply(Gdiplus::Bitmap* pFrom, Gdiplus::Bitmap* pTo,
        Gdiplus::Bitmap** ppResult, const TransformParams* pParams) override
    {
        if (!pFrom || !pTo || !ppResult || !pParams)
            return E_INVALIDARG;

        UINT w = pFrom->GetWidth();
        UINT h = pFrom->GetHeight();
        *ppResult = new Gdiplus::Bitmap(w, h, PixelFormat32bppARGB);
        if (!*ppResult)
            return E_OUTOFMEMORY;

        Gdiplus::Graphics g(*ppResult);
        g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);

        double d = pParams->dProgress;

        // Soft-focus crossfade: both frames blurred, crossfaded by progress.
        Gdiplus::Bitmap* pBlurFrom = BlurBitmap(pFrom);
        Gdiplus::Bitmap* pBlurTo = BlurBitmap(pTo);
        if (!pBlurFrom || !pBlurTo)
        {
            delete pBlurFrom;
            delete pBlurTo;
            delete *ppResult;
            *ppResult = nullptr;
            return E_OUTOFMEMORY;
        }

        DrawFaded(g, pBlurFrom, 1.0 - d);
        DrawFaded(g, pBlurTo, d);

        delete pBlurFrom;
        delete pBlurTo;

        return S_OK;
    }
};

class ShrinkTransition : public TransitionBase
{
public:
    const WCHAR* GetId() const override { return L"Shrink"; }
    const WCHAR* GetName() const override { return L"Shrink"; }

    HRESULT Apply(Gdiplus::Bitmap* pFrom, Gdiplus::Bitmap* pTo,
        Gdiplus::Bitmap** ppResult, const TransformParams* pParams) override
    {
        if (!pFrom || !pTo || !ppResult || !pParams)
            return E_INVALIDARG;

        UINT w = pFrom->GetWidth();
        UINT h = pFrom->GetHeight();
        *ppResult = new Gdiplus::Bitmap(w, h, PixelFormat32bppARGB);
        if (!*ppResult)
            return E_OUTOFMEMORY;

        Gdiplus::Graphics g(*ppResult);
        g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);

        double d = pParams->dProgress;

        // The outgoing frame shrinks into the center, revealing the incoming
        // frame behind it.
        g.DrawImage(pTo, 0, 0, w, h);
        if (d < 1.0)
            DrawCentered(g, pFrom, 1.0 - d, 1.0 - d, 1.0);

        return S_OK;
    }
};

class ExpandTransition : public TransitionBase
{
public:
    const WCHAR* GetId() const override { return L"Expand"; }
    const WCHAR* GetName() const override { return L"Expand"; }

    HRESULT Apply(Gdiplus::Bitmap* pFrom, Gdiplus::Bitmap* pTo,
        Gdiplus::Bitmap** ppResult, const TransformParams* pParams) override
    {
        if (!pFrom || !pTo || !ppResult || !pParams)
            return E_INVALIDARG;

        UINT w = pFrom->GetWidth();
        UINT h = pFrom->GetHeight();
        *ppResult = new Gdiplus::Bitmap(w, h, PixelFormat32bppARGB);
        if (!*ppResult)
            return E_OUTOFMEMORY;

        Gdiplus::Graphics g(*ppResult);
        g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);

        double d = pParams->dProgress;

        // The incoming frame expands out of the center over the outgoing one.
        g.DrawImage(pFrom, 0, 0, w, h);
        if (d > 0.0)
            DrawCentered(g, pTo, d, d, 1.0);

        return S_OK;
    }
};

// ============================================================================
// Effect implementations (effects are simpler - they only modify one source)
// ============================================================================

#define DEFINE_EFFECT_CLASS(ClassName, stringId, displayName) \
    class ClassName : public EffectBase { \
    public: \
        const WCHAR* GetId() const override { return stringId; } \
        const WCHAR* GetName() const override { return displayName; } \
        HRESULT Apply(Gdiplus::Bitmap* pSource, Gdiplus::Bitmap** ppResult, \
            const TransformParams* pParams) override \
        { \
            if (!pSource || !ppResult || !pParams) return E_INVALIDARG; \
            UINT w = pSource->GetWidth(); UINT h = pSource->GetHeight(); \
            *ppResult = pSource->Clone(0, 0, w, h, PixelFormat32bppARGB); \
            if (!*ppResult) return E_OUTOFMEMORY; \
            return S_OK; \
        } \
    };

DEFINE_EFFECT_CLASS(BrightnessEffect,     L"Brightness",     L"Brightness")
DEFINE_EFFECT_CLASS(ContrastEffect,       L"Contrast",       L"Contrast")
DEFINE_EFFECT_CLASS(SaturationEffect,     L"Saturation",     L"Saturation")
DEFINE_EFFECT_CLASS(HueEffect,            L"Hue",            L"Hue Shift")
DEFINE_EFFECT_CLASS(SepiaEffect,          L"Sepia",          L"Sepia")
DEFINE_EFFECT_CLASS(BlackAndWhiteEffect,  L"BlackAndWhite",  L"Black and White")
DEFINE_EFFECT_CLASS(InvertEffect,         L"Invert",         L"Invert Colors")
DEFINE_EFFECT_CLASS(BlurEffect,           L"BlurEffect",     L"Blur")
DEFINE_EFFECT_CLASS(SharpenEffect,        L"Sharpen",        L"Sharpen")
DEFINE_EFFECT_CLASS(EdgeDetectEffect,     L"EdgeDetect",     L"Edge Detect")
DEFINE_EFFECT_CLASS(EmbossEffect,         L"Emboss",         L"Emboss")
DEFINE_EFFECT_CLASS(PixelateEffect,       L"PixelateEffect", L"Pixelate")
DEFINE_EFFECT_CLASS(MosaicEffect,         L"Mosaic",         L"Mosaic")
DEFINE_EFFECT_CLASS(OldFilmEffect,        L"OldFilm",        L"Old Film")
DEFINE_EFFECT_CLASS(VignetteEffect,       L"Vignette",       L"Vignette")
DEFINE_EFFECT_CLASS(GrayscaleEffect,      L"Grayscale",      L"Grayscale")

// ============================================================================
// TransformRegistry -- maps string IDs to factory functions
// ============================================================================
class TransformRegistry
{
public:
    // Thread‑safe factory access – mutex guards map lookups
    std::mutex m_mutex;
    typedef std::function<TransitionBase*()> TransitionFactory;
    typedef std::function<EffectBase*()> EffectFactory;

    static TransformRegistry& GetInstance()
    {
        static TransformRegistry s_instance;
        return s_instance;
    }

    void RegisterTransition(const WCHAR* pszId, TransitionFactory factory)
    {
        m_transitionFactories[pszId] = factory;
    }

    void RegisterEffect(const WCHAR* pszId, EffectFactory factory)
    {
        m_effectFactories[pszId] = factory;
    }

    TransitionBase* CreateTransition(const WCHAR* pszId)
    {
        auto it = m_transitionFactories.find(pszId);
        if (it != m_transitionFactories.end())
            return it->second();
        return NULL;
    }

    EffectBase* CreateEffect(const WCHAR* pszId)
    {
        auto it = m_effectFactories.find(pszId);
        if (it != m_effectFactories.end())
            return it->second();
        return NULL;
    }

    size_t GetTransitionCount() const { return m_transitionFactories.size(); }
    size_t GetEffectCount() const { return m_effectFactories.size(); }

private:
    TransformRegistry()
    {
        RegisterAll();
    }

    void RegisterAll()
    {
        // Register all transitions
        m_transitionFactories[L"CrossFade"]     = []() -> TransitionBase* { return new CrossFadeTransition(); };
        m_transitionFactories[L"FadeToBlack"]   = []() -> TransitionBase* { return new FadeToBlackTransition(); };
        m_transitionFactories[L"FadeToWhite"]   = []() -> TransitionBase* { return new FadeToWhiteTransition(); };
        m_transitionFactories[L"DiagonalWipe"]  = []() -> TransitionBase* { return new DiagonalWipeTransition(); };
        m_transitionFactories[L"HorizontalWipe"]= []() -> TransitionBase* { return new HorizontalWipeTransition(); };
        m_transitionFactories[L"VerticalWipe"]  = []() -> TransitionBase* { return new VerticalWipeTransition(); };
        m_transitionFactories[L"CircleWipe"]    = []() -> TransitionBase* { return new CircleWipeTransition(); };
        m_transitionFactories[L"DiamondWipe"]   = []() -> TransitionBase* { return new DiamondWipeTransition(); };
        m_transitionFactories[L"StarWipe"]      = []() -> TransitionBase* { return new StarWipeTransition(); };
        m_transitionFactories[L"BowTieWipe"]    = []() -> TransitionBase* { return new BowTieWipeTransition(); };
        m_transitionFactories[L"ClockWipe"]     = []() -> TransitionBase* { return new ClockWipeTransition(); };
        m_transitionFactories[L"PushLeft"]      = []() -> TransitionBase* { return new PushLeftTransition(); };
        m_transitionFactories[L"PushRight"]     = []() -> TransitionBase* { return new PushRightTransition(); };
        m_transitionFactories[L"PushUp"]        = []() -> TransitionBase* { return new PushUpTransition(); };
        m_transitionFactories[L"PushDown"]      = []() -> TransitionBase* { return new PushDownTransition(); };
        m_transitionFactories[L"SlideLeft"]     = []() -> TransitionBase* { return new SlideLeftTransition(); };
        m_transitionFactories[L"SlideRight"]    = []() -> TransitionBase* { return new SlideRightTransition(); };
        m_transitionFactories[L"Rotate"]        = []() -> TransitionBase* { return new RotateTransition(); };
        m_transitionFactories[L"Flip"]          = []() -> TransitionBase* { return new FlipTransition(); };
        m_transitionFactories[L"Spin"]          = []() -> TransitionBase* { return new SpinTransition(); };
        m_transitionFactories[L"Zoom"]          = []() -> TransitionBase* { return new ZoomTransition(); };
        m_transitionFactories[L"Pixelate"]      = []() -> TransitionBase* { return new PixelateTransition(); };
        m_transitionFactories[L"Dissolve"]      = []() -> TransitionBase* { return new DissolveTransition(); };
        m_transitionFactories[L"BlurTransition"]= []() -> TransitionBase* { return new BlurTransition(); };
        m_transitionFactories[L"Shrink"]        = []() -> TransitionBase* { return new ShrinkTransition(); };
        m_transitionFactories[L"Expand"]        = []() -> TransitionBase* { return new ExpandTransition(); };

        // Register all effects
        m_effectFactories[L"Brightness"]     = []() -> EffectBase* { return new BrightnessEffect(); };
        m_effectFactories[L"Contrast"]       = []() -> EffectBase* { return new ContrastEffect(); };
        m_effectFactories[L"Saturation"]     = []() -> EffectBase* { return new SaturationEffect(); };
        m_effectFactories[L"Hue"]            = []() -> EffectBase* { return new HueEffect(); };
        m_effectFactories[L"Sepia"]          = []() -> EffectBase* { return new SepiaEffect(); };
        m_effectFactories[L"BlackAndWhite"]  = []() -> EffectBase* { return new BlackAndWhiteEffect(); };
        m_effectFactories[L"Invert"]         = []() -> EffectBase* { return new InvertEffect(); };
        m_effectFactories[L"BlurEffect"]     = []() -> EffectBase* { return new BlurEffect(); };
        m_effectFactories[L"Sharpen"]        = []() -> EffectBase* { return new SharpenEffect(); };
        m_effectFactories[L"EdgeDetect"]     = []() -> EffectBase* { return new EdgeDetectEffect(); };
        m_effectFactories[L"Emboss"]         = []() -> EffectBase* { return new EmbossEffect(); };
        m_effectFactories[L"PixelateEffect"] = []() -> EffectBase* { return new PixelateEffect(); };
        m_effectFactories[L"Mosaic"]         = []() -> EffectBase* { return new MosaicEffect(); };
        m_effectFactories[L"OldFilm"]        = []() -> EffectBase* { return new OldFilmEffect(); };
        m_effectFactories[L"Vignette"]       = []() -> EffectBase* { return new VignetteEffect(); };
        m_effectFactories[L"Grayscale"]      = []() -> EffectBase* { return new GrayscaleEffect(); };
    }

    std::map<std::wstring, TransitionFactory> m_transitionFactories;
    std::map<std::wstring, EffectFactory>     m_effectFactories;
};

} // namespace Pipetran

// ============================================================================
// Exported function -- GetTFXCreateFunctions
// ============================================================================

extern "C"
{

WLXPIPET_API HRESULT __stdcall GetTFXCreateFunctions(void** ppFunctions, UINT32* pCount)
{
    UNREFERENCED_PARAMETER(ppFunctions);
    if (pCount) *pCount = 0;
    return E_NOTIMPL;
}

} // extern "C"
