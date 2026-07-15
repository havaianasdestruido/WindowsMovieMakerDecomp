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

// ============================================================================
// Extended timeline behavior resource IDs
// ============================================================================
#define IDR_TIMELINE_BEHAVIOR_SELECTION             3901
#define IDR_TIMELINE_BEHAVIOR_DRAG_DROP             3902
#define IDR_TIMELINE_BEHAVIOR_RESIZE                3903
#define IDR_TIMELINE_BEHAVIOR_TRIM                  3904
#define IDR_TIMELINE_BEHAVIOR_SPLIT                 3905
#define IDR_TIMELINE_BEHAVIOR_REORDER               3906
#define IDR_TIMELINE_BEHAVIOR_ZOOM                  3907
#define IDR_TIMELINE_BEHAVIOR_SCROLL                3908
#define IDR_TIMELINE_BEHAVIOR_SNAP                  3909

// ============================================================================
// Ribbon extended behavior resource IDs
// ============================================================================
#define IDR_RIBBON_COMMAND_HANDLER                  4001
#define IDR_RIBBON_EVENT_HANDLER                    4002
#define IDR_RIBBON_SITE                             4003
#define IDR_RIBBON_CONTEXTUAL_UI                    4004
#define IDR_RIBBON_QUICK_ACCESS                     4005
#define IDR_RIBBON_GALLERY_BEHAVIOR                 4006
#define IDR_RIBBON_RECENT_ITEMS                     4007

// ============================================================================
// Selection / Editor behavior resource IDs
// ============================================================================
#define IDR_SELECTION_ROOT_IMPL                     4101
#define IDR_SELECTION_MANAGER                       4102
#define IDR_SELECTION_RANGE                         4103
#define IDR_SELECTION_CONTEXT                       4104
#define IDR_SELECTION_UI                            4105
#define IDR_EDITOR_TOOLBAR                          4106
#define IDR_EDITOR_COMMAND_HANDLER                  4107
#define IDR_EDITOR_STATE                            4108

// ============================================================================
// Sundance element behavior resource IDs
// ============================================================================
#define IDR_SUNDANCE_RIBBON_ELEMENT_BEHAVIOR        4201
#define IDR_SUNDANCE_STATUS_BAR_ELEMENT_BEHAVIOR    4202
#define IDR_SUNDANCE_TIMELINE_ELEMENT_BEHAVIOR      4203
#define IDR_SUNDANCE_PREVIEW_ELEMENT_BEHAVIOR       4204
#define IDR_SUNDANCE_PROPERTY_ELEMENT_BEHAVIOR      4205

// ============================================================================
// Webcam / Narration behavior resource IDs
// ============================================================================
#define IDR_WEBCAM_UI_BEHAVIOR                      4301
#define IDR_WEBCAM_PREVIEW_BEHAVIOR                 4302
#define IDR_WEBCAM_CAPTURE_BEHAVIOR                 4303
#define IDR_NARRATION_UI_BEHAVIOR                   4304
#define IDR_NARRATION_RECORD_BEHAVIOR               4305
#define IDR_NARRATION_PLAYBACK_BEHAVIOR             4306

// ============================================================================
// Encode profile extended behavior resource IDs
// ============================================================================
#define IDR_USER_ENCODE_PROFILE_MANAGER             4401
#define IDR_USER_ENCODE_PROFILE_PRESET              4402
#define IDR_USER_ENCODE_PROFILE_CUSTOM              4403
#define IDR_USER_ENCODE_PROFILE_REGISTRY            4404

// ============================================================================
// Publish dialog behavior resource IDs
// ============================================================================
#define IDR_PUBLISH_DIALOG_BASE                     4501
#define IDR_PUBLISH_DIALOG_YOUTUBE                  4502
#define IDR_PUBLISH_DIALOG_FACEBOOK                 4503
#define IDR_PUBLISH_DIALOG_SKYDRIVE                 4504
#define IDR_PUBLISH_DIALOG_EMAIL                    4505
#define IDR_PUBLISH_DIALOG_LOCAL_FILE               4506

// ============================================================================
// Miscellaneous UI behavior resource IDs
// ============================================================================
#define IDR_PROGRESS_BAR_UI_BEHAVIOR                4601
#define IDR_ERROR_DIALOG_UI_BEHAVIOR                4602
#define IDR_WARNING_DIALOG_UI_BEHAVIOR              4603
#define IDR_INFO_DIALOG_UI_BEHAVIOR                 4604
#define IDR_CONFIRMATION_DIALOG_UI_BEHAVIOR         4605
#define IDR_FILE_BROWSER_UI_BEHAVIOR                4606
#define IDR_FOLDER_BROWSER_UI_BEHAVIOR              4607
#define IDR_COLOR_PICKER_UI_BEHAVIOR                4608
#define IDR_FONT_PICKER_UI_BEHAVIOR                 4609
#define IDR_CROP_UI_BEHAVIOR                        4610

#endif // SUNDANCE_RESOURCE_IDS_H
