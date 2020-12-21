#include "PCH.h"

#include "../Tree/Tree.h"

//#include <Bullet/btBulletDynamicsCommon.h>

#include <KrautGraphics/RenderAPI/Main.h>
#include <KrautGraphics/TextureManager/Main.h>
#include <KrautGraphics/ShaderManager/Main.h>
#include <KrautGraphics/Framebuffer/Main.h>
#include "../Physics/PhysicsObject.h"
using namespace AE_NS_GRAPHICS;

using namespace NS_GLIM;

#ifdef USE_BULLET
extern btDiscreteDynamicsWorld* m_dynamicsWorld;
extern aeDeque<btRigidBody*> g_RigidBodies;
extern bool g_bCollisionDataChanged;
#endif

static GLIM_BATCH glimCollMesh;

#ifdef USE_BULLET
class aePhysicsRenderer : public btIDebugDraw
{
public:
  virtual void	drawLine(const btVector3& from,const btVector3& to,const btVector3& color)
  {
    if (color.x () == 1.0f && color.y () == 1.0f && color.z () == 0.0f)
    {
      //return;
    }

    glimCollMesh.Begin (GLIM_LINES);
    glimCollMesh.Attribute4f ("attr_Color", color.x (), color.y (), color.z (), 1.0f);

    glimCollMesh.Vertex (from.x (), from.y (), from.z ());
    glimCollMesh.Vertex (to.x (), to.y (), to.z ());

    glimCollMesh.End ();
  }

  virtual void drawTransform(const btTransform& transform, btScalar orthoLen)
  {
  }
    

  virtual void	drawContactPoint(const btVector3& PointOnB,const btVector3& normalOnB,btScalar distance,int lifeTime,const btVector3& color)
  {
  }

  virtual void	setDebugMode(int debugMode)
  {
  }

  virtual void	reportErrorWarning(const char* warningString)
  {
  }

  virtual void	draw3dText(const btVector3& location,const char* textString) {}

  virtual int		getDebugMode() const { return 0; }

};

static aePhysicsRenderer g_Drawer;
#endif


void aeTree::RenderCollisionMesh (void)
{
  if (!g_Globals.s_bRenderCollisionMesh)
    return; 

  
  aeShaderManager::setShader (g_Globals.s_hSkeletonShader);
  aeRenderAPI::setFaceCulling (AE_CULL_FACE_NONE);

#ifdef USE_BULLET
  if (g_bCollisionDataChanged)
  {
    g_bCollisionDataChanged = false;

    glimCollMesh.BeginBatch ();
    glimCollMesh.Attribute4f ("attr_Color", 1, 1, 1, 1);
    
    m_dynamicsWorld->setDebugDrawer (&g_Drawer);
    m_dynamicsWorld->debugDrawWorld ();

    for (aeUInt32 i = 0; i < g_RigidBodies.size (); ++i)
    {
      m_dynamicsWorld->debugDrawObject (g_RigidBodies[i]->getWorldTransform (), g_RigidBodies[i]->getCollisionShape (), btVector3 (0, 1, 0));
    }

    //for (aeUInt32 i = 0; i < g_PhysicsObjects.size (); ++i)
    //{
    //  if (g_PhysicsObjects[i]->m_pRigidBody)
    //    m_dynamicsWorld->debugDrawObject (g_PhysicsObjects[i]->m_pRigidBody->getWorldTransform (), g_PhysicsObjects[i]->m_pRigidBody->getCollisionShape (), btVector3 (0, 0, 1));
    //}

    glimCollMesh.EndBatch ();
  }
#endif

  glimCollMesh.RenderBatch ();
}

void aeTree::RenderForces (bool bForPicking)
{
  for (aeUInt32 i = 0; i < m_Descriptor.m_Forces.size (); ++i)
  {
    m_Descriptor.m_Forces[i]->Render (bForPicking);
  }
}

void aeTree::RenderGroundPlane (void)
{
  aeTextureResourceHandle hTexture = aeTextureResource::LoadResource ("Textures/GroundPlane.dds");
  aeShaderManager::BindTexture ("tex_Diffuse", hTexture);

  aeShaderManager::setShader (g_Globals.s_hGroundPlaneShader);
  aeRenderAPI::setFaceCulling (AE_CULL_FACE_BACK);

  aeShaderManager::setShaderBuilderVariable ("GROUNDPLANE_DISCARD", false);

  static GLIM_BATCH glimGround;

  if (glimGround.IsCleared ())
  {
    const float fSize = 1000.0f;
    const float fTexCoord = (fSize / 100.0f) * 50.0f;

    glimGround.BeginBatch ();
    glimGround.Attribute4f ("attr_Color", 0, 1, 0, 1);
    glimGround.Attribute2f ("attr_TexCoords", 0, 0);

    glimGround.Begin (GLIM_QUADS);

      glimGround.Attribute2f ("attr_TexCoords", -fTexCoord, -fTexCoord);
      glimGround.Vertex (-fSize, 0, fSize);
      glimGround.Attribute2f ("attr_TexCoords", fTexCoord, -fTexCoord);
      glimGround.Vertex ( fSize, 0, fSize);
      glimGround.Attribute2f ("attr_TexCoords", fTexCoord, fTexCoord);
      glimGround.Vertex ( fSize, 0,-fSize);
      glimGround.Attribute2f ("attr_TexCoords", -fTexCoord, fTexCoord);
      glimGround.Vertex (-fSize, 0,-fSize);

    glimGround.End ();

    glimGround.EndBatch ();
  }

  glimGround.RenderBatch ();
}



