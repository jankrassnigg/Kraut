#include "PCH.h"

#include "../Tree/Tree.h"
#include "../Basics/Plugin.h"

#include "../GUI/qtTreeEditWidget/qtTreeEditWidget.moc.h"
#include <KrautFoundation/Configuration/VariableRegistry.h>

#include <KrautGraphics/RenderAPI/Main.h>
#include <KrautGraphics/TextureManager/Main.h>
#include <KrautGraphics/ShaderManager/Main.h>
#include <KrautGraphics/Framebuffer/Main.h>
#include <KrautEditorBasics/Gizmos/TransformGizmo.h>

using namespace AE_NS_GRAPHICS;

extern aeCVarInt CVar_AmbientLowRed;
extern aeCVarInt CVar_AmbientLowGreen;
extern aeCVarInt CVar_AmbientLowBlue;

extern aeCVarInt CVar_AmbientHighRed;
extern aeCVarInt CVar_AmbientHighGreen;
extern aeCVarInt CVar_AmbientHighBlue;

bool s_bTreeNeedsUpdate = true;
bool s_bTreeHasSimpleChanges = true;
bool s_bTreeInfluencesChanged = true;
bool s_bStatsNeedUpdate = true;

void RenderPhysicsObjects (void);
void UpdateAllPhysicsObjects (void);
void UpdateAmbientShaderColors (void);

void aeTreePlugin::UpdateCamera (void)
{
  const float fFactor = 0.25f;

  if (g_Globals.s_bOrbitCamera)
  {
    const aeVec3 vDir = (g_Globals.s_vCameraPivot - g_Globals.s_vCameraPosition).GetNormalized ();

    if (m_bLeftMouseDown || m_bRightMouseDown)
    {
      aeQuaternion qRotY;
      qRotY.CreateQuaternion (aeVec3 (0, 1, 0), -m_iMouseMoveX * fFactor);
      g_Globals.s_vCameraPosition = qRotY * g_Globals.s_vCameraPosition;
    }

    if (m_bLeftMouseDown)
    {
      const aeVec3 vX = vDir.Cross (aeVec3 (0, 1, 0)).GetNormalized ();

      aeQuaternion qRotX;
      qRotX.CreateQuaternion (vX, -m_iMouseMoveY * fFactor);

      g_Globals.s_vCameraPosition = (qRotX * (g_Globals.s_vCameraPosition - g_Globals.s_vCameraPivot)) + g_Globals.s_vCameraPivot;
    }
    else
    if (m_bRightMouseDown)
    {
      g_Globals.s_vCameraPivot    += aeVec3 (0, -m_iMouseMoveY * fFactor, 0) * 0.1f;
      g_Globals.s_vCameraPosition += aeVec3 (0, -m_iMouseMoveY * fFactor, 0) * 0.1f;
    }
    else
    if (m_iMouseWheel != 0)
    {
      aeVec3 vNewPos = g_Globals.s_vCameraPivot + ((g_Globals.s_vCameraPosition - g_Globals.s_vCameraPivot) * (1.0f + m_iMouseWheel * -0.1f));

      const float fDist = (vNewPos - g_Globals.s_vCameraPivot).GetLength ();
      
      if ((fDist < 0.1f) || (fDist > 50.0f))
        vNewPos = g_Globals.s_vCameraPosition;

      g_Globals.s_vCameraPosition = vNewPos;
    }
    else
    if (m_bMiddleMouseDown)
    {
      aeVec3 vNewPos = g_Globals.s_vCameraPivot + ((g_Globals.s_vCameraPosition - g_Globals.s_vCameraPivot) * (1.0f + m_iMouseMoveY * 0.003f));

      const float fDist = (vNewPos - g_Globals.s_vCameraPivot).GetLength ();
      
      if ((fDist < 0.1f) || (fDist > 50.0f))
        vNewPos = g_Globals.s_vCameraPosition;

      g_Globals.s_vCameraPosition = vNewPos;
    }
  }
  else
  {
    if (m_bLeftMouseDown)
    {
      const aeVec3 vX = g_Globals.s_vCameraLookDir.Cross (aeVec3 (0, 1, 0)).GetNormalized ();

      aeQuaternion qRotY;
      qRotY.CreateQuaternion (aeVec3 (0, 1, 0), -m_iMouseMoveX * fFactor);
      g_Globals.s_vCameraLookDir = qRotY * g_Globals.s_vCameraLookDir;

      aeQuaternion qRotX;
      qRotX.CreateQuaternion (vX, -m_iMouseMoveY * fFactor);
      g_Globals.s_vCameraLookDir = qRotX * g_Globals.s_vCameraLookDir;
    }
    else
    if (m_bRightMouseDown)
    {
      const aeVec3 vRightDir = g_Globals.s_vCameraLookDir.Cross (aeVec3 (0, 1, 0)).GetNormalized ();
      const aeVec3 vUpDir = vRightDir.Cross (g_Globals.s_vCameraLookDir).GetNormalized ();

      g_Globals.s_vCameraPosition += vRightDir * m_iMouseMoveX * 0.03f * fFactor;
      g_Globals.s_vCameraPosition -= vUpDir * m_iMouseMoveY * 0.03f * fFactor;
    }
    else
    if (m_iMouseWheel != 0)
    {
      g_Globals.s_vCameraPosition += g_Globals.s_vCameraLookDir.GetNormalized () * m_iMouseWheel * 0.4f;
    }
    else
    if (m_bMiddleMouseDown)
    {
      g_Globals.s_vCameraPosition += g_Globals.s_vCameraLookDir.GetNormalized () * m_iMouseMoveY * -0.02f;
    }

    g_Globals.s_vCameraPivot = g_Globals.s_vCameraPosition + g_Globals.s_vCameraLookDir;
  }

  m_iMouseMoveX = 0;
  m_iMouseMoveY = 0;
  m_iMouseWheel = 0;
}



void aeTreePlugin::Render ()
{
  if ((s_bTreeNeedsUpdate) || (s_bTreeInfluencesChanged))
  {
    g_Tree.GenerateTree (s_bTreeNeedsUpdate);

    s_bTreeNeedsUpdate = false;
    s_bTreeHasSimpleChanges = false;
    s_bStatsNeedUpdate = true;
    s_bTreeInfluencesChanged = false;
  }

  if (s_bTreeHasSimpleChanges)
  {
    aeUndo::ModifyTree(&g_Tree);
  }

  g_Tree.m_bModifiedViaUndo = false;

  UpdateAllPhysicsObjects ();

  aeInt32 uiResolutionX = 0;
  aeInt32 uiResolutionY = 0;

  aeVariableRegistry::RetrieveInt ("system/graphics/resolution_x", uiResolutionX);
  aeVariableRegistry::RetrieveInt ("system/graphics/resolution_y", uiResolutionY);


  aeFramebuffer::BindBackbuffer ();

  if (g_Globals.s_bPreviewLeafCard)
  {
    aeRenderAPI::Clear (true, true, 0, 0, 0, 0);

    m_iMouseMoveX = 0;
    m_iMouseMoveY = 0;
    m_iMouseWheel = 0;

    RenderLeafCard (aeMath::Min (uiResolutionX, uiResolutionY), false);
    return;
  }

  aeRenderAPI::SetVSync (true);
  aeRenderAPI::Clear (true, true, 153 / 255.0f, 171 / 255.0f, 193 / 255.0f, 0.0f);

  UpdateCamera ();

  aeRenderAPI::setViewport (0, 0, uiResolutionX, uiResolutionY);

  g_Globals.m_Camera.setPerspectiveCamera (80.0f, 0.01f, 1000.0f);
  g_Globals.m_Camera.setLookAt (g_Globals.s_vCameraPosition, g_Globals.s_vCameraPivot);
  g_Globals.m_Camera.setViewport (0, 0, uiResolutionX, uiResolutionY);

  UpdateAmbientShaderColors ();

  g_Globals.s_vSunLightDir.Normalize ();
  aeShaderManager::setUniformFloat ("unif_LightPos", 3, g_Globals.s_vPointLightPos.x, g_Globals.s_vPointLightPos.y, g_Globals.s_vPointLightPos.z);
  aeShaderManager::setUniformFloat ("unif_SunDir", 3, g_Globals.s_vSunLightDir.x, g_Globals.s_vSunLightDir.y, g_Globals.s_vSunLightDir.z);

  g_Globals.m_Camera.ApplyCamera ();

  if (g_Globals.s_bShowCollisionObjects)
    RenderPhysicsObjects ();

  g_Tree.Render (g_Globals.m_Camera.getPosition (), false, false);

  aeTreePlugin::RenderPaintingPlane ();

  aeTransformGizmo::RenderAllGizmos ();

  if (s_bStatsNeedUpdate)
  {
    qtTreeEditWidget::s_pWidget->UpdateStats ();
    s_bStatsNeedUpdate = false;
  }
}

AE_ON_GLOBAL_EVENT (aeFrame_End)
{
  g_Plugin.Render ();
}


