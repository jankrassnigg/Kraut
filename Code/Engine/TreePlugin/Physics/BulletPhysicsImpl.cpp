#include "PCH.h"

#include <TreePlugin/Physics/BulletPhysicsImpl.h>

bool g_bCollisionDataChanged = true;

BulletPhysicsImpl::BulletPhysicsImpl()
{
#ifdef USE_BULLET
  m_collisionConfiguration = new btDefaultCollisionConfiguration();
  m_dispatcher = new btCollisionDispatcher(m_collisionConfiguration);
  m_broadphase = new btDbvtBroadphase();
  m_solver = new btSequentialImpulseConstraintSolver;

  m_dynamicsWorld = new btDiscreteDynamicsWorld(m_dispatcher, m_broadphase, m_solver, m_collisionConfiguration);

  m_dynamicsWorld->setGravity(btVector3(0, -10, 0));
#endif
}

BulletPhysicsImpl::~BulletPhysicsImpl()
{
#ifdef USE_BULLET
  delete m_dynamicsWorld;
  delete m_solver;
  delete m_broadphase;
  delete m_dispatcher;
  delete m_collisionConfiguration;
#endif
}

void BulletPhysicsImpl::Reset()
{
#ifdef USE_BULLET
  if (m_CollisionCapsules.empty())
    return;

  g_bCollisionDataChanged = true;

  for (aeUInt32 ui = 0; ui < g_RigidBodies.size(); ++ui)
  {
    m_dynamicsWorld->removeRigidBody(g_RigidBodies[ui]);
  }

  g_RigidBodies.clear();
#endif
}

void BulletPhysicsImpl::AddColliderCapsule(const aeVec3& vPos, const aeVec3& vPosTo, float fRadius)
{
#ifdef USE_BULLET
  g_bCollisionDataChanged = true;

  aeVec3 vCenter = vPos + (vPosTo - vPos) / 2.0f;

  btCollisionShape* pShape = new btCapsuleShape(fRadius, (vPosTo - vPos).GetLength());

  btMatrix3x3 m;
  m.setIdentity();

  aeVec3 vDir = (vPosTo - vPos).GetNormalized();

  aeMatrix m2;

  if (aeMath::Abs(vDir.Dot(aeVec3(0, 0, 1))) < 0.8f)
    m2.SetObjectOrientationMatrixY(vCenter, vDir);
  else
    m2.SetObjectOrientationMatrixY(vCenter, vDir, aeVec3(1, 0, 0));

  m.setValue(m2.m_fColumn[0][0], m2.m_fColumn[0][1], m2.m_fColumn[0][2],
    m2.m_fColumn[1][0], m2.m_fColumn[1][1], m2.m_fColumn[1][2],
    m2.m_fColumn[2][0], m2.m_fColumn[2][1], m2.m_fColumn[2][2]);
  m = m.transpose();

  btTransform Transform;
  Transform.setIdentity();
  Transform.setBasis(m);
  Transform.setOrigin(btVector3(vCenter.x, vCenter.y, vCenter.z));

  btVector3 localInertia(0, 0, 0);

  myMotionState = new btDefaultMotionState(Transform);

  btRigidBody::btRigidBodyConstructionInfo rbInfo(0.0f, myMotionState, pShape, localInertia);
  btRigidBody* body = new btRigidBody(rbInfo);

  m_dynamicsWorld->addRigidBody(body, btBroadphaseProxy::DefaultFilter, btBroadphaseProxy::AllFilter);

  g_RigidBodies.push_back(body);
#endif
}

bool BulletPhysicsImpl::IsLineObstructed(const aeVec3& vPos, const aeVec3& vPosTo, float& out_fLineFraction) const
{
#ifdef USE_BULLET
  btCollisionWorld::ClosestRayResultCallback res(btVector3(vPos.x, vPos.y, vPos.z), btVector3(vPosTo.x, vPosTo.y, vPosTo.z));
  res.m_collisionFilterGroup = btBroadphaseProxy::DefaultFilter;
  res.m_collisionFilterMask = btBroadphaseProxy::AllFilter;

  m_dynamicsWorld->rayTest(btVector3(vPos.x, vPos.y, vPos.z), btVector3(vPosTo.x, vPosTo.y, vPosTo.z), res);

  out_fLineFraction = res.m_closestHitFraction;

  return res.hasHit();
#else
  return false;
#endif
}
