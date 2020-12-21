#include "PCH.h"

#include "../Tree.h"


static aeVec3 GetTargetDir (aeTargetDir::Enum dir, const aeVec3& vParentDir, float fMaxDeviation, const aeVec3& vRotationAxis, const aeVec3& vOwnStartDir, bool bRelativeToParent)
{
  aeVec3 r;
  float fAngle = 0.0f; 

  switch (dir)
  {
  case aeTargetDir::Straight:
    r.CreateRandomVector (vOwnStartDir, (aeUInt8) fMaxDeviation);
    return r;
  case aeTargetDir::Upwards:
    fAngle = 0.0f;
    break;
  case aeTargetDir::Degree22:
    fAngle = 22.5f;
    break;
  case aeTargetDir::Degree45:
    fAngle = 45.0f;
    break;
  case aeTargetDir::Degree67:
    fAngle = 67.5f;
    break;
  case aeTargetDir::Degree90:
    fAngle = 90.0f;
    break;
  case aeTargetDir::Degree112:
    fAngle = 112.5f;
    break;
  case aeTargetDir::Degree135:
    fAngle = 135.0f;
    break;
  case aeTargetDir::Degree157:
    fAngle = 157.5f;
    break;
  case aeTargetDir::Downwards:
    fAngle = 180.0f;
    break;
  }

  aeVec3 vNormal;

  if (bRelativeToParent)
  {
    aeQuaternion q;
    q.CreateQuaternion (vRotationAxis, fAngle);

    vNormal = q * vParentDir;
  }
  else
  {
    aeVec3 vRotateDownAxis;
    if (aeMath::Abs (vOwnStartDir.Dot (aeVec3 (0, 1, 0))) > 0.99999f)
    {
      vRotateDownAxis = vRotationAxis;
    }
    else
      vRotateDownAxis = aeVec3 (0, 1, 0).Cross (vOwnStartDir).GetNormalized ();

    aeQuaternion q;
    q.CreateQuaternion (vRotateDownAxis, fAngle);
    
    vNormal = q * aeVec3 (0, 1, 0);
  }

  r.CreateRandomVector (vNormal, (aeUInt8) fMaxDeviation);

  return r;
}

static float GetGrowDir2Distance (aeTargetDir2Usage::Enum Usage, float fBranchLength, float fUseDist)
{
  switch (Usage)
  {
  case aeTargetDir2Usage::Off:
    return 10000.0f;
  case aeTargetDir2Usage::Absolute:
    return fUseDist;
  case aeTargetDir2Usage::Relative:
    return fBranchLength * fUseDist / 5.0f; // the targetdirusage is in [0;5] range
  }

  AE_CHECK_DEV (false, "Wrong Code Path.");
  return 0;
}

void aeTree::GrowSkeleton ()
{
  for (aeUInt32 i = 0; i < aeBranchType::ENUM_COUNT; ++i)
    m_Descriptor.m_BranchTypes[i].m_Type = (aeBranchType::Enum) i;

  if (m_Descriptor.m_bGrowProceduralTrunks)
  {
    // for each trunk type
    for (aeUInt32 trunks = 0; trunks < 3; ++trunks)
    {
      const aeSpawnNodeDesc& bnd = m_Descriptor.m_BranchTypes[aeBranchType::Trunk1 + trunks];

      if (!bnd.m_bUsed)
        continue;

      aeBranchNodeRandomData NodeRD;
      NodeRD.m_BranchRD.m_uiSeedValue = m_Descriptor.m_uiRandomSeed + trunks;
      NodeRD.m_SpawnNodesRD = m_Descriptor.m_uiRandomSeed + trunks;
      aeMath::SetRandSeed (NodeRD.m_SpawnNodesRD);

      // determine how many trunks to spawn (of the current type)
      const aeUInt32 uiTrunks = aeMath::Rand (bnd.m_uiMinBranches, bnd.m_uiMaxBranches);

      aeInt32 iFirstBranch = -1;
      // for each trunk to spawn
      for (aeUInt32 t = 0; t < uiTrunks; ++t)
      {
        // TODO: Handle LeafCard mode here
        const float fRotAngle = t * (360.0f / uiTrunks);
        aeQuaternion qMainDir;
        qMainDir.CreateQuaternion (aeVec3 (0, 1, 0), fRotAngle);

        const float fMinDistanceToStartPos = (uiTrunks > 1) ? aeMath::Max (0.01f, bnd.m_fNodeSpacingBefore) : 0.01f;

        aeMath::SetRandSeed (NodeRD.m_NodePlacementRD.GetRandomNumber ());
        const float fDistance = fMinDistanceToStartPos + aeMath::Rand ((aeInt32) (bnd.m_fNodeHeight / 0.05f) + 1) * 0.05f;

        aeBranchDesc dTrunk = bnd.CreateBranchDesc (NodeRD.GetBranchRD ());

        dTrunk.m_vStartPosition = qMainDir * aeVec3 (fDistance, 0, 0);

        aeMath::SetRandSeed (dTrunk.m_RandomData.m_TargetDirRD);

        aeVec3 vBranchAngleAxis;

        {
          const float fRotationalDev = (bnd.m_fMaxRotationalDeviation > 0) ? (aeMath::Rand (bnd.m_fMaxRotationalDeviation * 2) - bnd.m_fMaxRotationalDeviation) : aeMath::Rand (1);
          aeQuaternion qRotationalDev;
          qRotationalDev.CreateQuaternion (aeVec3 (0, 1, 0), fRotationalDev);

          const aeVec3 vDirToPos = qRotationalDev * dTrunk.m_vStartPosition.GetNormalized ();
          vBranchAngleAxis = aeVec3 (0, 1, 0).Cross (vDirToPos).GetNormalized ();
          const float fBranchAngle = (bnd.m_fBranchAngle - 90) + aeMath::Randf (bnd.m_fMaxBranchAngleDeviation * 2.0f) - bnd.m_fMaxBranchAngleDeviation;
          aeQuaternion qBranchAngle;
          qBranchAngle.CreateQuaternion (vBranchAngleAxis, fBranchAngle);

          dTrunk.m_vStartDirection = qBranchAngle * aeVec3 (0, 1, 0);
        }

        dTrunk.m_vGrowDirection = GetTargetDir (bnd.m_TargetDirection,  aeVec3 (0, 1, 0), bnd.m_fMaxTargetDirDeviation, vBranchAngleAxis, dTrunk.m_vStartDirection, bnd.m_bTargetDirRelative);
        dTrunk.m_vGrowDirection2= GetTargetDir (bnd.m_TargetDirection2, aeVec3 (0, 1, 0), bnd.m_fMaxTargetDirDeviation, vBranchAngleAxis, dTrunk.m_vStartDirection, bnd.m_bTargetDirRelative);
        dTrunk.m_fGrowDir2UUsageDistance = GetGrowDir2Distance (bnd.m_TargetDir2Uage, dTrunk.m_fBranchLength, bnd.m_fTargetDir2Usage);

        iFirstBranch = GrowBranch (dTrunk);
        if ((bnd.m_BranchTypeMode == aeBranchTypeMode::Umbrella) || (iFirstBranch < 0))
          break;
      }

      if ((iFirstBranch >= 0) && (bnd.m_BranchTypeMode == aeBranchTypeMode::Umbrella))
      {
        float fSpawnAngle = 0.0f;
        const float fSpawnAngleStep = 360.0f / uiTrunks;

        aeInt32 iLastBranchID = iFirstBranch;
        for (aeUInt32 t = 1; t < uiTrunks; ++t)
        {
          aeBranchRandomData BranchRD = NodeRD.GetBranchRD ();

          // the next branch will be spawned in another direction
          fSpawnAngle += fSpawnAngleStep;

          iLastBranchID = DuplicateBranch (iFirstBranch, fSpawnAngle, iLastBranchID);
        }

        m_Branches[iFirstBranch].m_fUmbrellaBranchRotation = 0.0f;
        m_Branches[iFirstBranch].m_iUmbrellaBuddyID = iLastBranchID;

        break;
      }
    }
  }

  aeBranchType::Enum UpdateTypes[4][3] = { 
    { aeBranchType::Trunk1,        aeBranchType::Trunk2,        aeBranchType::Trunk3        },
    { aeBranchType::MainBranches1, aeBranchType::MainBranches2, aeBranchType::MainBranches3 },
    { aeBranchType::SubBranches1,  aeBranchType::SubBranches2,  aeBranchType::SubBranches3  },
    { aeBranchType::Twigs1,        aeBranchType::Twigs2,        aeBranchType::Twigs3        } };
//    { aeBranchType::SubTwigs1,     aeBranchType::SubTwigs2,     aeBranchType::SubTwigs3     } };

  for (int r = 1; r <= 3; ++r)
  {
    const aeUInt32 uiMaxBranches = m_Branches.size ();
    for (aeUInt32 b = 0; b < uiMaxBranches; ++b)
    {
      if (m_Branches[b].IsDeleted ())
        continue;

      aeBranchType::Enum type = m_Branches[b].m_Type;

      if ((type != UpdateTypes[r - 1][0]) && (type != UpdateTypes[r - 1][1]) && (type != UpdateTypes[r - 1][2]))
        continue;

      if (m_Branches[b].m_bManuallyCreated)
      {
        aeBranchNodeRandomData NodeRD;
        NodeRD.m_BranchRD.m_uiSeedValue = m_Descriptor.m_uiRandomSeed;

        m_Branches[b].m_RandomData = NodeRD.GetBranchRD ();
      }

      SpawnSubBranches_Reverse (b, m_Descriptor.m_BranchTypes[UpdateTypes[r][0]], m_Descriptor.m_BranchTypes[UpdateTypes[r][1]], m_Descriptor.m_BranchTypes[UpdateTypes[r][2]]);
    }
  }
}

aeInt32 aeTree::InsertOneBranch (aeUInt32 uiParentBranch, aeUInt32 uiStartNode0, const aeBranchDesc& desc, float fBranchDistance, float fRotation, const aeSpawnNodeDesc& NodeDesc, aeBranchRandomData& BranchRD, bool bReverse)
{
  aeInt32 uiStartNode = uiStartNode0;

  aeBranchDesc bd = desc;

  aeBranch* pBranch = &m_Branches[uiParentBranch];

  const aeSpawnNodeDesc& ParentDesc = m_Descriptor.m_BranchTypes[pBranch->m_Type];

  float fCurDistance = 0.01f;

  aeInt32 iPrevNode = uiStartNode;

  // find a node that is far enough away to spawn the branch
  while (fCurDistance < fBranchDistance)
  {
    iPrevNode = uiStartNode;

    if (bReverse)
      --uiStartNode;
    else
      ++uiStartNode;

    if ((uiStartNode < 0) || (uiStartNode >= (aeInt32) pBranch->m_Nodes.size ()))
      return -1;

    fCurDistance += (pBranch->m_Nodes[uiStartNode].m_vPosition - pBranch->m_Nodes[iPrevNode].m_vPosition).GetLength ();
  }

  // now we know at which node to spawn this branch (uiStartNode)

  const float fRealSpawnAngle = fRotation + desc.m_fRotationalDeviation;

  aeVec3 vNodeDirection;
  aeVec3 vStartNodeDirection;
  {
    if (uiStartNode > 0)
      vNodeDirection = (pBranch->m_Nodes[uiStartNode].m_vPosition - pBranch->m_Nodes[uiStartNode - 1].m_vPosition).GetNormalized ();
    else
      vNodeDirection = (pBranch->m_Nodes[1].m_vPosition - pBranch->m_Nodes[0].m_vPosition).GetNormalized ();

    if (uiStartNode0 > 0)
      vStartNodeDirection = (pBranch->m_Nodes[uiStartNode0].m_vPosition - pBranch->m_Nodes[uiStartNode0 - 1].m_vPosition).GetNormalized ();
    else
      vStartNodeDirection = (pBranch->m_Nodes[1].m_vPosition - pBranch->m_Nodes[0].m_vPosition).GetNormalized ();
  }

  // determine the exact spawn position for this branch (can be between nodes)
  aeVec3 vBranchSpawnPosition = pBranch->m_Nodes[uiStartNode].m_vPosition;
  {
    vBranchSpawnPosition -= vNodeDirection * (fCurDistance - fBranchDistance);
  }

  aeVec3 vOrthoDir;

  if (!m_Descriptor.m_bLeafCardMode)
  {
    if (aeMath::Abs (vStartNodeDirection.Dot (aeVec3 (0, 1, 0))) > 0.999f)
      vOrthoDir = vStartNodeDirection.GetOrthogonalVector ();
    else
    {
      vOrthoDir = aeVec3 (0, 1, 0).Cross (vStartNodeDirection);
      vOrthoDir = vStartNodeDirection.Cross (vOrthoDir);
    }

    vOrthoDir.Normalize ();
  }
  else
    vOrthoDir.SetVector (0, 0, 1);

  const float fBranchAngle = desc.m_fBranchAngle;

  aeQuaternion qBranchAngle, qBranchDir;
  qBranchAngle.CreateQuaternion (vOrthoDir, fBranchAngle);
  qBranchDir.CreateQuaternion (vNodeDirection, fRealSpawnAngle);

  const aeVec3 vBranchDir = qBranchDir * qBranchAngle * vNodeDirection;

  const aeVec3 vRotationAxis = -vBranchDir.Cross (vNodeDirection).GetNormalized ();

  // check how long the branch shall be that is spawned at this position
  bd.m_fBranchLength *= NodeDesc.m_MaxBranchLengthParentScale.GetValueAt ((float) uiStartNode / (pBranch->m_Nodes.size () - 1));

  bd.m_vGrowDirection = GetTargetDir (NodeDesc.m_TargetDirection, vNodeDirection, NodeDesc.m_fMaxTargetDirDeviation, vRotationAxis, vBranchDir, NodeDesc.m_bTargetDirRelative);
  bd.m_vGrowDirection2= GetTargetDir (NodeDesc.m_TargetDirection2, vNodeDirection, NodeDesc.m_fMaxTargetDirDeviation, vRotationAxis, vBranchDir, NodeDesc.m_bTargetDirRelative);
  bd.m_fGrowDir2UUsageDistance = GetGrowDir2Distance (NodeDesc.m_TargetDir2Uage, bd.m_fBranchLength, NodeDesc.m_fTargetDir2Usage);

  bd.m_vStartDirection = vBranchDir;
  bd.m_vStartPosition = vBranchSpawnPosition;
  bd.m_iParentBranchID = uiParentBranch;
  bd.m_iParentBranchNodeID = uiStartNode;

  const aeInt32 iBranchID = GrowBranch (bd);

  if (iBranchID >= 0)
    pBranch->m_Nodes[uiStartNode].m_bHasChildBranches = true;

  return iBranchID;
}

aeUInt32 aeTree::DuplicateBranch (aeUInt32 uiOriginalBranchID, float fRotation, aeInt32 iBranchBuddy)
{
  aeBranch* pOriginalBranch = &m_Branches[uiOriginalBranchID];

  const aeVec3 vOrigin = pOriginalBranch->m_Nodes[0].m_vPosition;
  const aeInt32 iParentBranch = pOriginalBranch->m_iParentBranchID;
  aeVec3 vParentDirection (0, 1, 0);

  if (iParentBranch >= 0)
  {
    const aeBranch* pParent = &m_Branches[iParentBranch];
    
    const aeVec3 vPrevOrigin = pParent->m_Nodes[pOriginalBranch->m_uiParentBranchNodeID - 1].m_vPosition;
    vParentDirection = (vOrigin - vPrevOrigin).GetNormalized ();
  }

  const aeUInt32 uiNewBranchID = GetFreeBranch ();
  aeBranch* pNewBranch = &m_Branches[uiNewBranchID];

  aeMatrix mRotation, mTransI, mTrans, mFinal;
  mTransI.SetTranslationMatrix (-vOrigin);
  mTrans.SetTranslationMatrix (vOrigin);
  mRotation.SetRotationMatrix (vParentDirection, fRotation);
  
  mFinal = mTrans * mRotation * mTransI;

  *pNewBranch = *pOriginalBranch;
  pNewBranch->m_iUmbrellaBuddyID = iBranchBuddy;
  pNewBranch->m_fUmbrellaBranchRotation = fRotation;

  for (aeUInt32 n = 0; n < pNewBranch->m_Nodes.size (); ++n)
    pNewBranch->m_Nodes[n].m_vPosition = mFinal * pNewBranch->m_Nodes[n].m_vPosition;

  return uiNewBranchID;
}

void aeTree::InsertBranchNode (aeUInt32 uiParentBranch, aeUInt32 uiStartNode, const aeSpawnNodeDesc& desc, aeBranchNodeRandomData& NodeRD, float fDistAtStartNode, float fBranchlessPartEnd, bool bReverse)
{
  const aeBranch* pParentBranch = &m_Branches[uiParentBranch];
  const aeSpawnNodeDesc& ParentBND = m_Descriptor.m_BranchTypes[pParentBranch->m_Type];
  const float fParentSegmentLength = ParentBND.m_iSegmentLengthCM / 100.0f;

  aeMath::SetRandSeed (NodeRD.m_SpawnNodesRD);
  aeUInt32 uiBranches = aeMath::Rand (desc.m_uiMinBranches, desc.m_uiMaxBranches + 1);

  AE_CHECK_DEV (uiBranches >= desc.m_uiMinBranches, "Number of Branches to spawn (%i) is smaller than minimum number of branches (%i). This should not happen.", uiBranches, desc.m_uiMinBranches);

  aeArray<float> BranchPos; BranchPos.reserve (uiBranches);
  aeArray<bool> Used; Used.reserve (uiBranches);
  for (aeUInt32 b = 0; b < uiBranches; ++b)
    Used.push_back (false);

  aeMath::SetRandSeed (NodeRD.m_NodePlacementRD.GetRandomNumber ());

  for (aeUInt32 b = 0; b < uiBranches; ++b)
  {
    aeInt32 iIndex = aeMath::Rand (uiBranches);
    while (Used[iIndex])
      iIndex = aeMath::Rand (uiBranches);

    Used[iIndex] = true;

    float fOffset = (desc.m_fNodeHeight / uiBranches) * 0.5f;
    float fDist = fOffset + (desc.m_fNodeHeight / uiBranches) * iIndex;

    if (fDistAtStartNode + fDist <= fBranchlessPartEnd)
      BranchPos.push_back (fDist);
  }

  // in which direction to spawn the next branch
  float fSpawnAngle = 0.0f;
  float fSpawnAngleStep = 360.0f / uiBranches;
  const float fMaxRotationalDeviation = 180.0f / BranchPos.size ();

  if (m_Descriptor.m_bLeafCardMode)
    fSpawnAngleStep = 180.0f;

  if (desc.m_BranchTypeMode == aeBranchTypeMode::Umbrella)
  {
    aeBranchRandomData BranchRD = NodeRD.GetBranchRD ();
    const aeInt32 iFirstBranch = InsertOneBranch (uiParentBranch, uiStartNode, desc.CreateBranchDesc (BranchRD, fMaxRotationalDeviation), BranchPos[0], fSpawnAngle, desc, BranchRD, bReverse);

    if (iFirstBranch >= 0)
    {
      aeInt32 iLastBranchID = iFirstBranch;

      for (aeUInt32 b = 1; b < uiBranches; ++b)
      {
        aeBranchRandomData BranchRD = NodeRD.GetBranchRD ();

        // the next branch will be spawned in another direction
        fSpawnAngle += fSpawnAngleStep;

        iLastBranchID = DuplicateBranch (iFirstBranch, fSpawnAngle, iLastBranchID);
      }

      m_Branches[iFirstBranch].m_fUmbrellaBranchRotation = 0.0f;
      m_Branches[iFirstBranch].m_iUmbrellaBuddyID = iLastBranchID;
    }
  }
  else
  {
    // go through all branches that should be spawned
    for (aeUInt32 b = 0; b < BranchPos.size (); ++b)
    {
      aeBranchRandomData BranchRD = NodeRD.GetBranchRD ();
      InsertOneBranch (uiParentBranch, uiStartNode, desc.CreateBranchDesc (BranchRD, fMaxRotationalDeviation), BranchPos[b], fSpawnAngle, desc, BranchRD, bReverse);

      // the next branch will be spawned in another direction
      fSpawnAngle += fSpawnAngleStep;
    }
  }
}

static bool SkipNodes_Reverse (aeInt32& uiNode, float& fDistance, aeBranch* pBranch, float fSkipDistance, float fMaxDistance, aeUInt8 cRed, aeUInt8 cGreen, aeUInt8 cBlue)
{
  float fSkipped = 0.0f;

  if (fDistance > fMaxDistance)
    return false;

  while (uiNode > 0)
  {
    if (fSkipped >= fSkipDistance)
      return true;

    --uiNode;

    const float fNodeDist = (pBranch->m_Nodes[uiNode].m_vPosition - pBranch->m_Nodes[uiNode + 1].m_vPosition).GetLength ();

    fDistance += fNodeDist;
    fSkipped += fNodeDist;

    if (fDistance > fMaxDistance)
      return false;
  }

  return false;
}

void aeTree::SpawnSubBranches_Reverse (aeUInt32 uiParentBranch, const aeSpawnNodeDesc& desc1, const aeSpawnNodeDesc& desc2, const aeSpawnNodeDesc& desc3)
{
  aeBranch* pBranch = &m_Branches[uiParentBranch];
  const aeSpawnNodeDesc& ParentDesc = m_Descriptor.m_BranchTypes[pBranch->m_Type];

  const float fBranchlessPart = pBranch->m_fBranchLength - ParentDesc.m_fBranchlessPartABS;

  aeInt32 uiNode = pBranch->m_Nodes.size () - 1;
  float fAccumDist = 0.01f;

  // skip all nodes that are in the branchless part
  {
    if (!SkipNodes_Reverse (uiNode, fAccumDist, pBranch, ParentDesc.m_fBranchlessPartEndABS - fAccumDist, fBranchlessPart, 96, 96, 96))
      goto end;
  }

  aeInt32 uiLastNode = uiNode + 1;
  while (uiNode > 0)
  {
    // don't want to end up in an endless loop
    if (uiLastNode == uiNode)
    {
      --uiNode;

      if ((uiNode >= 0) && (uiNode < (aeInt32) pBranch->m_Nodes.size () - 1))
        fAccumDist += (pBranch->m_Nodes[uiNode].m_vPosition - pBranch->m_Nodes[uiNode+1].m_vPosition).GetLength ();
    }

    uiLastNode = uiNode;

    const aeSpawnNodeDesc* pCurDesc = NULL;

    aeBranchNodeRandomData NodeRD = pBranch->m_RandomData.GetBranchNodeRD ();

    {
      const aeUInt8 uiCurRelPos = 100 - (aeUInt8) ((fAccumDist / pBranch->m_fBranchLength) * 100.0f);

      const aeSpawnNodeDesc* pCandidates[3];
      aeUInt32 uiCandidates = 0;

      // if a branch is manually created and it wants to grow certain sub-types, then do so
      // if a branch is procedurally created and the description allows a certain sub-type, then grow it
      // however only grow the sub-type, if it is in the specified range of the parent branch
      if ((desc1.m_bUsed && ParentDesc.m_bAllowSubType[0]) && (desc1.m_uiLowerBound < uiCurRelPos) && (uiCurRelPos < desc1.m_uiUpperBound))
        pCandidates[uiCandidates++] = &desc1;
      if ((desc2.m_bUsed && ParentDesc.m_bAllowSubType[1]) && (desc2.m_uiLowerBound < uiCurRelPos) && (uiCurRelPos < desc2.m_uiUpperBound))
        pCandidates[uiCandidates++] = &desc2;
      if ((desc3.m_bUsed && ParentDesc.m_bAllowSubType[2]) && (desc3.m_uiLowerBound < uiCurRelPos) && (uiCurRelPos < desc3.m_uiUpperBound))
        pCandidates[uiCandidates++] = &desc3;

      // if none of the three possible candidates is active in the current branch area,
      // increase the accumulated distance and node counter and skip this node
      if (uiCandidates == 0)
        continue;

      aeMath::SetRandSeed (NodeRD.m_BranchCandidateRD.GetRandomNumber ());
      pCurDesc = pCandidates[aeMath::Rand (uiCandidates)];
    }

    // skip all nodes that are in the 'free space before node' range
    if (!SkipNodes_Reverse (uiNode, fAccumDist, pBranch, pCurDesc->m_fNodeSpacingAfter, fBranchlessPart, 0, 0, 200))
      goto end;

    AE_CHECK_DEV (uiNode > 0, "1");

    InsertBranchNode (uiParentBranch, uiNode, *pCurDesc, NodeRD, fAccumDist, fBranchlessPart, true);

    // skip all nodes in the 'free space after node' range
    if (!SkipNodes_Reverse (uiNode, fAccumDist, pBranch, pCurDesc->m_fNodeHeight, fBranchlessPart, 0, 200, 0))
      goto end;
    if (!SkipNodes_Reverse (uiNode, fAccumDist, pBranch, pCurDesc->m_fNodeSpacingBefore, fBranchlessPart, 200, 0, 0))
      goto end;
  }

  return;

end:
  SkipNodes_Reverse (uiNode, fAccumDist, pBranch, 1000, 1000, 96, 96, 96);
}



