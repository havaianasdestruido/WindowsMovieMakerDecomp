/*
 * Dialogs.h
 *
 * Dialog behavior classes for the Sundance UI layer. Each dialog is
 * implemented as a DirectUI behavior that manages its own element tree,
 * button handlers, and data binding.
 *
 * RTTI classes:
 *   ?AVRenderSummaryDialog@@
 *   ?AVUserEncodeProfileDialog@@
 *   ?AVUserEncodeProfileRecommended@@
 *   ?AVUserEncodeProfileWLVS@@
 *   ?AVUserEncodeProfileInfo@@
 *   ?AVMultipleEffectDialog@@
 *   ?AVSundanceApplicationOptionsDialog@@
 *   ?AVDontShowPromptDialog@@
 *   ?AVSundanceDontShowPromptDialog@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once
#ifndef SUNDANCE_DIALOGS_H
#define SUNDANCE_DIALOGS_H

#include "../../pch.h"
#include "../../MovieMakerCore.h"
#include "DuiInterfaces.h"

class SundanceAppMain;

namespace Sundance
{

// ============================================================================
// RenderSummaryDialog
// ============================================================================
// Render/export summary dialog. Displays a summary of the rendering
// operation including output file, duration, estimated file size,
// and estimated render time. Provides "Publish", "Cancel", and
// "Back" buttons.
//
// RTTI: ?AVRenderSummaryDialog@@
// ATL:  CComObjectNoLock<RenderSummaryDialog>
//
class ATL_NO_VTABLE RenderSummaryDialog :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<RenderSummaryDialog>,
    public IDuiBehaviorImpl
{
public:
    RenderSummaryDialog();
    virtual ~RenderSummaryDialog();

    DECLARE_REGISTRY_RESOURCEID(IDR_RENDER_SUMMARY_DIALOG)
    DECLARE_NOT_AGGREGATABLE(RenderSummaryDialog)

    BEGIN_COM_MAP(RenderSummaryDialog)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    // -- IDuiBehavior --
    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;
    STDMETHOD(OnMessage)(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled) override;

    // -- Dialog data --
    void SetOutputFilePath(LPCWSTR pszPath);
    void SetOutputDuration(double dDurationSeconds);
    void SetEstimatedFileSize(ULONGLONG cbSize);
    void SetEstimatedRenderTime(DWORD dwTimeSeconds);
    void SetOutputFormat(LPCWSTR pszFormat);

    // -- Dialog result --
    enum DialogResult { DialogResultNone, DialogResultPublish, DialogResultCancel, DialogResultBack };
    DialogResult GetResult() const throw();

private:
    IDuiElement*    m_pElement;
    SundanceAppMain* m_pAppMain;
    DialogResult    m_result;

    ATL::CString    m_strOutputPath;
    double          m_dDuration;
    ULONGLONG       m_cbFileSize;
    DWORD           m_dwRenderTimeSeconds;
    ATL::CString    m_strFormat;

    void UpdateSummaryDisplay();
};

// ============================================================================
// UserEncodeProfileDialog
// ============================================================================
// Custom encode profile selection dialog. Shows available encoding profiles
// with details (resolution, bitrate, codec) and allows the user to select
// or create a custom profile.
//
// RTTI: ?AVUserEncodeProfileDialog@@
// ATL:  CComObjectNoLock<UserEncodeProfileDialog>
//
class ATL_NO_VTABLE UserEncodeProfileDialog :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<UserEncodeProfileDialog>,
    public IDuiBehaviorImpl
{
public:
    UserEncodeProfileDialog();
    virtual ~UserEncodeProfileDialog();

    DECLARE_REGISTRY_RESOURCEID(IDR_USER_ENCODE_PROFILE_DIALOG)
    DECLARE_NOT_AGGREGATABLE(UserEncodeProfileDialog)

    BEGIN_COM_MAP(UserEncodeProfileDialog)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;
    STDMETHOD(OnMessage)(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled) override;

    void SetSelectedProfile(DWORD dwProfileId);
    DWORD GetSelectedProfile() const throw();

private:
    IDuiElement*    m_pElement;
    SundanceAppMain* m_pAppMain;
    DWORD           m_dwSelectedProfile;
};

// ============================================================================
// UserEncodeProfileRecommended
// ============================================================================
// Recommended encode profiles sub-panel within the profile dialog.
// Shows Microsoft-recommended encoding settings.
//
// RTTI: ?AVUserEncodeProfileRecommended@@
// ATL:  CComObjectNoLock<UserEncodeProfileRecommended>
//
class ATL_NO_VTABLE UserEncodeProfileRecommended :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<UserEncodeProfileRecommended>,
    public IDuiBehaviorImpl
{
public:
    UserEncodeProfileRecommended();
    virtual ~UserEncodeProfileRecommended();

    DECLARE_REGISTRY_RESOURCEID(IDR_USER_ENCODE_PROFILE_RECOMMENDED)
    DECLARE_NOT_AGGREGATABLE(UserEncodeProfileRecommended)

    BEGIN_COM_MAP(UserEncodeProfileRecommended)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;

    void RefreshProfiles();

private:
    IDuiElement* m_pElement;
    struct ProfileInfo { DWORD dwId; ATL::CString strName; ATL::CString strDescription; };
    std::vector<ProfileInfo> m_profiles;
};

// ============================================================================
// UserEncodeProfileWLVS
// ============================================================================
// Windows Live Video Studio encode profiles sub-panel.
//
// RTTI: ?AVUserEncodeProfileWLVS@@
// ATL:  CComObjectNoLock<UserEncodeProfileWLVS>
//
class ATL_NO_VTABLE UserEncodeProfileWLVS :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<UserEncodeProfileWLVS>,
    public IDuiBehaviorImpl
{
public:
    UserEncodeProfileWLVS();
    virtual ~UserEncodeProfileWLVS();

    DECLARE_REGISTRY_RESOURCEID(IDR_USER_ENCODE_PROFILE_WLVS)
    DECLARE_NOT_AGGREGATABLE(UserEncodeProfileWLVS)

    BEGIN_COM_MAP(UserEncodeProfileWLVS)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;

    void RefreshProfiles();

private:
    IDuiElement* m_pElement;
};

// ============================================================================
// UserEncodeProfileInfo
// ============================================================================
// Encode profile information display panel. Shows codec, resolution,
// frame rate, bitrate, and audio settings for a selected profile.
//
// RTTI: ?AVUserEncodeProfileInfo@@
// ATL:  CComObjectNoLock<UserEncodeProfileInfo>
//
class ATL_NO_VTABLE UserEncodeProfileInfo :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<UserEncodeProfileInfo>,
    public IDuiBehaviorImpl
{
public:
    UserEncodeProfileInfo();
    virtual ~UserEncodeProfileInfo();

    DECLARE_REGISTRY_RESOURCEID(IDR_USER_ENCODE_PROFILE_INFO)
    DECLARE_NOT_AGGREGATABLE(UserEncodeProfileInfo)

    BEGIN_COM_MAP(UserEncodeProfileInfo)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;

    void SetProfileId(DWORD dwProfileId);
    void UpdateDisplay();

private:
    IDuiElement* m_pElement;
    DWORD        m_dwProfileId;

    struct ProfileDetails
    {
        ATL::CString strCodec;
        int          nWidth;
        int          nHeight;
        int          nFrameRate;
        DWORD        dwBitrate;
        ATL::CString strAudioCodec;
        int          nAudioSampleRate;
        DWORD        dwAudioBitrate;
    };

    ProfileDetails m_details;
    void LoadProfileDetails();
};

// ============================================================================
// MultipleEffectDialog
// ============================================================================
// Effect selection dialog. Displays a grid of available video effects
// with previews and allows the user to select one or more effects
// to apply to the selected timeline item.
//
// RTTI: ?AVMultipleEffectDialog@@
// ATL:  CComObjectNoLock<MultipleEffectDialog>
//
class ATL_NO_VTABLE MultipleEffectDialog :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<MultipleEffectDialog>,
    public IDuiBehaviorImpl
{
public:
    MultipleEffectDialog();
    virtual ~MultipleEffectDialog();

    DECLARE_REGISTRY_RESOURCEID(IDR_MULTIPLE_EFFECT_DIALOG)
    DECLARE_NOT_AGGREGATABLE(MultipleEffectDialog)

    BEGIN_COM_MAP(MultipleEffectDialog)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;
    STDMETHOD(OnMessage)(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled) override;

    void SetTargetItemId(DWORD dwItemId);
    void SetAlreadyAppliedEffects(const std::vector<DWORD>& effectIds);
    HRESULT GetSelectedEffects(std::vector<DWORD>& effectIds);

private:
    IDuiElement*    m_pElement;
    SundanceAppMain* m_pAppMain;
    DWORD           m_dwTargetItemId;
    std::vector<DWORD> m_appliedEffects;
    std::vector<DWORD> m_selectedEffects;
};

// ============================================================================
// SundanceApplicationOptionsDialog
// ============================================================================
// Application options dialog. Provides settings for default project
// dimensions, default music, auto-save interval, photo duration,
// and other application-wide preferences.
//
// RTTI: ?AVSundanceApplicationOptionsDialog@@
// ATL:  CComObjectNoLock<SundanceApplicationOptionsDialog>
//
class ATL_NO_VTABLE SundanceApplicationOptionsDialog :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<SundanceApplicationOptionsDialog>,
    public IDuiBehaviorImpl
{
public:
    SundanceApplicationOptionsDialog();
    virtual ~SundanceApplicationOptionsDialog();

    DECLARE_REGISTRY_RESOURCEID(IDR_SUNDANCE_APPLICATION_OPTIONS_DIALOG)
    DECLARE_NOT_AGGREGATABLE(SundanceApplicationOptionsDialog)

    BEGIN_COM_MAP(SundanceApplicationOptionsDialog)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;
    STDMETHOD(OnMessage)(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled) override;

    // -- Options data --
    void SetDefaultPhotoDuration(DWORD dwDurationMs);
    DWORD GetDefaultPhotoDuration() const throw();
    void SetAutoSaveInterval(DWORD dwIntervalMs);
    DWORD GetAutoSaveInterval() const throw();

private:
    IDuiElement*    m_pElement;
    SundanceAppMain* m_pAppMain;
    DWORD           m_dwDefaultPhotoDuration;
    DWORD           m_dwAutoSaveInterval;
};

// ============================================================================
// DontShowPromptDialog
// ============================================================================
// "Don't show this again" prompt dialog. Used for generic confirmation
// prompts throughout the application. Contains a message, OK/Cancel
// buttons, and a "Don't show this again" checkbox.
//
// RTTI: ?AVDontShowPromptDialog@@
// ATL:  CComObjectNoLock<DontShowPromptDialog>
//
class ATL_NO_VTABLE DontShowPromptDialog :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<DontShowPromptDialog>,
    public IDuiBehaviorImpl
{
public:
    DontShowPromptDialog();
    virtual ~DontShowPromptDialog();

    DECLARE_REGISTRY_RESOURCEID(IDR_DONT_SHOW_PROMPT_DIALOG)
    DECLARE_NOT_AGGREGATABLE(DontShowPromptDialog)

    BEGIN_COM_MAP(DontShowPromptDialog)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;

    // -- Prompt data --
    void SetMessage(LPCWSTR pszMessage);
    void SetPromptKey(LPCWSTR pszKey);
    bool IsDontShowChecked() const throw();

    enum PromptResult { PromptResultOK, PromptResultCancel };
    PromptResult GetResult() const throw();

private:
    IDuiElement*    m_pElement;
    ATL::CString    m_strMessage;
    ATL::CString    m_strPromptKey;
    bool            m_bDontShowChecked;
    PromptResult    m_result;
};

// ============================================================================
// SundanceDontShowPromptDialog
// ============================================================================
// Movie Maker-specific don't show prompt. Similar to DontShowPromptDialog
// but integrates with SundanceAppMain's DontShow prompt management
// for prompt persistence across sessions.
//
// RTTI: ?AVSundanceDontShowPromptDialog@@
// ATL:  CComObjectNoLock<SundanceDontShowPromptDialog>
//
class ATL_NO_VTABLE SundanceDontShowPromptDialog :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<SundanceDontShowPromptDialog>,
    public IDuiBehaviorImpl
{
public:
    SundanceDontShowPromptDialog();
    virtual ~SundanceDontShowPromptDialog();

    DECLARE_REGISTRY_RESOURCEID(IDR_SUNDANCE_DONT_SHOW_PROMPT_DIALOG)
    DECLARE_NOT_AGGREGATABLE(SundanceDontShowPromptDialog)

    BEGIN_COM_MAP(SundanceDontShowPromptDialog)
        COM_INTERFACE_ENTRY(IDuiBehavior)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    STDMETHOD(OnElementAttached)(IDuiElement* pElement) override;
    STDMETHOD(OnElementDetached)(IDuiElement* pElement) override;

    void SetMessage(LPCWSTR pszMessage);
    void SetPromptKey(LPCWSTR pszKey);
    bool ShouldShow() const;

    enum PromptResult { PromptResultOK, PromptResultCancel };
    PromptResult GetResult() const throw();

private:
    IDuiElement*    m_pElement;
    SundanceAppMain* m_pAppMain;
    ATL::CString    m_strMessage;
    ATL::CString    m_strPromptKey;
    bool            m_bDontShowChecked;
    PromptResult    m_result;
};

} // namespace Sundance

#endif // SUNDANCE_DIALOGS_H
