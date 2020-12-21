#include "PCH.h"

#include "../../Tree/Tree.h"
#include "../../Basics/Plugin.h"
#include <KrautFoundation/FileSystem/FileIn.h>
#include <KrautFoundation/FileSystem/FileOut.h>
#include "../qtExportWidget/qtExportWidget.moc.h"
#include "../qtAboutDialog/qtAboutDialog.moc.h"
#include "qtTreeEditWidget.moc.h"

aeFilePath sPrevFile = "";

extern QToolButton* g_pRenderModeButton;
extern QToolButton* g_pTransformModeButton;
extern aeCVarBool CVar_OnExportConvertToDDS;
extern aeDeque<aeFilePath> g_LRU_Trees;

void qtTreeEditWidget::on_spin_RandomSeed_valueChanged (int i)
{
  g_Tree.m_Descriptor.m_uiRandomSeed = i;

  AE_BROADCAST_EVENT (aeTreeEdit_TreeModified);
}

// File Actions


void qtTreeEditWidget::on_actionOpenRecentFile_triggered (QAction* pAction)
{
  if (!g_Plugin.CloseTree ())
    return;

  aeInt32 iEntry = pAction->data ().toInt ();
  const aeFilePath sCurFile = g_LRU_Trees[iEntry];

  if (!g_Plugin.LoadTree (sCurFile.c_str ()))
    QMessageBox::critical (this, "Kraut", "Could not open the selected file.");
}

void qtTreeEditWidget::on_action_New_triggered ()
{
  if (!g_Plugin.CloseTree ())
    return;

  sPrevFile.clear ();
  g_Tree.Reset ();

  SetCurrentlyEditedBranchType (aeBranchType::Trunk1, true);
  combo_LoD->setCurrentIndex (aeLod::Lod0);

  AE_BROADCAST_EVENT (aeEditor_SetWindowTitle, "new");

  AE_BROADCAST_EVENT (aeEditor_WorkspaceUnmodified);

  AE_BROADCAST_EVENT (aeTreePlugin_TreeLoaded, "");

  qtTreeEditWidget::s_pWidget->SetCurrentlyEditedBranchType (aeBranchType::Trunk1, true);
}

void qtTreeEditWidget::on_action_Open_triggered ()
{
  if (!g_Plugin.CloseTree ())
    return;

  aeFilePath sCurFile = sPrevFile;

  if (sCurFile.empty ())
    aeFileSystem::MakeValidPath ("Trees/", false, &sCurFile, NULL);

  QString result = QFileDialog::getOpenFileName (this, 
    QFileDialog::tr ("Load Tree"), sCurFile.c_str (),
	  QFileDialog::tr ("Tree-Files (*.tree);;All Files (*.*)"),	0); 

  if (result.size() == 0)
	  return;

  sPrevFile = result.toLatin1 ().data ();

  if (!g_Plugin.LoadTree (sPrevFile.c_str ()))
    QMessageBox::critical (this, "Kraut", "Could not open the selected file.");
}

bool qtTreeEditWidget::SaveTree (void)
{
  if (sPrevFile == "")
    return SaveTreeAs ();

  aeFileOut f;
  if (f.Open (sPrevFile.c_str ()))
  {
    g_Tree.Save (f);
    AE_BROADCAST_EVENT (aeTreePlugin_TreeSaved, sPrevFile.c_str ());

    AE_BROADCAST_EVENT (aeEditor_WorkspaceUnmodified);

    return true;
  }
  else
  {
    QMessageBox::critical (this, "Kraut", "Could not open the selected file for writing.");
  }

  return false;
}

void qtTreeEditWidget::on_action_Save_triggered ()
{
  SaveTree ();
}

bool qtTreeEditWidget::SaveTreeAs (void)
{
  aeFilePath sCurFile = sPrevFile;

  if (sCurFile.empty ())
    aeFileSystem::MakeValidPath ("Trees/", true, &sCurFile, NULL);

  QString result = QFileDialog::getSaveFileName (this, 
    QFileDialog::tr ("Save Tree"), sCurFile.c_str (),
	  QFileDialog::tr ("Tree-Files (*.tree);;All Files (*.*)"),	0); 

  if (result.size() == 0)
	  return false;

  sPrevFile = result.toLatin1 ().data ();
  g_Tree.m_sTreeFile = sPrevFile;

  aeFileOut f;
  if (f.Open (sPrevFile.c_str ()))
  {
    g_Tree.Save (f);
    AE_BROADCAST_EVENT (aeTreePlugin_TreeSaved, sPrevFile.c_str ());

    AE_BROADCAST_EVENT (aeEditor_SetWindowTitle, sPrevFile.c_str ());
    AE_BROADCAST_EVENT (aeEditor_WorkspaceUnmodified);

    return true;
  }
  else
  {
    QMessageBox::critical (this, "Kraut", "Could not open the selected file for writing.");
    return false;
  }
}

void qtTreeEditWidget::on_action_SaveAs_triggered ()
{
  SaveTreeAs ();
}

void qtTreeEditWidget::on_action_Export_triggered ()
{
  if (g_Tree.m_sTreeFile.empty ())
  {
    on_action_Save_triggered();

    if (g_Tree.m_sTreeFile.empty ())
    {
      QMessageBox::warning (this, "Kraut", "You need to save the tree before it can be exported.");
      return;
    }
  }


  qtExportWidget dialog (this);

  if (dialog.exec () == QDialog::Rejected)
    return;

  // 0 is the maximum possible resolution (render target size 4096 x 1024)
  // divide by 2, 4, 8, 16 via right-shift to get to 512, 256, 128, 64
  const aeUInt32 uiImpostorResolution = 1024 >> aeMath::Clamp<aeInt32> (g_Tree.m_Descriptor.m_iImpostorResolution, 0, 4);

  g_Tree.ExportToFile (qtExportWidget::s_sExportPath.c_str (), 
    qtExportWidget::s_bExportBranches, 
    qtExportWidget::s_bExportFronds, 
    qtExportWidget::s_bExportLeaves, 
    qtExportWidget::s_bCapTrunk, qtExportWidget::s_bCapBranches, 
    qtExportWidget::s_bExportLODs[0], qtExportWidget::s_bExportLODs[1], qtExportWidget::s_bExportLODs[2], qtExportWidget::s_bExportLODs[3], qtExportWidget::s_bExportLODs[4], qtExportWidget::s_bExportLODs[5], 
    CVar_OnExportConvertToDDS, 
    uiImpostorResolution);

  qtExportWidget::RunViewer ();
}

// Render Mode Selection

void qtTreeEditWidget::SetDefaultRenderModeButton (aeTreeRenderMode::Enum Mode)
{
  g_Globals.s_RenderMode = Mode;
  g_pRenderModeButton->setDefaultAction (actionToggleRenderMode);

  switch (Mode)
  {
  case aeTreeRenderMode::Default:
    actionToggleRenderMode->setIcon (actionRenderDefault->icon());
    break;
  case aeTreeRenderMode::Normals:
    actionToggleRenderMode->setIcon (actionRenderNormals->icon());
    break;
  case aeTreeRenderMode::Skeleton:
    actionToggleRenderMode->setIcon (actionRenderSkeleton->icon());
    break;
  case aeTreeRenderMode::Wireframe:
    actionToggleRenderMode->setIcon (actionRenderWireframe->icon());
    break;
  case aeTreeRenderMode::Diffuse:
    actionToggleRenderMode->setIcon (actionRenderDiffuse->icon());
    break;
  case aeTreeRenderMode::Light:
    actionToggleRenderMode->setIcon (actionRenderLight->icon());
    break;
  case aeTreeRenderMode::Ambient:
    actionToggleRenderMode->setIcon (actionRenderAmbient->icon());
    break;
  case aeTreeRenderMode::Skylight:
    actionToggleRenderMode->setIcon (actionRenderSkylight->icon());
    break;
  }

  AE_BROADCAST_EVENT (aeEditor_QueueRedraw);
}

void qtTreeEditWidget::on_actionRenderDefault_triggered ()
{
  SetDefaultRenderModeButton (aeTreeRenderMode::Default);
}

void qtTreeEditWidget::on_actionRenderWireframe_triggered ()
{
  SetDefaultRenderModeButton (aeTreeRenderMode::Wireframe);
}

void qtTreeEditWidget::on_actionRenderSkeleton_triggered ()
{
  SetDefaultRenderModeButton (aeTreeRenderMode::Skeleton);
}

void qtTreeEditWidget::on_actionRenderNormals_triggered ()
{
  SetDefaultRenderModeButton (aeTreeRenderMode::Normals);
}

void qtTreeEditWidget::on_actionRenderDiffuse_triggered ()
{
  SetDefaultRenderModeButton (aeTreeRenderMode::Diffuse);
}

void qtTreeEditWidget::on_actionRenderLight_triggered ()
{
  SetDefaultRenderModeButton (aeTreeRenderMode::Light);
}

void qtTreeEditWidget::on_actionRenderAmbient_triggered ()
{
  SetDefaultRenderModeButton (aeTreeRenderMode::Ambient);
}

void qtTreeEditWidget::on_actionRenderSkylight_triggered ()
{
  SetDefaultRenderModeButton (aeTreeRenderMode::Skylight);
}

void qtTreeEditWidget::on_actionToggleRenderMode_triggered ()
{
  aeInt32 iRenderMode = (aeInt32) g_Globals.s_RenderMode;
  iRenderMode = (iRenderMode + 1) % aeTreeRenderMode::ENUM_COUNT;
  SetDefaultRenderModeButton ((aeTreeRenderMode::Enum) iRenderMode);
}

// Leaves

void qtTreeEditWidget::on_actionShowHideLeaves_triggered ()
{
  g_Globals.s_bRenderLeaves = actionShowHideLeaves->isChecked ();
  AE_BROADCAST_EVENT (aeEditor_QueueRedraw);
}

// Camera

void qtTreeEditWidget::on_actionOrbitCamera_triggered ()
{
  g_Globals.s_bOrbitCamera = actionOrbitCamera->isChecked ();

  if (!g_Globals.s_bOrbitCamera)
    g_Globals.s_vCameraLookDir = g_Globals.s_vCameraPivot - g_Globals.s_vCameraPosition;
  else
  {
    g_Globals.s_vCameraPivot.SetZero ();
    g_Globals.s_vCameraPivot.y = g_Globals.s_vCameraPosition.y;
  }

  AE_BROADCAST_EVENT (aeEditor_QueueRedraw);
}

// Gizmo

void qtTreeEditWidget::on_actionToggleGizmo_triggered ()
{
  switch (aeTransformGizmo::s_GizmoMode)
  {
  case aeTransformGizmo::Translate:
    on_actionRotate_triggered ();
    break;
  case aeTransformGizmo::Rotate:
    on_actionHideGizmos_triggered ();
    break;
  case aeTransformGizmo::Invisible:
    on_actionTranslate_triggered ();
    break;
  }
}

void qtTreeEditWidget::on_actionHideGizmos_triggered ()
{
  g_Globals.s_bUpdatePickingBuffer = true;
  aeTransformGizmo::s_GizmoMode = aeTransformGizmo::Invisible;

  AE_BROADCAST_EVENT (aeEditor_QueueRedraw);
}

void qtTreeEditWidget::on_actionTranslate_triggered ()
{
  g_Globals.s_bUpdatePickingBuffer = true;
  aeTransformGizmo::s_GizmoMode = aeTransformGizmo::Translate;

  AE_BROADCAST_EVENT (aeEditor_QueueRedraw);
}

void qtTreeEditWidget::on_actionRotate_triggered ()
{
  g_Globals.s_bUpdatePickingBuffer = true;
  aeTransformGizmo::s_GizmoMode = aeTransformGizmo::Rotate;

  AE_BROADCAST_EVENT (aeEditor_QueueRedraw);
}

// Undo / Redo

void qtTreeEditWidget::on_actionUndo_triggered ()
{
  AE_BROADCAST_EVENT (aeTreeEdit_UndoTriggered);

  SetCurrentlyEditedBranchType (m_CurrentlyEditedType, true);
}

void qtTreeEditWidget::on_actionRedo_triggered ()
{
  AE_BROADCAST_EVENT (aeTreeEdit_RedoTriggered);

  SetCurrentlyEditedBranchType (m_CurrentlyEditedType, true);
}

void qtTreeEditWidget::on_actionAbout_triggered ()
{
  qtAboutDialog dialog (this);
  dialog.exec ();
}
