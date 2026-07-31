// MFByteStreamOnStream.cpp - IStream to IMFByteStream adapter implementation

#include "pch.h"
#include "MFByteStreamOnStream.h"

namespace HMRAVSource
{

// ============================================================================
// MFByteStreamOnStream
// ============================================================================

MFByteStreamOnStream::MFByteStreamOnStream()
    : m_cRef(1)
    , m_qwPosition(0)
{
    InitializeCriticalSection(&m_csLock);
}

MFByteStreamOnStream::~MFByteStreamOnStream()
{
    DeleteCriticalSection(&m_csLock);
}

HRESULT MFByteStreamOnStream::CreateInstance(IStream* pStream, IMFByteStream** ppByteStream)
{
    if (!pStream || !ppByteStream)
        return E_POINTER;

    *ppByteStream = nullptr;

    MFByteStreamOnStream* pInstance = new (std::nothrow) MFByteStreamOnStream();
    if (!pInstance)
        return E_OUTOFMEMORY;

    pInstance->m_spStream = pStream;

    *ppByteStream = pInstance;
    (*ppByteStream)->AddRef();
    return S_OK;
}

STDMETHODIMP MFByteStreamOnStream::QueryInterface(REFIID riid, void** ppvObject)
{
    if (!ppvObject)
        return E_POINTER;

    *ppvObject = nullptr;

    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IMFByteStream))
    {
        *ppvObject = static_cast<IMFByteStream*>(this);
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) MFByteStreamOnStream::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) MFByteStreamOnStream::Release()
{
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if (cRef == 0)
        delete this;
    return cRef;
}

STDMETHODIMP MFByteStreamOnStream::GetCapabilities(DWORD* pdwCapabilities)
{
    if (!pdwCapabilities)
        return E_POINTER;

    *pdwCapabilities = MFBYTESTREAM_IS_LOADING | MFBYTESTREAM_IS_SEEKING;
    return S_OK;
}

STDMETHODIMP MFByteStreamOnStream::GetLength(QWORD* pqwLength)
{
    if (!pqwLength)
        return E_POINTER;

    if (!m_spStream)
        return E_UNEXPECTED;

    STATSTG stat = {};
    HRESULT hr = m_spStream->Stat(&stat, STATFLAG_NONAME);
    if (FAILED(hr))
        return hr;

    *pqwLength = stat.cbSize.QuadPart;
    return S_OK;
}

STDMETHODIMP MFByteStreamOnStream::SetLength(QWORD qwLength)
{
    if (!m_spStream)
        return E_UNEXPECTED;

    EnterCriticalSection(&m_csLock);

    LARGE_INTEGER liZero = {};
    HRESULT hr = m_spStream->Seek(liZero, STREAM_SEEK_SET, nullptr);
    if (SUCCEEDED(hr))
    {
        ULARGE_INTEGER uliSize = {};
        uliSize.QuadPart = static_cast<ULONGLONG>(qwLength);
        hr = m_spStream->SetSize(uliSize);
    }

    LeaveCriticalSection(&m_csLock);
    return hr;
}

STDMETHODIMP MFByteStreamOnStream::GetCurrentPosition(QWORD* pqwPosition)
{
    if (!pqwPosition)
        return E_POINTER;

    *pqwPosition = m_qwPosition;
    return S_OK;
}

STDMETHODIMP MFByteStreamOnStream::SetCurrentPosition(QWORD qwPosition)
{
    m_qwPosition = qwPosition;

    if (m_spStream)
    {
        LARGE_INTEGER li;
        li.QuadPart = static_cast<LONGLONG>(qwPosition);
        return m_spStream->Seek(li, STREAM_SEEK_SET, nullptr);
    }

    return S_OK;
}

STDMETHODIMP MFByteStreamOnStream::IsEndOfStream(BOOL* pfEndOfStream)
{
    if (!pfEndOfStream)
        return E_POINTER;

    *pfEndOfStream = FALSE;

    QWORD qwLength = 0;
    HRESULT hr = GetLength(&qwLength);
    if (SUCCEEDED(hr))
        *pfEndOfStream = (m_qwPosition >= qwLength) ? TRUE : FALSE;

    return S_OK;
}

STDMETHODIMP MFByteStreamOnStream::Read(BYTE* pb, ULONG cb, ULONG* pcbRead)
{
    if (!pb || !pcbRead)
        return E_POINTER;

    *pcbRead = 0;

    if (!m_spStream)
        return E_UNEXPECTED;

    EnterCriticalSection(&m_csLock);

    LARGE_INTEGER li;
    li.QuadPart = static_cast<LONGLONG>(m_qwPosition);
    HRESULT hr = m_spStream->Seek(li, STREAM_SEEK_SET, nullptr);
    if (FAILED(hr))
    {
        LeaveCriticalSection(&m_csLock);
        return hr;
    }

    hr = m_spStream->Read(pb, cb, pcbRead);
    if (SUCCEEDED(hr))
        m_qwPosition += *pcbRead;

    LeaveCriticalSection(&m_csLock);
    return hr;
}

STDMETHODIMP MFByteStreamOnStream::BeginRead(BYTE* pb, ULONG cb, IMFAsyncCallback* pCallback, IUnknown* punkState)
{
    if (!pCallback)
        return E_POINTER;

    if (!pb || cb == 0)
        return E_INVALIDARG;

    // Synchronous fallback - read immediately and post completion
    ULONG cbRead = 0;
    HRESULT hr = Read(pb, cb, &cbRead);

    CComPtr<MFAsyncResult> spResult;
    HRESULT hrCreate = MFAsyncResult::CreateInstance(nullptr, punkState, pCallback, &spResult);
    if (FAILED(hrCreate))
        return hrCreate;

    spResult->SetStatus(hr);
    spResult->SetBytesTransferred(cbRead);
    spResult->InvokeCallback();

    return S_OK;
}

STDMETHODIMP MFByteStreamOnStream::EndRead(IMFAsyncResult* pResult, ULONG* pcbRead)
{
    if (!pResult || !pcbRead)
        return E_POINTER;

    MFAsyncResult* pAsyncResult = static_cast<MFAsyncResult*>(pResult);
    *pcbRead = pAsyncResult->GetBytesTransferred();
    return pAsyncResult->GetStatus();
}

STDMETHODIMP MFByteStreamOnStream::Write(const BYTE* pb, ULONG cb, ULONG* pcbWritten)
{
    if (pcbWritten)
        *pcbWritten = 0;

    if (!pb || cb == 0)
        return E_INVALIDARG;

    if (!m_spStream)
        return E_UNEXPECTED;

    EnterCriticalSection(&m_csLock);

    ULONG cbWritten = 0;
    HRESULT hr = m_spStream->Write(const_cast<BYTE*>(pb), cb, &cbWritten);

    if (SUCCEEDED(hr))
    {
        m_qwPosition += cbWritten;
        if (pcbWritten)
            *pcbWritten = cbWritten;
    }

    LeaveCriticalSection(&m_csLock);
    return hr;
}

STDMETHODIMP MFByteStreamOnStream::BeginWrite(const BYTE* pb, ULONG cb, IMFAsyncCallback* pCallback, IUnknown* punkState)
{
    if (!pb || cb == 0)
        return E_INVALIDARG;

    if (!pCallback)
        return E_POINTER;

    if (!m_spStream)
        return E_UNEXPECTED;

    ULONG cbWritten = 0;
    HRESULT hrWrite = m_spStream->Write(const_cast<BYTE*>(pb), cb, &cbWritten);

    if (SUCCEEDED(hrWrite))
    {
        EnterCriticalSection(&m_csLock);
        m_qwPosition += cbWritten;
        LeaveCriticalSection(&m_csLock);
    }

    CComPtr<MFAsyncResult> spResult;
    HRESULT hrCreate = MFAsyncResult::CreateInstance(
        static_cast<IUnknown*>(this), punkState, pCallback, &spResult);
    if (FAILED(hrCreate))
        return hrCreate;

    spResult->SetAsyncResult(hrWrite);
    spResult->SetBytesTransferred(cbWritten);
    return spResult->InvokeCallback();
}

STDMETHODIMP MFByteStreamOnStream::EndWrite(IMFAsyncResult* pResult, ULONG* pcbWritten)
{
    if (!pResult)
        return E_POINTER;

    if (pcbWritten)
    {
        MFAsyncResult* pAsyncResult = static_cast<MFAsyncResult*>(pResult);
        *pcbWritten = pAsyncResult->GetBytesTransferred();
    }

    return pResult->GetStatus();
}

STDMETHODIMP MFByteStreamOnStream::Seek(
    MFBYTESTREAM_SEEK_ORIGIN SeekOrigin,
    LONGLONG llSeekOffset,
    DWORD /*dwSeekFlags*/,
    QWORD* pqwCurrentPosition)
{
    if (!pqwCurrentPosition)
        return E_POINTER;

    EnterCriticalSection(&m_csLock);

    LONGLONG llNewPosition = 0;

    switch (SeekOrigin)
    {
    case msoBegin:
        llNewPosition = llSeekOffset;
        break;
    case msoCurrent:
        llNewPosition = static_cast<LONGLONG>(m_qwPosition) + llSeekOffset;
        break;
    default:
        LeaveCriticalSection(&m_csLock);
        return E_INVALIDARG;
    }

    if (llNewPosition < 0)
        llNewPosition = 0;

    m_qwPosition = static_cast<QWORD>(llNewPosition);

    if (m_spStream)
    {
        LARGE_INTEGER li;
        li.QuadPart = llNewPosition;
        m_spStream->Seek(li, STREAM_SEEK_SET, nullptr);
    }

    *pqwCurrentPosition = m_qwPosition;

    LeaveCriticalSection(&m_csLock);
    return S_OK;
}

STDMETHODIMP MFByteStreamOnStream::Flush()
{
    return S_OK;
}

STDMETHODIMP MFByteStreamOnStream::Close()
{
    EnterCriticalSection(&m_csLock);
    if (m_spStream)
    {
        m_spStream.Release();
    }
    m_qwPosition = 0;
    LeaveCriticalSection(&m_csLock);
    return S_OK;
}

HRESULT MFByteStreamOnStream::GetStream(IStream** ppStream)
{
    if (!ppStream)
        return E_POINTER;

    *ppStream = m_spStream;
    if (*ppStream)
        (*ppStream)->AddRef();
    return S_OK;
}

HRESULT MFByteStreamOnStream::EnsureStream()
{
    return m_spStream ? S_OK : E_UNEXPECTED;
}

// ============================================================================
// MFAsyncResult
// ============================================================================

MFAsyncResult::MFAsyncResult()
    : m_cRef(1)
    , m_hrStatus(E_PENDING)
    , m_cbTransferred(0)
    , m_pCallback(nullptr)
    , m_hEvent(nullptr)
    , m_fCompleted(false)
{
    m_hEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
}

MFAsyncResult::~MFAsyncResult()
{
    if (m_hEvent)
        CloseHandle(m_hEvent);
}

HRESULT MFAsyncResult::CreateInstance(
    IUnknown* pUnkObject,
    IUnknown* pUnkState,
    IMFAsyncCallback* pCallback,
    MFAsyncResult** ppResult)
{
    if (!ppResult)
        return E_POINTER;

    *ppResult = nullptr;

    MFAsyncResult* pResult = new (std::nothrow) MFAsyncResult();
    if (!pResult)
        return E_OUTOFMEMORY;

    pResult->m_spObject = pUnkObject;
    pResult->m_spState = pUnkState;
    pResult->m_pCallback = pCallback;

    *ppResult = pResult;
    return S_OK;
}

STDMETHODIMP MFAsyncResult::QueryInterface(REFIID riid, void** ppvObject)
{
    if (!ppvObject)
        return E_POINTER;

    *ppvObject = nullptr;

    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IMFAsyncResult))
    {
        *ppvObject = static_cast<IMFAsyncResult*>(this);
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) MFAsyncResult::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) MFAsyncResult::Release()
{
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if (cRef == 0)
        delete this;
    return cRef;
}

STDMETHODIMP MFAsyncResult::GetState(IUnknown** ppunkState)
{
    if (!ppunkState)
        return E_POINTER;

    *ppunkState = m_spState;
    if (*ppunkState)
        (*ppunkState)->AddRef();
    return S_OK;
}

STDMETHODIMP MFAsyncResult::GetStatus()
{
    return m_hrStatus;
}

STDMETHODIMP MFAsyncResult::SetStatus(HRESULT hrStatus)
{
    m_hrStatus = hrStatus;
    return S_OK;
}

STDMETHODIMP MFAsyncResult::GetObject(IUnknown** ppunkObject)
{
    if (!ppunkObject)
        return E_POINTER;

    *ppunkObject = m_spObject;
    if (*ppunkObject)
        (*ppunkObject)->AddRef();
    return S_OK;
}

IUnknown* MFAsyncResult::GetStateNoAddRef()
{
    return m_spState;
}

HRESULT MFAsyncResult::SetAsyncResult(HRESULT hr)
{
    m_hrStatus = hr;
    m_fCompleted = true;

    if (m_hEvent)
        SetEvent(m_hEvent);

    return S_OK;
}

HRESULT MFAsyncResult::SetBytesTransferred(ULONG cbTransferred)
{
    m_cbTransferred = cbTransferred;
    return S_OK;
}

ULONG MFAsyncResult::GetBytesTransferred() const throw()
{
    return m_cbTransferred;
}

HRESULT MFAsyncResult::Wait(DWORD dwTimeoutMs)
{
    if (!m_hEvent)
        return E_UNEXPECTED;

    if (m_fCompleted)
        return S_OK;

    DWORD dwResult = WaitForSingleObject(m_hEvent, dwTimeoutMs);
    if (dwResult == WAIT_TIMEOUT)
        return E_PENDING;

    return m_hrStatus;
}

HRESULT MFAsyncResult::GetAsyncResult(HRESULT* phr)
{
    if (phr)
        *phr = m_hrStatus;
    return m_hrStatus;
}

void MFAsyncResult::Reset()
{
    m_hrStatus = E_PENDING;
    m_cbTransferred = 0;
    m_fCompleted = false;

    if (m_hEvent)
        ResetEvent(m_hEvent);
}

HRESULT MFAsyncResult::InvokeCallback()
{
    if (m_pCallback)
    {
        m_fCompleted = true;
        if (m_hEvent)
            SetEvent(m_hEvent);
        return m_pCallback->Invoke(static_cast<IMFAsyncResult*>(this));
    }

    m_fCompleted = true;
    if (m_hEvent)
        SetEvent(m_hEvent);
    return S_OK;
}

} // namespace HMRAVSource
