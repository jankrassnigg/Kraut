#include "PCH.h"

#include "../Tree/Tree.h"
#include "../Basics/Plugin.h"
#include <KrautFoundation/Configuration/VariableRegistry.h>
#include <QMainWindow>



using namespace AE_NS_GRAPHICS;

AE_ON_GLOBAL_EVENT (aeTreeEdit_DeleteSelectedBranch)
{
  if (g_Plugin.GetSelectedBranchID () == -1)
    return;
  if (!g_Tree.m_Branches[g_Plugin.GetSelectedBranchID ()].m_bManuallyCreated)
    return;

  QMainWindow* pMainWindow = NULL;
  aeVariableRegistry::RetrieveRaw ("system/qt/mainwidget", &pMainWindow, sizeof (QMainWindow*));

  if (QMessageBox::question(pMainWindow, QString("Kraut"), QString("Should this entire branch and all its sub-branches be deleted?"), QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No, QMessageBox::StandardButton::Yes) != QMessageBox::StandardButton::Yes)
    return;

  g_Tree.DeleteBranch (g_Plugin.GetSelectedBranchID (), false);
}

void aeTree::DeleteBranch (aeInt32 iBranch, bool bIsRecursion)
{
  m_Branches[iBranch].SetDeleted (true);

  for (aeUInt32 b = 0; b < m_Branches.size (); ++b)
  {
    if (m_Branches[b].IsDeleted ())
      continue;

    if (m_Branches[b].m_iParentBranchID == iBranch)
      DeleteBranch (b, true);
  }

  if (!bIsRecursion)
    AE_BROADCAST_EVENT (aeTreeEdit_TreeModified);
}




