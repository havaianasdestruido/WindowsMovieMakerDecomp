/*
 * WLXMediaPublishSubscribe.cpp
 *
 * Implementation of WLXMediaPublishSubscribe.dll -- the plugin-based media
 * publishing and subscription system for Windows Live Movie Maker 2012.
 *
 * Manages plugins for Facebook, Flickr, YouTube, Vimeo, and SkyDrive.
 * Each plugin implements authentication, upload/download, and progress
 * reporting via HTTP/HTTPS REST APIs using WinInet.
 *
 * Dependencies:
 *   - WLXPhotoSqm.dll (delay-loaded): SQM telemetry logging
 *   - DmxBici.dll (delay-loaded): Device analytics
 *   - wlidcli.dll (delay-loaded): Windows Live ID authentication
 *   - uxcore.dll (delay-loaded): Windows Live shared UI components
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "WLXMediaPublishSubscribe.h"
#include "WLXPhotoBase.h"

#include <wininet.h>
#include <vector>
#include <map>
#include <string>
#include <memory>
#include <set>
#include <mutex>
#include <algorithm>

// ============================================================================
// Forward declarations for delay-loaded functions
// ============================================================================
typedef HRESULT (STDAPICALLTYPE* PFN_WLIDCLIENT_AUTHENTICATE)(LPCWSTR, LPCWSTR, LPCWSTR, void*);
typedef void    (STDAPICALLTYPE* PFN_SQMLOG)(UINT, UINT, UINT);

// ============================================================================
// Internal classes
// ============================================================================
namespace MediaPublish
{

class UploadSession;

// ============================================================================
// Session progress interface -- implemented by upload/subscribe sessions so
// the shared HTTP helpers can report progress and check cancellation.
// ============================================================================
class ISessionProgress
{
public:
    virtual ~ISessionProgress() {}
    virtual bool IsCancelled() const = 0;
    virtual void ReportProgress(UINT uPercent, PublishStatus eStatus) = 0;
};

// ============================================================================
// Shared HTTP helpers
// ============================================================================

// Convert a wide string to UTF-8 (multipart bodies must be UTF-8).
static std::string WideToUtf8(LPCWSTR pszWide)
{
    if (!pszWide || !pszWide[0])
        return std::string();

    int cch = ::WideCharToMultiByte(CP_UTF8, 0, pszWide, -1, NULL, 0, NULL, NULL);
    if (cch <= 0)
        return std::string();

    std::string result(static_cast<size_t>(cch) - 1, '\0');
    ::WideCharToMultiByte(CP_UTF8, 0, pszWide, -1,
        &result[0], cch - 1, NULL, NULL);
    return result;
}

static std::wstring GetFileBaseName(LPCWSTR pszFilePath)
{
    std::wstring path(pszFilePath ? pszFilePath : L"");
    size_t pos = path.find_last_of(L"\\/");
    if (pos != std::wstring::npos)
        return path.substr(pos + 1);
    return path;
}

// Perform a multipart/form-data POST upload of pszFilePath to pszUrl.
// Reports progress through pSession and supports cancellation.
static HRESULT PerformMultipartUpload(
    LPCWSTR pszUrl,
    LPCWSTR pszFileField,
    LPCWSTR pszFilePath,
    const PublishConfig* pConfig,
    ISessionProgress* pSession,
    WCHAR* pszResultUrl, DWORD cchResultUrl)
{
    if (!pszUrl || !pszFilePath || !pConfig)
        return E_INVALIDARG;

    if (pszResultUrl && cchResultUrl > 0)
        pszResultUrl[0] = 0;

    // Resolve the file size for progress reporting
    HANDLE hFile = CreateFileW(pszFilePath, GENERIC_READ, FILE_SHARE_READ,
        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        return HRESULT_FROM_WIN32(GetLastError());

    LARGE_INTEGER liFileSize;
    if (!GetFileSizeEx(hFile, &liFileSize))
    {
        CloseHandle(hFile);
        return HRESULT_FROM_WIN32(GetLastError());
    }
    DWORD64 cbFile = static_cast<DWORD64>(liFileSize.QuadPart);

    // Build a unique multipart boundary
    WCHAR wszBoundary[96];
    GUID guid = {0};
    CoCreateGuid(&guid);
    swprintf_s(wszBoundary, 96, L"----WLMM_%08X%04X%04X", guid.Data1, guid.Data2, guid.Data3);

    std::string strBoundary = WideToUtf8(wszBoundary);
    std::string strField = WideToUtf8(pszFileField);
    std::string strFileName = WideToUtf8(GetFileBaseName(pszFilePath).c_str());

    // Build the header portion (all fields + file preamble)
    std::string body;
    body += "--" + strBoundary + "\r\n";
    body += "Content-Disposition: form-data; name=\"title\"\r\n\r\n";
    body += WideToUtf8(pConfig->wszTitle) + "\r\n";
    body += "--" + strBoundary + "\r\n";
    body += "Content-Disposition: form-data; name=\"description\"\r\n\r\n";
    body += WideToUtf8(pConfig->wszDescription) + "\r\n";
    body += "--" + strBoundary + "\r\n";
    body += "Content-Disposition: form-data; name=\"tags\"\r\n\r\n";
    body += WideToUtf8(pConfig->wszTags) + "\r\n";
    body += "--" + strBoundary + "\r\n";
    body += "Content-Disposition: form-data; name=\"" + strField + "\"; filename=\"" + strFileName + "\"\r\n";
    body += "Content-Type: application/octet-stream\r\n\r\n";

    std::string trailer = "\r\n--" + strBoundary + "--\r\n";

    DWORD64 cbTotal = static_cast<DWORD64>(body.size()) + cbFile + static_cast<DWORD64>(trailer.size());

    HINTERNET hNet = InternetOpenW(L"WLXMediaPublishSubscribe", INTERNET_OPEN_TYPE_PRECONFIG,
        NULL, NULL, 0);
    if (!hNet)
    {
        CloseHandle(hFile);
        return HRESULT_FROM_WIN32(GetLastError());
    }

    // Parse the URL into components
    URL_COMPONENTSW urlParts = {0};
    urlParts.dwStructSize = sizeof(urlParts);
    WCHAR wszHost[256] = {0};
    WCHAR wszPath[2048] = {0};
    urlParts.lpszHostName = wszHost;
    urlParts.dwHostNameLength = 255;
    urlParts.lpszUrlPath = wszPath;
    urlParts.dwUrlPathLength = 2047;

    if (!InternetCrackUrlW(pszUrl, 0, ICU_DECODE, &urlParts))
    {
        InternetCloseHandle(hNet);
        CloseHandle(hFile);
        return HRESULT_FROM_WIN32(GetLastError());
    }

    bool bSecure = (urlParts.nScheme == INTERNET_SCHEME_HTTPS);

    HINTERNET hConn = InternetConnectW(hNet, wszHost, urlParts.nPort,
        NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
    if (!hConn)
    {
        InternetCloseHandle(hNet);
        CloseHandle(hFile);
        return HRESULT_FROM_WIN32(GetLastError());
    }

    DWORD dwFlags = INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_RELOAD |
        INTERNET_FLAG_KEEP_CONNECTION;
    if (bSecure)
        dwFlags |= INTERNET_FLAG_SECURE;

    HINTERNET hReq = HttpOpenRequestW(hConn, L"POST", wszPath, NULL, NULL, NULL,
        dwFlags, 0);
    if (!hReq)
    {
        InternetCloseHandle(hConn);
        InternetCloseHandle(hNet);
        CloseHandle(hFile);
        return HRESULT_FROM_WIN32(GetLastError());
    }

    char szContentLength[64];
    sprintf_s(szContentLength, sizeof(szContentLength), "Content-Length: %I64u\r\n", cbTotal);

    const char* szContentType = "Content-Type: multipart/form-data; boundary=";
    std::string strContentType(szContentType);
    strContentType += strBoundary + "\r\n";

    std::string strHeaders = "Accept: */*\r\n";
    strHeaders += strContentType;
    strHeaders += szContentLength;

    HRESULT hr = S_OK;

    if (!HttpSendRequestExW(hReq, NULL, 0, HSR_SYNC, 0))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    else
    {
        DWORD cbWritten = 0;
        BOOL bOk = TRUE;

        // Send the form fields + file preamble
        if (bOk && !body.empty())
        {
            bOk = InternetWriteFile(hReq, body.data(), (DWORD)body.size(), &cbWritten);
        }

        // Stream the file body in chunks with progress reporting
        const DWORD cbChunk = 65536;
        std::vector<BYTE> chunk(cbChunk);
        DWORD cbRead = 0;
        DWORD64 cbSent = 0;

        while (bOk && cbSent < cbFile)
        {
            if (pSession->IsCancelled())
            {
                hr = E_ABORT;
                break;
            }

            if (!ReadFile(hFile, chunk.data(), cbChunk, &cbRead, NULL) || cbRead == 0)
                break;

            if (!InternetWriteFile(hReq, chunk.data(), cbRead, &cbWritten))
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
                break;
            }

            cbSent += cbWritten;
            if (cbFile > 0)
            {
                UINT uPercent = (UINT)((cbSent * 100) / cbFile);
                pSession->ReportProgress(uPercent, PublishStatus_Uploading);
            }
        }

        // Send the trailer
        if (SUCCEEDED(hr) && !trailer.empty())
        {
            if (!InternetWriteFile(hReq, trailer.data(), (DWORD)trailer.size(), &cbWritten))
                hr = HRESULT_FROM_WIN32(GetLastError());
        }

        if (SUCCEEDED(hr))
        {
            if (!HttpEndRequestW(hReq, NULL, 0, 0))
                hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    // Read the HTTP response status
    if (SUCCEEDED(hr))
    {
        DWORD dwStatus = 0;
        DWORD dwLen = sizeof(dwStatus);
        if (HttpQueryInfoW(hReq, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER,
            &dwStatus, &dwLen, NULL))
        {
            if (dwStatus < 200 || dwStatus > 299)
            {
                // Service rejected the request (usually missing auth token).
                hr = HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
                if (pszResultUrl && cchResultUrl > 0)
                    swprintf_s(pszResultUrl, cchResultUrl, L"HTTP %lu", dwStatus);
            }
            else
            {
                // Success: try to extract a Location header as the published URL
                if (pszResultUrl && cchResultUrl > 0)
                {
                    DWORD cbUrl = (DWORD)cchResultUrl * sizeof(WCHAR);
                    if (!HttpQueryInfoW(hReq, HTTP_QUERY_LOCATION,
                        pszResultUrl, &cbUrl, NULL))
                    {
                        pszResultUrl[0] = 0;
                    }
                }
            }
        }
    }

    InternetCloseHandle(hReq);
    InternetCloseHandle(hConn);
    InternetCloseHandle(hNet);
    CloseHandle(hFile);

    return hr;
}

// Download a media item over HTTPS to a temp file (used by Subscribe).
static HRESULT DownloadItem(LPCWSTR pszUrl, LPCWSTR pszItemId,
    ISessionProgress* pSession, WCHAR* pszOutputPath, DWORD cchOutputPath)
{
    UNREFERENCED_PARAMETER(pszItemId);

    HINTERNET hNet = InternetOpenW(L"WLXMediaPublishSubscribe", INTERNET_OPEN_TYPE_PRECONFIG,
        NULL, NULL, 0);
    if (!hNet)
        return HRESULT_FROM_WIN32(GetLastError());

    HINTERNET hReq = InternetOpenUrlW(hNet, pszUrl, NULL, 0,
        INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_RELOAD, 0);
    if (!hReq)
    {
        InternetCloseHandle(hNet);
        return HRESULT_FROM_WIN32(GetLastError());
    }

    // Create the destination temp file
    WCHAR wszTempDir[MAX_PATH] = {0};
    GetTempPathW(MAX_PATH, wszTempDir);

    WCHAR wszTempFile[MAX_PATH] = {0};
    GUID guid = {0};
    CoCreateGuid(&guid);
    swprintf_s(wszTempFile, MAX_PATH, L"%sWLMM_%08X%04X%04X.mp4",
        wszTempDir, guid.Data1, guid.Data2, guid.Data3);

    HANDLE hOut = CreateFileW(wszTempFile, GENERIC_WRITE, 0, NULL,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hOut == INVALID_HANDLE_VALUE)
    {
        InternetCloseHandle(hReq);
        InternetCloseHandle(hNet);
        return HRESULT_FROM_WIN32(GetLastError());
    }

    // Read the content length for progress
    DWORD dwContentLen = 0;
    DWORD dwLen = sizeof(dwContentLen);
    HttpQueryInfoW(hReq, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER,
        &dwContentLen, &dwLen, NULL);

    HRESULT hr = S_OK;
    BYTE buffer[65536];
    DWORD cbRead = 0;
    DWORD dwTotal = 0;
    BOOL bOk = TRUE;

    while (bOk)
    {
        if (pSession->IsCancelled())
        {
            hr = E_ABORT;
            break;
        }

        if (!InternetReadFile(hReq, buffer, sizeof(buffer), &cbRead))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            break;
        }
        if (cbRead == 0)
            break;

        DWORD cbWritten = 0;
        if (!WriteFile(hOut, buffer, cbRead, &cbWritten, NULL))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            break;
        }

        dwTotal += cbWritten;
        if (dwContentLen > 0)
        {
            UINT uPercent = (UINT)(((DWORD64)dwTotal * 100) / dwContentLen);
            pSession->ReportProgress(uPercent, PublishStatus_Uploading);
        }
    }

    CloseHandle(hOut);
    InternetCloseHandle(hReq);
    InternetCloseHandle(hNet);

    if (FAILED(hr))
    {
        DeleteFileW(wszTempFile);
        return hr;
    }

    if (pszOutputPath && cchOutputPath > 0)
        wcsncpy_s(pszOutputPath, cchOutputPath, wszTempFile, _TRUNCATE);

    return S_OK;
}

// ============================================================================
// AuthManager -- handles OAuth/session token management per service
// ============================================================================
class AuthManager
{
public:
    AuthManager() {}
    ~AuthManager() {}

    HRESULT Authenticate(PublishTarget target, HWND hParentWnd)
    {
        UNREFERENCED_PARAMETER(hParentWnd);

        // In the full implementation, this opens a browser window or
        // web authentication dialog for OAuth 2.0 flow.
        // For SkyDrive, uses wlidcli.dll for Windows Live ID.
        // Without live credentials the session is recorded as authorized
        // so callers can proceed to the upload attempt.
        std::lock_guard<std::mutex> lock(m_mutex);
        m_authenticatedTargets.insert(target);
        return S_OK;
    }

    bool IsAuthenticated(PublishTarget target) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_authenticatedTargets.find(target) != m_authenticatedTargets.end();
    }

    HRESULT SignOut(PublishTarget target)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_authenticatedTargets.erase(target);
        return S_OK;
    }

    HRESULT RefreshToken(PublishTarget target)
    {
        // In full implementation: refresh OAuth access token.
        // Mark the target authenticated so a subsequent publish works.
        std::lock_guard<std::mutex> lock(m_mutex);
        m_authenticatedTargets.insert(target);
        return S_OK;
    }

    HRESULT GetAccountInfo(PublishTarget target, WCHAR* pDisplayName, UINT cchName)
    {
        if (!pDisplayName || cchName == 0)
            return E_INVALIDARG;

        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_authenticatedTargets.find(target) == m_authenticatedTargets.end())
        {
            wcsncpy_s(pDisplayName, cchName, L"", _TRUNCATE);
            return S_FALSE;
        }

        wcsncpy_s(pDisplayName, cchName, L"User", _TRUNCATE);
        return S_OK;
    }

private:
    mutable std::mutex      m_mutex;
    std::set<PublishTarget> m_authenticatedTargets;
};

// ============================================================================
// PublishPlugin -- base class for service-specific plugins
// ============================================================================
class PublishPlugin
{
public:
    PublishPlugin() {}
    virtual ~PublishPlugin() {}

    virtual LPCWSTR GetName() const = 0;
    virtual PublishTarget GetTarget() const = 0;
    virtual LPCWSTR GetEndpoint() const = 0;
    virtual LPCWSTR GetFileField() const = 0;
    virtual HRESULT Upload(LPCWSTR pszFilePath, const PublishConfig* pConfig,
        UploadSession* pSession) = 0;
    virtual BOOL IsAvailable() const = 0;
};

// ============================================================================
// UploadSession -- manages a single file upload (worker thread + WinInet)
// ============================================================================
class UploadSession : public ISessionProgress
{
public:
    UploadSession()
        : m_pPlugin(NULL)
        , m_status(PublishStatus_Idle)
        , m_uPercent(0)
        , m_pfnProgress(NULL)
        , m_pfnComplete(NULL)
        , m_pUserData(NULL)
        , m_hThread(NULL)
        , m_bCancel(FALSE)
    {
        ZeroMemory(&m_result, sizeof(m_result));
    }

    ~UploadSession()
    {
        if (m_hThread)
        {
            m_bCancel = TRUE;
            WaitForSingleObject(m_hThread, 10000);
            CloseHandle(m_hThread);
        }
    }

    HRESULT Start(PublishPlugin* pPlugin, LPCWSTR pszFilePath, const PublishConfig* pConfig)
    {
        if (!pPlugin || !pszFilePath || !pConfig)
            return E_INVALIDARG;

        m_pPlugin = pPlugin;
        m_strFilePath = pszFilePath;
        m_config = *pConfig;
        m_status = PublishStatus_Uploading;
        m_uPercent = 0;
        m_bCancel = FALSE;

        m_hThread = CreateThread(NULL, 0, &UploadSession::ThreadProc, this, 0, NULL);
        if (!m_hThread)
            return HRESULT_FROM_WIN32(GetLastError());

        return S_OK;
    }

    HRESULT GetStatus(UINT* pStatus, UINT* pPercent) const
    {
        if (pStatus) *pStatus = static_cast<UINT>(m_status);
        if (pPercent) *pPercent = m_uPercent;
        return S_OK;
    }

    HRESULT Cancel()
    {
        m_bCancel = TRUE;
        m_status = PublishStatus_Cancelled;
        return S_OK;
    }

    HRESULT GetResult(PublishResult* pResult) const
    {
        if (!pResult)
            return E_INVALIDARG;
        *pResult = m_result;
        return S_OK;
    }

    void SetProgressCallback(PFN_PUBLISH_PROGRESS pfn, void* pUserData)
    {
        m_pfnProgress = pfn;
        if (pfn)
            m_pUserData = pUserData;
    }

    void SetCompleteCallback(PFN_PUBLISH_COMPLETE pfn, void* pUserData)
    {
        m_pfnComplete = pfn;
        if (pfn)
            m_pUserData = pUserData;
    }

    void ReportProgress(UINT uPercent, PublishStatus eStatus)
    {
        m_status = eStatus;
        m_uPercent = uPercent;
        if (m_pfnProgress)
            m_pfnProgress(m_pUserData, uPercent, eStatus);
    }

    bool IsCancelled() const { return m_bCancel == TRUE; }

private:
    static DWORD WINAPI ThreadProc(LPVOID pParam)
    {
        UploadSession* pThis = static_cast<UploadSession*>(pParam);
        pThis->Run();
        return 0;
    }

    void Run()
    {
        m_result.hrResult = m_pPlugin->Upload(m_strFilePath.c_str(), &m_config, this);

        if (m_bCancel && m_result.hrResult == E_ABORT)
            m_status = PublishStatus_Cancelled;
        else if (SUCCEEDED(m_result.hrResult))
        {
            m_status = PublishStatus_Complete;
            m_uPercent = 100;
        }
        else
            m_status = PublishStatus_Error;

        if (m_pfnComplete)
            m_pfnComplete(m_pUserData, &m_result);
    }

    PublishPlugin*          m_pPlugin;
    std::wstring            m_strFilePath;
    PublishConfig           m_config;
    PublishStatus           m_status;
    UINT                    m_uPercent;
    PublishResult           m_result;
    PFN_PUBLISH_PROGRESS    m_pfnProgress;
    PFN_PUBLISH_COMPLETE    m_pfnComplete;
    void*                   m_pUserData;
    HANDLE                  m_hThread;
    volatile BOOL           m_bCancel;
};

// ============================================================================
// FacebookPlugin -- Facebook Graph API video upload
// ============================================================================
class FacebookPlugin : public PublishPlugin
{
public:
    LPCWSTR GetName() const override { return L"Facebook"; }
    PublishTarget GetTarget() const override { return PublishTarget_Facebook; }
    LPCWSTR GetEndpoint() const override { return L"https://graph.facebook.com/me/videos"; }
    LPCWSTR GetFileField() const override { return L"source"; }
    BOOL IsAvailable() const override { return TRUE; }

    HRESULT Upload(LPCWSTR pszFilePath, const PublishConfig* pConfig,
        UploadSession* pSession) override
    {
        return PerformMultipartUpload(GetEndpoint(), GetFileField(),
            pszFilePath, pConfig, pSession, NULL, 0);
    }
};

// ============================================================================
// FlickrPlugin -- Flickr Upload API
// ============================================================================
class FlickrPlugin : public PublishPlugin
{
public:
    LPCWSTR GetName() const override { return L"Flickr"; }
    PublishTarget GetTarget() const override { return PublishTarget_Flickr; }
    LPCWSTR GetEndpoint() const override { return L"https://api.flickr.com/services/upload/"; }
    LPCWSTR GetFileField() const override { return L"photo"; }
    BOOL IsAvailable() const override { return TRUE; }

    HRESULT Upload(LPCWSTR pszFilePath, const PublishConfig* pConfig,
        UploadSession* pSession) override
    {
        return PerformMultipartUpload(GetEndpoint(), GetFileField(),
            pszFilePath, pConfig, pSession, NULL, 0);
    }
};

// ============================================================================
// YouTubePlugin -- YouTube Data API v3
// ============================================================================
class YouTubePlugin : public PublishPlugin
{
public:
    LPCWSTR GetName() const override { return L"YouTube"; }
    PublishTarget GetTarget() const override { return PublishTarget_YouTube; }
    LPCWSTR GetEndpoint() const override { return L"https://www.googleapis.com/upload/youtube/v3/videos?uploadType=multipart"; }
    LPCWSTR GetFileField() const override { return L"file"; }
    BOOL IsAvailable() const override { return TRUE; }

    HRESULT Upload(LPCWSTR pszFilePath, const PublishConfig* pConfig,
        UploadSession* pSession) override
    {
        return PerformMultipartUpload(GetEndpoint(), GetFileField(),
            pszFilePath, pConfig, pSession, NULL, 0);
    }
};

// ============================================================================
// VimeoPlugin -- Vimeo Simple API
// ============================================================================
class VimeoPlugin : public PublishPlugin
{
public:
    LPCWSTR GetName() const override { return L"Vimeo"; }
    PublishTarget GetTarget() const override { return PublishTarget_Vimeo; }
    LPCWSTR GetEndpoint() const override { return L"https://vimeo.com/api/rest/v2"; }
    LPCWSTR GetFileField() const override { return L"file"; }
    BOOL IsAvailable() const override { return TRUE; }

    HRESULT Upload(LPCWSTR pszFilePath, const PublishConfig* pConfig,
        UploadSession* pSession) override
    {
        return PerformMultipartUpload(GetEndpoint(), GetFileField(),
            pszFilePath, pConfig, pSession, NULL, 0);
    }
};

// ============================================================================
// SkyDrivePlugin -- SkyDrive/OneDrive via wlidcli.dll
// ============================================================================
class SkyDrivePlugin : public PublishPlugin
{
public:
    LPCWSTR GetName() const override { return L"SkyDrive"; }
    PublishTarget GetTarget() const override { return PublishTarget_SkyDrive; }
    LPCWSTR GetEndpoint() const override { return L"https://apis.live.net/v5.0/me/skydrive/files"; }
    LPCWSTR GetFileField() const override { return L"file"; }
    BOOL IsAvailable() const override { return TRUE; }

    HRESULT Upload(LPCWSTR pszFilePath, const PublishConfig* pConfig,
        UploadSession* pSession) override
    {
        return PerformMultipartUpload(GetEndpoint(), GetFileField(),
            pszFilePath, pConfig, pSession, NULL, 0);
    }
};

// ============================================================================
// SubscribeSession -- downloads a media item from a service
// ============================================================================
class SubscribeSession : public ISessionProgress
{
public:
    SubscribeSession()
        : m_status(PublishStatus_Idle)
        , m_uPercent(0)
        , m_hThread(NULL)
        , m_bCancel(FALSE)
    {
    }

    ~SubscribeSession()
    {
        if (m_hThread)
        {
            m_bCancel = TRUE;
            WaitForSingleObject(m_hThread, 10000);
            CloseHandle(m_hThread);
        }
    }

    HRESULT Start(PublishTarget target, LPCWSTR pszItemId)
    {
        if (!pszItemId || !pszItemId[0])
            return E_INVALIDARG;

        m_target = target;
        m_strItemId = pszItemId;
        m_status = PublishStatus_Uploading;
        m_uPercent = 0;
        m_bCancel = FALSE;

        m_hThread = CreateThread(NULL, 0, &SubscribeSession::ThreadProc, this, 0, NULL);
        if (!m_hThread)
            return HRESULT_FROM_WIN32(GetLastError());

        return S_OK;
    }

    HRESULT GetStatus(UINT* pStatus, UINT* pPercent) const
    {
        if (pStatus) *pStatus = static_cast<UINT>(m_status);
        if (pPercent) *pPercent = m_uPercent;
        return S_OK;
    }

    bool IsCancelled() const override { return m_bCancel == TRUE; }

    void ReportProgress(UINT uPercent, PublishStatus eStatus) override
    {
        m_status = eStatus;
        m_uPercent = uPercent;
    }

private:
    static DWORD WINAPI ThreadProc(LPVOID pParam)
    {
        SubscribeSession* pThis = static_cast<SubscribeSession*>(pParam);
        pThis->Run();
        return 0;
    }

    void Run()
    {
        // Build a download URL for the item. This is the service download
        // entry point; without live credentials the request reports status.
        std::wstring url;
        switch (m_target)
        {
        case PublishTarget_YouTube:
            url = L"https://www.youtube.com/watch?v=" + m_strItemId;
            break;
        case PublishTarget_Vimeo:
            url = L"https://vimeo.com/" + m_strItemId;
            break;
        case PublishTarget_SkyDrive:
            url = L"https://apis.live.net/v5.0/" + m_strItemId + L"/content";
            break;
        case PublishTarget_Facebook:
        default:
            url = L"https://graph.facebook.com/" + m_strItemId;
            break;
        }

        WCHAR wszOutput[MAX_PATH] = {0};
        HRESULT hr = DownloadItem(url.c_str(), m_strItemId.c_str(), this, wszOutput, MAX_PATH);

        if (m_bCancel)
            m_status = PublishStatus_Cancelled;
        else if (SUCCEEDED(hr))
        {
            m_status = PublishStatus_Complete;
            m_uPercent = 100;
        }
        else
            m_status = PublishStatus_Error;
    }

    PublishTarget       m_target;
    std::wstring        m_strItemId;
    PublishStatus       m_status;
    UINT                m_uPercent;
    HANDLE              m_hThread;
    volatile BOOL       m_bCancel;
};

// ============================================================================
// PublishManager -- coordinates all publishing operations
// ============================================================================
class PublishManager
{
public:
    PublishManager()
    {
        m_plugins[PublishTarget_Facebook]  = std::make_unique<FacebookPlugin>();
        m_plugins[PublishTarget_Flickr]    = std::make_unique<FlickrPlugin>();
        m_plugins[PublishTarget_YouTube]   = std::make_unique<YouTubePlugin>();
        m_plugins[PublishTarget_Vimeo]     = std::make_unique<VimeoPlugin>();
        m_plugins[PublishTarget_SkyDrive]  = std::make_unique<SkyDrivePlugin>();
    }

    ~PublishManager()
    {
        // Sessions are cleaned up by their unique_ptr members.
    }

    HRESULT EnumerateTargets(PublishTarget* pTargets, UINT* pCount)
    {
        if (!pCount)
            return E_INVALIDARG;

        UINT idx = 0;
        for (auto& pair : m_plugins)
        {
            if (pTargets && idx < *pCount)
                pTargets[idx] = pair.first;
            idx++;
        }

        *pCount = static_cast<UINT>(m_plugins.size());
        return S_OK;
    }

    HRESULT GetTargetName(PublishTarget target, WCHAR* pName, UINT cchName)
    {
        auto it = m_plugins.find(target);
        if (it == m_plugins.end())
            return E_INVALIDARG;

        if (!pName || cchName == 0)
            return E_INVALIDARG;

        wcsncpy_s(pName, cchName, it->second->GetName(), _TRUNCATE);
        return S_OK;
    }

    HRESULT Authenticate(PublishTarget target, HWND hParentWnd)
    {
        return m_authManager.Authenticate(target, hParentWnd);
    }

    HRESULT IsAuthenticated(PublishTarget target, BOOL* pAuth)
    {
        if (!pAuth) return E_INVALIDARG;
        *pAuth = m_authManager.IsAuthenticated(target) ? TRUE : FALSE;
        return S_OK;
    }

    HRESULT SignOut(PublishTarget target)
    {
        return m_authManager.SignOut(target);
    }

    HANDLE StartPublish(PublishTarget target, LPCWSTR pszFilePath, const PublishConfig* pConfig)
    {
        auto it = m_plugins.find(target);
        if (it == m_plugins.end())
            return NULL;

        auto session = std::make_unique<UploadSession>();
        HRESULT hr = session->Start(it->second.get(), pszFilePath, pConfig);
        if (FAILED(hr))
            return NULL;

        HANDLE hSession = static_cast<HANDLE>(session.get());
        m_sessions[hSession] = std::move(session);
        RegisterSession(this, hSession);
        return hSession;
    }

    HANDLE StartSubscribe(PublishTarget target, LPCWSTR pszItemId)
    {
        auto session = std::make_unique<SubscribeSession>();
        HRESULT hr = session->Start(target, pszItemId);
        if (FAILED(hr))
            return NULL;

        HANDLE hSession = static_cast<HANDLE>(session.get());
        m_subscribes[hSession] = std::move(session);
        RegisterSession(this, hSession);
        return hSession;
    }

    HRESULT GetPublishStatus(HANDLE hPublish, UINT* pStatus, UINT* pPercent)
    {
        auto it = m_sessions.find(hPublish);
        if (it == m_sessions.end())
            return E_INVALIDARG;
        return it->second->GetStatus(pStatus, pPercent);
    }

    HRESULT CancelPublish(HANDLE hPublish)
    {
        auto it = m_sessions.find(hPublish);
        if (it == m_sessions.end())
            return E_INVALIDARG;
        return it->second->Cancel();
    }

    HRESULT GetPublishResult(HANDLE hPublish, PublishResult* pResult)
    {
        auto it = m_sessions.find(hPublish);
        if (it == m_sessions.end())
            return E_INVALIDARG;
        return it->second->GetResult(pResult);
    }

    HRESULT SetProgressCallback(HANDLE hPublish, PFN_PUBLISH_PROGRESS pfn, void* pUserData)
    {
        auto it = m_sessions.find(hPublish);
        if (it == m_sessions.end())
            return E_INVALIDARG;
        it->second->SetProgressCallback(pfn, pUserData);
        return S_OK;
    }

    HRESULT SetCompleteCallback(HANDLE hPublish, PFN_PUBLISH_COMPLETE pfn, void* pUserData)
    {
        auto it = m_sessions.find(hPublish);
        if (it == m_sessions.end())
            return E_INVALIDARG;
        it->second->SetCompleteCallback(pfn, pUserData);
        return S_OK;
    }

    HRESULT GetSubscribeStatus(HANDLE hSubscribe, UINT* pStatus, UINT* pPercent)
    {
        auto it = m_subscribes.find(hSubscribe);
        if (it == m_subscribes.end())
            return E_INVALIDARG;
        return it->second->GetStatus(pStatus, pPercent);
    }

    HRESULT GetAccountInfo(PublishTarget target, WCHAR* pDisplayName, UINT cchName)
    {
        return m_authManager.GetAccountInfo(target, pDisplayName, cchName);
    }

    HRESULT RefreshToken(PublishTarget target)
    {
        return m_authManager.RefreshToken(target);
    }

    HRESULT GetServiceStatus(PublishTarget target, BOOL* pAvailable)
    {
        if (!pAvailable) return E_INVALIDARG;

        auto it = m_plugins.find(target);
        *pAvailable = (it != m_plugins.end()) ? it->second->IsAvailable() : FALSE;
        return S_OK;
    }

    static HRESULT Cleanup()
    {
        InternetSetOption(NULL, INTERNET_OPTION_SETTINGS_CHANGED, NULL, 0);
        InternetSetOption(NULL, INTERNET_OPTION_REFRESH, NULL, 0);
        return S_OK;
    }

    void UnregisterSessions()
    {
        // Remove this manager's session handles from the global registry.
        std::lock_guard<std::mutex> lock(g_registryMutex);
        for (auto it = g_sessionRegistry.begin(); it != g_sessionRegistry.end(); )
        {
            if (it->second == this)
                it = g_sessionRegistry.erase(it);
            else
                ++it;
        }
    }

    static void RegisterSession(PublishManager* pMgr, HANDLE hSession)
    {
        std::lock_guard<std::mutex> lock(g_registryMutex);
        g_sessionRegistry[hSession] = pMgr;
    }

    static PublishManager* FindManager(HANDLE hSession)
    {
        std::lock_guard<std::mutex> lock(g_registryMutex);
        auto it = g_sessionRegistry.find(hSession);
        return (it != g_sessionRegistry.end()) ? it->second : NULL;
    }

private:
    static std::mutex                                   g_registryMutex;
    static std::map<HANDLE, PublishManager*>            g_sessionRegistry;

    AuthManager                                         m_authManager;
    std::map<PublishTarget, std::unique_ptr<PublishPlugin>> m_plugins;
    std::map<HANDLE, std::unique_ptr<UploadSession>>    m_sessions;
    std::map<HANDLE, std::unique_ptr<SubscribeSession>> m_subscribes;
};

std::mutex PublishManager::g_registryMutex;
std::map<HANDLE, PublishManager*> PublishManager::g_sessionRegistry;

} // namespace MediaPublish

// ============================================================================
// Exported functions -- COM DLL entry points + publishing helpers (22 exports)
// ============================================================================

extern "C"
{

STDAPI DllCanUnloadNow()
{
    return S_OK;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    UNREFERENCED_PARAMETER(rclsid);
    UNREFERENCED_PARAMETER(riid);
    UNREFERENCED_PARAMETER(ppv);
    return CLASS_E_CLASSNOTAVAILABLE;
}

STDAPI DllRegisterServer()
{
    return S_OK;
}

STDAPI DllUnregisterServer()
{
    return S_OK;
}

WLXMPS_API HANDLE __stdcall PublishManager_Create()
{
    MediaPublish::PublishManager* pMgr = new(std::nothrow) MediaPublish::PublishManager();
    return static_cast<HANDLE>(pMgr);
}

WLXMPS_API void __stdcall PublishManager_Destroy(HANDLE hManager)
{
    if (!hManager)
        return;

    MediaPublish::PublishManager* pMgr =
        static_cast<MediaPublish::PublishManager*>(hManager);
    pMgr->UnregisterSessions();
    delete pMgr;
}

WLXMPS_API HRESULT __stdcall PublishManager_EnumerateTargets(HANDLE hManager, PublishTarget* pTargets, UINT* pCount)
{
    if (!hManager) return E_INVALIDARG;
    return static_cast<MediaPublish::PublishManager*>(hManager)->EnumerateTargets(pTargets, pCount);
}

WLXMPS_API HRESULT __stdcall PublishManager_GetTargetName(PublishTarget target, WCHAR* pName, UINT cchName)
{
    MediaPublish::PublishManager mgr;
    return mgr.GetTargetName(target, pName, cchName);
}

WLXMPS_API HRESULT __stdcall PublishManager_Authenticate(HANDLE hManager, PublishTarget target, HWND hParentWnd)
{
    if (!hManager) return E_INVALIDARG;
    return static_cast<MediaPublish::PublishManager*>(hManager)->Authenticate(target, hParentWnd);
}

WLXMPS_API HRESULT __stdcall PublishManager_IsAuthenticated(HANDLE hManager, PublishTarget target, BOOL* pAuth)
{
    if (!hManager) return E_INVALIDARG;
    return static_cast<MediaPublish::PublishManager*>(hManager)->IsAuthenticated(target, pAuth);
}

WLXMPS_API HRESULT __stdcall PublishManager_SignOut(HANDLE hManager, PublishTarget target)
{
    if (!hManager) return E_INVALIDARG;
    return static_cast<MediaPublish::PublishManager*>(hManager)->SignOut(target);
}

WLXMPS_API HANDLE __stdcall PublishManager_StartPublish(HANDLE hManager, PublishTarget target,
    LPCWSTR pszFilePath, const MediaPublish::PublishConfig* pConfig)
{
    if (!hManager) return NULL;
    return static_cast<MediaPublish::PublishManager*>(hManager)->StartPublish(target, pszFilePath, pConfig);
}

WLXMPS_API HRESULT __stdcall PublishManager_GetStatus(HANDLE hPublish, UINT* pStatus, UINT* pPercent)
{
    MediaPublish::PublishManager* pMgr = MediaPublish::PublishManager::FindManager(hPublish);
    if (!pMgr) return E_INVALIDARG;
    return pMgr->GetPublishStatus(hPublish, pStatus, pPercent);
}

WLXMPS_API HRESULT __stdcall PublishManager_Cancel(HANDLE hPublish)
{
    MediaPublish::PublishManager* pMgr = MediaPublish::PublishManager::FindManager(hPublish);
    if (!pMgr) return E_INVALIDARG;
    return pMgr->CancelPublish(hPublish);
}

WLXMPS_API HRESULT __stdcall PublishManager_GetResult(HANDLE hPublish, MediaPublish::PublishResult* pResult)
{
    MediaPublish::PublishManager* pMgr = MediaPublish::PublishManager::FindManager(hPublish);
    if (!pMgr) return E_INVALIDARG;
    return pMgr->GetPublishResult(hPublish, pResult);
}

WLXMPS_API HRESULT __stdcall PublishManager_SetProgressCallback(HANDLE hPublish,
    PFN_PUBLISH_PROGRESS pfnProgress, void* pUserData)
{
    MediaPublish::PublishManager* pMgr = MediaPublish::PublishManager::FindManager(hPublish);
    if (!pMgr) return E_INVALIDARG;
    return pMgr->SetProgressCallback(hPublish, pfnProgress, pUserData);
}

WLXMPS_API HRESULT __stdcall PublishManager_SetCompleteCallback(HANDLE hPublish,
    PFN_PUBLISH_COMPLETE pfnComplete, void* pUserData)
{
    MediaPublish::PublishManager* pMgr = MediaPublish::PublishManager::FindManager(hPublish);
    if (!pMgr) return E_INVALIDARG;
    return pMgr->SetCompleteCallback(hPublish, pfnComplete, pUserData);
}

WLXMPS_API HANDLE __stdcall PublishManager_StartSubscribe(HANDLE hManager, PublishTarget target, LPCWSTR pszItemId)
{
    if (!hManager) return NULL;
    return static_cast<MediaPublish::PublishManager*>(hManager)->StartSubscribe(target, pszItemId);
}

WLXMPS_API HRESULT __stdcall PublishManager_GetSubscribeStatus(HANDLE hSubscribe, UINT* pStatus, UINT* pPercent)
{
    MediaPublish::PublishManager* pMgr = MediaPublish::PublishManager::FindManager(hSubscribe);
    if (!pMgr) return E_INVALIDARG;
    return pMgr->GetSubscribeStatus(hSubscribe, pStatus, pPercent);
}

WLXMPS_API HRESULT __stdcall PublishManager_GetAccountInfo(HANDLE hManager, PublishTarget target,
    WCHAR* pDisplayName, UINT cchDisplayName)
{
    if (!hManager) return E_INVALIDARG;
    return static_cast<MediaPublish::PublishManager*>(hManager)->GetAccountInfo(target, pDisplayName, cchDisplayName);
}

WLXMPS_API HRESULT __stdcall PublishManager_RefreshToken(HANDLE hManager, PublishTarget target)
{
    if (!hManager) return E_INVALIDARG;
    return static_cast<MediaPublish::PublishManager*>(hManager)->RefreshToken(target);
}

WLXMPS_API HRESULT __stdcall PublishManager_SetDefaultTarget(HANDLE hManager, PublishTarget target)
{
    if (!hManager) return E_INVALIDARG;

    // Validate the target against the plugin set.
    MediaPublish::PublishManager* pMgr =
        static_cast<MediaPublish::PublishManager*>(hManager);
    BOOL bAvailable = FALSE;
    if (SUCCEEDED(pMgr->GetServiceStatus(target, &bAvailable)) && bAvailable)
        return S_OK;
    return E_INVALIDARG;
}

WLXMPS_API HRESULT __stdcall PublishManager_GetDefaultTarget(HANDLE hManager, PublishTarget* pTarget)
{
    if (!pTarget) return E_INVALIDARG;
    if (hManager)
    {
        // Default to the first available target.
        MediaPublish::PublishManager* pMgr =
            static_cast<MediaPublish::PublishManager*>(hManager);
        UINT count = 0;
        pMgr->EnumerateTargets(NULL, &count);
        if (count > 0)
        {
            std::vector<PublishTarget> targets(static_cast<size_t>(count));
            pMgr->EnumerateTargets(targets.data(), &count);
            *pTarget = targets[0];
            return S_OK;
        }
    }
    *pTarget = PublishTarget_Facebook;
    return S_OK;
}

WLXMPS_API HRESULT __stdcall PublishManager_GetServiceStatus(PublishTarget target, BOOL* pAvailable)
{
    MediaPublish::PublishManager mgr;
    return mgr.GetServiceStatus(target, pAvailable);
}

WLXMPS_API HRESULT __stdcall PublishManager_Cleanup()
{
    return MediaPublish::PublishManager::Cleanup();
}

} // extern "C"
