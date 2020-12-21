#ifndef AE_TREEPLUGIN_TREEUNDO_H
#define AE_TREEPLUGIN_TREEUNDO_H

#include <KrautFoundation/Containers/Deque.h>
#include <KrautFoundation/Streams/MemoryStream.h>

using namespace AE_NS_FOUNDATION;

class aeTree;

class aeUndo
{
public:
  static void ClearHistory (void);

  static void BeginOperation (void);
  static void EndOperation (void);

  static void ModifyTree (aeTree* pTree);

  static void Undo (aeTree* pTarget);
  static void Redo (aeTree* pTarget);

  static void ResetModified (void) { s_bHasBeenModified = false; s_iSavedStateDiff = 0; }
  static bool HasBeenModified (void) { return s_bHasBeenModified; }

  //! Returns true if an undoable operation is currently in the process of being written to a memory stream.
  static bool IsStoringUndoOperation (void) { return s_iInOperation > 0; }

private:
  static bool s_bHasBeenModified;
  static aeInt32 s_iInOperation;
  static bool s_bAddedAny;
  static aeInt32 s_iOpsUndone;
  static aeDeque<aeMemoryStream> s_UndoStack;
  static aeInt32 s_iSavedStateDiff;
};


#endif

