#include "PCH.h"

#include "../Tree/Tree.h"
#include "../GUI/qtResourceEditorWidget/MaterialLibrary.h"
#include <KrautFoundation/Math/Plane.h>


using namespace AE_NS_GRAPHICS;

void aeTree::AddBillbardLeaf (aeUInt32 uiBranch, aeLod::Enum lod, const aeVec3& vPos, float fSize, const aeTreeMaterial* pMaterial)
{
  if (fSize < 0.05f)
    return;

  aeBranch& Branch = m_Branches[uiBranch];
  const aeSpawnNodeDesc& bd = g_Tree.m_Descriptor.m_BranchTypes[Branch.m_Type];

  aeMeshTriangle t;
  t.m_uiPickingID = Branch.m_uiPickID;
  t.m_uiPickingSubID = 0;

  aeMeshVertex mv;
  mv.m_vPosition = vPos;
  mv.m_uiColorVariation = (aeUInt8) aeMath::Rand (255);

  const aeInt32 iTexX = aeMath::Rand (pMaterial->m_uiTilingX);
  const aeInt32 iTexY = aeMath::Rand (pMaterial->m_uiTilingX);

  const float fTexWidth  = 1.0f / pMaterial->m_uiTilingX;
  const float fTexHeight = 1.0f / pMaterial->m_uiTilingY;

  const float fTexCoordU0 = fTexWidth * iTexX;
  const float fTexCoordV0 = fTexHeight * iTexY;

  const float fTexCoordU1 = fTexCoordU0 + fTexWidth;
  const float fTexCoordV1 = fTexCoordV0 + fTexHeight;

  aeUInt32 uiVtx[4];

  // add the 4 vertices with different texcoords
  mv.m_vTexCoord.SetVector (fTexCoordU0, fTexCoordV0, fSize);
  mv.m_vTangent.SetVector (0, 0); // encode the billboard span factor into the tangent
  uiVtx[0] = Branch.m_LODs[lod].m_SubMesh[aeMeshType::Leaf].AddVertex (mv);
  
  mv.m_vTexCoord.SetVector (fTexCoordU1, fTexCoordV0, fSize);
  mv.m_vTangent.SetVector (1, 0); // encode the billboard span factor into the tangent
  uiVtx[1] = Branch.m_LODs[lod].m_SubMesh[aeMeshType::Leaf].AddVertex (mv);

  mv.m_vTexCoord.SetVector (fTexCoordU1, fTexCoordV1, fSize);
  mv.m_vTangent.SetVector (1, 1); // encode the billboard span factor into the tangent
  uiVtx[2] = Branch.m_LODs[lod].m_SubMesh[aeMeshType::Leaf].AddVertex (mv);

  mv.m_vTexCoord.SetVector (fTexCoordU0, fTexCoordV1, fSize);
  mv.m_vTangent.SetVector (0, 1); // encode the billboard span factor into the tangent
  uiVtx[3] = Branch.m_LODs[lod].m_SubMesh[aeMeshType::Leaf].AddVertex (mv);

  // add the two triangles
  t.m_uiVertexIDs[0] = uiVtx[0];
  t.m_uiVertexIDs[1] = uiVtx[1];
  t.m_uiVertexIDs[2] = uiVtx[2];
  Branch.m_LODs[lod].m_SubMesh[aeMeshType::Leaf].m_Triangles.push_back (t);

  t.m_uiVertexIDs[0] = uiVtx[0];
  t.m_uiVertexIDs[1] = uiVtx[2];
  t.m_uiVertexIDs[2] = uiVtx[3];
  Branch.m_LODs[lod].m_SubMesh[aeMeshType::Leaf].m_Triangles.push_back (t);
}

void aeTree::AddStaticLeaf (aeUInt32 uiBranch, aeLod::Enum lod, aeUInt32 uiNodeID, float fSize, const aeTreeMaterial* pMaterial)
{
  if (fSize < 0.05f)
    return;

  AE_CHECK_DEV (uiNodeID > 0, "Cannot spawn leaves at node zero.");

  aeBranch& Branch = m_Branches[uiBranch];
  const aeSpawnNodeDesc& bd = g_Tree.m_Descriptor.m_BranchTypes[Branch.m_Type];

  const aeVec3 vPos = Branch.m_Nodes[uiNodeID].m_vPosition;
  const aeVec3 vPrevPos = Branch.m_Nodes[uiNodeID - 1].m_vPosition;
  const aeVec3 vDir = (vPos - vPrevPos).GetNormalized ();

  aeVec3 vRight = vDir.GetOrthogonalVector ().GetNormalized ();
  aeVec3 vUp = vDir.Cross (vRight).GetNormalized ();

  aeMeshTriangle t;
  t.m_uiPickingID = Branch.m_uiPickID;
  t.m_uiPickingSubID = 0;

  aeMeshVertex mv;
  mv.m_uiColorVariation = (aeUInt8) aeMath::Rand (255);
  mv.m_vNormal = vDir;
  mv.m_vTangent = vRight;
  mv.m_vBiTangent = vUp;

  vRight *= fSize;
  vUp *= fSize;

  const aeInt32 iTexX = aeMath::Rand (pMaterial->m_uiTilingX);
  const aeInt32 iTexY = aeMath::Rand (pMaterial->m_uiTilingX);

  const float fTexWidth  = 1.0f / pMaterial->m_uiTilingX;
  const float fTexHeight = 1.0f / pMaterial->m_uiTilingY;

  const float fTexCoordU0 = fTexWidth * iTexX;
  const float fTexCoordV0 = fTexHeight * iTexY;

  const float fTexCoordU1 = fTexCoordU0 + fTexWidth;
  const float fTexCoordV1 = fTexCoordV0 + fTexHeight;

  aeUInt32 uiVtx[4];

  // add the 4 vertices with different texcoords
  mv.m_vTexCoord.SetVector (fTexCoordU0, fTexCoordV0, 1);
  mv.m_vPosition = vPos - vRight - vUp;
  mv.m_vTangent.SetVector (0, 0); // encode the billboard span factor into the tangent
  uiVtx[0] = Branch.m_LODs[lod].m_SubMesh[aeMeshType::Leaf].AddVertex (mv);
  
  mv.m_vTexCoord.SetVector (fTexCoordU1, fTexCoordV0, 1);
  mv.m_vPosition = vPos + vRight - vUp;
  mv.m_vTangent.SetVector (1, 0); // encode the billboard span factor into the tangent
  uiVtx[1] = Branch.m_LODs[lod].m_SubMesh[aeMeshType::Leaf].AddVertex (mv);

  mv.m_vTexCoord.SetVector (fTexCoordU1, fTexCoordV1, 1);
  mv.m_vPosition = vPos + vRight + vUp;
  mv.m_vTangent.SetVector (1, 1); // encode the billboard span factor into the tangent
  uiVtx[2] = Branch.m_LODs[lod].m_SubMesh[aeMeshType::Leaf].AddVertex (mv);

  mv.m_vTexCoord.SetVector (fTexCoordU0, fTexCoordV1, 1);
  mv.m_vPosition = vPos - vRight + vUp;
  mv.m_vTangent.SetVector (0, 1); // encode the billboard span factor into the tangent
  uiVtx[3] = Branch.m_LODs[lod].m_SubMesh[aeMeshType::Leaf].AddVertex (mv);

  // add the two triangles
  t.m_uiVertexIDs[0] = uiVtx[0];
  t.m_uiVertexIDs[1] = uiVtx[1];
  t.m_uiVertexIDs[2] = uiVtx[2];
  Branch.m_LODs[lod].m_SubMesh[aeMeshType::Leaf].m_Triangles.push_back (t);

  t.m_uiVertexIDs[0] = uiVtx[0];
  t.m_uiVertexIDs[1] = uiVtx[2];
  t.m_uiVertexIDs[2] = uiVtx[3];
  Branch.m_LODs[lod].m_SubMesh[aeMeshType::Leaf].m_Triangles.push_back (t);
}

void aeTree::GenerateLeafMesh (aeUInt32 uiBranch, aeLod::Enum lod)
{
  aeBranch& Branch = m_Branches[uiBranch];
  const aeSpawnNodeDesc& bd = g_Tree.m_Descriptor.m_BranchTypes[Branch.m_Type];

  if (!m_Descriptor.m_LodData[lod].m_AllowTypes[aeMeshType::Leaf].IsAnyFlagSet (1 << Branch.m_Type))
    return;

  if (Branch.IsDeleted ())
    return;
  if (Branch.m_LODs[lod].m_NodeIDs.size () < 2)
    return;

  const aeUInt32 uiMaxNodes = Branch.m_Nodes.size () - 1;
  aeUInt32 uiNodeID = uiMaxNodes;
  
  aeDeque<aeUInt32> Positions;
  Positions.push_back (uiNodeID);

  float fDistanceGone = 0.0f;

  if (bd.m_fLeafInterval > 0)
  {
    while (uiNodeID > 1)
    {
      fDistanceGone = 0.0f;
      
      while ((uiNodeID > 1) && (fDistanceGone < bd.m_fLeafInterval))
      {
        aeVec3 vPos1 = Branch.m_Nodes[uiNodeID].m_vPosition;
        aeVec3 vPos2 = Branch.m_Nodes[uiNodeID - 1].m_vPosition;

        fDistanceGone += (vPos1 - vPos2).GetLength ();

        --uiNodeID;
      }

      if (uiNodeID > 1)
        Positions.push_back (uiNodeID);
    }

    if ((fDistanceGone < bd.m_fLeafInterval / 0.5f) && (Positions.size () > 1))
      Positions.pop_back ();
  }

  aeMath::SetRandSeed (Branch.m_RandomData.m_FrondColorVariationRD);

  const aeTreeMaterial* pMaterial = aeTreeMaterialLibrary::GetOrAddMaterial (bd.m_sTexture[aeMeshType::Leaf].c_str ());

  for (aeUInt32 i = 0; i < Positions.size (); ++i)
  {
    if (bd.m_bBillboardLeaves)
      AddBillbardLeaf (uiBranch, lod, Branch.m_Nodes[Positions[i]].m_vPosition, bd.m_fLeafSize * bd.m_LeafScale.GetValueAt (Positions[i] / (float) uiMaxNodes), pMaterial);
    else
      AddStaticLeaf (uiBranch, lod, Positions[i], bd.m_fLeafSize * bd.m_LeafScale.GetValueAt (Positions[i] / (float) uiMaxNodes), pMaterial);
  }
}

