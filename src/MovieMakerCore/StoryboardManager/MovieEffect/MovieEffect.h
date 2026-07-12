/*
 * MovieEffect.h
 *
 * Movie effect definitions for the StoryboardManager namespace.
 * Provides effect types applied to extents on the timeline, including
 * text overlays, pan/zoom shapes, and audio ducking properties.
 *
 * RTTI classes:
 *   ?AVMovieEffect@@, ?AVTextEffect@@, ?AVPanAndZoomShapeEffect@@
 *   ?AVAudioDuckingProperties@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef MOVIEEFFECT_H
#define MOVIEEFFECT_H

#include "../StoryboardManager.h"

namespace StoryboardManager
{

// ============================================================================
// Movie effect type enumeration
// ============================================================================
enum MovieEffectType
{
    MovieEffectTypeNone             = 0,
    MovieEffectTypeVisual           = 1,
    MovieEffectTypeText             = 2,
    MovieEffectTypePanZoom          = 3,
    MovieEffectTypeTransition       = 4,
    MovieEffectTypeAudioDucking     = 5,
    MovieEffectTypeColorAdjust      = 6,
    MovieEffectTypeBlur             = 7,
    MovieEffectTypeSpeed            = 8
};

// ============================================================================
// PanZoomShape
// ============================================================================
enum PanZoomShape
{
    PanZoomShapeNone        = 0,
    PanZoomShapeRect        = 1,
    PanZoomShapeEllipse     = 2,
    PanZoomShapeStar4       = 3,
    PanZoomShapeStar5       = 4,
    PanZoomShapeStar6       = 5,
    PanZoomShapeCustom      = 6
};

// ============================================================================
// MovieEffect
// ============================================================================
// Base class for movie effects applied to timeline extents. Provides
// common properties (type, duration, parameters) and serialization.
//
class STORYBOARD_API MovieEffect
{
public:
    MovieEffect();
    virtual ~MovieEffect();

    // Effect type
    MovieEffectType GetType() const throw();
    void SetType(MovieEffectType type) throw();

    // Effect identity
    DWORD GetEffectId() const throw();
    void SetEffectId(DWORD dwId) throw();

    ATL::CString GetName() const;
    void SetName(LPCWSTR pszName);

    // Duration (hundred-nanoseconds, 0 = full extent duration)
    LONGLONG GetDurationHns() const throw();
    void SetDurationHns(LONGLONG llDuration) throw();

    // Start offset within extent (hundred-nanoseconds)
    LONGLONG GetStartOffsetHns() const throw();
    void SetStartOffsetHns(LONGLONG llOffset) throw();

    // Intensity (0.0-1.0)
    float GetIntensity() const throw();
    void SetIntensity(float flIntensity) throw();

    // Parameters
    size_t GetParameterCount() const throw();
    ATL::CString GetParameter(LPCWSTR pszKey) const;
    void SetParameter(LPCWSTR pszKey, LPCWSTR pszValue);
    void RemoveParameter(LPCWSTR pszKey);

    // Enabled state
    bool IsEnabled() const throw();
    void SetEnabled(bool fEnabled) throw();

    // Serialization
    virtual HRESULT LoadFromXml(IXmlReader* pReader);
    virtual HRESULT SaveToXml(IXmlWriter* pWriter);

private:
    MovieEffectType     m_type;
    DWORD               m_dwEffectId;
    ATL::CString        m_strName;
    LONGLONG            m_llDurationHns;
    LONGLONG            m_llStartOffsetHns;
    float               m_flIntensity;
    bool                m_fEnabled;

    struct EffectParam
    {
        ATL::CString strKey;
        ATL::CString strValue;
    };
    ATL::CAtlArray<EffectParam> m_arrParameters;
};

// ============================================================================
// TextEffect
// ============================================================================
// Text overlay effect applied to an extent. Renders styled text over
// the video/photo at a specified position.
//
class STORYBOARD_API TextEffect : public MovieEffect
{
public:
    TextEffect();
    virtual ~TextEffect();

    // Text content
    ATL::CString GetText() const;
    void SetText(LPCWSTR pszText);

    // Font
    ATL::CString GetFontFamily() const;
    void SetFontFamily(LPCWSTR pszFontFamily);

    float GetFontSize() const throw();
    void SetFontSize(float flSize) throw();

    DWORD GetFontColor() const throw();
    void SetFontColor(DWORD dwColor) throw();

    DWORD GetFontStyle() const throw();
    void SetFontStyle(DWORD dwStyle) throw();

    // Position (normalized 0.0-1.0)
    float GetPositionX() const throw();
    float GetPositionY() const throw();
    void SetPosition(float x, float y) throw();

    // Alignment
    enum TextEffectAlignment
    {
        TextEffectAlignLeft    = 0,
        TextEffectAlignCenter  = 1,
        TextEffectAlignRight   = 2
    };

    TextEffectAlignment GetAlignment() const throw();
    void SetAlignment(TextEffectAlignment align) throw();

    // Background
    bool HasBackground() const throw();
    void SetBackground(bool fBackground) throw();
    DWORD GetBackgroundColor() const throw();
    void SetBackgroundColor(DWORD dwColor) throw();

    // Animation
    DWORD GetAnimationType() const throw();
    void SetAnimationType(DWORD dwType) throw();

    LONGLONG GetAnimationDurationHns() const throw();
    void SetAnimationDurationHns(LONGLONG llDuration) throw();

    virtual HRESULT LoadFromXml(IXmlReader* pReader) override;
    virtual HRESULT SaveToXml(IXmlWriter* pWriter) override;

private:
    ATL::CString        m_strText;
    ATL::CString        m_strFontFamily;
    float               m_flFontSize;
    DWORD               m_dwFontColor;
    DWORD               m_dwFontStyle;
    float               m_flPositionX;
    float               m_flPositionY;
    TextEffectAlignment m_alignment;
    bool                m_fBackground;
    DWORD               m_dwBackgroundColor;
    DWORD               m_dwAnimationType;
    LONGLONG            m_llAnimationDurationHns;
};

// ============================================================================
// PanAndZoomShapeEffect
// ============================================================================
// Ken Burns pan and zoom effect with shape masking. Defines the start/end
// rectangles for the pan/zoom animation and an optional shape mask.
//
class STORYBOARD_API PanAndZoomShapeEffect : public MovieEffect
{
public:
    PanAndZoomShapeEffect();
    virtual ~PanAndZoomShapeEffect();

    // Start rectangle (normalized 0.0-1.0)
    float GetStartLeft() const throw();
    float GetStartTop() const throw();
    float GetStartRight() const throw();
    float GetStartBottom() const throw();
    void SetStartRect(float left, float top, float right, float bottom) throw();

    // End rectangle (normalized 0.0-1.0)
    float GetEndLeft() const throw();
    float GetEndTop() const throw();
    float GetEndRight() const throw();
    float GetEndBottom() const throw();
    void SetEndRect(float left, float top, float right, float bottom) throw();

    // Shape mask
    PanZoomShape GetShape() const throw();
    void SetShape(PanZoomShape shape) throw();

    // Easing (ease-in, ease-out, ease-in-out)
    enum EasingType
    {
        EasingLinear      = 0,
        EasingEaseIn      = 1,
        EasingEaseOut     = 2,
        EasingEaseInOut   = 3
    };

    EasingType GetEasing() const throw();
    void SetEasing(EasingType easing) throw();

    // Random start position flag
    bool IsRandomStartPosition() const throw();
    void SetRandomStartPosition(bool fRandom) throw();

    // Static frame hold at start/end (hundred-nanoseconds)
    LONGLONG GetHoldStartHns() const throw();
    void SetHoldStartHns(LONGLONG llHold) throw();

    LONGLONG GetHoldEndHns() const throw();
    void SetHoldEndHns(LONGLONG llHold) throw();

    virtual HRESULT LoadFromXml(IXmlReader* pReader) override;
    virtual HRESULT SaveToXml(IXmlWriter* pWriter) override;

private:
    float           m_flStartLeft;
    float           m_flStartTop;
    float           m_flStartRight;
    float           m_flStartBottom;
    float           m_flEndLeft;
    float           m_flEndTop;
    float           m_flEndRight;
    float           m_flEndBottom;
    PanZoomShape    m_shape;
    EasingType      m_easing;
    bool            m_fRandomStartPosition;
    LONGLONG        m_llHoldStartHns;
    LONGLONG        m_llHoldEndHns;
};

// ============================================================================
// AudioDuckingProperties
// ============================================================================
// Properties for audio ducking - automatically lowering background music
// volume when narration/dialogue is present.
//
class STORYBOARD_API AudioDuckingProperties
{
public:
    AudioDuckingProperties();
    ~AudioDuckingProperties();

    // Enable ducking
    bool IsEnabled() const throw();
    void SetEnabled(bool fEnabled) throw();

    // Ducking level (0.0 = full volume, 1.0 = silence)
    float GetDuckLevel() const throw();
    void SetDuckLevel(float flLevel) throw();

    // Fade in time (ms)
    DWORD GetFadeInMs() const throw();
    void SetFadeInMs(DWORD dwMs) throw();

    // Fade out time (ms)
    DWORD GetFadeOutMs() const throw();
    void SetFadeOutMs(DWORD dwMs) throw();

    // Threshold (dB above which ducking triggers)
    float GetThresholdDb() const throw();
    void SetThresholdDb(float flDb) throw();

    // Target track (which track to duck)
    DWORD GetTargetTrackIndex() const throw();
    void SetTargetTrackIndex(DWORD dwIndex) throw();

    // Serialization
    HRESULT LoadFromXml(IXmlReader* pReader);
    HRESULT SaveToXml(IXmlWriter* pWriter);

private:
    bool    m_fEnabled;
    float   m_flDuckLevel;
    DWORD   m_dwFadeInMs;
    DWORD   m_dwFadeOutMs;
    float   m_flThresholdDb;
    DWORD   m_dwTargetTrackIndex;
};

} // namespace StoryboardManager

#endif // MOVIEEFFECT_H
