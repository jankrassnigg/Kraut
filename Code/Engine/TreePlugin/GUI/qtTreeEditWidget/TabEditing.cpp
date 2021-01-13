#include "PCH.h"

#include "../../Basics/Plugin.h"
#include "../../Tree/Tree.h"
#include "../../Undo/TreeUndo.h"
#include "qtTreeEditWidget.moc.h"


//
//AE_ON_GLOBAL_EVENT(aeTreePlugin_SelectedBranchChanged)
//{
//  if ((param0.iInt32 != -1) && (g_Tree.m_TreeStructure.m_Branches[param0.iInt32]->m_bManuallyCreated))
//  {
//    Kraut::BranchStructure* pBranch = g_Tree.m_TreeStructure.m_Branches[param0.iInt32].get();
//
//    qtTreeEditWidget::s_pWidget->SetCurrentlyEditedBranchType(pBranch->m_Type, false);
//    qtTreeEditWidget::s_pWidget->button_DeleteBranch->setDisabled(false);
//  }
//  else
//  {
//    qtTreeEditWidget::s_pWidget->button_DeleteBranch->setDisabled(true);
//  }
//}

void qtTreeEditWidget::on_button_MakeEditable_clicked()
{
  //if (QMessageBox::question(this, QString::fromUtf8("Kraut"), QString::fromUtf8("This operation will make the entire tree manually editable in its current form.\nThis will disable all procedural tree creation and the links between the different component types will be disconnected (see the Tree Component Editor).\nIf you want to grow additional procedural trees on the manual tree, you can re-enable certain links in the Tree Component Editor again.\nPlease note that the skeleton of manual branches will not be affected by changes to their branch type. Their overall appearance however will still be affected.\n\nDo you want to continue?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes)
  //  return;

  //for (aeUInt32 b = 0; b < g_Tree.m_TreeStructure.m_Branches.size(); ++b)
  //  g_Tree.m_TreeStructure.m_Branches[b]->m_bManuallyCreated = true;

  //for (aeUInt32 bt = 0; bt < Kraut::BranchType::ENUM_COUNT; ++bt)
  //{
  //  g_Tree.m_Descriptor.m_StructureDesc.m_BranchTypes[bt].m_bAllowSubType[0] = false;
  //  g_Tree.m_Descriptor.m_StructureDesc.m_BranchTypes[bt].m_bAllowSubType[1] = false;
  //  g_Tree.m_Descriptor.m_StructureDesc.m_BranchTypes[bt].m_bAllowSubType[2] = false;
  //}

  //g_Tree.m_Descriptor.m_StructureDesc.m_bGrowProceduralTrunks = false;
  //check_CreateProcedural->setChecked(g_Tree.m_Descriptor.m_StructureDesc.m_bGrowProceduralTrunks);

  //AE_BROADCAST_EVENT(aeTreeEdit_TreeModified);
}

void qtTreeEditWidget::on_actionDeleteBranch_triggered()
{
  //AE_BROADCAST_EVENT(aeTreeEdit_DeleteSelectedBranch);
}

void qtTreeEditWidget::on_button_DeleteBranch_clicked()
{
  //AE_BROADCAST_EVENT(aeTreeEdit_DeleteSelectedBranch);
}

void qtTreeEditWidget::on_check_ModifyLocal_clicked()
{
  //g_Globals.s_bModifyLocally = check_ModifyLocal->isChecked();
}

void qtTreeEditWidget::on_slider_ModifyStrength_valueChanged()
{
  //g_Globals.s_uiModifyStrength = slider_ModifyStrength->value();
}

void qtTreeEditWidget::on_check_VisPaintingPlane_clicked()
{
  //g_Globals.s_bVisualizePaintingPlane = check_VisPaintingPlane->isChecked();

  //AE_BROADCAST_EVENT(aeEditor_QueueRedraw);
}

void qtTreeEditWidget::on_radio_PaintInViewplane_clicked()
{
  //if (radio_PaintInViewplane->isChecked())
  //  g_Globals.s_PaintingPlaneMode = aePaintingPlaneMode::InViewPlane;
}

void qtTreeEditWidget::on_radio_PaintUpwards_clicked()
{
  //if (radio_PaintUpwards->isChecked())
  //  g_Globals.s_PaintingPlaneMode = aePaintingPlaneMode::Upwards;
}

void qtTreeEditWidget::on_radio_PaintPlanar_clicked()
{
  //if (radio_PaintPlanar->isChecked())
  //  g_Globals.s_PaintingPlaneMode = aePaintingPlaneMode::Planar;
}

void qtTreeEditWidget::on_radio_PaintAlongBranch_clicked()
{
  //if (radio_PaintAlongBranch->isChecked())
  //  g_Globals.s_PaintingPlaneMode = aePaintingPlaneMode::AlongBranch;
}

void qtTreeEditWidget::on_radio_PaintOrthogonal_clicked()
{
  //if (radio_PaintOrthogonal->isChecked())
  //  g_Globals.s_PaintingPlaneMode = aePaintingPlaneMode::OrthogonalToBranch;
}

void qtTreeEditWidget::on_check_CreateProcedural_clicked()
{
  //g_Tree.m_Descriptor.m_StructureDesc.m_bGrowProceduralTrunks = check_CreateProcedural->isChecked();

  //AE_BROADCAST_EVENT(aeTreeEdit_TreeModified);
}
