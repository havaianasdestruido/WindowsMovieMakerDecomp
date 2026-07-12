/*
 * MediaItem.h
 *
 * Media item hierarchy for timeline content. Defines the base class
 * and derived types for video, image, audio, and title clips.
 *
 * RTTI classes from analysis:
 *   ?AVMetadata@@
 *   ?AVMediaItemBase@@
 *   ?AVMediaItem@@
 *   ?AVMediaClipBase@@
 *   ?AVAudioVideoMediaClip@@
 *   ?AVVideoClip@@
 *   ?AVImageClip@@
 *   ?AVAudioClip@@
 *   ?AVTitleClip@@
 *   ?AVProxyInfo@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef MEDIAITEM_H
#define MEDIAITEM_H

#include "../StoryboardManager.h"

namespace StoryboardManager
{

// ============================================================================
// MediaItemType
// ============================================================================
enum MediaItemType
{
    MediaItemTypeUnknown  = 0,
    MediaItemTypeVideo    = 1,
    MediaItemTypeImage    = 2,
    MediaItemTypeAudio    = 3,
    MediaItemTypeTitle    = 4
};

// ============================================================================
// Metadata
// ============================================================================
// Encapsulates intrinsic media metadata: duration, resolution, codec info,
// frame rate, audio properties, and date/time. Populated during import
// from file properties or Media Foundation attributes.
//
class STORYBOARD_API Metadata
{
public:
    Metadata();
    ~Metadata();

    // -- Duration --
    LONGLONG GetDurationHns() const throw();
    void SetDurationHns(LONGLONG llDuration) throw();

    // -- Dimensions --
    UINT GetWidth() const throw();
    UINT GetHeight() const throw();
    void SetDimensions(UINT cx, UINT cy) throw();

    // -- Frame rate --
    DWORD GetFrameRate() const throw();
    void SetFrameRate(DWORD dwFrameRate) throw();

    // -- Video codec --
    ATL::CString GetVideoCodec() const;
    void SetVideoCodec(LPCWSTR pszCodec);

    // -- Video bit rate --
    DWORD GetVideoBitRate() const throw();
    void SetVideoBitRate(DWORD dwBitRate) throw();

    // -- Audio codec --
    ATL::CString GetAudioCodec() const;
    void SetAudioCodec(LPCWSTR pszCodec);

    // -- Audio sample rate --
    DWORD GetAudioSampleRate() const throw();
    void SetAudioSampleRate(DWORD dwSampleRate) throw();

    // -- Audio channels --
    DWORD GetAudioChannels() const throw();
    void SetAudioChannels(DWORD dwChannels) throw();

    // -- Audio bit rate --
    DWORD GetAudioBitRate() const throw();
    void SetAudioBitRate(DWORD dwBitRate) throw();

    // -- Date taken --
    FILETIME GetDateTaken() const throw();
    void SetDateTaken(const FILETIME& ft) throw();

    // -- Rating --
    UINT GetRating() const throw();
    void SetRating(UINT uRating) throw();

    // -- Tags --
    ATL::CString GetTags() const;
    void SetTags(LPCWSTR pszTags);

    // -- Title (embedded metadata) --
    ATL::CString GetTitle() const;
    void SetTitle(LPCWSTR pszTitle);

    // -- Subtitle --
    ATL::CString GetSubtitle() const;
    void SetSubtitle(LPCWSTR pszSubtitle);

    // -- Artist --
    ATL::CString GetArtist() const;
    void SetArtist(LPCWSTR pszArtist);

    // -- Album --
    ATL::CString GetAlbum() const;
    void SetAlbum(LPCWSTR pszAlbum);

    // -- Genre --
    ATL::CString GetGenre() const;
    void SetGenre(LPCWSTR pszGenre);

private:
    LONGLONG    m_llDurationHns;
    UINT        m_uWidth;
    UINT        m_uHeight;
    DWORD       m_dwFrameRate;
    ATL::CString m_strVideoCodec;
    DWORD       m_dwVideoBitRate;
    ATL::CString m_strAudioCodec;
    DWORD       m_dwAudioSampleRate;
    DWORD       m_dwAudioChannels;
    DWORD       m_dwAudioBitRate;
    FILETIME    m_ftDateTaken;
    UINT        m_uRating;
    ATL::CString m_strTags;
    ATL::CString m_strTitle;
    ATL::CString m_strSubtitle;
    ATL::CString m_strArtist;
    ATL::CString m_strAlbum;
    ATL::CString m_strGenre;
};

// ============================================================================
// ProxyInfo
// ============================================================================
// Proxy transcoding metadata. Stores the state and path of a transcoded
// proxy file used for smooth editing of high-resolution source media.
//
class STORYBOARD_API ProxyInfo
{
public:
    ProxyInfo();
    ~ProxyInfo();

    // -- Proxy file path --
    ATL::CString GetProxyPath() const;
    void SetProxyPath(LPCWSTR pszPath);

    // -- Proxy resolution --
    UINT GetProxyWidth() const throw();
    UINT GetProxyHeight() const throw();
    void SetProxyDimensions(UINT cx, UINT cy) throw();

    // -- Transcode state --
    DWORD GetTranscodeState() const throw();
    void SetTranscodeState(DWORD dwState) throw();

    // -- Source hash (for cache validation) --
    ATL::CString GetSourceHash() const;
    void SetSourceHash(LPCWSTR pszHash);

    // -- Validity --
    bool IsValid() const throw();
    bool ProxyFileExists() const;

private:
    ATL::CString m_strProxyPath;
    UINT        m_uProxyWidth;
    UINT        m_uProxyHeight;
    DWORD       m_dwTranscodeState;
    ATL::CString m_strSourceHash;
};

// ============================================================================
// MediaItemBase
// ============================================================================
// Base class for all media items. Provides common identity, path, duration,
// metadata, and proxy info shared across all clip types.
//
class STORYBOARD_API MediaItemBase
{
public:
    MediaItemBase();
    virtual ~MediaItemBase();

    // -- Identity --
    DWORD GetItemId() const throw();
    void SetItemId(DWORD dwId) throw();

    // -- Item type --
    virtual MediaItemType GetItemType() const throw();

    // -- Source file path --
    ATL::CString GetSourcePath() const;
    void SetSourcePath(LPCWSTR pszPath);

    // -- Display name --
    ATL::CString GetDisplayName() const;
    void SetDisplayName(LPCWSTR pszName);

    // -- Duration --
    LONGLONG GetDurationHns() const throw();
    void SetDurationHns(LONGLONG llDuration) throw();

    // -- Metadata --
    const Metadata& GetMetadata() const;
    Metadata& GetMetadata();
    void SetMetadata(const Metadata& metadata);

    // -- Proxy --
    const ProxyInfo& GetProxyInfo() const;
    ProxyInfo& GetProxyInfo();
    void SetProxyInfo(const ProxyInfo& proxyInfo);

    // -- Validity --
    virtual bool IsValid() const;
    virtual bool FileExists() const;

    // -- Clone --
    virtual MediaItemBase* Clone() const;

private:
    DWORD       m_dwItemId;
    ATL::CString m_strSourcePath;
    ATL::CString m_strDisplayName;
    LONGLONG    m_llDurationHns;
    Metadata    m_metadata;
    ProxyInfo   m_proxyInfo;
};

// ============================================================================
// MediaItem
// ============================================================================
// Concrete media item representing an imported file (video, image, or audio).
// Thin wrapper that adds media-type classification to MediaItemBase.
//
class STORYBOARD_API MediaItem : public MediaItemBase
{
public:
    MediaItem();
    virtual ~MediaItem();

    virtual MediaItemType GetItemType() const throw() override;
    void SetItemType(MediaItemType type) throw();

    // -- Thumbnail path --
    ATL::CString GetThumbnailPath() const;
    void SetThumbnailPath(LPCWSTR pszPath);

    // -- File size --
    ULONGLONG GetFileSize() const throw();
    void SetFileSize(ULONGLONG cbSize) throw();

    // -- Clone --
    virtual MediaItemBase* Clone() const override;

private:
    MediaItemType m_itemType;
    ATL::CString  m_strThumbnailPath;
    ULONGLONG     m_cbFileSize;
};

// ============================================================================
// MediaClipBase
// ============================================================================
// Base class for timeline clips that reference a MediaItem. Adds trim
// points (start/end) and playback parameters (speed, volume, pan).
//
class STORYBOARD_API MediaClipBase : public MediaItemBase
{
public:
    MediaClipBase();
    virtual ~MediaClipBase();

    // -- Trim points (in hundred-nanoseconds) --
    LONGLONG GetTrimStartHns() const throw();
    void SetTrimStartHns(LONGLONG llStart) throw();

    LONGLONG GetTrimEndHns() const throw();
    void SetTrimEndHns(LONGLONG llEnd) throw();

    // -- Computed clip duration --
    LONGLONG GetClipDurationHns() const throw();

    // -- Speed factor --
    double GetSpeedFactor() const throw();
    void SetSpeedFactor(double dblSpeed) throw();

    // -- Volume (0.0 - 1.0) --
    double GetVolume() const throw();
    void SetVolume(double dblVolume) throw();

    // -- Pan (-1.0 left, 0.0 center, 1.0 right) --
    double GetPan() const throw();
    void SetPan(double dblPan) throw();

    // -- Muted --
    bool IsMuted() const throw();
    void SetMuted(bool fMuted) throw();

    // -- Reversed --
    bool IsReversed() const throw();
    void SetReversed(bool fReversed) throw();

    // -- Fade in/out --
    LONGLONG GetFadeInDurationHns() const throw();
    void SetFadeInDurationHns(LONGLONG llDuration) throw();

    LONGLONG GetFadeOutDurationHns() const throw();
    void SetFadeOutDurationHns(LONGLONG llDuration) throw();

private:
    LONGLONG    m_llTrimStartHns;
    LONGLONG    m_llTrimEndHns;
    double      m_dblSpeedFactor;
    double      m_dblVolume;
    double      m_dblPan;
    bool        m_fMuted;
    bool        m_fReversed;
    LONGLONG    m_llFadeInDurationHns;
    LONGLONG    m_llFadeOutDurationHns;
};

// ============================================================================
// AudioVideoMediaClip
// ============================================================================
// A/V clip with both video and audio stream information. Extends
// MediaClipBase with audio stream selection and video stream index.
//
class STORYBOARD_API AudioVideoMediaClip : public MediaClipBase
{
public:
    AudioVideoMediaClip();
    virtual ~AudioVideoMediaClip();

    virtual MediaItemType GetItemType() const throw() override;

    // -- Video stream index --
    DWORD GetVideoStreamIndex() const throw();
    void SetVideoStreamIndex(DWORD dwIndex) throw();

    // -- Audio stream index --
    DWORD GetAudioStreamIndex() const throw();
    void SetAudioStreamIndex(DWORD dwIndex) throw();

    // -- Has audio stream --
    bool HasAudioStream() const throw();
    void SetHasAudioStream(bool fHas) throw();

    // -- Clone --
    virtual MediaItemBase* Clone() const override;

private:
    DWORD m_dwVideoStreamIndex;
    DWORD m_dwAudioStreamIndex;
    bool  m_fHasAudioStream;
};

// ============================================================================
// VideoClip
// ============================================================================
// Video-specific clip. Adds video rendering properties to AudioVideoMediaClip.
//
class STORYBOARD_API VideoClip : public AudioVideoMediaClip
{
public:
    VideoClip();
    virtual ~VideoClip();

    virtual MediaItemType GetItemType() const throw() override;

    // -- Video effect IDs --
    size_t GetEffectCount() const throw();
    DWORD GetEffectAt(size_t nIndex) const;
    void AddEffect(DWORD dwEffectId);
    void RemoveAllEffects();

    // -- Clone --
    virtual MediaItemBase* Clone() const override;

private:
    ATL::CAtlArray<DWORD> m_arrEffectIds;
};

// ============================================================================
// ImageClip
// ============================================================================
// Photo/image clip with Ken Burns pan/zoom parameters.
//
class STORYBOARD_API ImageClip : public MediaClipBase
{
public:
    ImageClip();
    virtual ~ImageClip();

    virtual MediaItemType GetItemType() const throw() override;

    // -- Pan start/end (normalized 0.0 - 1.0) --
    double GetPanStartX() const throw();
    double GetPanStartY() const throw();
    double GetPanEndX() const throw();
    double GetPanEndY() const throw();
    void SetPanPoints(double dblStartX, double dblStartY,
                      double dblEndX, double dblEndY) throw();

    // -- Zoom start/end --
    double GetZoomStart() const throw();
    double GetZoomEnd() const throw();
    void SetZoomRange(double dblStart, double dblEnd) throw();

    // -- Rotation angle (degrees) --
    double GetRotationDegrees() const throw();
    void SetRotationDegrees(double dblDegrees) throw();

    // -- Clone --
    virtual MediaItemBase* Clone() const override;

private:
    double m_dblPanStartX;
    double m_dblPanStartY;
    double m_dblPanEndX;
    double m_dblPanEndY;
    double m_dblZoomStart;
    double m_dblZoomEnd;
    double m_dblRotationDegrees;
};

// ============================================================================
// AudioClip
// ============================================================================
// Audio-only clip for background music and narration tracks.
//
class STORYBOARD_API AudioClip : public MediaClipBase
{
public:
    AudioClip();
    virtual ~AudioClip();

    virtual MediaItemType GetItemType() const throw() override;

    // -- Fade in/out (hundred-nanoseconds) --
    LONGLONG GetAudioFadeInHns() const throw();
    void SetAudioFadeInHns(LONGLONG llDuration) throw();

    LONGLONG GetAudioFadeOutHns() const throw();
    void SetAudioFadeOutHns(LONGLONG llDuration) throw();

    // -- Normalize audio --
    bool IsNormalizeEnabled() const throw();
    void SetNormalizeEnabled(bool fEnabled) throw();

    // -- Clone --
    virtual MediaItemBase* Clone() const override;

private:
    LONGLONG m_llAudioFadeInHns;
    LONGLONG m_llAudioFadeOutHns;
    bool     m_fNormalizeEnabled;
};

// ============================================================================
// TitleClip
// ============================================================================
// Text-only clip for titles and credits. Contains text content, font
// properties, and animation type.
//
class STORYBOARD_API TitleClip : public MediaItemBase
{
public:
    TitleClip();
    virtual ~TitleClip();

    virtual MediaItemType GetItemType() const throw() override;

    // -- Title text --
    ATL::CString GetTitleText() const;
    void SetTitleText(LPCWSTR pszText);

    // -- Font family --
    ATL::CString GetFontFamily() const;
    void SetFontFamily(LPCWSTR pszFontFamily);

    // -- Font size (points) --
    float GetFontSize() const throw();
    void SetFontSize(float flSize) throw();

    // -- Font color (ARGB) --
    DWORD GetFontColor() const throw();
    void SetFontColor(DWORD dwColor) throw();

    // -- Bold/Italic --
    bool IsBold() const throw();
    void SetBold(bool fBold) throw();

    bool IsItalic() const throw();
    void SetItalic(bool fItalic) throw();

    // -- Background color (ARGB, 0 = transparent) --
    DWORD GetBackgroundColor() const throw();
    void SetBackgroundColor(DWORD dwColor) throw();

    // -- Position (normalized 0.0 - 1.0) --
    double GetPositionX() const throw();
    double GetPositionY() const throw();
    void SetPosition(double dblX, double dblY) throw();

    // -- Animation ID --
    DWORD GetAnimationId() const throw();
    void SetAnimationId(DWORD dwAnimId) throw();

    // -- Duration --
    LONGLONG GetTitleDurationHns() const throw();
    void SetTitleDurationHns(LONGLONG llDuration) throw();

    // -- Clone --
    virtual MediaItemBase* Clone() const override;

private:
    ATL::CString m_strTitleText;
    ATL::CString m_strFontFamily;
    float       m_flFontSize;
    DWORD       m_dwFontColor;
    bool        m_fBold;
    bool        m_fItalic;
    DWORD       m_dwBackgroundColor;
    double      m_dblPositionX;
    double      m_dblPositionY;
    DWORD       m_dwAnimationId;
    LONGLONG    m_llTitleDurationHns;
};

} // namespace StoryboardManager

#endif // MEDIAITEM_H
