#pragma once

#include <KrautGenerator/Description/Physics.h>

#ifdef USE_BULLET
#  include <Bullet/btBulletDynamicsCommon.h>
#endif

using namespace AE_NS_FOUNDATION;

struct BulletPhysicsImpl : public Kraut::Physics_BaseImpl
{
  BulletPhysicsImpl();
  ~BulletPhysicsImpl();

  virtual void Reset() override;

  virtual void AddColliderCapsule(const aeVec3& vPos, const aeVec3& vPosTo, float fRadius) override;

  virtual bool IsLineObstructed(const aeVec3& vPos, const aeVec3& vPosTo, float& out_fLineFraction) const override;

  bool m_bCollisionDataChanged = true;

#ifdef USE_BULLET
  btDiscreteDynamicsWorld* m_dynamicsWorld = nullptr;

  btDefaultCollisionConfiguration* m_collisionConfiguration = nullptr;
  btCollisionDispatcher* m_dispatcher = nullptr;
  btDbvtBroadphase* m_broadphase = nullptr;
  btSequentialImpulseConstraintSolver* m_solver = nullptr;
  btDefaultMotionState* myMotionState = nullptr;

  aeDeque<btRigidBody*> m_CollisionCapsules = nullptr;
#endif
};
