#include "pch.h"
#include "UndoManager.h"

UndoManager::UndoManager()
    : m_bInTransaction(false)
{
}

UndoManager::~UndoManager()
{
    Clear();
}

HRESULT UndoManager::Push(UndoAction doAction, UndoAction undoAction)
{
    if (!doAction || !undoAction)
        return E_INVALIDARG;

    UndoEntry entry;
    entry.doAction = doAction;
    entry.undoAction = undoAction;

    if (m_bInTransaction)
    {
        m_transactionStack.push_back(entry);
    }
    else
    {
        m_undoStack.push_back(entry);
        m_redoStack.clear();
    }

    return S_OK;
}

void UndoManager::Clear()
{
    m_undoStack.clear();
    m_redoStack.clear();
    m_transactionStack.clear();
    m_bInTransaction = false;
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

// ============================================================================
// Transaction grouping
// ============================================================================

HRESULT UndoManager::BeginTransaction()
{
    if (m_bInTransaction)
        return E_UNEXPECTED;

    m_bInTransaction = true;
    m_transactionStack.clear();
    return S_OK;
}

HRESULT UndoManager::EndTransaction()
{
    if (!m_bInTransaction)
        return E_UNEXPECTED;

    m_bInTransaction = false;

    if (m_transactionStack.empty())
        return S_OK;

    // Move the accumulated entries into a composite undo entry.
    // The do/undo lambdas capture the sub-entries by value so they
    // remain valid even after the transaction vectors are cleared.
    std::vector<UndoEntry> entries;
    entries.swap(m_transactionStack);
    m_transactionStack.clear();

    UndoEntry composite;
    composite.doAction = [entries]() -> HRESULT
    {
        for (size_t i = 0; i < entries.size(); ++i)
        {
            HRESULT hr = entries[i].doAction();
            if (FAILED(hr))
                return hr;
        }
        return S_OK;
    };

    composite.undoAction = [entries]() -> HRESULT
    {
        for (size_t i = entries.size(); i > 0; --i)
        {
            HRESULT hr = entries[i - 1].undoAction();
            if (FAILED(hr))
                return hr;
        }
        return S_OK;
    };

    m_undoStack.push_back(composite);
    m_redoStack.clear();
    return S_OK;
}

void UndoManager::CancelTransaction()
{
    m_bInTransaction = false;
    m_transactionStack.clear();
}

bool UndoManager::InTransaction() const throw()
{
    return m_bInTransaction;
}
