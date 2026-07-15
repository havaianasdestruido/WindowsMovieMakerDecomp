/*
 * ResourceIds.h
 *
 * Resource ID definitions matching the MovieMakerCore.dll binary's resource
 * tree. These IDs correspond to resources embedded in the DLL: dialogs,
 * bitmaps, icons, menus, string tables, and DirectUI XML layouts.
 *
 * Resource types extracted from PE resource analysis:
 *   RT_DIALOG, RT_BITMAP, RT_ICON, RT_MENU, RT_STRING, RT_MANIFEST,
 *   "DUIXML", "UNICODERESOURCE"
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef RESOURCEIDS_H
#define RESOURCEIDS_H

// ============================================================================
// DuiDirectUI XML resource IDs
// ============================================================================
// These map to "DUIXML" and "UNICODERESOURCE" entries in the .rsrc section
// that define the DirectUI layout hierarchy.
//
#define IDR_DUIFRAME                    1001
#define IDR_TIMELINE                    1002
#define IDR_COMMANDBAR                  1003
#define IDR_MENUBAR                     1004
#define IDR_STATUSBAR                   1005
#define IDR_TOOLBAR                     1006
#define IDR_APP_OPTIONS_PANEL           1007
#define IDR_TITLE_EDITOR_PANEL          1008
#define IDR_CREDITS_EDITOR_PANEL        1009
#define IDR_TRANSITION_PICKER           1010
#define IDR_EFFECT_PICKER               1011
#define IDR_MEDIA_BROWSER_PANEL         1012
#define IDR_STORYBOARD_VIEW             1013
#define IDR_PREVIEW_PANEL               1014
#define IDR_EXPORT_DIALOG               1015
#define IDR_ABOUT_DIALOG                1016
#define IDR_WEBCAM_PREVIEW_PANEL        1017
#define IDR_NARRATION_PANEL             1018
#define IDR_AUTO_MUSIC_PANEL            1019
#define IDR_SNAPSHOT_PANEL              1020
#define IDR_CREDIT_ROLL                 1021
#define IDR_THEME_SELECTOR              1022
#define IDR_CAPTION_BUBBLE              1023
#define IDR_ANIMATION_PANEL             1024

// ============================================================================
// Dialog resource IDs (IDD_*)
// ============================================================================
#define IDD_EXPORT_WIZARD               2001
#define IDD_EXPORT_SETTINGS             2002
#define IDD_APP_OPTIONS                 2003
#define IDD_ABOUT                       2004
#define IDD_TITLE_TEXT_ENTRY            2005
#define IDD_CREDITS_TEXT_ENTRY          2006
#define IDD_RENDER_SUMMARY              2007
#define IDD_USER_ENCODE_PROFILE         2008
#define IDD_USER_ENCODE_PROFILE_INFO    2009
#define IDD_MULTIPLE_EFFECT_WARNING     2010
#define IDD_DONT_SHOW_PROMPT            2011
#define IDD_OPEN_PROJECT                2012
#define IDD_SAVE_PROJECT                2013
#define IDD_BURN_DISC                   2014
#define IDD_WEBCAM_SETUP                2015
#define IDD_NARRATION_SETUP             2016
#define IDD_AUTO_SAVE_RECOVERY          2017

// ============================================================================
// Bitmap resource IDs (IDB_*)
// ============================================================================
#define IDB_APP_LOGO                    3001
#define IDB_APP_LOGO_SMALL              3002
#define IDB_BUBBLE                      3003
#define IDB_TEXTEFFECT_STARBURST        3004
#define IDB_TEXTEFFECT_SIMPLE           3005
#define IDB_TEXTEFFECT_CINEMATIC        3006
#define IDB_TEXTEFFECT_BOUNCE           3007
#define IDB_TEXTEFFECT_FADEIN           3008
#define IDB_TEXTEFFECT_SCROLL           3009
#define IDB_TEXTEFFECT_SLIDELEFT        3010
#define IDB_TEXTEFFECT_SLIDERIGHT       3011
#define IDB_TEXTEFFECT_ZOOMIN           3012
#define IDB_TEXTEFFECT_ZOOMOUT          3013
#define IDB_TEXTEFFECT_SPIN             3014
#define IDB_TEXTEFFECT_WIPE             3015
#define IDB_TRANSITION_DISSOLVE         3016
#define IDB_TRANSITION_DISSOLVE_NOISE   3017
#define IDB_TRANSITION_WIPE_LEFT        3018
#define IDB_TRANSITION_WIPE_RIGHT       3019
#define IDB_TRANSITION_WIPE_UP          3020
#define IDB_TRANSITION_WIPE_DOWN        3021
#define IDB_TRANSITION_PUSH_LEFT        3022
#define IDB_TRANSITION_PUSH_RIGHT       3023
#define IDB_TRANSITION_PUSH_UP          3024
#define IDB_TRANSITION_PUSH_DOWN        3025
#define IDB_TRANSITION_FADE             3026
#define IDB_TRANSITION_CROSSFADE        3027
#define IDB_TRANSITION_SHRINK           3028
#define IDB_TRANSITION_GROW             3029
#define IDB_TRANSITION_FLIP             3030
#define IDB_TRANSITION_SPIN_IN          3031
#define IDB_TRANSITION_ZOOM             3032
#define IDB_TRANSITION_REVEAL_LEFT      3033
#define IDB_TRANSITION_REVEAL_RIGHT     3034
#define IDB_EFFECT_BW                  3035
#define IDB_EFFECT_SEPIA               3036
#define IDB_EFFECT_VINTAGE             3037
#define IDB_EFFECT_BRIGHTNESS          3038
#define IDB_EFFECT_CONTRAST            3039
#define IDB_EFFECT_BLUR                3040
#define IDB_EFFECT_SHARPEN             3041
#define IDB_EFFECT_POSTERIZE           3042
#define IDB_EFFECT_PIXELATE            3043
#define IDB_EFFECT_EDGE_DETECT         3044
#define IDB_EFFECT_EMBOSS              3045
#define IDB_EFFECT_ENGRAVE             3046
#define IDB_TIMELINE_BG                3047
#define IDB_TIMELINE_ITEM_VIDEO        3048
#define IDB_TIMELINE_ITEM_AUDIO        3049
#define IDB_TIMELINE_ITEM_MUSIC        3050
#define IDB_TIMELINE_ITEM_TITLE        3051
#define IDB_TIMELINE_ITEM_TRANSITION   3052
#define IDB_TIMELINE_SPLITTER          3053
#define IDB_TIMELINE_PLAYHEAD          3054
#define IDB_SCRUBBER_HANDLE             3055
#define IDB_BUTTON_NORMAL              3056
#define IDB_BUTTON_HOVER               3057
#define IDB_BUTTON_PRESSED             3058
#define IDB_BUTTON_DISABLED            3059
#define IDB_PROGRESS_BACKGROUND        3060
#define IDB_PROGRESS_FILL              3061
#define IDB_STARBURST                  3062
#define IDB_FILM_STRIP                 3063
#define IDB_VOLUME_ICON                3064
#define IDB_MUTE_ICON                  3065
#define IDB_PLAY_ICON                  3066
#define IDB_PAUSE_ICON                 3067
#define IDB_STOP_ICON                  3068
#define IDB_RECORD_ICON                3069
#define IDB_SKIP_BACK_ICON             3070
#define IDB_SKIP_FORWARD_ICON          3071

// ============================================================================
// Icon resource IDs (IDI_*)
// ============================================================================
#define IDI_APP_ICON                    4001
#define IDI_APP_ICON_SMALL              4002
#define IDI_APP_ICON_XSMALL             4003
#define IDI_VIDEO_FILE                  4004
#define IDI_AUDIO_FILE                  4005
#define IDI_IMAGE_FILE                  4006
#define IDI_PROJECT_FILE                4007
#define IDI_EXPORT_FILE                 4008
#define IDI_WARNING                     4009
#define IDI_ERROR                       4010
#define IDI_INFO                        4011
#define IDI_QUESTION                    4012

// ============================================================================
// Menu resource IDs (IDR_MENU_*)
// ============================================================================
#define IDR_MENU_MAIN                   5001
#define IDR_MENU_CONTEXT_TIMELINE       5002
#define IDR_MENU_CONTEXT_STORYBOARD     5003
#define IDR_MENU_CONTEXT_MEDIA          5004
#define IDR_MENU_CONTEXT_TITLE          5005
#define IDR_MENU_CONTEXT_TRANSITION     5006
#define IDR_MENU_CONTEXT_EFFECT         5007
#define IDR_MENU_FILE                   5008
#define IDR_MENU_EDIT                   5009
#define IDR_MENU_VIEW                   5010
#define IDR_MENU_PROJECT                5011
#define IDR_MENU_HELP                   5012
#define IDR_MENU_EXPORT                 5013
#define IDR_MENU_CAPTURE                5014

// ============================================================================
// String table IDs (STR_* / IDS_*)
// ============================================================================
// String table entries are in StringTableIds.h (IDs 1-5000 for string resources).
// These are additional named string constants for specific UI text.
//
#define IDS_APP_TITLE                   6001
#define IDS_APP_DESCRIPTION             6002
#define IDS_UNTITLED_PROJECT            6003
#define IDS_EXPORT_COMPLETE             6004
#define IDS_EXPORT_FAILED               6005
#define IDS_EXPORT_CANCELLED            6006
#define IDS_EXPORT_IN_PROGRESS          6007
#define IDS_SAVE_CHANGES_PROMPT         6008
#define IDS_OVERWRITE_PROMPT            6009
#define IDS_FILE_NOT_FOUND              6010
#define IDS_UNSUPPORTED_FORMAT          6011
#define IDS_NO_CAMERA_DETECTED          6012
#define IDS_NO_MICROPHONE_DETECTED      6013
#define IDS_CAPTURE_IN_PROGRESS         6014
#define IDS_AUTOSAVE_RECOVERY_PROMPT    6015
#define IDS_PROJECT_CORRUPT             6016
#define IDS_MEMORY_ERROR                6017
#define IDS_INITIALIZING                6018
#define IDS_ENCODING_VIDEO              6019
#define IDS_ENCODING_AUDIO              6020
#define IDS_LOADING_PROJECT             6021
#define IDS_SAVING_PROJECT              6022
#define IDS_IMPORTING_MEDIA             6023
#define IDS_GENERATING_THUMBNAILS       6024
#define IDS_NO_MEDIA_ITEMS              6025
#define IDS_DRAG_MEDIA_HERE             6026
#define IDS_CLICK_TO_ADD_TITLE          6027
#define IDS_TITLE_PLACEHOLDER           6028
#define IDS_CREDITS_PLACEHOLDER         6029

// ============================================================================
// Accelerator table IDs
// ============================================================================
#define IDR_ACCELERATOR_MAIN            7001
#define IDR_ACCELERATOR_TIMELINE        7002
#define IDR_ACCELERATOR_EDIT            7003

// ============================================================================
// Version resource IDs
// ============================================================================
#define IDR_VERSION_INFO                8001

// ============================================================================
// Manifest resource ID
// ============================================================================
#define IDR_MANIFEST                    9001

// ============================================================================
// Effect resource IDs (compiled HLSL effects)
// ============================================================================
#define IDR_COMMON_EFFECT               10001

#endif // RESOURCEIDS_H
