#include "PCH.h"

#include "TreeUndo.h"
#include "../Tree/Tree.h"
#include "../GUI/qtTreeEditWidget/qtTreeEditWidget.moc.h"
#include <KrautFoundation/Logging/Log.h>

aeInt32 aeUndo::s_iInOperation = 0;
bool aeUndo::s_bAddedAny = false;
aeInt32 aeUndo::s_iOpsUndone = 0;
aeDeque<aeMemoryStream> aeUndo::s_UndoStack;
bool aeUndo::s_bHasBeenModified = false;
aeInt32 aeUndo::s_iSavedStateDiff = 0;

AE_ON_GLOBAL_EVENT (aeTreePlugin_TreeLoaded)
{
  aeUndo::ClearHistory ();
}

AE_ON_GLOBAL_EVENT (aeTreePlugin_TreeSaved)
{
  aeUndo::ResetModified ();
}

void aeUndo::ClearHistory (void)
{
  s_iOpsUndone = 0;
  s_UndoStack.clear ();
  ResetModified ();

  qtTreeEditWidget::s_pWidget->actionUndo->setDisabled (true);
  qtTreeEditWidget::s_pWidget->actionRedo->setDisabled (true);
}

void aeUndo::BeginOperation (void)
{
  ++s_iInOperation;

  if (s_iInOperation == 1)
    s_bAddedAny = false;
}

void aeUndo::EndOperation (void)
{
  if (s_iInOperation == 0)
    return;

  --s_iInOperation;
}

void aeUndo::ModifyTree (aeTree* pTree)
{
  BeginOperation ();

  while (s_iOpsUndone > 0)
  {
    s_UndoStack.pop_back ();
    --s_iOpsUndone;
  }

  if (s_iInOperation > 0)
  {
    if (s_bAddedAny)
      s_UndoStack.pop_back ();

    s_bAddedAny = true;
  }

  

  s_UndoStack.push_back (aeMemoryStream ());
  pTree->Save (s_UndoStack.back ());

  if (s_UndoStack.size () >= 2)
  {
    const aeDeque<aeUInt8>& s1 = s_UndoStack[s_UndoStack.size () - 1].GetDataStream ();
    const aeDeque<aeUInt8>& s2 = s_UndoStack[s_UndoStack.size () - 2].GetDataStream ();

    // remove modifications that did not modify anything
    if (s1.size () == s2.size ())
    {
      const aeUInt32 uiSize = s1.size ();
      for (aeUInt32 i = 0; i < uiSize; ++i)
      {
        if (s1[i] != s2[i])
          goto wasmodified;
      }

      s_UndoStack.pop_back ();
      s_bAddedAny = false;
    }
  }

wasmodified:
  if (s_UndoStack.size () > 1)
  {
    s_iSavedStateDiff++;

    if (!s_bHasBeenModified)
      AE_BROADCAST_EVENT (aeEditor_WorkspaceModified);

    qtTreeEditWidget::s_pWidget->actionUndo->setDisabled (false);
    qtTreeEditWidget::s_pWidget->actionRedo->setDisabled (true);

    s_bHasBeenModified = true;
  }

  EndOperation ();
}

AE_ON_GLOBAL_EVENT (aeTreeEdit_UndoTriggered)
{
  aeUndo::Undo (&g_Tree);
}

void aeUndo::Undo (aeTree* pTarget)
{
  aeInt32 iOpsUndoable = ((aeInt32) s_UndoStack.size ()) - 1;
  if (iOpsUndoable <= s_iOpsUndone)
    return;

  pTarget->m_bModifiedViaUndo = true;

  ++s_iOpsUndone;
  --s_iSavedStateDiff;

  AE_BROADCAST_EVENT (aeTreePlugin_BeforeUndo);

  aeMemoryStream Temp = s_UndoStack[iOpsUndoable - s_iOpsUndone];
  pTarget->Load (Temp);

  AE_BROADCAST_EVENT (aeTreePlugin_AfterUndo);

  AE_BROADCAST_EVENT (aeTreeEdit_TreeModified);

  if (s_iSavedStateDiff == 0)
  {
    s_bHasBeenModified = false;
    AE_BROADCAST_EVENT (aeEditor_WorkspaceUnmodified);
  }
  else
  if (!s_bHasBeenModified)
  {
    s_bHasBeenModified = true;
    AE_BROADCAST_EVENT (aeEditor_WorkspaceModified);
  }

  qtTreeEditWidget::s_pWidget->actionUndo->setDisabled (iOpsUndoable <= s_iOpsUndone);
  qtTreeEditWidget::s_pWidget->actionRedo->setDisabled (false);
}

AE_ON_GLOBAL_EVENT (aeTreeEdit_RedoTriggered)
{
  aeUndo::Redo (&g_Tree);
}

void aeUndo::Redo (aeTree* pTarget)
{
  if (s_iOpsUndone <= 0)
    return;

  pTarget->m_bModifiedViaUndo = true;

  AE_BROADCAST_EVENT (aeTreePlugin_BeforeRedo);

  aeMemoryStream Temp = s_UndoStack[s_UndoStack.size () - s_iOpsUndone];
  pTarget->Load (Temp);
  --s_iOpsUndone;
  ++s_iSavedStateDiff;

  AE_BROADCAST_EVENT (aeTreePlugin_AfterRedo);

  AE_BROADCAST_EVENT (aeTreeEdit_TreeModified);

  if (s_iSavedStateDiff == 0)
  {
    s_bHasBeenModified = false;
    AE_BROADCAST_EVENT (aeEditor_WorkspaceUnmodified);
  }
  else
  if (!s_bHasBeenModified)
  {
    s_bHasBeenModified = true;
    AE_BROADCAST_EVENT (aeEditor_WorkspaceModified);
  }

  qtTreeEditWidget::s_pWidget->actionUndo->setDisabled (false);
  qtTreeEditWidget::s_pWidget->actionRedo->setDisabled (s_iOpsUndone <= 0);
}





