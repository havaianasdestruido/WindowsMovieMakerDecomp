/*
 * AudioQueue.h
 *
 * Thread-safe audio sample queue for buffering audio samples between
 * the source reader and audio output renderer. Supports multiple
 * producer/consumer threading patterns.
 *
 * RTTI classes:
 *   ?AVAudioQueue@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef AUDIOQUEUE_H
#define AUDIOQUEUE_H

#include "../HMRAVSource.h"

namespace HMRAVSource
{

// ============================================================================
// AudioQueueItem
// ============================================================================
struct AudioQueueItem
{
    ATL::CAtlArray<BYTE>    arrData;
    DWORD                   cbData;
    LONGLONG                llTimestampHns;
    DWORD                   dwFlags;

    AudioQueueItem()
        : cbData(0)
        , llTimestampHns(0)
        , dwFlags(0)
    {
    }
};

// ============================================================================
// AudioQueue
// ============================================================================
// Thread-safe FIFO queue for audio samples. Uses critical sections for
// thread safety and events for blocking wait when the queue is empty.
//
class AVSOURCE_API AudioQueue
{
public:
    AudioQueue();
    ~AudioQueue();

    // Lifecycle
    HRESULT Initialize(DWORD dwMaxItems = 256);
    HRESULT Shutdown();
    bool IsInitialized() const throw();

    // Queue operations
    HRESULT Enqueue(const BYTE* pData, DWORD cbData, LONGLONG llTimestampHns, DWORD dwFlags = 0);
    HRESULT EnqueueSample(IMFSample* pSample);
    HRESULT Dequeue(AudioQueueItem* pItem);
    HRESULT Peek(AudioQueueItem* pItem) const;
    void Flush();

    // State
    size_t GetCount() const throw();
    bool IsEmpty() const throw();
    bool IsFull() const throw();

    // Blocking wait
    HRESULT WaitForItem(DWORD dwTimeoutMs = INFINITE);
    void SignalShutdown();

    // Capacity
    DWORD GetMaxItems() const throw();
    HRESULT SetMaxItems(DWORD dwMaxItems);

    // Statistics
    DWORD GetTotalEnqueued() const throw();
    DWORD GetTotalDequeued() const throw();
    DWORD GetTotalDropped() const throw();

private:
    mutable CRITICAL_SECTION    m_cs;
    HANDLE                      m_hEventItemAvailable;
    HANDLE                      m_hEventShutdown;

    ATL::CAtlArray<AudioQueueItem> m_arrItems;
    DWORD                       m_dwMaxItems;
    DWORD                       m_dwHead;
    DWORD                       m_dwTail;
    DWORD                       m_dwCount;

    DWORD                       m_dwTotalEnqueued;
    DWORD                       m_dwTotalDequeued;
    DWORD                       m_dwTotalDropped;
    bool                        m_fInitialized;
};

} // namespace HMRAVSource

#endif // AUDIOQUEUE_H
