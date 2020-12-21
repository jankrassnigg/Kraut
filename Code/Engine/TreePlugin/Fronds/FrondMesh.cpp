#include "PCH.h"

#include "../Tree/Tree.h"
#include <KrautFoundation/Math/Plane.h>


using namespace AE_NS_GRAPHICS;

aeUInt32 AddVertex (aeUInt32 x, aeUInt32 uiWidth, aeUInt32 y, aeSubMesh& out_Tris, aeUInt32 uiFirstVertex, const aeMeshVertex& vtx)
{
  aeUInt32 uiSupposedIndex = uiFirstVertex + (y * uiWidth) + x;

  if (out_Tris.m_Vertices.size () <= uiSupposedIndex)
  {
    out_Tris.m_Vertices.resize (uiSupposedIndex + 1);
    out_Tris.m_Vertices[uiSupposedIndex] = vtx;
  }
  else
  {
    if (out_Tris.m_Vertices[uiSupposedIndex].m_vPosition.IsZeroVector ())
      out_Tris.m_Vertices[uiSupposedIndex] = vtx;
    else
    {
      out_Tris.m_Vertices[uiSupposedIndex].m_vNormal += vtx.m_vNormal;
      out_Tris.m_Vertices[uiSupposedIndex].m_vBiTangent += vtx.m_vBiTangent;
    }
  }

  return uiSupposedIndex;
}

static void AddFrondQuads (aeUInt32 uiSlices, aeUInt32 uiNode, const aeArray<aeVec3>& Positions, const aeArray<float>& PosAlongBranch, const aeArray<aeVec3>& UpVectors, const aeArray<aeVec3>& OrthoVectors, const aeArray<float>& NodeWidth, const aeArray<float>& NodeHeight, const aeCurve& Contour, aeUInt32 uiPickingID, aeSubMesh& out_Tris, float fSide, aeUInt32 uiFirstVertex, aeSpawnNodeDesc::FrondContourMode FrondMode, aeBranch& Branch, aeUInt32 lod, const aeSpawnNodeDesc& bnd, float fFrondFract, float fTextureRepeatDivider, float fBranchLength, const aeTreeMaterial* pMaterial, aeInt32 iCurFrondIndex)
{
  const float fColorVariation = Branch.m_fFrondColorVariation;

  bool bAddPoly0 = true;
  bool bAddPoly1 = true;

  // if the frond is not wide enough at this vertex, cut it off
  if (NodeWidth[uiNode] < 0.01f)
    bAddPoly0 = false;
  if (NodeWidth[uiNode+1] < 0.01f)
    bAddPoly1 = false;

  if (!bAddPoly0 || !bAddPoly1)
    return;

  // this is used for the texture atlas feature to offset the texcoords
  const float fTextureWidth  = 1.0f / pMaterial->m_uiTilingX;
  const float fTextureOffset = fTextureWidth * ((Branch.m_uiFrondTextureVariation + iCurFrondIndex) % pMaterial->m_uiTilingX);


  const aeVec3 vGrowDir = (Positions[uiNode+1] - Positions[uiNode]).GetNormalized ();

  const float fAlongBranch0 = PosAlongBranch[uiNode];
  const float fAlongBranch1 = PosAlongBranch[uiNode + 1];

  const float fTexCoord0 = fAlongBranch0 / fTextureRepeatDivider;
  const float fTexCoord1 = fAlongBranch1 / fTextureRepeatDivider;

  const aeVec3 vStartPos0 = Positions[uiNode];
  const aeVec3 vStartPos1 = Positions[uiNode + 1];

  float fContourCenter = 0;
  
  if (FrondMode == aeSpawnNodeDesc::Full)
    fContourCenter = Contour.GetValueAt (0.5f);
  else
    fContourCenter = Contour.GetValueAt (0.0f);

  if ((FrondMode == aeSpawnNodeDesc::InverseSymetric) && (fSide < 0.0f))
    fContourCenter = Contour.m_fMaxValue - fContourCenter;

  aeMeshTriangle t;
  aeMeshVertex vtx[3];

  for (int i = 0; i < 3; ++i)
    vtx[i].m_uiColorVariation = (aeUInt8) (fColorVariation * 255);

  t.m_uiPickingID = uiPickingID;
  t.m_uiPickingSubID = 0;//uiNode;

  aeVec3 vOffset0 (0.0f);
  aeVec3 vOffset1 (0.0f);

  if (bnd.m_bAlignFrondsOnSurface)
  {
    // thickness at those nodes
    const float fRadius0 = Branch.m_Nodes[Branch.m_LODs[lod].m_NodeIDs[uiNode + 0]].m_fThickness * 0.5f;
    const float fRadius1 = Branch.m_Nodes[Branch.m_LODs[lod].m_NodeIDs[uiNode + 1]].m_fThickness * 0.5f;

    // the flare width along the branch there
    const float fFlareWidth0 = bnd.GetFlareWidthAt(fAlongBranch0 / fBranchLength, fRadius0);
    const float fFlareWidth1 = bnd.GetFlareWidthAt(fAlongBranch1 / fBranchLength, fRadius1);

    // compute the 'angle' around the branch at that position (each frond has two angles)
    fFrondFract /= 2.0f;
    if (fSide > 0)
      fFrondFract += 0.5f;

    // get the actual distances of the flare mesh at those angles
    const float fFlareDistance0 = bnd.GetFlareDistance (fAlongBranch0 / fBranchLength, fRadius0, fFlareWidth0, fFrondFract);
    const float fFlareDistance1 = bnd.GetFlareDistance (fAlongBranch1 / fBranchLength, fRadius1, fFlareWidth1, fFrondFract);

    // compute the offset
    vOffset0 = fSide * OrthoVectors[uiNode + 0] * fFlareDistance0;
    vOffset1 = fSide * OrthoVectors[uiNode + 1] * fFlareDistance1;

    // reduce the offset to prevent holes due to tesselation/interpolation
    vOffset0 -= vOffset0.GetNormalized () * 0.01f;
    vOffset1 -= vOffset1.GetNormalized () * 0.01f;
  }

  for (aeUInt32 slice = 0; slice < uiSlices; ++slice)
  {
    const float fPos0 = (float) slice       / (float) uiSlices;
    const float fPos1 = (float) (slice + 1) / (float) uiSlices;

    float fContour0 = 0;(Contour.GetValueAt (0.5f + fSide * fPos0 * 0.5f) - fContourCenter);
    float fContour1 = 0;(Contour.GetValueAt (0.5f + fSide * fPos1 * 0.5f) - fContourCenter);

    if (FrondMode == aeSpawnNodeDesc::Full)
    {
      fContour0 = Contour.GetValueAt (0.5f + fSide * fPos0 * 0.5f);
      fContour1 = Contour.GetValueAt (0.5f + fSide * fPos1 * 0.5f);
    }
    else
    {
      fContour0 = Contour.GetValueAt (fPos0);
      fContour1 = Contour.GetValueAt (fPos1);
    }

    if ((FrondMode == aeSpawnNodeDesc::InverseSymetric) && (fSide < 0.0f))
    {
      fContour0 = Contour.m_fMaxValue - fContour0;
      fContour1 = Contour.m_fMaxValue - fContour1;
    }

    fContour0 -= fContourCenter;
    fContour1 -= fContourCenter;


    aeVec3 vCurPos00 = vOffset0 + vStartPos0 + fSide * OrthoVectors[uiNode]   * NodeWidth[uiNode]   * fPos0 + UpVectors[uiNode]   * fContour0 * NodeHeight[uiNode];
    aeVec3 vCurPos01 = vOffset0 + vStartPos0 + fSide * OrthoVectors[uiNode]   * NodeWidth[uiNode]   * fPos1 + UpVectors[uiNode]   * fContour1 * NodeHeight[uiNode];
    aeVec3 vCurPos10 = vOffset1 + vStartPos1 + fSide * OrthoVectors[uiNode+1] * NodeWidth[uiNode+1] * fPos0 + UpVectors[uiNode+1] * fContour0 * NodeHeight[uiNode+1];
    aeVec3 vCurPos11 = vOffset1 + vStartPos1 + fSide * OrthoVectors[uiNode+1] * NodeWidth[uiNode+1] * fPos1 + UpVectors[uiNode+1] * fContour1 * NodeHeight[uiNode+1];

    const aeVec3 vBiTangent0 = (vCurPos10 - vCurPos00).GetNormalized ();
    const aeVec3 vBiTangent1 = (vCurPos11 - vCurPos01).GetNormalized ();

    const float fW0 = NodeWidth[uiNode]   * 1;
    const float fW1 = NodeWidth[uiNode+1] * 1;

    if (bAddPoly0)
    {
      vtx[0].m_vPosition = vCurPos00;
      vtx[1].m_vPosition = vCurPos01;
      vtx[2].m_vPosition = vCurPos10;

      aePlane p (vtx[0].m_vPosition, vtx[1].m_vPosition, vtx[2].m_vPosition);

      if (fSide < 0)
        p.FlipPlane ();

      vtx[0].m_vNormal = p.m_vNormal;
      vtx[1].m_vNormal = p.m_vNormal;
      vtx[2].m_vNormal = p.m_vNormal;

      vtx[0].m_vTexCoord.x = (fTextureOffset + (0.5f + fSide * fPos0 * 0.5f) * fTextureWidth) * fW0;
      vtx[1].m_vTexCoord.x = (fTextureOffset + (0.5f + fSide * fPos1 * 0.5f) * fTextureWidth) * fW0;
      vtx[2].m_vTexCoord.x = (fTextureOffset + (0.5f + fSide * fPos0 * 0.5f) * fTextureWidth) * fW1;

      vtx[0].m_vTexCoord.y = fTexCoord0 * fW0;
      vtx[1].m_vTexCoord.y = fTexCoord0 * fW0;
      vtx[2].m_vTexCoord.y = fTexCoord1 * fW1;

      vtx[0].m_vTexCoord.z = fW0;
      vtx[1].m_vTexCoord.z = fW0;
      vtx[2].m_vTexCoord.z = fW1;

      vtx[0].m_vBiTangent = vBiTangent0;
      vtx[1].m_vBiTangent = vBiTangent1;
      vtx[2].m_vBiTangent = vBiTangent0;

      t.m_uiVertexIDs[0] = AddVertex (slice+0, uiSlices + 1, uiNode+0, out_Tris, uiFirstVertex, vtx[0]);
      t.m_uiVertexIDs[1] = AddVertex (slice+1, uiSlices + 1, uiNode+0, out_Tris, uiFirstVertex, vtx[1]);
      t.m_uiVertexIDs[2] = AddVertex (slice+0, uiSlices + 1, uiNode+1, out_Tris, uiFirstVertex, vtx[2]);

      if (fSide < 0)
        t.Flip ();

      out_Tris.m_Triangles.push_back (t);
    }

    if (bAddPoly1)
    {
      vtx[0].m_vPosition = vCurPos01;
      vtx[1].m_vPosition = vCurPos11;
      vtx[2].m_vPosition = vCurPos10;

      aePlane p (vtx[0].m_vPosition, vtx[1].m_vPosition, vtx[2].m_vPosition);

      if (fSide < 0)
        p.FlipPlane ();

      vtx[0].m_vNormal = p.m_vNormal;
      vtx[1].m_vNormal = p.m_vNormal;
      vtx[2].m_vNormal = p.m_vNormal;

      vtx[0].m_vTexCoord.x = (fTextureOffset + (0.5f + fSide * fPos1 * 0.5f) * fTextureWidth) * fW0;
      vtx[1].m_vTexCoord.x = (fTextureOffset + (0.5f + fSide * fPos1 * 0.5f) * fTextureWidth) * fW1;
      vtx[2].m_vTexCoord.x = (fTextureOffset + (0.5f + fSide * fPos0 * 0.5f) * fTextureWidth) * fW1;
      
      vtx[0].m_vTexCoord.y = fTexCoord0 * fW0;
      vtx[1].m_vTexCoord.y = fTexCoord1 * fW1;
      vtx[2].m_vTexCoord.y = fTexCoord1 * fW1;

      vtx[0].m_vTexCoord.z = fW0;
      vtx[1].m_vTexCoord.z = fW1;
      vtx[2].m_vTexCoord.z = fW1;

      vtx[0].m_vBiTangent = vBiTangent1;
      vtx[1].m_vBiTangent = vBiTangent0;
      vtx[2].m_vBiTangent = vBiTangent1;

      t.m_uiVertexIDs[0] = AddVertex (slice+1, uiSlices + 1, uiNode+0, out_Tris, uiFirstVertex, vtx[0]);
      t.m_uiVertexIDs[1] = AddVertex (slice+1, uiSlices + 1, uiNode+1, out_Tris, uiFirstVertex, vtx[1]);
      t.m_uiVertexIDs[2] = AddVertex (slice+0, uiSlices + 1, uiNode+1, out_Tris, uiFirstVertex, vtx[2]);

      if (fSide < 0)
        t.Flip ();

      out_Tris.m_Triangles.push_back (t);
    }
  }
}



static void AddSingleFrond (aeUInt32 uiNode, const aeArray<aeVec3>& Positions, const aeArray<float>& PosAlongBranch, const aeArray<aeVec3>& UpVectors, const aeArray<aeVec3>& OrthoVectors, const aeArray<float>& NodeWidth, aeUInt32 uiPickingID, aeSubMesh& out_Tris, aeUInt32 uiFirstVertex, aeBranch& Branch, float fTextureRepeatDivider, const aeTreeMaterial* pMaterial, aeInt32 iCurFrondIndex)
{
  const float fColorVariation = Branch.m_fFrondColorVariation;

  // if the frond is not wide enough at this vertex, cut it off
  if ((NodeWidth[uiNode] < 0.05f) || (NodeWidth[uiNode+1] < 0.05f))
    return;

  // this is used for the texture atlas feature to offset the texcoords
  const float fTextureWidth  = 1.0f / pMaterial->m_uiTilingX;
  const float fTextureOffset = fTextureWidth * ((Branch.m_uiFrondTextureVariation + iCurFrondIndex) % pMaterial->m_uiTilingX);


  const aeVec3 vGrowDir = (Positions[uiNode+1] - Positions[uiNode]).GetNormalized ();

  const float fAlongBranch0 = PosAlongBranch[uiNode];
  const float fAlongBranch1 = PosAlongBranch[uiNode + 1];

  const float fTexCoord0 = fAlongBranch0 / fTextureRepeatDivider;
  const float fTexCoord1 = fAlongBranch1 / fTextureRepeatDivider;

  const aeVec3 vStartPos0 = Positions[uiNode];
  const aeVec3 vStartPos1 = Positions[uiNode + 1];

  aeMeshTriangle t;
  aeMeshVertex vtx[3];

  for (int i = 0; i < 3; ++i)
    vtx[i].m_uiColorVariation = (aeUInt8) (fColorVariation * 255);

  t.m_uiPickingID = uiPickingID;
  t.m_uiPickingSubID = 0;//uiNode;

  {
    aeVec3 vCurPos00 = vStartPos0 - OrthoVectors[uiNode]   * NodeWidth[uiNode];
    aeVec3 vCurPos01 = vStartPos0 + OrthoVectors[uiNode]   * NodeWidth[uiNode];
    aeVec3 vCurPos10 = vStartPos1 - OrthoVectors[uiNode+1] * NodeWidth[uiNode+1];
    aeVec3 vCurPos11 = vStartPos1 + OrthoVectors[uiNode+1] * NodeWidth[uiNode+1];

    float fWidth0 = (vCurPos01 - vCurPos00).GetLength ();
    float fWidth1 = (vCurPos11 - vCurPos10).GetLength ();

    const aeVec3 vBiTangent0 = (vCurPos10 - vCurPos00).GetNormalized ();
    const aeVec3 vBiTangent1 = (vCurPos11 - vCurPos01).GetNormalized ();

    {
      vtx[0].m_vPosition = vCurPos00;
      vtx[1].m_vPosition = vCurPos01;
      vtx[2].m_vPosition = vCurPos10;

      aePlane p (vtx[0].m_vPosition, vtx[1].m_vPosition, vtx[2].m_vPosition);

      vtx[0].m_vNormal = p.m_vNormal;
      vtx[1].m_vNormal = p.m_vNormal;
      vtx[2].m_vNormal = p.m_vNormal;

      vtx[0].m_vTexCoord.x = (fTextureOffset + 0.0f * fTextureWidth) * fWidth0;
      vtx[1].m_vTexCoord.x = (fTextureOffset + 1.0f * fTextureWidth) * fWidth0;
      vtx[2].m_vTexCoord.x = (fTextureOffset + 0.0f * fTextureWidth) * fWidth1;

      vtx[0].m_vTexCoord.y = fTexCoord0 * fWidth0;
      vtx[1].m_vTexCoord.y = fTexCoord0 * fWidth0;
      vtx[2].m_vTexCoord.y = fTexCoord1 * fWidth1;

      vtx[0].m_vTexCoord.z = fWidth0;
      vtx[1].m_vTexCoord.z = fWidth0;
      vtx[2].m_vTexCoord.z = fWidth1;

      vtx[0].m_vBiTangent = vBiTangent0;
      vtx[1].m_vBiTangent = vBiTangent1;
      vtx[2].m_vBiTangent = vBiTangent0;

      t.m_uiVertexIDs[0] = AddVertex (0+0, 2, uiNode+0, out_Tris, uiFirstVertex, vtx[0]);
      t.m_uiVertexIDs[1] = AddVertex (0+1, 2, uiNode+0, out_Tris, uiFirstVertex, vtx[1]);
      t.m_uiVertexIDs[2] = AddVertex (0+0, 2, uiNode+1, out_Tris, uiFirstVertex, vtx[2]);

      out_Tris.m_Triangles.push_back (t);
    }

    {
      vtx[0].m_vPosition = vCurPos01;
      vtx[1].m_vPosition = vCurPos11;
      vtx[2].m_vPosition = vCurPos10;

      aePlane p (vtx[0].m_vPosition, vtx[1].m_vPosition, vtx[2].m_vPosition);

      vtx[0].m_vNormal = p.m_vNormal;
      vtx[1].m_vNormal = p.m_vNormal;
      vtx[2].m_vNormal = p.m_vNormal;

      vtx[0].m_vTexCoord.x = (fTextureOffset + 1.0f * fTextureWidth) * fWidth0;
      vtx[1].m_vTexCoord.x = (fTextureOffset + 1.0f * fTextureWidth) * fWidth1;
      vtx[2].m_vTexCoord.x = (fTextureOffset + 0.0f * fTextureWidth) * fWidth1;
      
      vtx[0].m_vTexCoord.y = fTexCoord0 * fWidth0;
      vtx[1].m_vTexCoord.y = fTexCoord1 * fWidth1;
      vtx[2].m_vTexCoord.y = fTexCoord1 * fWidth1;

      vtx[0].m_vTexCoord.z = fWidth0;
      vtx[1].m_vTexCoord.z = fWidth1;
      vtx[2].m_vTexCoord.z = fWidth1;

      vtx[0].m_vBiTangent = vBiTangent1;
      vtx[1].m_vBiTangent = vBiTangent0;
      vtx[2].m_vBiTangent = vBiTangent1;

      t.m_uiVertexIDs[0] = AddVertex (0+1, 2, uiNode+0, out_Tris, uiFirstVertex, vtx[0]);
      t.m_uiVertexIDs[1] = AddVertex (0+1, 2, uiNode+1, out_Tris, uiFirstVertex, vtx[1]);
      t.m_uiVertexIDs[2] = AddVertex (0+0, 2, uiNode+1, out_Tris, uiFirstVertex, vtx[2]);

      out_Tris.m_Triangles.push_back (t);
    }
  }
}


void aeTree::GenerateFrondMesh (aeUInt32 uiBranch, aeLod::Enum lod)
{
  aeBranch& Branch = m_Branches[uiBranch];
  const aeSpawnNodeDesc& bd = g_Tree.m_Descriptor.m_BranchTypes[Branch.m_Type];

  if (Branch.IsDeleted ())
    return;
  if (Branch.m_LODs[lod].m_NodeIDs.size () < 2)
    return;

  const aeVec3 vStartGrowDir = (Branch.m_Nodes[1].m_vPosition - Branch.m_Nodes[0].m_vPosition).GetNormalized ();

  aeQuaternion qRot;
  qRot.CreateQuaternion (vStartGrowDir, 180.0f / bd.m_uiNumFronds);

  aeVec3 vUp = Branch.m_vLeafUpDirection.GetNormalized ();

  for (aeUInt32 f = 0; f < bd.m_uiNumFronds; ++f)
  {
    GenerateFrondMesh (uiBranch, lod, vUp, f);

    vUp = qRot * vUp;
  }
}

void aeTree::GenerateFrondMesh (aeUInt32 uiBranch, aeLod::Enum lod, const aeVec3& vStartUpDirection, aeInt32 iCurFrondIndex)
{
  aeBranch& Branch = m_Branches[uiBranch];
  const aeSpawnNodeDesc& bd = g_Tree.m_Descriptor.m_BranchTypes[Branch.m_Type];

  if (Branch.IsDeleted ())
    return;

  if (Branch.m_LODs[lod].m_NodeIDs.size () < 2)
    return;

  if (!m_Descriptor.m_LodData[lod].m_AllowTypes[aeMeshType::Frond].IsAnyFlagSet (1 << Branch.m_Type))
    return;

  const float fFrondFract = (float) iCurFrondIndex / (float) bd.m_uiNumFronds;

  const aeUInt32 uiNodeCount = Branch.m_LODs[lod].m_NodeIDs.size ();

  float fLodBranchLength = 0.0f;
  {
    for (aeUInt32 i = 1; i < uiNodeCount; ++i)
    {
      const aeVec3 v0 = Branch.m_Nodes[Branch.m_LODs[lod].m_NodeIDs[i - 1]].m_vPosition;
      const aeVec3 v1 = Branch.m_Nodes[Branch.m_LODs[lod].m_NodeIDs[i - 0]].m_vPosition;

      fLodBranchLength += (v1 - v0).GetLength ();
    }
  }

  const aeVec3 vStartGrowDir = (Branch.m_Nodes.back().m_vPosition - Branch.m_Nodes[0].m_vPosition).GetNormalized ();
  const aeVec3 vStartOrtho  = vStartGrowDir.Cross (vStartUpDirection).GetNormalized ();
  

  aeArray<aeVec3> Positions (uiNodeCount);
  aeArray<aeVec3> UpVectors (uiNodeCount);
  aeArray<aeVec3> OrthoVectors (uiNodeCount);
  aeArray<float> NodeWidth (uiNodeCount);
  aeArray<float> NodeHeight (uiNodeCount);
  aeArray<float> PosAlongBranch (uiNodeCount);
  float fBranchLength = 0.0f;

  // precompute all the node up vectors
  {
    aeVec3 vLastGrowDir = vStartGrowDir;
    UpVectors[0] = vStartOrtho.Cross (vStartGrowDir).GetNormalized ();
    OrthoVectors[0] = vStartOrtho;

    for (aeUInt32 i = 1; i < uiNodeCount - 1; ++i)
    {
      const aeVec3 vCurGrowDir = (Branch.m_Nodes[Branch.m_LODs[lod].m_NodeIDs[i+1]].m_vPosition - Branch.m_Nodes[Branch.m_LODs[lod].m_NodeIDs[i]].m_vPosition).GetNormalized ();

      aeQuaternion rot;
      rot.CreateQuaternion (vLastGrowDir, vCurGrowDir);

      // rotate the up vector by the amount of change in the frond direction
      UpVectors[i] = rot * UpVectors[i - 1];
      OrthoVectors[i] = rot * OrthoVectors[i - 1];

      vLastGrowDir = vCurGrowDir;
    }

    // copy the last computed value
    UpVectors.back () = UpVectors[UpVectors.size () - 2];
    OrthoVectors.back () = OrthoVectors[OrthoVectors.size () - 2];
  }

  // now smooth all up vectors
  {
    for (aeUInt32 i = 1; i < uiNodeCount - 1; ++i)
    {
      UpVectors[i] = (UpVectors[i - 1] + UpVectors[i]).GetNormalized ();
      OrthoVectors[i] = (OrthoVectors[i - 1] + OrthoVectors[i]).GetNormalized ();
    }
  }

  // now compute the width and height for all nodes
  {
    for (aeUInt32 i = 0; i < uiNodeCount; ++i)
    {
      const float fPosAlongBranch = (float) i / (float) (uiNodeCount-1);

      Positions[i] = Branch.m_Nodes[Branch.m_LODs[lod].m_NodeIDs[i]].m_vPosition;
      NodeWidth[i] = aeMath::Max (0.01f, bd.m_fFrondWidth * bd.m_FrondWidth.GetValueAt  (fPosAlongBranch));
      NodeHeight[i]= bd.m_fFrondHeight * bd.m_FrondHeight.GetValueAt (fPosAlongBranch);
    }
  }

  float fTextureRepeatDivider = bd.m_fTextureRepeat;

  // compute the branch length (in this LOD)
  {
    for (aeUInt32 i = 1; i < uiNodeCount; ++i)
    {
      fBranchLength += (Positions[i - 1] - Positions[i]).GetLength ();
      PosAlongBranch[i] = fBranchLength;
    }

    PosAlongBranch[0] = 0.0f;

    if (bd.m_fTextureRepeat <= 0.01f)
      fTextureRepeatDivider = fBranchLength;
  }


  aeUInt32 uiFirstVertex0 = Branch.m_LODs[lod].m_SubMesh[aeMeshType::Frond].m_Vertices.size ();

  aeInt32 iFrondDetail = bd.m_bAlignFrondsOnSurface ? aeMath::Max<aeInt32> (1, bd.m_uiFrondDetail) : bd.m_uiFrondDetail;
  iFrondDetail -= m_Descriptor.m_LodData[lod].m_iFrondDetailReduction;
  iFrondDetail = aeMath::Clamp<aeInt32> (iFrondDetail, 0, m_Descriptor.m_LodData[lod].m_iMaxFrondDetail);

  // use the width value of the second last node also for the last node to prevent extreme texture stretching
  NodeWidth.back () = NodeWidth[NodeWidth.size () - 2];

  const aeTreeMaterial* pMaterial = aeTreeMaterialLibrary::GetOrAddMaterial (bd.m_sTexture[aeMeshType::Frond].c_str ());

  // now generate all the fronds
  if (iFrondDetail > 0)
  {
    for (aeUInt32 i = 0; i < uiNodeCount - 1; ++i)
      AddFrondQuads (iFrondDetail, i, Positions, PosAlongBranch, UpVectors, OrthoVectors, NodeWidth, NodeHeight, bd.m_FrondContour, Branch.m_uiPickID, Branch.m_LODs[lod].m_SubMesh[aeMeshType::Frond],  1.0f, uiFirstVertex0, bd.m_FrondContourMode, Branch, lod, bd, fFrondFract, fTextureRepeatDivider, fBranchLength, pMaterial, iCurFrondIndex);

    aeUInt32 uiFirstVertex1 = Branch.m_LODs[lod].m_SubMesh[aeMeshType::Frond].m_Vertices.size ();
    for (aeUInt32 i = 0; i < uiNodeCount - 1; ++i)
      AddFrondQuads (iFrondDetail, i, Positions, PosAlongBranch, UpVectors, OrthoVectors, NodeWidth, NodeHeight, bd.m_FrondContour, Branch.m_uiPickID, Branch.m_LODs[lod].m_SubMesh[aeMeshType::Frond], -1.0f, uiFirstVertex1, bd.m_FrondContourMode, Branch, lod, bd, fFrondFract, fTextureRepeatDivider, fBranchLength, pMaterial, iCurFrondIndex);
  }
  else
  {
    for (aeUInt32 i = 0; i < uiNodeCount - 1; ++i)
      AddSingleFrond (i, Positions, PosAlongBranch, UpVectors, OrthoVectors, NodeWidth, Branch.m_uiPickID, Branch.m_LODs[lod].m_SubMesh[aeMeshType::Frond], uiFirstVertex0, Branch, fTextureRepeatDivider, pMaterial, iCurFrondIndex);
  }

  for (aeUInt32 v = uiFirstVertex0; v < Branch.m_LODs[lod].m_SubMesh[aeMeshType::Frond].m_Vertices.size (); ++v)
  {
    Branch.m_LODs[lod].m_SubMesh[aeMeshType::Frond].m_Vertices[v].m_vNormal.Normalize ();
    Branch.m_LODs[lod].m_SubMesh[aeMeshType::Frond].m_Vertices[v].m_vBiTangent.Normalize ();

    Branch.m_LODs[lod].m_SubMesh[aeMeshType::Frond].m_Vertices[v].m_vTangent = 
      Branch.m_LODs[lod].m_SubMesh[aeMeshType::Frond].m_Vertices[v].m_vBiTangent.Cross (Branch.m_LODs[lod].m_SubMesh[aeMeshType::Frond].m_Vertices[v].m_vNormal);
  }
}


