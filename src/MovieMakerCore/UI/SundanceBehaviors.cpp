/*
 * SundanceBehaviors.cpp
 *
 * Behavior factory registration for all Sundance DirectUI behaviors.
 * This file registers every behavior class with SundanceBehaviorFactory
 * so that DirectUI can instantiate them by name when loading .duxt
 * resource files.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "pch.h"
#include "SundanceBehaviors.h"

// ============================================================================
// External references
// ============================================================================
extern HINSTANCE MovieCore_GetInstance(void);

namespace Sundance
{

// ============================================================================
// Behavior registration entry
// ============================================================================
struct BehaviorRegistration
{
    LPCWSTR     pszClassName;
    FACTORYFN   pfnCreate;
};

// ============================================================================
// Forward declarations: factory functions for each behavior class
// ============================================================================
// Each behavior class provides a static CreateBehavior factory function
// that returns an IDuiBehaviorImpl* wrapped in CComObjectNoLock.

static HRESULT CreateTimelineBehavior(IDuiBehavior** ppBehavior);
static HRESULT CreateTimelineItemBehavior(IDuiBehavior** ppBehavior);
static HRESULT CreateTimelineSecondaryTrackItemBehavior(IDuiBehavior** ppBehavior);
static HRESULT CreateTimelineVisualTrackItemBehavior(IDuiBehavior** ppBehavior);
static HRESULT CreateTimelineItemInputBehavior(IDuiBehavior** ppBehavior);
static HRESULT CreateTimelineInstructionsBehavior(IDuiBehavior** ppBehavior);
static HRESULT CreateTimelineSelectionRootBehavior(IDuiBehavior** ppBehavior);
static HRESULT CreateSundanceMainElementBehavior(IDuiBehavior** ppBehavior);
static HRESULT CreateAMPMainWindowBehavior(IDuiBehavior** ppBehavior);
static HRESULT CreateAMPCommandBarBehavior(IDuiBehavior** ppBehavior);
static HRESULT CreateRenderSummaryDialog(IDuiBehavior** ppBehavior);
static HRESULT CreateMultipleEffectDialog(IDuiBehavior** ppBehavior);
static HRESULT CreateSundanceApplicationOptionsDialog(IDuiBehavior** ppBehavior);
static HRESULT CreatePopUpSlider(IDuiBehavior** ppBehavior);
static HRESULT CreateProgressStatusBar(IDuiBehavior** ppBehavior);
static HRESULT CreateProgressDialog(IDuiBehavior** ppBehavior);
static HRESULT CreateTrimBehavior(IDuiBehavior** ppBehavior);
static HRESULT CreateTextBoxBehavior(IDuiBehavior** ppBehavior);
static HRESULT CreateRichEditControlBehavior(IDuiBehavior** ppBehavior);
static HRESULT CreateCaretBehavior(IDuiBehavior** ppBehavior);
static HRESULT CreateCaretScrollBehavior(IDuiBehavior** ppBehavior);
static HRESULT CreateAboveHomerBehavior(IDuiBehavior** ppBehavior);
static HRESULT CreateHelpBehavior(IDuiBehavior** ppBehavior);
static HRESULT CreateCaptureUIBehavior(IDuiBehavior** ppBehavior);
static HRESULT CreateFullscreenLayoutBehavior(IDuiBehavior** ppBehavior);
static HRESULT CreateResizeablePaneBehavior(IDuiBehavior** ppBehavior);
static HRESULT CreateProjectWorkspaceLayoutBehavior(IDuiBehavior** ppBehavior);
static HRESULT CreateInlinePreviewLayoutBehavior(IDuiBehavior** ppBehavior);
static HRESULT CreateInlinePreviewSliderBehavior(IDuiBehavior** ppBehavior);
static HRESULT CreateHomerHeavyLayerBehavior(IDuiBehavior** ppBehavior);
static HRESULT CreateStandardLayerBehavior(IDuiBehavior** ppBehavior);
static HRESULT CreateMultipleEffectBehavior(IDuiBehavior** ppBehavior);
static HRESULT CreateWebcamElementBehavior(IDuiBehavior** ppBehavior);
static HRESULT CreateAmpPreventSparseModeBehavior(IDuiBehavior** ppBehavior);
static HRESULT CreateAmpSlideAnimationBehavior(IDuiBehavior** ppBehavior);
static HRESULT CreateAmpFadeAnimationBehavior(IDuiBehavior** ppBehavior);
static HRESULT CreateUserEncodeProfileBehavior(IDuiBehavior** ppBehavior);
static HRESULT CreateOptionsDialogBehavior(IDuiBehavior** ppBehavior);

// ============================================================================
// Factory function type
// ============================================================================
typedef HRESULT (CALLBACK* FACTORYFN)(IDuiBehavior** ppBehavior);

// ============================================================================
// Static registration table
// ============================================================================
// Maps DirectUI class names (as referenced in .duxt markup) to factory
// functions. The SundanceBehaviorFactory iterates this table at startup
// to build a name-to-factory lookup map.
//
static const BehaviorRegistration g_behaviorRegistrations[] =
{
    // -- Timeline behaviors --
    { L"TimelineBehavior",                              CreateTimelineBehavior },
    { L"TimelineItemBehavior",                          CreateTimelineItemBehavior },
    { L"TimelineSecondaryTrackItemBehavior",            CreateTimelineSecondaryTrackItemBehavior },
    { L"TimelineVisualTrackItemBehavior",               CreateTimelineVisualTrackItemBehavior },
    { L"TimelineItemInputBehavior",                     CreateTimelineItemInputBehavior },
    { L"TimelineInstructionsBehavior",                  CreateTimelineInstructionsBehavior },
    { L"TimelineSelectionRootBehavior",                 CreateTimelineSelectionRootBehavior },

    // -- UI component behaviors --
    { L"SundanceMainElementBehavior",                   CreateSundanceMainElementBehavior },
    { L"AMPMainWindowBehavior",                         CreateAMPMainWindowBehavior },
    { L"AMPCommandBarBehavior",                         CreateAMPCommandBarBehavior },

    // -- Dialog behaviors --
    { L"RenderSummaryDialog",                           CreateRenderSummaryDialog },
    { L"MultipleEffectDialog",                          CreateMultipleEffectDialog },
    { L"SundanceApplicationOptionsDialog",              CreateSundanceApplicationOptionsDialog },

    // -- Slider UI --
    { L"PopUpSlider",                                   CreatePopUpSlider },

    // -- Progress behaviors --
    { L"ProgressStatusBar",                             CreateProgressStatusBar },
    { L"ProgressDialog",                                CreateProgressDialog },

    // -- Editing behaviors --
    { L"TrimBehavior",                                  CreateTrimBehavior },
    { L"TextBoxBehavior",                               CreateTextBoxBehavior },
    { L"RichEditControlBehavior",                       CreateRichEditControlBehavior },
    { L"CaretBehavior",                                 CreateCaretBehavior },
    { L"CaretScrollBehavior",                           CreateCaretScrollBehavior },
    { L"AboveHomerBehavior",                            CreateAboveHomerBehavior },
    { L"HelpBehavior",                                  CreateHelpBehavior },
    { L"CaptureUIBehavior",                             CreateCaptureUIBehavior },

    // -- Layout behaviors --
    { L"FullscreenLayoutBehavior",                      CreateFullscreenLayoutBehavior },
    { L"ResizeablePaneBehavior",                        CreateResizeablePaneBehavior },
    { L"ProjectWorkspaceLayoutBehavior",                CreateProjectWorkspaceLayoutBehavior },
    { L"InlinePreviewLayoutBehavior",                   CreateInlinePreviewLayoutBehavior },
    { L"InlinePreviewSliderBehavior",                   CreateInlinePreviewSliderBehavior },

    // -- Special behaviors --
    { L"HomerHeavyLayerBehavior",                       CreateHomerHeavyLayerBehavior },
    { L"StandardLayerBehavior",                         CreateStandardLayerBehavior },
    { L"MultipleEffectBehavior",                        CreateMultipleEffectBehavior },
    { L"WebcamElementBehavior",                         CreateWebcamElementBehavior },
    { L"AmpPreventSparseModeBehavior",                  CreateAmpPreventSparseModeBehavior },
    { L"AmpSlideAnimationBehavior",                     CreateAmpSlideAnimationBehavior },
    { L"AmpFadeAnimationBehavior",                      CreateAmpFadeAnimationBehavior },
    { L"UserEncodeProfileBehavior",                     CreateUserEncodeProfileBehavior },
    { L"OptionsDialogBehavior",                         CreateOptionsDialogBehavior },
};

static const size_t g_cBehaviorRegistrations =
    sizeof(g_behaviorRegistrations) / sizeof(g_behaviorRegistrations[0]);

// ============================================================================
// SundanceBehaviorFactory
// ============================================================================
// Singleton factory that manages behavior creation. At startup, builds a
// std::map<LPCWSTR, FACTORYFN> from the registration table. When DirectUI
// loads a .duxt resource and encounters a Behavior attribute, it calls
// into this factory to instantiate the named behavior class.
//
class SundanceBehaviorFactory
{
public:
    SundanceBehaviorFactory()
        : m_bRegistered(false)
    {
    }

    static SundanceBehaviorFactory& Instance()
    {
        static SundanceBehaviorFactory s_instance;
        return s_instance;
    }

    // Register all behavior factory functions. Called once at DLL startup.
    HRESULT RegisterAll()
    {
        if (m_bRegistered)
            return S_OK;

        for (size_t i = 0; i < g_cBehaviorRegistrations; ++i)
        {
            m_factoryMap[std::wstring(g_behaviorRegistrations[i].pszClassName)] =
                g_behaviorRegistrations[i].pfnCreate;
        }

        m_bRegistered = true;
        return S_OK;
    }

    // Create a behavior by class name. Called by DirectUI element builder.
    HRESULT CreateBehavior(LPCWSTR pszClassName, IDuiBehavior** ppBehavior)
    {
        if (!ppBehavior)
            return E_POINTER;

        *ppBehavior = NULL;

        if (!pszClassName || !m_bRegistered)
            return E_FAIL;

        FactoryMap::iterator it = m_factoryMap.find(std::wstring(pszClassName));
        if (it == m_factoryMap.end())
        {
            // Unknown behavior class name
            ATLTRACE(L"SundanceBehaviorFactory: unknown behavior class '%s'\n", pszClassName);
            return CLASS_E_CLASSNOTAVAILABLE;
        }

        return it->second(ppBehavior);
    }

    // Check if a behavior class is registered
    bool IsRegistered(LPCWSTR pszClassName) const
    {
        if (!pszClassName || !m_bRegistered)
            return false;

        return m_factoryMap.find(std::wstring(pszClassName)) != m_factoryMap.end();
    }

    // Get the count of registered behaviors
    size_t GetRegisteredCount() const
    {
        return m_bRegistered ? m_factoryMap.size() : 0;
    }

private:
    typedef std::map<std::wstring, FACTORYFN> FactoryMap;
    FactoryMap  m_factoryMap;
    bool        m_bRegistered;
};

// ============================================================================
// Module-level initialization function
// ============================================================================
// Called from SundanceAppMain::InitializeUI() to register all behaviors
// before any .duxt resources are loaded.
//
HRESULT RegisterSundanceBehaviors()
{
    return SundanceBehaviorFactory::Instance().RegisterAll();
}

// ============================================================================
// Behavior creation entry point
// ============================================================================
// Called by the DirectUI element builder when instantiating behaviors.
//
HRESULT CreateSundanceBehavior(LPCWSTR pszClassName, IDuiBehavior** ppBehavior)
{
    return SundanceBehaviorFactory::Instance().CreateBehavior(pszClassName, ppBehavior);
}

} // namespace Sundance

// ============================================================================
// Stub factory function implementations
// ============================================================================
// Each function below creates the corresponding behavior class wrapped in
// CComObjectNoLock<IDuiBehaviorImpl> adapter. The actual behavior logic is
// in the respective header/implementation file pair.
//
// These are forward-declared above and resolved at link time.
//

// -- Timeline --
static HRESULT CreateTimelineBehavior(IDuiBehavior** ppBehavior)
{
    if (!ppBehavior) return E_POINTER;
    *ppBehavior = NULL;
    CComObjectNoLock<Sundance::TimelineBehavior>* pObj = new CComObjectNoLock<Sundance::TimelineBehavior>();
    if (!pObj) return E_OUTOFMEMORY;
    pObj->AddRef();
    *ppBehavior = static_cast<IDuiBehavior*>(pObj);
    return S_OK;
}

static HRESULT CreateTimelineItemBehavior(IDuiBehavior** ppBehavior)
{
    if (!ppBehavior) return E_POINTER;
    *ppBehavior = NULL;
    CComObjectNoLock<Sundance::TimelineItemBehavior>* pObj = new CComObjectNoLock<Sundance::TimelineItemBehavior>();
    if (!pObj) return E_OUTOFMEMORY;
    pObj->AddRef();
    *ppBehavior = static_cast<IDuiBehavior*>(pObj);
    return S_OK;
}

static HRESULT CreateTimelineSecondaryTrackItemBehavior(IDuiBehavior** ppBehavior)
{
    if (!ppBehavior) return E_POINTER;
    *ppBehavior = NULL;
    CComObjectNoLock<Sundance::TimelineSecondaryTrackItemBehavior>* pObj = new CComObjectNoLock<Sundance::TimelineSecondaryTrackItemBehavior>();
    if (!pObj) return E_OUTOFMEMORY;
    pObj->AddRef();
    *ppBehavior = static_cast<IDuiBehavior*>(pObj);
    return S_OK;
}

static HRESULT CreateTimelineVisualTrackItemBehavior(IDuiBehavior** ppBehavior)
{
    if (!ppBehavior) return E_POINTER;
    *ppBehavior = NULL;
    CComObjectNoLock<Sundance::TimelineVisualTrackItemBehavior>* pObj = new CComObjectNoLock<Sundance::TimelineVisualTrackItemBehavior>();
    if (!pObj) return E_OUTOFMEMORY;
    pObj->AddRef();
    *ppBehavior = static_cast<IDuiBehavior*>(pObj);
    return S_OK;
}

static HRESULT CreateTimelineItemInputBehavior(IDuiBehavior** ppBehavior)
{
    if (!ppBehavior) return E_POINTER;
    *ppBehavior = NULL;
    CComObjectNoLock<Sundance::TimelineItemInputBehavior>* pObj = new CComObjectNoLock<Sundance::TimelineItemInputBehavior>();
    if (!pObj) return E_OUTOFMEMORY;
    pObj->AddRef();
    *ppBehavior = static_cast<IDuiBehavior*>(pObj);
    return S_OK;
}

static HRESULT CreateTimelineInstructionsBehavior(IDuiBehavior** ppBehavior)
{
    if (!ppBehavior) return E_POINTER;
    *ppBehavior = NULL;
    CComObjectNoLock<Sundance::TimelineInstructionsBehavior>* pObj = new CComObjectNoLock<Sundance::TimelineInstructionsBehavior>();
    if (!pObj) return E_OUTOFMEMORY;
    pObj->AddRef();
    *ppBehavior = static_cast<IDuiBehavior*>(pObj);
    return S_OK;
}

static HRESULT CreateTimelineSelectionRootBehavior(IDuiBehavior** ppBehavior)
{
    if (!ppBehavior) return E_POINTER;
    *ppBehavior = NULL;
    CComObjectNoLock<Sundance::TimelineSelectionRootBehavior>* pObj = new CComObjectNoLock<Sundance::TimelineSelectionRootBehavior>();
    if (!pObj) return E_OUTOFMEMORY;
    pObj->AddRef();
    *ppBehavior = static_cast<IDuiBehavior*>(pObj);
    return S_OK;
}

// -- UI Components --
static HRESULT CreateSundanceMainElementBehavior(IDuiBehavior** ppBehavior)
{
    if (!ppBehavior) return E_POINTER;
    *ppBehavior = NULL;
    CComObjectNoLock<Sundance::SundanceMainElementBehavior>* pObj = new CComObjectNoLock<Sundance::SundanceMainElementBehavior>();
    if (!pObj) return E_OUTOFMEMORY;
    pObj->AddRef();
    *ppBehavior = static_cast<IDuiBehavior*>(pObj);
    return S_OK;
}

static HRESULT CreateAMPMainWindowBehavior(IDuiBehavior** ppBehavior)
{
    if (!ppBehavior) return E_POINTER;
    *ppBehavior = NULL;
    CComObjectNoLock<Sundance::AMPMainWindowBehavior>* pObj = new CComObjectNoLock<Sundance::AMPMainWindowBehavior>();
    if (!pObj) return E_OUTOFMEMORY;
    pObj->AddRef();
    *ppBehavior = static_cast<IDuiBehavior*>(pObj);
    return S_OK;
}

static HRESULT CreateAMPCommandBarBehavior(IDuiBehavior** ppBehavior)
{
    if (!ppBehavior) return E_POINTER;
    *ppBehavior = NULL;
    CComObjectNoLock<Sundance::AMPCommandBarBehavior>* pObj = new CComObjectNoLock<Sundance::AMPCommandBarBehavior>();
    if (!pObj) return E_OUTOFMEMORY;
    pObj->AddRef();
    *ppBehavior = static_cast<IDuiBehavior*>(pObj);
    return S_OK;
}

// -- Dialogs --
static HRESULT CreateRenderSummaryDialog(IDuiBehavior** ppBehavior)
{
    if (!ppBehavior) return E_POINTER;
    *ppBehavior = NULL;
    CComObjectNoLock<Sundance::RenderSummaryDialog>* pObj = new CComObjectNoLock<Sundance::RenderSummaryDialog>();
    if (!pObj) return E_OUTOFMEMORY;
    pObj->AddRef();
    *ppBehavior = static_cast<IDuiBehavior*>(pObj);
    return S_OK;
}

static HRESULT CreateMultipleEffectDialog(IDuiBehavior** ppBehavior)
{
    if (!ppBehavior) return E_POINTER;
    *ppBehavior = NULL;
    CComObjectNoLock<Sundance::MultipleEffectDialog>* pObj = new CComObjectNoLock<Sundance::MultipleEffectDialog>();
    if (!pObj) return E_OUTOFMEMORY;
    pObj->AddRef();
    *ppBehavior = static_cast<IDuiBehavior*>(pObj);
    return S_OK;
}

static HRESULT CreateSundanceApplicationOptionsDialog(IDuiBehavior** ppBehavior)
{
    if (!ppBehavior) return E_POINTER;
    *ppBehavior = NULL;
    CComObjectNoLock<Sundance::SundanceApplicationOptionsDialog>* pObj = new CComObjectNoLock<Sundance::SundanceApplicationOptionsDialog>();
    if (!pObj) return E_OUTOFMEMORY;
    pObj->AddRef();
    *ppBehavior = static_cast<IDuiBehavior*>(pObj);
    return S_OK;
}

// -- Slider --
static HRESULT CreatePopUpSlider(IDuiBehavior** ppBehavior)
{
    if (!ppBehavior) return E_POINTER;
    *ppBehavior = NULL;
    CComObjectNoLock<Sundance::PopUpSlider>* pObj = new CComObjectNoLock<Sundance::PopUpSlider>();
    if (!pObj) return E_OUTOFMEMORY;
    pObj->AddRef();
    *ppBehavior = static_cast<IDuiBehavior*>(pObj);
    return S_OK;
}

// -- Progress --
static HRESULT CreateProgressStatusBar(IDuiBehavior** ppBehavior)
{
    if (!ppBehavior) return E_POINTER;
    *ppBehavior = NULL;
    CComObjectNoLock<Sundance::ProgressStatusBar>* pObj = new CComObjectNoLock<Sundance::ProgressStatusBar>();
    if (!pObj) return E_OUTOFMEMORY;
    pObj->AddRef();
    *ppBehavior = static_cast<IDuiBehavior*>(pObj);
    return S_OK;
}

static HRESULT CreateProgressDialog(IDuiBehavior** ppBehavior)
{
    if (!ppBehavior) return E_POINTER;
    *ppBehavior = NULL;
    CComObjectNoLock<Sundance::ProgressDialog>* pObj = new CComObjectNoLock<Sundance::ProgressDialog>();
    if (!pObj) return E_OUTOFMEMORY;
    pObj->AddRef();
    *ppBehavior = static_cast<IDuiBehavior*>(pObj);
    return S_OK;
}

// -- Editing --
static HRESULT CreateTrimBehavior(IDuiBehavior** ppBehavior)
{
    if (!ppBehavior) return E_POINTER;
    *ppBehavior = NULL;
    CComObjectNoLock<Sundance::TrimBehavior>* pObj = new CComObjectNoLock<Sundance::TrimBehavior>();
    if (!pObj) return E_OUTOFMEMORY;
    pObj->AddRef();
    *ppBehavior = static_cast<IDuiBehavior*>(pObj);
    return S_OK;
}

static HRESULT CreateTextBoxBehavior(IDuiBehavior** ppBehavior)
{
    if (!ppBehavior) return E_POINTER;
    *ppBehavior = NULL;
    CComObjectNoLock<Sundance::TextBoxBehavior>* pObj = new CComObjectNoLock<Sundance::TextBoxBehavior>();
    if (!pObj) return E_OUTOFMEMORY;
    pObj->AddRef();
    *ppBehavior = static_cast<IDuiBehavior*>(pObj);
    return S_OK;
}

static HRESULT CreateRichEditControlBehavior(IDuiBehavior** ppBehavior)
{
    if (!ppBehavior) return E_POINTER;
    *ppBehavior = NULL;
    CComObjectNoLock<Sundance::RichEditControlBehavior>* pObj = new CComObjectNoLock<Sundance::RichEditControlBehavior>();
    if (!pObj) return E_OUTOFMEMORY;
    pObj->AddRef();
    *ppBehavior = static_cast<IDuiBehavior*>(pObj);
    return S_OK;
}

static HRESULT CreateCaretBehavior(IDuiBehavior** ppBehavior)
{
    if (!ppBehavior) return E_POINTER;
    *ppBehavior = NULL;
    CComObjectNoLock<Sundance::CaretBehavior>* pObj = new CComObjectNoLock<Sundance::CaretBehavior>();
    if (!pObj) return E_OUTOFMEMORY;
    pObj->AddRef();
    *ppBehavior = static_cast<IDuiBehavior*>(pObj);
    return S_OK;
}

static HRESULT CreateCaretScrollBehavior(IDuiBehavior** ppBehavior)
{
    if (!ppBehavior) return E_POINTER;
    *ppBehavior = NULL;
    CComObjectNoLock<Sundance::CaretScrollBehavior>* pObj = new CComObjectNoLock<Sundance::CaretScrollBehavior>();
    if (!pObj) return E_OUTOFMEMORY;
    pObj->AddRef();
    *ppBehavior = static_cast<IDuiBehavior*>(pObj);
    return S_OK;
}

static HRESULT CreateAboveHomerBehavior(IDuiBehavior** ppBehavior)
{
    if (!ppBehavior) return E_POINTER;
    *ppBehavior = NULL;
    CComObjectNoLock<Sundance::AboveHomerBehavior>* pObj = new CComObjectNoLock<Sundance::AboveHomerBehavior>();
    if (!pObj) return E_OUTOFMEMORY;
    pObj->AddRef();
    *ppBehavior = static_cast<IDuiBehavior*>(pObj);
    return S_OK;
}

static HRESULT CreateHelpBehavior(IDuiBehavior** ppBehavior)
{
    if (!ppBehavior) return E_POINTER;
    *ppBehavior = NULL;
    CComObjectNoLock<Sundance::HelpBehavior>* pObj = new CComObjectNoLock<Sundance::HelpBehavior>();
    if (!pObj) return E_OUTOFMEMORY;
    pObj->AddRef();
    *ppBehavior = static_cast<IDuiBehavior*>(pObj);
    return S_OK;
}

static HRESULT CreateCaptureUIBehavior(IDuiBehavior** ppBehavior)
{
    if (!ppBehavior) return E_POINTER;
    *ppBehavior = NULL;
    CComObjectNoLock<Sundance::CaptureUIBehavior>* pObj = new CComObjectNoLock<Sundance::CaptureUIBehavior>();
    if (!pObj) return E_OUTOFMEMORY;
    pObj->AddRef();
    *ppBehavior = static_cast<IDuiBehavior*>(pObj);
    return S_OK;
}

// -- Layout --
static HRESULT CreateFullscreenLayoutBehavior(IDuiBehavior** ppBehavior)
{
    if (!ppBehavior) return E_POINTER;
    *ppBehavior = NULL;
    CComObjectNoLock<Sundance::FullscreenLayoutBehavior>* pObj = new CComObjectNoLock<Sundance::FullscreenLayoutBehavior>();
    if (!pObj) return E_OUTOFMEMORY;
    pObj->AddRef();
    *ppBehavior = static_cast<IDuiBehavior*>(pObj);
    return S_OK;
}

static HRESULT CreateResizeablePaneBehavior(IDuiBehavior** ppBehavior)
{
    if (!ppBehavior) return E_POINTER;
    *ppBehavior = NULL;
    CComObjectNoLock<Sundance::ResizeablePaneBehavior>* pObj = new CComObjectNoLock<Sundance::ResizeablePaneBehavior>();
    if (!pObj) return E_OUTOFMEMORY;
    pObj->AddRef();
    *ppBehavior = static_cast<IDuiBehavior*>(pObj);
    return S_OK;
}

static HRESULT CreateProjectWorkspaceLayoutBehavior(IDuiBehavior** ppBehavior)
{
    if (!ppBehavior) return E_POINTER;
    *ppBehavior = NULL;
    CComObjectNoLock<Sundance::ProjectWorkspaceLayoutBehavior>* pObj = new CComObjectNoLock<Sundance::ProjectWorkspaceLayoutBehavior>();
    if (!pObj) return E_OUTOFMEMORY;
    pObj->AddRef();
    *ppBehavior = static_cast<IDuiBehavior*>(pObj);
    return S_OK;
}

static HRESULT CreateInlinePreviewLayoutBehavior(IDuiBehavior** ppBehavior)
{
    if (!ppBehavior) return E_POINTER;
    *ppBehavior = NULL;
    CComObjectNoLock<Sundance::InlinePreviewLayoutBehavior>* pObj = new CComObjectNoLock<Sundance::InlinePreviewLayoutBehavior>();
    if (!pObj) return E_OUTOFMEMORY;
    pObj->AddRef();
    *ppBehavior = static_cast<IDuiBehavior*>(pObj);
    return S_OK;
}

static HRESULT CreateInlinePreviewSliderBehavior(IDuiBehavior** ppBehavior)
{
    if (!ppBehavior) return E_POINTER;
    *ppBehavior = NULL;
    CComObjectNoLock<Sundance::InlinePreviewSliderBehavior>* pObj = new CComObjectNoLock<Sundance::InlinePreviewSliderBehavior>();
    if (!pObj) return E_OUTOFMEMORY;
    pObj->AddRef();
    *ppBehavior = static_cast<IDuiBehavior*>(pObj);
    return S_OK;
}

// -- Special --
static HRESULT CreateHomerHeavyLayerBehavior(IDuiBehavior** ppBehavior)
{
    if (!ppBehavior) return E_POINTER;
    *ppBehavior = NULL;
    CComObjectNoLock<Sundance::HomerHeavyLayerBehavior>* pObj = new CComObjectNoLock<Sundance::HomerHeavyLayerBehavior>();
    if (!pObj) return E_OUTOFMEMORY;
    pObj->AddRef();
    *ppBehavior = static_cast<IDuiBehavior*>(pObj);
    return S_OK;
}

static HRESULT CreateStandardLayerBehavior(IDuiBehavior** ppBehavior)
{
    if (!ppBehavior) return E_POINTER;
    *ppBehavior = NULL;
    CComObjectNoLock<Sundance::StandardLayerBehavior>* pObj = new CComObjectNoLock<Sundance::StandardLayerBehavior>();
    if (!pObj) return E_OUTOFMEMORY;
    pObj->AddRef();
    *ppBehavior = static_cast<IDuiBehavior*>(pObj);
    return S_OK;
}

static HRESULT CreateMultipleEffectBehavior(IDuiBehavior** ppBehavior)
{
    if (!ppBehavior) return E_POINTER;
    *ppBehavior = NULL;
    CComObjectNoLock<Sundance::MultipleEffectBehavior>* pObj = new CComObjectNoLock<Sundance::MultipleEffectBehavior>();
    if (!pObj) return E_OUTOFMEMORY;
    pObj->AddRef();
    *ppBehavior = static_cast<IDuiBehavior*>(pObj);
    return S_OK;
}

static HRESULT CreateWebcamElementBehavior(IDuiBehavior** ppBehavior)
{
    if (!ppBehavior) return E_POINTER;
    *ppBehavior = NULL;
    CComObjectNoLock<Sundance::WebcamElementBehavior>* pObj = new CComObjectNoLock<Sundance::WebcamElementBehavior>();
    if (!pObj) return E_OUTOFMEMORY;
    pObj->AddRef();
    *ppBehavior = static_cast<IDuiBehavior*>(pObj);
    return S_OK;
}

static HRESULT CreateAmpPreventSparseModeBehavior(IDuiBehavior** ppBehavior)
{
    if (!ppBehavior) return E_POINTER;
    *ppBehavior = NULL;
    CComObjectNoLock<Sundance::AmpPreventSparseModeBehavior>* pObj = new CComObjectNoLock<Sundance::AmpPreventSparseModeBehavior>();
    if (!pObj) return E_OUTOFMEMORY;
    pObj->AddRef();
    *ppBehavior = static_cast<IDuiBehavior*>(pObj);
    return S_OK;
}

static HRESULT CreateAmpSlideAnimationBehavior(IDuiBehavior** ppBehavior)
{
    if (!ppBehavior) return E_POINTER;
    *ppBehavior = NULL;
    CComObjectNoLock<Sundance::AmpSlideAnimationBehavior>* pObj = new CComObjectNoLock<Sundance::AmpSlideAnimationBehavior>();
    if (!pObj) return E_OUTOFMEMORY;
    pObj->AddRef();
    *ppBehavior = static_cast<IDuiBehavior*>(pObj);
    return S_OK;
}

static HRESULT CreateAmpFadeAnimationBehavior(IDuiBehavior** ppBehavior)
{
    if (!ppBehavior) return E_POINTER;
    *ppBehavior = NULL;
    CComObjectNoLock<Sundance::AmpFadeAnimationBehavior>* pObj = new CComObjectNoLock<Sundance::AmpFadeAnimationBehavior>();
    if (!pObj) return E_OUTOFMEMORY;
    pObj->AddRef();
    *ppBehavior = static_cast<IDuiBehavior*>(pObj);
    return S_OK;
}

static HRESULT CreateUserEncodeProfileBehavior(IDuiBehavior** ppBehavior)
{
    if (!ppBehavior) return E_POINTER;
    *ppBehavior = NULL;
    CComObjectNoLock<Sundance::UserEncodeProfileBehavior>* pObj = new CComObjectNoLock<Sundance::UserEncodeProfileBehavior>();
    if (!pObj) return E_OUTOFMEMORY;
    pObj->AddRef();
    *ppBehavior = static_cast<IDuiBehavior*>(pObj);
    return S_OK;
}

static HRESULT CreateOptionsDialogBehavior(IDuiBehavior** ppBehavior)
{
    if (!ppBehavior) return E_POINTER;
    *ppBehavior = NULL;
    CComObjectNoLock<Sundance::OptionsDialogBehavior>* pObj = new CComObjectNoLock<Sundance::OptionsDialogBehavior>();
    if (!pObj) return E_OUTOFMEMORY;
    pObj->AddRef();
    *ppBehavior = static_cast<IDuiBehavior*>(pObj);
    return S_OK;
}
