#include "pch.h"

/*
 * DragDropStub.cpp
 *
 * Implementation of DynamicDataObjectWrapper and ClipboardChainWindow.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "DragDropStub.h"

// ============================================================================
// DynamicDataObjectWrapper implementation
// ============================================================================

DynamicDataObjectWrapper::DynamicDataObjectWrapper()
    : m_cRef(1)
{
}

DynamicDataObjectWrapper::~DynamicDataObjectWrapper()
{
    // Release stored data
    for (auto& entry : m_dataEntries)
    {
        if (entry.hData)
            ::GlobalFree(entry.hData);
    }
    m_dataEntries.clear();
}

STDMETHODIMP DynamicDataObjectWrapper::QueryInterface(REFIID riid, void** ppvObject)
{
    if (!ppvObject)
        return E_POINTER;

    *ppvObject = nullptr;

    if (riid == IID_IUnknown || riid == IID_IDataObject)
    {
        *ppvObject = static_cast<IDataObject*>(this);
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) DynamicDataObjectWrapper::AddRef()
{
    return static_cast<ULONG>(InterlockedIncrement(&m_cRef));
}

STDMETHODIMP_(ULONG) DynamicDataObjectWrapper::Release()
{
    long refCount = InterlockedDecrement(&m_cRef);
    if (refCount == 0)
    {
        delete this;
        return 0;
    }
    return static_cast<ULONG>(refCount);
}

STDMETHODIMP DynamicDataObjectWrapper::GetData(FORMATETC* pformatetcIn, STGMEDIUM* pmedium)
{
    if (!pformatetcIn || !pmedium)
        return E_POINTER;

    ZeroMemory(pmedium, sizeof(*pmedium));

    // Check if we have data in the requested format
    for (const auto& entry : m_dataEntries)
    {
        if (entry.uClipFormat == pformatetcIn->cfFormat && entry.hData)
        {
            // Duplicate the global handle
            SIZE_T size = ::GlobalSize(entry.hData);
            HANDLE hNew = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, size);
            if (!hNew)
                return E_OUTOFMEMORY;

            void* pSrc = ::GlobalLock(entry.hData);
            void* pDst = ::GlobalLock(hNew);
            if (pSrc && pDst)
            {
                CopyMemory(pDst, pSrc, size);
            }
            if (pSrc) ::GlobalUnlock(entry.hData);
            if (pDst) ::GlobalUnlock(hNew);

            pmedium->tymed = TYMED_HGLOBAL;
            pmedium->hGlobal = hNew;
            pmedium->pUnkForRelease = nullptr;

            return S_OK;
        }
    }

    return DV_E_FORMATETC;
}

STDMETHODIMP DynamicDataObjectWrapper::GetDataHere(FORMATETC* pformatetc, STGMEDIUM* pmedium)
{
    UNREFERENCED_PARAMETER(pformatetc);
    UNREFERENCED_PARAMETER(pmedium);
    // TODO(reconstruction): Support caller-provided storage for compatible clipboard formats.
    return E_NOTIMPL;
}

STDMETHODIMP DynamicDataObjectWrapper::QueryGetData(FORMATETC* pformatetc)
{
    if (!pformatetc)
        return E_INVALIDARG;

    for (const auto& entry : m_dataEntries)
    {
        if (entry.uClipFormat == pformatetc->cfFormat)
            return S_OK;
    }

    return DV_E_FORMATETC;
}

STDMETHODIMP DynamicDataObjectWrapper::GetCanonicalFormatEtc(FORMATETC* pformatectIn, FORMATETC* pformatetcOut)
{
    if (!pformatectIn || !pformatetcOut)
        return E_POINTER;

    *pformatetcOut = *pformatectIn;
    pformatetcOut->ptd = nullptr;
    return DATA_S_SAMEFORMATETC;
}

STDMETHODIMP DynamicDataObjectWrapper::SetData(FORMATETC* pformatetc, STGMEDIUM* pmedium, BOOL fRelease)
{
    if (!pformatetc || !pmedium)
        return E_POINTER;

    // Check if format already exists
    for (auto& entry : m_dataEntries)
    {
        if (entry.uClipFormat == pformatetc->cfFormat)
        {
            if (entry.hData)
                ::GlobalFree(entry.hData);
            entry.hData = fRelease ? pmedium->hGlobal : ::GlobalAlloc(GMEM_MOVEABLE, 0);
            return S_OK;
        }
    }

    DataEntry entry;
    entry.uClipFormat = pformatetc->cfFormat;
    entry.hData = fRelease ? pmedium->hGlobal : ::GlobalAlloc(GMEM_MOVEABLE, 0);
    m_dataEntries.push_back(entry);

    return S_OK;
}

STDMETHODIMP DynamicDataObjectWrapper::EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC** ppenumFormatEtc)
{
    UNREFERENCED_PARAMETER(dwDirection);
    UNREFERENCED_PARAMETER(ppenumFormatEtc);
    // TODO(reconstruction): Return an IEnumFORMATETC over m_dataEntries for drag/drop consumers.
    return E_NOTIMPL;
}

STDMETHODIMP DynamicDataObjectWrapper::DAdvise(FORMATETC* pformatetc, DWORD advf, IAdviseSink* pAdvSink, DWORD* pdwConnection)
{
    UNREFERENCED_PARAMETER(pformatetc);
    UNREFERENCED_PARAMETER(advf);
    UNREFERENCED_PARAMETER(pAdvSink);
    UNREFERENCED_PARAMETER(pdwConnection);
    return OLE_E_ADVISENOTSUPPORTED;
}

STDMETHODIMP DynamicDataObjectWrapper::DUnadvise(DWORD dwConnection)
{
    UNREFERENCED_PARAMETER(dwConnection);
    return OLE_E_ADVISENOTSUPPORTED;
}

STDMETHODIMP DynamicDataObjectWrapper::EnumDAdvise(IEnumSTATDATA** ppenumAdvise)
{
    UNREFERENCED_PARAMETER(ppenumAdvise);
    return OLE_E_ADVISENOTSUPPORTED;
}

HRESULT DynamicDataObjectWrapper::SetClipboardData(UINT uClipFormat, HANDLE hData)
{
    for (auto& entry : m_dataEntries)
    {
        if (entry.uClipFormat == uClipFormat)
        {
            if (entry.hData)
                ::GlobalFree(entry.hData);
            entry.hData = hData;
            return S_OK;
        }
    }

    DataEntry entry;
    entry.uClipFormat = uClipFormat;
    entry.hData = hData;
    m_dataEntries.push_back(entry);
    return S_OK;
}

HANDLE DynamicDataObjectWrapper::GetClipboardData(UINT uClipFormat) const
{
    for (const auto& entry : m_dataEntries)
    {
        if (entry.uClipFormat == uClipFormat)
            return entry.hData;
    }
    return nullptr;
}

// ============================================================================
// ClipboardChainWindow implementation
// ============================================================================

ClipboardChainWindow::ClipboardChainWindow()
    : m_hWnd(nullptr)
    , m_hWndNextViewer(nullptr)
    , m_bInitialized(false)
{
}

ClipboardChainWindow::~ClipboardChainWindow()
{
    Shutdown();
}

HRESULT ClipboardChainWindow::Initialize(HWND hWndParent)
{
    if (m_bInitialized)
        return S_FALSE;

    // Register window class if not yet registered
    static const LPCWSTR kClassName = L"ClipboardChainWindowClass";
    static bool s_classRegistered = false;

    if (!s_classRegistered)
    {
        WNDCLASS wc = {};
        wc.lpfnWndProc = WndProc;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.lpszClassName = kClassName;
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        RegisterClass(&wc);
        s_classRegistered = true;
    }

    // Create the hidden window
    m_hWnd = CreateWindowW(kClassName, L"ClipboardChainWindow", WS_OVERLAPPED,
                            0, 0, 0, 0, hWndParent, nullptr,
                            GetModuleHandle(nullptr), this);

    if (!m_hWnd)
        return HRESULT_FROM_WIN32(GetLastError());

    // Set this instance as window user data
    SetWindowLongPtr(m_hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

    // Join the clipboard viewer chain
    m_hWndNextViewer = SetClipboardViewer(m_hWnd);
    m_bInitialized = true;

    return S_OK;
}

void ClipboardChainWindow::Shutdown()
{
    if (!m_bInitialized)
        return;

    if (m_hWnd)
    {
        ChangeClipboardChain(m_hWnd, m_hWndNextViewer);
        DestroyWindow(m_hWnd);
        m_hWnd = nullptr;
    }

    m_hWndNextViewer = nullptr;
    m_bInitialized = false;
}

bool ClipboardChainWindow::IsInitialized() const throw()
{
    return m_bInitialized;
}

void ClipboardChainWindow::SetClipboardChangeCallback(std::function<void()> fn)
{
    m_fnClipboardChange = std::move(fn);
}

bool ClipboardChainWindow::HasClipboardData(UINT uClipFormat) const
{
    if (!OpenClipboard(m_hWnd))
        return false;

    bool hasData = (GetClipboardData(uClipFormat) != nullptr);
    CloseClipboard();
    return hasData;
}

HANDLE ClipboardChainWindow::GetClipboardData(UINT uClipFormat) const
{
    if (!OpenClipboard(m_hWnd))
        return nullptr;

    HANDLE hData = ::GetClipboardData(uClipFormat);
    CloseClipboard();
    return hData;
}

LRESULT CALLBACK ClipboardChainWindow::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    ClipboardChainWindow* pThis = reinterpret_cast<ClipboardChainWindow*>(
        GetWindowLongPtr(hWnd, GWLP_USERDATA));

    if (pThis)
        return pThis->OnMessage(uMsg, wParam, lParam);

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

LRESULT ClipboardChainWindow::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CHANGECBCHAIN:
    {
        if (reinterpret_cast<HWND>(wParam) == m_hWndNextViewer)
            m_hWndNextViewer = reinterpret_cast<HWND>(lParam);

        if (m_hWndNextViewer)
            SendMessage(m_hWndNextViewer, uMsg, wParam, lParam);

        return 0;
    }

    case WM_DRAWCLIPBOARD:
    {
        if (m_fnClipboardChange)
            m_fnClipboardChange();

        // Pass to next viewer in chain
        if (m_hWndNextViewer)
            SendMessage(m_hWndNextViewer, uMsg, wParam, lParam);

        return 0;
    }
    }

    return DefWindowProc(m_hWnd, uMsg, wParam, lParam);
}
