#ifndef AE_TREEPLUGIN_GLOBALS_H
#define AE_TREEPLUGIN_GLOBALS_H

#include "Base.h"
#include <KrautEditorBasics/Gizmos/TransformGizmo.h>
#include <KrautGraphics/glCamera/Main.h>

using namespace AE_NS_EDITORBASICS;
using namespace AE_NS_GRAPHICS;

struct aeGlobals
{
  aeGlobals();
  void Reset(void);

  //bool s_bManualEditingActive;
  bool s_bShowCollisionObjects;
  bool s_bUpdatePickingBuffer;
  bool s_bOrbitCamera;
  aeVec3 s_vCameraLookDir;
  aeVec3 s_vCameraPosition;
  aeVec3 s_vCameraPivot;

  aeVec3 s_vSunLightDir;
  aeVec3 s_vPointLightPos;
  //bool s_bPaintingBranch;
  //aeDeque<aeVec3> s_PaintedBranch;
  //aeInt32 s_iParentBranchID;
  //aeUInt32 s_uiParentBranchNodeID;
  //bool s_bVisualizePaintingPlane;
  //aePaintingPlaneMode::Enum s_PaintingPlaneMode;

  //aePlane s_PaintingPlane;
  //aeVec3 s_vPaintingPlaneCenter;
  //aeInt32 s_iBranchToBe;
  //aeInt32 s_iSelectedBranch;
  //aeUInt32 s_uiSelectedBranchNode;
  aeGLCamera m_Camera;
  aeGLCamera m_OrthoCamera;
  //aeTransformGizmo s_BranchTransform;

  aeTreeRenderMode::Enum s_RenderMode;
  aeLod::Enum s_CurLoD;
  bool s_bRenderCollisionMesh;
  bool s_bDoPhysicsSimulation;
  bool s_bRenderLeaves;
  bool s_bPreviewLeafCard;
  //bool s_bModifyLocally;
  //aeUInt8 s_uiModifyStrength;
  bool s_bSelectLodAutomatically;

  aeShaderResourceHandle s_hBranchShader;
  aeShaderResourceHandle s_hFrondShader;
  aeShaderResourceHandle s_hImpostorShader;
  aeShaderResourceHandle s_hLeafShader;
  aeShaderResourceHandle s_hSkeletonShader;
  aeShaderResourceHandle s_hGroundPlaneShader;
  aeShaderResourceHandle s_hPhysicsObjectShader;
  aeShaderResourceHandle s_hBillboardShader;
  aeShaderResourceHandle s_hForceShader;
  aeShaderResourceHandle s_hSimpleGizmoShader;

  GLIM_BATCH s_GizmoMagnet;
  GLIM_BATCH s_GizmoDirection;
};


extern aeGlobals g_Globals;

#endif
