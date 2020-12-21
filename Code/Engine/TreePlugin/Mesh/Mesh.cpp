#include "PCH.h"

#include "../Basics/Plugin.h"
#include "../Tree/Tree.h"
#include <KrautFoundation/Math/IncludeAll.h>


static float ComputeVertexRingCircumference (const aeSpawnNodeDesc& bnd, aeUInt32 uiFlares, float fMinWidth, float fMaxWidth, float fPosAlongBranch)
{
  const float fCF = fMinWidth * aeMath::PI () * 2.0f;

  if (uiFlares == 0)
    return fCF;

  const aeUInt32 uiMaxVertices = aeMath::Max<aeUInt32> (4, (aeUInt32) (fCF / 0.1f));

  aeVec3 vDir (aeMath::CosDeg (0), 0, aeMath::SinDeg (0));
  aeVec3 vLastPos = vDir * bnd.GetFlareDistance (fPosAlongBranch, fMinWidth, fMaxWidth, 0.0f);

  const float fAngleStep = 360.0f / uiMaxVertices;

  float fCircumference = 0.0f;

  for (aeUInt32 vert = 1; vert < uiMaxVertices; ++vert)
  {
    const float fDist = bnd.GetFlareDistance (fPosAlongBranch, fMinWidth, fMaxWidth, (float) vert / (float) uiMaxVertices);
    const aeVec3 vPos (aeMath::CosDeg (fAngleStep * vert) * fDist, 0, aeMath::SinDeg (fAngleStep * vert) * fDist);

    fCircumference += (vLastPos - vPos).GetLength ();
    vLastPos = vPos;
  }

  return fCircumference;
}

aeInt32 aeTree::GenerateVertexRing (aeBranch* pBranch, aeUInt32 uiNode, aeVertexRing* pVertexRing, aeLod::Enum lod, bool bTip, const aeVec3& vNormalAnchor) const
{
  const aeSpawnNodeDesc& bnd = m_Descriptor.m_BranchTypes[pBranch->m_Type];

  const aeBranchNode* pPrevNode = NULL;
  const aeBranchNode* pCurNode  = NULL;
  const aeBranchNode* pNextNode = NULL;

  if (!bTip)
  {
    aeInt32 iPrevNode = ((aeInt32) uiNode) - 1;
    aeInt32 iNextNode = ((aeInt32) uiNode) + 1;

    iPrevNode = aeMath::Max (iPrevNode, 0);

    if (lod != aeLod::None)
    {
      iNextNode = aeMath::Min (iNextNode, (aeInt32) (pBranch->m_LODs[lod].m_NodeIDs.size ()) - 1);

      iPrevNode = pBranch->m_LODs[lod].m_NodeIDs[iPrevNode];
      uiNode    = pBranch->m_LODs[lod].m_NodeIDs[uiNode];
      iNextNode = pBranch->m_LODs[lod].m_NodeIDs[iNextNode];
    }
    else
    {
      iNextNode = aeMath::Min (iNextNode, (aeInt32) (pBranch->m_Nodes.size ()) - 1);
    }

    pPrevNode = &pBranch->m_Nodes[iPrevNode];
    pCurNode  = &pBranch->m_Nodes[uiNode];
    pNextNode = &pBranch->m_Nodes[iNextNode];
  }
  else
  {
    aeInt32 iPrevNode = ((aeInt32) uiNode) - 1;
    aeInt32 iNextNode = ((aeInt32) uiNode) + 1;

    iNextNode = aeMath::Min (iNextNode, (aeInt32) (pBranch->m_LODs[lod].m_TipNodes.size ()) - 1);

    if (iPrevNode < 0)
      pPrevNode = &pBranch->m_Nodes[pBranch->m_LODs[lod].m_NodeIDs.back ()];
    else
      pPrevNode = &pBranch->m_LODs[lod].m_TipNodes[iPrevNode];

    pCurNode  = &pBranch->m_LODs[lod].m_TipNodes[uiNode];
    pNextNode = &pBranch->m_LODs[lod].m_TipNodes[iNextNode];
  }

  aeVec3 vDirPrev = pCurNode->m_vPosition - pPrevNode->m_vPosition;
  aeVec3 vDirNext = pNextNode->m_vPosition - pCurNode->m_vPosition;

  vDirPrev.NormalizeSafe ();
  vDirNext.NormalizeSafe ();

  const aeVec3 vBranchDir = (vDirPrev + vDirNext).GetNormalized ();


  // used to rotate vectors from the XZ plane to the actual branch node plane
  aeQuaternion q;
  q.CreateQuaternion (aeVec3 (0, 1, 0), vBranchDir);

  const float fBranchRadius = aeMath::Max (0.001f, pCurNode->m_fThickness / 2.0f);

  float fPosAlongBranch = 1.0f;
  if (!bTip)
    fPosAlongBranch = uiNode / (float) (pBranch->m_Nodes.size ()-1);

  const float fFlareWidth = bnd.GetFlareWidthAt (fPosAlongBranch, fBranchRadius);

  const float fCircumference = ComputeVertexRingCircumference (bnd, bnd.m_uiFlares, fBranchRadius, fFlareWidth, fPosAlongBranch);

  aeUInt32 uiVertices = aeMath::Clamp ((int) (fCircumference * 2.0f / m_Descriptor.m_LodData[lod].m_fVertexRingDetail), 3, 128);

  if (bTip)
    uiVertices = pPrevNode->m_iSegments;

  pVertexRing->m_Vertices.resize (uiVertices);
  pVertexRing->m_VertexIDs.resize (uiVertices);
  pVertexRing->m_Normals.resize (uiVertices);

  // compute the vertex positions (including flares)
  for (aeUInt32 i = 0; i < uiVertices; ++i)
  {
    float fAngle = (360.0f / uiVertices) * i;

    aeVec3 v (aeMath::CosDeg (fAngle), 0, aeMath::SinDeg (fAngle));

    float fNewThickness = bnd.GetFlareDistance (fPosAlongBranch, fBranchRadius, fFlareWidth, (float) i / (float) uiVertices);

    pVertexRing->m_Vertices[i]  = q * (v * fNewThickness) + pCurNode->m_vPosition;
    pVertexRing->m_VertexIDs[i] = -1;
  }

  aeUInt32 iPrevNode = uiVertices - 1;
  // compute the smooth normals
  for (aeUInt32 i = 0; i < uiVertices; ++i)
  {
    pVertexRing->m_Normals[i]  = pVertexRing->m_Vertices[i] - vNormalAnchor;
    pVertexRing->m_Normals[i].NormalizeSafe ();

    iPrevNode = i;
  }

  pVertexRing->m_fDiameter = 0.0f;

  aeVec3 vLast = pVertexRing->m_Vertices.back ();
  for (aeUInt32 i = 0; i < pVertexRing->m_Vertices.size (); ++i)
  {
    aeVec3 vCur = pVertexRing->m_Vertices[i];

    pVertexRing->m_fDiameter += (vCur - vLast).GetLength ();

    vLast = vCur;
  }

  return uiVertices;
}

void aeTree::GenerateCapTriangles (aeUInt32 uiBranch, aeLod::Enum lod, aeVertexRing& VertexRing)
{
  if (m_Branches[uiBranch].IsDeleted ())
    return;

  aePlane p (VertexRing.m_Vertices[0], VertexRing.m_Vertices[1], VertexRing.m_Vertices[2]);
  aeMeshTriangle tri;
  aeMeshVertex vtx[3];

  tri.m_uiPickingID = m_Branches[uiBranch].m_uiPickID;
  tri.m_uiPickingSubID = 0;

   vtx[0].m_vNormal = p.m_vNormal;
   vtx[1].m_vNormal = p.m_vNormal;
   vtx[2].m_vNormal = p.m_vNormal;

  for (aeUInt32 t = 0; t < VertexRing.m_Vertices.size () - 2; ++t)
  {
    vtx[0].m_vPosition = VertexRing.m_Vertices[0];
    vtx[1].m_vPosition = VertexRing.m_Vertices[t + 1];
    vtx[2].m_vPosition = VertexRing.m_Vertices[t + 2];

    tri.m_uiVertexIDs[0] = m_Branches[uiBranch].m_LODs[lod].m_SubMesh[aeMeshType::Branch].AddVertex (vtx[0]);
    tri.m_uiVertexIDs[1] = m_Branches[uiBranch].m_LODs[lod].m_SubMesh[aeMeshType::Branch].AddVertex (vtx[1]);
    tri.m_uiVertexIDs[2] = m_Branches[uiBranch].m_LODs[lod].m_SubMesh[aeMeshType::Branch].AddVertex (vtx[2]);

    m_Branches[uiBranch].m_LODs[lod].m_SubMesh[aeMeshType::Branch].m_Triangles.push_back (tri);
  }
}

void aeTree::GenerateSegmentTriangles (aeLod::Enum lod, aeVertexRing& VertexRing0, aeVertexRing& VertexRing1, aeBranch* pBranch, aeBranchNode* pNode1, aeBranchNode* pNode2, aeUInt32 uiPickingSubID, bool bIsLastSegment, float fTexCoordUOffset1, float fTexCoordUOffset2)
{
  const float fTexCoordV0 = pNode1->m_fTexCoordV;
  const float fTexCoordV1 = pNode2->m_fTexCoordV;

  const aeUInt32 uiVertices0 = VertexRing0.m_Vertices.size ();
  const aeUInt32 uiVertices1 = VertexRing1.m_Vertices.size ();

  aeUInt32 uiCurVertex0 = 0;
  aeUInt32 uiCurVertex1 = 0;

  aeUInt32 uiNextVertex0 = 1;
  aeUInt32 uiNextVertex1 = 1;

  aeUInt32 uiProcVertices0 = 0;
  aeUInt32 uiProcVertices1 = 0;

  const float fTexCoordStepU0 = (1.0f / VertexRing0.m_Vertices.size ());
  const float fTexCoordStepU1 = (1.0f / VertexRing1.m_Vertices.size ());

  float fDistToNext0 = fTexCoordStepU0;
  float fDistToNext1 = fTexCoordStepU1;

  aeMeshTriangle tri;
  aeMeshVertex vtx[3];
  tri.m_uiPickingID = pBranch->m_uiPickID;
  tri.m_uiPickingSubID = uiPickingSubID;

  float fPrevTexCoordU0 = fTexCoordUOffset1;
  float fPrevTexCoordU1 = fTexCoordUOffset2;
  float fNextTexCoordU0 = fPrevTexCoordU0 + fTexCoordStepU0;
  float fNextTexCoordU1 = fPrevTexCoordU1 + fTexCoordStepU1;

  const float fRing0Length = VertexRing0.m_fDiameter;
  const float fRing1Length = VertexRing1.m_fDiameter;

  const float fRingQ = fRing1Length / fRing0Length;

  while ((uiProcVertices0 < uiVertices0) || (uiProcVertices1 < uiVertices1))
  {
    if ((uiProcVertices0 < uiVertices0) && (fDistToNext0 <= fDistToNext1))
    {
      aeInt32& iID0 = VertexRing0.m_VertexIDs[uiCurVertex0];
      aeInt32& iID1 = VertexRing1.m_VertexIDs[uiCurVertex1];
      aeInt32& iID2 = VertexRing0.m_VertexIDs[uiNextVertex0];

      vtx[0].m_vPosition = VertexRing0.m_Vertices[uiCurVertex0];
      vtx[1].m_vPosition = VertexRing1.m_Vertices[uiCurVertex1];
      vtx[2].m_vPosition = VertexRing0.m_Vertices[uiNextVertex0];

      vtx[0].m_vNormal = VertexRing0.m_Normals[uiCurVertex0];
      vtx[1].m_vNormal = VertexRing1.m_Normals[uiCurVertex1];
      vtx[2].m_vNormal = VertexRing0.m_Normals[uiNextVertex0];

      vtx[0].m_vTangent = (VertexRing0.m_Vertices[uiNextVertex0] - VertexRing0.m_Vertices[uiCurVertex0]).GetNormalized ();
      vtx[1].m_vTangent = (VertexRing1.m_Vertices[uiNextVertex1] - VertexRing1.m_Vertices[uiCurVertex1]).GetNormalized ();
      vtx[2].m_vTangent = (VertexRing0.m_Vertices[uiNextVertex0] - VertexRing0.m_Vertices[uiCurVertex0]).GetNormalized ();

      for (int i = 0; i < 3; ++i)
      {
        vtx[i].m_vBiTangent = vtx[i].m_vNormal.Cross (vtx[i].m_vTangent).GetNormalized ();
        vtx[i].m_vTangent   = vtx[i].m_vBiTangent.Cross (vtx[i].m_vNormal).GetNormalized ();
      }

      vtx[0].m_vTexCoord.x = fPrevTexCoordU0;
      vtx[1].m_vTexCoord.x = fPrevTexCoordU1 * fRingQ;
      vtx[2].m_vTexCoord.x = fNextTexCoordU0;

      vtx[0].m_vTexCoord.y = fTexCoordV0;
      vtx[1].m_vTexCoord.y = fTexCoordV1 * fRingQ;
      vtx[2].m_vTexCoord.y = fTexCoordV0;

      vtx[0].m_vTexCoord.z = 1;
      vtx[1].m_vTexCoord.z = fRingQ;
      vtx[2].m_vTexCoord.z = 1;

      uiCurVertex0 = uiNextVertex0;
      ++uiNextVertex0;
      ++uiProcVertices0;

      if (uiNextVertex0 == uiVertices0)
      {
        uiNextVertex0 = 0;
        fDistToNext1 = -1;
      }
      else
        fDistToNext0 += fTexCoordStepU0;

      fPrevTexCoordU0 = fNextTexCoordU0;
      fNextTexCoordU0 += fTexCoordStepU0;

      vtx[0].m_iSharedVertex = iID0;
      vtx[1].m_iSharedVertex = iID1;
      vtx[2].m_iSharedVertex = iID2;

      tri.m_uiVertexIDs[0] = pBranch->m_LODs[lod].m_SubMesh[aeMeshType::Branch].AddVertex (vtx[0], iID0);

      if (bIsLastSegment)
        vtx[1].m_iSharedVertex = VertexRing1.m_VertexIDs[0];
      
      tri.m_uiVertexIDs[1] = pBranch->m_LODs[lod].m_SubMesh[aeMeshType::Branch].AddVertex (vtx[1], iID1);
      tri.m_uiVertexIDs[2] = pBranch->m_LODs[lod].m_SubMesh[aeMeshType::Branch].AddVertex (vtx[2], iID2);

      pBranch->m_LODs[lod].m_SubMesh[aeMeshType::Branch].m_Triangles.push_back (tri);
    }
    else
    {
      aeInt32& iID0 = VertexRing0.m_VertexIDs[uiCurVertex0];
      aeInt32& iID1 = VertexRing1.m_VertexIDs[uiCurVertex1];
      aeInt32& iID2 = VertexRing1.m_VertexIDs[uiNextVertex1];

      vtx[0].m_vPosition = VertexRing0.m_Vertices[uiCurVertex0];
      vtx[1].m_vPosition = VertexRing1.m_Vertices[uiCurVertex1];
      vtx[2].m_vPosition = VertexRing1.m_Vertices[uiNextVertex1];

      vtx[0].m_vNormal = VertexRing0.m_Normals[uiCurVertex0];
      vtx[1].m_vNormal = VertexRing1.m_Normals[uiCurVertex1];
      vtx[2].m_vNormal = VertexRing1.m_Normals[uiNextVertex1];
      
      vtx[0].m_vTangent = (VertexRing0.m_Vertices[uiNextVertex0] - VertexRing0.m_Vertices[uiCurVertex0]).GetNormalized ();
      vtx[1].m_vTangent = (VertexRing1.m_Vertices[uiNextVertex1] - VertexRing1.m_Vertices[uiCurVertex1]).GetNormalized ();
      vtx[2].m_vTangent = (VertexRing1.m_Vertices[uiNextVertex1] - VertexRing1.m_Vertices[uiCurVertex1]).GetNormalized ();

      for (int i = 0; i < 3; ++i)
      {
        vtx[i].m_vBiTangent = vtx[i].m_vNormal.Cross (vtx[i].m_vTangent).GetNormalized ();
        vtx[i].m_vTangent   = vtx[i].m_vBiTangent.Cross (vtx[i].m_vNormal).GetNormalized ();
      }

      vtx[0].m_vTexCoord.x = fPrevTexCoordU0;
      vtx[1].m_vTexCoord.x = fPrevTexCoordU1 * fRingQ;
      vtx[2].m_vTexCoord.x = fNextTexCoordU1 * fRingQ;
      
      vtx[0].m_vTexCoord.y = fTexCoordV0;
      vtx[1].m_vTexCoord.y = fTexCoordV1 * fRingQ;
      vtx[2].m_vTexCoord.y = fTexCoordV1 * fRingQ;
      
      vtx[0].m_vTexCoord.z = 1;
      vtx[1].m_vTexCoord.z = fRingQ;
      vtx[2].m_vTexCoord.z = fRingQ;

      uiCurVertex1 = uiNextVertex1;
      ++uiNextVertex1;
      ++uiProcVertices1;

      if (uiNextVertex1 == uiVertices1)
      {
        uiNextVertex1 = 0;
        fDistToNext0 = -1;
      }
      else
        fDistToNext1 += fTexCoordStepU1;

      fPrevTexCoordU1 = fNextTexCoordU1;
      fNextTexCoordU1 += fTexCoordStepU1;

      vtx[0].m_iSharedVertex = iID0;
      vtx[1].m_iSharedVertex = iID1;
      vtx[2].m_iSharedVertex = iID2;

      tri.m_uiVertexIDs[0] = pBranch->m_LODs[lod].m_SubMesh[aeMeshType::Branch].AddVertex (vtx[0], iID0);

      if (bIsLastSegment)
      {
        vtx[1].m_iSharedVertex = VertexRing1.m_VertexIDs[0];
        vtx[2].m_iSharedVertex = VertexRing1.m_VertexIDs[0];
      }

      tri.m_uiVertexIDs[1] = pBranch->m_LODs[lod].m_SubMesh[aeMeshType::Branch].AddVertex (vtx[1], iID1);
      tri.m_uiVertexIDs[2] = pBranch->m_LODs[lod].m_SubMesh[aeMeshType::Branch].AddVertex (vtx[2], iID2);

      pBranch->m_LODs[lod].m_SubMesh[aeMeshType::Branch].m_Triangles.push_back (tri);
    }
  }
}

void aeTree::AlignVertexRing (aeVertexRing& VertexRing, const aeVec3& vNodePosition, aeVec3& vRotationalDir)
{
  return;

  aeVec3 vRotationalDir2 = (VertexRing.m_Vertices[0] - vNodePosition).GetNormalizedSafe ();

  if (vRotationalDir2.IsZeroVector ())
    return;

  const aePlane SlicePlane (VertexRing.m_Vertices[0], VertexRing.m_Vertices[1], VertexRing.m_Vertices[2]);

  if (!SlicePlane.m_vNormal.IsValid ())
    return;

  aeVec3 vProjectedRotDir = vRotationalDir;
  vProjectedRotDir.MakeOrthogonalTo (SlicePlane.m_vNormal);

  aeQuaternion qRotateRing;
  qRotateRing.CreateQuaternion (vRotationalDir2, vProjectedRotDir);

  for (aeUInt32 i = 0; i < VertexRing.m_Vertices.size (); ++i)
  {
    VertexRing.m_Vertices[i] = qRotateRing * (VertexRing.m_Vertices[i] - vNodePosition) + vNodePosition;
    VertexRing.m_Normals[i] = qRotateRing * VertexRing.m_Normals[i];

    AE_CHECK_DEV (VertexRing.m_Vertices[i].IsValid (), "AlignVertexRing: Position is degenerate.");
    AE_CHECK_DEV (VertexRing.m_Normals[i].IsValid (), "AlignVertexRing: Normal is degenerate.");
  }

  vRotationalDir = vRotationalDir2;
}

void aeSubMesh::GenerateVertexNormals (void)
{
  // reset all normals
  const aeUInt32 uiVertices = m_Vertices.size ();
  for (aeUInt32 v = 0; v < uiVertices; ++v)
    m_Vertices[v].m_vNormal.SetZero ();

  // now go through all triangles, compute their normals, add them to the vertex normals
  const aeUInt32 uiTriangles = m_Triangles.size ();
  for (aeUInt32 t = 0; t < uiTriangles; ++t)
  {
    const aeMeshTriangle& tri = m_Triangles[t];

    aeVec3 pos[3];

    for (aeUInt32 v = 0; v < 3; ++v)
      pos[v] = m_Vertices[tri.m_uiVertexIDs[v]].m_vPosition;

    aePlane p (pos);

    for (aeUInt32 v = 0; v < 3; ++v)
      m_Vertices[m_Vertices[tri.m_uiVertexIDs[v]].m_iSharedVertex].m_vNormal += p.m_vNormal;
  }

  // re-normalize all normals
  // compute bitangents
  for (aeUInt32 v = 0; v < uiVertices; ++v)
  {
    m_Vertices[v].m_vNormal.Normalize ();
    m_Vertices[v].m_vBiTangent = m_Vertices[v].m_vNormal.Cross (m_Vertices[v].m_vTangent).GetNormalized ();
    m_Vertices[v].m_vTangent = m_Vertices[v].m_vBiTangent.Cross (m_Vertices[v].m_vNormal).GetNormalized ();
  }

  // now gather all the smooth normals for the triangles
  for (aeUInt32 t = 0; t < uiTriangles; ++t)
  {
    const aeMeshTriangle& tri = m_Triangles[t];

    for (aeUInt32 v = 0; v < 3; ++v)
    {
      m_Vertices[tri.m_uiVertexIDs[v]].m_vNormal    = m_Vertices[m_Vertices[tri.m_uiVertexIDs[v]].m_iSharedVertex].m_vNormal;
      m_Vertices[tri.m_uiVertexIDs[v]].m_vTangent   = m_Vertices[m_Vertices[tri.m_uiVertexIDs[v]].m_iSharedVertex].m_vTangent;
      m_Vertices[tri.m_uiVertexIDs[v]].m_vBiTangent = m_Vertices[m_Vertices[tri.m_uiVertexIDs[v]].m_iSharedVertex].m_vBiTangent;
    }
  }
}



