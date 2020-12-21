#include "PCH.h"
#include "Branch.h"

aeBranchDesc::aeBranchDesc ()
{
  aeMemory::FillWithZeros (this, sizeof (aeBranchDesc));

  m_iParentBranchID = -1;
  m_iParentBranchNodeID = 0;
  m_fFrondColorVariation = 0.0f;
  m_uiFrondTextureVariation = 0;
}