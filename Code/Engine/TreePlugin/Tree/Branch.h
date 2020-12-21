#ifndef TREEPLUGIN_BRANCH_H
#define TREEPLUGIN_BRANCH_H

#include "BranchLOD.h"
#include "RandomData.h"

class aeTree;

struct aeBranchDesc
{
  aeBranchDesc ();

  aeBranchRandomData m_RandomData;

  aeBranchType::Enum m_Type;

  aeInt32 m_iParentBranchID;
  aeInt32 m_iParentBranchNodeID;

  float m_fBranchLength;              //!< The length the branch
  float m_fBranchThickness;           //!< The (relative) thickness of the branch to its parent branch

  float m_fBranchAngle;
  float m_fRotationalDeviation;
  float m_fFrondColorVariation;
  aeUInt8 m_uiFrondTextureVariation;

  aeVec3 m_vStartPosition;            //!< At which point in space the branch starts
  aeVec3 m_vStartDirection;           //!< At which direction the branch starts growing
  aeVec3 m_vGrowDirection;            //!< Which general grow direction the branch has
  aeVec3 m_vGrowDirection2;           //!< Which general grow direction the branch has
  float m_fGrowDir2UUsageDistance;    //!< At which absolute distance to use the second grow direction
};

class aeBranch
{
public:
  aeBranch ();
  ~aeBranch ();

  void Reset (void);

  void Save (aeStreamOut& s);
  void Load (aeStreamIn& s);

  aeVec3 GetDirectionAtNode (aeUInt32 uiNode) const;

  aeTree* m_pTree;

  aeUInt32 m_uiPickID;

  aeInt32 m_iParentBranchID;
  aeUInt32 m_uiParentBranchNodeID;
  
  aeInt32 m_iUmbrellaBuddyID;
  float m_fUmbrellaBranchRotation;

  aeDeque<aeBranchNode> m_Nodes;
  aeBranchLod m_LODs[aeLod::ENUM_COUNT];

  bool m_bManuallyCreated;

  float m_fBranchLength;
  float m_fThickness;
  float m_fFrondColorVariation; // factor from 0 to 1, how much to use the texture color or the variation color
  aeUInt8 m_uiFrondTextureVariation; // if a texture contains more than one frond texture (atlas), this value describes which one to use

  aeVec3 m_vLeafUpDirection;

  aeBranchType::Enum m_Type;

  aeBranchRandomData m_RandomData;

  void TransformNode (aeUInt32 uiNode, const aeMatrix& mTransform, float fSelectionStrength, bool bLocal);

  const aeMatrix GetNodeTransformation (aeUInt32 uiNode);

  void SetDeleted (bool bDelete);
  bool IsDeleted (void) const { return m_bDeleted; }

private:
  bool m_bDeleted;
};


#endif

