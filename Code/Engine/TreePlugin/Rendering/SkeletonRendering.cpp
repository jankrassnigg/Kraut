#include "PCH.h"

#include "../Tree/Tree.h"

#include <KrautGraphics/RenderAPI/Main.h>
#include <KrautGraphics/TextureManager/Main.h>
#include <KrautGraphics/ShaderManager/Main.h>
#include <KrautGraphics/Framebuffer/Main.h>

using namespace AE_NS_GRAPHICS;

void aeTree::RenderSkeleton (aeLod::Enum lod)
{
  CreateSkeletonRenderData (lod);

  aeShaderManager::setShader (g_Globals.s_hSkeletonShader);

  m_SkeletonMesh[lod].RenderBatch ();
}

void aeTree::ClearSkeletonRenderData (void)
{
  for (aeUInt32 i = 0; i < aeLod::ENUM_COUNT; ++i)
    m_SkeletonMesh[i].Clear ();
}

void aeTree::CreateSkeletonRenderData (aeLod::Enum lod)
{
  if (!m_SkeletonMesh[lod].IsCleared ())
    return;

  EnsureSkeletonIsGenerated (lod);

  m_SkeletonMesh[lod].BeginBatch ();
  GLIM_ATTRIBUTE col = m_SkeletonMesh[lod].Attribute4ub ("attr_Color", 255, 255, 255);
  GLIM_ATTRIBUTE oid = m_SkeletonMesh[lod].Attribute4ub ("attr_PickingID", 0, 0, 0, 0);
  GLIM_ATTRIBUTE sid = m_SkeletonMesh[lod].Attribute4ub ("attr_PickingSubID", 0, 0, 0, 0);

  for (aeUInt32 b = 0; b < m_Branches.size (); ++b)
  {
    if (m_Branches[b].IsDeleted ())
      continue;

    const aeBranch& rBranch = m_Branches[b];
    const aeUInt32 uiNodes = rBranch.m_LODs[lod].m_NodeIDs.size ();

    if (uiNodes < 2)
      continue;

    m_SkeletonMesh[lod].Attribute4ub (oid, (rBranch.m_uiPickID >> 0) & 0xFF, (rBranch.m_uiPickID >> 8) & 0xFF, (rBranch.m_uiPickID >> 16) & 0xFF, (rBranch.m_uiPickID >> 24) & 0xFF);

    m_SkeletonMesh[lod].Begin (GLIM_LINE_STRIP);

    switch (rBranch.m_Type)
    {
    case aeBranchType::Trunk1:
    case aeBranchType::Trunk2:
    case aeBranchType::Trunk3:
      m_SkeletonMesh[lod].Attribute4ub (col, 0, 0, 0);
      break;
    case aeBranchType::MainBranches1:
    case aeBranchType::MainBranches2:
    case aeBranchType::MainBranches3:
      m_SkeletonMesh[lod].Attribute4ub (col, 185, 92, 0);
      break;
    case aeBranchType::SubBranches1:
    case aeBranchType::SubBranches2:
    case aeBranchType::SubBranches3:
      m_SkeletonMesh[lod].Attribute4ub (col, 0, 64, 0);
      break;
    case aeBranchType::Twigs1:
    case aeBranchType::Twigs2:
    case aeBranchType::Twigs3:
      m_SkeletonMesh[lod].Attribute4ub (col, 0, 170, 0);
      break;
    }

    for (aeUInt32 seg = 0; seg < uiNodes; ++seg)
    {
      aeVec3 v = rBranch.m_Nodes[rBranch.m_LODs[lod].m_NodeIDs[seg]].m_vPosition;

      const aeUInt32 sub = rBranch.m_LODs[lod].m_NodeIDs[seg];

      m_SkeletonMesh[lod].Vertex (v.x, v.y, v.z);

      m_SkeletonMesh[lod].Attribute4ub (sid, (sub >> 0) & 0xFF, (sub >> 8) & 0xFF, (sub >> 16) & 0xFF, (sub >> 24) & 0xFF);
    }

    m_SkeletonMesh[lod].End ();

  }

  m_SkeletonMesh[lod].EndBatch ();
}


