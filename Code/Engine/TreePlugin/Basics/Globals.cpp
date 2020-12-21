#include "PCH.h"

#include "Globals.h"

AE_ON_GLOBAL_EVENT (aeTreePlugin_TreeUnloaded)  { g_Globals.Reset (); }

aeGlobals g_Globals;

aeGlobals::aeGlobals() : m_Camera("Main Camera"), m_OrthoCamera("Ortho Camera")
{
  Reset();
}

void aeGlobals::Reset(void)
{
  s_bOrbitCamera = true;
  s_vSunLightDir.SetVector (1, -2, -0.5f);
  s_vPointLightPos.SetVector (3, 5, 2);
  s_bUpdatePickingBuffer = true;
  s_bShowCollisionObjects = true;
  s_bManualEditingActive = false;
  s_iSelectedBranch = -1;
  s_uiSelectedBranchNode = 0;
  s_PaintedBranch.clear ();
  s_bPaintingBranch = false;
  s_bVisualizePaintingPlane = true;
  s_iParentBranchID = -1;
  s_uiParentBranchNodeID = 0;
  s_PaintingPlaneMode = aePaintingPlaneMode::Upwards;
  s_iBranchToBe = -1;

  s_RenderMode = aeTreeRenderMode::Diffuse;
  s_bRenderCollisionMesh = false;
  s_bDoPhysicsSimulation = false;
  s_bRenderLeaves = true;
  s_bPreviewLeafCard = false;
  s_bModifyLocally = false;
  s_CurLoD = aeLod::Lod0;
  s_uiModifyStrength = 75;
  s_bSelectLodAutomatically = false;
}
//
//bool aeGlobals::s_bManualEditingActive = false;
//bool aeGlobals::s_bShowCollisionObjects = true;
//bool aeGlobals::s_bUpdatePickingBuffer = true;
//bool aeGlobals::s_bOrbitCamera = true;
//
//aeVec3 aeGlobals::s_vCameraLookDir;
//aeVec3 aeGlobals::s_vCameraPosition;
//aeVec3 aeGlobals::s_vCameraPivot;
//
//aeVec3 aeGlobals::s_vSunLightDir (1, -2, -0.5f);
//aeVec3 aeGlobals::s_vPointLightPos (3, 5, 2);
//bool aeGlobals::s_bPaintingBranch = false;
//aeDeque<aeVec3> aeGlobals::s_PaintedBranch;
//aeInt32 aeGlobals::s_iParentBranchID = -1;
//aeUInt32 aeGlobals::s_uiParentBranchNodeID = 0;
//bool aeGlobals::s_bVisualizePaintingPlane = true;
//aePaintingPlaneMode::Enum aeGlobals::s_PaintingPlaneMode = aePaintingPlaneMode::Upwards;
//
//aePlane aeGlobals::s_PaintingPlane;
//aeVec3 aeGlobals::s_vPaintingPlaneCenter;
//aeInt32 aeGlobals::s_iBranchToBe = -1;
//aeInt32 aeGlobals::s_iSelectedBranch = -1;
//aeUInt32 aeGlobals::s_uiSelectedBranchNode = 0;
//
//aeGLCamera aeGlobals::m_Camera ("Main Camera");
//aeGLCamera aeGlobals::m_OrthoCamera ("Ortho Camera");
//aeTransformGizmo aeGlobals::s_BranchTransform;
//
//aeTreeRenderMode::Enum aeGlobals::s_RenderMode = aeTreeRenderMode::Diffuse;
//aeLod::Enum aeGlobals::s_CurLoD = aeLod::Lod0;
//bool aeGlobals::s_bRenderCollisionMesh = false;
//bool aeGlobals::s_bDoPhysicsSimulation = false;
//bool aeGlobals::s_bRenderLeaves = true;
//bool aeGlobals::s_bPreviewLeafCard = false;
//bool aeGlobals::s_bModifyLocally = false;
//aeUInt8 aeGlobals::s_uiModifyStrength = 75;
//bool aeGlobals::s_bSelectLodAutomatically = false;
//
//aeShaderResourceHandle aeGlobals::s_hBranchShader;
//aeShaderResourceHandle aeGlobals::s_hFrondShader;
//aeShaderResourceHandle aeGlobals::s_hImpostorShader;
//aeShaderResourceHandle aeGlobals::s_hLeafShader;
//aeShaderResourceHandle aeGlobals::s_hSkeletonShader;
//aeShaderResourceHandle aeGlobals::s_hGroundPlaneShader;
//aeShaderResourceHandle aeGlobals::s_hPhysicsObjectShader;
//aeShaderResourceHandle aeGlobals::s_hBillboardShader;
//aeShaderResourceHandle aeGlobals::s_hForceShader;
//aeShaderResourceHandle aeGlobals::s_hSimpleGizmoShader;
//
//GLIM_BATCH aeGlobals::s_GizmoMagnet;
//GLIM_BATCH aeGlobals::s_GizmoDirection;







