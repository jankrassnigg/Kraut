#include "PCH.h"

#include "../Tree/Tree.h"

#include <KrautGraphics/Framebuffer/Main.h>
#include <KrautGraphics/RenderAPI/Main.h>
#include <KrautGraphics/ShaderManager/Main.h>
#include <KrautGraphics/TextureManager/Main.h>

using namespace AE_NS_GRAPHICS;

void aeTree::RenderSkeleton(aeLod::Enum lod)
{
  CreateSkeletonRenderData(lod);

  aeShaderManager::setShader(g_Globals.s_hSkeletonShader);

  m_SkeletonMesh[lod].RenderBatch();
}

void aeTree::ClearSkeletonRenderData(void)
{
  for (aeUInt32 i = 0; i < aeLod::ENUM_COUNT; ++i)
    m_SkeletonMesh[i].Clear();
}

void aeTree::CreateSkeletonRenderData(aeLod::Enum lod)
{
  if (!m_SkeletonMesh[lod].IsCleared())
    return;

  EnsureSkeletonIsGenerated(lod);

  m_SkeletonMesh[lod].BeginBatch();
  GLIM_ATTRIBUTE col = m_SkeletonMesh[lod].Attribute4ub("attr_Color", 255, 255, 255);
  GLIM_ATTRIBUTE oid = m_SkeletonMesh[lod].Attribute4ub("attr_PickingID", 0, 0, 0, 0);
  GLIM_ATTRIBUTE sid = m_SkeletonMesh[lod].Attribute4ub("attr_PickingSubID", 0, 0, 0, 0);

  for (aeUInt32 b = 0; b < m_TreeStructure.m_BranchStructures.size(); ++b)
  {
    const Kraut::BranchStructure& branchStructure = m_TreeStructure.m_BranchStructures[b];
    const Kraut::BranchStructureLod& branchLod = m_TreeStructureLod[lod].m_BranchLODs[b];
    const aeUInt32 uiNodes = branchLod.m_NodeIDs.size();

    if (uiNodes < 2)
      continue;

    const aeUInt32 uiPickingID = 0;

    m_SkeletonMesh[lod].Attribute4ub(oid, (uiPickingID >> 0) & 0xFF, (uiPickingID >> 8) & 0xFF, (uiPickingID >> 16) & 0xFF, (uiPickingID >> 24) & 0xFF);

    m_SkeletonMesh[lod].Begin(GLIM_LINE_STRIP);

    switch (branchStructure.m_Type)
    {
      case Kraut::BranchType::Trunk1:
      case Kraut::BranchType::Trunk2:
      case Kraut::BranchType::Trunk3:
        m_SkeletonMesh[lod].Attribute4ub(col, 0, 0, 0);
        break;
      case Kraut::BranchType::MainBranches1:
      case Kraut::BranchType::MainBranches2:
      case Kraut::BranchType::MainBranches3:
        m_SkeletonMesh[lod].Attribute4ub(col, 185, 92, 0);
        break;
      case Kraut::BranchType::SubBranches1:
      case Kraut::BranchType::SubBranches2:
      case Kraut::BranchType::SubBranches3:
        m_SkeletonMesh[lod].Attribute4ub(col, 0, 64, 0);
        break;
      case Kraut::BranchType::Twigs1:
      case Kraut::BranchType::Twigs2:
      case Kraut::BranchType::Twigs3:
        m_SkeletonMesh[lod].Attribute4ub(col, 0, 170, 0);
        break;
    }

    for (aeUInt32 seg = 0; seg < uiNodes; ++seg)
    {
      aeVec3 v = branchStructure.m_Nodes[branchLod.m_NodeIDs[seg]].m_vPosition;

      const aeUInt32 sub = branchLod.m_NodeIDs[seg];

      m_SkeletonMesh[lod].Vertex(v.x, v.y, v.z);

      m_SkeletonMesh[lod].Attribute4ub(sid, (sub >> 0) & 0xFF, (sub >> 8) & 0xFF, (sub >> 16) & 0xFF, (sub >> 24) & 0xFF);
    }

    m_SkeletonMesh[lod].End();
  }

  m_SkeletonMesh[lod].EndBatch();
}
