#include "PCH.h"

//#include "../Basics/Plugin.h"
//#include "../GUI/qtTreeEditWidget/qtTreeEditWidget.moc.h"
//#include "../Tree/Tree.h"
//#include "../Undo/TreeUndo.h"
//#include <KrautEditorBasics/Plugin.h>
//#include <KrautFoundation/Math/Plane.h>
//#include <KrautGraphics/RenderAPI/RenderAPI.h>
//#include <KrautGraphics/ShaderManager/ShaderManager.h>
//
//using namespace AE_NS_GRAPHICS;
//
//static void TransformNode(aeTree* pTree, Kraut::BranchStructure& branch, aeUInt32 uiNode, const aeMatrix& mTransform0, float fSelectionStrength0, bool bLocal)
//{
//  if (fSelectionStrength0 <= 0.0f)
//    return;
//
//  // if this transformation moves the entire branch, rotate the 'leaf up direction' accordingly
//  if (uiNode == 0)
//  {
//    branch.m_vLeafUpDirection = mTransform0.TransformDirection(branch.m_vLeafUpDirection);
//  }
//
//  const float fFactor = g_Globals.s_uiModifyStrength / 100.0f;
//
//  float fSelectionStrength = fSelectionStrength0;
//
//  for (aeUInt32 n = uiNode; n < branch.m_Nodes.size(); ++n)
//  {
//    if (!bLocal)
//      fSelectionStrength = 1.0f;
//
//    const aeMatrix mTransform = aeMath::Lerp(aeMatrix::IdentityMatrix(), mTransform0, aeMath::Max(0.0f, fSelectionStrength));
//    fSelectionStrength *= fFactor;
//
//    branch.m_Nodes[n].m_vPosition = mTransform * branch.m_Nodes[n].m_vPosition;
//
//    if (branch.m_Nodes[n].m_bHasChildBranches)
//    {
//      // search for all branches that branch from this node
//      for (aeUInt32 b = 0; b < pTree->m_TreeStructure.m_Branches.size(); ++b)
//      {
//        if (pTree->IsBranchDeleted(b))
//          continue;
//
//        if ((pTree->m_TreeStructure.m_Branches[b]->m_iParentBranchID >= 0) &&
//            (pTree->m_TreeStructure.m_Branches[pTree->m_TreeStructure.m_Branches[b]->m_iParentBranchID].get() == &branch) &&
//            (pTree->m_TreeStructure.m_Branches[b]->m_uiParentBranchNodeID == n))
//        {
//          TransformNode(pTree, *pTree->m_TreeStructure.m_Branches[b], 0, mTransform, fSelectionStrength, false);
//        }
//      }
//    }
//  }
//
//  fSelectionStrength = fSelectionStrength0;
//
//  for (aeInt32 n = (aeInt32)uiNode - 1; n >= 0; --n)
//  {
//    fSelectionStrength *= fFactor;
//
//    const aeMatrix mTransform = aeMath::Lerp(aeMatrix::IdentityMatrix(), mTransform0, aeMath::Max(0.0f, fSelectionStrength));
//
//    branch.m_Nodes[n].m_vPosition = mTransform * branch.m_Nodes[n].m_vPosition;
//
//    if (branch.m_Nodes[n].m_bHasChildBranches)
//    {
//      // search for all branches that branch from this node
//      for (aeUInt32 b = 0; b < pTree->m_TreeStructure.m_Branches.size(); ++b)
//      {
//        if (pTree->IsBranchDeleted(b))
//          continue;
//
//        if ((pTree->m_TreeStructure.m_Branches[b]->m_iParentBranchID >= 0) &&
//            (pTree->m_TreeStructure.m_Branches[pTree->m_TreeStructure.m_Branches[b]->m_iParentBranchID].get() == &branch) &&
//            (pTree->m_TreeStructure.m_Branches[b]->m_uiParentBranchNodeID == n))
//        {
//          TransformNode(pTree, *pTree->m_TreeStructure.m_Branches[b], 0, mTransform, fSelectionStrength, false);
//        }
//      }
//    }
//  }
//}
//
//
//void UpdateBranchTransform(void* pPassThrough, const void* pEventData)
//{
//  aeTransformGizmo::EventData* pED = (aeTransformGizmo::EventData*)pEventData;
//
//  if (pED->m_EventType == aeTransformGizmo::DragBegin)
//    aeUndo::BeginOperation();
//  if (pED->m_EventType == aeTransformGizmo::DragEnd)
//    aeUndo::EndOperation();
//
//  if (pED->m_EventType == aeTransformGizmo::Transformed)
//  {
//    const aeMatrix mPrevInv = pED->m_PrevTransform.GetInverse();
//
//    Kraut::BranchStructure* pBranch = (Kraut::BranchStructure*)pED->m_Gizmo->GetObjectPointer();
//    TransformNode(&g_Tree, *pBranch, pED->m_Gizmo->GetObjectID(), pED->m_Gizmo->GetTransform() * mPrevInv, 1.0f, g_Globals.s_bModifyLocally);
//
//    AE_BROADCAST_EVENT(aeTreeEdit_TreeModified);
//  }
//}
//
//AE_ON_GLOBAL_EVENT_ONCE(aeTreePlugin_Init)
//{
//  g_Globals.s_BranchTransform.SetActive(false);
//  g_Globals.s_BranchTransform.m_Events.RegisterEventReceiver(UpdateBranchTransform, nullptr);
//}
//
//AE_ON_GLOBAL_EVENT(aeTreePlugin_AfterUndo)
//{
//  aeTreePlugin::SetSelectedBranch(-1, 0);
//}
//
//AE_ON_GLOBAL_EVENT(aeTreePlugin_AfterRedo)
//{
//  aeTreePlugin::SetSelectedBranch(-1, 0);
//}
//
//
//
//aePlane aeTree::GetAdjustedPaintingPlane(aeInt32 iParentBranch, aeUInt32 uiParentBranchNode, const aeVec3& vStartPos, aePlane PaintingPlane)
//{
//  Kraut::BranchStructure* pParent = nullptr;
//  if (iParentBranch != -1)
//    pParent = m_TreeStructure.m_Branches[iParentBranch].get();
//
//  if (pParent)
//  {
//    // only allow to attach branches to manually created parent branches
//    if (!pParent->m_bManuallyCreated)
//      return PaintingPlane;
//
//    // search for the branch node that comes closest to the starting point of the to be added branch
//    float fMinDist = (vStartPos - pParent->m_Nodes[uiParentBranchNode].m_vPosition).GetLength();
//
//    for (aeUInt32 n = uiParentBranchNode; n < pParent->m_Nodes.size(); ++n)
//    {
//      const float fDist = (vStartPos - pParent->m_Nodes[n].m_vPosition).GetLength();
//
//      if (fDist < fMinDist)
//      {
//        fMinDist = fDist;
//        uiParentBranchNode = n;
//      }
//    }
//
//    // found the closest node of the parent branch
//    aeVec3 vBranchStartPos = pParent->m_Nodes[uiParentBranchNode].m_vPosition;
//
//    PaintingPlane.CreatePlane(PaintingPlane.m_vNormal, vBranchStartPos);
//  }
//
//  return PaintingPlane;
//}
//
//aeInt32 FindNodeClosestToBranchDirection(aeDeque<aeVec3>& vBranchPoints, const aePlane& PaintingPlane, const aeDeque<Kraut::BranchNode>& ParentBranchNodes)
//{
//  aeInt32 iClosest = -1;
//  float fAbsDist = aeMath::FloatMax_Pos();
//
//  for (aeUInt32 i = 0; i < ParentBranchNodes.size(); ++i)
//  {
//    //if (vCutPlane.GetDistanceToPoint (ParentBranchNodes[i].m_vPosition) >= 0.0f)
//    {
//      const float fDist = (ParentBranchNodes[i].m_vPosition - vBranchPoints[0]).GetLengthSquared();
//
//      if (fDist < fAbsDist)
//      {
//        fAbsDist = fDist;
//        iClosest = i;
//      }
//    }
//  }
//
//  AE_CHECK_DEV(iClosest >= 0, "There should always be a node that is closer than 'infinitely far away'.");
//
//  const aeVec3 vAttachPos = ParentBranchNodes[iClosest].m_vPosition;
//
//  while (vBranchPoints.size() > 2)
//  {
//    const aeVec3 vBranchDir = (vBranchPoints[1] - vBranchPoints[0]).GetNormalized();
//
//    const aeVec3 vCutNormal = -vBranchDir;
//    aePlane vCutPlane(vCutNormal, vBranchPoints[0]);
//
//    if (vCutPlane.GetDistanceToPoint(vAttachPos) < 0.0f)
//      vBranchPoints.pop_front();
//    else
//      break;
//  }
//
//  {
//    const aeVec3 vChildDir = (vBranchPoints[1] - vBranchPoints[0]).GetNormalized();
//    float fMinAngle = 180.0f;
//    aeInt32 iNewClosest = iClosest;
//
//    for (aeInt32 m = -5; m <= 5; ++m)
//    {
//      const aeInt32 iCur = iClosest + m;
//      if ((iCur >= 0) && (iCur < (aeInt32)ParentBranchNodes.size()))
//      {
//        const aeVec3 vBranchDir = (vBranchPoints[0] - ParentBranchNodes[iCur].m_vPosition).GetNormalized();
//
//        const float fAngle = vChildDir.GetAngleBetween(vBranchDir);
//
//        if (fAngle < fMinAngle)
//        {
//          fMinAngle = fAngle;
//          iNewClosest = iCur;
//        }
//      }
//    }
//
//    iClosest = iNewClosest;
//  }
//
//  return iClosest;
//}
//
//aeInt32 aeTree::AddPaintedBranch(aeInt32 iParentBranch, aeUInt32 uiParentBranchNode, const aeDeque<aeVec3>& vPoints0, const aePlane& PaintingPlane, bool bForPreview, aeInt32 iReuseBranch)
//{
//  if (vPoints0.size() <= 3)
//    return -1;
//
//  aeDeque<aeVec3> vPoints = vPoints0;
//
//  Kraut::BranchStructure* pParent = nullptr;
//  if (iParentBranch != -1)
//    pParent = m_TreeStructure.m_Branches[iParentBranch].get();
//
//  if (qtTreeEditWidget::s_pWidget->GetCurrentlyEditedBranchType() == Kraut::BranchType::None)
//    return -1;
//
//  Kraut::BranchNodeRandomData NodeRD;
//  NodeRD.m_BranchRD.m_uiSeedValue = m_Descriptor.m_StructureDesc.m_uiRandomSeed;
//
//  if (pParent)
//  {
//    // only allow to attach branches to manually created parent branches
//    if (!pParent->m_bManuallyCreated)
//      return -1;
//
//    vPoints.pop_front();
//
//    uiParentBranchNode = FindNodeClosestToBranchDirection(vPoints, PaintingPlane, pParent->m_Nodes);
//
//    vPoints.pop_front();
//
//    // found the closest node of the parent branch
//    aeVec3 vBranchStartPos = pParent->m_Nodes[uiParentBranchNode].m_vPosition;
//
//    vPoints.push_front(vBranchStartPos);
//
//    // now move all points of the new branch into the proper plane
//    {
//      for (aeUInt32 i = 0; i < vPoints.size(); ++i)
//        vPoints[i] = PaintingPlane.ProjectOntoPlane(vPoints[i]);
//    }
//
//    NodeRD = pParent->m_RandomData.GetBranchNodeRD();
//
//    if (!bForPreview)
//      pParent->m_Nodes[uiParentBranchNode].m_bHasChildBranches = true;
//  }
//
//  const Kraut::SpawnNodeDesc& BranchTypeDesc = m_Descriptor.m_StructureDesc.m_BranchTypes[qtTreeEditWidget::s_pWidget->GetCurrentlyEditedBranchType()];
//  Kraut::BranchStats dB = m_Descriptor.m_StructureDesc.m_BranchTypes[qtTreeEditWidget::s_pWidget->GetCurrentlyEditedBranchType()].CreateBranchDesc(NodeRD.GetBranchRD());
//
//  aeInt32 uiBranchID = iReuseBranch;
//
//  if (uiBranchID < 0)
//  {
//    uiBranchID = GetFreeBranch();
//  }
//  else
//  {
//    // recreate the branch data to reset it
//    ClearBranchData(uiBranchID);
//    AllocateBranch(uiBranchID);
//  }
//
//  Kraut::BranchStructure* pBranch = m_TreeStructure.m_Branches[uiBranchID].get();
//
//  pBranch->m_bManuallyCreated = true;
//  pBranch->m_iParentBranchID = iParentBranch;
//  pBranch->m_uiParentBranchNodeID = uiParentBranchNode;
//  pBranch->m_fBranchLength = 0.0f;
//  pBranch->m_fFrondColorVariation = dB.m_fFrondColorVariation;
//  pBranch->m_uiFrondTextureVariation = dB.m_uiFrondTextureVariation;
//  pBranch->m_fThickness = dB.m_fBranchThickness;
//  pBranch->m_RandomData = dB.m_RandomData;
//  pBranch->m_Type = qtTreeEditWidget::s_pWidget->GetCurrentlyEditedBranchType();
//  pBranch->m_vLeafUpDirection = PaintingPlane.m_vNormal.Cross(vPoints[1] - vPoints[0]).GetNormalized();
//
//  for (aeUInt32 i = 0; i < vPoints.size(); ++i)
//  {
//    if (i > 0)
//      pBranch->m_fBranchLength += (vPoints[i] - vPoints[i - 1]).GetLength();
//
//    pBranch->m_Nodes.push_back(Kraut::BranchNode());
//    pBranch->m_Nodes.back().m_vPosition = vPoints[i];
//  }
//
//  if (!pBranch->UpdateThickness(m_Descriptor.m_StructureDesc, m_TreeStructure))
//  {
//    DeleteBranch(uiBranchID, false);
//    return -1;
//  }
//
//  pBranch->GenerateTexCoordV(m_Descriptor.m_StructureDesc);
//
//  EnsureSkeletonIsGenerated(aeLod::None);
//
//  if (!bForPreview)
//  {
//    g_Globals.s_bUpdatePickingBuffer = true;
//
//    AE_BROADCAST_EVENT(aeTreeEdit_TreeModified);
//  }
//  else
//  {
//    SkeletonHasChanged(uiBranchID, false);
//    AE_BROADCAST_EVENT(aeEditor_QueueRedraw);
//  }
//
//  return uiBranchID;
//}
//
//void aeTreePlugin::CreatePaintingPlane(const aeVec3& vClickPos, const aeVec3& vCameraRight, const aeVec3& vCameraUp, aeInt32 iBranch, aeUInt32 iBranchNode)
//{
//  aePaintingPlaneMode::Enum mode = g_Globals.s_PaintingPlaneMode;
//
//  aeVec3 vBranchDir(0.0f);
//
//  // if there is no parent branch, restrict painting plane to upwards or viewplane
//  if (iBranch < 0)
//  {
//    if ((mode != aePaintingPlaneMode::Upwards) && (mode != aePaintingPlaneMode::InViewPlane))
//      mode = aePaintingPlaneMode::Upwards;
//  }
//  else
//  {
//    vBranchDir = g_Tree.m_TreeStructure.m_Branches[iBranch]->GetDirectionAtNode(iBranchNode);
//
//    if (vBranchDir.IsZeroVector())
//      mode = aePaintingPlaneMode::Upwards;
//  }
//
//  switch (mode)
//  {
//    case aePaintingPlaneMode::InViewPlane:
//    {
//      const aeVec3 vForwards = vCameraRight.Cross(vCameraUp);
//      g_Globals.s_PaintingPlane.CreatePlane(vForwards, vClickPos);
//    }
//    break;
//    case aePaintingPlaneMode::Upwards:
//    {
//      const aeVec3 vForwards = vCameraRight.Cross(aeVec3(0, 1, 0)); //vCameraUp);
//      g_Globals.s_PaintingPlane.CreatePlane(vForwards, vClickPos);
//    }
//    break;
//    case aePaintingPlaneMode::Planar:
//    {
//      g_Globals.s_PaintingPlane.CreatePlane(aeVec3(0, 1, 0), vClickPos);
//    }
//    break;
//    case aePaintingPlaneMode::AlongBranch:
//    {
//      const aeVec3 vForwards = vCameraRight.Cross(vCameraUp);
//      const aeVec3 vUpwards = vBranchDir.Cross(vForwards).GetNormalized();
//      const aeVec3 vPlane = vBranchDir.Cross(vUpwards).GetNormalized();
//
//      g_Globals.s_PaintingPlane.CreatePlane(vPlane, vClickPos);
//    }
//    break;
//    case aePaintingPlaneMode::OrthogonalToBranch:
//    {
//      g_Globals.s_PaintingPlane.CreatePlane(vBranchDir, vClickPos);
//    }
//    break;
//  }
//
//  g_Globals.s_PaintingPlane = g_Tree.GetAdjustedPaintingPlane(iBranch, iBranchNode, vClickPos, g_Globals.s_PaintingPlane);
//  g_Globals.s_vPaintingPlaneCenter = g_Globals.s_PaintingPlane.ProjectOntoPlane(vClickPos);
//}
//
//static aeShaderResourceHandle s_hPaintingPlaneShader;
//
//AE_ON_GLOBAL_EVENT(aeStartup_ShutdownEngine_Begin)
//{
//  s_hPaintingPlaneShader.Invalidate();
//}
//
//void aeTreePlugin::RenderPaintingPlane(void)
//{
//  if (!s_hPaintingPlaneShader.IsValid())
//    s_hPaintingPlaneShader = aeShaderResource::LoadResource("Shaders/PaintingPlane.shader");
//
//  if (g_Globals.s_bPaintingBranch && g_Globals.s_bVisualizePaintingPlane)
//  {
//    aeRenderAPI::PushRenderStates(true);
//    aeRenderAPI::setDepthMask(false);
//    aeRenderAPI::setFaceCulling(AE_CULL_FACE_NONE);
//
//    aeShaderManager::setShader(s_hPaintingPlaneShader);
//
//    const float fSize = 7.0f;
//    const aeVec3 v = g_Globals.s_vPaintingPlaneCenter;
//    const aeVec3 r = g_Globals.s_PaintingPlane.m_vNormal.GetOrthogonalVector().GetNormalized();
//    const aeVec3 u = g_Globals.s_PaintingPlane.m_vNormal.Cross(r).GetNormalized();
//
//    aeVec3 c[4] = {v - r * fSize - u * fSize, v + r * fSize - u * fSize, v + r * fSize + u * fSize, v - r * fSize + u * fSize};
//
//    // Render Lines
//    {
//      glim.BeginBatch();
//      glim.Attribute4ub("attr_Color", 136, 0, 4, 255);
//
//      glim.Begin(GLIM_LINES);
//
//      for (float f = -fSize + 1.0f; f < fSize; f += 1.0f)
//      {
//        const aeVec3 vPos0 = v + r * f - u * fSize;
//        const aeVec3 vPos1 = v + r * f + u * fSize;
//
//        glim.Vertex(vPos0.x, vPos0.y, vPos0.z);
//        glim.Vertex(vPos1.x, vPos1.y, vPos1.z);
//      }
//
//      for (float f = -fSize + 1.0f; f < fSize; f += 1.0f)
//      {
//        const aeVec3 vPos0 = v - r * fSize + u * f;
//        const aeVec3 vPos1 = v + r * fSize + u * f;
//
//        glim.Vertex(vPos0.x, vPos0.y, vPos0.z);
//        glim.Vertex(vPos1.x, vPos1.y, vPos1.z);
//      }
//
//      glim.End();
//
//      glim.EndBatch();
//      glim.RenderBatch();
//    }
//
//    // Render Opaque Plane
//    {
//      aeRenderAPI::setBlending(true, AE_FACTOR_SRC_ALPHA, AE_FACTOR_INV_SRC_ALPHA);
//
//      glim.BeginBatch();
//      glim.Attribute4ub("attr_Color", 0, 72, 255, 128);
//
//      glim.Begin(GLIM_QUADS);
//
//      for (int i = 0; i < 4; ++i)
//        glim.Vertex(c[i].x, c[i].y, c[i].z);
//
//      glim.End();
//
//      glim.EndBatch();
//      glim.RenderBatch();
//    }
//
//    aeRenderAPI::PopRenderStates();
//  }
//}
