/*
 * SundanceAppDataContext.h
 *
 * COM data context object for the Sundance application framework.
 * Implements IDirectUIDataContext to provide data binding between
 * the application model and DirectUI visual tree elements.
 *
 * RTTI: ?AVSundanceAppDataContext@@
 * ATL wrapper: ?AV?$CComObject@VSundanceAppDataContext@@@ATL@@
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once
#ifndef SUNDANCE_APP_DATA_CONTEXT_H
#define SUNDANCE_APP_DATA_CONTEXT_H

#include "../pch.h"
#include "../MovieMakerCore.h"

#ifndef IDR_SUNDANCE_APP_DATA_CONTEXT
#define IDR_SUNDANCE_APP_DATA_CONTEXT 2001
#endif

// ============================================================================
// SundanceAppDataContext
// ============================================================================
// ATL COM object that serves as the data context for DirectUI data binding.
// Exposes application-level properties (project state, timeline position,
// selection, undo/redo availability) as named properties accessible
// from DirectUI XAML markup via @{PropertyName} bindings.
//
// Created as CComObject<SundanceAppDataContext> and aggregated into the
// DirectUI element tree as the root data context.
//
class ATL_NO_VTABLE SundanceAppDataContext :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<SundanceAppDataContext>,
    public IDispatchImpl<IDispatch, &IID_IDispatch>
{
public:
    SundanceAppDataContext();
    ~SundanceAppDataContext();

    DECLARE_REGISTRY_RESOURCEID(IDR_SUNDANCE_APP_DATA_CONTEXT)
    DECLARE_NOT_AGGREGATABLE(SundanceAppDataContext)

    BEGIN_COM_MAP(SundanceAppDataContext)
        COM_INTERFACE_ENTRY(IDispatch)
    END_COM_MAP()

    // -- Named property accessors (IDispatch::GetIDsOfNames / Invoke) --
    // Exposed properties:
    //   "ProjectName"        - BSTR  - Current project file name
    //   "IsProjectOpen"      - BOOL  - Whether a project is loaded
    //   "IsProjectDirty"     - BOOL  - Whether project has unsaved changes
    //   "CanUndo"            - BOOL  - Undo available
    //   "CanRedo"            - BOOL  - Redo available
    //   "CanCut"             - BOOL  - Cut available
    //   "CanCopy"            - BOOL  - Copy available
    //   "CanPaste"           - BOOL  - Paste available
    //   "IsPlaying"          - BOOL  - Playback active
    //   "IsPublishing"       - BOOL  - Publish in progress
    //   "IsEncoding"         - BOOL  - Encoding in progress
    //   "TimelinePosition"   - DOUBLE - Current playback position (seconds)
    //   "TimelineDuration"   - DOUBLE - Total project duration (seconds)
    //   "SelectedTrack"      - LONG   - Currently selected timeline track
    //   "VersionString"      - BSTR  - Application version string

    // IDataContext support for DirectUI binding
    HRESULT STDMETHODCALLTYPE GetProperty(LPCWSTR pszName, VARIANT* pvarValue);
    HRESULT STDMETHODCALLTYPE SetProperty(LPCWSTR pszName, const VARIANT* varValue);

    // Refresh all bound properties (triggers DirectUI update)
    void RefreshAllProperties();

    // Notification handlers called by SundanceAppMain
    void OnProjectStateChanged();
    void OnTimelinePositionChanged(double dPosition);
    void OnSelectionChanged();

private:
    // Cached property values for change detection
    bool        m_bProjectOpen;
    bool        m_bProjectDirty;
    bool        m_bCanUndo;
    bool        m_bCanRedo;
    bool        m_bIsPlaying;
    bool        m_bIsPublishing;
    bool        m_bIsEncoding;
    double      m_dTimelinePosition;
    double      m_dTimelineDuration;
    long        m_lSelectedTrack;
    ATL::CString m_strProjectName;

    // Refresh individual property caches from model
    void RefreshProjectProperties();
    void RefreshCommandProperties();
    void RefreshPlaybackProperties();

    // Helper: fire a property change notification to DirectUI
    void FirePropertyChanged(LPCWSTR pszPropertyName);

    // IDispatch implementation
    STDMETHOD(GetIDsOfNames)(REFIID, LPOLESTR*, UINT, LCID, DISPID*) override;
    STDMETHOD(GetTypeInfo)(UINT, LCID, ITypeInfo**) override;
    STDMETHOD(GetTypeInfoCount)(UINT*) override;
    STDMETHOD(Invoke)(DISPID, REFIID, LCID, WORD, DISPPARAMS*, VARIANT*, EXCEPINFO*, UINT*) override;
};

// ============================================================================
// Helper to create an instance
// ============================================================================
HRESULT CreateSundanceAppDataContext(SundanceAppDataContext** ppContext);

#endif // SUNDANCE_APP_DATA_CONTEXT_H
