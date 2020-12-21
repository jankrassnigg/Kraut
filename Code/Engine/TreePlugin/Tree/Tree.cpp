#include "PCH.h"

#include "Tree.h"
#include "../Basics/Plugin.h"
#include <KrautGraphics/RenderAPI/Main.h>
#include <KrautGraphics/TextureManager/Main.h>
#include <KrautGraphics/ShaderManager/Main.h>
#include <KrautGraphics/Framebuffer/Main.h>
#include "../Undo/TreeUndo.h"



using namespace AE_NS_GRAPHICS;

aeTree g_Tree;

void ClearCapsules (void);

aeTree::aeTree ()
{
  m_bModifiedViaUndo = false;
  m_bLodImpostorsAreUpToDate = false;
}

aeUInt32 aeTree::GetFreeBranch ()
{
  for (aeUInt32 b = 0; b < m_Branches.size (); ++b)
  {
    if (m_Branches[b].IsDeleted ())
    {
      m_Branches[b].SetDeleted (false);
      return b;
    }
  }

  m_Branches.push_back (aeBranch ());
  m_Branches.back ().SetDeleted (false);
  return m_Branches.size () - 1;
}

AE_ON_GLOBAL_EVENT_ONCE (aeStartup_ShutdownEngine_Begin)
{
  g_Tree.ClearTreeRenderData ();

  g_Globals.s_hBranchShader.Invalidate ();
  g_Globals.s_hFrondShader.Invalidate ();
  g_Globals.s_hImpostorShader.Invalidate ();
  g_Globals.s_hLeafShader.Invalidate ();
  g_Globals.s_hSkeletonShader.Invalidate ();
  g_Globals.s_hGroundPlaneShader.Invalidate ();
  g_Globals.s_hPhysicsObjectShader.Invalidate ();
  g_Globals.s_hBillboardShader.Invalidate ();
  g_Globals.s_hForceShader.Invalidate ();
  g_Globals.s_hSimpleGizmoShader.Invalidate ();

  g_Tree.Shutdown ();

  for (aeUInt32 b = 0; b < aeBranchType::ENUM_COUNT; ++b)
  {
    for (aeUInt32 mt = 0; mt < aeMeshType::ENUM_COUNT; ++mt)
    {
      g_Tree.m_Descriptor.m_BranchTypes[b].m_hTexture[mt].Invalidate ();
      g_Tree.m_Descriptor.m_BranchTypes[b].m_hTextureN[mt].Invalidate ();
    }
  }
}

void aeTree::Shutdown (void)
{
  m_hLodTextures.Invalidate ();
  m_hLodTexturesN.Invalidate ();
}

aeUInt32 aeTree::GetNumBones (aeLod::Enum lod) const
{
  if (m_Descriptor.m_LodData[lod].m_Mode != aeLodMode::Full)
    return 0;

  aeUInt32 uiBones = 0;

  for (aeUInt32 b = 0; b < m_Branches.size (); ++b)
  {
    if (m_Branches[b].IsDeleted ())
      continue;

    uiBones += m_Branches[b].m_LODs[lod].m_NodeIDs.size ();
  }

  return uiBones;
}

aeUInt32 aeTree::GetNumTriangles (aeLod::Enum lod, aeMeshType::Enum mt) const
{
  if (m_Descriptor.m_LodData[lod].m_Mode != aeLodMode::Full)
    return 0;

  aeUInt32 uiTris = 0;
  
  for (aeUInt32 b = 0; b < m_Branches.size (); ++b)
  {
    if (m_Branches[b].IsDeleted ())
      continue;

    uiTris += m_Branches[b].m_LODs[lod].m_SubMesh[mt].GetNumTriangles ();
  }

  return uiTris;
}

aeUInt32 aeTree::GetNumAllTriangles (aeLod::Enum lod) const
{
  switch (m_Descriptor.m_LodData[lod].m_Mode)
  {
  case aeLodMode::Full:
    break;
  case aeLodMode::FourQuads:
    return 8;
  case aeLodMode::TwoQuads:
    return 4;
  case aeLodMode::Billboard:
    return 2;
  case aeLodMode::Disabled:
    return 0;
  }


  aeUInt32 num = 0;
  for (aeUInt32 mt = 0; mt < aeMeshType::ENUM_COUNT; ++mt)
    num += GetNumTriangles (lod, (aeMeshType::Enum) mt);

  return num;
}

void aeTree::SkeletonHasChanged (aeUInt32 b, bool bInsertUndoStep)
{
  if (m_Branches[b].IsDeleted ())
    return;

  if (bInsertUndoStep)
  {
    if (!m_bModifiedViaUndo)
      aeUndo::ModifyTree (this);
  }

  ClearSkeletonRenderData ();
  ClearCapsules ();

  ClearBranchRenderData (b);

  for (aeUInt32 i = 0; i < aeLod::ENUM_COUNT; ++i)
  {
    m_Branches[b].m_LODs[i].m_bSkeletonGenerated = false;
    m_Branches[b].m_LODs[i].m_NodeIDs.clear ();
    m_Branches[b].m_LODs[i].m_TipNodes.clear ();
    m_Branches[b].m_fThickness = m_Descriptor.m_BranchTypes[m_Branches[b].m_Type].m_uiMinBranchThicknessInCM / 100.0f;

    for (aeUInt32 mt = 0; mt < aeMeshType::ENUM_COUNT; ++mt)
    {
      m_Branches[b].m_LODs[i].m_bMeshGenerated[mt] = false;
      m_Branches[b].m_LODs[i].m_SubMesh[mt].clear ();
    }
  }

  ComputeBranchThickness (b);

  AE_BROADCAST_EVENT (aeEditor_QueueRedraw);
}

void aeTree::SkeletonHasChanged (void)
{
  bool bUndo = true;
  for (aeUInt32 b = 0; b < m_Branches.size (); ++b)
  {
    if (m_Branches[b].IsDeleted ())
      continue;

    SkeletonHasChanged (b, bUndo);
    bUndo = false;
  }
}

void aeTree::Reset (void)
{
  AE_BROADCAST_EVENT (aeEditor_BlockRedraw);

  m_sTreeFile = "";
  m_Descriptor.Reset ();
  m_Branches.clear ();

  AE_BROADCAST_EVENT (aeTreeEdit_TreeModified);

  AE_BROADCAST_EVENT (aeEditor_UnblockRedraw);
}

void aeTree::GenerateTree (bool bUndoableAction)
{
  if ((!m_bModifiedViaUndo) && (bUndoableAction))
    aeUndo::ModifyTree (this);

  AE_LOG_BLOCK ("aeTree::GenerateTree");

  for (aeUInt32 i = 0; i < m_Descriptor.m_Forces.size (); ++i)
    m_Descriptor.m_Forces[i]->ReactivateAllMeshSamples ();

  //m_BBox.SetInvalid ();

  ClearSkeletonRenderData ();
  ClearTreeRenderData ();
  ClearCapsules ();

  for (aeUInt32 b = 0; b < m_Branches.size (); ++b)
  {
    if (!m_Branches[b].m_bManuallyCreated)
      m_Branches[b].SetDeleted (true);
    else
      SkeletonHasChanged (b, false);
  }

  GrowSkeleton ();

  EnsureSkeletonIsGenerated (aeLod::None);

  ComputeBoundingBox ();

  g_Globals.s_bUpdatePickingBuffer = true;
  m_bLodImpostorsAreUpToDate = false;

  AE_BROADCAST_EVENT (aeTreePlugin_TreeGenerated);
}

void aeTree::EnsureSkeletonIsGenerated (aeLod::Enum lod)
{
  if (lod != aeLod::None)
    EnsureSkeletonIsGenerated (aeLod::None);

  if (lod == aeLod::None)
    GenerateFullDetailSkeleton ();
  else
    GenerateReducedSkeleton (lod);
}

void aeTree::EnsureMeshIsGenerated (aeLod::Enum lod, bool bTrunkCap, bool bBranchCap)
{
  AE_PREVENT_RECURSION;

  EnsureSkeletonIsGenerated (lod);

  GenerateAllTreeMeshes (lod, bTrunkCap, bBranchCap);
}

void aeTree::Save (aeStreamOut& stream)
{
  m_Descriptor.Save (stream);

  aeUInt8 uiVersion = 1;
  stream << uiVersion;

  aeUInt16 uiBranches = m_Branches.size ();
  stream << uiBranches;

  for (aeUInt32 b = 0; b < uiBranches; ++b)
  {
    m_Branches[b].Save (stream);
  }
}

void aeTree::Load (aeStreamIn& stream)
{
  AE_BROADCAST_EVENT (aeEditor_BlockRedraw);

  m_Descriptor.Load (stream);

  if (stream.IsEndOfStream ())
  {
    AE_BROADCAST_EVENT (aeEditor_UnblockRedraw);
    return;
  }

  aeUInt8 uiVersion;
  stream >> uiVersion;

  aeUInt16 uiBranches;
  stream >> uiBranches;
  m_Branches.resize (uiBranches);

  for (aeUInt32 b = 0; b < uiBranches; ++b)
  {
    m_Branches[b].Load (stream);
    m_Branches[b].m_pTree = this;
  }

  AE_BROADCAST_EVENT (aeEditor_UnblockRedraw);
}

static float GetSizeIncrease (const aeTreeDescriptor& td)
{
  float fResult = 0.0f;

  for (aeUInt32 bt = 0; bt < aeBranchType::ENUM_COUNT; ++bt)
  {
    const aeSpawnNodeDesc& snd = td.m_BranchTypes[bt];

    if (!snd.m_bUsed)
      continue;

    //if (snd.m_bEnable[aeMeshType::Branch])
    //{
    //  fResult = aeMath::Max (fResult, snd.m_uiMaxBranchThicknessInCM / 100.0f);

    //  if (snd.m_uiFlares > 0)
    //    fResult = aeMath::Max (fResult, (snd.m_uiMaxBranchThicknessInCM / 100.0f) * snd.m_fFlareWidth);
    //}

    if (snd.m_bEnable[aeMeshType::Frond])
    {
      // fronds are usually not oriented in a way that they take the maximum area (in the bounding box)
      // so scale them down by 0.5 to get a better approximation (otherwise the bbox gets too big)
      fResult = aeMath::Max (fResult, snd.m_fFrondHeight * 0.5f);
      fResult = aeMath::Max (fResult, snd.m_fFrondWidth  * 0.5f);
    }

    if (snd.m_bEnable[aeMeshType::Leaf])
    {
      fResult = aeMath::Max (fResult, snd.m_fLeafSize);
    }
  }

  return fResult;
}

static aeBBox GetTreeSkeletonBBox (const aeDeque<aeBranch>& Branches)
{
  aeBBox Result;
  Result.SetInvalid ();

  float fMaxThickness = 0.0f;

  // inrease the bbox size by all node that are in the high-detail skeleton
  const aeUInt32 uiBranches = Branches.size ();
  for (aeUInt32 b = 0; b < uiBranches; ++b)
  {
    const aeDeque<aeBranchNode>& nodes = Branches[b].m_Nodes;

    // all regular nodes
    const aeUInt32 uiNodes = nodes.size ();
    for (aeUInt32 n = 0; n < uiNodes; ++n)
    {
      Result.ExpandToInclude (nodes[n].m_vPosition);

      fMaxThickness = aeMath::Max (fMaxThickness, nodes[n].m_fThickness);
    }
  }

  Result.AddBoundary (aeVec3 (fMaxThickness));

  return Result;
}

void aeTree::ComputeBoundingBox (void)
{
  m_BBox = GetTreeSkeletonBBox (m_Branches);

  // increase the bbox by branch / frond / leaf size etc.
  m_BBox.AddBoundary (aeVec3 (GetSizeIncrease (m_Descriptor)));

  // apply user's bbox adjustment
  m_BBox.AddBoundary (aeVec3 (m_Descriptor.m_fBBoxAdjustment));

  // ensure the tree stands 'on the ground' (not above or below it)
  m_BBox.m_vMin.y = 0.0f;
}

void aeTree::ChangeBoundingAdjustment (float fNewAdjustment)
{
  if (fNewAdjustment == m_Descriptor.m_fBBoxAdjustment)
    return;

  m_bLodImpostorsAreUpToDate = false;

  for (aeUInt32 lod = 0; lod < aeLod::ENUM_COUNT; ++lod)
    m_LodImpostors[lod].Clear ();

  // remove the previous adjustment
  m_BBox.AddBoundary (aeVec3 (-m_Descriptor.m_fBBoxAdjustment));

  m_Descriptor.m_fBBoxAdjustment = fNewAdjustment;

  // apply the new adjustment
  m_BBox.AddBoundary (aeVec3 (m_Descriptor.m_fBBoxAdjustment));

  // ensure the tree stands 'on the ground' (not above or below it)
  m_BBox.m_vMin.y = 0.0f;

  AE_BROADCAST_EVENT (aeTreeEdit_TreeModifiedSimple);
}

