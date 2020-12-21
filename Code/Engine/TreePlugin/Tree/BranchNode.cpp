#include "PCH.h"
#include "BranchNode.h"

aeBranchNode::aeBranchNode ()
{
  aeMemory::FillWithZeros (this, sizeof (aeBranchNode));

  m_fThickness = 0.1f;
  m_iSegments = -1;
  m_fTexCoordV = 0.0f;
  m_bHasChildBranches = false;
}

void aeBranchNode::Save (aeStreamOut& s)
{
  aeUInt8 uiVersion = 1;
  s << uiVersion;

  aeUInt8 uiColor = 255;

  s << m_vPosition;
  s << m_fThickness;
  s << m_iSegments;
  s << m_bHasChildBranches;
  s << uiColor;
  s << uiColor;
  s << uiColor;
  s << m_fTexCoordV;
  aeVec3 m_vWindFactor(0.0f);
  s << m_vWindFactor;
}

void aeBranchNode::Load (aeStreamIn& s)
{
  aeUInt8 uiVersion;
  s >> uiVersion;

  aeUInt8 uiColor;

  s >> m_vPosition;
  s >> m_fThickness;
  s >> m_iSegments;
  s >> m_bHasChildBranches;
  s >> uiColor;
  s >> uiColor;
  s >> uiColor;
  s >> m_fTexCoordV;
  aeVec3 m_vWindFactor (0.0f);
  s >> m_vWindFactor;
}