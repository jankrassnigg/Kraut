#include "PCH.h"

#include "../Basics/Plugin.h"
#include "../Undo/TreeUndo.h"
#include "Tree.h"
#include <KrautGenerator/Lod/TreeStructureLodGenerator.h>
#include <KrautGenerator/Mesh/TreeMeshGenerator.h>
#include <KrautGraphics/Framebuffer/Main.h>
#include <KrautGraphics/RenderAPI/Main.h>
#include <KrautGraphics/ShaderManager/Main.h>
#include <KrautGraphics/TextureManager/Main.h>

using namespace AE_NS_GRAPHICS;

aeTree g_Tree;

aeTree::aeTree() = default;

AE_ON_GLOBAL_EVENT_ONCE(aeStartup_ShutdownEngine_Begin)
{
  g_Tree.ClearTreeRenderData();

  g_Globals.s_hBranchShader.Invalidate();
  g_Globals.s_hFrondShader.Invalidate();
  g_Globals.s_hImpostorShader.Invalidate();
  g_Globals.s_hLeafShader.Invalidate();
  g_Globals.s_hSkeletonShader.Invalidate();
  g_Globals.s_hGroundPlaneShader.Invalidate();
  g_Globals.s_hPhysicsObjectShader.Invalidate();
  g_Globals.s_hBillboardShader.Invalidate();
  g_Globals.s_hForceShader.Invalidate();
  g_Globals.s_hSimpleGizmoShader.Invalidate();

  g_Tree.Shutdown();
}

void aeTree::Shutdown(void)
{
  m_hLodTextures.Invalidate();
  m_hLodTexturesN.Invalidate();
}

aeUInt32 aeTree::GetNumBones(aeLod::Enum lod) const
{
  if (m_Descriptor.m_LodData[lod].m_Mode != Kraut::LodMode::Full)
    return 0;

  return m_TreeStructureLod[lod].GetNumBones();
}

aeUInt32 aeTree::GetNumTriangles(aeLod::Enum lod, Kraut::BranchGeometryType::Enum mt) const
{
  if (m_Descriptor.m_LodData[lod].m_Mode != Kraut::LodMode::Full)
    return 0;

  return m_TreeMesh[lod].GetNumTriangles(mt);
}

aeUInt32 aeTree::GetNumAllTriangles(aeLod::Enum lod) const
{
  switch (m_Descriptor.m_LodData[lod].m_Mode)
  {
    case Kraut::LodMode::Full:
      break;
    case Kraut::LodMode::FourQuads:
      return 8;
    case Kraut::LodMode::TwoQuads:
      return 4;
    case Kraut::LodMode::Billboard:
      return 2;
    case Kraut::LodMode::Disabled:
      return 0;
  }

  aeUInt32 result = 0;
  for (aeUInt32 gt = 0; gt < Kraut::BranchGeometryType::ENUM_COUNT; ++gt)
  {
    result += GetNumTriangles(lod, (Kraut::BranchGeometryType::Enum)gt);
  }

  return result;
}

//void aeTree::SkeletonHasChanged(aeUInt32 b, bool bInsertUndoStep)
//{
//  if (IsBranchDeleted(b))
//    return;
//
//  if (bInsertUndoStep)
//  {
//    if (!m_bModifiedViaUndo)
//      aeUndo::ModifyTree(this);
//  }
//
//  ClearSkeletonRenderData();
//  ClearCapsules();
//
//  for (aeUInt32 i = 0; i < aeLod::ENUM_COUNT; ++i)
//  {
//    m_TreeStructureLod[i].m_BranchLODs.clear();
//
//    m_TreeStructure.m_Branches[b]->m_fThickness = m_Descriptor.m_StructureDesc.m_BranchTypes[m_TreeStructure.m_Branches[b]->m_Type].m_uiMinBranchThicknessInCM / 100.0f;
//  }
//
//  m_TreeStructure.m_Branches[b]->UpdateThickness(m_Descriptor.m_StructureDesc, m_TreeStructure);
//
//  AE_BROADCAST_EVENT(aeEditor_QueueRedraw);
//}
//
//void aeTree::SkeletonHasChanged(void)
//{
//  bool bUndo = true;
//  for (aeUInt32 b = 0; b < m_TreeStructure.m_Branches.size(); ++b)
//  {
//    if (IsBranchDeleted(b))
//      continue;
//
//    SkeletonHasChanged(b, bUndo);
//    bUndo = false;
//  }
//}

void aeTree::Reset(void)
{
  AE_BROADCAST_EVENT(aeEditor_BlockRedraw);

  m_sTreeFile = "";
  m_Descriptor.Reset();
  m_BranchRenderInfo.clear();
  m_TreeStructure.Clear();

  for (aeUInt32 l = 0; l < aeLod::ENUM_COUNT; ++l)
  {
    m_TreeStructureLod[l].m_BranchLODs.clear();
    m_TreeMesh[l].Clear();
    m_TreeRenderData[l].Clear();
  }

  AE_BROADCAST_EVENT(aeTreeEdit_TreeModified);

  AE_BROADCAST_EVENT(aeEditor_UnblockRedraw);
}

void aeTree::GenerateTree(bool bUndoableAction)
{
  if ((!m_bModifiedViaUndo) && (bUndoableAction))
    aeUndo::ModifyTree(this);

  AE_LOG_BLOCK("aeTree::GenerateTree");

  m_TreeGenerator.m_pTreeStructure = &m_TreeStructure;
  m_TreeGenerator.m_pTreeStructureDesc = &m_Descriptor.m_StructureDesc;

  if (g_Globals.s_bDoPhysicsSimulation)
  {
    m_TreeGenerator.m_pPhysics = &m_BulletPhysicsImpl;
  }
  else
  {
    m_TreeGenerator.m_pPhysics = &m_NoPhysicsImpl;
  }

  for (aeUInt32 i = 0; i < m_Descriptor.m_Forces.size(); ++i)
    m_Descriptor.m_Forces[i]->ReactivateAllMeshSamples();

  ClearSkeletonRenderData();
  m_TreeGenerator.m_pPhysics->Reset();

  m_TreeStructure.Clear();

  for (aeUInt32 lod = 0; lod < aeLod::ENUM_COUNT; ++lod)
  {
    m_bLodStructureGenerated[lod] = false;
    m_bLodMeshGenerated[lod] = false;
    m_bLodRenderDataGenerated[lod] = false;
  }

  ConfigureInfluences();

  m_TreeGenerator.GenerateTreeStructure();

  EnsureSkeletonIsGenerated(aeLod::None);

  ComputeBoundingBox();

  g_Globals.s_bUpdatePickingBuffer = true;
  m_bLodImpostorsAreUpToDate = false;

  AE_BROADCAST_EVENT(aeTreePlugin_TreeGenerated);
}

void aeTree::EnsureSkeletonIsGenerated(aeLod::Enum lod)
{
  if (m_bLodStructureGenerated[lod])
    return;

  if (lod != aeLod::None)
    EnsureSkeletonIsGenerated(aeLod::None);

  Kraut::TreeStructureLodGenerator lodGen;
  lodGen.m_pTreeStructureLod = &m_TreeStructureLod[lod];
  lodGen.m_pTreeStructure = &m_TreeStructure;
  lodGen.m_pTreeStructureDesc = &m_Descriptor.m_StructureDesc;

  if (lod != aeLod::None)
  {
    lodGen.m_pLodDesc = &m_Descriptor.m_LodData[lod];
  }

  lodGen.GenerateTreeStructureLod();

  m_bLodStructureGenerated[lod] = true;
}

void aeTree::EnsureMeshIsGenerated(aeLod::Enum lod /*, bool bTrunkCap, bool bBranchCap*/)
{
  if (m_bLodMeshGenerated[lod])
    return;

  m_bLodMeshGenerated[lod] = true;

  EnsureSkeletonIsGenerated(lod);

  if (Kraut::LodMode::IsImpostorMode(m_Descriptor.m_LodData[lod].m_Mode))
  {
    CreateTreeImpostorMesh(lod);
  }
  else
  {
    for (aeUInt32 bt = 0; bt < Kraut::BranchType::ENUM_COUNT; ++bt)
    {
      auto& spawnDesc = m_Descriptor.m_StructureDesc.m_BranchTypes[bt];

      for (aeUInt32 gt = 0; gt < Kraut::BranchGeometryType::ENUM_COUNT; ++gt)
      {
        if (auto pMaterial = aeTreeMaterialLibrary::GetMaterial(spawnDesc.m_sTexture[gt].c_str()))
        {
          spawnDesc.m_uiTextureTilingX[gt] = pMaterial->m_uiTilingX;
          spawnDesc.m_uiTextureTilingY[gt] = pMaterial->m_uiTilingY;
        }
      }
    }

    Kraut::TreeMeshGenerator meshGenerator;
    meshGenerator.m_pLodDesc = &m_Descriptor.m_LodData[lod];
    meshGenerator.m_pTreeMesh = &m_TreeMesh[lod];
    meshGenerator.m_pTreeStructure = &m_TreeStructure;
    meshGenerator.m_pTreeStructureDesc = &m_Descriptor.m_StructureDesc;
    meshGenerator.m_pTreeStructureLod = &m_TreeStructureLod[lod];

    meshGenerator.GenerateTreeMesh();
  }
}

void aeTree::Save(aeStreamOut& stream)
{
  m_Descriptor.Save(stream);
}

void aeTree::Load(aeStreamIn& stream)
{
  AE_BROADCAST_EVENT(aeEditor_BlockRedraw);

  m_Descriptor.Load(stream);

  AE_BROADCAST_EVENT(aeEditor_UnblockRedraw);
}

void aeTree::ComputeBoundingBox(void)
{
  m_BBox = m_TreeStructure.ComputeBoundingBox();

  // increase the bbox by branch / frond / leaf size etc.
  m_BBox.AddBoundary(aeVec3(m_Descriptor.m_StructureDesc.GetBoundingBoxSizeIncrease()));

  // apply user's bbox adjustment
  m_BBox.AddBoundary(aeVec3(m_Descriptor.m_fBBoxAdjustment));

  // ensure the tree stands 'on the ground' (not above or below it)
  m_BBox.m_vMin.y = 0.0f;
}

void aeTree::ConfigureInfluences()
{
  m_Descriptor.m_StructureDesc.m_Influences.clear();

  for (aeUInt32 i = 0; i < m_Descriptor.m_Forces.size(); ++i)
  {
    const auto& force = m_Descriptor.m_Forces[i];

    if (!force->IsActive())
      continue;

    switch (force->GetType())
    {
      case aeForceType::Position:
      {
        m_Descriptor.m_StructureDesc.m_Influences.push_back();
        m_Descriptor.m_StructureDesc.m_Influences.back() = std::make_unique<Kraut::Influence_Position>();
        Kraut::Influence_Position* pInfluence = static_cast<Kraut::Influence_Position*>(m_Descriptor.m_StructureDesc.m_Influences.back().get());

        pInfluence->m_vPosition = force->GetTransform().GetTranslationVector();
        pInfluence->m_AffectedBranchTypes = force->GetBranchInfluences();
        pInfluence->m_fMinRadius = force->GetMinRadius();
        pInfluence->m_fMaxRadius = force->GetMaxRadius();
        pInfluence->m_fStrength = force->GetStrength();
        pInfluence->m_Falloff = static_cast<Kraut::Influence_Position::Falloff::Enum>(force->GetFalloff());

        break;
      }

      case aeForceType::Direction:
      {
        m_Descriptor.m_StructureDesc.m_Influences.push_back();
        m_Descriptor.m_StructureDesc.m_Influences.back() = std::make_unique<Kraut::Influence_Direction>();
        Kraut::Influence_Direction* pInfluence = static_cast<Kraut::Influence_Direction*>(m_Descriptor.m_StructureDesc.m_Influences.back().get());

        pInfluence->m_vPosition = force->GetTransform().GetTranslationVector();
        pInfluence->m_vDirection = force->GetTransform().TransformDirection(aeVec3(0, 1, 0));
        pInfluence->m_AffectedBranchTypes = force->GetBranchInfluences();
        pInfluence->m_fMinRadius = force->GetMinRadius();
        pInfluence->m_fMaxRadius = force->GetMaxRadius();
        pInfluence->m_fStrength = force->GetStrength();
        pInfluence->m_Falloff = static_cast<Kraut::Influence_Position::Falloff::Enum>(force->GetFalloff());
        break;
      }
    }
  }
}

void aeTree::ChangeBoundingAdjustment(float fNewAdjustment)
{
  if (fNewAdjustment == m_Descriptor.m_fBBoxAdjustment)
    return;

  m_bLodImpostorsAreUpToDate = false;

  for (aeUInt32 lod = 0; lod < aeLod::ENUM_COUNT; ++lod)
    m_LodImpostors[lod].Clear();

  // remove the previous adjustment
  m_BBox.AddBoundary(aeVec3(-m_Descriptor.m_fBBoxAdjustment));

  m_Descriptor.m_fBBoxAdjustment = fNewAdjustment;

  // apply the new adjustment
  m_BBox.AddBoundary(aeVec3(m_Descriptor.m_fBBoxAdjustment));

  // ensure the tree stands 'on the ground' (not above or below it)
  m_BBox.m_vMin.y = 0.0f;

  AE_BROADCAST_EVENT(aeTreeEdit_TreeModifiedSimple);
}

aeBranchRenderInfo::~aeBranchRenderInfo()
{
  if (m_uiPickID != 0)
  {
    aeEditorPlugin::UnregisterPickableObject(m_uiPickID);
  }
}
