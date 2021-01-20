#ifndef PHYSICSOBJECT_H
#define PHYSICSOBJECT_H

#include "../Basics/Base.h"
#include "../Other/OBJ/OBJLoader.h"
#include <KrautEditorBasics/Gizmos/TransformGizmo.h>
#include <KrautGraphics/glim/glim.h>

class QListWidgetItem;

using namespace NS_GLIM;
using namespace AE_NS_EDITORBASICS;

struct aePhysicsObject
{
  aePhysicsObject();
  ~aePhysicsObject();

  static void ClearAll(void);
  static void SetSelected(aePhysicsObject* pSelected);
  static void DeleteSelected(void);

  void CreatePhysicsMesh(void);

  NS_OBJLOADER::MESH m_OriginalMesh;
  aeTransformGizmo m_Gizmo;
  GLIM_BATCH m_RenderBatch;
  aeUInt32 m_uiPickID;
  QListWidgetItem* m_pListItem;

#ifdef USE_BULLET
  btRigidBody* m_pRigidBody;
#endif

  bool Update(void);
  void SetScaling(const aeVec3& vScaling);
  aeVec3 GetScaling(void) const { return m_vScaling; }

  static aePhysicsObject* GetSelected(void) { return s_pSelected; }

private:
  aeVec3 m_vScaling;
  static aePhysicsObject* s_pSelected;
};

extern aeDeque<aePhysicsObject*> g_PhysicsObjects;

void UpdateAllPhysicsObjects(void);


#endif
