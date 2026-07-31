/*
 * PreviewDataContext.h
 *
 * Data context for the preview window DirectUI bindings. Provides the
 * current frame image, play/pause/stop state, time position, and volume
 * to the SundanceMainElementBehavior for rendering in the preview panel.
 *
 * RTTI: ?AVPreviewDataContext@@, ?AV?$CComObject@VPreviewDataContext@@@ATL@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef PREVIEWDATACONTEXT_H
#define PREVIEWDATACONTEXT_H

#include "../pch.h"
#include "../MovieMakerCore.h"

class PreviewPresenterWrapper;

namespace StoryboardManager { class MovieProject; }

// ============================================================================
// PreviewDataContext
// ============================================================================
// COM object exposing preview state to DirectUI via IDispatch property
// binding. The DirectUI behavior layer (SundanceMainElementBehavior) reads
// properties by name through IDispatch::Invoke.
//
// Properties exposed:
//   CurrentPosition  (double, seconds)
//   TotalDuration    (double, seconds)
//   IsPlaying        (bool)
//   IsPaused         (bool)
//   IsStopped        (bool)
//   Volume           (double, 0.0-1.0)
//   PlaybackSpeed    (double, 1.0 = normal)
//   CurrentFrameUrl  (BSTR, thumbnail path for DUI image binding)
//   HasPreview       (bool, true if preview window is connected)
//
class ATL_NO_VTABLE PreviewDataContext :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<PreviewDataContext, &CLSID_NULL>,
    public IDispatchImpl<IDispatch, &IID_IDispatch>
{
public:
    PreviewDataContext();
    virtual ~PreviewDataContext();

    DECLARE_NO_REGISTRY()
    DECLARE_NOT_AGGREGATABLE(PreviewDataContext)

    BEGIN_COM_MAP(PreviewDataContext)
        COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()

    // -- IDispatch --
    STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR* rgszNames, UINT cNames,
                             LCID lcid, DISPID* rgDispId) override;
    STDMETHOD(GetTypeInfo)(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo) override;
    STDMETHOD(GetTypeInfoCount)(UINT* pctinfo) override;
    STDMETHOD(Invoke)(DISPID dispIdMember, REFIID riid, LCID lcid,
                      WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pVarResult,
                      EXCEPINFO* pExcepInfo, UINT* puArgErr) override;

    // -- Binding support --
    HRESULT GetProperty(LPCWSTR pszName, VARIANT* pvarValue);
    HRESULT SetProperty(LPCWSTR pszName, const VARIANT* varValue);
    void    RefreshAllProperties();

    // -- Presenter binding --
    void SetPresenter(PreviewPresenterWrapper* pPresenter);
    PreviewPresenterWrapper* GetPresenter() const throw();

    // -- Project binding --
    void SetProject(StoryboardManager::MovieProject* pProject);
    StoryboardManager::MovieProject* GetProject() const throw();

    // -- Position in 100ns units --
    LONGLONG GetCurrentPositionHns() const throw();
    LONGLONG GetTotalDurationHns() const throw();
    void SetCurrentPositionHns(LONGLONG llPositionHns);

    // -- Notification handlers (called from the render thread) --
    void OnPlaybackStateChanged();
    void OnPositionChanged(LONGLONG llPositionHns);
    void OnVolumeChanged(double dblVolume);

private:
    // -- Property refresh helpers --
    void RefreshPlaybackState();
    void RefreshPositionState();

    // -- Property change notification --
    void FirePropertyChanged(LPCWSTR pszPropertyName);

    // -- State cache --
    bool    m_fIsPlaying;
    bool    m_fIsPaused;
    bool    m_fIsStopped;
    bool    m_fHasPreview;
    double  m_dblCurrentPosition;
    double  m_dblTotalDuration;
    double  m_dblVolume;
    double  m_dblPlaybackSpeed;
    BSTR    m_bstrCurrentFrameUrl;

    // -- Position in 100ns units --
    LONGLONG m_llCurrentPositionHns;
    LONGLONG m_llTotalDurationHns;

    // -- Back-pointers (not owned) --
    PreviewPresenterWrapper*              m_pPresenter;
    StoryboardManager::MovieProject*      m_pProject;

    // -- Synchronization --
    CRITICAL_SECTION m_csLock;

    PreviewDataContext(const PreviewDataContext&);
    PreviewDataContext& operator=(const PreviewDataContext&);
};

// ============================================================================
// Factory
// ============================================================================
HRESULT CreatePreviewDataContext(PreviewDataContext** ppContext);

#endif // PREVIEWDATACONTEXT_H
