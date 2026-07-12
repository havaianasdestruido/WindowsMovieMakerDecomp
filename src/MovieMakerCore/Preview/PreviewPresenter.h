/*
 * PreviewPresenter.h
 *
 * COM wrapper for the preview presenter pipeline. Connects the
 * StoryboardManager's timeline model to the HMREngine rendering surface
 * for real-time preview playback.
 *
 * RTTI: ?AVPreviewPresenterWrapper@@, ?AV?$CComObject@VPreviewPresenterWrapper@@@ATL@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef PREVIEWPRESENTER_H
#define PREVIEWPRESENTER_H

#include "../pch.h"
#include "../MovieMakerCore.h"

namespace StoryboardManager
{
    class MovieProject;
    class MovieExtent;
    class ExtentCollection;
}

// ============================================================================
// Forward declarations
// ============================================================================
class PreviewDX;
class DefaultPreviewDX;

// ============================================================================
// PreviewPresenterWrapper
// ============================================================================
// COM object (CComObjectRootEx + CComCoClass) that wraps the preview
// rendering pipeline. Exposes IUnknown plus a set of preview-control
// methods consumed by the DirectUI behavior layer (SundanceMainElementBehavior).
//
// Lifecycle: Created once when the main window initializes, released when
// the main window is destroyed. Holds back-pointers to the StoryboardManager
// project and the HMREngine rendering surface (PreviewDX).
//
class ATL_NO_VTABLE PreviewPresenterWrapper :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<PreviewPresenterWrapper, &CLSID_Null>,
    public IUnknown
{
public:
    PreviewPresenterWrapper();
    virtual ~PreviewPresenterWrapper();

    DECLARE_NO_REGISTRY()
    DECLARE_NOT_AGGREGATABLE(PreviewPresenterWrapper)

    BEGIN_COM_MAP(PreviewPresenterWrapper)
        COM_INTERFACE_ENTRY(IUnknown)
    END_COM_MAP()

    // -- Preview lifecycle --
    HRESULT StartPreview();
    HRESULT StopPreview();
    HRESULT PausePreview();
    bool    IsPreviewActive() const throw();

    // -- Seeking --
    HRESULT SeekTo(LONGLONG llPositionHns);
    HRESULT SeekToNormalized(double dblPosition);
    LONGLONG GetCurrentPositionHns() const throw();
    double   GetNormalizedPosition() const throw();

    // -- Window management --
    HRESULT SetPreviewWindow(HWND hWnd);
    HWND    GetPreviewWindow() const throw();
    HRESULT ResizePreview(UINT cx, UINT cy);

    // -- Project binding --
    HRESULT SetProject(StoryboardManager::MovieProject* pProject);
    StoryboardManager::MovieProject* GetProject() const throw();

    // -- Rendering surface --
    HRESULT SetPreviewDX(PreviewDX* pPreviewDX);
    PreviewDX* GetPreviewDX() const throw();

    // -- Frame snapshot --
    HRESULT CaptureFrame(Gdiplus::Bitmap** ppBitmap);
    HRESULT CaptureFrameToHBitmap(HBITMAP* phBitmap);

    // -- Volume --
    HRESULT SetVolume(double dblVolume);
    double  GetVolume() const throw();

    // -- Speed --
    HRESULT SetPlaybackSpeed(double dblSpeed);
    double  GetPlaybackSpeed() const throw();

    // -- Playback state --
    enum PreviewState
    {
        PreviewStateStopped  = 0,
        PreviewStatePlaying  = 1,
        PreviewStatePaused   = 2,
        PreviewStateSeeking  = 3
    };

    PreviewState GetState() const throw();

private:
    // -- Internal rendering loop --
    static DWORD WINAPI RenderThreadProc(LPVOID pParam);
    void DoRenderLoop();
    void RenderCurrentFrame();

    // -- Extent navigation --
    int  FindExtentAtPosition(LONGLONG llPositionHns) const;
    LONGLONG GetExtentStartTime(int nIndex) const;
    LONGLONG GetExtentEndTime(int nIndex) const;

    // -- State --
    PreviewState    m_state;
    HWND            m_hWndPreview;
    bool            m_fPreviewActive;

    // -- Position --
    LONGLONG        m_llCurrentPositionHns;
    LONGLONG        m_llTotalDurationHns;
    double          m_dblVolume;
    double          m_dblPlaybackSpeed;

    // -- Threading --
    HANDLE          m_hRenderThread;
    HANDLE          m_hStopEvent;
    HANDLE          m_hSeekEvent;
    CRITICAL_SECTION m_csLock;

    // -- Back-pointers (not owned) --
    StoryboardManager::MovieProject* m_pProject;
    PreviewDX*                       m_pPreviewDX;

    // Non-copyable
    PreviewPresenterWrapper(const PreviewPresenterWrapper&);
    PreviewPresenterWrapper& operator=(const PreviewPresenterWrapper&);
};

// ============================================================================
// PreviewDX
// ============================================================================
// Abstract rendering surface for preview output. Manages the Direct3D 11
// device, swap chain, and EVR (Enhanced Video Renderer) integration.
// DefaultPreviewDX provides the default implementation backed by D3D11.
//
// RTTI: ?AVPreviewDX@@, ?AVDefaultPreviewDX@@
//
class PreviewDX
{
public:
    PreviewDX();
    virtual ~PreviewDX();

    virtual HRESULT Initialize(HWND hWnd, UINT cx, UINT cy) = 0;
    virtual HRESULT Uninitialize() = 0;
    virtual HRESULT Resize(UINT cx, UINT cy) = 0;

    virtual HRESULT BeginFrame() = 0;
    virtual HRESULT EndFrame() = 0;
    virtual HRESULT Present() = 0;

    virtual HRESULT DrawBitmap(Gdiplus::Bitmap* pBitmap) = 0;
    virtual HRESULT Clear() = 0;

    virtual ID3D11Device*        GetD3DDevice() const throw() = 0;
    virtual ID3D11DeviceContext*  GetD3DDeviceContext() const throw() = 0;

    bool IsInitialized() const throw();

protected:
    bool m_fInitialized;
    HWND m_hWnd;
    UINT m_cx;
    UINT m_cy;
};

// ============================================================================
// DefaultPreviewDX
// ============================================================================
// Default PreviewDX implementation using Direct3D 11. Creates a HWND-rendered
// swap chain with BGRA8 back buffer for GDI+ interop.
//
class DefaultPreviewDX : public PreviewDX
{
public:
    DefaultPreviewDX();
    virtual ~DefaultPreviewDX();

    virtual HRESULT Initialize(HWND hWnd, UINT cx, UINT cy) override;
    virtual HRESULT Uninitialize() override;
    virtual HRESULT Resize(UINT cx, UINT cy) override;

    virtual HRESULT BeginFrame() override;
    virtual HRESULT EndFrame() override;
    virtual HRESULT Present() override;

    virtual HRESULT DrawBitmap(Gdiplus::Bitmap* pBitmap) override;
    virtual HRESULT Clear() override;

    virtual ID3D11Device*        GetD3DDevice() const throw() override;
    virtual ID3D11DeviceContext*  GetD3DDeviceContext() const throw() override;

private:
    HRESULT CreateDeviceAndSwapChain();
    HRESULT CreateRenderTarget();
    void    ReleaseDeviceAndSwapChain();

    ID3D11Device*           m_pDevice;
    ID3D11DeviceContext*    m_pContext;
    IDXGISwapChain*         m_pSwapChain;
    ID3D11RenderTargetView* m_pRenderTarget;

    DXGI_SWAP_CHAIN_DESC    m_swapChainDesc;

    DefaultPreviewDX(const DefaultPreviewDX&);
    DefaultPreviewDX& operator=(const DefaultPreviewDX&);
};

// ============================================================================
// Factory
// ============================================================================
STORYBOARD_API HRESULT CreatePreviewPresenter(PreviewPresenterWrapper** ppPresenter);
STORYBOARD_API HRESULT CreateDefaultPreviewDX(DefaultPreviewDX** ppPreviewDX);

#endif // PREVIEWPRESENTER_H
