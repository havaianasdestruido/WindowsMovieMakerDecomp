/*
 * DragDropStub.h
 *
 * Lightweight stubs for drag-drop support classes referenced by MovieMakerCore.
 *
 * DynamicDataObjectWrapper: Wraps an IDataObject for custom drag-drop handling.
 * ClipboardChainWindow: Clipboard chain window for monitoring clipboard changes.
 *
 * RTTI: ?AVDynamicDataObjectWrapper@@, ?AVClipboardChainWindow@@
 *
 * These classes are normally provided by the Windows Live shared components.
 * These stubs provide the RTTI signatures and minimal implementations
 * needed for linking.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once
#ifndef DRAGDROP_STUB_H
#define DRAGDROP_STUB_H

#include "../pch.h"

// ============================================================================
// DynamicDataObjectWrapper
// ============================================================================
// Wraps an IDataObject implementation to provide dynamic clipboard format
// handling. Supports on-the-fly format conversion, virtualized data access,
// and deferred rendering for drag-drop operations.
//
class DynamicDataObjectWrapper : public IDataObject
{
public:
    DynamicDataObjectWrapper();
    virtual ~DynamicDataObjectWrapper();

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject) override;
    STDMETHODIMP_(ULONG) AddRef() override;
    STDMETHODIMP_(ULONG) Release() override;

    // IDataObject
    STDMETHODIMP GetData(FORMATETC* pformatetcIn, STGMEDIUM* pmedium) override;
    STDMETHODIMP GetDataHere(FORMATETC* pformatetc, STGMEDIUM* pmedium) override;
    STDMETHODIMP QueryGetData(FORMATETC* pformatetc) override;
    STDMETHODIMP GetCanonicalFormatEtc(FORMATETC* pformatectIn, FORMATETC* pformatetcOut) override;
    STDMETHODIMP SetData(FORMATETC* pformatetc, STGMEDIUM* pmedium, BOOL fRelease) override;
    STDMETHODIMP EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC** ppenumFormatEtc) override;
    STDMETHODIMP DAdvise(FORMATETC* pformatetc, DWORD advf, IAdviseSink* pAdvSink, DWORD* pdwConnection) override;
    STDMETHODIMP DUnadvise(DWORD dwConnection) override;
    STDMETHODIMP EnumDAdvise(IEnumSTATDATA** ppenumAdvise) override;

    // -- Custom data management --
    HRESULT SetClipboardData(UINT uClipFormat, HANDLE hData);
    HANDLE  GetClipboardData(UINT uClipFormat) const;

private:
    struct DataEntry
    {
        UINT  uClipFormat;
        HANDLE hData;
    };

    long                    m_cRef;
    std::vector<DataEntry>  m_dataEntries;

    DynamicDataObjectWrapper(const DynamicDataObjectWrapper&);
    DynamicDataObjectWrapper& operator=(const DynamicDataObjectWrapper&);
};

// ============================================================================
// ClipboardChainWindow
// ============================================================================
// A hidden window that participates in the Windows clipboard viewer chain.
// Monitors clipboard changes and notifies registered callbacks when the
// clipboard content is updated. Used by the Sundance clipboard manager.
//
class ClipboardChainWindow
{
public:
    ClipboardChainWindow();
    ~ClipboardChainWindow();

    // -- Lifetime --
    HRESULT Initialize(HWND hWndParent);
    void    Shutdown();
    bool    IsInitialized() const throw();

    // -- Callback --
    void SetClipboardChangeCallback(std::function<void()> fn);

    // -- Clipboard access --
    bool HasClipboardData(UINT uClipFormat) const;
    HANDLE GetClipboardData(UINT uClipFormat) const;

private:
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

    HWND        m_hWnd;
    HWND        m_hWndNextViewer;
    bool        m_bInitialized;
    std::function<void()> m_fnClipboardChange;
};

#endif // DRAGDROP_STUB_H
