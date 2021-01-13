#include "PCH.h"

#include "../GUI/ProgressBar.h"
#include "../Other/OBJ/OBJLoader.h"
#include "../Tree/Tree.h"

using namespace NS_OBJLOADER;

#ifdef USE_BULLET
btRigidBody* AddCollisionMesh(const MESH& obj, const aeVec3& vScale)
{
  aeProgressBar pb("Adding Collision Mesh", 6);

  g_bCollisionDataChanged = true;

  btTriangleMesh* m = new btTriangleMesh();

  aeUInt32 uiTriangles = 0;
  aeUInt32 uiVertices = 0;

  for (aeUInt32 f = 0; f < obj.m_Faces.size(); ++f)
  {
    uiTriangles += obj.m_Faces[f].m_Vertices.size() - 2;
    uiVertices += 3;
  }

  m->preallocateIndices(uiTriangles * 3);
  m->preallocateVertices(uiVertices);

  aeProgressBar::Update(); // 1

  for (aeUInt32 f = 0; f < obj.m_Faces.size(); ++f)
  {
    for (aeUInt32 v = 0; v < obj.m_Faces[f].m_Vertices.size() - 2; ++v)
    {
      VEC3 vert[3];
      vert[0] = obj.m_Positions[obj.m_Faces[f].m_Vertices[0].m_uiPositionID];
      vert[1] = obj.m_Positions[obj.m_Faces[f].m_Vertices[v + 1].m_uiPositionID];
      vert[2] = obj.m_Positions[obj.m_Faces[f].m_Vertices[v + 2].m_uiPositionID];

      for (int i = 0; i < 3; ++i)
      {
        vert[i].x *= vScale.x;
        vert[i].y *= vScale.y;
        vert[i].z *= vScale.z;
      }

      m->addTriangle(btVector3(vert[0].x, vert[0].y, vert[0].z),
        btVector3(vert[1].x, vert[1].y, vert[1].z),
        btVector3(vert[2].x, vert[2].y, vert[2].z));
    }
  }

  aeProgressBar::Update(); // 2

  btCollisionShape* pShape = new btBvhTriangleMeshShape(m, true);

  aeProgressBar::Update(); // 3

  btTransform Transform;
  Transform.setIdentity();

  btDefaultMotionState* pMotionState = new btDefaultMotionState(Transform);

  btVector3 localInertia(0, 0, 0);
  btRigidBody::btRigidBodyConstructionInfo rbInfo(0.0f, pMotionState, pShape, localInertia);
  btRigidBody* pRigidBody = new btRigidBody(rbInfo);

  aeProgressBar::Update(); // 4

  m_dynamicsWorld->addRigidBody(pRigidBody, btBroadphaseProxy::DefaultFilter, btBroadphaseProxy::AllFilter);

  aeProgressBar::Update(); // 5

  m_dynamicsWorld->stepSimulation(1.0f / 60.0f);

  aeProgressBar::Update(); // 6

  return pRigidBody;
}
#endif
