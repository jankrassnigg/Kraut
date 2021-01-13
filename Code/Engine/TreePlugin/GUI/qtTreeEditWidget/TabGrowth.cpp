#include "PCH.h"

#include "../../Tree/Tree.h"
#include "qtTreeEditWidget.moc.h"
#include "../../Undo/TreeUndo.h"




SLIDER_UNDO (slider_BranchRotationalDeviation);

void qtTreeEditWidget::on_slider_BranchRotationalDeviation_valueChanged ()
{
  m_pCurNT->m_fMaxRotationalDeviation = slider_BranchRotationalDeviation->value ();

  AE_BROADCAST_EVENT (aeTreeEdit_TreeModified);
}

SLIDER_UNDO (slider_BranchAngle);

void qtTreeEditWidget::on_slider_BranchAngle_valueChanged ()
{
  m_pCurNT->m_fBranchAngle = slider_BranchAngle->value ();

  AE_BROADCAST_EVENT (aeTreeEdit_TreeModified);
}

SLIDER_UNDO (slider_BranchAngleDeviation);

void qtTreeEditWidget::on_slider_BranchAngleDeviation_valueChanged ()
{
  m_pCurNT->m_fMaxBranchAngleDeviation = slider_BranchAngleDeviation->value ();

  AE_BROADCAST_EVENT (aeTreeEdit_TreeModified);
}

SLIDER_UNDO (slider_BranchDirectionDeviation);

void qtTreeEditWidget::on_slider_BranchDirectionDeviation_valueChanged ()
{
  m_pCurNT->m_fMaxTargetDirDeviation = slider_BranchDirectionDeviation->value ();

  AE_BROADCAST_EVENT (aeTreeEdit_TreeModified);
}

void qtTreeEditWidget::on_check_TargetDirRelative_clicked ()
{
  m_pCurNT->m_bTargetDirRelative = check_TargetDirRelative->isChecked ();

  AE_BROADCAST_EVENT (aeTreeEdit_TreeModified);
}

void qtTreeEditWidget::on_combo_BranchTargetDir_currentIndexChanged (int index)
{
  m_pCurNT->m_TargetDirection = (Kraut::BranchTargetDir::Enum) index;

  AE_BROADCAST_EVENT (aeTreeEdit_TreeModified);
}

void qtTreeEditWidget::on_combo_BranchTargetDir2_currentIndexChanged (int index)
{
  m_pCurNT->m_TargetDirection2 = (Kraut::BranchTargetDir::Enum) index;

  AE_BROADCAST_EVENT (aeTreeEdit_TreeModified);
}

SLIDER_UNDO (slider_SecondDirUsage);

void qtTreeEditWidget::on_slider_SecondDirUsage_valueChanged ()
{
  m_pCurNT->m_fTargetDir2Usage = slider_SecondDirUsage->value () / 100.0f;

  AE_BROADCAST_EVENT (aeTreeEdit_TreeModified);
}

void qtTreeEditWidget::on_combo_BranchSecondDirMode_currentIndexChanged (int index)
{
  m_pCurNT->m_TargetDir2Uage = (Kraut::BranchTargetDir2Usage::Enum) index;

  slider_SecondDirUsage->setEnabled (m_pCurNT->m_TargetDir2Uage != Kraut::BranchTargetDir2Usage::Off);
  combo_BranchTargetDir2->setEnabled (m_pCurNT->m_TargetDir2Uage != Kraut::BranchTargetDir2Usage::Off);

  AE_BROADCAST_EVENT (aeTreeEdit_TreeModified);
}

void qtTreeEditWidget::on_check_ActAsObstacle_clicked ()
{
  m_pCurNT->m_bActAsObstacle = check_ActAsObstacle->isChecked ();

  if (g_Globals.s_bDoPhysicsSimulation)
    AE_BROADCAST_EVENT (aeTreeEdit_TreeModified);
}

void qtTreeEditWidget::on_check_EnableObstacleAvoidance_clicked ()
{
  m_pCurNT->m_bDoPhysicalSimulation = check_EnableObstacleAvoidance->isChecked ();

  if (g_Globals.s_bDoPhysicsSimulation)
    AE_BROADCAST_EVENT (aeTreeEdit_TreeModified);
}

SLIDER_UNDO (slider_ObstacleLookAhead);

void qtTreeEditWidget::on_slider_ObstacleLookAhead_valueChanged ()
{
  m_pCurNT->m_fPhysicsLookAhead = slider_ObstacleLookAhead->value () / 100.0f;

  if ((g_Globals.s_bDoPhysicsSimulation) && (m_pCurNT->m_bDoPhysicalSimulation))
    AE_BROADCAST_EVENT (aeTreeEdit_TreeModified);
}

SLIDER_UNDO (slider_ObstacleEvasionAngle);

void qtTreeEditWidget::on_slider_ObstacleEvasionAngle_valueChanged ()
{
  m_pCurNT->m_fPhysicsEvasionAngle = slider_ObstacleEvasionAngle->value ();

  if ((g_Globals.s_bDoPhysicsSimulation) && (m_pCurNT->m_bDoPhysicalSimulation))
    AE_BROADCAST_EVENT (aeTreeEdit_TreeModified);
}

SLIDER_UNDO (slider_BranchGrowDirDev);

void qtTreeEditWidget::on_slider_BranchGrowDirDev_valueChanged ()
{
  m_pCurNT->m_fGrowMaxTargetDirDeviation = slider_BranchGrowDirDev->value ();

  AE_BROADCAST_EVENT (aeTreeEdit_TreeModified);
}

void qtTreeEditWidget::on_spin_BranchSegmentDirChange_valueChanged (int i)
{
  m_pCurNT->m_fGrowMaxDirChangePerSegment = i;

  AE_BROADCAST_EVENT (aeTreeEdit_TreeModified);
}

void qtTreeEditWidget::on_spin_MinBranchLength_valueChanged (double d)
{
  m_pCurNT->m_uiMinBranchLengthInCM = (aeUInt32) (spin_MinBranchLength->value () * 100);
  m_pCurNT->m_uiMaxBranchLengthInCM = aeMath::Clamp<aeUInt16> (m_pCurNT->m_uiMaxBranchLengthInCM, m_pCurNT->m_uiMinBranchLengthInCM, 10000);

  spin_MaxBranchLength->setValue (m_pCurNT->m_uiMaxBranchLengthInCM / 100.0);

  AE_BROADCAST_EVENT (aeTreeEdit_TreeModified);\
}

void qtTreeEditWidget::on_spin_MaxBranchLength_valueChanged (double d)
{
  m_pCurNT->m_uiMaxBranchLengthInCM = (aeUInt32) (spin_MaxBranchLength->value () * 100);
  m_pCurNT->m_uiMinBranchLengthInCM = aeMath::Clamp<aeUInt16> (m_pCurNT->m_uiMinBranchLengthInCM, 0, m_pCurNT->m_uiMaxBranchLengthInCM);

  spin_MinBranchLength->setValue (m_pCurNT->m_uiMinBranchLengthInCM / 100.0);

  AE_BROADCAST_EVENT (aeTreeEdit_TreeModified);
}

void qtTreeEditWidget::on_check_RestrictGrowToUpPlane_clicked ()
{
  m_pCurNT->m_bRestrictGrowthToFrondPlane = check_RestrictGrowToUpPlane->isChecked ();
  AE_BROADCAST_EVENT (aeTreeEdit_TreeModified);
}
