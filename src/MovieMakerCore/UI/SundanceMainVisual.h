/*
 * SundanceMainVisual.h
 *
 * Owner-drawn visual layer for the Sundance main window. Reproduces the
 * 16.4 ribbon shell measured from reference captures of the original
 * binary: File/Home/Animations/Visual Effects/Project/View tab strip with
 * contextual "Video Tools > Edit" / "Text Tools > Format" bands, the Home
 * ribbon groups (Clipboard, Add, AutoMovie themes, Editing, Share,
 * Save movie / Sign in), the split preview + filmstrip content area and
 * the light-blue status bar with the zoom slider.
 *
 * Geometry and palette match website/replica/moviemaker.html, which is the
 * pixel-verified reconstruction of the original shell at 96 DPI.
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once
#ifndef SUNDANCE_MAIN_VISUAL_H
#define SUNDANCE_MAIN_VISUAL_H

#include "../pch.h"

class SundanceAppMain;

namespace SundanceUI
{
    // Menu commands surfaced by the File application menu (returned from
    // MainVisual::OnLButtonDown when the popup dismisses).
    enum MainVisualMenuCmd
    {
        MainVisualMenuNone        = 0,
        MainVisualMenuOpenProject = 0xE104,
        MainVisualMenuSaveProject = 0xE103, // mirrors AppCommandIds::ID_APP_SAVE
        MainVisualMenuSaveProjectAs = 0xE106,
        MainVisualMenuSaveMovie   = 0xE203, // mirrors AppCommandIds::ID_APP_EXPORT
        MainVisualMenuOptions     = 0xE107,
        MainVisualMenuExit        = 0xE108,
    };

    class MainVisual
    {
    public:
        // Paints the full client-area shell (ribbon, content, status bar).
        static void Render(HWND hWnd, HDC hdc, const RECT& rcClient,
                           ::SundanceAppMain* pApp);

        // Mouse interaction. Returns a MainVisualMenuCmd when the File
        // application menu dismissed with a selection, otherwise
        // MainVisualMenuNone. Repaint is invalidated internally.
        static int  OnLButtonDown(HWND hWnd, const POINT& pt, ::SundanceAppMain* pApp);
        static bool OnMouseMove(HWND hWnd, const POINT& pt);
        static void OnMouseLeave(HWND hWnd);

        // Selection index into the project media list (-1 = none). Drives the
        // contextual tab bands, the clip outline and the "Item N of M" readout.
        static void SetSelectedClip(int nIndex);
        static int  GetSelectedClip();

    private:
        struct Layout; // geometry cache, see SundanceMainVisual.cpp
    };
}

#endif // SUNDANCE_MAIN_VISUAL_H
