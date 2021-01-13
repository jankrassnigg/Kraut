#include "PCH.h"

#include "../../Undo/TreeUndo.h"
#include "qtTreeEditWidget.moc.h"
#include <KrautFoundation/FileSystem/FileIn.h>

void qtTreeEditWidget::on_SpinBranchSegmentLength_valueChanged(int i)
{
  m_pCurNT->m_iSegmentLengthCM = i;

  AE_BROADCAST_EVENT(aeTreeEdit_TreeModified);
}

void qtTreeEditWidget::on_SpinBranchlessPartStart_valueChanged(double d)
{
  m_pCurNT->m_fBranchlessPartABS = d;

  AE_BROADCAST_EVENT(aeTreeEdit_TreeModified);
}

void qtTreeEditWidget::on_SpinMinBranchThickness_valueChanged(int i)
{
  AE_PREVENT_RECURSION;

  m_pCurNT->m_uiMinBranchThicknessInCM = i;
  m_pCurNT->m_uiMaxBranchThicknessInCM = aeMath::Max(m_pCurNT->m_uiMinBranchThicknessInCM, m_pCurNT->m_uiMaxBranchThicknessInCM);

  SpinMaxBranchThickness->setValue(m_pCurNT->m_uiMaxBranchThicknessInCM);

  AE_BROADCAST_EVENT(aeTreeEdit_TreeModified);
}

void qtTreeEditWidget::on_SpinMaxBranchThickness_valueChanged(int i)
{
  AE_PREVENT_RECURSION;

  m_pCurNT->m_uiMaxBranchThicknessInCM = i;
  m_pCurNT->m_uiMinBranchThicknessInCM = aeMath::Min(m_pCurNT->m_uiMinBranchThicknessInCM, m_pCurNT->m_uiMaxBranchThicknessInCM);

  SpinMinBranchThickness->setValue(m_pCurNT->m_uiMinBranchThicknessInCM);

  AE_BROADCAST_EVENT(aeTreeEdit_TreeModified);
}

void qtTreeEditWidget::on_SpinBranchlessPartEnd_valueChanged(double d)
{
  m_pCurNT->m_fBranchlessPartEndABS = d;

  AE_BROADCAST_EVENT(aeTreeEdit_TreeModified);
}

void qtTreeEditWidget::on_spin_BBoxAdjustment_valueChanged(double d)
{
  g_Tree.ChangeBoundingAdjustment((float)d);
}


void qtTreeEditWidget::on_SpinMinBranchesPerNode_valueChanged(int i)
{
  AE_PREVENT_RECURSION;

  m_pCurNT->m_uiMinBranches = i;
  m_pCurNT->m_uiMaxBranches = aeMath::Max(m_pCurNT->m_uiMinBranches, m_pCurNT->m_uiMaxBranches);

  SpinMaxBranchesPerNode->setValue(m_pCurNT->m_uiMaxBranches);

  AE_BROADCAST_EVENT(aeTreeEdit_TreeModified);
}

void qtTreeEditWidget::on_SpinMaxBranchesPerNode_valueChanged(int i)
{
  AE_PREVENT_RECURSION;

  m_pCurNT->m_uiMaxBranches = i;
  m_pCurNT->m_uiMinBranches = aeMath::Min(m_pCurNT->m_uiMinBranches, m_pCurNT->m_uiMaxBranches);

  SpinMinBranchesPerNode->setValue(m_pCurNT->m_uiMinBranches);

  AE_BROADCAST_EVENT(aeTreeEdit_TreeModified);
}

void qtTreeEditWidget::on_SpinFreeSpaceBeforeNode_valueChanged(double d)
{
  m_pCurNT->m_fNodeSpacingBefore = d;

  AE_BROADCAST_EVENT(aeTreeEdit_TreeModified);
}

void qtTreeEditWidget::on_SpinFreeSpaceAfterNode_valueChanged(double d)
{
  m_pCurNT->m_fNodeSpacingAfter = d;

  AE_BROADCAST_EVENT(aeTreeEdit_TreeModified);
}

void qtTreeEditWidget::on_SpinNodeHeight_valueChanged(double d)
{
  m_pCurNT->m_fNodeHeight = d;

  AE_BROADCAST_EVENT(aeTreeEdit_TreeModified);
}

void qtTreeEditWidget::on_ComboBranchTypeMode_currentIndexChanged(int index)
{
  m_pCurNT->m_BranchTypeMode = (Kraut::BranchTypeMode::Enum)index;

  UpdateBranchTypeModeGUI();

  AE_BROADCAST_EVENT(aeTreeEdit_TreeModified);
}


SLIDER_UNDO(SliderLowerBound);

void qtTreeEditWidget::on_SliderLowerBound_valueChanged()
{
  m_pCurNT->m_uiLowerBound = SliderLowerBound->value();

  m_pCurNT->m_uiUpperBound = aeMath::Max(m_pCurNT->m_uiUpperBound, m_pCurNT->m_uiLowerBound);
  SliderUpperBound->setValue(m_pCurNT->m_uiUpperBound);

  AE_BROADCAST_EVENT(aeTreeEdit_TreeModified);
}

SLIDER_UNDO(SliderUpperBound);

void qtTreeEditWidget::on_SliderUpperBound_valueChanged()
{
  m_pCurNT->m_uiUpperBound = SliderUpperBound->value();

  m_pCurNT->m_uiLowerBound = aeMath::Min(m_pCurNT->m_uiLowerBound, m_pCurNT->m_uiUpperBound);
  SliderLowerBound->setValue(m_pCurNT->m_uiLowerBound);

  AE_BROADCAST_EVENT(aeTreeEdit_TreeModified);
}
