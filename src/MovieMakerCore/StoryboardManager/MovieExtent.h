/*
 * MovieExtent.h
 *
 * MovieExtent class - represents a single segment (extent) on the timeline.
 * Each extent corresponds to a portion of a media file with start/end trim
 * points, and may have associated effects, transitions, and text overlays.
 *
 * Extents are the fundamental building blocks of the project timeline. They
 * reference media items by ID and define the in/out points used during
 * playback and export.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef MOVIEEXTENT_H
#define MOVIEEXTENT_H

#include "StoryboardManager.h"

namespace StoryboardManager
{

// ============================================================================
// MovieExtent
// ============================================================================
// A single extent on the timeline. Represents a trimmed segment of a media
// file with associated metadata. Extents are referenced by ExtentId within
// the ProjectTimeline and may be shared across tracks (e.g., audio from a
// video clip used on the audio track).
//
class STORYBOARD_API MovieExtent
{
public:
    MovieExtent();
    MovieExtent(DWORD dwId, DWORD dwMediaId);
    ~MovieExtent();

    // Identity
    DWORD GetExtentId() const throw();
    void SetExtentId(DWORD dwId) throw();

    // Media reference
    DWORD GetMediaId() const throw();
    void SetMediaId(DWORD dwMediaId) throw();

    // Trim points (in hundred-nanosecond units)
    LONGLONG GetStartTimeHns() const throw();
    void SetStartTimeHns(LONGLONG llStart) throw();

    LONGLONG GetEndTimeHns() const throw();
    void SetEndTimeHns(LONGLONG llEnd) throw();

    // Computed duration
    LONGLONG GetDurationHns() const throw();

    // Speed factor (1.0 = normal, 2.0 = double speed, 0.5 = half speed)
    double GetSpeedFactor() const throw();
    void SetSpeedFactor(double dblSpeed) throw();

    // Volume (0.0 = silent, 1.0 = full)
    double GetVolume() const throw();
    void SetVolume(double dblVolume) throw();

    // Fade in/out (hundred-nanoseconds)
    LONGLONG GetFadeInDurationHns() const throw();
    void SetFadeInDurationHns(LONGLONG llDuration) throw();

    LONGLONG GetFadeOutDurationHns() const throw();
    void SetFadeOutDurationHns(LONGLONG llDuration) throw();

    // Pan position (-1.0 = full left, 0.0 = center, 1.0 = full right)
    double GetPan() const throw();
    void SetPan(double dblPan) throw();

    // Reversed playback
    bool IsReversed() const throw();
    void SetReversed(bool fReversed) throw();

    // Muted
    bool IsMuted() const throw();
    void SetMuted(bool fMuted) throw();

    // Transcode state
    ExtentTranscodeState GetTranscodeState() const throw();
    void SetTranscodeState(ExtentTranscodeState state) throw();
    HRESULT GetTranscodeHResult() const throw();
    void SetTranscodeHResult(HRESULT hr) throw();

    // Title text (for title/credits extents)
    ATL::CString GetTitleText() const;
    void SetTitleText(LPCWSTR pszText);

    // Title font properties
    ATL::CString GetTitleFontFamily() const;
    void SetTitleFontFamily(LPCWSTR pszFontFamily);

    float GetTitleFontSize() const throw();
    void SetTitleFontSize(float flSize) throw();

    DWORD GetTitleFontColor() const throw();
    void SetTitleFontColor(DWORD dwColor) throw();

    bool IsTitleBold() const throw();
    void SetTitleBold(bool fBold) throw();

    bool IsTitleItalic() const throw();
    void SetTitleItalic(bool fItalic) throw();

    // Animation (title entrance/exit)
    DWORD GetTitleAnimationId() const throw();
    void SetTitleAnimationId(DWORD dwAnimId) throw();

    // Snapshot thumbnail (for storyboard grid view)
    bool HasSnapshot() const throw();
    Gdiplus::Bitmap* GetSnapshot() const;
    void SetSnapshot(Gdiplus::Bitmap* pBitmap);
    void ClearSnapshot();

    // Validity
    bool IsValid() const throw();

    // Clone
    MovieExtent* Clone() const;

    // Comparison
    bool operator==(const MovieExtent& other) const;
    bool operator!=(const MovieExtent& other) const;
    bool operator<(const MovieExtent& other) const;

private:
    DWORD               m_dwExtentId;
    DWORD               m_dwMediaId;
    LONGLONG            m_llStartTimeHns;
    LONGLONG            m_llEndTimeHns;
    double              m_dblSpeedFactor;
    double              m_dblVolume;
    LONGLONG            m_llFadeInDurationHns;
    LONGLONG            m_llFadeOutDurationHns;
    double              m_dblPan;
    bool                m_fReversed;
    bool                m_fMuted;
    ExtentTranscodeState m_transcodeState;
    HRESULT             m_hresult;

    // Title overlay properties
    ATL::CString        m_strTitleText;
    ATL::CString        m_strTitleFontFamily;
    float               m_flTitleFontSize;
    DWORD               m_dwTitleFontColor;
    bool                m_fTitleBold;
    bool                m_fTitleItalic;
    DWORD               m_dwTitleAnimationId;

    // Snapshot bitmap (owned)
    Gdiplus::Bitmap*    m_pSnapshot;
};

} // namespace StoryboardManager

#endif // MOVIEEXTENT_H
