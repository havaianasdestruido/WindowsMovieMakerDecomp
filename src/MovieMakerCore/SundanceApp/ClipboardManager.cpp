/*
 * ClipboardManager.cpp
 *
 * Implementation of the clipboard operations manager for Sundance.
 * Handles cut/copy/paste of timeline items via internal and system
 * clipboard mechanisms.
 *
 * Built with MSVC 11.0 (VS2012), targets Windows 6.2+ (Win8+).
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 * Source recreation for research and interoperability purposes.
 */

#include "pch.h"
#include "ClipboardManager.h"
#include "SundanceAppMain.h"

// ============================================================================
// Construction / destruction
// ============================================================================
ClipboardManager::ClipboardManager()
    : m_pAppMain(NULL)
    , m_bHasData(false)
    , m_bCutMode(false)
    , m_uClipFormat(0)
{
}

ClipboardManager::~ClipboardManager()
{
    Shutdown();
}

// ============================================================================
// Initialize
// ============================================================================
HRESULT ClipboardManager::Initialize(SundanceAppMain* pAppMain)
{
    if (!pAppMain)
        return E_INVALIDARG;

    m_pAppMain = pAppMain;

    HRESULT hr = RegisterFormat();
    if (FAILED(hr))
        return hr;

    return S_OK;
}

// ============================================================================
// Shutdown
// ============================================================================
void ClipboardManager::Shutdown()
{
    ReleaseClipboardData();
    m_pAppMain = NULL;
}

// ============================================================================
// RegisterFormat
// ============================================================================
HRESULT ClipboardManager::RegisterFormat()
{
    m_uClipFormat = ::RegisterClipboardFormatW(kSundanceClipboardFormat);
    return m_uClipFormat ? S_OK : E_FAIL;
}

// ============================================================================
// Cut
// ============================================================================
HRESULT ClipboardManager::Cut()
{
    if (!m_pAppMain)
        return E_UNEXPECTED;

    HRESULT hr = Copy();
    if (FAILED(hr))
        return hr;

    m_bCutMode = true;

    // Mark selected items for deletion on paste
    // The actual deletion happens in the timeline controller
    // after a successful paste operation
    return S_OK;
}

// ============================================================================
// Copy
// ============================================================================
HRESULT ClipboardManager::Copy()
{
    if (!m_pAppMain)
        return E_UNEXPECTED;

    ReleaseClipboardData();
    m_bCutMode = false;

    HRESULT hr = SerializeSelection();
    if (FAILED(hr))
        return hr;

    m_bHasData = !m_clipboardData.empty();
    return m_bHasData ? S_OK : S_FALSE;
}

// ============================================================================
// Paste
// ============================================================================
HRESULT ClipboardManager::Paste()
{
    if (!m_pAppMain || !m_bHasData)
        return E_UNEXPECTED;

    HRESULT hr = DeserializeAndPaste();
    if (SUCCEEDED(hr))
    {
        if (m_bCutMode)
        {
            // After a successful paste in cut mode, clear the source items
            ReleaseClipboardData();
            m_bCutMode = false;
        }
    }

    return hr;
}

// ============================================================================
// Delete
// ============================================================================
HRESULT ClipboardManager::Delete()
{
    if (!m_pAppMain)
        return E_UNEXPECTED;

    // Delete is equivalent to cut without putting data on clipboard
    // TODO: implement selection deletion via timeline controller
    return S_OK;
}

// ============================================================================
// CanCut / CanCopy / CanPaste / CanDelete
// ============================================================================
bool ClipboardManager::CanCut() const throw()
{
    return m_pAppMain && m_pAppMain->IsProjectOpen();
}

bool ClipboardManager::CanCopy() const throw()
{
    return m_pAppMain && m_pAppMain->IsProjectOpen();
}

bool ClipboardManager::CanPaste() const throw()
{
    return m_pAppMain && m_pAppMain->IsProjectOpen() && m_bHasData;
}

bool ClipboardManager::CanDelete() const throw()
{
    return m_pAppMain && m_pAppMain->IsProjectOpen();
}

// ============================================================================
// HasInternalClipboardData / GetClipboardItemCount / ClearClipboard
// ============================================================================
bool ClipboardManager::HasInternalClipboardData() const throw()
{
    return m_bHasData;
}

DWORD ClipboardManager::GetClipboardItemCount() const throw()
{
    return static_cast<DWORD>(m_clipboardData.size());
}

void ClipboardManager::ClearClipboard()
{
    ReleaseClipboardData();
}

// ============================================================================
// OnClipboardChanged
// ============================================================================
void ClipboardManager::OnClipboardChanged()
{
    // Check if our custom format is on the system clipboard
    if (::IsClipboardFormatAvailable(m_uClipFormat))
    {
        // Data available from external source if needed
    }
}

// ============================================================================
// SerializeSelection
// ============================================================================
HRESULT ClipboardManager::SerializeSelection()
{
    // TODO: Query the timeline controller for the current selection,
    // serialize each selected item into a ClipboardEntry, and store
    // them in m_clipboardData.

    // Placeholder: return S_FALSE (no selection)
    return S_FALSE;
}

// ============================================================================
// DeserializeAndPaste
// ============================================================================
HRESULT ClipboardManager::DeserializeAndPaste()
{
    if (m_clipboardData.empty())
        return S_FALSE;

    // TODO: For each entry in m_clipboardData, deserialize and insert
    // at the current timeline cursor position via the timeline controller.

    return S_OK;
}

// ============================================================================
// ReleaseClipboardData
// ============================================================================
void ClipboardManager::ReleaseClipboardData()
{
    m_clipboardData.clear();
    m_bHasData = false;
}
