/*
 * SerializationMemoryReaderWriter.cpp
 *
 * Implementation of the in-memory serialization buffer for project
 * snapshots, clipboard, and auto-save operations.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "SerializationMemoryReaderWriter.h"
#include "../MovieProject.h"

namespace StoryboardManager
{

// ============================================================================
// Constants
// ============================================================================
namespace
{
    const DWORD kInitialBufferSize = 4096;
    const DWORD kGrowFactor = 2;
}

// ============================================================================
// Construction / destruction
// ============================================================================

SerializationMemoryReaderWriter::SerializationMemoryReaderWriter()
    : m_hGlobal(nullptr)
    , m_pBuffer(nullptr)
    , m_cbBufferSize(0)
    , m_cbDataSize(0)
    , m_pStream(nullptr)
{
}

SerializationMemoryReaderWriter::~SerializationMemoryReaderWriter()
{
    Clear();
}

// ============================================================================
// WriteToBuffer
// ============================================================================

HRESULT SerializationMemoryReaderWriter::WriteToBuffer(MovieProject* pProject)
{
    if (!pProject)
        return E_POINTER;

    if (!m_hGlobal)
    {
        HRESULT hr = Reserve(kInitialBufferSize);
        if (FAILED(hr))
            return hr;
    }

    IStream* pStream = nullptr;
    HRESULT hr = CreateStreamOnHGlobal(m_hGlobal, FALSE, &pStream);
    if (FAILED(hr))
        return hr;

    hr = WriteToBuffer(pProject, pStream);
    pStream->Release();

    return hr;
}

HRESULT SerializationMemoryReaderWriter::WriteToBuffer(MovieProject* pProject, IStream* pStream)
{
    if (!pProject || !pStream)
        return E_POINTER;

    SerializationWriter writer;
    HRESULT hr = writer.WriteToStream(pStream, pProject);
    if (SUCCEEDED(hr))
    {
        STATSTG statstg;
        hr = pStream->Stat(&statstg, STATFLAG_NONAME);
        if (SUCCEEDED(hr))
        {
            m_cbDataSize = static_cast<DWORD>(statstg.cbSize.QuadPart);
        }
    }

    return hr;
}

// ============================================================================
// ReadFromBuffer
// ============================================================================

HRESULT SerializationMemoryReaderWriter::ReadFromBuffer(MovieProject* pProject)
{
    if (!pProject)
        return E_POINTER;

    if (!m_hGlobal || m_cbDataSize == 0)
        return E_UNEXPECTED;

    IStream* pStream = nullptr;
    HRESULT hr = CreateStreamOnHGlobal(m_hGlobal, FALSE, &pStream);
    if (FAILED(hr))
        return hr;

    hr = ReadFromStream(pStream, pProject);
    pStream->Release();

    return hr;
}

HRESULT SerializationMemoryReaderWriter::ReadFromStream(IStream* pStream, MovieProject* pProject)
{
    if (!pStream || !pProject)
        return E_POINTER;

    SerializationReader reader;
    return reader.ReadFromStream(pStream, pProject);
}

// ============================================================================
// Raw buffer access
// ============================================================================

const BYTE* SerializationMemoryReaderWriter::GetBuffer() const throw()
{
    return m_pBuffer;
}

DWORD SerializationMemoryReaderWriter::GetBufferSize() const throw()
{
    return m_cbBufferSize;
}

// ============================================================================
// Stream access
// ============================================================================

IStream* SerializationMemoryReaderWriter::GetStream()
{
    return m_pStream;
}

HRESULT SerializationMemoryReaderWriter::CreateStreamOnBuffer()
{
    if (!m_hGlobal)
    {
        HRESULT hr = Reserve(kInitialBufferSize);
        if (FAILED(hr))
            return hr;
    }

    if (m_pStream)
    {
        m_pStream->Release();
        m_pStream = nullptr;
    }

    return CreateStreamOnHGlobal(m_hGlobal, FALSE, &m_pStream);
}

// ============================================================================
// Buffer management
// ============================================================================

HRESULT SerializationMemoryReaderWriter::Reserve(DWORD cbSize)
{
    if (cbSize == 0)
        return E_INVALIDARG;

    if (m_hGlobal)
    {
        GlobalFree(m_hGlobal);
        m_hGlobal = nullptr;
        m_pBuffer = nullptr;
    }

    m_hGlobal = GlobalAlloc(GMEM_MOVEABLE, cbSize);
    if (!m_hGlobal)
        return E_OUTOFMEMORY;

    m_pBuffer = static_cast<BYTE*>(GlobalLock(m_hGlobal));
    if (!m_pBuffer)
    {
        GlobalFree(m_hGlobal);
        m_hGlobal = nullptr;
        return E_OUTOFMEMORY;
    }

    m_cbBufferSize = cbSize;
    m_cbDataSize = 0;

    return S_OK;
}

HRESULT SerializationMemoryReaderWriter::Clear()
{
    if (m_pStream)
    {
        m_pStream->Release();
        m_pStream = nullptr;
    }

    if (m_hGlobal)
    {
        if (m_pBuffer)
        {
            GlobalUnlock(m_hGlobal);
            m_pBuffer = nullptr;
        }
        GlobalFree(m_hGlobal);
        m_hGlobal = nullptr;
    }

    m_cbBufferSize = 0;
    m_cbDataSize = 0;

    return S_OK;
}

bool SerializationMemoryReaderWriter::IsEmpty() const throw()
{
    return (m_cbDataSize == 0);
}

// ============================================================================
// Copy operations
// ============================================================================

HRESULT SerializationMemoryReaderWriter::CopyFrom(const BYTE* pcbData, DWORD cbData)
{
    if (!pcbData || cbData == 0)
        return E_INVALIDARG;

    HRESULT hr = Reserve(cbData);
    if (FAILED(hr))
        return hr;

    CopyMemory(m_pBuffer, pcbData, cbData);
    m_cbDataSize = cbData;

    return S_OK;
}

HRESULT SerializationMemoryReaderWriter::CopyTo(BYTE* pcbBuffer, DWORD cbBufferSize,
                                                DWORD* pcbCopied)
{
    if (!pcbBuffer || cbBufferSize == 0)
        return E_INVALIDARG;

    DWORD cbToCopy = (m_cbDataSize < cbBufferSize) ? m_cbDataSize : cbBufferSize;

    if (m_pBuffer)
    {
        CopyMemory(pcbBuffer, m_pBuffer, cbToCopy);
    }

    if (pcbCopied)
    {
        *pcbCopied = cbToCopy;
    }

    return (cbToCopy >= m_cbDataSize) ? S_OK : S_FALSE;
}

// ============================================================================
// Snapshot helpers
// ============================================================================

HRESULT SerializationMemoryReaderWriter::CreateSnapshot(MovieProject* pProject,
    SerializationMemoryReaderWriter** ppSnapshot)
{
    if (!pProject || !ppSnapshot)
        return E_POINTER;

    *ppSnapshot = nullptr;

    SerializationMemoryReaderWriter* pSnapshot = new (std::nothrow) SerializationMemoryReaderWriter();
    if (!pSnapshot)
        return E_OUTOFMEMORY;

    HRESULT hr = pSnapshot->WriteToBuffer(pProject);
    if (FAILED(hr))
    {
        delete pSnapshot;
        return hr;
    }

    *ppSnapshot = pSnapshot;
    return S_OK;
}

HRESULT SerializationMemoryReaderWriter::RestoreSnapshot(MovieProject* pProject)
{
    if (!pProject)
        return E_POINTER;

    if (IsEmpty())
        return E_UNEXPECTED;

    return ReadFromBuffer(pProject);
}

} // namespace StoryboardManager
