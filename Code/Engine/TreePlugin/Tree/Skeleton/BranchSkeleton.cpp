#include "PCH.h"

#include "../Tree.h"
#include "../../Physics/Forces.h"
#include <KrautEditorBasics/Plugin.h>
#include <KrautFoundation/Math/Plane.h>


using namespace AE_NS_EDITORBASICS;

bool IsObstructed (const aeVec3& vPos, const aeVec3& vPosTo, float& out_fDistance);
aeVec3 FindLeastObstructedDirection (const aeVec3& vPos, const aeVec3& vPosTo, aeUInt32 uiMaxDeviation, float& out_fDistance);

bool aeTree::ComputeBranchThickness (aeUInt32 uiBranch)
{
  aeBranch& Branch = m_Branches[uiBranch];

  if (Branch.IsDeleted ())
    return false;

  float fThickness = Branch.m_fThickness;
  const float fMinThickness = m_Descriptor.m_BranchTypes[Branch.m_Type].m_uiMinBranchThicknessInCM / 100.0f;

  if (Branch.m_iParentBranchID != -1)
  {
    const float fParentThickness = m_Branches[Branch.m_iParentBranchID].m_Nodes[Branch.m_uiParentBranchNodeID].m_fThickness;

    if (fParentThickness < fMinThickness)
    {
      Branch.m_Nodes.clear ();
      return false;
    }

    fThickness = aeMath::Clamp (fThickness, fMinThickness, fParentThickness);
  }

  Branch.m_fThickness = fThickness;

  float fLengthAtNode = 0.0f;
  Branch.m_Nodes[0].m_fThickness = fThickness;

  const aeSpawnNodeDesc& bnd = m_Descriptor.m_BranchTypes[Branch.m_Type];

  float fThicknessFalloff = 1.0f;
  float fStepFalloff = (1.0f - bnd.m_fRoundnessFactor) / (Branch.m_Nodes.size ());

  for (aeUInt32 i = 0; i < Branch.m_Nodes.size (); ++i)
  {
    fThicknessFalloff -= fStepFalloff;

    const float fContour = m_Descriptor.m_BranchTypes[Branch.m_Type].m_BranchContour.GetValueAt ((float) i / (float) (Branch.m_Nodes.size ()-1));

    Branch.m_Nodes[i].m_fThickness = fThickness * fThicknessFalloff * fContour;
  }

  return true;
}

aeVec3 aeTree::ComputeLeafUpDirection (const aeBranch& Branch, const aeSpawnNodeDesc& bnd, const aeVec3& vGrowDirection)
{
  // if this branch has a parent branch, check its direction where the leaf is spawned
  aeVec3 vParentDirection (0, 1, 0);
  if (Branch.m_iParentBranchID >= 0) 
  {
    aeInt32 iNodeID = Branch.m_uiParentBranchNodeID;
    
    if (iNodeID == 0)
      iNodeID = 1;

    const aeVec3 vPos1 = m_Branches[Branch.m_iParentBranchID].m_Nodes[iNodeID].m_vPosition;
    const aeVec3 vPos0 = m_Branches[Branch.m_iParentBranchID].m_Nodes[iNodeID - 1].m_vPosition;

    vParentDirection = (vPos1 - vPos0).GetNormalized ();
  }

  const aeVec3 vGrowDir = vGrowDirection.GetNormalized ();

  // find out which direction is supposed to be 'up' for this leaf
  aeVec3 vRotationalDir (0, 1, 0);

  switch (bnd.m_FrondUpOrientation)
  {
  case aeLeafOrientation::Upwards:
    vRotationalDir.SetVector (0, 1, 0);
    break;
  case aeLeafOrientation::AlongBranch:
    vRotationalDir = vParentDirection;
    break;
  case aeLeafOrientation::OrthogonalToBranch:
    if (aeMath::Abs (vGrowDir.Dot (vParentDirection)) > aeMath::CosDeg (1.0f))
      vRotationalDir = vGrowDir.GetOrthogonalVector ().GetNormalized ();
    else
      vRotationalDir = vGrowDir.Cross (vParentDirection).GetNormalized ();
    break;
  }

  AE_CHECK_DEV (vRotationalDir.IsValid (), "Leaf Up Direction is degenerate.");

  // if the leaf-up dir and the grow dir are too close (1°), just get some random (orthogonal) vector as up-vector
  if (aeMath::Abs (vGrowDir.Dot (vRotationalDir)) > aeMath::CosDeg (1.0f))
    vRotationalDir = vGrowDir.GetOrthogonalVector ().GetNormalized ();

  AE_CHECK_DEV (vRotationalDir.IsValid (), "Leaf Up Direction is degenerate.");

  if (bnd.m_uiMaxFrondOrientationDeviation > 0)
  {
    aeMath::SetRandSeed (Branch.m_RandomData.m_uiLeafDeviationRD);

    aeInt32 uiRotation = (aeInt32) aeMath::Rand (bnd.m_uiMaxFrondOrientationDeviation * 2 + 1) - (aeInt32) bnd.m_uiMaxFrondOrientationDeviation;
    
    aeQuaternion q;
    q.CreateQuaternion (vGrowDir, (float) uiRotation);

    vRotationalDir = q * vRotationalDir;
  }

  AE_CHECK_DEV (vRotationalDir.IsValid (), "Leaf Up Direction is degenerate.");

  // in leaf card mode, always orient all leaves towards the camera
  if (m_Descriptor.m_bLeafCardMode)
    vRotationalDir.SetVector (0, 0, 1);

  return vRotationalDir;
}

aeInt32 aeTree::GrowBranch (const aeBranchDesc& bd)
{
  const aeUInt32 uiBranchID = GetFreeBranch ();
  aeBranch* pBranch = &m_Branches[uiBranchID];

  pBranch->m_pTree = this;

  // register this branch for picking
  pBranch->m_uiPickID = aeEditorPlugin::RegisterPickableObject (pBranch, uiBranchID, "aeTreeBranch");

  pBranch->m_RandomData = bd.m_RandomData;

  const aeSpawnNodeDesc& BranchTypeDesc = m_Descriptor.m_BranchTypes[bd.m_Type];

  pBranch->m_Type = bd.m_Type;
  pBranch->m_fThickness = bd.m_fBranchThickness;
  pBranch->m_fFrondColorVariation = bd.m_fFrondColorVariation;
  pBranch->m_uiFrondTextureVariation = bd.m_uiFrondTextureVariation;
  pBranch->m_iParentBranchID = bd.m_iParentBranchID;
  pBranch->m_uiParentBranchNodeID = bd.m_iParentBranchNodeID;

  aeVec3 vCurGrowDirection = bd.m_vStartDirection;
  aeVec3 vCurGrowTargetDir = bd.m_vGrowDirection;
  aeVec3 vCurNodePos = bd.m_vStartPosition;
  pBranch->m_fBranchLength = 0.0f;

  // if this branch is a leaf, only allow it to grow in a fixed plane
  // it needs an 'up' vector for this
  pBranch->m_vLeafUpDirection = ComputeLeafUpDirection (*pBranch, BranchTypeDesc, vCurGrowDirection);

  aeVec3 vLeafPlaneNormal;

  if (aeMath::Abs (bd.m_vStartDirection.Dot (bd.m_vGrowDirection)) > 0.999f)
  {
    vLeafPlaneNormal = bd.m_vStartDirection.GetOrthogonalVector ();
  }
  else
    vLeafPlaneNormal = bd.m_vStartDirection.Cross (bd.m_vGrowDirection);

  // if this branch is a leaf, it may only grow in this plane and not deviate from it
  const aePlane LeafPlane (vLeafPlaneNormal.GetNormalized (), aeVec3::ZeroVector ());

  AE_CHECK_DEV (LeafPlane.m_vNormal.IsValid (), "Leaf Plane is degenerate.");

  bool bChangeDir = true;

  const float fMaxDirectionChange = (BranchTypeDesc.m_fGrowMaxDirChangePerSegment / 10.0f) * BranchTypeDesc.m_iSegmentLengthCM;

  aeVec3 vTargetDir1 = bd.m_vGrowDirection;
  aeVec3 vTargetDir2 = bd.m_vGrowDirection2;

  // keep on growing until the target length is reached
  while (pBranch->m_fBranchLength < bd.m_fBranchLength)
  {
    aeBranchNode n;
    n.m_vPosition = vCurNodePos;

    AE_CHECK_DEV (n.m_vPosition.IsValid (), "GrowBranch: Skeleton Position is degenerate.");

    pBranch->m_Nodes.push_back (n);

    if ((bChangeDir) || (aeMath::ACosDeg (vCurGrowDirection.Dot (vCurGrowTargetDir)) < fMaxDirectionChange))
    {
      aeMath::SetRandSeed (pBranch->m_RandomData.m_CurGrowTargetDirRD.GetRandomNumber ());

      if (pBranch->m_fBranchLength >= bd.m_fGrowDir2UUsageDistance)
        vCurGrowTargetDir.CreateRandomVector (vTargetDir2, (aeUInt8) BranchTypeDesc.m_fGrowMaxTargetDirDeviation);
      else
        vCurGrowTargetDir.CreateRandomVector (vTargetDir1, (aeUInt8) BranchTypeDesc.m_fGrowMaxTargetDirDeviation);

      bChangeDir = false;

      if ((g_Globals.s_bDoPhysicsSimulation) && (BranchTypeDesc.m_bDoPhysicalSimulation))
      {
        const float fRemainingBranchLength = bd.m_fBranchLength - pBranch->m_fBranchLength;
        const float fRayCastLength = aeMath::Min (BranchTypeDesc.m_fPhysicsLookAhead, fRemainingBranchLength);
        const aeVec3 vRayCastTarget = vCurNodePos + vCurGrowTargetDir * fRayCastLength;

        float fDist;
        if (IsObstructed (vCurNodePos, vRayCastTarget, fDist))
          vCurGrowTargetDir = FindLeastObstructedDirection (vCurNodePos, vRayCastTarget, (aeUInt32) BranchTypeDesc.m_fPhysicsEvasionAngle, fDist);
      }

      // if this branch is a leaf, restrict its growth to the leaf plane
      if (BranchTypeDesc.m_bRestrictGrowthToFrondPlane)
        vCurGrowTargetDir = LeafPlane.ProjectOntoPlane (vCurGrowTargetDir).GetNormalized ();
    }

    aeQuaternion qSegmentRotation;
    qSegmentRotation.CreateQuaternion (vCurGrowDirection, vCurGrowTargetDir);
    aeVec3 vAxis = qSegmentRotation.GetRotationAxis ();

    AE_CHECK_DEV (vAxis.IsValid (), "Rotation Axis is degenerate.");

    // if this branch is a leaf, restrict its growth to the leaf plane
    if (BranchTypeDesc.m_bRestrictGrowthToFrondPlane)
    {
      if (vAxis.Dot (LeafPlane.m_vNormal) > 0.0f)
        vAxis = LeafPlane.m_vNormal;
      else
        vAxis = -LeafPlane.m_vNormal;
      //vCurGrowDirection = LeafPlane.ProjectOntoPlane (vCurGrowDirection);
    }

    float fAngle = qSegmentRotation.GetRotationAngle ();

    if (fAngle < 0.5f)
    {
      bChangeDir = true;
    }

    aeMath::SetRandSeed (pBranch->m_RandomData.m_GrowDirChange.GetRandomNumber ());
    float fRealAngle = /*aeMath::Randf (*/aeMath::Min (fAngle, fMaxDirectionChange);//);

    aeMatrix mSegmentRotation;
    mSegmentRotation.SetRotationMatrix (vAxis, fRealAngle);
    

    vCurGrowDirection = mSegmentRotation * vCurGrowDirection;
    vCurGrowDirection.Normalize ();

    const aeVec3 vInfluence = GetAverageForceAt (vCurNodePos, pBranch->m_Type);

    vCurGrowTargetDir += vInfluence;
    vCurGrowTargetDir.Normalize ();

    vTargetDir1 += vInfluence;
    vTargetDir1.Normalize ();
    vTargetDir2 += vInfluence;
    vTargetDir2.Normalize ();

    //vCurGrowDirection += vInfluence;
    vCurGrowDirection.Normalize ();

    AE_CHECK_DEV (vCurGrowDirection.IsValid (), "Grow Direction is degenerate.");

    const float fSegmentLength = BranchTypeDesc.m_iSegmentLengthCM / 100.0f; // Segment Length in Meters

    // if this branch uses obstacle avoidance and the general switch for phyiscal simulation is enabled, try to grow around obstacles
    if ((g_Globals.s_bDoPhysicsSimulation) && (BranchTypeDesc.m_bDoPhysicalSimulation))
    {
      const float fRemainingBranchLength = bd.m_fBranchLength - pBranch->m_fBranchLength;
      const float fRayCastLength = aeMath::Min (BranchTypeDesc.m_fPhysicsLookAhead, fRemainingBranchLength);
      const aeVec3 vRayCastTarget = vCurNodePos + vCurGrowDirection * fRayCastLength;

      float fDist;

      if (IsObstructed (vCurNodePos, vRayCastTarget, fDist))
      {
        if (fDist < 0.31f)
          break;

        if (fDist < 0.51f)
        {
          vCurGrowDirection = FindLeastObstructedDirection (vCurNodePos, vRayCastTarget, (aeUInt32) BranchTypeDesc.m_fPhysicsEvasionAngle, fDist);

          AE_CHECK_DEV (vCurGrowDirection.IsValid (), "Grow Direction is degenerate.");
        }
        else
        {
          vCurGrowTargetDir = FindLeastObstructedDirection (vCurNodePos, vRayCastTarget, (aeUInt32) BranchTypeDesc.m_fPhysicsEvasionAngle, fDist);

          // if this branch is a leaf, restrict its growth to the leaf plane
          if (BranchTypeDesc.m_bRestrictGrowthToFrondPlane)
            vCurGrowTargetDir = LeafPlane.ProjectOntoPlane (vCurGrowTargetDir).GetNormalized ();
        }
      }
      
      vCurGrowDirection.Normalize ();
    }

    AE_CHECK_DEV (vCurGrowDirection.IsValid (), "Grow Direction is degenerate.");

    vCurNodePos += vCurGrowDirection * fSegmentLength;

    pBranch->m_fBranchLength += fSegmentLength;
  }

  if (pBranch->m_Nodes.size () < 4)
  {
    pBranch->SetDeleted (true);
    return -1;
  }

  if (!ComputeBranchThickness (uiBranchID))
  {
    pBranch->SetDeleted (true);
    return -1;
  }

  CreateBranchCollisionCapsules (uiBranchID);
  return uiBranchID;
}

aeVec3 aeBranch::GetDirectionAtNode (aeUInt32 uiNode) const
{
  if ((uiNode >= m_Nodes.size ()) || (m_Nodes.size () < 2))
    return aeVec3::ZeroVector ();

  if (uiNode < m_Nodes.size () - 1)
    return (m_Nodes[uiNode+1].m_vPosition - m_Nodes[uiNode].m_vPosition).GetNormalized ();

  return (m_Nodes[uiNode].m_vPosition - m_Nodes[uiNode-1].m_vPosition).GetNormalized ();
}
