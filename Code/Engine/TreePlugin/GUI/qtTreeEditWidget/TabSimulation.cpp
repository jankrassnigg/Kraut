#include "PCH.h"

#include "../../Basics/Plugin.h"
#include "../../GUI/ProgressBar.h"
#include "../../Physics/PhysicsObject.h"
#include "../../Tree/Tree.h"
#include "qtTreeEditWidget.moc.h"
#include <KrautGraphics/FileSystem/FileIn.h>
#include <KrautGraphics/FileSystem/FileOut.h>



bool ImportPhysicsObject(const char* szFile, float fScale);

void qtTreeEditWidget::on_check_EnableCollisionAvoidance_clicked()
{
  g_Globals.s_bDoPhysicsSimulation = check_EnableCollisionAvoidance->isChecked();

  AE_BROADCAST_EVENT(aeTreeEdit_TreeModified);
}

void qtTreeEditWidget::on_check_VisualizeCollisionMesh_clicked()
{
  g_Globals.s_bRenderCollisionMesh = check_VisualizeCollisionMesh->isChecked();
  AE_BROADCAST_EVENT(aeEditor_QueueRedraw);
}

void qtTreeEditWidget::on_spin_ColObjScalingX_valueChanged(double val)
{
  if (aePhysicsObject::GetSelected() == nullptr)
    return;

  aeVec3 v = aePhysicsObject::GetSelected()->GetScaling();

  v.x = aeMath::Clamp<float>(val, 0.001f, 10000.0f);
  aePhysicsObject::GetSelected()->SetScaling(v);

  g_Globals.s_bUpdatePickingBuffer = true;
  AE_BROADCAST_EVENT(aeEditor_QueueRedraw);

  if (g_Globals.s_bDoPhysicsSimulation)
    AE_BROADCAST_EVENT(aeTreeEdit_TreeModified);
}

void qtTreeEditWidget::on_spin_ColObjScalingY_valueChanged(double val)
{
  if (aePhysicsObject::GetSelected() == nullptr)
    return;

  aeVec3 v = aePhysicsObject::GetSelected()->GetScaling();

  v.y = aeMath::Clamp<float>(val, 0.001f, 10000.0f);
  aePhysicsObject::GetSelected()->SetScaling(v);

  g_Globals.s_bUpdatePickingBuffer = true;
  AE_BROADCAST_EVENT(aeEditor_QueueRedraw);

  if (g_Globals.s_bDoPhysicsSimulation)
    AE_BROADCAST_EVENT(aeTreeEdit_TreeModified);
}

void qtTreeEditWidget::on_spin_ColObjScalingZ_valueChanged(double val)
{
  if (aePhysicsObject::GetSelected() == nullptr)
    return;

  aeVec3 v = aePhysicsObject::GetSelected()->GetScaling();

  v.z = aeMath::Clamp<float>(val, 0.001f, 10000.0f);
  aePhysicsObject::GetSelected()->SetScaling(v);

  g_Globals.s_bUpdatePickingBuffer = true;
  AE_BROADCAST_EVENT(aeEditor_QueueRedraw);

  if (g_Globals.s_bDoPhysicsSimulation)
    AE_BROADCAST_EVENT(aeTreeEdit_TreeModified);
}


void qtTreeEditWidget::on_button_AddCollisionObject_clicked()
{
  static aeString s_LastPath;
  aeString sCurFile = s_LastPath;

  if (sCurFile.empty())
    aeFileSystem::MakeValidPath("Meshes/", false, &sCurFile, nullptr);

  QString result = QFileDialog::getOpenFileName(this,
    QFileDialog::tr("Load Mesh"), sCurFile.c_str(),
    QFileDialog::tr("OBJ Files (*.obj);;All Files (*.*)"), 0);

  if (result.size() == 0)
    return;

  s_LastPath = result.toLatin1().data();

  ImportPhysicsObject(s_LastPath.c_str(), 1.0f);

  g_Globals.s_bUpdatePickingBuffer = true;
  AE_BROADCAST_EVENT(aeEditor_QueueRedraw);
}

void qtTreeEditWidget::on_button_DeleteCollisionObject_clicked()
{
  aePhysicsObject::DeleteSelected();
}

void qtTreeEditWidget::on_list_CollisionObjects_currentItemChanged(QListWidgetItem* current, QListWidgetItem* previous)
{
  if (current)
  {
    QVariant var = current->data(Qt::UserRole);
    aePhysicsObject* pObj = (aePhysicsObject*)var.value<void*>();

    aePhysicsObject::SetSelected(pObj);
  }

  g_Globals.s_bUpdatePickingBuffer = true;
  AE_BROADCAST_EVENT(aeEditor_QueueRedraw);
}

void qtTreeEditWidget::on_check_ShowCollisionObjects_clicked()
{
  g_Globals.s_bShowCollisionObjects = check_ShowCollisionObjects->isChecked();

  if (aePhysicsObject::GetSelected())
  {
    aePhysicsObject::GetSelected()->m_Gizmo.SetActive(g_Globals.s_bShowCollisionObjects);
  }

  AE_BROADCAST_EVENT(aeEditor_QueueRedraw);
}
