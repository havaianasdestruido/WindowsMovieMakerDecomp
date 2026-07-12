/*
 * SerializationMemoryReaderWriter.h
 *
 * In-memory serialization buffer for reading and writing .wlmp project
 * data without touching the filesystem. Used for clipboard operations,
 * undo/redo snapshots, and auto-save serialization.
 *
 * RTTI: ?AVSerializationMemoryReaderWriter@@
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#pragma once

#ifndef SERIALIZATIONMEMORYREADERWRITER_H
#define SERIALIZATIONMEMORYREADERWRITER_H

#include "SerializationContext.h"
#include "SerializationReader.h"
#include "SerializationWriter.h"

namespace StoryboardManager
{

class MovieProject;

// ============================================================================
// SerializationMemoryReaderWriter
// ============================================================================
// Dual-purpose in-memory buffer that can serialize a project to a memory
// block and deserialize it back. Backed by an HGLOBAL lock that is
// exposed as an IStream for XmlLite reader/writer integration.
//
class STORYBOARD_API SerializationMemoryReaderWriter
{
public:
    SerializationMemoryReaderWriter();
    ~SerializationMemoryReaderWriter();

    // -- Write project to memory buffer --
    HRESULT WriteToBuffer(MovieProject* pProject);
    HRESULT WriteToBuffer(MovieProject* pProject, IStream* pStream);

    // -- Read project from memory buffer --
    HRESULT ReadFromBuffer(MovieProject* pProject);
    HRESULT ReadFromStream(IStream* pStream, MovieProject* pProject);

    // -- Raw buffer access --
    const BYTE* GetBuffer() const throw();
    DWORD GetBufferSize() const throw();

    // -- Stream access --
    IStream* GetStream();
    HRESULT  CreateStreamOnBuffer();

    // -- Buffer management --
    HRESULT Reserve(DWORD cbSize);
    HRESULT Clear();
    bool    IsEmpty() const throw();

    // -- Copy --
    HRESULT CopyFrom(const BYTE* pcbData, DWORD cbData);
    HRESULT CopyTo(BYTE* pcbBuffer, DWORD cbBufferSize, DWORD* pcbCopied);

    // -- Snapshot helpers (for undo/redo) --
    static HRESULT CreateSnapshot(MovieProject* pProject,
                                  SerializationMemoryReaderWriter** ppSnapshot);
    HRESULT RestoreSnapshot(MovieProject* pProject);

private:
    // -- Internal state --
    HGLOBAL   m_hGlobal;
    BYTE*     m_pBuffer;
    DWORD     m_cbBufferSize;
    DWORD     m_cbDataSize;
    IStream*  m_pStream;
};

} // namespace StoryboardManager

#endif // SERIALIZATIONMEMORYREADERWRITER_H
