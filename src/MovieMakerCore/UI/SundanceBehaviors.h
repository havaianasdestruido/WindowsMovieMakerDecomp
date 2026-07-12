/*
 * SundanceBehaviors.h
 *
 * Master include header for all Sundance (Windows Live Movie Maker) DirectUI
 * behavior classes. This header pulls in every behavior category used by
 * MovieMakerCore.dll's UI layer.
 *
 * DirectUI behaviors implement IDuiBehaviorImpl and are created via
 * SundanceBehaviorFactory. Each .duxt resource file references behavior
 * class names that are resolved at load time through the factory.
 *
 * RTTI: Behavior classes listed per-header below.
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once
#ifndef SUNDANCE_BEHAVIORS_H
#define SUNDANCE_BEHAVIORS_H

#include "../../pch.h"
#include "../../MovieMakerCore.h"
#include "DuiInterfaces.h"

// ============================================================================
// Behavior category includes
// ============================================================================

// Timeline behaviors (main timeline control, items, tracks, input)
#include "TimelineBehavior.h"

// Timeline data sources and virtual layout support
#include "TimelineDataSources.h"

// Sundance UI components (main element, factory, window, command bar)
#include "SundanceUIComponents.h"

// Dialogs (render summary, encode profiles, effects, options)
#include "Dialogs.h"

// Slider and popup UI controls
#include "SliderUI.h"

// Progress indicators (status bar, modal dialog)
#include "ProgressUI.h"

// Editing behaviors (trim, text, rich edit, caret, capture)
#include "EditingBehaviors.h"

// Layout behaviors (fullscreen, resizeable, workspace, inline preview)
#include "LayoutBehaviors.h"

// Special behaviors (Homer layer, effects, webcam, animations)
#include "SpecialBehaviors.h"

// Clipboard chain window
#include "SundanceClipboardChainWindow.h"

// Native HWND hosting for DirectUI
#include "SundanceNativeHwndHost.h"

// GDI+/Direct2D brush wrapper
#include "UXBrush.h"

#endif // SUNDANCE_BEHAVIORS_H
