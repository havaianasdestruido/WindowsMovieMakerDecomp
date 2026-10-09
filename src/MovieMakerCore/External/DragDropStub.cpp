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
// CEnumFormatEtc
// ============================================================================
// IEnumFORMATETC over a snapshot of FORMATETC entries. The snapshot keeps the
// enumerator stable even if the underlying data entries change while the
// enumerator is alive.
//
class CEnumFormatEtc : public IEnumFORMATETC
{
public:
    explicit CEnumFormatEtc(const std::vector<FORMATETC>& entries)
        : m_cRef(1)
        , m_entries(entries)
        , m_uIndex(0)
    {
    }

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject)
    {
        if (!ppvObject)
            return E_POINTER;

        *ppvObject = nullptr;

        if (riid == IID_IUnknown || riid == IID_IEnumFORMATETC)
        {
            *ppvObject = static_cast<IEnumFORMATETC*>(this);
            AddRef();
            return S_OK;
        }

        return E_NOINTERFACE;
    }

    STDMETHODIMP_(ULONG) AddRef()
    {
        return static_cast<ULONG>(InterlockedIncrement(&m_cRef));
    }

    STDMETHODIMP_(ULONG) Release()
    {
        long refCount = InterlockedDecrement(&m_cRef);
        if (refCount == 0)
        {
            delete this;
            return 0;
        }
        return static_cast<ULONG>(refCount);
    }

    // IEnumFORMATETC
    STDMETHODIMP Next(ULONG celt, FORMATETC* rgelt, ULONG* pceltFetched)
    {
        if (!rgelt)
            return E_POINTER;

        ULONG cFetched = 0;
        while (cFetched < celt && m_uIndex < m_entries.size())
        {
            rgelt[cFetched] = m_entries[m_uIndex];
            // Hand out canonical entries: no target device.
            rgelt[cFetched].ptd = nullptr;
            ++m_uIndex;
            ++cFetched;
        }

        if (pceltFetched)
            *pceltFetched = cFetched;

        return (cFetched == celt) ? S_OK : S_FALSE;
    }

    STDMETHODIMP Skip(ULONG celt)
    {
        // m_uIndex never exceeds m_entries.size(), so this cannot underflow.
        if (celt > m_entries.size() - m_uIndex)
        {
            m_uIndex = static_cast<ULONG>(m_entries.size());
            return S_FALSE;
        }

        m_uIndex += celt;
        return S_OK;
    }

    STDMETHODIMP Reset()
    {
        m_uIndex = 0;
        return S_OK;
    }

    STDMETHODIMP Clone(IEnumFORMATETC** ppenum)
    {
        if (!ppenum)
            return E_POINTER;

        *ppenum = nullptr;

        CEnumFormatEtc* pClone = new CEnumFormatEtc(m_entries);
        pClone->m_uIndex = m_uIndex;

        *ppenum = pClone;
        return S_OK;
    }

private:
    ~CEnumFormatEtc()
    {
    }

    long                    m_cRef;
    std::vector<FORMATETC>  m_entries;
    ULONG                   m_uIndex;
};

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

    // The wrapper stores every entry as a movable global handle.
    if (!(pformatetcIn->tymed & TYMED_HGLOBAL))
        return DV_E_TYMED;

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
    if (!pformatetc || !pmedium)
        return E_POINTER;

    // The wrapper stores every entry as a movable global handle, so only
    // caller-provided HGLOBAL storage can be filled.
    if (!(pformatetc->tymed & TYMED_HGLOBAL) || !(pmedium->tymed & TYMED_HGLOBAL))
        return DV_E_TYMED;

    for (const auto& entry : m_dataEntries)
    {
        if (entry.uClipFormat == pformatetc->cfFormat && entry.hData)
        {
            if (!pmedium->hGlobal)
                return E_HANDLE;

            SIZE_T cbSrc = ::GlobalSize(entry.hData);
            SIZE_T cbDst = ::GlobalSize(pmedium->hGlobal);
            if (cbDst < cbSrc)
                return STG_E_MEDIUMFULL;

            void* pSrc = ::GlobalLock(entry.hData);
            void* pDst = ::GlobalLock(pmedium->hGlobal);
            if (pSrc && pDst)
                CopyMemory(pDst, pSrc, cbSrc);
            if (pSrc) ::GlobalUnlock(entry.hData);
            if (pDst) ::GlobalUnlock(pmedium->hGlobal);

            if (!pSrc || !pDst)
                return E_UNEXPECTED;

            // Ownership of the caller-provided storage stays with the caller.
            pmedium->pUnkForRelease = nullptr;
            return S_OK;
        }
    }

    return DV_E_FORMATETC;
}

STDMETHODIMP DynamicDataObjectWrapper::QueryGetData(FORMATETC* pformatetc)
{
    if (!pformatetc)
        return E_INVALIDARG;

    // The wrapper stores every entry as a movable global handle.
    if (!(pformatetc->tymed & TYMED_HGLOBAL))
        return DV_E_TYMED;

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

    // The wrapper only stores HGLOBAL payloads.
    if (!(pmedium->tymed & TYMED_HGLOBAL) || !pmedium->hGlobal)
        return DV_E_TYMED;

    HANDLE hStored = nullptr;
    if (fRelease)
    {
        // The caller transfers ownership of the medium.
        hStored = pmedium->hGlobal;
    }
    else
    {
        // The caller keeps its medium; copy the payload.
        SIZE_T cb = ::GlobalSize(pmedium->hGlobal);
        hStored = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, cb);
        if (!hStored)
            return E_OUTOFMEMORY;

        void* pSrc = ::GlobalLock(pmedium->hGlobal);
        void* pDst = ::GlobalLock(hStored);
        if (pSrc && pDst)
            CopyMemory(pDst, pSrc, cb);
        if (pSrc) ::GlobalUnlock(pmedium->hGlobal);
        if (pDst) ::GlobalUnlock(hStored);

        if (!pSrc || !pDst)
        {
            ::GlobalFree(hStored);
            return E_UNEXPECTED;
        }
    }

    // Check if format already exists
    for (auto& entry : m_dataEntries)
    {
        if (entry.uClipFormat == pformatetc->cfFormat)
        {
            if (entry.hData)
                ::GlobalFree(entry.hData);
            entry.hData = hStored;
            return S_OK;
        }
    }

    DataEntry entry;
    entry.uClipFormat = pformatetc->cfFormat;
    entry.hData = hStored;
    m_dataEntries.push_back(entry);

    return S_OK;
}

STDMETHODIMP DynamicDataObjectWrapper::EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC** ppenumFormatEtc)
{
    if (!ppenumFormatEtc)
        return E_POINTER;

    *ppenumFormatEtc = nullptr;

    // The format set is fixed by SetData calls; there is nothing to enumerate
    // for a set operation.
    if (dwDirection == DATADIR_SET)
        return E_NOTIMPL;

    // Snapshot the current formats so the enumerator stays stable even if the
    // underlying entries change while it is alive.
    std::vector<FORMATETC> entries;
    entries.reserve(m_dataEntries.size());
    for (const auto& entry : m_dataEntries)
    {
        FORMATETC fmt = {};
        fmt.cfFormat = static_cast<CLIPFORMAT>(entry.uClipFormat);
        fmt.ptd = nullptr;
        fmt.dwAspect = DVASPECT_CONTENT;
        fmt.lindex = -1;
        fmt.tymed = TYMED_HGLOBAL;
        entries.push_back(fmt);
    }

    *ppenumFormatEtc = new CEnumFormatEtc(entries);
    return S_OK;
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
