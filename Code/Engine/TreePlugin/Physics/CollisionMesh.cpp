#include "PCH.h"

#include "../Tree/Tree.h"
#include "../Other/OBJ/OBJLoader.h"
#include "../GUI/ProgressBar.h"
//#include <Bullet/btBulletDynamicsCommon.h>

using namespace NS_OBJLOADER;

bool IsObstructed (const aeVec3& vPos, const aeVec3& vPosTo, float& out_fDistance);
void ClearCapsules ();
void AddCapsule (const aeVec3& vPos, const aeVec3& vPosTo, float fRadius);

#ifdef USE_BULLET
extern btDiscreteDynamicsWorld* m_dynamicsWorld;
extern bool g_bCollisionDataChanged;
#endif

void aeTree::CreateBranchCollisionCapsules (aeUInt32 uiBranch)
{
  if (!g_Globals.s_bDoPhysicsSimulation)
    return;

  aeBranch& Branch = m_Branches[uiBranch];

  // if this type of branch should not act as an obstacle, do not insert collision shapes
  if (!m_Descriptor.m_BranchTypes[Branch.m_Type].m_bActAsObstacle)
    return;

  if (Branch.IsDeleted ())
    return;

  if (Branch.m_Nodes.size () < 4)
    return;

  const float fMaxDeviation = aeMath::CosDeg (7.0f);

  aeInt32 iAnchor0 = 0;
  aeInt32 iAnchor1 = 1;

  aeVec3 vDir = (Branch.m_Nodes[iAnchor1].m_vPosition - Branch.m_Nodes[iAnchor0].m_vPosition).GetNormalized ();
  float fMinRadius = aeMath::Min (Branch.m_Nodes[iAnchor0].m_fThickness, Branch.m_Nodes[iAnchor1].m_fThickness) / 2.0f;
  float fMaxRadius = aeMath::Max (Branch.m_Nodes[iAnchor0].m_fThickness, Branch.m_Nodes[iAnchor1].m_fThickness) / 2.0f;

  for (aeUInt32 n = 2; n < Branch.m_Nodes.size (); ++n)
  {
    aeVec3 vPos = Branch.m_Nodes[n].m_vPosition;
    aeVec3 vDirToPos = (vPos - Branch.m_Nodes[iAnchor0].m_vPosition).GetNormalized ();

    fMinRadius = aeMath::Min (fMinRadius, Branch.m_Nodes[n].m_fThickness / 2.0f);
    fMaxRadius = aeMath::Max (fMaxRadius, Branch.m_Nodes[n].m_fThickness / 2.0f);

    if ((vDirToPos.Dot (vDir) >= fMaxDeviation) && (fMaxRadius / fMinRadius < 2.0f))
    {
      iAnchor1 = n;
      continue;
    }

    AddCapsule (Branch.m_Nodes[iAnchor1].m_vPosition, Branch.m_Nodes[iAnchor0].m_vPosition, 1.5f * (fMinRadius + (fMaxRadius - fMinRadius) * 0.5f));

    iAnchor0 = n - 1;
    iAnchor1 = n;

    vDir = (Branch.m_Nodes[iAnchor1].m_vPosition - Branch.m_Nodes[iAnchor0].m_vPosition).GetNormalized ();
    fMinRadius = aeMath::Min (Branch.m_Nodes[iAnchor0].m_fThickness, Branch.m_Nodes[iAnchor1].m_fThickness) / 2.0f;
    fMaxRadius = aeMath::Max (Branch.m_Nodes[iAnchor0].m_fThickness, Branch.m_Nodes[iAnchor1].m_fThickness) / 2.0f;
  }

  AddCapsule (Branch.m_Nodes[iAnchor1].m_vPosition, Branch.m_Nodes[iAnchor0].m_vPosition, 1.5f * (fMinRadius + (fMaxRadius - fMinRadius) * 0.5f));
}

#ifdef USE_BULLET
btRigidBody* AddCollisionMesh (const MESH& obj, const aeVec3& vScale)
{
  aeProgressBar pb ("Adding Collision Mesh", 6);

  g_bCollisionDataChanged = true;

  btTriangleMesh* m = new btTriangleMesh ();

  aeUInt32 uiTriangles = 0;
  aeUInt32 uiVertices = 0;

  for (aeUInt32 f = 0; f < obj.m_Faces.size (); ++f)
  {
    uiTriangles += obj.m_Faces[f].m_Vertices.size () - 2;
    uiVertices += 3;
  }

  m->preallocateIndices (uiTriangles * 3);
  m->preallocateVertices (uiVertices);

  aeProgressBar::Update (); // 1
  
  for (aeUInt32 f = 0; f < obj.m_Faces.size (); ++f)
  {
    for (aeUInt32 v = 0; v < obj.m_Faces[f].m_Vertices.size () - 2; ++v)
    {
      VEC3 vert[3];
      vert[0] = obj.m_Positions[obj.m_Faces[f].m_Vertices[0].m_uiPositionID];
      vert[1] = obj.m_Positions[obj.m_Faces[f].m_Vertices[v+1].m_uiPositionID];
      vert[2] = obj.m_Positions[obj.m_Faces[f].m_Vertices[v+2].m_uiPositionID];

      for (int i = 0; i < 3; ++i)
      {
        vert[i].x *= vScale.x;
        vert[i].y *= vScale.y;
        vert[i].z *= vScale.z;
      }

      m->addTriangle (btVector3 (vert[0].x, vert[0].y, vert[0].z), 
                      btVector3 (vert[1].x, vert[1].y, vert[1].z), 
                      btVector3 (vert[2].x, vert[2].y, vert[2].z));
    }
  }

  aeProgressBar::Update (); // 2

  btCollisionShape* pShape = new btBvhTriangleMeshShape (m, true);

  aeProgressBar::Update (); // 3

  btTransform Transform;
  Transform.setIdentity();

  btDefaultMotionState* pMotionState = new btDefaultMotionState (Transform);

  btVector3 localInertia (0,0,0);
  btRigidBody::btRigidBodyConstructionInfo rbInfo (0.0f, pMotionState, pShape, localInertia);
  btRigidBody* pRigidBody = new btRigidBody (rbInfo);

  aeProgressBar::Update (); // 4

  m_dynamicsWorld->addRigidBody (pRigidBody, btBroadphaseProxy::DefaultFilter, btBroadphaseProxy::AllFilter);

  aeProgressBar::Update (); // 5

  m_dynamicsWorld->stepSimulation (1.0f / 60.0f);

  aeProgressBar::Update (); // 6

  return pRigidBody;
}
#endif
