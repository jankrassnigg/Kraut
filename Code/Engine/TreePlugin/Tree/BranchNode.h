#ifndef TREEPLUGIN_BRANCHNODE_H
#define TREEPLUGIN_BRANCHNODE_H

#include "../Basics/Base.h"

struct aeBranchNode
{
  aeBranchNode ();

  void Save (aeStreamOut& s);
  void Load (aeStreamIn& s);

  aeVec3 m_vPosition;
  float m_fThickness;
  aeInt16 m_iSegments;
  bool m_bHasChildBranches;
  float m_fTexCoordV;
};


#endif

