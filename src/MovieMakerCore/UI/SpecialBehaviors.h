/*
 * SpecialBehaviors.h
 *
 * Special-purpose DirectUI behaviors: Homer rendering layer, standard
 * layer, stacked effects, webcam, sparse mode, animations, and
 * encode profile selection.
 *
 * RTTI classes:
 *   ?AVHomerHeavyLayerBehavior@@
 *   ?AVStandardLayerBehavior@@
 *   ?AVMultipleEffectBehavior@@
 *   ?AVWebcamElementBehavior@@
 *   ?AVAmpPreventSparseModeBehavior@@
 *   ?AVAmpSlideAnimationBehavior@@
 *   ?AVAmpFadeAnimationBehavior@@
 *   ?AVUserEncodeProfileBehavior@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once
#ifndef SUNDANCE_SPECIAL_BEHAVIORS_H
#define SUNDANCE_SPECIAL_BEHAVIORS_H

#include "../../pch.h"
#include "../../MovieMakerCore.h"
#include "DuiInterfaces.h"

class SundanceAppMain;

namespace Sundance
{

// ============================================================================
// HomerHeavyLayerBehavior
// ============================================================================
// The primary rendering layer behavior for Homer (the Direct3D-based video
// preview engine). Implements IDuiLayerBehavior to provide compositing
// support. "Heavy" indicates this layer handles the main video preview
// rendering with full D3D/Direct2D composition.
//
// Homer is the internal codename for the video preview renderer in
// Windows Live Movie Maker, based on X3D scene graph rendering.
//
// RTTI: ?AVHomerHeavyLayerBehavior@@
// ATL:  CComObjectNoLock<HomerHeavyLayerBehavior>
//
class ATL_NO_VTABLE HomerHeavyLayerBehavior :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<HomerHeavyLayerBehavior>,
    public IDuiLayerBehaviorImpl
{
public:
    HomerHeavyLayerBehavior();
    virtual ~HomerHeavyLayerBehavior();

    DECLARE_REGISTRY_RESOURCEID(IDR_HOMER_HEAVY_LAYER_BEHAVIOR)
    DECLARE_NOT_AGGREGATABLE(HomerHeavyLayerBehavior)

    BEGIN_COM_MAP(HomerHeavyLayerBehavior)
        COM_INTERFACE_ENTRY(IDuiLayerBehavior)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    // -- IDuiBehavior --
    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;
    STDMETHOD(OnPaint)(HDC hdc, const RECT* prcBounds) override;

    // -- IDuiLayerBehavior --
    STDMETHOD(BeginLayer)(HDC hdc, const RECT* prcBounds) override;
    STDMETHOD(EndLayer)(HDC hdc, const RECT* prcBounds) override;
    STDMETHOD(InvalidateLayer)() override;

    // -- Homer layer control --
    void SetHomerScene(void* pScene);
    void SetPreviewSurface(void* pSurface);
    void SetRenderRect(const RECT& rcRender);

private:
    IDuiElement*    m_pElement;
    SundanceAppMain* m_pAppMain;
    void*           m_pHomerScene;
    void*           m_pPreviewSurface;
    RECT            m_rcRender;
    bool            m_bLayerActive;
};

// ============================================================================
// StandardLayerBehavior
// ============================================================================
// Standard (lightweight) rendering layer for non-Homer elements.
// Used for UI overlay elements that need layer compositing but don't
// require the full D3D rendering pipeline.
//
// RTTI: ?AVStandardLayerBehavior@@
// ATL:  CComObjectNoLock<StandardLayerBehavior>
//
class ATL_NO_VTABLE StandardLayerBehavior :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<StandardLayerBehavior>,
    public IDuiLayerBehaviorImpl
{
public:
    StandardLayerBehavior();
    virtual ~StandardLayerBehavior();

    DECLARE_REGISTRY_RESOURCEID(IDR_STANDARD_LAYER_BEHAVIOR)
    DECLARE_NOT_AGGREGATABLE(StandardLayerBehavior)

    BEGIN_COM_MAP(StandardLayerBehavior)
        COM_INTERFACE_ENTRY(IDuiLayerBehavior)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;
    STDMETHOD(OnPaint)(HDC hdc, const RECT* prcBounds) override;
    STDMETHOD(BeginLayer)(HDC hdc, const RECT* prcBounds) override;
    STDMETHOD(EndLayer)(HDC hdc, const RECT* prcBounds) override;
    STDMETHOD(InvalidateLayer)() override;

private:
    IDuiElement*    m_pElement;
    HDC             m_hdcMem;
    HBITMAP         m_hBmpMem;
    HBITMAP         m_hBmpOld;
    int             m_nWidth;
    int             m_nHeight;
};

// ============================================================================
// MultipleEffectBehavior
// ============================================================================
// Stacked effects behavior. Manages multiple video effects applied to
// a single timeline item, rendering them in sequence. Each effect
// parameter is exposed for animation (keyframe support).
//
// RTTI: ?AVMultipleEffectBehavior@@
// ATL:  CComObjectNoLock<MultipleEffectBehavior>
//
class ATL_NO_VTABLE MultipleEffectBehavior :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<MultipleEffectBehavior>,
    public IDuiBehaviorImpl
{
public:
    MultipleEffectBehavior();
    virtual ~MultipleEffectBehavior();

    DECLARE_REGISTRY_RESOURCEID(IDR_MULTIPLE_EFFECT_BEHAVIOR)
    DECLARE_NOT_AGGREGATABLE(MultipleEffectBehavior)

    BEGIN_COM_MAP(MultipleEffectBehavior)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;

    // -- Effect stack --
    HRESULT AddEffect(DWORD dwEffectId);
    HRESULT RemoveEffect(DWORD dwEffectId);
    HRESULT MoveEffect(DWORD dwEffectId, int nNewIndex);
    DWORD   GetEffectCount() const throw();
    HRESULT GetEffectId(int nIndex, DWORD* pdwEffectId);

    // -- Effect parameters --
    HRESULT SetEffectParameter(DWORD dwEffectId, LPCWSTR pszParamName, float fValue);
    HRESULT GetEffectParameter(DWORD dwEffectId, LPCWSTR pszParamName, float* pfValue);

private:
    IDuiElement*    m_pElement;
    SundanceAppMain* m_pAppMain;

    struct EffectEntry
    {
        DWORD   dwEffectId;
        int     nIndex;
        std::map<ATL::CString, float> parameters;
    };

    std::vector<EffectEntry> m_effectStack;
};

// ============================================================================
// WebcamElementBehavior
// ============================================================================
// Webcam display element behavior. Manages a live webcam preview
// rendered into a DirectUI element via a child HWND or DirectShow
// sample grabber.
//
// RTTI: ?AVWebcamElementBehavior@@
// ATL:  CComObjectNoLock<WebcamElementBehavior>
//
class ATL_NO_VTABLE WebcamElementBehavior :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<WebcamElementBehavior>,
    public IDuiBehaviorImpl
{
public:
    WebcamElementBehavior();
    virtual ~WebcamElementBehavior();

    DECLARE_REGISTRY_RESOURCEID(IDR_WEBCAM_ELEMENT_BEHAVIOR)
    DECLARE_NOT_AGGREGATABLE(WebcamElementBehavior)

    BEGIN_COM_MAP(WebcamElementBehavior)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;
    STDMETHOD(OnPaint)(HDC hdc, const RECT* prcBounds) override;

    // -- Webcam control --
    HRESULT StartPreview(HWND hWndHost);
    HRESULT StopPreview();
    bool    IsPreviewing() const throw();

    // -- Capture --
    HRESULT CaptureFrame(LPCWSTR pszOutputPath);

private:
    IDuiElement*    m_pElement;
    HWND            m_hWndHost;
    bool            m_bPreviewing;
    HBITMAP         m_hCurrentFrame;
};

// ============================================================================
// AmpPreventSparseModeBehavior
// ============================================================================
// Prevents the AMP layout engine from entering sparse mode for certain
// elements. Sparse mode is a DirectUI optimization that skips layout
// for off-screen or occluded elements; this behavior ensures certain
// elements (like the playback cursor) always receive layout passes.
//
// RTTI: ?AVAmpPreventSparseModeBehavior@@
// ATL:  CComObjectNoLock<AmpPreventSparseModeBehavior>
//
class ATL_NO_VTABLE AmpPreventSparseModeBehavior :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<AmpPreventSparseModeBehavior>,
    public IDuiBehaviorImpl
{
public:
    AmpPreventSparseModeBehavior();
    virtual ~AmpPreventSparseModeBehavior();

    DECLARE_REGISTRY_RESOURCEID(IDR_AMP_PREVENT_SPARSE_MODE_BEHAVIOR)
    DECLARE_NOT_AGGREGATABLE(AmpPreventSparseModeBehavior)

    BEGIN_COM_MAP(AmpPreventSparseModeBehavior)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;

private:
    IDuiElement* m_pElement;
};

// ============================================================================
// AmpSlideAnimationBehavior
// ============================================================================
// Slide animation behavior. Provides horizontal/vertical slide-in and
// slide-out transitions for UI elements during view mode changes.
// Uses timer-based animation with easing.
//
// RTTI: ?AVAmpSlideAnimationBehavior@@
// ATL:  CComObjectNoLock<AmpSlideAnimationBehavior>
//
class ATL_NO_VTABLE AmpSlideAnimationBehavior :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<AmpSlideAnimationBehavior>,
    public IDuiBehaviorImpl
{
public:
    AmpSlideAnimationBehavior();
    virtual ~AmpSlideAnimationBehavior();

    DECLARE_REGISTRY_RESOURCEID(IDR_AMP_SLIDE_ANIMATION_BEHAVIOR)
    DECLARE_NOT_AGGREGATABLE(AmpSlideAnimationBehavior)

    BEGIN_COM_MAP(AmpSlideAnimationBehavior)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;
    STDMETHOD(OnMessage)(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled) override;

    // -- Animation control --
    enum SlideDirection { SlideLeft, SlideRight, SlideUp, SlideDown };
    HRESULT StartSlide(SlideDirection direction, DWORD dwDurationMs);
    HRESULT StopSlide();
    bool    IsAnimating() const throw();

private:
    IDuiElement*    m_pElement;
    SlideDirection  m_direction;
    DWORD           m_dwDuration;
    DWORD           m_dwStartTime;
    bool            m_bAnimating;
    UINT            m_nTimerId;

    float EaseInOutCubic(float t) const;
};

// ============================================================================
// AmpFadeAnimationBehavior
// ============================================================================
// Fade animation behavior. Provides fade-in and fade-out transitions
// for UI elements. Modifies the alpha channel of the element's
// rendering.
//
// RTTI: ?AVAmpFadeAnimationBehavior@@
// ATL:  CComObjectNoLock<AmpFadeAnimationBehavior>
//
class ATL_NO_VTABLE AmpFadeAnimationBehavior :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<AmpFadeAnimationBehavior>,
    public IDuiBehaviorImpl
{
public:
    AmpFadeAnimationBehavior();
    virtual ~AmpFadeAnimationBehavior();

    DECLARE_REGISTRY_RESOURCEID(IDR_AMP_FADE_ANIMATION_BEHAVIOR)
    DECLARE_NOT_AGGREGATABLE(AmpFadeAnimationBehavior)

    BEGIN_COM_MAP(AmpFadeAnimationBehavior)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;
    STDMETHOD(OnMessage)(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled) override;

    // -- Animation control --
    HRESULT FadeIn(DWORD dwDurationMs);
    HRESULT FadeOut(DWORD dwDurationMs);
    void    SetAlpha(float fAlpha);
    float   GetAlpha() const throw();
    bool    IsAnimating() const throw();

private:
    IDuiElement*    m_pElement;
    float           m_fCurrentAlpha;
    float           m_fStartAlpha;
    float           m_fEndAlpha;
    DWORD           m_dwDuration;
    DWORD           m_dwStartTime;
    bool            m_bAnimating;
    bool            m_bFadeIn;
    UINT            m_nTimerId;
};

// ============================================================================
// UserEncodeProfileBehavior
// ============================================================================
// Encode profile selection behavior. Manages the encode profile picker
// UI that allows users to choose from built-in and custom encoding
// profiles for publishing their movie.
//
// RTTI: ?AVUserEncodeProfileBehavior@@
// ATL:  CComObjectNoLock<UserEncodeProfileBehavior>
//
class ATL_NO_VTABLE UserEncodeProfileBehavior :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<UserEncodeProfileBehavior>,
    public IDuiBehaviorImpl
{
public:
    UserEncodeProfileBehavior();
    virtual ~UserEncodeProfileBehavior();

    DECLARE_REGISTRY_RESOURCEID(IDR_USER_ENCODE_PROFILE_BEHAVIOR)
    DECLARE_NOT_AGGREGATABLE(UserEncodeProfileBehavior)

    BEGIN_COM_MAP(UserEncodeProfileBehavior)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;
    STDMETHOD(OnMessage)(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled) override;

    // -- Profile management --
    HRESULT LoadProfiles();
    HRESULT SelectProfile(DWORD dwProfileId);
    DWORD   GetSelectedProfile() const throw();
    HRESULT CreateCustomProfile(LPCWSTR pszName, DWORD* pdwNewProfileId);
    HRESULT DeleteCustomProfile(DWORD dwProfileId);

    // -- Profile data --
    DWORD   GetProfileCount() const throw();
    HRESULT GetProfileName(DWORD dwProfileId, LPWSTR pszName, UINT cchName);

private:
    IDuiElement*    m_pElement;
    SundanceAppMain* m_pAppMain;
    DWORD           m_dwSelectedProfile;

    struct EncodeProfileEntry
    {
        DWORD       dwId;
        ATL::CString strName;
        bool        bIsCustom;
    };

    std::vector<EncodeProfileEntry> m_profiles;
};

} // namespace Sundance

#endif // SUNDANCE_SPECIAL_BEHAVIORS_H
