#pragma once
#ifndef SUNDANCE_UNDO_MANAGER_H
#define SUNDANCE_UNDO_MANAGER_H

#include "../pch.h"
#include "../MovieMakerCore.h"

class UndoManager
{
public:
    UndoManager();
    ~UndoManager();

    void Clear();
    HRESULT Undo();
    HRESULT Redo();
    bool CanUndo() const throw();
    bool CanRedo() const throw();

private:
    UndoManager(const UndoManager&);
    UndoManager& operator=(const UndoManager&);
};

#endif
