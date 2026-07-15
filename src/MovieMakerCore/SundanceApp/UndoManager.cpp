#include "pch.h"
#include "UndoManager.h"

UndoManager::UndoManager() {}
UndoManager::~UndoManager() {}
void UndoManager::Clear() {}
HRESULT UndoManager::Undo() { return E_NOTIMPL; }
HRESULT UndoManager::Redo() { return E_NOTIMPL; }
bool UndoManager::CanUndo() const throw() { return false; }
bool UndoManager::CanRedo() const throw() { return false; }
