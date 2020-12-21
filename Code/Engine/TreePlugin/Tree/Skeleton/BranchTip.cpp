#include "PCH.h"

#include "../Tree.h"


void aeTree::GrowBranchTip (aeUInt32 uiBranch, aeLod::Enum lod)
{
  aeBranch& Branch = m_Branches[uiBranch];
  const aeSpawnNodeDesc& bnd = m_Descriptor.m_BranchTypes[Branch.m_Type];

  if (Branch.IsDeleted ())
    return;

  if (Branch.m_Nodes.size () <= 2)
    return;

  if (bnd.m_fRoundnessFactor > 0.01f)
  {
    const float fSegmentLength =  m_Descriptor.m_LodData[lod].m_fTipDetail;

    aeInt32 fSegments = aeMath::Max (1, (aeInt32) (Branch.m_Nodes.back ().m_fThickness / fSegmentLength));

    if (m_Descriptor.m_BranchTypes[Branch.m_Type].m_bEnable[aeMeshType::Branch])
      GrowTip (uiBranch, fSegments, Branch.m_Nodes.back ().m_fThickness / fSegments, lod);
  }

  if (!Branch.m_LODs[lod].m_TipNodes.empty ())
    Branch.m_LODs[lod].m_TipNodes.back ().m_fThickness = 0.001f;
}

void aeTree::GrowTip (aeUInt32 uiBranch, aeUInt32 uiSegments, float fSegmentLength, aeLod::Enum lod)
{
  aeBranch& Branch = m_Branches[uiBranch];

  if (Branch.IsDeleted ())
    return;
  if (fSegmentLength < 0.001f)
    return;

  const aeVec3 vPos = Branch.m_Nodes.back ().m_vPosition;
  const float fThickness = Branch.m_Nodes.back ().m_fThickness;

  const aeVec3 vDir = (vPos - Branch.m_Nodes[Branch.m_Nodes.size () - 2].m_vPosition).GetNormalized () * fSegmentLength;

  if ((!vDir.IsValid ()) || (vDir.IsZeroVector (0.00001f)))
    return;
  
  aeBranchNode n;
  n.m_vPosition = vPos;
  n.m_iSegments = -1;

  for (aeUInt32 i = 0; i < uiSegments; ++i)
  {
    n.m_vPosition += vDir;

    const float fFactor = ((i + 1.0f) / uiSegments);
    const float fCos =  aeMath::Sqrt(1.0f - fFactor*fFactor);

    n.m_fThickness = fCos;
    n.m_fThickness *= fThickness;

    Branch.m_LODs[lod].m_TipNodes.push_back (n);
  }
}

