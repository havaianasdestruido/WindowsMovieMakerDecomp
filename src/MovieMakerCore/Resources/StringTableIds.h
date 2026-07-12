/*
 * StringTableIds.h
 *
 * String table resource IDs matching the 201 string tables found in the
 * MovieMakerCore.dll binary. String table resources are grouped in blocks
 * of 16 strings per table entry (standard Win32 resource layout).
 *
 * ID range: 1 - 5000
 * Block size: 16 strings per resource block
 *
 * These IDs map to the RT_STRING resource type in the PE resource directory.
 * Each block contains strings grouped by functional area (UI text, errors,
 * prompts, file dialog filters, etc.).
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef STRINGTABLEIDS_H
#define STRINGTABLEIDS_H

// ============================================================================
// Block 1: Core UI strings (IDs 1-16)
// ============================================================================
#define IDS_STRING_0001                 1       // Application title
#define IDS_STRING_0002                 2       // Application description
#define IDS_STRING_0003                 3       // Version string
#define IDS_STRING_0004                 4       // Copyright notice
#define IDS_STRING_0005                 5       // "Untitled" project name
#define IDS_STRING_0006                 6       // File filter: Movie Maker projects
#define IDS_STRING_0007                 7       // File filter: Video files
#define IDS_STRING_0008                 8       // File filter: Audio files
#define IDS_STRING_0009                 9       // File filter: Image files
#define IDS_STRING_0010                 10      // File filter: All files
#define IDS_STRING_0011                 11      // File filter: WMV files
#define IDS_STRING_0012                 12      // File filter: MP4 files
#define IDS_STRING_0013                 13      // File filter: WMA files
#define IDS_STRING_0014                 14      // File filter: MP3 files
#define IDS_STRING_0015                 15      // File filter: JPEG files
#define IDS_STRING_0016                 16      // File filter: PNG files

// ============================================================================
// Block 2: Menu strings (IDs 17-32)
// ============================================================================
#define IDS_MENU_FILE                   17
#define IDS_MENU_EDIT                   18
#define IDS_MENU_VIEW                   19
#define IDS_MENU_PROJECT                20
#define IDS_MENU_TOOLS                  21
#define IDS_MENU_HELP                   22
#define IDS_MENU_IMPORT                 23
#define IDS_MENU_EXPORT                 24
#define IDS_MENU_NEW_PROJECT            25
#define IDS_MENU_OPEN_PROJECT           26
#define IDS_MENU_SAVE_PROJECT           27
#define IDS_MENU_SAVE_AS                28
#define IDS_MENU_EXIT                   29
#define IDS_MENU_UNDO                   30
#define IDS_MENU_REDO                   31
#define IDS_MENU_CUT                    32

// ============================================================================
// Block 3: More menu/toolbar strings (IDs 33-48)
// ============================================================================
#define IDS_MENU_COPY                   33
#define IDS_MENU_PASTE                  34
#define IDS_MENU_DELETE                  35
#define IDS_MENU_SELECT_ALL             36
#define IDS_MENU_ADD_TITLE              37
#define IDS_MENU_ADD_CREDITS            38
#define IDS_MENU_ADD_MUSIC              39
#define IDS_MENU_ADD_TRANSITION         40
#define IDS_MENU_ADD_EFFECT             41
#define IDS_MENU_SPLIT                  42
#define IDS_MENU_TRIM                   43
#define IDS_MENU_SPEED                  44
#define IDS_MENU_VOLUME                 45
#define IDS_MENU_ZOOM_IN                46
#define IDS_MENU_ZOOM_OUT               47
#define IDS_MENU_STORYBOARD_VIEW        48

// ============================================================================
// Block 4: Dialog strings (IDs 49-64)
// ============================================================================
#define IDS_DIALOG_EXPORT_TITLE         49
#define IDS_DIALOG_EXPORT_DESC          50
#define IDS_DIALOG_OPTIONS_TITLE        51
#define IDS_DIALOG_ABOUT_TITLE          52
#define IDS_DIALOG_TITLE_ENTRY_TITLE    53
#define IDS_DIALOG_CREDITS_ENTRY_TITLE  54
#define IDS_DIALOG_RENDER_SUMMARY_TITLE 55
#define IDS_DIALOG_PROFILE_TITLE        56
#define IDS_DIALOG_PROFILE_DESC         57
#define IDS_DIALOG_MULTIPLE_EFFECT_WARN 58
#define IDS_DIALOG_OPEN_TITLE           59
#define IDS_DIALOG_SAVE_TITLE           60
#define IDS_DIALOG_BURN_TITLE           61
#define IDS_DIALOG_WEBCAM_TITLE         62
#define IDS_DIALOG_NARRATION_TITLE      63
#define IDS_DIALOG_AUTOSAVE_TITLE       64

// ============================================================================
// Block 5: Button/label text (IDs 65-80)
// ============================================================================
#define IDS_BTN_OK                      65
#define IDS_BTN_CANCEL                  66
#define IDS_BTN_YES                     67
#define IDS_BTN_NO                      68
#define IDS_BTN_SAVE                    69
#define IDS_BTN_DONT_SAVE               70
#define IDS_BTN_RETRY                   71
#define IDS_BTN_IMPORT                  72
#define IDS_BTN_EXPORT                  73
#define IDS_BTN_BROWSE                  74
#define IDS_BTN_ADD                     75
#define IDS_BTN_REMOVE                  76
#define IDS_BTN_PLAY                    77
#define IDS_BTN_PAUSE                   78
#define IDS_BTN_STOP                    79
#define IDS_BTN_RECORD                  80

// ============================================================================
// Block 6: Status/progress strings (IDs 81-96)
// ============================================================================
#define IDS_STATUS_READY                81
#define IDS_STATUS_LOADING              82
#define IDS_STATUS_SAVING               83
#define IDS_STATUS_EXPORTING            84
#define IDS_STATUS_IMPORTING            85
#define IDS_STATUS_ENCODING             86
#define IDS_STATUS_DECODING             87
#define IDS_STATUS_RENDERING            88
#define IDS_STATUS_COMPLETE             89
#define IDS_STATUS_CANCELLED            90
#define IDS_STATUS_ERROR                91
#define IDS_STATUS_INITIALIZING         92
#define IDS_STATUS_LOADING_MEDIA        93
#define IDS_STATUS_GENERATING_THUMBS    94
#define IDS_STATUS_ENCODING_VIDEO       95
#define IDS_STATUS_ENCODING_AUDIO       96

// ============================================================================
// Block 7: Error messages (IDs 97-112)
// ============================================================================
#define IDS_ERR_GENERIC                 97
#define IDS_ERR_OUT_OF_MEMORY           98
#define IDS_ERR_FILE_NOT_FOUND          99
#define IDS_ERR_ACCESS_DENIED           100
#define IDS_ERR_INVALID_FORMAT          101
#define IDS_ERR_UNSUPPORTED_CODEC       102
#define IDS_ERR_DISK_FULL               103
#define IDS_ERR_DEVICE_NOT_FOUND        104
#define IDS_ERR_CAPTURE_FAILED          105
#define IDS_ERR_ENCODE_FAILED           106
#define IDS_ERR_PROJECT_CORRUPT         107
#define IDS_ERR_SAVE_FAILED             108
#define IDS_ERR_LOAD_FAILED             109
#define IDS_ERR_NO_RENDERER             110
#define IDS_ERR_D3D_NOT_AVAILABLE       111
#define IDS_ERR_MF_NOT_AVAILABLE        112

// ============================================================================
// Block 8: Prompt/confirmation strings (IDs 113-128)
// ============================================================================
#define IDS_PROMPT_SAVE_CHANGES         113
#define IDS_PROMPT_OVERWRITE             114
#define IDS_PROMPT_DELETE_EXTENT         115
#define IDS_PROMPT_DELETE_EFFECT         116
#define IDS_PROMPT_DELETE_TRANSITION     117
#define IDS_PROMPT_REPLACE_MUSIC         118
#define IDS_PROMPT_AUTOSAVE_RECOVERY     119
#define IDS_PROMPT_PROJECT_NEWER         120
#define IDS_PROMPT_WEBCAM_IN_USE         121
#define IDS_PROMPT_MIC_IN_USE            122
#define IDS_PROMPT_NO_MEDIA              123
#define IDS_PROMPT_EMPTY_PROJECT         124
#define IDS_PROMPT_LONG_PROJECT          125
#define IDS_PROMPT_LARGE_FILE            126
#define IDS_PROMPT_LOW_DISK              127
#define IDS_PROMPT_LOW_MEMORY            128

// ============================================================================
// Block 9: Tooltip strings (IDs 129-144)
// ============================================================================
#define IDS_TOOLTIP_PLAY                 129
#define IDS_TOOLTIP_PAUSE                130
#define IDS_TOOLTIP_STOP                 131
#define IDS_TOOLTIP_RECORD               132
#define IDS_TOOLTIP_SKIP_BACK            133
#define IDS_TOOLTIP_SKIP_FORWARD         134
#define IDS_TOOLTIP_VOLUME               135
#define IDS_TOOLTIP_MUTE                 136
#define IDS_TOOLTIP_TRIM_START           137
#define IDS_TOOLTIP_TRIM_END             138
#define IDS_TOOLTIP_SPLIT                139
#define IDS_TOOLTIP_ZOOM_IN              140
#define IDS_TOOLTIP_ZOOM_OUT             141
#define IDS_TOOLTIP_SNAPSHOT             142
#define IDS_TOOLTIP_NARRATE              143
#define IDS_TOOLTIP_WEBCAM               144

// ============================================================================
// Block 10: Export format strings (IDs 145-160)
// ============================================================================
#define IDS_EXPORT_FORMAT_WMV_SD         145
#define IDS_EXPORT_FORMAT_WMV_HD         146
#define IDS_EXPORT_FORMAT_MP4_SD         147
#define IDS_EXPORT_FORMAT_MP4_HD         148
#define IDS_EXPORT_FORMAT_MP4_1080P      149
#define IDS_EXPORT_FORMAT_MP4_720P       150
#define IDS_EXPORT_FORMAT_MP4_480P       151
#define IDS_EXPORT_FORMAT_WMV_ONLY       152
#define IDS_EXPORT_FORMAT_MP4_ONLY       153
#define IDS_EXPORT_FORMAT_AUDIO_ONLY     154
#define IDS_EXPORT_FORMAT_CUSTOM         155
#define IDS_EXPORT_SETTING_QUALITY       156
#define IDS_EXPORT_SETTING_SIZE          157
#define IDS_EXPORT_SETTING_DURATION      158
#define IDS_EXPORT_SETTING_FILESIZE      159
#define IDS_EXPORT_SETTING_ESTIMATE      160

// ============================================================================
// Block 11: Timeline strings (IDs 161-176)
// ============================================================================
#define IDS_TIMELINE_TRACK_VIDEO         161
#define IDS_TIMELINE_TRACK_AUDIO         162
#define IDS_TIMELINE_TRACK_MUSIC         163
#define IDS_TIMELINE_TRACK_TITLE         164
#define IDS_TIMELINE_TRACK_TRANSITION    165
#define IDS_TIMELINE_TRACK_CREDITS       166
#define IDS_TIMELINE_DURATION            167
#define IDS_TIMELINE_POSITION            168
#define IDS_TIMELINE_SELECT_ALL          169
#define IDS_TIMELINE_DESELECT_ALL        170
#define IDS_TIMELINE_EXTENT_COUNT        171
#define IDS_TIMELINE_TOTAL_DURATION      172
#define IDS_TIMELINE_NO_ITEMS            173
#define IDS_TIMELINE_DRAG_HINT           174
#define IDS_TIMELINE_SPLIT_HINT          175
#define IDS_TIMELINE_TRIM_HINT           176

// ============================================================================
// Block 12: Media type strings (IDs 177-192)
// ============================================================================
#define IDS_MEDIA_TYPE_VIDEO             177
#define IDS_MEDIA_TYPE_AUDIO             178
#define IDS_MEDIA_TYPE_IMAGE             179
#define IDS_MEDIA_TYPE_UNKNOWN           180
#define IDS_MEDIA_DURATION               181
#define IDS_MEDIA_SIZE                   182
#define IDS_MEDIA_DATE                   183
#define IDS_MEDIA_DIMENSIONS             184
#define IDS_MEDIA_BITRATE                185
#define IDS_MEDIA_CODEC                  186
#define IDS_MEDIA_FPS                    187
#define IDS_MEDIA_SAMPLE_RATE            188
#define IDS_MEDIA_CHANNELS               189
#define IDS_MEDIA_BITS_PER_SAMPLE        190
#define IDS_MEDIA_PATH                   191
#define IDS_MEDIA_THUMBNAIL              192

// ============================================================================
// Block 13: Title/effect strings (IDs 193-208)
// ============================================================================
#define IDS_TITLE_ANIM_FADE              193
#define IDS_TITLE_ANIM_SLIDE_LEFT        194
#define IDS_TITLE_ANIM_SLIDE_RIGHT       195
#define IDS_TITLE_ANIM_SLIDE_UP          196
#define IDS_TITLE_ANIM_SLIDE_DOWN        197
#define IDS_TITLE_ANIM_ZOOM_IN           198
#define IDS_TITLE_ANIM_ZOOM_OUT          199
#define IDS_TITLE_ANIM_SPIN              200
#define IDS_TITLE_ANIM_BOUNCE            201
#define IDS_TITLE_ANIM_WIPE              202
#define IDS_TITLE_ANIM_STARBURST         203
#define IDS_TITLE_ANIM_CINEMATIC         204
#define IDS_TITLE_ANIM_SIMPLE            205
#define IDS_TITLE_ANIM_NONE              206
#define IDS_EFFECT_NONE                  207
#define IDS_EFFECT_AUTO_FIX              208

// ============================================================================
// Block 14: Transition strings (IDs 209-224)
// ============================================================================
#define IDS_TRANS_NONE                   209
#define IDS_TRANS_CROSSFADE              210
#define IDS_TRANS_DISSOLVE               211
#define IDS_TRANS_DISSOLVE_NOISE         212
#define IDS_TRANS_WIPE_LEFT              213
#define IDS_TRANS_WIPE_RIGHT             214
#define IDS_TRANS_WIPE_UP                215
#define IDS_TRANS_WIPE_DOWN              216
#define IDS_TRANS_PUSH_LEFT              217
#define IDS_TRANS_PUSH_RIGHT             218
#define IDS_TRANS_PUSH_UP                219
#define IDS_TRANS_PUSH_DOWN              220
#define IDS_TRANS_FADE                   221
#define IDS_TRANS_SHRINK                 222
#define IDS_TRANS_GROW                   223
#define IDS_TRANS_FLIP                   224

// ============================================================================
// Block 15: Capture strings (IDs 225-240)
// ============================================================================
#define IDS_CAPTURE_WEBCAM               225
#define IDS_CAPTURE_NARRATION            226
#define IDS_CAPTURE_START                227
#define IDS_CAPTURE_STOP                 228
#define IDS_CAPTURE_PAUSE                229
#define IDS_CAPTURE_RESUME               230
#define IDS_CAPTURE_NO_DEVICE            231
#define IDS_CAPTURE_DEVICE_ERROR         232
#define IDS_CAPTURE_IN_PROGRESS          233
#define IDS_CAPTURE_COMPLETE             234
#define IDS_CAPTURE_PREVIEW              235
#define IDS_CAPTURE_RECORDING            236
#define IDS_CAPTURE_MAX_DURATION         237
#define IDS_CAPTURE_DISK_FULL            238
#define IDS_CAPTURE_FORMAT_ERROR         239
#define IDS_CAPTURE_TIMEOUT              240

// ============================================================================
// Block 16: Template/theme strings (IDs 241-256)
// ============================================================================
#define IDS_THEME_DEFAULT                241
#define IDS_THEME_CONTRAST               242
#define IDS_THEME_DARK                   243
#define IDS_THEME_BRIGHT                 244
#define IDS_THEME_WARM                   245
#define IDS_THEME_COOL                   246
#define IDS_THEME_VINTAGE                247
#define IDS_THEME_MODERN                 248
#define IDS_THEME_CLASSIC                249
#define IDS_THEME_CINEMATIC              250
#define IDS_THEME_BIRTHDAY               251
#define IDS_THEME_HOLIDAY                252
#define IDS_THEME_TRAVEL                 253
#define IDS_THEME_SPORTS                 254
#define IDS_THEME_WEDDING                255
#define IDS_THEME_CUSTOM                 256

// ============================================================================
// Remaining blocks (IDs 257-5000)
// ============================================================================
// IDs 257-512: Additional UI text and formatting strings
#define IDS_FORMAT_DURATION_HMS          257
#define IDS_FORMAT_DURATION_MS           258
#define IDS_FORMAT_FILESIZE_KB           259
#define IDS_FORMAT_FILESIZE_MB           260
#define IDS_FORMAT_FILESIZE_GB           261
#define IDS_FORMAT_RESOLUTION            262
#define IDS_FORMAT_FRAMERATE             263
#define IDS_FORMAT_BITRATE               264
#define IDS_FORMAT_SAMPLE_RATE           265
#define IDS_FORMAT_CHANNELS              266
#define IDS_FORMAT_PERCENT               267
#define IDS_FORMAT_TIME_REMAINING        268
#define IDS_FORMAT_FRAME_COUNT           269
#define IDS_FORMAT_EXTENT_INDEX          270
#define IDS_FORMAT_MEDIA_INDEX           271
#define IDS_FORMAT_PROJECT_NAME          272
#define IDS_FORMAT_AUTHOR_NAME           273
#define IDS_FORMAT_DATE_TAKEN            274
#define IDS_FORMAT_FILE_DATE             275
#define IDS_FORMAT_VERSION               276

// IDs 277-512: Accessibility, help text, registry keys, COM strings
#define IDS_A11Y_TIMELINE                277
#define IDS_A11Y_PREVIEW                 278
#define IDS_A11Y_STORYBOARD              279
#define IDS_A11Y_MEDIA_BROWSER           280
#define IDS_A11Y_TOOLBAR                 281
#define IDS_A11Y_STATUS_BAR              282
#define IDS_A11Y_TITLE_EDITOR            283
#define IDS_A11Y_TRANSITION_PICKER       284
#define IDS_A11Y_EFFECT_PICKER           285
#define IDS_A11Y_CAPTURE                 286
#define IDS_A11Y_EXPORT                  287
#define IDS_A11Y_OPTIONS                 288
#define IDS_A11Y_ABOUT                   289

// IDs 290-512: Internal strings, COM class names, registry values
#define IDS_REG_APP_PATH                 290
#define IDS_REG_VERSION                  291
#define IDS_REG_INSTALL_DATE             292
#define IDS_REG_LAST_PROJECT             293
#define IDS_REG_AUTO_SAVE                294
#define IDS_REG_DEFAULT_THEME            295
#define IDS_REG_DEFAULT_TRANSITION       296
#define IDS_REG_WEBCAM_DEVICE            297
#define IDS_REG_MIC_DEVICE               298
#define IDS_REG_OUTPUT_PATH              299
#define IDS_REG_LAST_EXPORT              300

// IDs 301-512: Additional formatting, resource references
#define IDS_RES_APP_MANIFEST             301
#define IDS_RES_ICON                     302
#define IDS_RES_LOGO                     303
#define IDS_RES_SPLASH                   304

// ============================================================================
// IDs 513-1024: Additional UI strings, extended descriptions
// ============================================================================
// These correspond to additional string table entries found in the binary.
// Grouped by functional area.
#define IDS_HELP_TIMELINE_DRAG           513
#define IDS_HELP_TIMELINE_TRIM           514
#define IDS_HELP_TIMELINE_SPLIT          515
#define IDS_HELP_TITLE_EDIT              516
#define IDS_HELP_CREDITS_EDIT            517
#define IDS_HELP_EXPORT                  518
#define IDS_HELP_IMPORT                  519
#define IDS_HELP_CAPTURE                 520
#define IDS_HELP_EFFECTS                 521
#define IDS_HELP_TRANSITIONS             522
#define IDS_HELP_THEMES                  523
#define IDS_HELP_PROJECT                 524
#define IDS_HELP_UNDO                    525
#define IDS_HELP_REDO                    526
#define IDS_HELP_COPY_PASTE              527
#define IDS_HELP_DELETE                  528
#define IDS_HELP_SELECT_ALL              529
#define IDS_HELP_ZOOM                    530
#define IDS_HELP_PLAYBACK                531

// IDs 532-768: Extended error and diagnostic strings
#define IDS_DIAG_RENDER_INIT             532
#define IDS_DIAG_D3D_CREATE              533
#define IDS_DIAG_SWAP_CHAIN              534
#define IDS_DIAG_TEXTURE_ALLOC           535
#define IDS_DIAG_SHADER_COMPILE          536
#define IDS_DIAG_VERTEX_BUFFER           537
#define IDS_DIAG_INDEX_BUFFER            538
#define IDS_DIAG_CONSTANT_BUFFER         539
#define IDS_DIAG_RENDER_TARGET           540
#define IDS_DIAG_DEPTH_STENCIL           541
#define IDS_DIAG_RESOURCE_CREATE         542
#define IDS_DIAG_RESOURCE_RELEASE         543
#define IDS_DIAG_DEVICE_LOST             544
#define IDS_DIAG_DEVICE_RESET            545
#define IDS_DIAG_OUT_OF_VIDMEM           546
#define IDS_DIAG_OUT_OF_SYSMEM           547
#define IDS_DIAG_FILE_IO_ERROR           548
#define IDS_DIAG_NETWORK_ERROR           549
#define IDS_DIAG_CODEC_ERROR             550

// IDs 769-1024: Extended format, serialization, and metadata strings
#define IDS_META_TITLE                   769
#define IDS_META_AUTHOR                  770
#define IDS_META_DESCRIPTION             771
#define IDS_META_KEYWORDS                772
#define IDS_META_RATING                  773
#define IDS_META_DATE_CREATED            774
#define IDS_META_DATE_MODIFIED           775
#define IDS_META_SOFTWARE                776
#define IDS_META_SOURCE                  777
#define IDS_META_COPYRIGHT               778

// ============================================================================
// IDs 1025-2048: Project format strings
// ============================================================================
#define IDS_PROJECT_ELEMENT              1025
#define IDS_PROJECT_VERSION              1026
#define IDS_PROJECT_MEDIA                1027
#define IDS_PROJECT_TIMELINE             1028
#define IDS_PROJECT_TRANSITIONS          1029
#define IDS_PROJECT_EFFECTS              1030
#define IDS_PROJECT_TITLES               1031
#define IDS_PROJECT_SETTINGS             1032
#define IDS_PROJECT_ATTRIBUTES           1033
#define IDS_PROJECT_AUTO_SAVE             1034

// ============================================================================
// IDs 2049-3072: X3D scene and rendering strings
// ============================================================================
#define IDS_X3D_SCENE                    2049
#define IDS_X3D_NODE                     2050
#define IDS_X3D_SHAPE                    2051
#define IDS_X3D_TRANSFORM                2052
#define IDS_X3D_GROUP                    2053
#define IDS_X3D_SWITCH                   2054
#define IDS_X3D_VIEWPORT                 2055
#define IDS_X3D_LAYER                    2056
#define IDS_X3D_LAYERSET                 2057
#define IDS_X3D_TEXTURE                  2058
#define IDS_X3D_MATERIAL                 2059
#define IDS_X3D_APPEARANCE               2060
#define IDS_X3D_GEOMETRY                 2061
#define IDS_X3D_INDEXEDFACESET           2062
#define IDS_X3D_TEXT_NODE                2063
#define IDS_X3D_GRID                     2064
#define IDS_X3D_TIMESENSOR               2065
#define IDS_X3D_INTERPOLATOR             2066
#define IDS_X3D_SEQUENCER                2067
#define IDS_X3D_ROUTE                    2068
#define IDS_X3D_FIELD                    2069
#define IDS_X3D_SENSOR                   2070

// ============================================================================
// IDs 3073-4096: Shader and effect strings
// ============================================================================
#define IDS_SHADER_GRID                  3073
#define IDS_SHADER_FADE                  3074
#define IDS_SHADER_DISSOLVE              3075
#define IDS_SHADER_CHANNEL_MIXER         3076
#define IDS_SHADER_BRIGHTNESS            3077
#define IDS_SHADER_BLUR                  3078
#define IDS_SHADER_POSTERIZE             3079
#define IDS_SHADER_PIXELATE              3080
#define IDS_SHADER_RIPPLE                3081
#define IDS_SHADER_HUE                   3082
#define IDS_SHADER_EDGE_DETECT           3083
#define IDS_SHADER_TEXT                  3084
#define IDS_SHADER_SCROLLING_TEXT        3085
#define IDS_SHADER_WIPE                  3086

// ============================================================================
// IDs 4097-5000: Miscellaneous, telemetry, and internal strings
// ============================================================================
#define IDS_TELEM_SESSION_START          4097
#define IDS_TELEM_SESSION_END            4098
#define IDS_TELEM_EXPORT_START           4099
#define IDS_TELEM_EXPORT_END             4100
#define IDS_TELEM_IMPORT_START           4101
#define IDS_TELEM_IMPORT_END             4102
#define IDS_TELEM_CAPTURE_START          4103
#define IDS_TELEM_CAPTURE_END            4104
#define IDS_TELEM_PROJECT_OPEN           4105
#define IDS_TELEM_PROJECT_SAVE           4106
#define IDS_TELEM_FEATURE_USED           4107
#define IDS_TELEM_ERROR_OCCURRED         4108
#define IDS_TELEM_PERFORMANCE            4109

#define IDS_INTERNAL_MAX                 5000

#endif // STRINGTABLEIDS_H
