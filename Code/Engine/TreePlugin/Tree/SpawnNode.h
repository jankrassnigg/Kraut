#ifndef AE_TREEPLUGIN_SPAWNNODE_H
#define AE_TREEPLUGIN_SPAWNNODE_H

#include "Branch.h"
#include "../Other/Curve.h"

struct aeSpawnNodeDesc
{
  aeSpawnNodeDesc ();
  aeBranchDesc CreateBranchDesc (aeBranchRandomData& rd, float fMaxAbsRotationalDeviation = 180.0f) const;

  void Reset (void);

  void Save (aeStreamOut& s);
  void Load (aeStreamIn& s);

  // ************** General **************

  aeBranchType::Enum m_Type;

  bool m_bUsed;                       //!< Whether this node desc is used at all
  bool m_bEnable[aeMeshType::ENUM_COUNT]; //!< Which mesh types are in use

  // ************** Spawn Node **************

  aeUInt8 m_uiMinBranches;            //!< The minimum number of branches per node
  aeUInt8 m_uiMaxBranches;            //!< The maximum number of branches per node

  float m_fNodeHeight;                //!< How high/long (absolute value) the a branch node of this type is
  float m_fNodeSpacingBefore;         //!< How much space between the previous node and this node (maximum of previous node's m_fNodeSpacingAfter and this)
  float m_fNodeSpacingAfter;          //!< How much space between this node and the next (maximum of next node's m_fNodeSpacingBefore and this)

  aeBranchTypeMode::Enum m_BranchTypeMode;

  // ************** Branch **************

  float m_fBranchlessPartABS;         //!< How much (absolute) space to the parent branch before this branch spawns children
  float m_fBranchlessPartEndABS;      //!< How much (absolute) space at the end of the branch, which will contain no subbranches

  aeCurve m_fBranchLengthScale;       //!< Scale of each child branch's length (relatively spaced along this branch's length)
  aeCurve m_MaxBranchLengthParentScale;
  aeUInt16 m_uiMinBranchLengthInCM;   //!< The minimum length of a branch in cm
  aeUInt16 m_uiMaxBranchLengthInCM;   //!< The maximum length of a branch in cm

  aeUInt16 m_uiMinBranchThicknessInCM;//!< The maximum thickness of a branch in cm
  aeUInt16 m_uiMaxBranchThicknessInCM;//!< The maximum thickness of a branch in cm

  aeCurve m_BranchContour;            //!< The contour of the branch

  float m_fMaxRotationalDeviation;    //!< The maximum deviation (in degrees) from the fixed rotation
  float m_fBranchAngle;               //!< The angle at which to branch a child
  float m_fMaxBranchAngleDeviation;   //!< The maximum deviation from the base branch angle

  bool m_bTargetDirRelative;          //!< Whether the target dir is relative to the parent branch, or in global space
  aeTargetDir::Enum m_TargetDirection;//!< The general grow direction for branches
  aeTargetDir::Enum m_TargetDirection2;//!< The general grow direction for branches at a later stage
  aeTargetDir2Usage::Enum m_TargetDir2Uage; //!< How to use the second target direction
  float m_fTargetDir2Usage;           //!< At which time / distance to switch to the second target dir
  float m_fMaxTargetDirDeviation;     //!< How much the branch may deviate from its general grow direction

  float m_fGrowMaxTargetDirDeviation; //!< How much the branch can change its general grow direction
  float m_fGrowMaxDirChangePerSegment;//!< How much the branch can change its grow direction each segment

  float m_fRoundnessFactor;           //!< How round or pointy the branch is

  bool  m_bActAsObstacle;              //!< Whether this branch shall insert collision shapes, against which other branches will collide
  bool  m_bDoPhysicalSimulation;      //!< Whether to avoid other branches (and act as an obstacle)
  float m_fPhysicsLookAhead;          //!< How many meters the raycasts will look ahead for collisions
  float m_fPhysicsEvasionAngle;       //!< How many degrees a branch may turn to evade a collision

  aeUInt8 m_uiLowerBound;             //!< Percentage: In which range (of the parent) this branch may be started
  aeUInt8 m_uiUpperBound;

  bool m_bAllowSubType[3];            //!< Which of the 3 sub-types are allowed to be branched off of this branch
  bool m_bVisible;                    //!< Whether the meshes from this type will be rendered (not serialized).

  aeInt8 m_iSegmentLengthCM;          //!< How long each segment will be (in cm), thus how detailed the branch is

  aeHybridString<128> m_sTexture[aeMeshType::ENUM_COUNT];
  //aeHybridString<128> m_sTextureN[aeMeshType::ENUM_COUNT];
  aeTextureResourceHandle m_hTexture[aeMeshType::ENUM_COUNT];
  aeTextureResourceHandle m_hTextureN[aeMeshType::ENUM_COUNT];
  aeUInt8 m_uiVariationColor[aeMeshType::ENUM_COUNT][4];   //!< The target color for frond color variations.


  // ************** Fronds **************

  enum FrondContourMode
  {
    Full,
    Symetric,
    InverseSymetric
  };

  bool m_bRestrictGrowthToFrondPlane; //!< If set, branches will only grow in the frond plane (2D growth -> up / down)
  aeUInt8 m_uiNumFronds;              //!< How many fronds shall be placed around the branch
  aeUInt8 m_uiFrondDetail;            //!< How often to tesselate the fronds

  float m_fFrondWidth;                //!< How wide the fronds shall be
  float m_fFrondHeight;               //!< How tall the fronds shall be
  aeUInt8 m_uiMaxFrondOrientationDeviation;  //!< Max degrees to deviate from the original leaf orientation (0 - 180 in bot directions)
  aeLeafOrientation::Enum m_FrondUpOrientation;      //!< How to orient the leaves (if this branch is a leaf type)
  FrondContourMode m_FrondContourMode;//!< If true, the contour curve describes only one half of the frond shape and the other half will be symetric.
  
  aeCurve m_FrondContour;             //!< The contour of the fronds
  aeCurve m_FrondWidth;               //!< Scale along the branch to modify the frond width
  aeCurve m_FrondHeight;              //!< Scale along the branch to modify the frond height

  float m_fTextureRepeat;             //!< At which interval to repeat the textures. 0 == stretch across branch
  bool  m_bAlignFrondsOnSurface;      //!< Whether fronds shall spawn at the branch center or surface
  


  // ************** Billboard Leaves **************

  bool m_bBillboardLeaves;            //!< Whether the leaves are billboards or static.
  float m_fLeafSize;                  //!< How large the billboards shall generally be.
  float m_fLeafInterval;              //!< Every how many units along the branch a billboard shall be placed. 0 == only one at the tip.
  aeCurve m_LeafScale;                //!< The scale of the billboards along the branch

  // ************** Flares **************

  float GetFlareWidthAt (float fPosAlongBranch, float fNodeRadius) const;
  float GetFlareDistance (float fPosAlongBranch, float fNodeRadius, float fFlareWidth, float fAtAngle) const;

  aeUInt8 m_uiFlares;
  float m_fFlareWidth;
  aeCurve m_FlareWidthCurve;
  float m_fFlareRotation;
  bool m_bRotateTexCoords;
};


#endif

