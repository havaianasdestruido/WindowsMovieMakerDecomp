#include "pch.h"
/*
 * MediaItem.cpp
 *
 * Implementation of the media item hierarchy for timeline content.
 * Includes Metadata, ProxyInfo, MediaItemBase, MediaItem, MediaClipBase,
 * AudioVideoMediaClip, VideoClip, ImageClip, AudioClip, and TitleClip.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "MediaItem.h"
#include <strsafe.h>

namespace StoryboardManager
{

// ============================================================================
// Metadata implementation
// ============================================================================

Metadata::Metadata()
    : m_llDurationHns(0)
    , m_uWidth(0)
    , m_uHeight(0)
    , m_dwFrameRate(0)
    , m_dwVideoBitRate(0)
    , m_dwAudioSampleRate(0)
    , m_dwAudioChannels(0)
    , m_dwAudioBitRate(0)
    , m_uRating(0)
{
    ZeroMemory(&m_ftDateTaken, sizeof(FILETIME));
}

Metadata::~Metadata()
{
}

LONGLONG Metadata::GetDurationHns() const throw()
{
    return m_llDurationHns;
}

void Metadata::SetDurationHns(LONGLONG llDuration) throw()
{
    m_llDurationHns = llDuration;
}

UINT Metadata::GetWidth() const throw()
{
    return m_uWidth;
}

UINT Metadata::GetHeight() const throw()
{
    return m_uHeight;
}

void Metadata::SetDimensions(UINT cx, UINT cy) throw()
{
    m_uWidth = cx;
    m_uHeight = cy;
}

DWORD Metadata::GetFrameRate() const throw()
{
    return m_dwFrameRate;
}

void Metadata::SetFrameRate(DWORD dwFrameRate) throw()
{
    m_dwFrameRate = dwFrameRate;
}

ATL::CString Metadata::GetVideoCodec() const
{
    return m_strVideoCodec;
}

void Metadata::SetVideoCodec(LPCWSTR pszCodec)
{
    m_strVideoCodec = pszCodec ? pszCodec : L"";
}

DWORD Metadata::GetVideoBitRate() const throw()
{
    return m_dwVideoBitRate;
}

void Metadata::SetVideoBitRate(DWORD dwBitRate) throw()
{
    m_dwVideoBitRate = dwBitRate;
}

ATL::CString Metadata::GetAudioCodec() const
{
    return m_strAudioCodec;
}

void Metadata::SetAudioCodec(LPCWSTR pszCodec)
{
    m_strAudioCodec = pszCodec ? pszCodec : L"";
}

DWORD Metadata::GetAudioSampleRate() const throw()
{
    return m_dwAudioSampleRate;
}

void Metadata::SetAudioSampleRate(DWORD dwSampleRate) throw()
{
    m_dwAudioSampleRate = dwSampleRate;
}

DWORD Metadata::GetAudioChannels() const throw()
{
    return m_dwAudioChannels;
}

void Metadata::SetAudioChannels(DWORD dwChannels) throw()
{
    m_dwAudioChannels = dwChannels;
}

DWORD Metadata::GetAudioBitRate() const throw()
{
    return m_dwAudioBitRate;
}

void Metadata::SetAudioBitRate(DWORD dwBitRate) throw()
{
    m_dwAudioBitRate = dwBitRate;
}

FILETIME Metadata::GetDateTaken() const throw()
{
    return m_ftDateTaken;
}

void Metadata::SetDateTaken(const FILETIME& ft) throw()
{
    m_ftDateTaken = ft;
}

UINT Metadata::GetRating() const throw()
{
    return m_uRating;
}

void Metadata::SetRating(UINT uRating) throw()
{
    m_uRating = (uRating <= 5) ? uRating : 0;
}

ATL::CString Metadata::GetTags() const
{
    return m_strTags;
}

void Metadata::SetTags(LPCWSTR pszTags)
{
    m_strTags = pszTags ? pszTags : L"";
}

ATL::CString Metadata::GetTitle() const
{
    return m_strTitle;
}

void Metadata::SetTitle(LPCWSTR pszTitle)
{
    m_strTitle = pszTitle ? pszTitle : L"";
}

ATL::CString Metadata::GetSubtitle() const
{
    return m_strSubtitle;
}

void Metadata::SetSubtitle(LPCWSTR pszSubtitle)
{
    m_strSubtitle = pszSubtitle ? pszSubtitle : L"";
}

ATL::CString Metadata::GetArtist() const
{
    return m_strArtist;
}

void Metadata::SetArtist(LPCWSTR pszArtist)
{
    m_strArtist = pszArtist ? pszArtist : L"";
}

ATL::CString Metadata::GetAlbum() const
{
    return m_strAlbum;
}

void Metadata::SetAlbum(LPCWSTR pszAlbum)
{
    m_strAlbum = pszAlbum ? pszAlbum : L"";
}

ATL::CString Metadata::GetGenre() const
{
    return m_strGenre;
}

void Metadata::SetGenre(LPCWSTR pszGenre)
{
    m_strGenre = pszGenre ? pszGenre : L"";
}

// ============================================================================
// ProxyInfo implementation
// ============================================================================

ProxyInfo::ProxyInfo()
    : m_uProxyWidth(0)
    , m_uProxyHeight(0)
    , m_transcodeState(ExtentTranscodeStateNone)
{
}

ProxyInfo::~ProxyInfo()
{
}

ATL::CString ProxyInfo::GetProxyPath() const
{
    return m_strProxyPath;
}

void ProxyInfo::SetProxyPath(LPCWSTR pszPath)
{
    m_strProxyPath = pszPath ? pszPath : L"";
}

UINT ProxyInfo::GetProxyWidth() const throw()
{
    return m_uProxyWidth;
}

UINT ProxyInfo::GetProxyHeight() const throw()
{
    return m_uProxyHeight;
}

void ProxyInfo::SetProxyDimensions(UINT cx, UINT cy) throw()
{
    m_uProxyWidth = cx;
    m_uProxyHeight = cy;
}

ExtentTranscodeState ProxyInfo::GetTranscodeState() const throw()
{
    return m_transcodeState;
}

void ProxyInfo::SetTranscodeState(ExtentTranscodeState state) throw()
{
    if (state >= ExtentTranscodeStateNone && state <= ExtentTranscodeStateCancelled)
        m_transcodeState = state;
}

ATL::CString ProxyInfo::GetSourceHash() const
{
    return m_strSourceHash;
}

void ProxyInfo::SetSourceHash(LPCWSTR pszHash)
{
    m_strSourceHash = pszHash ? pszHash : L"";
}

bool ProxyInfo::IsTranscoding() const throw()
{
    return m_transcodeState == ExtentTranscodeStatePending ||
           m_transcodeState == ExtentTranscodeStateInProgress;
}

bool ProxyInfo::IsReady() const throw()
{
    return m_transcodeState == ExtentTranscodeStateComplete;
}

bool ProxyInfo::IsError() const throw()
{
    return m_transcodeState == ExtentTranscodeStateFailed;
}

bool ProxyInfo::IsValid() const throw()
{
    return !m_strProxyPath.IsEmpty() &&
           m_transcodeState == ExtentTranscodeStateComplete;
}

bool ProxyInfo::ProxyFileExists() const
{
    if (m_strProxyPath.IsEmpty())
        return false;
    return ::PathFileExistsW(m_strProxyPath) != FALSE;
}

// ============================================================================
// MediaItemBase implementation
// ============================================================================

MediaItemBase::MediaItemBase()
    : m_dwItemId(0)
    , m_llDurationHns(0)
{
}

MediaItemBase::~MediaItemBase()
{
}

DWORD MediaItemBase::GetItemId() const throw()
{
    return m_dwItemId;
}

void MediaItemBase::SetItemId(DWORD dwId) throw()
{
    m_dwItemId = dwId;
}

MediaItemType MediaItemBase::GetItemType() const throw()
{
    return MediaItemTypeUnknown;
}

ATL::CString MediaItemBase::GetSourcePath() const
{
    return m_strSourcePath;
}

void MediaItemBase::SetSourcePath(LPCWSTR pszPath)
{
    m_strSourcePath = pszPath ? pszPath : L"";

    // Auto-generate display name from file name if not already set
    if (!m_strSourcePath.IsEmpty() && m_strDisplayName.IsEmpty())
    {
        LPCWSTR pszFileName = PathFindFileNameW(m_strSourcePath);
        if (pszFileName && pszFileName[0] != L'\0')
        {
            m_strDisplayName = pszFileName;

            // Strip extension for display
            LPCWSTR pszBase = (LPCWSTR)m_strDisplayName;
            LPWSTR pszDot = PathFindExtensionW(m_strDisplayName);
            if (pszDot && pszDot > pszBase && pszDot[0] == L'.')
                m_strDisplayName.SetAt((int)(pszDot - pszBase), L'\0');
        }
    }
}

ATL::CString MediaItemBase::GetDisplayName() const
{
    return m_strDisplayName;
}

void MediaItemBase::SetDisplayName(LPCWSTR pszName)
{
    m_strDisplayName = pszName ? pszName : L"";
}

LONGLONG MediaItemBase::GetDurationHns() const throw()
{
    return m_llDurationHns;
}

void MediaItemBase::SetDurationHns(LONGLONG llDuration) throw()
{
    m_llDurationHns = llDuration;
}

const Metadata& MediaItemBase::GetMetadata() const
{
    return m_metadata;
}

Metadata& MediaItemBase::GetMetadata()
{
    return m_metadata;
}

void MediaItemBase::SetMetadata(const Metadata& metadata)
{
    m_metadata = metadata;
}

const ProxyInfo& MediaItemBase::GetProxyInfo() const
{
    return m_proxyInfo;
}

ProxyInfo& MediaItemBase::GetProxyInfo()
{
    return m_proxyInfo;
}

void MediaItemBase::SetProxyInfo(const ProxyInfo& proxyInfo)
{
    m_proxyInfo = proxyInfo;
}

bool MediaItemBase::IsValid() const
{
    return m_dwItemId != 0 && !m_strSourcePath.IsEmpty();
}

bool MediaItemBase::FileExists() const
{
    if (m_strSourcePath.IsEmpty())
        return false;
    return ::PathFileExistsW(m_strSourcePath) != FALSE;
}

MediaItemBase* MediaItemBase::Clone() const
{
    MediaItemBase* pClone = new MediaItemBase();
    pClone->SetItemId(GetItemId());
    pClone->SetSourcePath(GetSourcePath());
    pClone->SetDisplayName(GetDisplayName());
    pClone->SetDurationHns(GetDurationHns());
    pClone->SetMetadata(GetMetadata());
    pClone->SetProxyInfo(GetProxyInfo());
    return pClone;
}

// ============================================================================
// MediaItem implementation
// ============================================================================

MediaItem::MediaItem()
    : m_itemType(MediaItemTypeUnknown)
    , m_cbFileSize(0)
{
}

MediaItem::~MediaItem()
{
}

MediaItemType MediaItem::GetItemType() const throw()
{
    return m_itemType;
}

void MediaItem::SetItemType(MediaItemType type) throw()
{
    m_itemType = type;
}

void MediaItem::AutoDetectType()
{
    ATL::CString strSourcePath = GetSourcePath();
    if (strSourcePath.IsEmpty())
    {
        m_itemType = MediaItemTypeUnknown;
        return;
    }

    LPCWSTR pszExt = PathFindExtensionW(strSourcePath);
    if (!pszExt || pszExt[0] == L'\0')
    {
        m_itemType = MediaItemTypeUnknown;
        return;
    }

    // Compare without case
    if (_wcsicmp(pszExt, L".jpg") == 0 || _wcsicmp(pszExt, L".jpeg") == 0 ||
        _wcsicmp(pszExt, L".png") == 0 || _wcsicmp(pszExt, L".bmp") == 0 ||
        _wcsicmp(pszExt, L".gif") == 0 || _wcsicmp(pszExt, L".tiff") == 0 ||
        _wcsicmp(pszExt, L".tif") == 0 || _wcsicmp(pszExt, L".ico") == 0 ||
        _wcsicmp(pszExt, L".webp") == 0)
    {
        m_itemType = MediaItemTypeImage;
    }
    else if (_wcsicmp(pszExt, L".mp3") == 0 || _wcsicmp(pszExt, L".wav") == 0 ||
             _wcsicmp(pszExt, L".wma") == 0 || _wcsicmp(pszExt, L".aac") == 0 ||
             _wcsicmp(pszExt, L".m4a") == 0 || _wcsicmp(pszExt, L".flac") == 0 ||
             _wcsicmp(pszExt, L".ogg") == 0)
    {
        m_itemType = MediaItemTypeAudio;
    }
    else if (_wcsicmp(pszExt, L".mp4") == 0 || _wcsicmp(pszExt, L".wmv") == 0 ||
             _wcsicmp(pszExt, L".avi") == 0 || _wcsicmp(pszExt, L".mov") == 0 ||
             _wcsicmp(pszExt, L".mkv") == 0 || _wcsicmp(pszExt, L".mpg") == 0 ||
             _wcsicmp(pszExt, L".mpeg") == 0 || _wcsicmp(pszExt, L".m4v") == 0 ||
             _wcsicmp(pszExt, L".3gp") == 0 || _wcsicmp(pszExt, L".mts") == 0 ||
             _wcsicmp(pszExt, L".m2ts") == 0 || _wcsicmp(pszExt, L".wm") == 0)
    {
        m_itemType = MediaItemTypeVideo;
    }
    else
    {
        m_itemType = MediaItemTypeUnknown;
    }
}

ATL::CString MediaItem::GetThumbnailPath() const
{
    return m_strThumbnailPath;
}

void MediaItem::SetThumbnailPath(LPCWSTR pszPath)
{
    m_strThumbnailPath = pszPath ? pszPath : L"";
}

ULONGLONG MediaItem::GetFileSize() const throw()
{
    return m_cbFileSize;
}

void MediaItem::SetFileSize(ULONGLONG cbSize) throw()
{
    m_cbFileSize = cbSize;
}

MediaItemBase* MediaItem::Clone() const
{
    MediaItem* pClone = new MediaItem();
    pClone->SetItemId(GetItemId());
    pClone->SetSourcePath(GetSourcePath());
    pClone->SetDisplayName(GetDisplayName());
    pClone->SetDurationHns(GetDurationHns());
    pClone->SetMetadata(GetMetadata());
    pClone->SetProxyInfo(GetProxyInfo());
    pClone->SetItemType(GetItemType());
    pClone->SetThumbnailPath(GetThumbnailPath());
    pClone->SetFileSize(GetFileSize());
    return pClone;
}

// ============================================================================
// MediaClipBase implementation
// ============================================================================

MediaClipBase::MediaClipBase()
    : m_llTrimStartHns(0)
    , m_llTrimEndHns(0)
    , m_dblSpeedFactor(1.0)
    , m_dblVolume(1.0)
    , m_dblPan(0.0)
    , m_fMuted(false)
    , m_fReversed(false)
    , m_llFadeInDurationHns(0)
    , m_llFadeOutDurationHns(0)
{
}

MediaClipBase::~MediaClipBase()
{
}

LONGLONG MediaClipBase::GetTrimStartHns() const throw()
{
    return m_llTrimStartHns;
}

void MediaClipBase::SetTrimStartHns(LONGLONG llStart) throw()
{
    m_llTrimStartHns = llStart;
}

LONGLONG MediaClipBase::GetTrimEndHns() const throw()
{
    return m_llTrimEndHns;
}

void MediaClipBase::SetTrimEndHns(LONGLONG llEnd) throw()
{
    m_llTrimEndHns = llEnd;
}

LONGLONG MediaClipBase::GetClipDurationHns() const throw()
{
    if (m_llTrimEndHns <= m_llTrimStartHns)
        return 0;
    return m_llTrimEndHns - m_llTrimStartHns;
}

double MediaClipBase::GetSpeedFactor() const throw()
{
    return m_dblSpeedFactor;
}

void MediaClipBase::SetSpeedFactor(double dblSpeed) throw()
{
    m_dblSpeedFactor = (dblSpeed > 0.0) ? dblSpeed : 1.0;
}

double MediaClipBase::GetVolume() const throw()
{
    return m_dblVolume;
}

void MediaClipBase::SetVolume(double dblVolume) throw()
{
    if (dblVolume < 0.0) dblVolume = 0.0;
    if (dblVolume > 1.0) dblVolume = 1.0;
    m_dblVolume = dblVolume;
}

double MediaClipBase::GetPan() const throw()
{
    return m_dblPan;
}

void MediaClipBase::SetPan(double dblPan) throw()
{
    if (dblPan < -1.0) dblPan = -1.0;
    if (dblPan > 1.0) dblPan = 1.0;
    m_dblPan = dblPan;
}

bool MediaClipBase::IsMuted() const throw()
{
    return m_fMuted;
}

void MediaClipBase::SetMuted(bool fMuted) throw()
{
    m_fMuted = fMuted;
}

bool MediaClipBase::IsReversed() const throw()
{
    return m_fReversed;
}

void MediaClipBase::SetReversed(bool fReversed) throw()
{
    m_fReversed = fReversed;
}

LONGLONG MediaClipBase::GetFadeInDurationHns() const throw()
{
    return m_llFadeInDurationHns;
}

void MediaClipBase::SetFadeInDurationHns(LONGLONG llDuration) throw()
{
    m_llFadeInDurationHns = llDuration;
}

LONGLONG MediaClipBase::GetFadeOutDurationHns() const throw()
{
    return m_llFadeOutDurationHns;
}

void MediaClipBase::SetFadeOutDurationHns(LONGLONG llDuration) throw()
{
    m_llFadeOutDurationHns = llDuration;
}

// ============================================================================
// AudioVideoMediaClip implementation
// ============================================================================

AudioVideoMediaClip::AudioVideoMediaClip()
    : m_dwVideoStreamIndex(0)
    , m_dwAudioStreamIndex(0)
    , m_fHasAudioStream(true)
{
}

AudioVideoMediaClip::~AudioVideoMediaClip()
{
}

MediaItemType AudioVideoMediaClip::GetItemType() const throw()
{
    return MediaItemTypeVideo;
}

DWORD AudioVideoMediaClip::GetVideoStreamIndex() const throw()
{
    return m_dwVideoStreamIndex;
}

void AudioVideoMediaClip::SetVideoStreamIndex(DWORD dwIndex) throw()
{
    m_dwVideoStreamIndex = dwIndex;
}

DWORD AudioVideoMediaClip::GetAudioStreamIndex() const throw()
{
    return m_dwAudioStreamIndex;
}

void AudioVideoMediaClip::SetAudioStreamIndex(DWORD dwIndex) throw()
{
    m_dwAudioStreamIndex = dwIndex;
}

bool AudioVideoMediaClip::HasAudioStream() const throw()
{
    return m_fHasAudioStream;
}

void AudioVideoMediaClip::SetHasAudioStream(bool fHas) throw()
{
    m_fHasAudioStream = fHas;
}

MediaItemBase* AudioVideoMediaClip::Clone() const
{
    AudioVideoMediaClip* pClone = new AudioVideoMediaClip();
    pClone->SetItemId(GetItemId());
    pClone->SetSourcePath(GetSourcePath());
    pClone->SetDisplayName(GetDisplayName());
    pClone->SetDurationHns(GetDurationHns());
    pClone->SetMetadata(GetMetadata());
    pClone->SetProxyInfo(GetProxyInfo());
    pClone->SetTrimStartHns(GetTrimStartHns());
    pClone->SetTrimEndHns(GetTrimEndHns());
    pClone->SetSpeedFactor(GetSpeedFactor());
    pClone->SetVolume(GetVolume());
    pClone->SetPan(GetPan());
    pClone->SetMuted(IsMuted());
    pClone->SetReversed(IsReversed());
    pClone->SetFadeInDurationHns(GetFadeInDurationHns());
    pClone->SetFadeOutDurationHns(GetFadeOutDurationHns());
    pClone->SetVideoStreamIndex(GetVideoStreamIndex());
    pClone->SetAudioStreamIndex(GetAudioStreamIndex());
    pClone->SetHasAudioStream(HasAudioStream());
    return pClone;
}

// ============================================================================
// VideoClip implementation
// ============================================================================

VideoClip::VideoClip()
{
}

VideoClip::~VideoClip()
{
}

MediaItemType VideoClip::GetItemType() const throw()
{
    return MediaItemTypeVideo;
}

size_t VideoClip::GetEffectCount() const throw()
{
    return m_arrEffectIds.GetCount();
}

DWORD VideoClip::GetEffectAt(size_t nIndex) const
{
    if (nIndex >= m_arrEffectIds.GetCount())
        return 0;
    return m_arrEffectIds.GetAt(nIndex);
}

void VideoClip::AddEffect(DWORD dwEffectId)
{
    m_arrEffectIds.Add(dwEffectId);
}

void VideoClip::RemoveAllEffects()
{
    m_arrEffectIds.RemoveAll();
}

MediaItemBase* VideoClip::Clone() const
{
    VideoClip* pClone = new VideoClip();
    pClone->SetItemId(GetItemId());
    pClone->SetSourcePath(GetSourcePath());
    pClone->SetDisplayName(GetDisplayName());
    pClone->SetDurationHns(GetDurationHns());
    pClone->SetMetadata(GetMetadata());
    pClone->SetProxyInfo(GetProxyInfo());
    pClone->SetTrimStartHns(GetTrimStartHns());
    pClone->SetTrimEndHns(GetTrimEndHns());
    pClone->SetSpeedFactor(GetSpeedFactor());
    pClone->SetVolume(GetVolume());
    pClone->SetPan(GetPan());
    pClone->SetMuted(IsMuted());
    pClone->SetReversed(IsReversed());
    pClone->SetFadeInDurationHns(GetFadeInDurationHns());
    pClone->SetFadeOutDurationHns(GetFadeOutDurationHns());
    pClone->SetVideoStreamIndex(GetVideoStreamIndex());
    pClone->SetAudioStreamIndex(GetAudioStreamIndex());
    pClone->SetHasAudioStream(HasAudioStream());
    for (size_t i = 0; i < GetEffectCount(); ++i)
        pClone->AddEffect(GetEffectAt(i));
    return pClone;
}

// ============================================================================
// ImageClip implementation
// ============================================================================

ImageClip::ImageClip()
    : m_dblPanStartX(0.0)
    , m_dblPanStartY(0.0)
    , m_dblPanEndX(1.0)
    , m_dblPanEndY(1.0)
    , m_dblZoomStart(1.0)
    , m_dblZoomEnd(1.0)
    , m_dblRotationDegrees(0.0)
{
}

ImageClip::~ImageClip()
{
}

MediaItemType ImageClip::GetItemType() const throw()
{
    return MediaItemTypeImage;
}

double ImageClip::GetPanStartX() const throw()
{
    return m_dblPanStartX;
}

double ImageClip::GetPanStartY() const throw()
{
    return m_dblPanStartY;
}

double ImageClip::GetPanEndX() const throw()
{
    return m_dblPanEndX;
}

double ImageClip::GetPanEndY() const throw()
{
    return m_dblPanEndY;
}

void ImageClip::SetPanPoints(double dblStartX, double dblStartY,
                              double dblEndX, double dblEndY) throw()
{
    m_dblPanStartX = dblStartX;
    m_dblPanStartY = dblStartY;
    m_dblPanEndX = dblEndX;
    m_dblPanEndY = dblEndY;
}

double ImageClip::GetZoomStart() const throw()
{
    return m_dblZoomStart;
}

double ImageClip::GetZoomEnd() const throw()
{
    return m_dblZoomEnd;
}

void ImageClip::SetZoomRange(double dblStart, double dblEnd) throw()
{
    m_dblZoomStart = dblStart;
    m_dblZoomEnd = dblEnd;
}

double ImageClip::GetRotationDegrees() const throw()
{
    return m_dblRotationDegrees;
}

void ImageClip::SetRotationDegrees(double dblDegrees) throw()
{
    m_dblRotationDegrees = dblDegrees;
}

MediaItemBase* ImageClip::Clone() const
{
    ImageClip* pClone = new ImageClip();
    pClone->SetItemId(GetItemId());
    pClone->SetSourcePath(GetSourcePath());
    pClone->SetDisplayName(GetDisplayName());
    pClone->SetDurationHns(GetDurationHns());
    pClone->SetMetadata(GetMetadata());
    pClone->SetProxyInfo(GetProxyInfo());
    pClone->SetTrimStartHns(GetTrimStartHns());
    pClone->SetTrimEndHns(GetTrimEndHns());
    pClone->SetSpeedFactor(GetSpeedFactor());
    pClone->SetVolume(GetVolume());
    pClone->SetPan(GetPan());
    pClone->SetMuted(IsMuted());
    pClone->SetReversed(IsReversed());
    pClone->SetFadeInDurationHns(GetFadeInDurationHns());
    pClone->SetFadeOutDurationHns(GetFadeOutDurationHns());
    pClone->SetPanPoints(GetPanStartX(), GetPanStartY(), GetPanEndX(), GetPanEndY());
    pClone->SetZoomRange(GetZoomStart(), GetZoomEnd());
    pClone->SetRotationDegrees(GetRotationDegrees());
    return pClone;
}

// ============================================================================
// AudioClip implementation
// ============================================================================

AudioClip::AudioClip()
    : m_llAudioFadeInHns(0)
    , m_llAudioFadeOutHns(0)
    , m_fNormalizeEnabled(false)
{
}

AudioClip::~AudioClip()
{
}

MediaItemType AudioClip::GetItemType() const throw()
{
    return MediaItemTypeAudio;
}

LONGLONG AudioClip::GetAudioFadeInHns() const throw()
{
    return m_llAudioFadeInHns;
}

void AudioClip::SetAudioFadeInHns(LONGLONG llDuration) throw()
{
    m_llAudioFadeInHns = llDuration;
}

LONGLONG AudioClip::GetAudioFadeOutHns() const throw()
{
    return m_llAudioFadeOutHns;
}

void AudioClip::SetAudioFadeOutHns(LONGLONG llDuration) throw()
{
    m_llAudioFadeOutHns = llDuration;
}

bool AudioClip::IsNormalizeEnabled() const throw()
{
    return m_fNormalizeEnabled;
}

void AudioClip::SetNormalizeEnabled(bool fEnabled) throw()
{
    m_fNormalizeEnabled = fEnabled;
}

MediaItemBase* AudioClip::Clone() const
{
    AudioClip* pClone = new AudioClip();
    pClone->SetItemId(GetItemId());
    pClone->SetSourcePath(GetSourcePath());
    pClone->SetDisplayName(GetDisplayName());
    pClone->SetDurationHns(GetDurationHns());
    pClone->SetMetadata(GetMetadata());
    pClone->SetProxyInfo(GetProxyInfo());
    pClone->SetTrimStartHns(GetTrimStartHns());
    pClone->SetTrimEndHns(GetTrimEndHns());
    pClone->SetSpeedFactor(GetSpeedFactor());
    pClone->SetVolume(GetVolume());
    pClone->SetPan(GetPan());
    pClone->SetMuted(IsMuted());
    pClone->SetReversed(IsReversed());
    pClone->SetFadeInDurationHns(GetFadeInDurationHns());
    pClone->SetFadeOutDurationHns(GetFadeOutDurationHns());
    pClone->SetAudioFadeInHns(GetAudioFadeInHns());
    pClone->SetAudioFadeOutHns(GetAudioFadeOutHns());
    pClone->SetNormalizeEnabled(IsNormalizeEnabled());
    return pClone;
}

// ============================================================================
// TitleClip implementation
// ============================================================================

TitleClip::TitleClip()
    : m_flFontSize(24.0f)
    , m_dwFontColor(0xFFFFFFFF)
    , m_fBold(false)
    , m_fItalic(false)
    , m_dwBackgroundColor(0x00000000)
    , m_dblPositionX(0.5)
    , m_dblPositionY(0.5)
    , m_dwAnimationId(0)
    , m_llTitleDurationHns(30000000)
{
}

TitleClip::~TitleClip()
{
}

MediaItemType TitleClip::GetItemType() const throw()
{
    return MediaItemTypeTitle;
}

ATL::CString TitleClip::GetTitleText() const
{
    return m_strTitleText;
}

void TitleClip::SetTitleText(LPCWSTR pszText)
{
    m_strTitleText = pszText ? pszText : L"";
}

ATL::CString TitleClip::GetFontFamily() const
{
    return m_strFontFamily;
}

void TitleClip::SetFontFamily(LPCWSTR pszFontFamily)
{
    m_strFontFamily = pszFontFamily ? pszFontFamily : L"Segoe UI";
}

float TitleClip::GetFontSize() const throw()
{
    return m_flFontSize;
}

void TitleClip::SetFontSize(float flSize) throw()
{
    m_flFontSize = flSize;
}

DWORD TitleClip::GetFontColor() const throw()
{
    return m_dwFontColor;
}

void TitleClip::SetFontColor(DWORD dwColor) throw()
{
    m_dwFontColor = dwColor;
}

bool TitleClip::IsBold() const throw()
{
    return m_fBold;
}

void TitleClip::SetBold(bool fBold) throw()
{
    m_fBold = fBold;
}

bool TitleClip::IsItalic() const throw()
{
    return m_fItalic;
}

void TitleClip::SetItalic(bool fItalic) throw()
{
    m_fItalic = fItalic;
}

DWORD TitleClip::GetBackgroundColor() const throw()
{
    return m_dwBackgroundColor;
}

void TitleClip::SetBackgroundColor(DWORD dwColor) throw()
{
    m_dwBackgroundColor = dwColor;
}

double TitleClip::GetPositionX() const throw()
{
    return m_dblPositionX;
}

double TitleClip::GetPositionY() const throw()
{
    return m_dblPositionY;
}

void TitleClip::SetPosition(double dblX, double dblY) throw()
{
    m_dblPositionX = dblX;
    m_dblPositionY = dblY;
}

DWORD TitleClip::GetAnimationId() const throw()
{
    return m_dwAnimationId;
}

void TitleClip::SetAnimationId(DWORD dwAnimId) throw()
{
    m_dwAnimationId = dwAnimId;
}

LONGLONG TitleClip::GetTitleDurationHns() const throw()
{
    return m_llTitleDurationHns;
}

void TitleClip::SetTitleDurationHns(LONGLONG llDuration) throw()
{
    m_llTitleDurationHns = llDuration;
}

MediaItemBase* TitleClip::Clone() const
{
    TitleClip* pClone = new TitleClip();
    pClone->SetItemId(GetItemId());
    pClone->SetSourcePath(GetSourcePath());
    pClone->SetDisplayName(GetDisplayName());
    pClone->SetDurationHns(GetDurationHns());
    pClone->SetMetadata(GetMetadata());
    pClone->SetProxyInfo(GetProxyInfo());
    pClone->SetTitleText(GetTitleText());
    pClone->SetFontFamily(GetFontFamily());
    pClone->SetFontSize(GetFontSize());
    pClone->SetFontColor(GetFontColor());
    pClone->SetBold(IsBold());
    pClone->SetItalic(IsItalic());
    pClone->SetBackgroundColor(GetBackgroundColor());
    pClone->SetPosition(GetPositionX(), GetPositionY());
    pClone->SetAnimationId(GetAnimationId());
    pClone->SetTitleDurationHns(GetTitleDurationHns());
    return pClone;
}

} // namespace StoryboardManager
