#include "PCH.h"

#include "../Basics/Plugin.h"
#include "../Undo/TreeUndo.h"
#include "../Tree/Tree.h"
#include "../Rendering/AmbientOcclusion.h"
#include "../Physics/Forces.h"


aeTreeDescriptor::aeTreeDescriptor ()
{
  Reset ();
}

void aeTreeDescriptor::Reset (void)
{
  RemoveAllForces ();

  m_fLodCrossFadeTransition = 0.1f;
  m_uiLeafCardTexelDilation = 4;
  m_LeafCardMipmaps = aeLeafCardMipmapResolution::None;
  m_LeafCardResolution = aeLeafCardResolution::Tex512;
  m_fBBoxAdjustment = 0.0f;
  m_bLeafCardMode = false;
  m_bGrowProceduralTrunks = true;
  m_uiRandomSeed = 0;
  m_bSnapshotFromAbove = false;
  m_iImpostorResolution = 2;
  m_fExportScale = 1.0f;

  for (aeUInt32 n = 0; n < aeBranchType::ENUM_COUNT; ++n)
  {
    m_BranchTypes[n].m_Type = (aeBranchType::Enum) n;
    m_BranchTypes[n].Reset ();
  }

  m_BranchTypes[aeBranchType::Trunk1].m_bUsed = true;
  m_BranchTypes[aeBranchType::Trunk1].m_uiMinBranchLengthInCM = 500;
  m_BranchTypes[aeBranchType::Trunk1].m_uiMaxBranchLengthInCM = 500;
  m_BranchTypes[aeBranchType::Trunk1].m_uiMinBranches = 1;
  m_BranchTypes[aeBranchType::Trunk1].m_uiMaxBranches = 1;

  m_BranchTypes[aeBranchType::MainBranches1].m_bUsed = true;

  for (aeUInt32 n = 0; n < aeLod::ENUM_COUNT; ++n)
    m_LodData[n].Reset ((aeLod::Enum) n);

  m_bUseAO = true;
  m_fAOSampleSize = 0.3f;
  m_fAOContrast = 1.0f;
}

void aeTreeDescriptor::Save (aeStreamOut& s)
{
  aeUInt32 uiVersion = 17;

  s << uiVersion;
  s << m_uiRandomSeed;
  s << m_bLeafCardMode;

  // Version 8: Removed
  // s << m_bManualMode;

  ClampLodValues (aeLod::None);

  for (aeUInt32 i = 0; i < aeLod::ENUM_COUNT; ++i)
    m_LodData[i].Save (s);

  // Version 8:
  aeUInt8 uiCount = aeBranchType::ENUM_COUNT;
  s << uiCount;

  for (aeUInt32 i = 0; i < aeBranchType::ENUM_COUNT; ++i)
  {
    m_BranchTypes[i].Save (s);
  }

  // Version 7
  s << m_bGrowProceduralTrunks;

  // Version 9
  {
    s << m_bSnapshotFromAbove;
    aeUInt32 i = 0;     s << i;//m_uiNumLeaves;
  }

  // Version 10
  s << m_fBBoxAdjustment;

  // Version 11
  s << m_iImpostorResolution;

  // Version 12
  s << m_uiLeafCardTexelDilation;

  const aeUInt8 uiLeafCardResolution = m_LeafCardResolution;
  s << uiLeafCardResolution;

  const aeUInt8 uiLeafCardMipmaps = m_LeafCardMipmaps;
  s << uiLeafCardMipmaps;

  // Version 13
  s << m_bUseAO;
  s << m_fAOSampleSize;
  s << m_fAOContrast;

  // Version 14
  aeAOGrid::Save (s);

  // Version 15
  s << m_fExportScale;

  // Version 16
  {
    const aeUInt32 uiForces = m_Forces.size ();

    s << uiForces;

    for (aeUInt32 i = 0; i < uiForces; ++i)
    {
      m_Forces[i]->Save (s);
    }
  }

  // Version 17
  s << m_fLodCrossFadeTransition;
}

void aeTreeDescriptor::Load (aeStreamIn& s)
{
  Reset ();

  aeUInt32 uiVersion = 1;
  s >> uiVersion;
  s >> m_uiRandomSeed;

  if (uiVersion >= 3)
    s >> m_bLeafCardMode;

  if ((uiVersion >= 5) && (uiVersion <= 7))
  {
    bool m_bManualMode;
    s >> m_bManualMode;
  }

  if (uiVersion >= 2)
  {
    for (aeUInt32 i = 0; i < aeLod::ENUM_COUNT; ++i)
      m_LodData[i].Load (s, (aeLod::Enum) i);

    ClampLodValues (aeLod::None);
  }

  if (uiVersion <= 3)
  {
    float fDummy;
    s >> fDummy; //m_fMinThickness;
    s >> fDummy; //m_fMaxThickness;
  }

  aeUInt8 uiCount = 15; // number of types in Version < 8
  if (uiVersion >= 8)
    s >> uiCount;

  for (aeUInt32 i = 0; i < uiCount; ++i)
  {
    if (i < aeBranchType::ENUM_COUNT)
      m_BranchTypes[i].Load (s);
    else
    {
      aeSpawnNodeDesc dummy;
      dummy.Load (s);
    }

    if ((uiVersion < 6) && (i == 0)) // Version 6 added 2 trunk types
      i += 2;
  }

  if (uiVersion >= 7)
  {
    s >> m_bGrowProceduralTrunks;
  }

  if (uiVersion >= 9)
  {
    aeUInt32 i;
    s >> m_bSnapshotFromAbove;
    s >> i; //s >> m_uiNumLeaves;
  }

  if (uiVersion >= 10)
  {
    s >> m_fBBoxAdjustment;
  }

  if (uiVersion >= 11)
  {
    s >> m_iImpostorResolution;
  }

  if (uiVersion >= 12)
  {
    s >> m_uiLeafCardTexelDilation;

    aeUInt8 uiLeafCardResolution;
    s >> uiLeafCardResolution;
    m_LeafCardResolution = (aeLeafCardResolution::Enum) uiLeafCardResolution;

    aeUInt8 uiLeafCardMipmaps;
    s >> uiLeafCardMipmaps;
    m_LeafCardMipmaps = (aeLeafCardMipmapResolution::Enum) uiLeafCardMipmaps;
  }

  if (uiVersion >= 13)
  {
    s >> m_bUseAO;
    s >> m_fAOSampleSize;
    s >> m_fAOContrast;
  }

  if (uiVersion >= 14)
  {
    aeAOGrid::Load (s);
  }

  if (uiVersion >= 15)
  {
    s >> m_fExportScale;
  }

  if (uiVersion >= 16)
  {
    aeUInt32 uiForces = 0;

    s >> uiForces;

    for (aeUInt32 i = 0; i < uiForces; ++i)
    {
      m_Forces.push_back (new aeForce ());
      m_Forces.back ()->Load (s);
    }
  }

  if (uiVersion >= 17)
  {
    s >> m_fLodCrossFadeTransition;
  }
}

static void PushLodSettingsDown (aeUInt32 uiStartLod, aeLodData* m_LodData)
{
  for (aeUInt32 i = uiStartLod + 1; i < aeLod::ENUM_COUNT; ++i)
  {
    m_LodData[i].m_Mode = (aeLodMode::Enum) aeMath::Max (m_LodData[i].m_Mode, m_LodData[i - 1].m_Mode);
    m_LodData[i].m_Mode = aeMath::Clamp (m_LodData[i].m_Mode, aeLodMode::Full, aeLodMode::Disabled);

    m_LodData[i].m_uiLodDistance       = aeMath::Max (m_LodData[i].m_uiLodDistance,       m_LodData[i - 1].m_uiLodDistance);
    m_LodData[i].m_fCurvatureThreshold = aeMath::Max (m_LodData[i].m_fCurvatureThreshold, m_LodData[i - 1].m_fCurvatureThreshold);
    m_LodData[i].m_fThicknessThreshold = aeMath::Max (m_LodData[i].m_fThicknessThreshold, m_LodData[i - 1].m_fThicknessThreshold);
    m_LodData[i].m_fTipDetail          = aeMath::Max (m_LodData[i].m_fTipDetail,          m_LodData[i - 1].m_fTipDetail);
    m_LodData[i].m_fVertexRingDetail   = aeMath::Max (m_LodData[i].m_fVertexRingDetail,   m_LodData[i - 1].m_fVertexRingDetail);

    for (aeUInt32 mt = 0; mt < aeMeshType::ENUM_COUNT; ++mt)
      m_LodData[i].m_AllowTypes[(aeMeshType::Enum) mt].SetFlags (m_LodData[i].m_AllowTypes[(aeMeshType::Enum) mt].GetData () & m_LodData[i - 1].m_AllowTypes[(aeMeshType::Enum) mt].GetData ());

    m_LodData[i].m_iMaxFrondDetail       = aeMath::Min (m_LodData[i].m_iMaxFrondDetail,       m_LodData[i - 1].m_iMaxFrondDetail);
    m_LodData[i].m_iFrondDetailReduction = aeMath::Max (m_LodData[i].m_iFrondDetailReduction, m_LodData[i - 1].m_iFrondDetailReduction);
  }
}

static void PushLodSettingsUp (aeUInt32 uiStartLod, aeLodData* m_LodData)
{
  for (aeInt32 i = uiStartLod - 1; i >= 0; --i)
  {
    m_LodData[i].m_Mode = (aeLodMode::Enum) aeMath::Min (m_LodData[i].m_Mode, m_LodData[i + 1].m_Mode);
    m_LodData[i].m_Mode = aeMath::Clamp (m_LodData[i].m_Mode, aeLodMode::Full, aeLodMode::Disabled);

    m_LodData[i].m_uiLodDistance       = aeMath::Min (m_LodData[i].m_uiLodDistance,       m_LodData[i + 1].m_uiLodDistance);
    m_LodData[i].m_fCurvatureThreshold = aeMath::Min (m_LodData[i].m_fCurvatureThreshold, m_LodData[i + 1].m_fCurvatureThreshold);
    m_LodData[i].m_fThicknessThreshold = aeMath::Min (m_LodData[i].m_fThicknessThreshold, m_LodData[i + 1].m_fThicknessThreshold);
    m_LodData[i].m_fTipDetail          = aeMath::Min (m_LodData[i].m_fTipDetail,          m_LodData[i + 1].m_fTipDetail);
    m_LodData[i].m_fVertexRingDetail   = aeMath::Min (m_LodData[i].m_fVertexRingDetail,   m_LodData[i + 1].m_fVertexRingDetail);

    for (aeUInt32 mt = 0; mt < aeMeshType::ENUM_COUNT; ++mt)
      m_LodData[i].m_AllowTypes[(aeMeshType::Enum) mt].SetFlags (m_LodData[i].m_AllowTypes[(aeMeshType::Enum) mt].GetData () | m_LodData[i + 1].m_AllowTypes[(aeMeshType::Enum) mt].GetData ());

    m_LodData[i].m_iMaxFrondDetail       = aeMath::Max (m_LodData[i].m_iMaxFrondDetail,       m_LodData[i + 1].m_iMaxFrondDetail);
    m_LodData[i].m_iFrondDetailReduction = aeMath::Min (m_LodData[i].m_iFrondDetailReduction, m_LodData[i + 1].m_iFrondDetailReduction);
  }  
}

void aeTreeDescriptor::ClampLodValues (aeLod::Enum CurLod)
{
  if (aeUndo::IsStoringUndoOperation ())
    return;

  PushLodSettingsDown (CurLod, m_LodData);
  PushLodSettingsUp   (CurLod, m_LodData);

  PushLodSettingsDown (aeLod::None, m_LodData);
  PushLodSettingsUp   (aeLod::None, m_LodData);
}



