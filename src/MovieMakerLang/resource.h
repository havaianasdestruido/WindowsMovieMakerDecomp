#ifndef MOVIEMAKERLANG_RESOURCE_H
#define MOVIEMAKERLANG_RESOURCE_H

// Version & Manifest
#define IDR_VERSION                     1
#define IDR_MANIFEST                    2

// Icons
#define IDI_MAIN_ICON                  100
#define IDI_SMALL_ICON                 101

// Cursors
#define IDC_APPSTARTING                200
#define IDC_WAIT                       201

// Bitmaps / Images
#define IDB_SPLASH                     300
#define IDB_TOOLBAR                    301
#define IDB_TOOLBAR_DARK               302
#define IDB_WELCOME_BANNER             303

// UIFILE (DirectUI layouts)
#define RT_UIFILE_RIBBON               10000
#define RT_UIFILE_MAIN                 10001
#define RT_UIFILE_DIALOGS              10002

// Dialog templates
#define IDD_OPTIONS                    500
#define IDD_PUBLISH                    501
#define IDD_ABOUT                      502
#define IDD_CAPTURE                    503
#define IDD_EFFECTS                    504
#define IDD_SAVEAS                     505
#define IDD_NEWPROJECT                 506
#define IDD_PROGRESS                   507
#define IDD_ERROR                      508
#define IDD_WARNING                    509

// Menu resources
#define IDM_MAIN                       600
#define IDM_PROJECT                    601
#define IDM_CONTEXT                    602

// Accelerator tables
#define IDA_MAIN                       700
#define IDA_RIBBON                     701

// ============================================================================
// String table IDs (range 1000-1999)
// ============================================================================

// Application & window titles
#define IDS_APP_TITLE                 1000
#define IDS_UNTITLED_PROJECT          1001
#define IDS_WINDOW_TITLE              1002

// Main menu items
#define IDS_FILE_MENU                 1003
#define IDS_EDIT_MENU                 1004
#define IDS_VIEW_MENU                 1005
#define IDS_TOOLS_MENU                1006
#define IDS_HELP_MENU                 1007
#define IDS_NEW_PROJECT               1008
#define IDS_OPEN_PROJECT              1009
#define IDS_SAVE_PROJECT              1010
#define IDS_SAVE_PROJECT_AS           1011
#define IDS_UNDO                      1012
#define IDS_REDO                      1013
#define IDS_CUT                       1014
#define IDS_COPY                      1015
#define IDS_PASTE                     1016
#define IDS_SELECT_ALL                1017
#define IDS_OPTIONS                   1018
#define IDS_ABOUT                     1019

// Toolbar tooltips
#define IDS_TIP_ADD_VIDEOS            1020
#define IDS_TIP_ADD_MUSIC             1021
#define IDS_TIP_ADD_NARRATION         1022
#define IDS_TIP_TRIM                  1023
#define IDS_TIP_PUBLISH               1024
#define IDS_TIP_EFFECTS               1025
#define IDS_TIP_STABILIZE             1026
#define IDS_TIP_PREVIEW_FULL          1027
#define IDS_TIP_MENU_BAR              1028
#define IDS_GENERAL_TAB               2000
#define IDS_VERSION_STRING            2001
#define IDS_COPYRIGHT                 2002
#define IDS_STATUS_DETAILS            2003
#define IDS_STATUS_PROGRESS           2004
#define IDS_STATUS_CANCEL             2005
#define IDS_HOME_TAB                  2006
#define IDS_ANIMATIONS_TAB            2007
#define IDS_VISUAL_EFFECTS_TAB        2008
#define IDS_PROJECT_TAB               2009
#define IDS_EXIT                      2010

// Dialog button text
#define IDS_OK                        1030
#define IDS_CANCEL                    1031
#define IDS_SAVE                      1032
#define IDS_DONT_SHOW_AGAIN           1033
#define IDS_ABORT                     1034
#define IDS_RETRY                     1035
#define IDS_IGNORE                    1036
#define IDS_YES                       1037
#define IDS_YES_ALL                   1038
#define IDS_NO                        1039
#define IDS_NO_ALL                    1040
#define IDS_CLOSE                     1041
#define IDS_EXIT                      2010
#define IDS_TRY_AGAIN                 1043
#define IDS_CONTINUE                  1044
#define IDS_APPLY                     1045
#define IDS_DOWNLOAD                  1046
#define IDS_REVERT                    1047
#define IDS_RESTORE_PROJECT           1048
#define IDS_FIX_ITEM                  1049

// Transition / effect names
#define IDS_FX_FADE                   1050
#define IDS_FX_DISSOLVE               1051
#define IDS_FX_WIPE                   1052
#define IDS_FX_SLIDE                  1053
#define IDS_FX_ZOOM                   1054
#define IDS_FX_CIRCLE                 1055
#define IDS_FX_DIAMOND                1056
#define IDS_FX_HEART                  1057
#define IDS_FX_STAR                   1058
#define IDS_FX_WHEEL                  1059
#define IDS_FX_PAGE_CURL              1060
#define IDS_FX_IRIS                   1061
#define IDS_FX_CROSSFADE              1062
#define IDS_FX_CHECKERBOARD           1063
#define IDS_FX_FLIP                   1064
#define IDS_FX_ROLL                   1065
#define IDS_FX_SWING                  1066
#define IDS_FX_WARP                   1067
#define IDS_FX_BLUR                   1068
#define IDS_FX_SPLIT                  1069
#define IDS_FX_CINEMATIC              1070
#define IDS_FX_PAN_LEFT               1071
#define IDS_FX_PAN_RIGHT              1072
#define IDS_FX_PAN_UP                 1073
#define IDS_FX_PAN_DOWN               1074
#define IDS_FX_ZOOM_IN_CENTER         1075
#define IDS_FX_ZOOM_OUT_CENTER        1076
#define IDS_FX_CONTEMPORARY_DROP      1077
#define IDS_FX_CONTEMPORARY_FADE      1078
#define IDS_FX_CONTEMPORARY_FLY       1079
#define IDS_FX_CONTEMPORARY_SLIDE     1080

// Publishing targets
#define IDS_PUBLISH_ONEDRIVE          1081
#define IDS_PUBLISH_FACEBOOK          1082
#define IDS_PUBLISH_YOUTUBE           1083
#define IDS_PUBLISH_FLICKR            1084
#define IDS_PUBLISH_VIMEO             1085
#define IDS_PUBLISH_BLIPTV            1086
#define IDS_PUBLISH_MYSPACE           1087
#define IDS_PUBLISH_DAILYMOTION       1088
#define IDS_PUBLISH_DVD               1089
#define IDS_PUBLISH_WATCH_ONLINE      1090
#define IDS_PUBLISH_EMAIL             1091

// Device profiles
#define IDS_DEV_WP_LARGE              1092
#define IDS_DEV_WP_SMALL              1093
#define IDS_DEV_IPHONE                1094
#define IDS_DEV_IPAD                  1095
#define IDS_DEV_ANDROID_MED           1096
#define IDS_DEV_ANDROID_LARGE         1097
#define IDS_DEV_ANDROID_1080P         1098
#define IDS_DEV_ANDROID_720P          1099
#define IDS_DEV_ZUNE_HD_720P          1100
#define IDS_DEV_ZUNE_HD_DEVICE        1101
#define IDS_DEV_FEATURE_SMALL         1102
#define IDS_DEV_FEATURE_LARGE         1103
#define IDS_DEV_AUDIO_ONLY            1104
#define IDS_DEV_WIN7_720P             1105
#define IDS_DEV_WIN8_1080P            1106

// Credit templates
#define IDS_CREDIT_DIRECTED_BY        1107
#define IDS_CREDIT_NAME               1108
#define IDS_CREDIT_STARRING           1109
#define IDS_CREDIT_CAST               1110
#define IDS_CREDIT_LOCATION           1111
#define IDS_CREDIT_LOC_TEXT           1112
#define IDS_CREDIT_SOUNDTRACK         1113
#define IDS_CREDIT_SOUND_TEXT         1114
#define IDS_CREDIT_PRODUCED_BY        1115
#define IDS_CREDIT_DIRECTOR           1116

// Generic UI labels
#define IDS_LOADING                   1117
#define IDS_LOADING_AUDIO             1118
#define IDS_BROWSE                    1119
#define IDS_APPLY_TO_ALL              1120
#define IDS_FIT_TO_MUSIC              1121
#define IDS_FIT_PHOTOS                1122
#define IDS_SAVE_CLOSE                1123
#define IDS_CLOSE_NO_SAVE             1124
#define IDS_NO_THANKS                 1125
#define IDS_DELETE                    1126
#define IDS_REMOVE                    1127
#define IDS_ADD_VIDEOS                1128
#define IDS_ADD_MUSIC                 1129
#define IDS_ADD_SOUND                 1130
#define IDS_ADD_TITLE                 1131
#define IDS_ADD_CREDITS               1132
#define IDS_ADD_EFFECTS               1133
#define IDS_PUBLISH                   1134

// Ribbon / tab labels
#define IDS_HOME_TAB                  1135
#define IDS_ANIMATIONS_TAB            1136
#define IDS_VISUAL_EFFECTS_TAB        1137
#define IDS_PROJECT_TAB               1138
#define IDS_VIEW_TAB                  1139
#define IDS_FORMAT_TAB                1140
#define IDS_EDIT_TAB                  1141
#define IDS_VIDEO_TOOLS               1142
#define IDS_MUSIC_TOOLS               1143
#define IDS_NARRATION_TOOLS           1144
#define IDS_TEXT_TOOLS                1145
#define IDS_CAPTIONS_TOOLS            1146
#define IDS_AUDIO_VIDEO               1147
#define IDS_AUDIO_MUSIC               1148
#define IDS_GENERAL_TAB               1149
#define IDS_ADVANCED_TAB              1150
#define IDS_CAPTIONS_TAB              1151
#define IDS_TRUST_CENTER_TAB           1152
#define IDS_AUDIO_VIDEO_TAB           1153

// Error messages
#define IDS_ERR_CRASH                 1154
#define IDS_ERR_CMDLINE               1155
#define IDS_ERR_FILE_TYPE             1156
#define IDS_ERR_OPEN_PROJECT          1157
#define IDS_ERR_FILE_NOT_FOUND        1158
#define IDS_ERR_CODEC                 1159
#define IDS_ERR_NETWORK_FOLDER        1160
#define IDS_ERR_REMOVABLE_DRIVE       1161
#define IDS_ERR_DRM                   1162
#define IDS_ERR_INTERNET              1163
#define IDS_ERR_AUDIO_CODEC           1164
#define IDS_ERR_DIRECTSHOW_CODEC      1165
#define IDS_ERR_OLD_CODEC             1166
#define IDS_ERR_MPEG2_CODEC           1167
#define IDS_ERR_CANT_START            1168
#define IDS_ERR_PUBLISH_PLUGIN        1169
#define IDS_ERR_FILE_TOO_LARGE        1170
#define IDS_ERR_PUBLISH_FAILED        1171
#define IDS_ERR_SERVICE_UNAVAIL       1172
#define IDS_ERR_NO_PERMISSION         1173
#define IDS_ERR_SAVE_PERMISSION       1174
#define IDS_ERR_CONTACT_ADMIN         1175
#define IDS_ERR_MISSING_FILES         1176
#define IDS_ERR_TIMEOUT               1177
#define IDS_ERR_DISK_FULL             1178
#define IDS_ERR_OUT_OF_MEMORY         1179
#define IDS_ERR_INVALID_FORMAT        1180
#define IDS_ERR_UNKNOWN               1181

// Status bar strings
#define IDS_STATUS_READY              1182
#define IDS_STATUS_RENDERING          1183
#define IDS_STATUS_PUBLISHING         1184
#define IDS_STATUS_SAVING             1185
#define IDS_STATUS_ENCODING           1186
#define IDS_STATUS_CANCEL             1187
#define IDS_STATUS_PROGRESS           1188
#define IDS_STATUS_DETAILS            1189
#define IDS_STATUS_DURATION           1190
#define IDS_STATUS_EST_SIZE           1191
#define IDS_STATUS_FREE_SPACE         1192

// File type descriptions
#define IDS_PROJECT_DESC              1193
#define IDS_SETTINGS_DESC             1194
#define IDS_FILE_WMA                  1195
#define IDS_FILE_WMV                  1196
#define IDS_FILE_MP4_AUDIO            1197
#define IDS_FILE_MP4_VIDEO            1198
#define IDS_FILE_ALL                  1199

// Webcam / capture
#define IDS_CAP_DEVICES               1200
#define IDS_CAP_WEBCAMS               1201
#define IDS_CAP_AUDIO_DEVICES         1202
#define IDS_CAP_PREVIEW               1203
#define IDS_CAP_RECORDING             1204
#define IDS_CAP_START_VIDEO           1205
#define IDS_CAP_STOP_VIDEO            1206
#define IDS_CAP_START_NARRATION       1207
#define IDS_CAP_STOP_NARRATION        1208
#define IDS_CAP_NO_WEBCAM             1209
#define IDS_CAP_NO_AUDIO              1210
#define IDS_CAP_MAX_TIME              1211
#define IDS_CAP_MAX_RES               1212

// Publish / save settings
#define IDS_PUB_RECOMMENDED           1213
#define IDS_PUB_HD_DISPLAY            1214
#define IDS_PUB_COMPUTER              1215
#define IDS_PUB_CUSTOM                1216
#define IDS_SAVEAS_PROJECT            1217

// Miscellaneous strings
#define IDS_VERSION_STRING            1218
#define IDS_COPYRIGHT                 1219
#define IDS_JPEG_ACK                  1220
#define IDS_TELEMETRY                 1221
#define IDS_FONT                      1222
#define IDS_HELP_URL                  1223
#define IDS_WL_ID                     1224
#define IDS_ASPECT_WIDE               1225
#define IDS_ASPECT_STD                1226
#define IDS_OPTIONS_TITLE             1227
#define IDS_ABOUT_TITLE               1228

// UI automation / accessibility
#define IDS_AUTO_TIMELINE             1229
#define IDS_AUTO_STORYBOARD           1230
#define IDS_AUTO_PREVIEW              1231
#define IDS_AUTO_TEXT_BOX             1232
#define IDS_AUTO_TEXT_TRACK           1233
#define IDS_AUTO_MUSIC_TRACK          1234
#define IDS_AUTO_VISUALS_TRACK        1235
#define IDS_AUTO_AUDIO_TRACK          1236
#define IDS_AUTO_CAPTIONS             1237
#define IDS_AUTO_PLAYBACK             1238
#define IDS_AUTO_SCRUB                1239
#define IDS_AUTO_WAVEFORM             1240
#define IDS_AUTO_TIME_ZOOM            1241
#define IDS_AUTO_TRIM_MARKERS         1242
#define IDS_AUTO_TRIM_TOOL            1243

// Dialogs / prompts
#define IDS_DLG_NEW_PROJECT           1244
#define IDS_DLG_SAVE_PROJECT          1245
#define IDS_DLG_EXIT_CONFIRM          1246
#define IDS_DLG_DISCARD_CHANGES       1247
#define IDS_DLG_OVERWRITE_FILE        1248

#endif // MOVIEMAKERLANG_RESOURCE_H
