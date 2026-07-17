#include "pch.h"
#include "UndoManager.h"

UndoManager::UndoManager() {}
UndoManager::~UndoManager() { Clear(); }

HRESULT UndoManager::Push(UndoAction doAction, UndoAction undoAction)
{
    if (!doAction || !undoAction)
        return E_INVALIDARG;

    UndoEntry entry;
    entry.doAction = doAction;
    entry.undoAction = undoAction;
    m_undoStack.push_back(entry);
    m_redoStack.clear();
    return S_OK;
}

void UndoManager::Clear()
{
    m_undoStack.clear();
    m_redoStack.clear();
}

HRESULT UndoManager::Undo()
{
    if (m_undoStack.empty())
        return S_FALSE;

    UndoEntry entry = m_undoStack.back();
    m_undoStack.pop_back();

    HRESULT hr = entry.undoAction();
    if (FAILED(hr))
        return hr;

    m_redoStack.push_back(entry);
    return S_OK;
}

HRESULT UndoManager::Redo()
{
    if (m_redoStack.empty())
        return S_FALSE;

    UndoEntry entry = m_redoStack.back();
    m_redoStack.pop_back();

    HRESULT hr = entry.doAction();
    if (FAILED(hr))
        return hr;

    m_undoStack.push_back(entry);
    return S_OK;
}

bool UndoManager::CanUndo() const throw()
{
    return !m_undoStack.empty();
}

bool UndoManager::CanRedo() const throw()
{
    return !m_redoStack.empty();
}

size_t UndoManager::GetUndoCount() const throw()
{
    return m_undoStack.size();
}

size_t UndoManager::GetRedoCount() const throw()
{
    return m_redoStack.size();
}
