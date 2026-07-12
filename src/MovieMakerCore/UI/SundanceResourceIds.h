/*
 * SundanceResourceIds.h
 *
 * Resource ID definitions for Sundance DirectUI behavior classes.
 * These IDs are used in DECLARE_REGISTRY_RESOURCEID macros and
 * correspond to entries in the ATL registry map for each behavior.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once
#ifndef SUNDANCE_RESOURCE_IDS_H
#define SUNDANCE_RESOURCE_IDS_H

// ============================================================================
// Timeline behavior resource IDs
// ============================================================================
#define IDR_TIMELINE_BASE_BEHAVIOR                  3001
#define IDR_TIMELINE_BEHAVIOR                       3002
#define IDR_TIMELINE_ITEM_BEHAVIOR                  3003
#define IDR_TIMELINE_SECONDARY_TRACK_ITEM_BEHAVIOR  3004
#define IDR_TIMELINE_VISUAL_TRACK_ITEM_BEHAVIOR     3005
#define IDR_TIMELINE_ITEM_INPUT_BEHAVIOR            3006
#define IDR_TIMELINE_INSTRUCTIONS_BEHAVIOR          3007
#define IDR_TIMELINE_SELECTION_ROOT_BEHAVIOR        3008

// ============================================================================
// Timeline data source resource IDs
// ============================================================================
#define IDR_TIMELINE_DATA_SOURCE                    3101
#define IDR_TIMELINE_TEMPLATE_SOURCE                3102
#define IDR_TIMELINE_LAYOUT_MODE                    3103

// ============================================================================
// UI component resource IDs
// ============================================================================
#define IDR_SUNDANCE_MAIN_ELEMENT_BEHAVIOR          3201
#define IDR_AMP_MAIN_WINDOW_BEHAVIOR                3202
#define IDR_AMP_COMMAND_BAR_BEHAVIOR                3203
#define IDR_AMP_DATA_CONTEXT                        3204

// ============================================================================
// Dialog resource IDs
// ============================================================================
#define IDR_RENDER_SUMMARY_DIALOG                   3301
#define IDR_USER_ENCODE_PROFILE_DIALOG              3302
#define IDR_USER_ENCODE_PROFILE_RECOMMENDED         3303
#define IDR_USER_ENCODE_PROFILE_WLVS                3304
#define IDR_USER_ENCODE_PROFILE_INFO                3305
#define IDR_MULTIPLE_EFFECT_DIALOG                  3306
#define IDR_SUNDANCE_APPLICATION_OPTIONS_DIALOG     3307
#define IDR_DONT_SHOW_PROMPT_DIALOG                 3308
#define IDR_SUNDANCE_DONT_SHOW_PROMPT_DIALOG        3309

// ============================================================================
// Slider UI resource IDs
// ============================================================================
#define IDR_POPUP_SLIDER                            3401

// ============================================================================
// Progress UI resource IDs
// ============================================================================
#define IDR_PROGRESS_STATUS_BAR                     3501
#define IDR_PROGRESS_DIALOG                         3502

// ============================================================================
// Editing behavior resource IDs
// ============================================================================
#define IDR_TRIM_BEHAVIOR                           3601
#define IDR_TEXT_BOX_BEHAVIOR                       3602
#define IDR_RICH_EDIT_CONTROL_BEHAVIOR              3603
#define IDR_CARET_BEHAVIOR                          3604
#define IDR_CARET_SCROLL_BEHAVIOR                   3605
#define IDR_ABOVE_HOMER_BEHAVIOR                    3606
#define IDR_OPTIONS_DIALOG_BEHAVIOR                 3607
#define IDR_HELP_BEHAVIOR                           3608
#define IDR_CAPTURE_UI_BEHAVIOR                     3609

// ============================================================================
// Layout behavior resource IDs
// ============================================================================
#define IDR_FULLSCREEN_LAYOUT_BEHAVIOR              3701
#define IDR_RESIZEABLE_PANE_BEHAVIOR                3702
#define IDR_PROJECT_WORKSPACE_LAYOUT_BEHAVIOR       3703
#define IDR_INLINE_PREVIEW_LAYOUT_BEHAVIOR          3704
#define IDR_INLINE_PREVIEW_SLIDER_BEHAVIOR          3705

// ============================================================================
// Special behavior resource IDs
// ============================================================================
#define IDR_HOMER_HEAVY_LAYER_BEHAVIOR              3801
#define IDR_STANDARD_LAYER_BEHAVIOR                 3802
#define IDR_MULTIPLE_EFFECT_BEHAVIOR                3803
#define IDR_WEBCAM_ELEMENT_BEHAVIOR                 3804
#define IDR_AMP_PREVENT_SPARSE_MODE_BEHAVIOR        3805
#define IDR_AMP_SLIDE_ANIMATION_BEHAVIOR            3806
#define IDR_AMP_FADE_ANIMATION_BEHAVIOR             3807
#define IDR_USER_ENCODE_PROFILE_BEHAVIOR            3808

#endif // SUNDANCE_RESOURCE_IDS_H
