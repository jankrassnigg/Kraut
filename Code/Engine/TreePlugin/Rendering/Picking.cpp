#include "PCH.h"

#include "../Tree/Tree.h"
#include "../Basics/Plugin.h"

#include "../GUI/qtTreeEditWidget/qtTreeEditWidget.moc.h"

#include <KrautFoundation/Configuration/VariableRegistry.h>

#include <KrautGraphics/RenderAPI/Main.h>
#include <KrautGraphics/TextureManager/Main.h>
#include <KrautGraphics/ShaderManager/Main.h>
#include <KrautGraphics/Framebuffer/Main.h>
using namespace AE_NS_GRAPHICS;

void EnsureColorRendering (void);

aeFramebuffer g_PickingFB;
aeTextureResourceHandle g_hPickingObjectIDRT;
aeTextureResourceHandle g_hPickingSubIDRT;
aeTextureResourceHandle g_hPickingDepthRT;

AE_ON_GLOBAL_EVENT_ONCE (aeStartup_ShutdownEngine_Begin)
{
  g_PickingFB.Shutdown ();
  g_hPickingObjectIDRT.Invalidate ();
  g_hPickingSubIDRT.Invalidate ();
  g_hPickingDepthRT.Invalidate ();
}

void RenderPhysicsObjects (void);

static void SetupFBs (aeUInt32 uiWidth, aeUInt32 uiHeight)
{
  static aeUInt32 uiResX = 0;
  static aeUInt32 uiResY = 0;

  if ((uiResX == uiWidth) && (uiResY == uiHeight))
    return;

  uiResX = uiWidth;
  uiResY = uiHeight;

  aeTextureResourceDescriptor trd;
  trd.m_bAllowToReadContents = true;
  trd.m_bGenerateMipmaps = false;
  trd.m_TextureFormat = AE_FORMAT_R8G8B8A8_UNORM;
  trd.m_TextureType = AE_TYPE_TEXTURE_2D;
  trd.m_uiWidth = uiWidth;
  trd.m_uiHeight = uiHeight;

  aeTextureResource::CreateResource (g_hPickingObjectIDRT, &trd);
  aeTextureResource::CreateResource (g_hPickingSubIDRT, &trd);

  trd.m_bAllowToReadContents = true;
  trd.m_TextureFormat = AE_FORMAT_DEPTH24_STENCIL8;

  aeTextureResource::CreateResource (g_hPickingDepthRT, &trd);

  g_PickingFB.setDepthAttachment (g_hPickingDepthRT);
  g_PickingFB.setColorAttachment (0, g_hPickingObjectIDRT);
  g_PickingFB.setColorAttachment (1, g_hPickingSubIDRT);
}

static aeArray<aeUInt8> ImagePickingIDs;
static aeArray<aeUInt8> ImagePickingSubIDs;
static aeArray<aeUInt8> ImagePickingDepth;

void aeTreePlugin::UpdatePickingBuffer (void)
{
  {
    aeMatrix mC = aeRenderAPI::GetMatrix_WorldToCamera ().GetMatrix ();
    aeMatrix mP = aeRenderAPI::GetMatrix_CameraToScreen ().GetMatrix ();

    aeUInt32 uiVP[4];
    float fN, fF;
    aeRenderAPI::getViewportAndDepthRange (uiVP[0], uiVP[1], uiVP[2], uiVP[3], fN, fF);

    static aeMatrix mLastC;
    static aeMatrix mLastP;
    static aeUInt32 uiLastVP[4];
    static float fLastN, fLastF;

    if ((!g_Globals.s_bUpdatePickingBuffer) && (mLastC == mC) && (mLastP == mP) && (uiLastVP[0] == uiVP[0]) && (uiLastVP[1] == uiVP[1]) && (uiLastVP[2] == uiVP[2]) && (uiLastVP[3] == uiVP[3]) &&
        (fLastN == fN) && (fLastF == fF))
        return;

    g_Globals.s_bUpdatePickingBuffer = false;
    mLastC = mC;
    mLastP = mP;
    uiLastVP[0] = uiVP[0];
    uiLastVP[1] = uiVP[1];
    uiLastVP[2] = uiVP[2];
    uiLastVP[3] = uiVP[3];
    fLastN = fN;
    fLastF = fF;
  }

  aeInt32 uiResolutionX = 0;
  aeInt32 uiResolutionY = 0;

  aeVariableRegistry::RetrieveInt ("system/graphics/resolution_x", uiResolutionX);
  aeVariableRegistry::RetrieveInt ("system/graphics/resolution_y", uiResolutionY);

  if (uiResolutionX == 0 || uiResolutionY == 0)
    return;

  SetupFBs (uiResolutionX, uiResolutionY);

  g_PickingFB.BindFramebuffer ();

  aeRenderAPI::Clear (true, true, 0, 0, 0, 0);
  aeRenderAPI::setViewport (0, 0, uiResolutionX, uiResolutionY);

  g_Globals.m_Camera.setPerspectiveCamera (80.0f, 0.01f, 1000.0f);
  g_Globals.m_Camera.setLookAt (g_Globals.s_vCameraPosition, g_Globals.s_vCameraPivot);
  g_Globals.m_Camera.setViewport (0, 0, uiResolutionX, uiResolutionY);

  g_Globals.m_Camera.ApplyCamera ();

  {
    EnsureColorRendering ();
    aeShaderManager::setShaderBuilderVariable ("PICKING", true);
    aeShaderManager::setShaderBuilderVariable ("CROSSFADE", false);

    g_Tree.RenderTree (g_Globals.s_CurLoD);
    g_Tree.RenderForces (true);

    g_Tree.RenderGroundPlane ();
  }

  if (g_Globals.s_bShowCollisionObjects)
    RenderPhysicsObjects ();

  aeTransformGizmo::RenderAllGizmos ();

  aeShaderManager::setShaderBuilderVariable ("PICKING", false);

  {
    ImagePickingDepth.resize (uiResolutionX * uiResolutionY * 4);
    g_hPickingDepthRT.GetResource ()->ReadbackTexture (&ImagePickingDepth[0]);
  }

  {
    ImagePickingIDs.resize (uiResolutionX * uiResolutionY * 4);
    g_hPickingObjectIDRT.GetResource ()->ReadbackTexture (&ImagePickingIDs[0]);
  }

  {
    ImagePickingSubIDs.resize (uiResolutionX * uiResolutionY * 4);
    g_hPickingSubIDRT.GetResource ()->ReadbackTexture (&ImagePickingSubIDs[0]);
  }
}



aeUInt32 aeTreePlugin::PickAt (aeUInt32 x, aeUInt32 y, aeVec3* out_pPosition, aeUInt32& out_uiSubID)
{
  out_uiSubID = 0;
  UpdatePickingBuffer ();

  aeInt32 uiResolutionX = 0;
  aeInt32 uiResolutionY = 0;

  aeVariableRegistry::RetrieveInt ("system/graphics/resolution_x", uiResolutionX);
  aeVariableRegistry::RetrieveInt ("system/graphics/resolution_y", uiResolutionY);

  if (((aeInt32) x >= uiResolutionX) || ((aeInt32) y >= uiResolutionY))
    return 0;

  if (out_pPosition)
  {
    float fDepth = *((float*) &ImagePickingDepth[(uiResolutionY - y - 1) * uiResolutionX * 4 + x * 4]);

    float fN, fF;
    const aeMatrix mC = aeRenderAPI::GetMatrix_WorldToCamera ().GetMatrix ();
    const aeMatrix mP = aeRenderAPI::GetMatrix_CameraToScreen ().GetMatrix ();

    aeUInt32 g_3DWindowViewport[4];
    aeRenderAPI::getViewportAndDepthRange (g_3DWindowViewport[0], g_3DWindowViewport[1], g_3DWindowViewport[2], g_3DWindowViewport[3], fN, fF);

    const aeMatrix mWindowTransformation = mP * mC;
    const aeMatrix mInvOCP = mWindowTransformation.GetInverse ();

    const aeVec3 vScreenPos (x, uiResolutionY - y, fDepth);
    aeVec3 vPointPos;

    out_pPosition->SetVector (-1);

    if (!UnProjectFromScreen (mInvOCP, g_3DWindowViewport, vScreenPos, *out_pPosition))
    {
      out_pPosition->SetVector (0, 0, 0);
    }
  }

  {
    aeUInt8* pID = &ImagePickingSubIDs[(uiResolutionY - y - 1) * uiResolutionX * 4 + x * 4];
    out_uiSubID = pID[0] | (pID[1] << 8) | (pID[2] << 16) | (pID[3] << 24);

    pID = &ImagePickingIDs[(uiResolutionY - y - 1) * uiResolutionX * 4 + x * 4];
    aeUInt32 uiID = pID[0] | (pID[1] << 8) | (pID[2] << 16) | (pID[3] << 24);

    return uiID;
  }
}


