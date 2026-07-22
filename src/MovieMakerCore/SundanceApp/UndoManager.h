#pragma once
#ifndef SUNDANCE_UNDO_MANAGER_H
#define SUNDANCE_UNDO_MANAGER_H

#include "../pch.h"
#include "../MovieMakerCore.h"

class UndoManager
{
public:
    typedef std::function<HRESULT()> UndoAction;

    UndoManager();
    ~UndoManager();

    HRESULT Push(UndoAction doAction, UndoAction undoAction);
    void Clear();
    HRESULT Undo();
    HRESULT Redo();
    bool CanUndo() const throw();
    bool CanRedo() const throw();
    size_t GetUndoCount() const throw();
    size_t GetRedoCount() const throw();

    // Transaction grouping — groups multiple Push calls into a single undo entry
    HRESULT BeginTransaction();
    HRESULT EndTransaction();
    void    CancelTransaction();
    bool    InTransaction() const throw();

private:
    UndoManager(const UndoManager&);
    UndoManager& operator=(const UndoManager&);

    struct UndoEntry
    {
        UndoAction doAction;
        UndoAction undoAction;
    };

    std::vector<UndoEntry> m_undoStack;
    std::vector<UndoEntry> m_redoStack;

    // Transaction grouping state
    bool                    m_bInTransaction;
    std::vector<UndoEntry>  m_transactionStack;
};

#endif
