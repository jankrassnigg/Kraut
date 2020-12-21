#include "PCH.h"

#include <KrautEditorBasics/Gizmos/TransformGizmo.h>
#include <KrautGraphics/glim/glim.h>
#include <KrautFoundation/FileSystem/FileIn.h>
#include <KrautGraphics/RenderAPI/RenderAPI.h>
#include <KrautGraphics/ShaderManager/ShaderManager.h>
#include "../Other/OBJ/OBJLoader.h"
#include "../Tree/Tree.h"
#include <KrautEditorBasics/Plugin.h>
#include "PhysicsObject.h"
#include <KrautFoundation/Configuration/VariableRegistry.h>
#include "../Basics/Plugin.h"
#include "../GUI/qtTreeEditWidget/qtTreeEditWidget.moc.h"
#include "../GUI/ProgressBar.h"


using namespace NS_GLIM;
using namespace NS_OBJLOADER;
using namespace AE_NS_EDITORBASICS;

extern bool g_bCollisionDataChanged;

#ifdef USE_BULLET
extern btDiscreteDynamicsWorld* m_dynamicsWorld;

btRigidBody* AddCollisionMesh (const MESH& obj, const aeVec3& vScale);
#endif

aeDeque<aePhysicsObject*> g_PhysicsObjects;
aePhysicsObject* aePhysicsObject::s_pSelected = NULL;

static void OnPhysicsObjectEvent (void* pPassThrough, const void* pEventData)
{
  aePhysicsObject* pObj = (aePhysicsObject*) pPassThrough;
  aeTransformGizmo::EventData* pEvent = (aeTransformGizmo::EventData*) pEventData;

  if (pEvent->m_EventType == aeTransformGizmo::DragEnd)
  {
    AE_BROADCAST_EVENT (aeTreeEdit_TreeInfluencesChanged);
  }
}

AE_ON_GLOBAL_EVENT (aeTreeEdit_DeleteSelectedBranch)
{
  aePhysicsObject::DeleteSelected ();
}

AE_ON_GLOBAL_EVENT (aeTreePlugin_TreeLoaded)
{
  if (g_PhysicsObjects.empty ())
    return;

  QMainWindow* pMainWindow = NULL;
  aeVariableRegistry::RetrieveRaw ("system/qt/mainwidget", &pMainWindow, sizeof (QMainWindow*));

  if (QMessageBox::question (pMainWindow, "Kraut",  "Do you want to delete all Collision Objects?", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
    aePhysicsObject::ClearAll ();
}

aePhysicsObject::aePhysicsObject ()
{
#ifdef USE_BULLET
  m_pRigidBody = NULL;
#endif

  m_Gizmo.m_Events.RegisterEventReceiver (OnPhysicsObjectEvent, this);
  m_Gizmo.SetActive (false);
  m_vScaling.SetVector (1.0f);
  m_pListItem = NULL;
}

aePhysicsObject::~aePhysicsObject ()
{
  if (s_pSelected == this)
    SetSelected (NULL);

#ifdef USE_BULLET
  if (m_pRigidBody)
  {
    m_dynamicsWorld->removeRigidBody (m_pRigidBody);
    delete m_pRigidBody;

    AE_BROADCAST_EVENT (aeTreeEdit_TreeInfluencesChanged);
  }
#endif

  if (m_pListItem)
    delete m_pListItem;
}

void aePhysicsObject::ClearAll (void)
{
  SetSelected (NULL);

  if (!g_PhysicsObjects.empty ())
  {
    for (aeUInt32 i = 0; i < g_PhysicsObjects.size (); ++i)
      delete g_PhysicsObjects[i];

    g_PhysicsObjects.clear ();
  }
}

void aePhysicsObject::DeleteSelected (void)
{
  if (aePhysicsObject::GetSelected () == NULL)
    return;

  QMainWindow* pMainWindow = NULL;
  aeVariableRegistry::RetrieveRaw ("system/qt/mainwidget", &pMainWindow, sizeof (QMainWindow*));

  if (QMessageBox::question (pMainWindow, "Kraut",  "Do you want to delete the selected Collision Object?", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) != QMessageBox::Yes)
    return;

  for (aeUInt32 i = 0; i < g_PhysicsObjects.size (); ++i)
  {
    if (g_PhysicsObjects[i] == s_pSelected)
    {
      g_PhysicsObjects[i] = g_PhysicsObjects.back ();
      g_PhysicsObjects.pop_back ();
      delete s_pSelected;
      //s_pSelected = NULL; // this is already done in the destructor of aePhysicsObjects and if there are additional objects
      // in the list, another one is selected via qt, so setting it to NULL here would make the pointer incorrect

      if (g_PhysicsObjects.empty ())
        s_pSelected = NULL;

      break;
    }
  }
}

void aePhysicsObject::SetSelected (aePhysicsObject* pSelected)
{
  if (s_pSelected == pSelected)
    return;

  if (s_pSelected != NULL)
  {
    s_pSelected->m_Gizmo.SetActive (false);
    s_pSelected->m_pListItem->setSelected (false);
  }

  s_pSelected = pSelected;

  if (s_pSelected != NULL)
  {
    s_pSelected->m_Gizmo.SetActive (g_Globals.s_bShowCollisionObjects);

    aeVec3 vScale = s_pSelected->GetScaling ();

    qtTreeEditWidget::s_pWidget->spin_ColObjScalingX->setValue (vScale.x);
    qtTreeEditWidget::s_pWidget->spin_ColObjScalingY->setValue (vScale.y);
    qtTreeEditWidget::s_pWidget->spin_ColObjScalingZ->setValue (vScale.z);

    s_pSelected->m_pListItem->setSelected (true);
  }

  {
    qtTreeEditWidget::s_pWidget->button_DeleteCollisionObject->setEnabled (s_pSelected != NULL);
    qtTreeEditWidget::s_pWidget->spin_ColObjScalingX->setEnabled (s_pSelected != NULL);
    qtTreeEditWidget::s_pWidget->spin_ColObjScalingY->setEnabled (s_pSelected != NULL);
    qtTreeEditWidget::s_pWidget->spin_ColObjScalingZ->setEnabled (s_pSelected != NULL);
  }

  AE_BROADCAST_EVENT (aeEditor_QueueRedraw);
}

bool aePhysicsObject::Update (void)
{
#ifdef USE_BULLET
  if (!m_pRigidBody)
    return false;

  const aeMatrix t = m_Gizmo.GetTransform ();

  btMatrix3x3 m;
  m.setValue (t.m_fColumn[0][0], t.m_fColumn[1][0], t.m_fColumn[2][0],
              t.m_fColumn[0][1], t.m_fColumn[1][1], t.m_fColumn[2][1],
              t.m_fColumn[0][2], t.m_fColumn[1][2], t.m_fColumn[2][2]);

  btTransform Transform;
  Transform.setIdentity();
  Transform.setOrigin (btVector3 (t.m_fColumn[3][0], t.m_fColumn[3][1], t.m_fColumn[3][2]));
  Transform.setBasis (m);

  btTransform OldTransform = m_pRigidBody->getWorldTransform ();

  if (!aeMemory::Compare (&OldTransform, &Transform, sizeof (btTransform)))
  {
    g_bCollisionDataChanged = true;
    m_pRigidBody->setWorldTransform (Transform);
    return true;
  }
#endif

  return false;
}

void RenderPhysicsObjects (void)
{
  if (g_PhysicsObjects.empty ())
    return;

  aeShaderManager::setShader (g_Globals.s_hPhysicsObjectShader);

  for (aeUInt32 o = 0; o < g_PhysicsObjects.size (); ++o)
  {
    aeRenderAPI::GetMatrix_ObjectToWorld ().PushMatrix ();
    aeRenderAPI::GetMatrix_ObjectToWorld ().LoadMatrix (g_PhysicsObjects[o]->m_Gizmo.GetTransform ());

    aeMatrix mScale;
    mScale.SetScalingMatrix (g_PhysicsObjects[o]->GetScaling ().x, g_PhysicsObjects[o]->GetScaling ().y, g_PhysicsObjects[o]->GetScaling ().z);

    aeRenderAPI::GetMatrix_ObjectToWorld ().MultMatrix (mScale);

    aeShaderManager::setShaderBuilderVariable ("SELECTED", g_PhysicsObjects[o] == aePhysicsObject::GetSelected ());

    g_PhysicsObjects[o]->m_RenderBatch.RenderBatch ();

    aeRenderAPI::GetMatrix_ObjectToWorld ().PopMatrix ();
  }
}

bool ImportPhysicsObject (const char* szFile, float fScale)
{
  aeProgressBar pb ("Importing Mesh", 4);

  MESH obj;
  std::map<std::string, MATERIAL> Materials;
  if (!LoadOBJFile (szFile, obj, Materials, "", false, true))
    return false;

  aeProgressBar::Update (); // 1

  g_PhysicsObjects.push_back (new aePhysicsObject);
  aePhysicsObject* pObj = g_PhysicsObjects.back ();

  pObj->m_uiPickID = aeEditorPlugin::RegisterPickableObject (pObj, g_PhysicsObjects.size () - 1, "aePhysicsObject");
  pObj->m_RenderBatch.BeginBatch ();
  pObj->m_RenderBatch.Attribute4ub ("attr_Color", 128, 128, 128, 255);
  pObj->m_RenderBatch.Attribute4ub ("attr_PickingID", (pObj->m_uiPickID >> 0) & 0xFF, (pObj->m_uiPickID >> 8) & 0xFF, (pObj->m_uiPickID >> 16) & 0xFF, (pObj->m_uiPickID >> 24) & 0xFF);

  for (aeUInt32 f = 0; f < obj.m_Faces.size (); ++f)
  {
    pObj->m_RenderBatch.Begin (GLIM_POLYGON);

    for (aeUInt32 v = 0; v < obj.m_Faces[f].m_Vertices.size (); ++v)
    {
      const VEC3 pos = fScale * obj.m_Positions[obj.m_Faces[f].m_Vertices[v].m_uiPositionID];
      pObj->m_RenderBatch.Vertex (pos.x, pos.y, pos.z);
    }

    pObj->m_RenderBatch.End ();
  }

  pObj->m_RenderBatch.EndBatch ();

  aeProgressBar::Update (); // 2

  pObj->m_OriginalMesh = obj;
  pObj->CreatePhysicsMesh ();

  aeProgressBar::Update (); // 3

  pObj->m_pListItem = new QListWidgetItem ();
  pObj->m_pListItem->setText (aePathFunctions::GetFileName (szFile).c_str ());
  pObj->m_pListItem->setToolTip (szFile);
  pObj->m_pListItem->setData (Qt::UserRole, qVariantFromValue ((void*) pObj));

  qtTreeEditWidget::s_pWidget->list_CollisionObjects->addItem (pObj->m_pListItem);

  aePhysicsObject::SetSelected (pObj);

  aeProgressBar::Update (); // 4

  return true;
}

void aePhysicsObject::CreatePhysicsMesh (void)
{
#ifdef USE_BULLET
  aeProgressBar pb ("Creating Physics Mesh", 3);

  if (m_pRigidBody)
  {
    m_dynamicsWorld->removeRigidBody (m_pRigidBody);
    delete m_pRigidBody;
  }

  aeProgressBar::Update (); // 1

  m_pRigidBody = AddCollisionMesh (m_OriginalMesh, m_vScaling);

  aeProgressBar::Update (); // 2

  Update ();

  aeProgressBar::Update (); // 3

  m_dynamicsWorld->stepSimulation (1.0f / 60.0f);

  aeProgressBar::Update (); // 4

  AE_BROADCAST_EVENT (aeTreeEdit_TreeInfluencesChanged);
#endif
}

void UpdateAllPhysicsObjects (void)
{
#ifdef USE_BULLET
  bool b = false;

  for (aeUInt32 i = 0; i < g_PhysicsObjects.size (); ++i)
    b |= g_PhysicsObjects[i]->Update ();

  if (b)
    m_dynamicsWorld->stepSimulation (1.0f / 60.0f);
#endif
}

void aePhysicsObject::SetScaling (const aeVec3& vScaling)
{
  m_vScaling = vScaling;

  CreatePhysicsMesh ();
}

