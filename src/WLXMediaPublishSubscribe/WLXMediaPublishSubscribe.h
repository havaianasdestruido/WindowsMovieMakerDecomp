/*
 * WLXMediaPublishSubscribe.h
 *
 * Public header for WLXMediaPublishSubscribe.dll.
 * Provides the plugin-based media publishing and subscription system for
 * Windows Live Movie Maker 2012.
 *
 * Supports publishing to:
 *   - Facebook (video upload via Graph API)
 *   - Flickr (photo/video upload via Upload API)
 *   - YouTube (video upload via Data API v3)
 *   - Vimeo (video upload via Simple API)
 *   - SkyDrive/OneDrive (via wlidcli.dll)
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once
#ifndef WLXMEDIAPUBLISHSUBSCRIBE_H
#define WLXMEDIAPUBLISHSUBSCRIBE_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#define WINVER        0x0602
#define _WIN32_WINNT  0x0602

#include <windows.h>
#include <objbase.h>
#include <gdiplus.h>

#ifdef WLXMEDIAPUBLISHSUBSCRIBE_EXPORTS
    #define WLXMPS_API __declspec(dllexport)
#else
    #define WLXMPS_API __declspec(dllimport)
#endif

// ============================================================================
// Forward declarations
// ============================================================================
namespace MediaPublish
{
    class PublishManager;
    class PublishPlugin;
    class FacebookPlugin;
    class FlickrPlugin;
    class YouTubePlugin;
    class VimeoPlugin;
    class SkyDrivePlugin;
    class AuthManager;
    class UploadSession;
    struct PublishResult;
    struct PublishConfig;
}

// ============================================================================
// Publishing status
// ============================================================================
enum PublishStatus
{
    PublishStatus_Idle         = 0,
    PublishStatus_Authenticating = 1,
    PublishStatus_Uploading    = 2,
    PublishStatus_Processing   = 3,
    PublishStatus_Complete     = 4,
    PublishStatus_Error        = 5,
    PublishStatus_Cancelled    = 6,
};

// ============================================================================
// Target service identifier
// ============================================================================
enum PublishTarget
{
    PublishTarget_Facebook  = 0,
    PublishTarget_Flickr    = 1,
    PublishTarget_YouTube   = 2,
    PublishTarget_Vimeo     = 3,
    PublishTarget_SkyDrive  = 4,
};

// ============================================================================
// Publish configuration
// ============================================================================
struct PublishConfig
{
    PublishTarget   target;
    WCHAR           wszTitle[512];
    WCHAR           wszDescription[2048];
    WCHAR           wszTags[1024];
    WCHAR           wszCategory[256];
    BOOL            bPrivate;
    BOOL            bAllowEmbed;
    UINT32          uPrivacyLevel;      // service-specific

    PublishConfig()
        : target(PublishTarget_Facebook)
        , bPrivate(FALSE)
        , bAllowEmbed(TRUE)
        , uPrivacyLevel(0)
    {
        ZeroMemory(wszTitle, sizeof(wszTitle));
        ZeroMemory(wszDescription, sizeof(wszDescription));
        ZeroMemory(wszTags, sizeof(wszTags));
        ZeroMemory(wszCategory, sizeof(wszCategory));
    }
};

// ============================================================================
// Publish result
// ============================================================================
struct PublishResult
{
    HRESULT     hrResult;
    WCHAR       wszPublishUrl[2048];
    WCHAR       wszError[1024];
    UINT64      ullItemId;

    PublishResult()
        : hrResult(S_OK)
        , ullItemId(0)
    {
        ZeroMemory(wszPublishUrl, sizeof(wszPublishUrl));
        ZeroMemory(wszError, sizeof(wszError));
    }
};

// ============================================================================
// Progress callback
// ============================================================================
typedef void (CALLBACK* PFN_PUBLISH_PROGRESS)(void* pUserData, UINT uPercent, PublishStatus eStatus);
typedef void (CALLBACK* PFN_PUBLISH_COMPLETE)(void* pUserData, const PublishResult* pResult);

// ============================================================================
// Exported functions (22 exports)
// ============================================================================
extern "C"
{
    // Plugin management
    WLXMPS_API HANDLE __stdcall PublishManager_Create();
    WLXMPS_API void   __stdcall PublishManager_Destroy(HANDLE hManager);
    WLXMPS_API HRESULT __stdcall PublishManager_EnumerateTargets(HANDLE hManager, PublishTarget* pTargets, UINT* pCount);
    WLXMPS_API HRESULT __stdcall PublishManager_GetTargetName(PublishTarget target, WCHAR* pName, UINT cchName);

    // Authentication
    WLXMPS_API HRESULT __stdcall PublishManager_Authenticate(HANDLE hManager, PublishTarget target, HWND hParentWnd);
    WLXMPS_API HRESULT __stdcall PublishManager_IsAuthenticated(HANDLE hManager, PublishTarget target, BOOL* pAuth);
    WLXMPS_API HRESULT __stdcall PublishManager_SignOut(HANDLE hManager, PublishTarget target);

    // Publishing
    WLXMPS_API HANDLE __stdcall PublishManager_StartPublish(HANDLE hManager, PublishTarget target,
        LPCWSTR pszFilePath, const PublishConfig* pConfig);
    WLXMPS_API HRESULT __stdcall PublishManager_GetStatus(HANDLE hPublish, UINT* pStatus, UINT* pPercent);
    WLXMPS_API HRESULT __stdcall PublishManager_Cancel(HANDLE hPublish);
    WLXMPS_API HRESULT __stdcall PublishManager_GetResult(HANDLE hPublish, PublishResult* pResult);

    // Callback registration
    WLXMPS_API HRESULT __stdcall PublishManager_SetProgressCallback(HANDLE hPublish,
        PFN_PUBLISH_PROGRESS pfnProgress, void* pUserData);
    WLXMPS_API HRESULT __stdcall PublishManager_SetCompleteCallback(HANDLE hPublish,
        PFN_PUBLISH_COMPLETE pfnComplete, void* pUserData);

    // Subscription (download from services)
    WLXMPS_API HANDLE __stdcall PublishManager_StartSubscribe(HANDLE hManager, PublishTarget target,
        LPCWSTR pszItemId);
    WLXMPS_API HRESULT __stdcall PublishManager_GetSubscribeStatus(HANDLE hSubscribe, UINT* pStatus, UINT* pPercent);

    // Account management
    WLXMPS_API HRESULT __stdcall PublishManager_GetAccountInfo(HANDLE hManager, PublishTarget target,
        WCHAR* pDisplayName, UINT cchDisplayName);
    WLXMPS_API HRESULT __stdcall PublishManager_RefreshToken(HANDLE hManager, PublishTarget target);

    // Settings
    WLXMPS_API HRESULT __stdcall PublishManager_SetDefaultTarget(HANDLE hManager, PublishTarget target);
    WLXMPS_API HRESULT __stdcall PublishManager_GetDefaultTarget(HANDLE hManager, PublishTarget* pTarget);

    // Utility
    WLXMPS_API HRESULT __stdcall PublishManager_GetServiceStatus(PublishTarget target, BOOL* pAvailable);
    WLXMPS_API HRESULT __stdcall PublishManager_Cleanup();
}

#endif // WLXMEDIAPUBLISHSUBSCRIBE_H
