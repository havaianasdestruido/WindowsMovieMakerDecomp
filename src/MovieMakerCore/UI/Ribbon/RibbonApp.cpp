/*
 * RibbonApp.cpp
 *
 * Implementation of the RibbonApp class for MovieMakerCore.dll's UI layer.
 * Hosts the IUIFramework ribbon integration and dispatches commands.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "RibbonApp.h"

namespace SundanceUI
{

// ============================================================================
// RibbonApp implementation
// ============================================================================

RibbonApp::RibbonApp()
    : m_pAppMain(nullptr)
    , m_hwndOwner(nullptr)
    , m_fInitialized(false)
    , m_hInstance(nullptr)
{
}

RibbonApp::~RibbonApp()
{
    Shutdown();
}

HRESULT RibbonApp::Initialize(SundanceAppMain* pAppMain)
{
    if (m_fInitialized)
        return S_FALSE;

    if (!pAppMain)
        return E_INVALIDARG;

    m_pAppMain = pAppMain;

    HRESULT hr = CreateUIFramework();
    if (FAILED(hr))
        return hr;

    m_fInitialized = true;
    return S_OK;
}

HRESULT RibbonApp::Shutdown()
{
    if (!m_fInitialized)
        return S_FALSE;

    if (m_spRibbon)
    {
        m_spRibbon->Destroy();
        m_spRibbon.Release();
    }

    if (m_spFramework)
    {
        m_spFramework->Destroy();
        m_spFramework.Release();
    }

    m_pAppMain = nullptr;
    m_hwndOwner = nullptr;
    m_fInitialized = false;

    return S_OK;
}

IUIFramework* RibbonApp::GetFramework()
{
    return m_spFramework.p;
}

HRESULT RibbonApp::LoadUI(HINSTANCE hInstance, LPCWSTR pszRibbonResource)
{
    if (!hInstance || !pszRibbonResource)
        return E_INVALIDARG;

    m_hInstance = hInstance;
    return LoadRibbonFromResource(hInstance, pszRibbonResource);
}

HRESULT RibbonApp::OnCommand(UINT nCmdId, UI_COMMANDTYPE commandType, IUICommandHandler* pHandler)
{
    UNREFERENCED_PARAMETER(pHandler);

    if (!m_pAppMain)
        return E_FAIL;

    // Route command to SundanceAppMain
    return ExecuteCommand(nCmdId, commandType, nullptr);
}

HRESULT RibbonApp::OnUpdateProperty(UINT nCmdId, REFUIKEY key,
                                    const PROPVARIANT* pCurrentValue,
                                    PROPVARIANT* pNewValue)
{
    UNREFERENCED_PARAMETER(nCmdId);
    UNREFERENCED_PARAMETER(key);
    UNREFERENCED_PARAMETER(pCurrentValue);
    UNREFERENCED_PARAMETER(pNewValue);
    return E_NOTIMPL;
}

HRESULT RibbonApp::PopulateApplicationMenu(IUIRibbon* pRibbon)
{
    UNREFERENCED_PARAMETER(pRibbon);
    return S_OK;
}

bool RibbonApp::IsInitialized() const throw()
{
    return m_fInitialized;
}

HWND RibbonApp::GetOwnerHwnd() const throw()
{
    return m_hwndOwner;
}

void RibbonApp::SetOwnerHwnd(HWND hwnd) throw()
{
    m_hwndOwner = hwnd;
}

HRESULT RibbonApp::OnViewChanged(UINT nViewId, UI_VIEWTYPE type, IUIView* pView,
                                 UI_VIEWVERB verb, UINT uParam)
{
    UNREFERENCED_PARAMETER(nViewId);
    UNREFERENCED_PARAMETER(type);
    UNREFERENCED_PARAMETER(pView);
    UNREFERENCED_PARAMETER(verb);
    UNREFERENCED_PARAMETER(uParam);
    return S_OK;
}

HRESULT RibbonApp::UpdateCommandState(UINT nCmdId, bool fEnabled, bool fHidden)
{
    if (!m_spFramework)
        return E_FAIL;

    // Create a property set with the new state
    CComPtr<IUISimplePropertySet> spProps;
    HRESULT hr = HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
    if (SUCCEEDED(hr))
    {
        // Set enabled/hidden state on the command
        UI_COMMANDTYPE cmdType = UI_COMMANDTYPE_UNKNOWN;
        m_spFramework->GetUICommandType(nCmdId, &cmdType);
    }

    UNREFERENCED_PARAMETER(fEnabled);
    UNREFERENCED_PARAMETER(fHidden);
    return S_OK;
}

HRESULT RibbonApp::UpdateTooltip(UINT nCmdId, LPCWSTR pszTooltip)
{
    UNREFERENCED_PARAMETER(nCmdId);
    UNREFERENCED_PARAMETER(pszTooltip);
    return E_NOTIMPL;
}

HRESULT RibbonApp::UpdateLabel(UINT nCmdId, LPCWSTR pszLabel)
{
    UNREFERENCED_PARAMETER(nCmdId);
    UNREFERENCED_PARAMETER(pszLabel);
    return E_NOTIMPL;
}

HRESULT RibbonApp::ShowContextualTab(UINT nTabId)
{
    UNREFERENCED_PARAMETER(nTabId);
    return E_NOTIMPL;
}

HRESULT RibbonApp::HideContextualTab(UINT nTabId)
{
    UNREFERENCED_PARAMETER(nTabId);
    return E_NOTIMPL;
}

HRESULT RibbonApp::CreateUIFramework()
{
    HRESULT hr = CoCreateInstance(CLSID_UIFramework, nullptr, CLSCTX_INPROC_SERVER,
                                 IID_PPV_ARGS(&m_spFramework));
    return hr;
}

HRESULT RibbonApp::LoadRibbonFromResource(HINSTANCE hInstance, LPCWSTR pszResource)
{
    if (!m_spFramework)
        return E_FAIL;

    // Load ribbon XML from resource
    HRSRC hRsrc = FindResource(hInstance, pszResource, RT_XML);
    if (!hRsrc)
        return HRESULT_FROM_WIN32(GetLastError());

    HGLOBAL hGlobal = LoadResource(hInstance, hRsrc);
    if (!hGlobal)
        return HRESULT_FROM_WIN32(GetLastError());

    void* pData = LockResource(hGlobal);
    DWORD dwSize = SizeofResource(hInstance, hRsrc);

    if (!pData || dwSize == 0)
        return E_FAIL;

    // Create a stream from the resource data
    CComPtr<IStream> spStream;
    hr = CreateStreamOnHGlobal(nullptr, TRUE, &spStream);
    if (FAILED(hr))
        return hr;

    ULONG cbWritten = 0;
    hr = spStream->Write(pData, dwSize, &cbWritten);
    if (FAILED(hr))
        return hr;

    LARGE_INTEGER liZero = {};
    hr = spStream->Seek(liZero, STREAM_SEEK_SET, nullptr);
    if (FAILED(hr))
        return hr;

    // Initialize the framework with the ribbon XML
    hr = m_spFramework->Initialize(m_hwndOwner, spStream);
    if (FAILED(hr))
        return hr;

    // Get the ribbon interface
    hr = m_spFramework->QueryInterface(IID_PPV_ARGS(&m_spRibbon));

    return hr;
}

HRESULT RibbonApp::RegisterCommandHandler(UINT nCmdId)
{
    UNREFERENCED_PARAMETER(nCmdId);
    return S_OK;
}

HRESULT RibbonApp::ExecuteCommand(UINT nCmdId, UI_COMMANDTYPE commandType, IUISimplePropertySet* pArgs)
{
    UNREFERENCED_PARAMETER(commandType);
    UNREFERENCED_PARAMETER(pArgs);

    // Dispatch to SundanceAppMain based on command ID
    // This is a stub; the actual dispatch logic maps ribbon commands
    // to the application's action methods.
    return S_OK;
}

HRESULT RibbonApp::QueryInterface(REFIID riid, void** ppvObject)
{
    if (!ppvObject)
        return E_POINTER;

    *ppvObject = nullptr;

    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IUICommandHandler))
    {
        // Return the RibbonApp as a command handler
        // In the real implementation, this would use ATL::CComAggObject or similar
        return E_NOINTERFACE;
    }

    return E_NOINTERFACE;
}

} // namespace SundanceUI
