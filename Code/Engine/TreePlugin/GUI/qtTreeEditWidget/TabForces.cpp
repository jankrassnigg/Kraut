#include "PCH.h"
#include "qtTreeEditWidget.moc.h"
#include "../../Basics/Plugin.h"
#include "../../Tree/Tree.h"
#include "../../Physics/Forces.h"

AE_ON_GLOBAL_EVENT (aeTreeEdit_SelectedForceChanged)
{
  g_Globals.s_bUpdatePickingBuffer = true;

  if (qtTreeEditWidget::s_pWidget)
    qtTreeEditWidget::s_pWidget->UpdateForcesUI ();
}

AE_ON_GLOBAL_EVENT (aeTreeEdit_ForceListChanged)
{
  g_Globals.s_bUpdatePickingBuffer = true;

  if (qtTreeEditWidget::s_pWidget)
    qtTreeEditWidget::s_pWidget->UpdateForcesUI ();
}

AE_ON_GLOBAL_EVENT (aeTreeEdit_SelectedForceModified)
{
  g_Globals.s_bUpdatePickingBuffer = true;

  if (qtTreeEditWidget::s_pWidget)
    qtTreeEditWidget::s_pWidget->UpdateForcesUI ();
}

void qtTreeEditWidget::UpdateForcesUI (void)
{
  AE_PREVENT_RECURSION;

  QListWidget* pList = ListForces;

  const bool b = pList->blockSignals (true);

  pList->clear ();

  aeDeque<aeForce*>& Forces = g_Tree.m_Descriptor.m_Forces;

  for (aeUInt32 i = 0; i < Forces.size (); ++i)
  {
    QListWidgetItem* pItem = new QListWidgetItem ();
    
    pList->addItem (pItem);
    
    pItem->setFlags (Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable);
    pItem->setData (Qt::CheckStateRole, Forces[i]->IsActive () ? Qt::Checked : Qt::Unchecked);
    pItem->setData (Qt::UserRole, QVariant::fromValue<void*> (Forces[i]));
    pItem->setText (QString::fromUtf8 (Forces[i]->GetDescription ().c_str ()));

    Forces[i]->m_pListItem = pItem;
  }

  const aeForce* pSelectedForce = aeForce::GetSelectedForce ();
  const bool bForceSelected = pSelectedForce != NULL;

  if (bForceSelected)
  {
    ComboForceType->setCurrentIndex ((aeInt32) pSelectedForce->GetType ());
    ComboForceFalloff->setCurrentIndex ((aeInt32) pSelectedForce->GetFalloff ());
    pSelectedForce->m_pListItem->setSelected (true);

    SpinForceStrength->setValue (pSelectedForce->GetStrength ());

    SpinForceMinRadius->setEnabled (pSelectedForce->EnableMinRadius ());
    SpinForceMaxRadius->setEnabled (pSelectedForce->EnableMaxRadius ());

    LineForceMesh->setEnabled (pSelectedForce->EnableMeshSelector ());
    ButtonSelectForceMesh->setEnabled (pSelectedForce->EnableMeshSelector ());

    LineForceMesh->setText (pSelectedForce->GetMesh ());
    SpinForceMinRadius->setValue (pSelectedForce->GetMinRadius ());
    SpinForceMaxRadius->setValue (pSelectedForce->GetMaxRadius ());

    QCheckBox* CheckInfluence[aeBranchType::ENUM_COUNT] =
    { 
      CheckForceTrunk1,        CheckForceTrunk2,        CheckForceTrunk3,
      CheckForceBranches1,     CheckForceBranches2,     CheckForceBranches3,
      CheckForceSubBranches1,  CheckForceSubBranches2,  CheckForceSubBranches3,
      CheckForceTwigs1,        CheckForceTwigs2,        CheckForceTwigs3
    };

    for (aeUInt32 bt = 0; bt < aeBranchType::ENUM_COUNT; ++bt)
    {
      CheckInfluence[bt]->setChecked (pSelectedForce->m_InfluencesBranchTypes.IsAnyFlagSet (1 << bt));
      CheckInfluence[bt]->setVisible (g_Tree.m_Descriptor.m_BranchTypes[bt].m_bUsed);
    }
  }
  else
  {
    LineForceMesh->setEnabled (false);
    ButtonSelectForceMesh->setEnabled (false);
    LineForceMesh->setText ("");
  }

  ButtonDeleteForce->setEnabled    (bForceSelected);
  GroupForceInfluences->setEnabled (bForceSelected);
  GroupSelectedForce->setEnabled   (bForceSelected);

  pList->blockSignals (b);
}

void qtTreeEditWidget::on_ButtonAddForce_clicked ()
{
  g_Tree.AddForce ();
}

void qtTreeEditWidget::on_ButtonDeleteForce_clicked ()
{
  g_Tree.RemoveSelectedForce ();
}

void qtTreeEditWidget::on_ListForces_itemSelectionChanged ()
{
  AE_PREVENT_RECURSION;

  QList<QListWidgetItem*> Selection = ListForces->selectedItems();

  if (Selection.empty ())
    aeForce::SetSelectedForce (NULL);
  else
    aeForce::SetSelectedForce ((aeForce*) Selection[0]->data(Qt::UserRole).value<void*> ());
}

void qtTreeEditWidget::on_ListForces_itemChanged (QListWidgetItem* item)
{
  AE_PREVENT_RECURSION;

  aeForce* pForce = ((aeForce*) item->data(Qt::UserRole).value<void*> ());
  pForce->SetName (item->text().toUtf8 ().data ());
  pForce->SetActive (item->checkState() == Qt::Checked);
}

void qtTreeEditWidget::on_ComboForceType_currentIndexChanged (int index)
{
  AE_PREVENT_RECURSION;

  if (aeForce::GetSelectedForce ())
    aeForce::GetSelectedForce ()->SetType ((aeForceType::Enum) index);
}

void qtTreeEditWidget::on_ComboForceFalloff_currentIndexChanged (int index)
{
  AE_PREVENT_RECURSION;

  if (aeForce::GetSelectedForce ())
    aeForce::GetSelectedForce ()->SetFalloff ((aeForceFalloff::Enum) index);
}


void qtTreeEditWidget::on_SpinForceStrength_valueChanged (double d)
{
  AE_PREVENT_RECURSION;

  if (aeForce::GetSelectedForce ())
    aeForce::GetSelectedForce ()->SetStrength (d);
}

void qtTreeEditWidget::on_SpinForceMinRadius_valueChanged (double d)
{
  AE_PREVENT_RECURSION;

  if (aeForce::GetSelectedForce ())
  {
    aeForce::GetSelectedForce ()->SetMinRadius (d);
    SpinForceMaxRadius->setValue (aeForce::GetSelectedForce ()->GetMaxRadius ());
  }
}

void qtTreeEditWidget::on_SpinForceMaxRadius_valueChanged (double d)
{
  AE_PREVENT_RECURSION;

  if (aeForce::GetSelectedForce ())
  {
    aeForce::GetSelectedForce ()->SetMaxRadius (d);
    SpinForceMinRadius->setValue (aeForce::GetSelectedForce ()->GetMinRadius ());
  }
}

void qtTreeEditWidget::GetForceInfluences (void)
{
  AE_PREVENT_RECURSION;

  if (aeForce::GetSelectedForce ())
  {
    QCheckBox* CheckInfluence[aeBranchType::ENUM_COUNT] =
    { 
      CheckForceTrunk1,        CheckForceTrunk2,        CheckForceTrunk3,
      CheckForceBranches1,     CheckForceBranches2,     CheckForceBranches3,
      CheckForceSubBranches1,  CheckForceSubBranches2,  CheckForceSubBranches3,
      CheckForceTwigs1,        CheckForceTwigs2,        CheckForceTwigs3
    };

    aeFlags32 Influences (0);

    for (aeUInt32 bt = 0; bt < aeBranchType::ENUM_COUNT; ++bt)
    {
      Influences.RaiseOrClearFlags (1 << bt, CheckInfluence[bt]->isChecked ()); 
    }

    aeForce::GetSelectedForce ()->SetBranchInfluences (Influences.GetData ());
  }
}

void qtTreeEditWidget::on_ButtonSelectForceMesh_clicked ()
{
  if (!aeForce::GetSelectedForce ())
    return;

  static aeFilePath s_LastPath;

  aeHybridString<128> sPath;
  if (!SelectFile (s_LastPath, sPath, LineForceMesh, "OBJ Files (*.obj);;All Files (*.*)", "Meshes/"))
    return;

  aeForce::GetSelectedForce ()->SetMesh (sPath.c_str ());

  //ImportPhysicsObject (s_LastPath.c_str (), 1.0f);

  //g_Globals.s_bUpdatePickingBuffer = true;
  //AE_BROADCAST_EVENT (aeEditor_QueueRedraw);

  UpdateForcesUI ();
}

