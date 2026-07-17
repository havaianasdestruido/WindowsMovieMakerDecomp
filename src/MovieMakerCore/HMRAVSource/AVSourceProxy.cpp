// AVSourceProxy.cpp - Thread-safe proxy wrapper implementation

#include "pch.h"
#include "AVSourceProxy.h"

namespace HMRAVSource
{

// ============================================================================
// Static members
// ============================================================================
DWORD AVSourceProxy::s_dwNextProxyId = 1;

static const UINT WM_PROXY_INVOKE = WM_USER + 100;

// ============================================================================
// Construction / Destruction
// ============================================================================

AVSourceProxy::AVSourceProxy()
    : m_pSource(nullptr)
    , m_dwCreationThreadId(0)
    , m_dwProxyId(0)
    , m_fCrossThread(false)
    , m_fInitialized(false)
    , m_hMessageWnd(nullptr)
{
    m_dwProxyId = InterlockedIncrement(reinterpret_cast<LONG*>(&s_dwNextProxyId));
}

AVSourceProxy::~AVSourceProxy()
{
    Shutdown();
}

// ============================================================================
// Lifecycle
// ============================================================================

HRESULT AVSourceProxy::Initialize(AVSource* pSource)
{
    if (!pSource)
        return E_POINTER;

    if (m_fInitialized)
        return E_UNEXPECTED;

    m_pSource = pSource;
    m_dwCreationThreadId = GetCurrentThreadId();
    m_fCrossThread = false;
    m_fInitialized = true;

    return S_OK;
}

HRESULT AVSourceProxy::Initialize(AVSource* pSource, DWORD dwCreationThreadId)
{
    if (!pSource)
        return E_POINTER;

    if (m_fInitialized)
        return E_UNEXPECTED;

    m_pSource = pSource;
    m_dwCreationThreadId = dwCreationThreadId;
    m_fCrossThread = (dwCreationThreadId != GetCurrentThreadId());
    m_fInitialized = true;

    return S_OK;
}

HRESULT AVSourceProxy::Shutdown()
{
    if (!m_fInitialized)
        return S_OK;

    if (m_hMessageWnd)
    {
        DestroyWindow(m_hMessageWnd);
        m_hMessageWnd = nullptr;
    }

    m_pSource = nullptr;
    m_fInitialized = false;
    m_fCrossThread = false;
    m_dwCreationThreadId = 0;

    return S_OK;
}

bool AVSourceProxy::IsInitialized() const throw()
{
    return m_fInitialized;
}

// ============================================================================
// Delegated AVSource operations
// ============================================================================

HRESULT AVSourceProxy::Open(const AVSourceDesc& desc)
{
    if (!m_fInitialized || !m_pSource)
        return E_UNEXPECTED;

    if (!EnsureOnCorrectThread())
        return E_UNEXPECTED;

    return m_pSource->Open(desc);
}

HRESULT AVSourceProxy::Close()
{
    if (!m_fInitialized || !m_pSource)
        return E_UNEXPECTED;

    if (!EnsureOnCorrectThread())
        return E_UNEXPECTED;

    return m_pSource->Close();
}

bool AVSourceProxy::IsOpen() const throw()
{
    if (!m_fInitialized || !m_pSource)
        return false;

    return m_pSource->IsOpen();
}

HRESULT AVSourceProxy::Start()
{
    if (!m_fInitialized || !m_pSource)
        return E_UNEXPECTED;

    if (!EnsureOnCorrectThread())
        return E_UNEXPECTED;

    return m_pSource->Start();
}

HRESULT AVSourceProxy::Stop()
{
    if (!m_fInitialized || !m_pSource)
        return E_UNEXPECTED;

    if (!EnsureOnCorrectThread())
        return E_UNEXPECTED;

    return m_pSource->Stop();
}

HRESULT AVSourceProxy::Pause()
{
    if (!m_fInitialized || !m_pSource)
        return E_UNEXPECTED;

    if (!EnsureOnCorrectThread())
        return E_UNEXPECTED;

    return m_pSource->Pause();
}

HRESULT AVSourceProxy::Resume()
{
    if (!m_fInitialized || !m_pSource)
        return E_UNEXPECTED;

    if (!EnsureOnCorrectThread())
        return E_UNEXPECTED;

    return m_pSource->Resume();
}

LONGLONG AVSourceProxy::GetPositionHns() const throw()
{
    if (!m_fInitialized || !m_pSource)
        return 0;

    return m_pSource->GetPositionHns();
}

HRESULT AVSourceProxy::SetPositionHns(LONGLONG llPosition)
{
    if (!m_fInitialized || !m_pSource)
        return E_UNEXPECTED;

    if (!EnsureOnCorrectThread())
        return E_UNEXPECTED;

    return m_pSource->SetPositionHns(llPosition);
}

LONGLONG AVSourceProxy::GetDurationHns() const throw()
{
    if (!m_fInitialized || !m_pSource)
        return 0;

    return m_pSource->GetDurationHns();
}

HRESULT AVSourceProxy::ReadSample(IMFSample** ppSample, DWORD dwStreamIndex)
{
    if (!m_fInitialized || !m_pSource)
        return E_UNEXPECTED;

    if (!EnsureOnCorrectThread())
        return E_UNEXPECTED;

    return m_pSource->ReadSample(ppSample, dwStreamIndex);
}

HRESULT AVSourceProxy::Flush()
{
    if (!m_fInitialized || !m_pSource)
        return E_UNEXPECTED;

    if (!EnsureOnCorrectThread())
        return E_UNEXPECTED;

    return m_pSource->Flush();
}

// ============================================================================
// Source info
// ============================================================================

AVSourceState AVSourceProxy::GetState() const throw()
{
    if (!m_fInitialized || !m_pSource)
        return AVSourceStateIdle;

    return m_pSource->GetState();
}

const AVSourceInfo& AVSourceProxy::GetInfo() const
{
    static AVSourceInfo s_emptyInfo;
    if (!m_fInitialized || !m_pSource)
        return s_emptyInfo;

    return m_pSource->GetInfo();
}

AVSourceType AVSourceProxy::GetType() const throw()
{
    if (!m_fInitialized || !m_pSource)
        return AVSourceTypeUnknown;

    return m_pSource->GetType();
}

// ============================================================================
// Thread affinity
// ============================================================================

DWORD AVSourceProxy::GetCreationThreadId() const throw()
{
    return m_dwCreationThreadId;
}

bool AVSourceProxy::IsOnCreationThread() const throw()
{
    return GetCurrentThreadId() == m_dwCreationThreadId;
}

// ============================================================================
// Cross-thread marshaling control
// ============================================================================

HRESULT AVSourceProxy::SetMarshalingMode(bool fCrossThread)
{
    m_fCrossThread = fCrossThread;
    return S_OK;
}

bool AVSourceProxy::IsCrossThreadMarshaling() const throw()
{
    return m_fCrossThread;
}

// ============================================================================
// Error info
// ============================================================================

HRESULT AVSourceProxy::GetLastResult() const throw()
{
    if (!m_fInitialized || !m_pSource)
        return E_UNEXPECTED;

    return m_pSource->GetLastResult();
}

// ============================================================================
// Private helpers
// ============================================================================

bool AVSourceProxy::EnsureOnCorrectThread() const
{
    if (!m_fCrossThread)
        return true;

    return GetCurrentThreadId() == m_dwCreationThreadId;
}

HRESULT AVSourceProxy::PostToCreationThread(std::function<void()> fn)
{
    if (!fn)
        return E_POINTER;

    if (!m_hMessageWnd)
        return E_UNEXPECTED;

    auto* pfn = new (std::nothrow) std::function<void()>(std::move(fn));
    if (!pfn)
        return E_OUTOFMEMORY;

    BOOL fPosted = PostMessage(m_hMessageWnd, WM_PROXY_INVOKE, 0,
        reinterpret_cast<LPARAM>(pfn));

    if (!fPosted)
    {
        delete pfn;
        return HRESULT_FROM_WIN32(GetLastError());
    }

    return S_OK;
}

LRESULT AVSourceProxy::SendMessageToCreationThread(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (!m_hMessageWnd)
        return E_UNEXPECTED;

    return ::SendMessage(m_hMessageWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK AVSourceProxy::ProxyWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_DESTROY)
    {
        SetWindowLongPtr(hWnd, GWLP_USERDATA, 0);
        return 0;
    }

    if (uMsg == WM_PROXY_INVOKE)
    {
        auto pfn = reinterpret_cast<std::function<void()>*>(lParam);
        if (pfn)
        {
            (*pfn)();
            delete pfn;
        }
        return 1;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

} // namespace HMRAVSource
