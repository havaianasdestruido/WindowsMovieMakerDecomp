/*
 * SundanceMainVisual.cpp
 *
 * GDI implementation of the Sundance main-window shell. All geometry and
 * colors were measured against reference captures of the original
 * 16.4.3528.0331 binary and are shared with website/replica/moviemaker.html
 * (the pixel-verified reconstruction). Rendering is fully double-buffered;
 * the ribbon layout below mirrors the Home tab of the original: Clipboard,
 * Add, AutoMovie themes, Editing, Share, Save movie / Sign in.
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "pch.h"
#include "SundanceMainVisual.h"

#include "../SundanceApp/SundanceAppMain.h"
#include "../StoryboardManager/MovieProject.h"

#include <windowsx.h>
#include <strsafe.h>
#include <vector>

namespace SundanceUI {

// ============================================================================
// Geometry (96 DPI) -- measured from reference captures
// ============================================================================
namespace {

const int kTabStripH   = 23;   // tab band ("File | Home | ...")
const int kRibbonBodyH = 84;   // ribbon groups area
const int kStatusBarH  = 23;
const int kSplitterW   = 4;
const int kFilmstripW  = 552;  // right storyboard pane
const int kTransportH  = 52;   // seek + transport zone under the preview
const int kClipH       = 96;   // filmstrip clip height
const int kClipGap     = 14;
const int kSprocketW   = 9;    // film perforation strip inside each clip
const int kSeekPct     = 32;   // static preview position (percent of project)

// ============================================================================
// Palette -- sampled from reference captures of the original binary
// ============================================================================
const COLORREF kFileTabBlue    = RGB(0x15, 0x74, 0xC4);
const COLORREF kFileTabHot     = RGB(0x3C, 0x8F, 0xD4);
const COLORREF kTabstripBg     = RGB(0xF6, 0xF6, 0xF7);
const COLORREF kTabstripLine   = RGB(0xE4, 0xE6, 0xE9);
const COLORREF kTabText        = RGB(0x1E, 0x1E, 0x1E);
const COLORREF kTabHoverFill   = RGB(0xDC, 0xE9, 0xF5);
const COLORREF kTabHoverLine   = RGB(0xB8, 0xD2, 0xEA);
const COLORREF kTabActiveLine  = RGB(0xC6, 0xCC, 0xD3);
const COLORREF kRibbonBg       = RGB(0xF4, 0xF5, 0xF7);
const COLORREF kRibbonLine     = RGB(0xD9, 0xDC, 0xE1);
const COLORREF kGroupLabel     = RGB(0x6D, 0x6D, 0x6D);
const COLORREF kGroupSep       = RGB(0xDD, 0xE0, 0xE4);
const COLORREF kCtxVideoBand   = RGB(0xF0, 0xEF, 0x87); // "Video Tools" band
const COLORREF kCtxTextBand    = RGB(0xE3, 0xC8, 0xF5); // "Text Tools" band
const COLORREF kHoverFill      = RGB(0xDC, 0xEA, 0xF7);
const COLORREF kHoverLine      = RGB(0xB9, 0xD5, 0xEC);
const COLORREF kStatusBg       = RGB(0xEA, 0xF3, 0xFA);
const COLORREF kStatusLine     = RGB(0xD5, 0xE5, 0xF0);
const COLORREF kStatusText     = RGB(0x3A, 0x3A, 0x3A);
const COLORREF kTimeText       = RGB(0x8A, 0x91, 0x99);
const COLORREF kClipBorder     = RGB(0xAE, 0xB7, 0xC0);
const COLORREF kClipBg         = RGB(0x1B, 0x1E, 0x22);
const COLORREF kSprocketLight  = RGB(0xF2, 0xF4, 0xF6);
const COLORREF kSprocketDark   = RGB(0xC9, 0xCE, 0xD4);
const COLORREF kSprocketEdge   = RGB(0xB6, 0xBD, 0xC5);
const COLORREF kWaveFill       = RGB(0x6F, 0x98, 0xC0);
const COLORREF kSelBlue        = RGB(0x5A, 0xA4, 0xE0);
const COLORREF kTransportBlue  = RGB(0x1D, 0x6F, 0xC2);
const COLORREF kSeekTrack      = RGB(0xCF, 0xD8, 0xE0);
const COLORREF kZoomTrack      = RGB(0xC3, 0xD4, 0xE2);
const COLORREF kChipBorder     = RGB(0xB9, 0xC6, 0xD4);
const COLORREF kThemeBarLine   = RGB(0xC9, 0xCE, 0xD4);
const COLORREF kSplitterBg     = RGB(0xE3, 0xE3, 0xE5);
const COLORREF kWhite          = RGB(0xFF, 0xFF, 0xFF);

// ============================================================================
// Visual state (main window is single-instance: Global\Sundance mutex)
// ============================================================================
struct VisualState
{
    int  nActiveTab;      // 1=Home ... 5=View (0=File handled as popup menu)
    int  nHotTab;         // hovered tab index, -1 none
    int  nHotButton;      // hovered ribbon button id, -1 none
    int  nSelectedClip;   // project media index, -1 none
    bool bTrackMouse;     // TrackMouseEvent armed

    VisualState()
        : nActiveTab(1), nHotTab(-1), nHotButton(-1),
          nSelectedClip(-1), bTrackMouse(false) {}
};
VisualState g_vis;

// Tab order: File, Home, Animations, Visual Effects, Project, View
const LPCWSTR kTabNames[6] =
{
    L"File", L"Home", L"Animations", L"Visual Effects", L"Project", L"View"
};
const int kTabCount = 6;

// ============================================================================
// Fonts -- created once per process (freed at process teardown)
// ============================================================================
HFONT g_fontTab   = NULL; // 12px tab / button labels
HFONT g_fontSmall = NULL; // 10px contextual superscript
HFONT g_fontGroup = NULL; // 11px group captions, status bar

void EnsureFonts()
{
    if (g_fontTab)
        return;

    g_fontTab = CreateFontW(-12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    g_fontSmall = CreateFontW(-10, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
    g_fontGroup = CreateFontW(-11, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
}

HFONT SelFont(HDC hdc, HFONT hFont)
{
    return (HFONT)SelectObject(hdc, hFont);
}

// ============================================================================
// Primitives
// ============================================================================
void FillCr(HDC hdc, const RECT& rc, COLORREF cr)
{
    RECT rc2 = rc;
    HBRUSH hbr = CreateSolidBrush(cr);
    FillRect(hdc, &rc2, hbr);
    DeleteObject(hbr);
}

void FrameCr(HDC hdc, const RECT& rc, COLORREF cr)
{
    RECT rc2 = rc;
    HBRUSH hbr = CreateSolidBrush(cr);
    FrameRect(hdc, &rc2, hbr);
    DeleteObject(hbr);
}

void LineCr(HDC hdc, int x1, int y1, int x2, int y2, COLORREF cr)
{
    HPEN hPen = CreatePen(PS_SOLID, 1, cr);
    HPEN hOld = (HPEN)SelectObject(hdc, hPen);
    MoveToEx(hdc, x1, y1, NULL);
    LineTo(hdc, x2, y2);
    SelectObject(hdc, hOld);
    DeleteObject(hPen);
}

void Tri(HDC hdc, const POINT* pts, int c, COLORREF cr)
{
    HBRUSH hbr = CreateSolidBrush(cr);
    HPEN hPen = CreatePen(PS_SOLID, 1, cr);
    HBRUSH hOld = (HBRUSH)SelectObject(hdc, hbr);
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
    Polygon(hdc, pts, c);
    SelectObject(hdc, hOld);
    SelectObject(hdc, hOldPen);
    DeleteObject(hbr);
    DeleteObject(hPen);
}

void Circle(HDC hdc, int cx, int cy, int r, COLORREF cr)
{
    HBRUSH hbr = CreateSolidBrush(cr);
    HPEN hPen = CreatePen(PS_SOLID, 1, cr);
    HBRUSH hOld = (HBRUSH)SelectObject(hdc, hbr);
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
    Ellipse(hdc, cx - r, cy - r, cx + r, cy + r);
    SelectObject(hdc, hOld);
    SelectObject(hdc, hOldPen);
    DeleteObject(hbr);
    DeleteObject(hPen);
}

void DrawText1(HDC hdc, LPCWSTR psz, const RECT& rc, COLORREF cr, UINT flags)
{
    RECT rc2 = rc;
    SetTextColor(hdc, cr);
    SetBkMode(hdc, TRANSPARENT);
    DrawTextW(hdc, psz, -1, &rc2, flags | DT_SINGLELINE | DT_NOPREFIX);
}

// ============================================================================
// Icon primitives (small vector-like glyphs drawn with GDI primitives)
// ============================================================================

// Clipboard icon for the Paste button (wood board + white sheet + clip)
void IconPaste(HDC hdc, const RECT& rc)
{
    int cx = (rc.left + rc.right) / 2;
    int cy = (rc.top + rc.bottom) / 2;
    RECT board = { cx - 9, cy - 13, cx + 9, cy + 13 };
    FillCr(hdc, board, RGB(0xCA, 0xA0, 0x6A));
    RECT sheet = { cx - 7, cy - 10, cx + 7, cy + 11 };
    FillCr(hdc, sheet, RGB(0xFB, 0xFB, 0xF7));
    RECT clip = { cx - 4, cy - 14, cx + 4, cy - 11 };
    FillCr(hdc, clip, RGB(0x8F, 0xA6, 0xB8));
}

// Film + photo icon for "Add videos and photos"
void IconAddMedia(HDC hdc, const RECT& rc)
{
    int cx = (rc.left + rc.right) / 2;
    int cy = (rc.top + rc.bottom) / 2;
    RECT frame = { cx - 13, cy - 9, cx + 6, cy + 7 };
    FillCr(hdc, frame, RGB(0x4A, 0x5E, 0x70));
    RECT photo = { cx - 11, cy - 7, cx + 4, cy + 5 };
    FillCr(hdc, photo, RGB(0xDF, 0xE7, 0xEE));
    // mountain silhouette
    POINT mtn[4] = {
        { cx - 11, cy + 5 }, { cx - 5, cy - 2 }, { cx - 1, cy + 1 }, { cx + 4, cy + 5 } };
    Tri(hdc, mtn, 4, RGB(0x7F, 0xA3, 0xC0));
    // camcorder body at right
    RECT cam = { cx + 6, cy - 3, cx + 13, cy + 6 };
    FillCr(hdc, cam, RGB(0x4A, 0x5E, 0x70));
    POINT lens[3] = { { cx + 13, cy - 3 }, { cx + 16, cy - 5 }, { cx + 16, cy + 4 } };
    Tri(hdc, lens, 3, RGB(0x4A, 0x5E, 0x70));
}

// Eighth-note for "Add music"
void IconAddMusic(HDC hdc, const RECT& rc)
{
    int cx = (rc.left + rc.right) / 2;
    int cy = (rc.top + rc.bottom) / 2;
    HPEN hPen = CreatePen(PS_SOLID, 2, kTransportBlue);
    HPEN hOld = (HPEN)SelectObject(hdc, hPen);
    MoveToEx(hdc, cx - 4, cy - 12, NULL);
    LineTo(hdc, cx - 4, cy + 6);
    MoveToEx(hdc, cx - 4, cy - 12, NULL);
    LineTo(hdc, cx + 9, cy - 8);
    MoveToEx(hdc, cx + 9, cy - 8, NULL);
    LineTo(hdc, cx + 9, cy + 2);
    SelectObject(hdc, hOld);
    DeleteObject(hPen);
    HBRUSH hbr = CreateSolidBrush(kTransportBlue);
    HBRUSH hOldBr = (HBRUSH)SelectObject(hdc, hbr);
    Ellipse(hdc, cx - 10, cy + 3, cx + 2, cy + 11);
    Ellipse(hdc, cx + 5, cy - 1, cx + 13, cy + 6);
    SelectObject(hdc, hOldBr);
    DeleteObject(hbr);
}

// Webcam for "Webcam video"
void IconWebcam(HDC hdc, const RECT& rc)
{
    int cx = (rc.left + rc.right) / 2;
    int cy = (rc.top + rc.bottom) / 2;
    RECT body = { cx - 5, cy - 9, cx + 5, cy + 4 };
    FillCr(hdc, body, RGB(0x5A, 0x66, 0x73));
    Circle(hdc, cx, cy - 2, 3, RGB(0xDF, 0xE6, 0xEC));
    Circle(hdc, cx, cy - 2, 1, RGB(0x5A, 0x66, 0x73));
    RECT base = { cx - 2, cy + 4, cx + 2, cy + 9 };
    FillCr(hdc, base, RGB(0x5A, 0x66, 0x73));
}

// Microphone for "Record narration"
void IconMic(HDC hdc, const RECT& rc)
{
    int cx = (rc.left + rc.right) / 2;
    int cy = (rc.top + rc.bottom) / 2;
    RECT head = { cx - 6, cy - 9, cx + 2, cy - 1 };
    FillCr(hdc, head, RGB(0xB9, 0x8A, 0x56));
    HPEN hPen = CreatePen(PS_SOLID, 2, RGB(0x5A, 0x66, 0x73));
    HPEN hOld = (HPEN)SelectObject(hdc, hPen);
    MoveToEx(hdc, cx - 1, cy - 1, NULL);
    LineTo(hdc, cx + 6, cy + 8);
    SelectObject(hdc, hOld);
    DeleteObject(hPen);
}

// Camera for "Snapshot"
void IconSnapshot(HDC hdc, const RECT& rc)
{
    int cx = (rc.left + rc.right) / 2;
    int cy = (rc.top + rc.bottom) / 2;
    RECT body = { cx - 8, cy - 5, cx + 8, cy + 6 };
    FillCr(hdc, body, RGB(0x5A, 0x66, 0x73));
    Circle(hdc, cx, cy, 3, RGB(0xDF, 0xE6, 0xEC));
    Circle(hdc, cx, cy, 1, RGB(0x5A, 0x66, 0x73));
    RECT top = { cx - 3, cy - 7, cx + 3, cy - 4 };
    FillCr(hdc, top, RGB(0x5A, 0x66, 0x73));
}

// Dark tile with a glyph for Title / Caption / Credits
void IconTextChip(HDC hdc, const RECT& rc, WCHAR ch)
{
    RECT tile = { rc.left, rc.top, rc.left + 14, rc.bottom };
    FillCr(hdc, tile, RGB(0x4D, 0x5A, 0x68));
    RECT rcText = { tile.left, tile.top, tile.right, tile.bottom - 1 };
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(0xE8, 0xEE, 0xF4));
    HFONT hFont = CreateFontW(-9, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
    HFONT hOld = SelFont(hdc, hFont);
    DrawTextW(hdc, &ch, 1, &rcText, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
    SelFont(hdc, hOld);
    DeleteObject(hFont);
}

// Rotate arrows (left/right)
void IconRotate(HDC hdc, const RECT& rc, bool bLeft)
{
    int cx = (rc.left + rc.right) / 2;
    int cy = (rc.top + rc.bottom) / 2;
    HPEN hPen = CreatePen(PS_SOLID, 2, RGB(0xC4, 0x7F, 0x3C));
    HPEN hOld = (HPEN)SelectObject(hdc, hPen);
    if (bLeft)
        Arc(hdc, cx - 7, cy - 6, cx + 7, cy + 8, cx + 6, cy + 5, cx - 6, cy + 5);
    else
        Arc(hdc, cx - 7, cy - 6, cx + 7, cy + 8, cx - 6, cy + 5, cx + 6, cy + 5);
    SelectObject(hdc, hOld);
    DeleteObject(hPen);
    POINT head[3];
    if (bLeft)
    {
        head[0] = { cx - 8, cy + 8 }; head[1] = { cx - 2, cy + 8 }; head[2] = { cx - 7, cy + 2 };
    }
    else
    {
        head[0] = { cx + 8, cy + 8 }; head[1] = { cx + 2, cy + 8 }; head[2] = { cx + 7, cy + 2 };
    }
    Tri(hdc, head, 3, RGB(0xC4, 0x7F, 0x3C));
}

// Red X for "Remove"
void IconRemove(HDC hdc, const RECT& rc)
{
    int cx = (rc.left + rc.right) / 2;
    int cy = (rc.top + rc.bottom) / 2;
    HPEN hPen = CreatePen(PS_SOLID, 2, RGB(0xC2, 0x3A, 0x3A));
    HPEN hOld = (HPEN)SelectObject(hdc, hPen);
    MoveToEx(hdc, cx - 5, cy - 5, NULL); LineTo(hdc, cx + 5, cy + 5);
    MoveToEx(hdc, cx + 5, cy - 5, NULL); LineTo(hdc, cx - 5, cy + 5);
    SelectObject(hdc, hOld);
    DeleteObject(hPen);
}

// Bordered square for "Select all"
void IconSelectAll(HDC hdc, const RECT& rc)
{
    int cx = (rc.left + rc.right) / 2;
    int cy = (rc.top + rc.bottom) / 2;
    RECT sq = { cx - 6, cy - 6, cx + 6, cy + 6 };
    FrameCr(hdc, sq, RGB(0x5A, 0x66, 0x73));
    LineCr(hdc, sq.left, sq.top + 3, sq.right, sq.top + 3, RGB(0x5A, 0x66, 0x73));
}

// OneDrive-style cloud for Share
void IconCloud(HDC hdc, const RECT& rc)
{
    int cx = (rc.left + rc.right) / 2;
    int cy = (rc.top + rc.bottom) / 2;
    HBRUSH hbr = CreateSolidBrush(RGB(0x1E, 0x78, 0xC8));
    HBRUSH hOld = (HBRUSH)SelectObject(hdc, hbr);
    Ellipse(hdc, cx - 12, cy - 4, cx - 2, cy + 6);
    Ellipse(hdc, cx - 6, cy - 9, cx + 6, cy + 3);
    Ellipse(hdc, cx + 2, cy - 4, cx + 12, cy + 6);
    RECT base = { cx - 11, cy, cx + 11, cy + 6 };
    FillRect(hdc, &base, hbr);
    SelectObject(hdc, hOld);
    DeleteObject(hbr);
}

// Facebook tile for Share
void IconFacebook(HDC hdc, const RECT& rc)
{
    RECT tile = { rc.left + 1, rc.top, rc.right - 1, rc.bottom };
    FillCr(hdc, tile, RGB(0x3B, 0x59, 0x98));
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, kWhite);
    HFONT hFont = CreateFontW(-16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
    HFONT hOld = SelFont(hdc, hFont);
    RECT rcF = { tile.left, tile.top - 1, tile.right, tile.bottom };
    DrawTextW(hdc, L"f", -1, &rcF, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
    SelFont(hdc, hOld);
    DeleteObject(hFont);
}

// Floppy for "Save movie"
void IconSaveMovie(HDC hdc, const RECT& rc)
{
    int cx = (rc.left + rc.right) / 2;
    int cy = (rc.top + rc.bottom) / 2;
    RECT body = { cx - 8, cy - 8, cx + 8, cy + 8 };
    FillCr(hdc, body, RGB(0x4D, 0x5A, 0x68));
    RECT shutter = { cx - 5, cy - 7, cx + 5, cy - 2 };
    FillCr(hdc, shutter, RGB(0xDF, 0xE7, 0xEE));
    RECT label = { cx - 5, cy + 1, cx + 5, cy + 7 };
    FillCr(hdc, label, RGB(0xEE, 0xF2, 0xF6));
}

// Person tile for "Sign in"
void IconSignIn(HDC hdc, const RECT& rc)
{
    int cx = (rc.left + rc.right) / 2;
    int cy = (rc.top + rc.bottom) / 2;
    RECT frame = { cx - 8, cy - 9, cx + 8, cy + 9 };
    FillCr(hdc, frame, RGB(0xCF, 0xE0, 0xEE));
    FrameCr(hdc, frame, RGB(0x8F, 0xB0, 0xCC));
    Circle(hdc, cx, cy - 2, 3, RGB(0xF0, 0xF4, 0xF8));
    HBRUSH hbr = CreateSolidBrush(RGB(0xF0, 0xF4, 0xF8));
    HBRUSH hOld = (HBRUSH)SelectObject(hdc, hbr);
    Ellipse(hdc, cx - 5, cy + 3, cx + 5, cy + 12);
    SelectObject(hdc, hOld);
    DeleteObject(hbr);
}

void DropdownArrow(HDC hdc, const RECT& rc, COLORREF cr)
{
    int cx = (rc.left + rc.right) / 2;
    int cy = (rc.top + rc.bottom) / 2;
    POINT pts[3] = { { cx - 3, cy - 1 }, { cx + 3, cy - 1 }, { cx, cy + 3 } };
    Tri(hdc, pts, 3, cr);
}

// ============================================================================
// Layout
// ============================================================================
struct ButtonRects
{
    RECT rcPaste;
    RECT rcAddMedia;
    RECT rcAddMusic;
    RECT rcStack1[3];   // Webcam video, Record narration, Snapshot
    RECT rcStack2[3];   // Title, Caption, Credits
    RECT rcThemes;      // AutoMovie themes bar
    RECT rcThemeTh[4];  // theme thumbnails
    RECT rcRotL, rcRotR, rcRemove, rcSelectAll;
    RECT rcCloud, rcFacebook, rcShareArrows;
    RECT rcSaveMovie, rcSignIn;
};

ButtonRects g_btn;

void LayoutRibbon(const RECT& rcBody)
{
    const int y0 = rcBody.top + 4;
    int x = rcBody.left + 4;

    // -- Clipboard --
    g_btn.rcPaste = { x, y0, x + 47, y0 + 56 };
    x += 47 + 12;

    // -- Add --
    g_btn.rcAddMedia = { x, y0, x + 64, y0 + 56 };
    x += 64 + 2;
    g_btn.rcAddMusic = { x, y0, x + 44, y0 + 56 };
    x += 44 + 6;
    const int wStack1 = 112, wStack2 = 66;
    for (int i = 0; i < 3; ++i)
    {
        g_btn.rcStack1[i] = { x, y0 + i * 19, x + wStack1, y0 + i * 19 + 18 };
        g_btn.rcStack2[i] = { x + wStack1 + 2, y0 + i * 19,
                              x + wStack1 + 2 + wStack2, y0 + i * 19 + 18 };
    }
    x += wStack1 + 2 + wStack2 + 12;

    // -- AutoMovie themes --
    g_btn.rcThemes = { x, y0 + 1, x + 218, y0 + 47 };
    for (int i = 0; i < 4; ++i)
    {
        int tx = g_btn.rcThemes.left + 2 + i * 51;
        g_btn.rcThemeTh[i] = { tx, g_btn.rcThemes.top + 2,
                               tx + 48, g_btn.rcThemes.top + 44 };
    }
    x += 218 + 12;

    // -- Editing --
    g_btn.rcRotL = { x, y0, x + 92, y0 + 20 };
    g_btn.rcRotR = { x, y0 + 21, x + 92, y0 + 41 };
    g_btn.rcRemove = { x + 94, y0, x + 94 + 74, y0 + 20 };
    g_btn.rcSelectAll = { x + 94, y0 + 21, x + 94 + 74, y0 + 41 };
    x += 94 + 74 + 12;

    // -- Share --
    g_btn.rcCloud = { x, y0, x + 46, y0 + 46 };
    g_btn.rcFacebook = { x + 48, y0, x + 48 + 46, y0 + 46 };
    g_btn.rcShareArrows = { x + 97, y0 + 8, x + 105, y0 + 40 };
    x += 46 + 48 + 46 + 12;

    // -- Save movie / Sign in --
    g_btn.rcSaveMovie = { x, y0, x + 50, y0 + 56 };
    g_btn.rcSignIn = { x + 52, y0, x + 52 + 40, y0 + 56 };
}

int LayoutTabs(const RECT& rcStrip, RECT* prcTabs, RECT* prcCtxVideo, RECT* prcCtxText)
{
    // File button
    prcTabs[0] = { rcStrip.left + 4, rcStrip.top + 1,
                   rcStrip.left + 48, rcStrip.bottom - 2 };

    int x = prcTabs[0].right + 6;
    for (int i = 1; i < kTabCount; ++i)
    {
        int w = 11 + 2 * 11 + (i == 3 ? 22 : 0); // "Visual Effects" wider
        prcTabs[i] = { x, rcStrip.top, x + w + 18, rcStrip.bottom };
        x = prcTabs[i].right;
    }

    // Contextual tabs appear at the right of the static tabs when media
    // is selected (Video Tools > Edit; Text Tools > Format for text).
    *prcCtxVideo = { x + 6, rcStrip.top, x + 66, rcStrip.bottom };
    *prcCtxText  = { x + 72, rcStrip.top, x + 132, rcStrip.bottom };
    return x;
}

// ============================================================================
// Ribbon rendering
// ============================================================================
void DrawTabs(HDC hdc, const RECT& rcStrip)
{
    FillCr(hdc, rcStrip, kTabstripBg);
    LineCr(hdc, rcStrip.left, rcStrip.top, rcStrip.right, rcStrip.top, kTabstripLine);

    RECT rcTabs[kTabCount];
    RECT rcCtxVideo, rcCtxText;
    LayoutTabs(rcStrip, rcTabs, &rcCtxVideo, &rcCtxText);

    SetBkMode(hdc, TRANSPARENT);

    // -- File application button --
    const RECT& rcFile = rcTabs[0];
    FillCr(hdc, rcFile, g_vis.nHotTab == 0 ? kFileTabHot : kFileTabBlue);
    HFONT hOld = SelFont(hdc, g_fontTab);
    DrawText1(hdc, kTabNames[0], rcFile, kWhite, DT_CENTER | DT_VCENTER);

    // -- Static tabs --
    for (int i = 1; i < kTabCount; ++i)
    {
        const RECT& rc = rcTabs[i];
        if (i == g_vis.nActiveTab)
        {
            RECT rcActive = { rc.left, rc.top, rc.right, rc.bottom + 1 };
            FillCr(hdc, rcActive, kWhite);
            FrameCr(hdc, rcActive, kTabActiveLine);
            // re-white the interior so the frame reads as top/left/right only
            RECT rcInner = { rc.left + 1, rc.top + 1, rc.right - 1, rc.bottom + 1 };
            FillCr(hdc, rcInner, kWhite);
            DrawText1(hdc, kTabNames[i], rc, kTabText, DT_CENTER | DT_VCENTER);
        }
        else
        {
            if (i == g_vis.nHotTab)
            {
                RECT rcHot = { rc.left + 1, rc.top + 2, rc.right - 1, rc.bottom };
                FillCr(hdc, rcHot, kTabHoverFill);
                FrameCr(hdc, rcHot, kTabHoverLine);
            }
            DrawText1(hdc, kTabNames[i], rc, kTabText, DT_CENTER | DT_VCENTER);
        }
    }

    // -- Contextual tabs (visible while a clip is selected) --
    if (g_vis.nSelectedClip >= 0)
    {
        const RECT& rcV = rcCtxVideo;
        RECT rcBand = { rcV.left, rcV.top, rcV.right, rcV.top + 12 };
        FillCr(hdc, rcBand, kCtxVideoBand);
        if (g_vis.nHotTab == 6)
        {
            RECT rcHot = { rcV.left + 1, rcV.top + 2, rcV.right - 1, rcV.bottom };
            FrameCr(hdc, rcHot, kTabHoverLine);
        }
        SelFont(hdc, g_fontSmall);
        DrawText1(hdc, L"Video Tools", rcBand, kTabText, DT_CENTER | DT_VCENTER);
        SelFont(hdc, g_fontTab);
        RECT rcLabel = { rcV.left, rcBand.bottom, rcV.right, rcV.bottom };
        DrawText1(hdc, L"Edit", rcLabel, kTabText, DT_CENTER | DT_VCENTER);
    }

    SelFont(hdc, hOld);

    // Help button (top-right blue "?" globe)
    RECT rcHelp = { rcStrip.right - 28, rcStrip.top + 3,
                    rcStrip.right - 10, rcStrip.top + 21 };
    Circle(hdc, (rcHelp.left + rcHelp.right) / 2, (rcHelp.top + rcHelp.bottom) / 2, 9,
           RGB(0x2B, 0x7C, 0xD3));
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, kWhite);
    HFONT hF = CreateFontW(-11, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
    HFONT hOldF = SelFont(hdc, hF);
    DrawTextW(hdc, L"?", -1, &rcHelp, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
    SelFont(hdc, hOldF);
    DeleteObject(hF);
}

// ============================================================================
// Home ribbon groups
// ============================================================================
void DrawRibbonBody(HDC hdc, const RECT& rcBody)
{
    FillCr(hdc, rcBody, kRibbonBg);
    LineCr(hdc, rcBody.left, rcBody.bottom, rcBody.right, rcBody.bottom, kRibbonLine);

    if (g_vis.nActiveTab != 1)
        return; // other tabs render an empty body (UIFILE not recovered)

    LayoutRibbon(rcBody);

    const int yLabel = rcBody.bottom - 16;

    // ---- group separators ----
    int seps[5];
    seps[0] = g_btn.rcAddMedia.left - 8;
    seps[1] = g_btn.rcThemes.left - 8;
    seps[2] = g_btn.rcRotL.left - 8;
    seps[3] = g_btn.rcCloud.left - 8;
    seps[4] = g_btn.rcSaveMovie.left - 8;
    for (int i = 0; i < 5; ++i)
        LineCr(hdc, seps[i], rcBody.top + 4, seps[i], yLabel - 2, kGroupSep);

    HFONT hOld = SelFont(hdc, g_fontTab);

    // ---- Clipboard ----
    if (g_vis.nHotButton == 1)
    {
        RECT rcHot = { g_btn.rcPaste.left - 1, g_btn.rcPaste.top - 1,
                       g_btn.rcPaste.right + 1, g_btn.rcPaste.bottom + 1 };
        FillCr(hdc, rcHot, kHoverFill);
        FrameCr(hdc, rcHot, kHoverLine);
    }
    IconPaste(hdc, g_btn.rcPaste);
    {
        RECT rcL = { g_btn.rcPaste.left - 6, g_btn.rcPaste.bottom + 2,
                     g_btn.rcPaste.right + 6, g_btn.rcPaste.bottom + 14 };
        DrawText1(hdc, L"Paste", rcL, kTabText, DT_CENTER);
        RECT rcG = { g_btn.rcPaste.left - 12, yLabel, g_btn.rcPaste.right + 12, yLabel + 14 };
        SelFont(hdc, g_fontGroup);
        DrawText1(hdc, L"Clipboard", rcG, kGroupLabel, DT_CENTER);
        SelFont(hdc, g_fontTab);
    }

    // ---- Add ----
    if (g_vis.nHotButton == 2)
    {
        RECT rcHot = { g_btn.rcAddMedia.left - 1, g_btn.rcAddMedia.top - 1,
                       g_btn.rcAddMedia.right + 1, g_btn.rcAddMedia.bottom + 1 };
        FillCr(hdc, rcHot, kHoverFill); FrameCr(hdc, rcHot, kHoverLine);
    }
    IconAddMedia(hdc, g_btn.rcAddMedia);
    {
        RECT rcL1 = { g_btn.rcAddMedia.left - 4, g_btn.rcAddMedia.bottom + 2,
                      g_btn.rcAddMedia.right + 4, g_btn.rcAddMedia.bottom + 14 };
        RECT rcL2 = { g_btn.rcAddMedia.left - 4, g_btn.rcAddMedia.bottom + 13,
                      g_btn.rcAddMedia.right + 4, g_btn.rcAddMedia.bottom + 25 };
        DrawText1(hdc, L"Add videos", rcL1, kTabText, DT_CENTER);
        DrawText1(hdc, L"and photos", rcL2, kTabText, DT_CENTER);
    }
    if (g_vis.nHotButton == 3)
    {
        RECT rcHot = { g_btn.rcAddMusic.left - 1, g_btn.rcAddMusic.top - 1,
                       g_btn.rcAddMusic.right + 1, g_btn.rcAddMusic.bottom + 1 };
        FillCr(hdc, rcHot, kHoverFill); FrameCr(hdc, rcHot, kHoverLine);
    }
    IconAddMusic(hdc, g_btn.rcAddMusic);
    {
        RECT rcL1 = { g_btn.rcAddMusic.left - 4, g_btn.rcAddMusic.bottom + 2,
                      g_btn.rcAddMusic.right + 10, g_btn.rcAddMusic.bottom + 14 };
        RECT rcL2 = { g_btn.rcAddMusic.left - 4, g_btn.rcAddMusic.bottom + 13,
                      g_btn.rcAddMusic.right + 4, g_btn.rcAddMusic.bottom + 25 };
        DrawText1(hdc, L"Add \x25B8", rcL1, kTabText, DT_CENTER);
        DrawText1(hdc, L"music", rcL2, kTabText, DT_CENTER);
    }

    LPCWSTR kStack1[3] = { L"Webcam video", L"Record narration \x25B8", L"Snapshot" };
    for (int i = 0; i < 3; ++i)
    {
        const RECT& rc = g_btn.rcStack1[i];
        if (g_vis.nHotButton == 10 + i)
        {
            RECT rcHot = { rc.left, rc.top, rc.right + 1, rc.bottom + 1 };
            FillCr(hdc, rcHot, kHoverFill); FrameCr(hdc, rcHot, kHoverLine);
        }
        switch (i)
        {
        case 0: IconWebcam(hdc, rc); break;
        case 1: IconMic(hdc, rc); break;
        case 2: IconSnapshot(hdc, rc); break;
        }
        RECT rcT = { rc.left + 18, rc.top, rc.right, rc.bottom };
        DrawText1(hdc, kStack1[i], rcT, kTabText, DT_LEFT | DT_VCENTER);
    }

    const WCHAR kStack2[3] = { L'T', L'A', L'A' };
    for (int i = 0; i < 3; ++i)
    {
        const RECT& rc = g_btn.rcStack2[i];
        if (g_vis.nHotButton == 20 + i)
        {
            RECT rcHot = { rc.left, rc.top, rc.right + 1, rc.bottom + 1 };
            FillCr(hdc, rcHot, kHoverFill); FrameCr(hdc, rcHot, kHoverLine);
        }
        IconTextChip(hdc, rc, kStack2[i]);
        RECT rcT = { rc.left + 18, rc.top, rc.right - 8, rc.bottom };
        switch (i)
        {
        case 0: DrawText1(hdc, L"Title", rcT, kTabText, DT_LEFT | DT_VCENTER); break;
        case 1: DrawText1(hdc, L"Caption", rcT, kTabText, DT_LEFT | DT_VCENTER); break;
        case 2:
            DrawText1(hdc, L"Credits", rcT, kTabText, DT_LEFT | DT_VCENTER);
            RECT rcDd = { rc.right - 10, rc.top, rc.right, rc.bottom };
            DropdownArrow(hdc, rcDd, RGB(0x44, 0x44, 0x44));
            break;
        }
    }

    {
        RECT rcG = { g_btn.rcStack2[0].left, yLabel,
                     g_btn.rcStack2[0].right + 8, yLabel + 14 };
        SelFont(hdc, g_fontGroup);
        DrawText1(hdc, L"Add", rcG, kGroupLabel, DT_CENTER);
        SelFont(hdc, g_fontTab);
    }

    // ---- AutoMovie themes ----
    {
        const RECT& rcBar = g_btn.rcThemes;
        FillCr(hdc, rcBar, kWhite);
        FrameCr(hdc, rcBar, kThemeBarLine);

        // four thumbnails: sky + hills + white frame
        const COLORREF skies[4][2] = {
            { RGB(0x8E, 0xC3, 0xE2), RGB(0xC5, 0xE0, 0xEE) },
            { RGB(0xA7, 0xCF, 0xE4), RGB(0xD6, 0xEB, 0xF4) },
            { RGB(0x9C, 0xC8, 0xDE), RGB(0xCF, 0xE6, 0xEF) },
            { RGB(0xB5, 0xD4, 0xE0), RGB(0xE2, 0xF0, 0xF4) },
        };
        const COLORREF hills[4] = {
            RGB(0x3E, 0x79, 0xA3), RGB(0x52, 0x86, 0x9E),
            RGB(0x48, 0x78, 0x8F), RGB(0x5D, 0x8B, 0xA0),
        };
        for (int i = 0; i < 4; ++i)
        {
            const RECT& rc = g_btn.rcThemeTh[i];
            int midY = rc.top + (rc.bottom - rc.top) * 6 / 10;
            RECT skyTop = { rc.left, rc.top, rc.right, midY };
            RECT skyBot = { rc.left, midY, rc.right, rc.bottom };
            FillCr(hdc, skyTop, skies[i][0]);
            FillCr(hdc, skyBot, skies[i][1]);
            if (i == 1)
                Circle(hdc, rc.right - 12, rc.top + 8, 4, RGB(0xF7, 0xEC, 0xC9));
            POINT hill[4] = {
                { rc.left, rc.bottom }, { rc.left + (rc.right - rc.left) / 3, midY + 4 },
                { rc.left + 2 * (rc.right - rc.left) / 3, midY + 9 }, { rc.right, rc.bottom } };
            Tri(hdc, hill, 4, hills[i]);
            FrameCr(hdc, rc, kWhite);

            if (i == 0)
            {
                // selected thumbnail outline
                RECT rcSel = { rc.left - 1, rc.top - 1, rc.right + 1, rc.bottom + 1 };
                FrameCr(hdc, rcSel, RGB(0x7F, 0xB1, 0xE4));
            }
        }
        // up/down arrows at the right edge of the bar
        int ax = rcBar.right - 6;
        int ay = rcBar.top + rcBar.bottom / 2;
        POINT up[3]   = { { ax - 3, ay - 4 }, { ax + 3, ay - 4 }, { ax, ay - 8 } };
        POINT down[3] = { { ax - 3, ay + 4 }, { ax + 3, ay + 4 }, { ax, ay + 8 } };
        Tri(hdc, up, 3, RGB(0x5A, 0x60, 0x67));
        Tri(hdc, down, 3, RGB(0x5A, 0x60, 0x67));

        RECT rcG = { rcBar.left - 20, yLabel, rcBar.right + 20, yLabel + 14 };
        SelFont(hdc, g_fontGroup);
        DrawText1(hdc, L"AutoMovie themes", rcG, kGroupLabel, DT_CENTER);
        SelFont(hdc, g_fontTab);
    }

    // ---- Editing ----
    LPCWSTR kEditL[2] = { L"Rotate left", L"Rotate right" };
    RECT* rcEditL[2]  = { &g_btn.rcRotL, &g_btn.rcRotR };
    for (int i = 0; i < 2; ++i)
    {
        const RECT& rc = *rcEditL[i];
        if (g_vis.nHotButton == 30 + i)
        {
            RECT rcHot = { rc.left, rc.top, rc.right + 1, rc.bottom + 1 };
            FillCr(hdc, rcHot, kHoverFill); FrameCr(hdc, rcHot, kHoverLine);
        }
        RECT rcIcon = { rc.left, rc.top, rc.left + 18, rc.bottom };
        IconRotate(hdc, rcIcon, i == 0);
        RECT rcT = { rc.left + 20, rc.top, rc.right, rc.bottom };
        DrawText1(hdc, kEditL[i], rcT, kTabText, DT_LEFT | DT_VCENTER);
    }
    {
        const RECT& rc = g_btn.rcRemove;
        if (g_vis.nHotButton == 32)
        {
            RECT rcHot = { rc.left, rc.top, rc.right + 1, rc.bottom + 1 };
            FillCr(hdc, rcHot, kHoverFill); FrameCr(hdc, rcHot, kHoverLine);
        }
        RECT rcIcon = { rc.left, rc.top, rc.left + 18, rc.bottom };
        IconRemove(hdc, rcIcon);
        RECT rcT = { rc.left + 20, rc.top, rc.right, rc.bottom };
        DrawText1(hdc, L"Remove", rcT, kTabText, DT_LEFT | DT_VCENTER);

        const RECT& rc2 = g_btn.rcSelectAll;
        if (g_vis.nHotButton == 33)
        {
            RECT rcHot = { rc2.left, rc2.top, rc2.right + 1, rc2.bottom + 1 };
            FillCr(hdc, rcHot, kHoverFill); FrameCr(hdc, rcHot, kHoverLine);
        }
        RECT rcIcon2 = { rc2.left, rc2.top, rc2.left + 18, rc2.bottom };
        IconSelectAll(hdc, rcIcon2);
        RECT rcT2 = { rc2.left + 20, rc2.top, rc2.right, rc2.bottom };
        DrawText1(hdc, L"Select all", rcT2, kTabText, DT_LEFT | DT_VCENTER);

        RECT rcG = { g_btn.rcRotL.left, yLabel, g_btn.rcSelectAll.right, yLabel + 14 };
        SelFont(hdc, g_fontGroup);
        DrawText1(hdc, L"Editing", rcG, kGroupLabel, DT_CENTER);
        SelFont(hdc, g_fontTab);
    }

    // ---- Share ----
    {
        if (g_vis.nHotButton == 40)
        {
            RECT rcHot = { g_btn.rcCloud.left - 1, g_btn.rcCloud.top - 1,
                           g_btn.rcCloud.right + 1, g_btn.rcCloud.bottom + 1 };
            FillCr(hdc, rcHot, kHoverFill); FrameCr(hdc, rcHot, kHoverLine);
        }
        IconCloud(hdc, g_btn.rcCloud);
        if (g_vis.nHotButton == 41)
        {
            RECT rcHot = { g_btn.rcFacebook.left - 1, g_btn.rcFacebook.top - 1,
                           g_btn.rcFacebook.right + 1, g_btn.rcFacebook.bottom + 1 };
            FillCr(hdc, rcHot, kHoverFill); FrameCr(hdc, rcHot, kHoverLine);
        }
        IconFacebook(hdc, g_btn.rcFacebook);
        // up/down arrows between the two tiles
        int ax = (g_btn.rcShareArrows.left + g_btn.rcShareArrows.right) / 2;
        int ay = (g_btn.rcShareArrows.top + g_btn.rcShareArrows.bottom) / 2;
        POINT up[3]   = { { ax - 3, ay - 5 }, { ax + 3, ay - 5 }, { ax, ay - 9 } };
        POINT down[3] = { { ax - 3, ay + 5 }, { ax + 3, ay + 5 }, { ax, ay + 9 } };
        Tri(hdc, up, 3, RGB(0x5A, 0x60, 0x67));
        Tri(hdc, down, 3, RGB(0x5A, 0x60, 0x67));

        RECT rcG = { g_btn.rcCloud.left, yLabel, g_btn.rcFacebook.right, yLabel + 14 };
        SelFont(hdc, g_fontGroup);
        DrawText1(hdc, L"Share", rcG, kGroupLabel, DT_CENTER);
        SelFont(hdc, g_fontTab);
    }

    // ---- Save movie / Sign in ----
    {
        if (g_vis.nHotButton == 50)
        {
            RECT rcHot = { g_btn.rcSaveMovie.left - 1, g_btn.rcSaveMovie.top - 1,
                           g_btn.rcSaveMovie.right + 1, g_btn.rcSaveMovie.bottom + 1 };
            FillCr(hdc, rcHot, kHoverFill); FrameCr(hdc, rcHot, kHoverLine);
        }
        IconSaveMovie(hdc, g_btn.rcSaveMovie);
        {
            RECT rcL1 = { g_btn.rcSaveMovie.left - 4, g_btn.rcSaveMovie.bottom + 2,
                          g_btn.rcSaveMovie.right + 6, g_btn.rcSaveMovie.bottom + 14 };
            RECT rcL2 = { g_btn.rcSaveMovie.left - 4, g_btn.rcSaveMovie.bottom + 13,
                          g_btn.rcSaveMovie.right + 6, g_btn.rcSaveMovie.bottom + 25 };
            DrawText1(hdc, L"Save", rcL1, kTabText, DT_CENTER);
            DrawText1(hdc, L"movie \x25BE", rcL2, kTabText, DT_CENTER);
        }
        if (g_vis.nHotButton == 51)
        {
            RECT rcHot = { g_btn.rcSignIn.left - 1, g_btn.rcSignIn.top - 1,
                           g_btn.rcSignIn.right + 1, g_btn.rcSignIn.bottom + 1 };
            FillCr(hdc, rcHot, kHoverFill); FrameCr(hdc, rcHot, kHoverLine);
        }
        IconSignIn(hdc, g_btn.rcSignIn);
        RECT rcL1 = { g_btn.rcSignIn.left - 4, g_btn.rcSignIn.bottom + 2,
                      g_btn.rcSignIn.right + 4, g_btn.rcSignIn.bottom + 14 };
        RECT rcL2 = { g_btn.rcSignIn.left - 4, g_btn.rcSignIn.bottom + 13,
                      g_btn.rcSignIn.right + 4, g_btn.rcSignIn.bottom + 25 };
        DrawText1(hdc, L"Sign", rcL1, kTabText, DT_CENTER);
        DrawText1(hdc, L"in", rcL2, kTabText, DT_CENTER);
    }

    SelFont(hdc, hOld);
}

// ============================================================================
// Content: preview pane + transport + filmstrip
// ============================================================================
struct ClipInfo
{
    ATL::CString strName;
    DWORD        dwId;
};

void FormatHns(LONGLONG llHns, LPWSTR pszBuf, size_t cchBuf)
{
    LONGLONG llTotalCs = llHns / 100000; // centiseconds
    int cs  = (int)(llTotalCs % 100);
    int sec = (int)((llTotalCs / 100) % 60);
    int min = (int)((llTotalCs / 6000) % 60);
    int hr  = (int)(llTotalCs / 360000);
    StringCchPrintfW(pszBuf, cchBuf, L"%d:%02d:%02d.%02d", hr, min, sec, cs);
}

void DrawTransport(HDC hdc, const RECT& rcZone)
{
    // seek row
    int yTrack = rcZone.top + 6;
    RECT rcTrack = { rcZone.left + 6, yTrack, rcZone.right - 6, yTrack + 3 };
    FillCr(hdc, rcTrack, kSeekTrack);

    int w = rcTrack.right - rcTrack.left;
    int xPlayed = rcTrack.left + w * kSeekPct / 100;
    RECT rcPlayed = { rcTrack.left, yTrack, xPlayed, yTrack + 3 };
    FillCr(hdc, rcPlayed, kTransportBlue);
    Circle(hdc, xPlayed, yTrack + 1, 4, kTransportBlue);

    // time readout above-right
    RECT rcTime = { rcZone.right - 150, rcZone.top - 14, rcZone.right - 2, rcZone.top + 2 };
    SelFont(hdc, g_fontGroup);
    WCHAR szTime[32];
    StringCchPrintfW(szTime, 32, L"0:00:00.00   0:00:00.00");
    DrawText1(hdc, szTime, rcTime, kTimeText, DT_RIGHT | DT_VCENTER);

    // transport buttons centered
    int cx = (rcZone.left + rcZone.right) / 2;
    int cy = rcZone.top + 30;
    // previous frame
    RECT rcB1 = { cx - 34, cy - 9, cx - 14, cy + 9 };
    {
        int x0 = rcB1.left + 4;
        RECT bar = { x0, cy - 7, x0 + 2, cy + 7 };
        FillCr(hdc, bar, kTransportBlue);
        POINT t[3] = { { x0 + 14, cy - 7 }, { x0 + 14, cy + 7 }, { x0 + 3, cy } };
        Tri(hdc, t, 3, kTransportBlue);
    }
    // play
    {
        POINT t[3] = { { cx - 4, cy - 7 }, { cx - 4, cy + 7 }, { cx + 8, cy } };
        Tri(hdc, t, 3, kTransportBlue);
    }
    // next frame
    {
        int x0 = cx + 14;
        POINT t[3] = { { x0, cy - 7 }, { x0, cy + 7 }, { x0 + 11, cy } };
        Tri(hdc, t, 3, kTransportBlue);
        RECT bar = { x0 + 12, cy - 7, x0 + 14, cy + 7 };
        FillCr(hdc, bar, kTransportBlue);
    }
    SelFont(hdc, g_fontTab);
}

void DrawClip(HDC hdc, const RECT& rc, DWORD dwSeed, bool bSelected)
{
    FillCr(hdc, rc, kClipBg);

    // sprocket strips
    RECT rcL = { rc.left, rc.top, rc.left + kSprocketW, rc.bottom };
    RECT rcR = { rc.right - kSprocketW, rc.top, rc.right, rc.bottom };
    FillCr(hdc, rcL, kSprocketLight);
    FillCr(hdc, rcR, kSprocketLight);
    int y = rc.top;
    int row = 0;
    while (y < rc.bottom)
    {
        int h = (row % 2 == 0) ? 4 : 3;
        int yEnd = y + h; if (yEnd > rc.bottom) yEnd = rc.bottom;
        RECT bandL = { rcL.left, y, rcL.right, yEnd };
        RECT bandR = { rcR.left, y, rcR.right, yEnd };
        FillCr(hdc, bandL, (row % 2 == 0) ? kSprocketLight : kSprocketDark);
        FillCr(hdc, bandR, (row % 2 == 0) ? kSprocketLight : kSprocketDark);
        y += h;
        ++row;
    }
    LineCr(hdc, rcL.right, rc.top, rcL.right, rc.bottom, kSprocketEdge);
    LineCr(hdc, rcR.left, rc.top, rcR.left, rc.bottom, kSprocketEdge);

    // waveform body (deterministic, seeded by the media item id)
    int x0 = rcL.right + 2;
    int x1 = rcR.left - 2;
    int baseY = rc.top + (rc.bottom - rc.top) * 66 / 100;
    int amp   = (rc.bottom - rc.top) * 46 / 100;

    DWORD s = dwSeed ? dwSeed : 1;
    const int kStep = 3;
    std::vector<POINT> pts;
    pts.reserve((x1 - x0) / kStep + 3);
    for (int x = x0; x <= x1; x += kStep)
    {
        s = (s * 1103515245 + 12345) & 0x7fffffff;
        int a = (int)(((0.25 + (s / 2147483648.0) * 0.75)) * amp);
        pts.push_back({ x, baseY - a });
    }
    pts.push_back({ x1, baseY });
    pts.push_back({ x0, baseY });
    Tri(hdc, pts.data(), (int)pts.size(), kWaveFill);

    // horizon line
    LineCr(hdc, x0, baseY + 6, x1, baseY + 6, RGB(0x3A, 0x41, 0x48));

    if (bSelected)
    {
        RECT rcSel = { rc.left - 1, rc.top - 1, rc.right + 1, rc.bottom + 1 };
        FrameCr(hdc, rcSel, kSelBlue);
    }
    else
    {
        FrameCr(hdc, rc, kClipBorder);
    }
}

void DrawFilmstrip(HDC hdc, const RECT& rc, ::SundanceAppMain* pApp)
{
    FillCr(hdc, rc, kWhite);
    if (!pApp || !pApp->IsProjectOpen() || !pApp->GetProject())
        return;

    StoryboardManager::MovieProject* pProject = pApp->GetProject();
    size_t cItems = pProject->GetMediaItemCount();
    if (cItems == 0)
        return;

    int x = rc.left + 12;
    int w = (rc.right - rc.left) - 24 - 12; // leave room for the scrollbar
    int y = rc.top + 10;

    for (size_t i = 0; i < cItems; ++i)
    {
        StoryboardManager::ProjectMediaItem* pItem = pProject->GetMediaItem(i);
        if (!pItem)
            continue;

        RECT rcClip = { x, y, x + w, y + kClipH };
        DrawClip(hdc, rcClip, pItem->GetMediaId(), (int)i == g_vis.nSelectedClip);

        // selection label chip under the selected clip
        if ((int)i == g_vis.nSelectedClip)
        {
            ATL::CString strName = pItem->GetSourcePath();
            int nSlash = strName.ReverseFind(L'\\');
            if (nSlash >= 0)
                strName = strName.Mid(nSlash + 1);

            RECT rcChip = { x, y + kClipH + 2, x + 12 + 8 + 130, y + kClipH + 20 };
            FillCr(hdc, rcChip, kWhite);
            FrameCr(hdc, rcChip, kChipBorder);
            RECT rcIcon = { rcChip.left + 4, rcChip.top + 3,
                            rcChip.left + 13, rcChip.bottom - 3 };
            FillCr(hdc, rcIcon, kTransportBlue);
            RECT rcText = { rcChip.left + 18, rcChip.top, rcChip.right - 2, rcChip.bottom };
            SelFont(hdc, g_fontGroup);
            DrawText1(hdc, strName, rcText, kTabText, DT_LEFT | DT_VCENTER);
            SelFont(hdc, g_fontTab);
            y += 22;
        }

        y += kClipH + kClipGap;
    }
}

void DrawStatusBar(HDC hdc, const RECT& rc, ::SundanceAppMain* pApp)
{
    FillCr(hdc, rc, kStatusBg);
    LineCr(hdc, rc.left, rc.top, rc.right, rc.top, kStatusLine);

    SelFont(hdc, g_fontGroup);

    // "Item N of M"
    WCHAR szItems[64];
    szItems[0] = L'\0';
    size_t cItems = 0;
    if (pApp && pApp->IsProjectOpen() && pApp->GetProject())
        cItems = pApp->GetProject()->GetMediaItemCount();
    if (cItems > 0 && g_vis.nSelectedClip >= 0)
        StringCchPrintfW(szItems, 64, L"Item %d of %zu", g_vis.nSelectedClip + 1, cItems);
    RECT rcItems = { rc.left + 8, rc.top, rc.left + 220, rc.bottom };
    DrawText1(hdc, szItems, rcItems, kStatusText, DT_LEFT | DT_VCENTER);

    // -- right cluster: mute, minus, zoom slider, plus --
    int xRight = rc.right - 10;
    RECT rcPlus = { xRight - 8, rc.top, xRight, rc.bottom };
    DrawText1(hdc, L"+", rcPlus, RGB(0x56, 0x60, 0x6A), DT_CENTER | DT_VCENTER);

    int xSliderRight = rcPlus.left - 6;
    int wSlider = 130;
    int xSliderLeft = xSliderRight - wSlider;
    int yMid = (rc.top + rc.bottom) / 2;
    RECT rcTrack = { xSliderLeft, yMid - 1, xSliderRight, yMid + 2 };
    FillCr(hdc, rcTrack, kZoomTrack);
    int xThumb = xSliderLeft + wSlider * 62 / 100;
    Circle(hdc, xThumb, yMid, 5, kTransportBlue);

    RECT rcMinus = { xSliderLeft - 16, rc.top, xSliderLeft - 4, rc.bottom };
    DrawText1(hdc, L"\x2013", rcMinus, RGB(0x56, 0x60, 0x6A), DT_CENTER | DT_VCENTER);

    // speaker glyph
    int sx = xSliderLeft - 30;
    POINT spk[5] = {
        { sx - 6, yMid - 3 }, { sx - 3, yMid - 3 }, { sx + 1, yMid - 7 },
        { sx + 1, yMid + 7 }, { sx - 3, yMid + 3 }
    };
    Tri(hdc, spk, 5, RGB(0x56, 0x60, 0x6A));
    HPEN hPen = CreatePen(PS_SOLID, 1, RGB(0x56, 0x60, 0x6A));
    HPEN hOld = (HPEN)SelectObject(hdc, hPen);
    Arc(hdc, sx + 2, yMid - 5, sx + 8, yMid + 5, sx + 3, yMid - 4, sx + 3, yMid + 4);
    SelectObject(hdc, hOld);
    DeleteObject(hPen);

    SelFont(hdc, g_fontTab);
}

} // namespace

// ============================================================================
// Public API
// ============================================================================
void MainVisual::Render(HWND hWnd, HDC hdc, const RECT& rcClient,
                        ::SundanceAppMain* pApp)
{
    EnsureFonts();

    // Drop a stale selection that no longer matches the project (media
    // removed, project closed or reloaded) so the contextual tabs and the
    // "Item N of M" readout never reference a nonexistent clip. Valid
    // selections and the no-selection state are preserved.
    int cMedia = 0;
    if (pApp && pApp->IsProjectOpen() && pApp->GetProject())
        cMedia = static_cast<int>(pApp->GetProject()->GetMediaItemCount());
    if (g_vis.nSelectedClip >= cMedia)
        g_vis.nSelectedClip = -1;

    int w = rcClient.right - rcClient.left;
    int h = rcClient.bottom - rcClient.top;
    if (w <= 0 || h <= 0)
        return;

    // double buffer
    HDC hdcMem = CreateCompatibleDC(hdc);
    HBITMAP hbm = CreateCompatibleBitmap(hdc, w, h);
    HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbm);

    RECT rcAll = { 0, 0, w, h };
    FillCr(hdcMem, rcAll, kWhite);

    // -- ribbon --
    RECT rcStrip = { 0, 0, w, kTabStripH };
    RECT rcBody  = { 0, kTabStripH, w, kTabStripH + kRibbonBodyH };
    DrawTabs(hdcMem, rcStrip);
    DrawRibbonBody(hdcMem, rcBody);

    // -- content --
    int nContentW = w - kSplitterW - kFilmstripW;
    if (nContentW < 0) nContentW = 0;
    RECT rcContent = { 0, kTabStripH + kRibbonBodyH, nContentW, h - kStatusBarH };
    RECT rcSplit   = { rcContent.right, rcContent.top,
                       rcContent.right + kSplitterW, rcContent.bottom };
    RECT rcStrip2  = { rcSplit.right, rcContent.top, w, rcContent.bottom };

    bool bMedia = pApp && pApp->IsProjectOpen() && pApp->GetProject() &&
                  pApp->GetProject()->GetMediaItemCount() > 0;

    FillCr(hdcMem, rcContent, kWhite);
    FillCr(hdcMem, rcSplit, kSplitterBg);

    if (bMedia)
    {
        // letterboxed 16:9 preview stage
        int stageW = rcContent.right - rcContent.left - 24;
        int stageH = (rcContent.bottom - kTransportH - rcContent.top) - 16;
        int vW = stageW;
        int vH = vW * 9 / 16;
        if (vH > stageH)
        {
            vH = stageH;
            vW = vH * 16 / 9;
        }
        RECT rcVideo = {
            rcContent.left + (rcContent.right - rcContent.left - vW) / 2,
            rcContent.top + (stageH + 8 - vH) / 2,
            rcContent.left + (rcContent.right - rcContent.left - vW) / 2 + vW,
            rcContent.top + (stageH + 8 - vH) / 2 + vH };
        FillCr(hdcMem, rcVideo, RGB(0, 0, 0));

        // transport zone
        RECT rcZone = { rcContent.left + 8, rcContent.bottom - kTransportH,
                        rcContent.right - 8, rcContent.bottom };
        DrawTransport(hdcMem, rcZone);
    }

    DrawFilmstrip(hdcMem, rcStrip2, pApp);

    // -- status bar --
    RECT rcStatus = { 0, h - kStatusBarH, w, h };
    DrawStatusBar(hdcMem, rcStatus, pApp);

    BitBlt(hdc, 0, 0, w, h, hdcMem, 0, 0, SRCCOPY);

    SelectObject(hdcMem, hbmOld);
    DeleteObject(hbm);
    DeleteDC(hdcMem);
}

int MainVisual::OnLButtonDown(HWND hWnd, const POINT& pt, ::SundanceAppMain* pApp)
{
    RECT rcFullStrip = { 0 };
    ::GetClientRect(hWnd, &rcFullStrip);
    rcFullStrip.bottom = kTabStripH;

    RECT rcTabs[kTabCount];
    RECT rcCtxVideo, rcCtxText;
    LayoutTabs(rcFullStrip, rcTabs, &rcCtxVideo, &rcCtxText);

    // File application menu
    if (PtInRect(&rcTabs[0], pt))
    {
        HMENU hMenu = CreatePopupMenu();
        AppendMenuW(hMenu, MF_STRING, MainVisualMenuOpenProject,   L"Open project\tCtrl+O");
        AppendMenuW(hMenu, MF_STRING, MainVisualMenuSaveProject,   L"Save project\tCtrl+S");
        AppendMenuW(hMenu, MF_STRING, MainVisualMenuSaveProjectAs, L"Save project as");
        AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
        AppendMenuW(hMenu, MF_STRING, MainVisualMenuSaveMovie,     L"Save movie");
        AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
        AppendMenuW(hMenu, MF_STRING, MainVisualMenuOptions,       L"Options");
        AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
        AppendMenuW(hMenu, MF_STRING, MainVisualMenuExit,          L"Exit");

        POINT ptMenu = pt;
        ClientToScreen(hWnd, &ptMenu);
        int nCmd = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_LEFTBUTTON,
                                  ptMenu.x, ptMenu.y, 0, hWnd, NULL);
        DestroyMenu(hMenu);
        return nCmd;
    }

    // static tabs
    for (int i = 1; i < kTabCount; ++i)
    {
        if (PtInRect(&rcTabs[i], pt))
        {
            if (g_vis.nActiveTab != i)
            {
                g_vis.nActiveTab = i;
                InvalidateRect(hWnd, NULL, FALSE);
            }
            return MainVisualMenuNone;
        }
    }

    // contextual tabs
    if (g_vis.nSelectedClip >= 0 && PtInRect(&rcCtxVideo, pt))
    {
        g_vis.nActiveTab = 1; // Edit routes back to the Home visual group
        InvalidateRect(hWnd, NULL, FALSE);
        return MainVisualMenuNone;
    }

    // filmstrip clips
    int w = rcFullStrip.right;
    int h = rcFullStrip.bottom;
    int nPaneX = w - kSplitterW - kFilmstripW;
    if (nPaneX < 0) nPaneX = 0;
    RECT rcStripPane = { nPaneX, kTabStripH + kRibbonBodyH, w, h - kStatusBarH };
    if (PtInRect(&rcStripPane, pt) && pApp && pApp->IsProjectOpen() && pApp->GetProject())
    {
        size_t cItems = pApp->GetProject()->GetMediaItemCount();
        int x = rcStripPane.left + 12;
        int cw = (rcStripPane.right - rcStripPane.left) - 24 - 12;
        int y = rcStripPane.top + 10;
        for (size_t i = 0; i < cItems; ++i)
        {
            RECT rcClip = { x, y, x + cw, y + kClipH };
            if (PtInRect(&rcClip, pt))
            {
                g_vis.nSelectedClip = (int)i;
                InvalidateRect(hWnd, NULL, FALSE);
                return MainVisualMenuNone;
            }
            y += kClipH + kClipGap;
            if ((int)i == g_vis.nSelectedClip)
                y += 22; // label chip
        }
        if (g_vis.nSelectedClip != -1)
        {
            g_vis.nSelectedClip = -1;
            InvalidateRect(hWnd, NULL, FALSE);
        }
    }

    return MainVisualMenuNone;
}

bool MainVisual::OnMouseMove(HWND hWnd, const POINT& pt)
{
    if (!g_vis.bTrackMouse)
    {
        TRACKMOUSEEVENT tme = { sizeof(tme), TME_LEAVE, hWnd, 0 };
        TrackMouseEvent(&tme);
        g_vis.bTrackMouse = true;
    }

    bool bChanged = false;

    RECT rcFullStrip = { 0 };
    ::GetClientRect(hWnd, &rcFullStrip);
    rcFullStrip.bottom = kTabStripH;

    RECT rcTabs[kTabCount];
    RECT rcCtxVideo, rcCtxText;
    LayoutTabs(rcFullStrip, rcTabs, &rcCtxVideo, &rcCtxText);

    int nHot = -1;
    if (PtInRect(&rcTabs[0], pt)) nHot = 0;
    for (int i = 1; i < kTabCount && nHot < 0; ++i)
        if (PtInRect(&rcTabs[i], pt)) nHot = i;
    if (g_vis.nSelectedClip >= 0 && nHot < 0 && PtInRect(&rcCtxVideo, pt)) nHot = 6;

    if (nHot != g_vis.nHotTab)
    {
        g_vis.nHotTab = nHot;
        bChanged = true;
    }

    // ribbon buttons (only on Home)
    int nBtn = -1;
    if (g_vis.nActiveTab == 1 &&
        pt.y >= kTabStripH && pt.y < kTabStripH + kRibbonBodyH)
    {
        if (PtInRect(&g_btn.rcPaste, pt)) nBtn = 1;
        else if (PtInRect(&g_btn.rcAddMedia, pt)) nBtn = 2;
        else if (PtInRect(&g_btn.rcAddMusic, pt)) nBtn = 3;
        else if (PtInRect(&g_btn.rcCloud, pt)) nBtn = 40;
        else if (PtInRect(&g_btn.rcFacebook, pt)) nBtn = 41;
        else if (PtInRect(&g_btn.rcSaveMovie, pt)) nBtn = 50;
        else if (PtInRect(&g_btn.rcSignIn, pt)) nBtn = 51;
        else if (PtInRect(&g_btn.rcRemove, pt)) nBtn = 32;
        else if (PtInRect(&g_btn.rcSelectAll, pt)) nBtn = 33;
        else
        {
            for (int i = 0; i < 3; ++i)
            {
                if (PtInRect(&g_btn.rcStack1[i], pt)) { nBtn = 10 + i; break; }
                if (PtInRect(&g_btn.rcStack2[i], pt)) { nBtn = 20 + i; break; }
            }
            if (nBtn < 0)
            {
                if (PtInRect(&g_btn.rcRotL, pt)) nBtn = 30;
                else if (PtInRect(&g_btn.rcRotR, pt)) nBtn = 31;
            }
        }
    }
    if (nBtn != g_vis.nHotButton)
    {
        g_vis.nHotButton = nBtn;
        bChanged = true;
    }

    if (bChanged)
        InvalidateRect(hWnd, NULL, FALSE);
    return bChanged;
}

void MainVisual::OnMouseLeave(HWND hWnd)
{
    g_vis.bTrackMouse = false;
    if (g_vis.nHotTab != -1 || g_vis.nHotButton != -1)
    {
        g_vis.nHotTab = -1;
        g_vis.nHotButton = -1;
        InvalidateRect(hWnd, NULL, FALSE);
    }
}

void MainVisual::SetSelectedClip(int nIndex)
{
    g_vis.nSelectedClip = nIndex;
}

int MainVisual::GetSelectedClip()
{
    return g_vis.nSelectedClip;
}

} // namespace SundanceUI
