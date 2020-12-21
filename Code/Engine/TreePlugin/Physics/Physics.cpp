#include "PCH.h"

#include <KrautGraphics/glim/glim.h>

using namespace NS_GLIM;

#ifdef USE_BULLET
#include <Bullet/btBulletDynamicsCommon.h>

btDiscreteDynamicsWorld* m_dynamicsWorld;

btDefaultCollisionConfiguration* m_collisionConfiguration;
btCollisionDispatcher* m_dispatcher;
btDbvtBroadphase* m_broadphase;
btSequentialImpulseConstraintSolver* m_solver;
btDefaultMotionState* myMotionState;

aeDeque<btRigidBody*> g_RigidBodies;
#endif

bool g_bCollisionDataChanged = true;

void AddCapsule (const aeVec3& vPos, const aeVec3& vPosTo, float fRadius);


AE_ON_GLOBAL_EVENT_ONCE (aeTreePlugin_Init)
{
#ifdef USE_BULLET
  m_collisionConfiguration = new btDefaultCollisionConfiguration();
  m_dispatcher = new	btCollisionDispatcher(m_collisionConfiguration);
  m_broadphase = new btDbvtBroadphase();
  m_solver = new btSequentialImpulseConstraintSolver;

  m_dynamicsWorld = new btDiscreteDynamicsWorld(m_dispatcher,m_broadphase,m_solver,m_collisionConfiguration);
	
  m_dynamicsWorld->setGravity(btVector3(0,-10,0));
#endif
}

void ClearCapsules ()
{
#ifdef USE_BULLET
  if (g_RigidBodies.empty ())
    return;

  g_bCollisionDataChanged = true;

  for (aeUInt32 ui = 0; ui < g_RigidBodies.size (); ++ui)
  {
    m_dynamicsWorld->removeRigidBody (g_RigidBodies[ui]);
  }

  g_RigidBodies.clear ();
#endif
}

void AddCapsule (const aeVec3& vPos, const aeVec3& vPosTo, float fRadius)
{
#ifdef USE_BULLET
  g_bCollisionDataChanged = true;

  aeVec3 vCenter = vPos + (vPosTo - vPos) / 2.0f;

  btCollisionShape* pShape = new btCapsuleShape (fRadius, (vPosTo - vPos).GetLength ());
  
  btMatrix3x3 m;
  m.setIdentity ();

  aeVec3 vDir = (vPosTo - vPos).GetNormalized ();
  
  aeMatrix m2;

  if (aeMath::Abs (vDir.Dot (aeVec3 (0, 0, 1))) < 0.8f)
    m2.SetObjectOrientationMatrixY (vCenter, vDir);
  else
    m2.SetObjectOrientationMatrixY (vCenter, vDir, aeVec3 (1, 0, 0));

  m.setValue (m2.m_fColumn[0][0], m2.m_fColumn[0][1], m2.m_fColumn[0][2],
              m2.m_fColumn[1][0], m2.m_fColumn[1][1], m2.m_fColumn[1][2],
              m2.m_fColumn[2][0], m2.m_fColumn[2][1], m2.m_fColumn[2][2]);
  m = m.transpose ();
  
  btTransform Transform;
  Transform.setIdentity();
  Transform.setBasis (m);
  Transform.setOrigin (btVector3 (vCenter.x, vCenter.y, vCenter.z));

  btVector3 localInertia(0,0,0);

  myMotionState = new btDefaultMotionState(Transform);

  btRigidBody::btRigidBodyConstructionInfo rbInfo (0.0f, myMotionState, pShape, localInertia);
  btRigidBody* body = new btRigidBody (rbInfo);

  m_dynamicsWorld->addRigidBody(body, btBroadphaseProxy::DefaultFilter, btBroadphaseProxy::AllFilter);

  

  g_RigidBodies.push_back (body);
#endif
}

aeVec3 FindLeastObstructedDirection (const aeVec3& vPos, const aeVec3& vPosTo, aeUInt32 uiMaxDeviation, float& out_fDistance)
{
  const float fRayLength = (vPosTo - vPos).GetLength ();
  const aeVec3 vRayDir = (vPosTo - vPos).GetNormalized ();

  float fMaxLength = 0.0f;
  aeVec3 vBestDir = vRayDir;

  const aeVec3 vAngleOrtho = vRayDir.GetOrthogonalVector ().GetNormalized ();

  for (aeUInt32 angle = 1; angle < uiMaxDeviation; angle += 5)
  {
    aeMatrix mAngle;
    mAngle.SetRotationMatrix (vAngleOrtho, (float) angle);

    for (aeUInt32 round = 0; round < 360; round += 20)
    {
      aeMatrix mRound;
      mRound.SetRotationMatrix (vRayDir, (float) round);

      const aeVec3 vNewDir = mRound * (mAngle * vRayDir);

      const aeVec3 vTarget = vPos + vNewDir * fRayLength;

#ifdef USE_BULLET
      btCollisionWorld::ClosestRayResultCallback res (btVector3 (vPos.x, vPos.y, vPos.z), btVector3 (vTarget.x, vTarget.y, vTarget.z));
      res.m_collisionFilterGroup = btBroadphaseProxy::DefaultFilter;
      res.m_collisionFilterMask = btBroadphaseProxy::AllFilter;

      m_dynamicsWorld->rayTest (btVector3 (vPos.x, vPos.y, vPos.z), btVector3 (vTarget.x, vTarget.y, vTarget.z), res);

      if (!res.hasHit ())
#endif
        return vNewDir;

#ifdef USE_BULLET
      if (res.m_closestHitFraction > fMaxLength)
      {
        fMaxLength = res.m_closestHitFraction;
        vBestDir = vNewDir;

        out_fDistance = (aeVec3 (res.m_hitPointWorld.x (), res.m_hitPointWorld.y (), res.m_hitPointWorld.z ()) - vPos).GetLength ();
      }
#endif
    }
  }

  return vBestDir;
}

bool IsObstructed (const aeVec3& vPos, const aeVec3& vPosTo, float& out_fDistance)
{
#ifdef USE_BULLET
  btCollisionWorld::ClosestRayResultCallback res (btVector3 (vPos.x, vPos.y, vPos.z), btVector3 (vPosTo.x, vPosTo.y, vPosTo.z));
  res.m_collisionFilterGroup = btBroadphaseProxy::DefaultFilter;
  res.m_collisionFilterMask = btBroadphaseProxy::AllFilter;

  m_dynamicsWorld->rayTest (btVector3 (vPos.x, vPos.y, vPos.z), btVector3 (vPosTo.x, vPosTo.y, vPosTo.z), res);

  out_fDistance = (aeVec3 (res.m_hitPointWorld.x (), res.m_hitPointWorld.y (), res.m_hitPointWorld.z ()) - vPos).GetLength ();
  
  return res.hasHit ();
#else
  return false;
#endif
}


