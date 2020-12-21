#include "PCH.h"

#include "../Tree/Tree.h"
#include <KrautEditorBasics/Plugin.h>
#include <KrautFoundation/Math/Plane.h>

using namespace AE_NS_EDITORBASICS;


aeBranch::aeBranch ()
{
  m_uiPickID = 0;
  Reset ();
}

aeBranch::~aeBranch ()
{
  if (m_uiPickID != 0)
    aeEditorPlugin::UnregisterPickableObject (m_uiPickID);
}

void aeBranch::SetDeleted (bool bDelete)
{
  if (m_bDeleted == bDelete)
    return;

  if (bDelete)
    AE_BROADCAST_EVENT (aeTreeEdit_BranchDeleted, m_pTree, this);

  Reset ();
  m_bDeleted = bDelete;
}

void aeBranch::Reset (void)
{
  m_pTree = NULL;
  m_bDeleted = false;
  m_bManuallyCreated = false;
  m_iParentBranchID = -1;
  m_uiParentBranchNodeID = 0;
  m_iUmbrellaBuddyID = -1;
  m_fUmbrellaBranchRotation = 0;
  m_fThickness = 0.0f;
  m_fBranchLength = 0.0f;
  m_Type = aeBranchType::None;
  m_fFrondColorVariation = 0.0f;
  m_uiFrondTextureVariation = 0;

  for (aeUInt32 i = 0; i < aeLod::ENUM_COUNT; ++i)
    m_LODs[i].Reset ();

  m_Nodes.clear ();
}

void aeBranch::Save (aeStreamOut& s)
{
  aeUInt8 uiVersion = 7;
  s << uiVersion;

  // Version 1
  s << m_iParentBranchID;
  s << m_uiParentBranchNodeID;
  s << m_fBranchLength;
  s << m_fThickness;

  float fRoundnessFactor = 0.5f;
  s << fRoundnessFactor;
  s << (aeUInt8) m_Type;

  aeUInt32 uiNodes = m_Nodes.size ();
  s << uiNodes;

  for (aeUInt32 n = 0; n < uiNodes; ++n)
  {
    m_Nodes[n].Save (s);
  }

  // Version 2
  s << m_bManuallyCreated;

  // Version 4
  s << m_bDeleted;

  // Version 5
  const bool bFalse = false;
  s << bFalse;
  s << bFalse;
  s << bFalse;

  // Version 6
  s << m_vLeafUpDirection;

  // Version 7
  s << m_iUmbrellaBuddyID;
  s << m_fUmbrellaBranchRotation;
}

void aeBranch::Load (aeStreamIn& s)
{
  Reset ();

  aeUInt8 uiVersion;
  s >> uiVersion;

  s >> m_iParentBranchID;
  s >> m_uiParentBranchNodeID;
  s >> m_fBranchLength;
  s >> m_fThickness;

  float fRoundnessFactor = 0.5f;
  s >> fRoundnessFactor;

  aeUInt8 t;
  s >> t; m_Type = (aeBranchType::Enum) t;

  if (uiVersion < 3)
  {
    if (t > 0)
      m_Type = (aeBranchType::Enum) (t + 2);
  }

  aeUInt32 uiNodes;
  s >> uiNodes;
  m_Nodes.resize (uiNodes);

  for (aeUInt32 n = 0; n < uiNodes; ++n)
  {
    m_Nodes[n].Load (s);
  }

  if (uiVersion >= 2)
    s >> m_bManuallyCreated;

  if (uiVersion >= 4)
    s >> m_bDeleted;

  if (uiVersion >= 5)
  {
    bool bFalse;
    s >> bFalse;
    s >> bFalse;
    s >> bFalse;
  }

  if (uiVersion >= 6)
  {
    s >> m_vLeafUpDirection;
  }

  if (uiVersion >= 7)
  {
    s >> m_iUmbrellaBuddyID;
    s >> m_fUmbrellaBranchRotation;
  }
}

const aeMatrix aeBranch::GetNodeTransformation (aeUInt32 uiNode)
{
  AE_CHECK_DEV (uiNode < m_Nodes.size (), "Invalid Node-Index");

  aeUInt32 uiNode1 = uiNode;
  aeUInt32 uiNode2 = uiNode + 1;

  if (uiNode == m_Nodes.size () - 1)
  {
    uiNode1 = uiNode - 1;
    uiNode2 = uiNode;
  }

  aeVec3 vDir = (m_Nodes[uiNode2].m_vPosition - m_Nodes[uiNode1].m_vPosition).GetNormalized ();

  aeMatrix m;

  if (vDir.GetAngleBetween (aeVec3 (0, 1, 0)) < 0.5f)
    m.SetObjectOrientationMatrixZ (m_Nodes[uiNode].m_vPosition, vDir, vDir.GetOrthogonalVector ());
  else
    m.SetObjectOrientationMatrixZ (m_Nodes[uiNode].m_vPosition, vDir);

  return m;
}

void aeBranch::TransformNode (aeUInt32 uiNode, const aeMatrix& mTransform0, float fSelectionStrength0, bool bLocal)
{
  if (fSelectionStrength0 <= 0.0f)
    return;

  // if this transformation moves the entire branch, rotate the 'leaf up direction' accordingly
  if (uiNode == 0)
  {
    m_vLeafUpDirection = mTransform0.TransformDirection (m_vLeafUpDirection);
  }

  const float fFactor = g_Globals.s_uiModifyStrength / 100.0f;

  float fSelectionStrength = fSelectionStrength0;

  for (aeUInt32 n = uiNode; n < m_Nodes.size (); ++n)
  {
    if (!bLocal)
      fSelectionStrength = 1.0f;

    const aeMatrix mTransform = aeMath::Lerp (aeMatrix::IdentityMatrix (), mTransform0, aeMath::Max (0.0f, fSelectionStrength));
    fSelectionStrength *= fFactor;

    m_Nodes[n].m_vPosition = mTransform * m_Nodes[n].m_vPosition;

    if (m_Nodes[n].m_bHasChildBranches)
    {
      // search for all branches that branch from this node
      for (aeUInt32 b = 0; b < m_pTree->m_Branches.size (); ++b)
      {
        if (m_pTree->m_Branches[b].IsDeleted ())
          continue;

        if ((m_pTree->m_Branches[b].m_iParentBranchID >= 0) &&
          (&m_pTree->m_Branches[m_pTree->m_Branches[b].m_iParentBranchID] == this) &&
          (m_pTree->m_Branches[b].m_uiParentBranchNodeID == n))
        {
          m_pTree->m_Branches[b].TransformNode (0, mTransform, fSelectionStrength, false);
        }
      }
    }
  }

  fSelectionStrength = fSelectionStrength0;

  for (aeInt32 n = (aeInt32) uiNode - 1; n >= 0; --n)
  {
    fSelectionStrength *= fFactor;

    const aeMatrix mTransform = aeMath::Lerp (aeMatrix::IdentityMatrix (), mTransform0, aeMath::Max (0.0f, fSelectionStrength));

    m_Nodes[n].m_vPosition = mTransform * m_Nodes[n].m_vPosition;

    if (m_Nodes[n].m_bHasChildBranches)
    {
      // search for all branches that branch from this node
      for (aeUInt32 b = 0; b < m_pTree->m_Branches.size (); ++b)
      {
        if (m_pTree->m_Branches[b].IsDeleted ())
          continue;

        if ((m_pTree->m_Branches[b].m_iParentBranchID >= 0) &&
          (&m_pTree->m_Branches[m_pTree->m_Branches[b].m_iParentBranchID] == this) &&
          (m_pTree->m_Branches[b].m_uiParentBranchNodeID == n))
        {
          m_pTree->m_Branches[b].TransformNode (0, mTransform, fSelectionStrength, false);
        }
      }
    }
  }
}

  
void aeTree::GenerateAllTreeMeshes (aeLod::Enum lod, bool bTrunkCap, bool bBranchCap)
{
  if (aeLodMode::IsImpostorMode (m_Descriptor.m_LodData[lod].m_Mode))
  {
    CreateTreeImpostorMesh (lod);
    return;
  }

  for (aeUInt32 b = 0; b < m_Branches.size (); ++b)
  {
    if (m_Branches[b].IsDeleted ())
      continue;

    const aeSpawnNodeDesc& bnd = m_Descriptor.m_BranchTypes[m_Branches[b].m_Type];

    if ((!m_Branches[b].m_LODs[lod].m_bMeshGenerated[aeMeshType::Branch]) ||
        (m_Branches[b].m_LODs[lod].m_bMeshGeneratedTrunkCaps != bTrunkCap) ||
        (m_Branches[b].m_LODs[lod].m_bMeshGeneratedBranchCaps != bBranchCap))
    {
      m_Branches[b].m_LODs[lod].m_bMeshGenerated[aeMeshType::Branch] = true;
      m_Branches[b].m_LODs[lod].m_bMeshGeneratedTrunkCaps = bTrunkCap;
      m_Branches[b].m_LODs[lod].m_bMeshGeneratedBranchCaps = bBranchCap;

      m_Branches[b].m_LODs[lod].m_SubMesh[aeMeshType::Branch].clear ();
    
      if (m_Branches[b].m_uiPickID == 0)
        m_Branches[b].m_uiPickID = aeEditorPlugin::RegisterPickableObject (&m_Branches[b], b, "aeTreeBranch");

      if (bnd.m_bEnable[aeMeshType::Branch])
        GenerateBranchMesh (b, lod, bBranchCap || ((b == 0) && bTrunkCap));
    }

    if (!m_Branches[b].m_LODs[lod].m_bMeshGenerated[aeMeshType::Frond])
    {
      m_Branches[b].m_LODs[lod].m_bMeshGenerated[aeMeshType::Frond] = true;
      m_Branches[b].m_LODs[lod].m_SubMesh[aeMeshType::Frond].clear ();

      if (m_Branches[b].m_uiPickID == 0)
        m_Branches[b].m_uiPickID = aeEditorPlugin::RegisterPickableObject (&m_Branches[b], b, "aeTreeBranch");

      if (bnd.m_bEnable[aeMeshType::Frond])
      {
        if (bnd.m_BranchTypeMode == aeBranchTypeMode::Umbrella)
          GenerateUmbrellaMesh (b, lod);
        else
          GenerateFrondMesh (b, lod);
      }
    }

    if (!m_Branches[b].m_LODs[lod].m_bMeshGenerated[aeMeshType::Leaf])
    {
      m_Branches[b].m_LODs[lod].m_bMeshGenerated[aeMeshType::Leaf] = true;
      m_Branches[b].m_LODs[lod].m_SubMesh[aeMeshType::Leaf].clear ();

      if (m_Branches[b].m_uiPickID == 0)
        m_Branches[b].m_uiPickID = aeEditorPlugin::RegisterPickableObject (&m_Branches[b], b, "aeTreeBranch");

      if (bnd.m_bEnable[aeMeshType::Leaf])
        GenerateLeafMesh (b, lod);
    }
  }
}

void aeTree::ComputeNodeVTextureCoordinates (aeUInt32 uiBranch, aeLod::Enum lod)
{
  if (m_Branches[uiBranch].IsDeleted ())
    return;

  aeBranch& Branch = m_Branches[uiBranch];

  aeInt32 iCurRing = 1;
  aeInt32 iPrevRing = 0;

  aeVertexRing VertexRing[2];
  Branch.m_Nodes[0].m_iSegments = GenerateVertexRing (&Branch, 0, &VertexRing[iPrevRing], aeLod::None, false, Branch.m_Nodes[0].m_vPosition);

  float fPrevTexCoordV = 0.0f;

  float fLastLength = VertexRing[iPrevRing].m_fDiameter;

  aeVec3 vLastPos = Branch.m_Nodes[0].m_vPosition;

  for (aeUInt32 n = 1; n < Branch.m_Nodes.size (); ++n)
  {
    Branch.m_Nodes[n].m_iSegments = GenerateVertexRing (&Branch, n, &VertexRing[iCurRing], aeLod::None, false, Branch.m_Nodes[n].m_vPosition);

    const float fDist = (vLastPos - Branch.m_Nodes[n].m_vPosition).GetLength ();

    float fNextTexCoordV = fPrevTexCoordV + (fDist / fLastLength);

    if (VertexRing[iCurRing].m_fDiameter > 0.1f)
      fLastLength = VertexRing[iCurRing].m_fDiameter;

    Branch.m_Nodes[n - 1].m_fTexCoordV = fPrevTexCoordV;
    Branch.m_Nodes[n].m_fTexCoordV     = fNextTexCoordV;

    fPrevTexCoordV = fNextTexCoordV;
    vLastPos = Branch.m_Nodes[n].m_vPosition;
  }

  // Now the same for the tip

  const aeVec3 vNormalAnchor = Branch.m_Nodes.back ().m_vPosition;

  for (aeUInt32 n = 0; n < Branch.m_LODs[lod].m_TipNodes.size (); ++n)
  {
    Branch.m_LODs[lod].m_TipNodes[n].m_iSegments = GenerateVertexRing (&Branch, n, &VertexRing[iCurRing], lod, true, vNormalAnchor);

    const float fDist = (vLastPos - Branch.m_LODs[lod].m_TipNodes[n].m_vPosition).GetLength ();

    float fNextTexCoordV = fPrevTexCoordV + (fDist / fLastLength);

    if (VertexRing[iCurRing].m_fDiameter > 0.1f)
      fLastLength = VertexRing[iCurRing].m_fDiameter;

    Branch.m_LODs[lod].m_TipNodes[n].m_fTexCoordV     = fNextTexCoordV;

    fPrevTexCoordV = fNextTexCoordV;
    vLastPos = Branch.m_LODs[lod].m_TipNodes[n].m_vPosition;
  }
}

void aeTree::GenerateBranchMesh (aeUInt32 uiBranch, aeLod::Enum lod, bool bGenerateCap)
{
  aeBranch& Branch = m_Branches[uiBranch];

  if (Branch.IsDeleted ())
    return;

  if (Branch.m_LODs[lod].m_NodeIDs.size () < 2)
    return;

  if (!m_Descriptor.m_LodData[lod].m_AllowTypes[aeMeshType::Branch].IsAnyFlagSet (1 << Branch.m_Type))
    return;

  const aeSpawnNodeDesc& bnd = m_Descriptor.m_BranchTypes[Branch.m_Type];

  ComputeNodeVTextureCoordinates (uiBranch, lod);

  aeInt32 iCurRing = 1;
  aeInt32 iPrevRing = 0;

  aeVertexRing VertexRing[2];

  Branch.m_Nodes[Branch.m_LODs[lod].m_NodeIDs[0]].m_iSegments = GenerateVertexRing (&Branch, 0, &VertexRing[iPrevRing], lod, false, Branch.m_Nodes[Branch.m_LODs[lod].m_NodeIDs[0]].m_vPosition);
  aeVec3 vRotationalDir = (VertexRing[iPrevRing].m_Vertices[0] - Branch.m_Nodes[Branch.m_LODs[lod].m_NodeIDs[0]].m_vPosition).GetNormalized ();

  if (bGenerateCap)
    GenerateCapTriangles (uiBranch, lod, VertexRing[iPrevRing]);

  float fPrevTexCoordV = Branch.m_Nodes[Branch.m_LODs[lod].m_NodeIDs[0]].m_fTexCoordV;

  const aeUInt32 uiMaxNodes = Branch.m_LODs[lod].m_NodeIDs.size ();
  for (aeUInt32 n = 1; n < uiMaxNodes; ++n)
  {
    const float fPosAlongBranch0 = (float) (n-1) / (float) (uiMaxNodes - 1);
    const float fPosAlongBranch1 = (float) n / (float) (uiMaxNodes - 1);
    const float fFlareRotation0 = bnd.m_bRotateTexCoords ? (bnd.m_fFlareRotation * fPosAlongBranch0) / 360.0f : 0.0f;
    const float fFlareRotation1 = bnd.m_bRotateTexCoords ? (bnd.m_fFlareRotation * fPosAlongBranch1) / 360.0f : 0.0f;

    Branch.m_Nodes[Branch.m_LODs[lod].m_NodeIDs[n]].m_iSegments = GenerateVertexRing (&Branch, n, &VertexRing[iCurRing], lod, false, Branch.m_Nodes[Branch.m_LODs[lod].m_NodeIDs[n]].m_vPosition);

    AlignVertexRing (VertexRing[iCurRing], Branch.m_Nodes[Branch.m_LODs[lod].m_NodeIDs[n]].m_vPosition, vRotationalDir);

    float fNextTexCoordV = Branch.m_Nodes[Branch.m_LODs[lod].m_NodeIDs[n]].m_fTexCoordV;

    GenerateSegmentTriangles (lod, VertexRing[iPrevRing], VertexRing[iCurRing], &Branch, &Branch.m_Nodes[Branch.m_LODs[lod].m_NodeIDs[n-1]], &Branch.m_Nodes[Branch.m_LODs[lod].m_NodeIDs[n]], Branch.m_LODs[lod].m_NodeIDs[n-1], false, fFlareRotation0, fFlareRotation1);

    aeMath::Swap (iPrevRing, iCurRing);

    fPrevTexCoordV = fNextTexCoordV;
  }

  // Do the same for the tip

  aeBranchNode* pPrevNode = &Branch.m_Nodes[Branch.m_LODs[lod].m_NodeIDs.back ()];

  if ((Branch.m_LODs[lod].m_TipNodes.size () == 1) && (VertexRing[iPrevRing].m_Vertices.size () == 3) && (lod > aeLod::Lod1))
  {
    // in case of lod 2, add a one-triangle cap
    if (lod <= aeLod::Lod2)
    {
      aeVec3 vPos1 = Branch.m_Nodes[Branch.m_LODs[lod].m_NodeIDs[Branch.m_LODs[lod].m_NodeIDs.size () - 1]].m_vPosition;
      aeVec3 vPos0 = Branch.m_Nodes[Branch.m_LODs[lod].m_NodeIDs[Branch.m_LODs[lod].m_NodeIDs.size () - 2]].m_vPosition;
      aeVec3 vDir = vPos1 - vPos0;
      vDir.Normalize ();

      aeMeshTriangle t;
      aeMeshVertex vtx[3];

      t.m_uiPickingID = Branch.m_uiPickID;
      t.m_uiPickingSubID = Branch.m_Nodes.size () - 1;

      vtx[0].m_vPosition = VertexRing[iPrevRing].m_Vertices[0];
      vtx[1].m_vPosition = VertexRing[iPrevRing].m_Vertices[2];
      vtx[2].m_vPosition = VertexRing[iPrevRing].m_Vertices[1];
      vtx[1].m_vTexCoord.x = 1;
      vtx[1].m_vTexCoord.y = 0;
      vtx[2].m_vTexCoord.x = 0.5f;
      vtx[2].m_vTexCoord.y = 1;
      vtx[0].m_vNormal = (VertexRing[iPrevRing].m_Vertices[0] - vPos1).GetNormalized ();
      vtx[1].m_vNormal = (VertexRing[iPrevRing].m_Vertices[2] - vPos1).GetNormalized ();
      vtx[2].m_vNormal = (VertexRing[iPrevRing].m_Vertices[1] - vPos1).GetNormalized ();

      vtx[0].m_iSharedVertex = VertexRing[iPrevRing].m_VertexIDs[0];
      vtx[1].m_iSharedVertex = VertexRing[iPrevRing].m_VertexIDs[2];
      vtx[2].m_iSharedVertex = VertexRing[iPrevRing].m_VertexIDs[1];

      t.m_uiVertexIDs[0] = Branch.m_LODs[lod].m_SubMesh[aeMeshType::Branch].AddVertex (vtx[0], VertexRing[iPrevRing].m_VertexIDs[0]);
      t.m_uiVertexIDs[1] = Branch.m_LODs[lod].m_SubMesh[aeMeshType::Branch].AddVertex (vtx[1], VertexRing[iPrevRing].m_VertexIDs[2]);
      t.m_uiVertexIDs[2] = Branch.m_LODs[lod].m_SubMesh[aeMeshType::Branch].AddVertex (vtx[2], VertexRing[iPrevRing].m_VertexIDs[1]);

      Branch.m_LODs[lod].m_SubMesh[aeMeshType::Branch].m_Triangles.push_back (t);
    }

    // in case of lod 2 and above, just leave a hole

  }
  else
  {
    const aeVec3 vNormalAnchor = Branch.m_Nodes[Branch.m_LODs[lod].m_NodeIDs.back ()].m_vPosition;

    const float fFlareRotation = bnd.m_bRotateTexCoords ? (bnd.m_fFlareRotation) / 360.0f : 0.0f;

    for (aeUInt32 n = 0; n < Branch.m_LODs[lod].m_TipNodes.size (); ++n)
    {
      Branch.m_LODs[lod].m_TipNodes[n].m_iSegments = GenerateVertexRing (&Branch, n, &VertexRing[iCurRing], lod, true, vNormalAnchor);

      AlignVertexRing (VertexRing[iCurRing], Branch.m_LODs[lod].m_TipNodes[n].m_vPosition, vRotationalDir);

      const float fNextTexCoordV = Branch.m_LODs[lod].m_TipNodes[n].m_fTexCoordV;

      GenerateSegmentTriangles (lod, VertexRing[iPrevRing], VertexRing[iCurRing], &Branch, pPrevNode, &Branch.m_LODs[lod].m_TipNodes[n], Branch.m_Nodes.size () - 1, n == Branch.m_LODs[lod].m_TipNodes.size () - 1, fFlareRotation, fFlareRotation);

      aeMath::Swap (iPrevRing, iCurRing);

      fPrevTexCoordV = fNextTexCoordV;
      pPrevNode = &Branch.m_LODs[lod].m_TipNodes[n];
    }
  }

  Branch.m_LODs[lod].m_SubMesh[aeMeshType::Branch].GenerateVertexNormals ();
}


