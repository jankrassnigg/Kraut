#include "PCH.h"

#include "../../Tree/Tree.h"
#include "qtTreeEditWidget.moc.h"
#include "../../Undo/TreeUndo.h"


void qtTreeEditWidget::on_combo_LoD_currentIndexChanged (int index)
{
  SetCurrentlyEditedLoD ((aeLod::Enum) index);

  
  AE_BROADCAST_EVENT (aeEditor_QueueRedraw);
}

void qtTreeEditWidget::on_check_LodSelection_clicked ()
{
  g_Globals.s_bSelectLodAutomatically = check_LodSelection->isChecked ();

  SpinCrossFade->setEnabled (check_LodSelection->isChecked ());

  AE_BROADCAST_EVENT (aeEditor_QueueRedraw);
}

void qtTreeEditWidget::on_SpinCrossFade_valueChanged (double d)
{
  g_Tree.m_Descriptor.m_fLodCrossFadeTransition = d;

  AE_BROADCAST_EVENT (aeTreeEdit_TreeModifiedSimple);
}

void qtTreeEditWidget::on_combo_LodMode_currentIndexChanged (int index)
{
  g_Tree.m_Descriptor.m_LodData[g_Globals.s_CurLoD].m_Mode = (aeLodMode::Enum) index;

  const bool bDisableLodSettings = (g_Globals.s_CurLoD == aeLod::None) || (aeLodMode::IsImpostorMode (g_Tree.m_Descriptor.m_LodData[g_Globals.s_CurLoD].m_Mode) || g_Tree.m_Descriptor.m_LodData[g_Globals.s_CurLoD].m_Mode == aeLodMode::Disabled);
  group_LODSettings->setDisabled (bDisableLodSettings);

  AE_BROADCAST_EVENT (aeTreeEdit_TreeModified);
}

void qtTreeEditWidget::on_spin_TipDetailThreshold_valueChanged (double d)
{
  g_Tree.m_Descriptor.m_LodData[g_Globals.s_CurLoD].m_fTipDetail = d;

  AE_BROADCAST_EVENT (aeTreeEdit_TreeModified);
}

void qtTreeEditWidget::on_spin_CurvatureThreshold_valueChanged (double d)
{
  g_Tree.m_Descriptor.m_LodData[g_Globals.s_CurLoD].m_fCurvatureThreshold = d;

  AE_BROADCAST_EVENT (aeTreeEdit_TreeModified);
}

void qtTreeEditWidget::on_spin_ThicknessThreshold_valueChanged (double d)
{
  g_Tree.m_Descriptor.m_LodData[g_Globals.s_CurLoD].m_fThicknessThreshold = d;

  AE_BROADCAST_EVENT (aeTreeEdit_TreeModified);
}

void qtTreeEditWidget::on_spin_RingDetailThreshold_valueChanged (double d)
{
  g_Tree.m_Descriptor.m_LodData[g_Globals.s_CurLoD].m_fVertexRingDetail = d;

  AE_BROADCAST_EVENT (aeTreeEdit_TreeModified);
}

void qtTreeEditWidget::on_spin_LodDistance_valueChanged (int i)
{
  g_Tree.m_Descriptor.m_LodData[g_Globals.s_CurLoD].m_uiLodDistance = i;

  AE_BROADCAST_EVENT (aeTreeEdit_TreeModifiedSimple);
}

void qtTreeEditWidget::on_spin_MaxFrondDetail_valueChanged (int i)
{
  g_Tree.m_Descriptor.m_LodData[g_Globals.s_CurLoD].m_iMaxFrondDetail = i;

  AE_BROADCAST_EVENT (aeTreeEdit_TreeModified);
}

void qtTreeEditWidget::on_spin_FrondDetailReduction_valueChanged (int i)
{
  g_Tree.m_Descriptor.m_LodData[g_Globals.s_CurLoD].m_iFrondDetailReduction = i;

  AE_BROADCAST_EVENT (aeTreeEdit_TreeModified);
}



// Branch Meshes

void qtTreeEditWidget::GetAllowedLodMeshes (void)
{
  QCheckBox* cbLod[aeMeshType::ENUM_COUNT][aeBranchType::ENUM_COUNT] =
  {
    { check_LodTrunk1  , check_LodTrunk2  , check_LodTrunk3  , check_LodMainBranches1  , check_LodMainBranches2  , check_LodMainBranches3  , check_LodSubBranches1  , check_LodSubBranches2  , check_LodSubBranches3  , check_LodTwigs1  , check_LodTwigs2  , check_LodTwigs3   },
    { check_LodTrunk1_2, check_LodTrunk2_2, check_LodTrunk3_2, check_LodMainBranches1_2, check_LodMainBranches2_2, check_LodMainBranches3_2, check_LodSubBranches1_2, check_LodSubBranches2_2, check_LodSubBranches3_2, check_LodTwigs1_2, check_LodTwigs2_2, check_LodTwigs3_2 },
    { check_LodTrunk1_3, check_LodTrunk2_3, check_LodTrunk3_3, check_LodMainBranches1_3, check_LodMainBranches2_3, check_LodMainBranches3_3, check_LodSubBranches1_3, check_LodSubBranches2_3, check_LodSubBranches3_3, check_LodTwigs1_3, check_LodTwigs2_3, check_LodTwigs3_3 },
  };

  for (aeUInt32 mt = 0; mt < aeMeshType::ENUM_COUNT; ++mt)
  {
    for (aeUInt32 bt = 0; bt < aeBranchType::ENUM_COUNT; ++bt)
    {
      g_Tree.m_Descriptor.m_LodData[g_Globals.s_CurLoD].m_AllowTypes[mt].RaiseOrClearFlags (1 << bt, cbLod[mt][bt]->isChecked ()); 
    }
  }

  AE_BROADCAST_EVENT (aeTreeEdit_TreeModified);
}

