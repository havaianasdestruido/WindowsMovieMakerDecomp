/*
 * AudioQueue.cpp
 *
 * Implementation of the AudioQueue thread-safe audio sample queue.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "AudioQueue.h"

namespace HMRAVSource
{

AudioQueue::AudioQueue()
    : m_hEventItemAvailable(nullptr)
    , m_hEventShutdown(nullptr)
    , m_dwMaxItems(256)
    , m_dwHead(0)
    , m_dwTail(0)
    , m_dwCount(0)
    , m_dwTotalEnqueued(0)
    , m_dwTotalDequeued(0)
    , m_dwTotalDropped(0)
    , m_fInitialized(false)
{
    InitializeCriticalSection(&m_cs);
}

AudioQueue::~AudioQueue()
{
    Shutdown();
    DeleteCriticalSection(&m_cs);
}

HRESULT AudioQueue::Initialize(DWORD dwMaxItems)
{
    if (m_fInitialized)
        return S_FALSE;

    m_dwMaxItems = dwMaxItems > 0 ? dwMaxItems : 256;
    m_arrItems.SetCount(m_dwMaxItems);
    m_dwHead = 0;
    m_dwTail = 0;
    m_dwCount = 0;

    m_hEventItemAvailable = CreateEvent(nullptr, TRUE, FALSE, nullptr);
    m_hEventShutdown = CreateEvent(nullptr, TRUE, FALSE, nullptr);

    if (!m_hEventItemAvailable || !m_hEventShutdown)
        return E_OUTOFMEMORY;

    m_fInitialized = true;
    return S_OK;
}

HRESULT AudioQueue::Shutdown()
{
    if (!m_fInitialized)
        return S_FALSE;

    SignalShutdown();

    if (m_hEventItemAvailable) { CloseHandle(m_hEventItemAvailable); m_hEventItemAvailable = nullptr; }
    if (m_hEventShutdown) { CloseHandle(m_hEventShutdown); m_hEventShutdown = nullptr; }

    m_arrItems.RemoveAll();
    m_dwCount = 0;
    m_fInitialized = false;
    return S_OK;
}

bool AudioQueue::IsInitialized() const throw() { return m_fInitialized; }

HRESULT AudioQueue::Enqueue(const BYTE* pData, DWORD cbData, LONGLONG llTimestampHns, DWORD dwFlags)
{
    if (!m_fInitialized || !pData)
        return E_INVALIDARG;

    EnterCriticalSection(&m_cs);

    if (m_dwCount >= m_dwMaxItems)
    {
        // Queue full - drop oldest item
        m_dwHead = (m_dwHead + 1) % m_dwMaxItems;
        m_dwCount--;
        m_dwTotalDropped++;
    }

    AudioQueueItem& item = m_arrItems.GetAt(m_dwTail);
    item.arrData.RemoveAll();
    item.arrData.SetCount(cbData);
    memcpy(item.arrData.GetData(), pData, cbData);
    item.cbData = cbData;
    item.llTimestampHns = llTimestampHns;
    item.dwFlags = dwFlags;

    m_dwTail = (m_dwTail + 1) % m_dwMaxItems;
    m_dwCount++;
    m_dwTotalEnqueued++;

    SetEvent(m_hEventItemAvailable);
    LeaveCriticalSection(&m_cs);
    return S_OK;
}

HRESULT AudioQueue::EnqueueSample(IMFSample* pSample)
{
    if (!pSample) return E_INVALIDARG;

    CComPtr<IMFMediaBuffer> spBuffer;
    HRESULT hr = pSample->ConvertToContiguousBuffer(&spBuffer);
    if (FAILED(hr)) return hr;

    BYTE* pData = nullptr;
    DWORD cbData = 0;
    hr = spBuffer->Lock(&pData, nullptr, &cbData);
    if (FAILED(hr)) return hr;

    LONGLONG llTimestamp = 0;
    pSample->GetSampleTime(&llTimestamp);

    DWORD dwFlags = 0;
    if (SUCCEEDED(pSample->GetUINT32(MFSampleExtension_Discontinuity, &dwFlags)) && dwFlags)
        dwFlags = 1;

    hr = Enqueue(pData, cbData, llTimestamp, dwFlags);
    spBuffer->Unlock();
    return hr;
}

HRESULT AudioQueue::Dequeue(AudioQueueItem* pItem)
{
    if (!m_fInitialized || !pItem)
        return E_INVALIDARG;

    EnterCriticalSection(&m_cs);

    if (m_dwCount == 0)
    {
        LeaveCriticalSection(&m_cs);
        return HRESULT_FROM_WIN32(ERROR_NO_DATA);
    }

    AudioQueueItem& srcItem = m_arrItems.GetAt(m_dwHead);
    pItem->arrData.RemoveAll();
    pItem->arrData.SetCount(srcItem.cbData);
    memcpy(pItem->arrData.GetData(), srcItem.arrData.GetData(), srcItem.cbData);
    pItem->cbData = srcItem.cbData;
    pItem->llTimestampHns = srcItem.llTimestampHns;
    pItem->dwFlags = srcItem.dwFlags;

    srcItem.arrData.RemoveAll();
    srcItem.cbData = 0;

    m_dwHead = (m_dwHead + 1) % m_dwMaxItems;
    m_dwCount--;
    m_dwTotalDequeued++;

    if (m_dwCount == 0)
        ResetEvent(m_hEventItemAvailable);

    LeaveCriticalSection(&m_cs);
    return S_OK;
}

HRESULT AudioQueue::Peek(AudioQueueItem* pItem) const
{
    if (!m_fInitialized || !pItem)
        return E_INVALIDARG;

    EnterCriticalSection(&m_cs);

    if (m_dwCount == 0)
    {
        LeaveCriticalSection(&m_cs);
        return HRESULT_FROM_WIN32(ERROR_NO_DATA);
    }

    const AudioQueueItem& srcItem = m_arrItems.GetAt(m_dwHead);
    pItem->arrData.RemoveAll();
    pItem->arrData.SetCount(srcItem.cbData);
    memcpy(pItem->arrData.GetData(), srcItem.arrData.GetData(), srcItem.cbData);
    pItem->cbData = srcItem.cbData;
    pItem->llTimestampHns = srcItem.llTimestampHns;
    pItem->dwFlags = srcItem.dwFlags;

    LeaveCriticalSection(&m_cs);
    return S_OK;
}

void AudioQueue::Flush()
{
    EnterCriticalSection(&m_cs);

    for (DWORD i = 0; i < m_dwMaxItems; ++i)
        m_arrItems.GetAt(i).arrData.RemoveAll();

    m_dwHead = 0;
    m_dwTail = 0;
    m_dwCount = 0;
    ResetEvent(m_hEventItemAvailable);

    LeaveCriticalSection(&m_cs);
}

size_t AudioQueue::GetCount() const throw()
{
    EnterCriticalSection(&m_cs);
    size_t count = m_dwCount;
    LeaveCriticalSection(&m_cs);
    return count;
}

bool AudioQueue::IsEmpty() const throw() { return GetCount() == 0; }
bool AudioQueue::IsFull() const throw() { return GetCount() >= m_dwMaxItems; }

HRESULT AudioQueue::WaitForItem(DWORD dwTimeoutMs)
{
    if (!m_fInitialized) return E_FAIL;

    HANDLE handles[] = { m_hEventItemAvailable, m_hEventShutdown };
    DWORD dwResult = WaitForMultipleObjects(2, handles, FALSE, dwTimeoutMs);

    if (dwResult == WAIT_OBJECT_0 + 1)
        return E_ABORT; // shutdown signaled

    return (dwResult == WAIT_OBJECT_0) ? S_OK : HRESULT_FROM_WIN32(ERROR_TIMEOUT);
}

void AudioQueue::SignalShutdown()
{
    if (m_hEventShutdown)
        SetEvent(m_hEventShutdown);
}

DWORD AudioQueue::GetMaxItems() const throw() { return m_dwMaxItems; }

HRESULT AudioQueue::SetMaxItems(DWORD dwMaxItems)
{
    if (dwMaxItems == 0) return E_INVALIDARG;
    m_dwMaxItems = dwMaxItems;
    return S_OK;
}

DWORD AudioQueue::GetTotalEnqueued() const throw() { return m_dwTotalEnqueued; }
DWORD AudioQueue::GetTotalDequeued() const throw() { return m_dwTotalDequeued; }
DWORD AudioQueue::GetTotalDropped() const throw() { return m_dwTotalDropped; }

} // namespace HMRAVSource
