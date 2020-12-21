#ifndef AE_TREEPLUGIN_RANDOMDATA_H
#define AE_TREEPLUGIN_RANDOMDATA_H

#include "../Basics/Base.h"

struct aeBranchRandomData;

struct aeBranchNodeRandomData
{
  aeUInt32 m_SpawnNodesRD;
  aeRandomNumberGenerator m_NodePlacementRD;
  aeRandomNumberGenerator m_BranchCandidateRD;
  aeRandomNumberGenerator m_BranchRD;

  aeBranchRandomData GetBranchRD ();
};

struct aeBranchRandomData
{
  aeUInt32 m_LengthRD;
  aeUInt32 m_ThicknessRD;
  aeUInt32 m_AngleDeviationRD;
  aeUInt32 m_RotationalDeviationRD;
  aeUInt32 m_FrondColorVariationRD;
  aeRandomNumberGenerator m_ColorVariation;
  aeRandomNumberGenerator m_GrowDirChange;
  aeRandomNumberGenerator m_BranchNodeRD;
  aeUInt32 m_TargetDirRD;
  aeRandomNumberGenerator m_CurGrowTargetDirRD;
  aeUInt32 m_uiLeafDeviationRD;

  aeBranchNodeRandomData GetBranchNodeRD ();
};

#endif

