#include "PCH.h"

#include "Globals.h"

AE_ON_GLOBAL_EVENT(aeTreePlugin_TreeUnloaded)
{
  g_Globals.Reset();
}

aeGlobals g_Globals;

aeGlobals::aeGlobals()
  : m_Camera("Main Camera")
  , m_OrthoCamera("Ortho Camera")
{
  Reset();
}

void aeGlobals::Reset(void)
{
  s_bOrbitCamera = true;
  s_vSunLightDir.SetVector(1, -2, -0.5f);
  s_vPointLightPos.SetVector(3, 5, 2);
  s_bUpdatePickingBuffer = true;
  s_bShowCollisionObjects = true;
  //s_bManualEditingActive = false;
  //s_iSelectedBranch = -1;
  //s_uiSelectedBranchNode = 0;
  //s_PaintedBranch.clear ();
  //s_bPaintingBranch = false;
  //s_bVisualizePaintingPlane = true;
  //s_iParentBranchID = -1;
  //s_uiParentBranchNodeID = 0;
  //s_PaintingPlaneMode = aePaintingPlaneMode::Upwards;
  //s_iBranchToBe = -1;

  s_RenderMode = aeTreeRenderMode::Diffuse;
  s_bRenderCollisionMesh = false;
  s_bDoPhysicsSimulation = false;
  s_bRenderLeaves = true;
  s_bPreviewLeafCard = false;
  //s_bModifyLocally = false;
  s_CurLoD = aeLod::Lod0;
  //s_uiModifyStrength = 75;
  s_bSelectLodAutomatically = false;
}
