#include "PCH.h"

#include "../Tree.h"


void aeTree::GenerateFullDetailSkeleton (void)
{
  for (aeUInt32 b = 0; b < m_Branches.size (); ++b)
  {
    if (m_Branches[b].IsDeleted ())
      continue;

    if (m_Branches[b].m_LODs[aeLod::None].m_bSkeletonGenerated)
      continue;

    m_Branches[b].m_LODs[aeLod::None].m_bSkeletonGenerated = true;

    AE_CHECK_DEV (m_Branches[b].m_LODs[aeLod::None].m_NodeIDs.empty (), "Array must be empty.");

    for (aeUInt32 n = 0; n < m_Branches[b].m_Nodes.size (); ++n)
    {
      m_Branches[b].m_LODs[aeLod::None].m_NodeIDs.push_back (n);
    }

    GrowBranchTip (b, aeLod::None);
  }
}

void aeTree::GenerateReducedSkeleton (aeLod::Enum lod)
{
  for (aeUInt32 b = 0; b < m_Branches.size (); ++b)
  {
    if (m_Branches[b].IsDeleted ())
      continue;

    if (m_Branches[b].m_LODs[lod].m_bSkeletonGenerated)
      continue;

    m_Branches[b].m_LODs[lod].m_bSkeletonGenerated = true;

    GenerateReducedNodes (b, lod);

    GrowBranchTip (b, lod);
  }
}

static float GetDistanceToLineSQR (const aeVec3& A, const aeVec3& vLineDir, const aeVec3& P)
{
  // bring P into the space of the line AB
  const aeVec3 P2 = P - A;

  const float fProjectedLength = vLineDir.Dot (P2);

  const aeVec3 vProjectedPos = fProjectedLength * vLineDir;

  return (vProjectedPos - P2).GetLengthSquared ();
}

float GetNodeFlareThickness (const aeSpawnNodeDesc& bnd, const aeBranch& Branch, aeUInt32 uiNode)
{
  if ((bnd.m_uiFlares == 0) || (bnd.m_fFlareWidth <= 1.0f))
    return Branch.m_Nodes[uiNode].m_fThickness;

  float fPosAlongBranch = uiNode / (float) (Branch.m_Nodes.size () - 1);
  float fFlareCurve = bnd.m_FlareWidthCurve.GetValueAt (fPosAlongBranch);
  float fFlareThickness = (1.0f + fFlareCurve * (bnd.m_fFlareWidth - 1.0f));
  float fRet = Branch.m_Nodes[uiNode].m_fThickness * fFlareThickness;
  return fRet;
}

void aeTree::GenerateReducedNodes (aeUInt32 uiBranch, aeLod::Enum lod)
{
  aeBranch& Branch = m_Branches[uiBranch];
  const aeSpawnNodeDesc& bnd = m_Descriptor.m_BranchTypes[Branch.m_Type];

  if (Branch.IsDeleted ())
    return;

  if (Branch.m_Nodes.size () < 4)
    return;

  AE_CHECK_DEV (Branch.m_LODs[lod].m_NodeIDs.empty (), "Array must be empty.");

  const float fMaxCurvatureDistanceSQR = aeMath::Square (m_Descriptor.m_LodData[lod].m_fCurvatureThreshold * 0.01f);

  aeInt32 iAnchor0 = 0;
  aeInt32 iAnchor1 = 1;

  aeVec3 vDir = (Branch.m_Nodes[iAnchor1].m_vPosition - Branch.m_Nodes[iAnchor0].m_vPosition).GetNormalized ();
  float fMinRadius = aeMath::Min (GetNodeFlareThickness (bnd, Branch, iAnchor0), GetNodeFlareThickness (bnd, Branch, iAnchor1));
  float fMaxRadius = aeMath::Max (GetNodeFlareThickness (bnd, Branch, iAnchor0), GetNodeFlareThickness (bnd, Branch, iAnchor1));

  if ((Branch.m_iUmbrellaBuddyID != -1) && (!m_Branches[Branch.m_iUmbrellaBuddyID].m_LODs[lod].m_NodeIDs.empty ()))
  {
    Branch.m_LODs[lod].m_NodeIDs = m_Branches[Branch.m_iUmbrellaBuddyID].m_LODs[lod].m_NodeIDs;
    return;
  }  

  Branch.m_LODs[lod].m_NodeIDs.push_back (iAnchor0);

  const float fSegmentLength = m_Descriptor.m_BranchTypes[Branch.m_Type].m_iSegmentLengthCM / 100.0f;
  float fAccuLength = 0.0f;

  const aeUInt32 uiMaxNodes = Branch.m_Nodes.size ();
  for (aeUInt32 n = 2; n < uiMaxNodes; ++n)
  {
    const float fAnchorRotation = ((float) iAnchor0 / (float) (uiMaxNodes - 1)) * bnd.m_fFlareRotation;

    //aeVec3 vPos = Branch.m_Nodes[n].m_vPosition;
    //aeVec3 vDirToPos = (vPos - Branch.m_Nodes[iAnchor0].m_vPosition).GetNormalized ();

    fMinRadius = aeMath::Min (fMinRadius, GetNodeFlareThickness (bnd, Branch, n));
    fMaxRadius = aeMath::Max (fMaxRadius, GetNodeFlareThickness (bnd, Branch, n));

    // do not insert nodes unless the segment length is reached
    // all manually painted branches have a segment lenght of ~10cm, independent from the specified segment length
    // so this allows to skip nodes to reduce complexity
    if (fAccuLength < fSegmentLength)
    {
      fAccuLength += (Branch.m_Nodes[n].m_vPosition - Branch.m_Nodes[n-1].m_vPosition).GetLength ();
      continue;
    }

    // if the branch curvature is below the threshold angle
    //if (/*(!Branch.m_Nodes[n-1].m_bHasChildBranches) && */(vDirToPos.Dot (vDir) >= fMaxDeviation))
    {
      // check whether the thickness threshold is reached

      const float fAnchor0Thickness = GetNodeFlareThickness (bnd, Branch, iAnchor0);
      const float fAnchor1Thickness = GetNodeFlareThickness (bnd, Branch, n);

      // compute the distance between the last anchor and the current node
      float fMaxLength = 0;
      for (aeUInt32 a = iAnchor0 + 1; a <= n; ++a)
        fMaxLength += (Branch.m_Nodes[a].m_vPosition - Branch.m_Nodes[a - 1].m_vPosition).GetLength ();

      bool bFullFillsThicknessReq = true;
      bool bFullFillsCurvatureReq = true;

      const aeVec3 vLineDir = (Branch.m_Nodes[n].m_vPosition - Branch.m_Nodes[iAnchor0].m_vPosition).GetNormalized ();

      // go through all nodes between the last anchor and this node and compute the interpolated thickness
      // because the polygon will "interpolate" the thickness from anchor0 to anchor1, thus we need to know that
      // to check how much the "desired" thickness deviates from the low-poly mesh thickness
      float fPieceLength = 0;
      for (aeUInt32 a = iAnchor0 + 1; a <= n; ++a)
      {
        fPieceLength += (Branch.m_Nodes[a].m_vPosition - Branch.m_Nodes[a - 1].m_vPosition).GetLength ();
        const float fDesiredThickness = aeMath::Lerp (fAnchor0Thickness, fAnchor1Thickness, (fPieceLength / fMaxLength));

        // the interpolated thickness and the desired thickness should match within some threshold
        if (aeMath::Abs (1.0f - (GetNodeFlareThickness (bnd, Branch, a) / fDesiredThickness)) > m_Descriptor.m_LodData[lod].m_fThicknessThreshold)
        {
          bFullFillsThicknessReq = false;
          break;
        }

        const float fFlareRotation = ((float) a / (float) (uiMaxNodes - 1)) * bnd.m_fFlareRotation;
        const float fRotationDiff = (aeMath::Abs (fFlareRotation - fAnchorRotation) / 360.0f) / 50;

        if (GetDistanceToLineSQR (Branch.m_Nodes[iAnchor0].m_vPosition, vLineDir, Branch.m_Nodes[a].m_vPosition) + fRotationDiff > fMaxCurvatureDistanceSQR)
        {
          bFullFillsCurvatureReq = false;
          break;
        }
      }

      if (bFullFillsThicknessReq && bFullFillsCurvatureReq)
      {
        iAnchor1 = n;
        continue;
      }
    }

    // if this code is reached, some threshold is not reached and a new node must be inserted

    fAccuLength = 0.0f;
    Branch.m_LODs[lod].m_NodeIDs.push_back (iAnchor1);

    // reset the next direction, and thickness thresholds
    vDir = (Branch.m_Nodes[n].m_vPosition - Branch.m_Nodes[iAnchor1].m_vPosition).GetNormalized ();
    fMinRadius = aeMath::Min (GetNodeFlareThickness (bnd, Branch, n), GetNodeFlareThickness (bnd, Branch, iAnchor1));
    fMaxRadius = aeMath::Max (GetNodeFlareThickness (bnd, Branch, n), GetNodeFlareThickness (bnd, Branch, iAnchor1));


    // set the new anchors
    iAnchor0 = iAnchor1;
    iAnchor1 = n;
  }

  Branch.m_LODs[lod].m_NodeIDs.push_back (iAnchor1);

  if (iAnchor1 != uiMaxNodes-1)
    Branch.m_LODs[lod].m_NodeIDs.push_back (uiMaxNodes-1);
}
