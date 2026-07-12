/*
 * MovieExtent.cpp
 *
 * Implementation of the MovieExtent class - timeline extent representation
 * with media references, trim points, transcode state, and title overlay
 * properties.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "MovieExtent.h"

namespace StoryboardManager
{

// ============================================================================
// MovieExtent implementation
// ============================================================================

MovieExtent::MovieExtent()
    : m_dwExtentId(0)
    , m_dwMediaId(0)
    , m_llStartTimeHns(0)
    , m_llEndTimeHns(0)
    , m_dblSpeedFactor(1.0)
    , m_dblVolume(1.0)
    , m_llFadeInDurationHns(0)
    , m_llFadeOutDurationHns(0)
    , m_dblPan(0.0)
    , m_fReversed(false)
    , m_fMuted(false)
    , m_transcodeState(ExtentTranscodeStateNone)
    , m_hresult(S_OK)
    , m_flTitleFontSize(24.0f)
    , m_dwTitleFontColor(0xFFFFFFFF) // white
    , m_fTitleBold(false)
    , m_fTitleItalic(false)
    , m_dwTitleAnimationId(0)
    , m_pSnapshot(nullptr)
{
}

MovieExtent::MovieExtent(DWORD dwId, DWORD dwMediaId)
    : m_dwExtentId(dwId)
    , m_dwMediaId(dwMediaId)
    , m_llStartTimeHns(0)
    , m_llEndTimeHns(0)
    , m_dblSpeedFactor(1.0)
    , m_dblVolume(1.0)
    , m_llFadeInDurationHns(0)
    , m_llFadeOutDurationHns(0)
    , m_dblPan(0.0)
    , m_fReversed(false)
    , m_fMuted(false)
    , m_transcodeState(ExtentTranscodeStateNone)
    , m_hresult(S_OK)
    , m_flTitleFontSize(24.0f)
    , m_dwTitleFontColor(0xFFFFFFFF)
    , m_fTitleBold(false)
    , m_fTitleItalic(false)
    , m_dwTitleAnimationId(0)
    , m_pSnapshot(nullptr)
{
}

MovieExtent::~MovieExtent()
{
    ClearSnapshot();
}

// ============================================================================
// Identity
// ============================================================================

DWORD MovieExtent::GetExtentId() const throw()
{
    return m_dwExtentId;
}

void MovieExtent::SetExtentId(DWORD dwId) throw()
{
    m_dwExtentId = dwId;
}

// ============================================================================
// Media reference
// ============================================================================

DWORD MovieExtent::GetMediaId() const throw()
{
    return m_dwMediaId;
}

void MovieExtent::SetMediaId(DWORD dwMediaId) throw()
{
    m_dwMediaId = dwMediaId;
}

// ============================================================================
// Trim points
// ============================================================================

LONGLONG MovieExtent::GetStartTimeHns() const throw()
{
    return m_llStartTimeHns;
}

void MovieExtent::SetStartTimeHns(LONGLONG llStart) throw()
{
    m_llStartTimeHns = llStart;
}

LONGLONG MovieExtent::GetEndTimeHns() const throw()
{
    return m_llEndTimeHns;
}

void MovieExtent::SetEndTimeHns(LONGLONG llEnd) throw()
{
    m_llEndTimeHns = llEnd;
}

LONGLONG MovieExtent::GetDurationHns() const throw()
{
    if (m_llEndTimeHns <= m_llStartTimeHns)
        return 0;
    return m_llEndTimeHns - m_llStartTimeHns;
}

// ============================================================================
// Speed / volume / pan
// ============================================================================

double MovieExtent::GetSpeedFactor() const throw()
{
    return m_dblSpeedFactor;
}

void MovieExtent::SetSpeedFactor(double dblSpeed) throw()
{
    // Clamp to reasonable range
    if (dblSpeed < 0.25) dblSpeed = 0.25;
    if (dblSpeed > 4.0)  dblSpeed = 4.0;
    m_dblSpeedFactor = dblSpeed;
}

double MovieExtent::GetVolume() const throw()
{
    return m_dblVolume;
}

void MovieExtent::SetVolume(double dblVolume) throw()
{
    if (dblVolume < 0.0) dblVolume = 0.0;
    if (dblVolume > 1.0) dblVolume = 1.0;
    m_dblVolume = dblVolume;
}

// ============================================================================
// Fades
// ============================================================================

LONGLONG MovieExtent::GetFadeInDurationHns() const throw()
{
    return m_llFadeInDurationHns;
}

void MovieExtent::SetFadeInDurationHns(LONGLONG llDuration) throw()
{
    m_llFadeInDurationHns = llDuration;
}

LONGLONG MovieExtent::GetFadeOutDurationHns() const throw()
{
    return m_llFadeOutDurationHns;
}

void MovieExtent::SetFadeOutDurationHns(LONGLONG llDuration) throw()
{
    m_llFadeOutDurationHns = llDuration;
}

// ============================================================================
// Pan
// ============================================================================

double MovieExtent::GetPan() const throw()
{
    return m_dblPan;
}

void MovieExtent::SetPan(double dblPan) throw()
{
    if (dblPan < -1.0) dblPan = -1.0;
    if (dblPan > 1.0)  dblPan = 1.0;
    m_dblPan = dblPan;
}

// ============================================================================
// Reversed / muted
// ============================================================================

bool MovieExtent::IsReversed() const throw()
{
    return m_fReversed;
}

void MovieExtent::SetReversed(bool fReversed) throw()
{
    m_fReversed = fReversed;
}

bool MovieExtent::IsMuted() const throw()
{
    return m_fMuted;
}

void MovieExtent::SetMuted(bool fMuted) throw()
{
    m_fMuted = fMuted;
}

// ============================================================================
// Transcode state
// ============================================================================

ExtentTranscodeState MovieExtent::GetTranscodeState() const throw()
{
    return m_transcodeState;
}

void MovieExtent::SetTranscodeState(ExtentTranscodeState state) throw()
{
    m_transcodeState = state;
}

HRESULT MovieExtent::GetTranscodeHResult() const throw()
{
    return m_hresult;
}

void MovieExtent::SetTranscodeHResult(HRESULT hr) throw()
{
    m_hresult = hr;
}

// ============================================================================
// Title overlay properties
// ============================================================================

ATL::CString MovieExtent::GetTitleText() const
{
    return m_strTitleText;
}

void MovieExtent::SetTitleText(LPCWSTR pszText)
{
    m_strTitleText = pszText ? pszText : L"";
}

ATL::CString MovieExtent::GetTitleFontFamily() const
{
    return m_strTitleFontFamily;
}

void MovieExtent::SetTitleFontFamily(LPCWSTR pszFontFamily)
{
    m_strTitleFontFamily = pszFontFamily ? pszFontFamily : L"Segoe UI";
}

float MovieExtent::GetTitleFontSize() const throw()
{
    return m_flTitleFontSize;
}

void MovieExtent::SetTitleFontSize(float flSize) throw()
{
    m_flTitleFontSize = flSize;
}

DWORD MovieExtent::GetTitleFontColor() const throw()
{
    return m_dwTitleFontColor;
}

void MovieExtent::SetTitleFontColor(DWORD dwColor) throw()
{
    m_dwTitleFontColor = dwColor;
}

bool MovieExtent::IsTitleBold() const throw()
{
    return m_fTitleBold;
}

void MovieExtent::SetTitleBold(bool fBold) throw()
{
    m_fTitleBold = fBold;
}

bool MovieExtent::IsTitleItalic() const throw()
{
    return m_fTitleItalic;
}

void MovieExtent::SetTitleItalic(bool fItalic) throw()
{
    m_fTitleItalic = fItalic;
}

DWORD MovieExtent::GetTitleAnimationId() const throw()
{
    return m_dwTitleAnimationId;
}

void MovieExtent::SetTitleAnimationId(DWORD dwAnimId) throw()
{
    m_dwTitleAnimationId = dwAnimId;
}

// ============================================================================
// Snapshot
// ============================================================================

bool MovieExtent::HasSnapshot() const throw()
{
    return m_pSnapshot != nullptr;
}

Gdiplus::Bitmap* MovieExtent::GetSnapshot() const
{
    return m_pSnapshot;
}

void MovieExtent::SetSnapshot(Gdiplus::Bitmap* pBitmap)
{
    ClearSnapshot();
    // Clone the bitmap so we own it
    if (pBitmap)
    {
        m_pSnapshot = pBitmap->Clone(0, 0, pBitmap->GetWidth(), pBitmap->GetHeight(),
                                      pBitmap->GetPixelFormat());
    }
}

void MovieExtent::ClearSnapshot()
{
    if (m_pSnapshot)
    {
        delete m_pSnapshot;
        m_pSnapshot = nullptr;
    }
}

// ============================================================================
// Validity
// ============================================================================

bool MovieExtent::IsValid() const throw()
{
    return m_dwExtentId != 0 && m_dwMediaId != 0 && GetDurationHns() > 0;
}

// ============================================================================
// Clone
// ============================================================================

MovieExtent* MovieExtent::Clone() const
{
    MovieExtent* pClone = new MovieExtent(m_dwExtentId, m_dwMediaId);

    pClone->m_llStartTimeHns = m_llStartTimeHns;
    pClone->m_llEndTimeHns = m_llEndTimeHns;
    pClone->m_dblSpeedFactor = m_dblSpeedFactor;
    pClone->m_dblVolume = m_dblVolume;
    pClone->m_llFadeInDurationHns = m_llFadeInDurationHns;
    pClone->m_llFadeOutDurationHns = m_llFadeOutDurationHns;
    pClone->m_dblPan = m_dblPan;
    pClone->m_fReversed = m_fReversed;
    pClone->m_fMuted = m_fMuted;
    pClone->m_transcodeState = m_transcodeState;
    pClone->m_hresult = m_hresult;

    pClone->m_strTitleText = m_strTitleText;
    pClone->m_strTitleFontFamily = m_strTitleFontFamily;
    pClone->m_flTitleFontSize = m_flTitleFontSize;
    pClone->m_dwTitleFontColor = m_dwTitleFontColor;
    pClone->m_fTitleBold = m_fTitleBold;
    pClone->m_fTitleItalic = m_fTitleItalic;
    pClone->m_dwTitleAnimationId = m_dwTitleAnimationId;

    if (m_pSnapshot)
    {
        pClone->SetSnapshot(m_pSnapshot);
    }

    return pClone;
}

// ============================================================================
// Comparison operators
// ============================================================================

bool MovieExtent::operator==(const MovieExtent& other) const
{
    return m_dwExtentId == other.m_dwExtentId;
}

bool MovieExtent::operator!=(const MovieExtent& other) const
{
    return !(*this == other);
}

bool MovieExtent::operator<(const MovieExtent& other) const
{
    return m_llStartTimeHns < other.m_llStartTimeHns;
}

} // namespace StoryboardManager
