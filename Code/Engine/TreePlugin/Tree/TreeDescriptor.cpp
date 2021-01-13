#include "PCH.h"

#include "../Basics/Plugin.h"
#include "../Physics/Forces.h"
#include "../Rendering/AmbientOcclusion.h"
#include "../Tree/Tree.h"
#include "../Undo/TreeUndo.h"

static void ResetLodDesc(Kraut::LodDesc& desc, aeLod::Enum lod)
{
  desc.m_Mode = Kraut::LodMode::Full;
  desc.m_fTipDetail = 0.04f;
  desc.m_fCurvatureThreshold = 5.0f;
  desc.m_fThicknessThreshold = 0.2f;
  desc.m_fVertexRingDetail = 0.2f;


  desc.m_iMaxFrondDetail = 32;
  desc.m_iFrondDetailReduction = 0;
  desc.m_uiLodDistance = 0;
  desc.m_BranchSpikeTipMode = Kraut::BranchSpikeTipMode::FullDetail;

  switch (lod)
  {
    case aeLod::None:
      desc.m_fCurvatureThreshold = 0.0f;
      desc.m_fThicknessThreshold = 1.0f / 100.0f;
      desc.m_fTipDetail = 0.03f;
      desc.m_fVertexRingDetail = 0.04f;
      break;
    case aeLod::Lod0:
      desc.m_fCurvatureThreshold = 2.0f;
      desc.m_fThicknessThreshold = 5.0f / 100.0f;
      desc.m_fTipDetail = 0.04f;
      desc.m_fVertexRingDetail = 0.2f;
      desc.m_uiLodDistance = 10;
      break;
    case aeLod::Lod1:
      desc.m_fCurvatureThreshold = 5.0f;
      desc.m_fThicknessThreshold = 10.0f / 100.0f;
      desc.m_fTipDetail = 0.10f;
      desc.m_fVertexRingDetail = 0.4f;
      desc.m_iMaxFrondDetail = 6;
      desc.m_iFrondDetailReduction = 1;
      desc.m_uiLodDistance = 20;
      break;
    case aeLod::Lod2:
      desc.m_fCurvatureThreshold = 10.0f;
      desc.m_fThicknessThreshold = 15.0f / 100.0f;
      desc.m_fTipDetail = 0.20f;
      desc.m_fVertexRingDetail = 0.6f;
      desc.m_iMaxFrondDetail = 4;
      desc.m_iFrondDetailReduction = 2;
      desc.m_uiLodDistance = 30;
      desc.m_BranchSpikeTipMode = Kraut::BranchSpikeTipMode::SingleTriangle;
      break;
    case aeLod::Lod3:
      desc.m_Mode = Kraut::LodMode::TwoQuads;
      desc.m_fCurvatureThreshold = 15.0f;
      desc.m_fThicknessThreshold = 20.0f / 100.0f;
      desc.m_fTipDetail = 0.30f;
      desc.m_fVertexRingDetail = 0.8f;
      desc.m_iMaxFrondDetail = 2;
      desc.m_iFrondDetailReduction = 3;
      desc.m_uiLodDistance = 40;
      desc.m_BranchSpikeTipMode = Kraut::BranchSpikeTipMode::Hole;
      break;
    case aeLod::Lod4:
      desc.m_Mode = Kraut::LodMode::Disabled;
      desc.m_fCurvatureThreshold = 20.0f;
      desc.m_fThicknessThreshold = 50.0f / 200.0f;
      desc.m_fTipDetail = 0.40f;
      desc.m_fVertexRingDetail = 1.0f;
      desc.m_iMaxFrondDetail = 0;
      desc.m_iFrondDetailReduction = 4;
      desc.m_uiLodDistance = 1000;
      desc.m_BranchSpikeTipMode = Kraut::BranchSpikeTipMode::Hole;
      break;
  }
}

aeTreeDescriptor::aeTreeDescriptor()
{
  Reset();
}

void aeTreeDescriptor::Reset(void)
{
  RemoveAllForces();

  m_fLodCrossFadeTransition = 0.1f;
  m_uiLeafCardTexelDilation = 4;
  m_LeafCardMipmaps = aeLeafCardMipmapResolution::None;
  m_LeafCardResolution = aeLeafCardResolution::Tex512;
  m_fBBoxAdjustment = 0.0f;
  m_StructureDesc.m_bLeafCardMode = false;
  m_StructureDesc.m_bGrowProceduralTrunks = true;
  m_StructureDesc.m_uiRandomSeed = 0;
  m_bSnapshotFromAbove = false;
  m_iImpostorResolution = 2;
  m_fExportScale = 1.0f;

  for (aeUInt32 n = 0; n < Kraut::BranchType::ENUM_COUNT; ++n)
  {
    //m_StructureDesc.m_BranchTypes[n].m_Type = (Kraut::BranchType::Enum)n;
    m_StructureDesc.m_BranchTypes[n].Reset();
  }

  m_StructureDesc.m_BranchTypes[Kraut::BranchType::Trunk1].m_bUsed = true;
  m_StructureDesc.m_BranchTypes[Kraut::BranchType::Trunk1].m_uiMinBranchLengthInCM = 500;
  m_StructureDesc.m_BranchTypes[Kraut::BranchType::Trunk1].m_uiMaxBranchLengthInCM = 500;
  m_StructureDesc.m_BranchTypes[Kraut::BranchType::Trunk1].m_uiMinBranches = 1;
  m_StructureDesc.m_BranchTypes[Kraut::BranchType::Trunk1].m_uiMaxBranches = 1;

  m_StructureDesc.m_BranchTypes[Kraut::BranchType::MainBranches1].m_bUsed = true;

  for (aeUInt32 n = 0; n < aeLod::ENUM_COUNT; ++n)
    ResetLodDesc(m_LodData[n], (aeLod::Enum)n);

  m_bUseAO = true;
  m_fAOSampleSize = 0.3f;
  m_fAOContrast = 1.0f;
}

void aeTreeDescriptor::Save(aeStreamOut& s)
{
  aeUInt32 uiVersion = 17;

  s << uiVersion;
  s << m_StructureDesc.m_uiRandomSeed;
  s << m_StructureDesc.m_bLeafCardMode;

  // Version 8: Removed
  // s << m_bManualMode;

  ClampLodValues(aeLod::None);

  for (aeUInt32 i = 0; i < aeLod::ENUM_COUNT; ++i)
    m_LodData[i].Save(s);

  // Version 8:
  aeUInt8 uiCount = Kraut::BranchType::ENUM_COUNT;
  s << uiCount;

  for (aeUInt32 i = 0; i < Kraut::BranchType::ENUM_COUNT; ++i)
  {
    m_StructureDesc.m_BranchTypes[i].Save(s);
  }

  // Version 7
  s << m_StructureDesc.m_bGrowProceduralTrunks;

  // Version 9
  {
    s << m_bSnapshotFromAbove;
    aeUInt32 i = 0;
    s << i; //m_uiNumLeaves;
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
  aeAOGrid::Save(s);

  // Version 15
  s << m_fExportScale;

  // Version 16
  {
    const aeUInt32 uiForces = m_Forces.size();

    s << uiForces;

    for (aeUInt32 i = 0; i < uiForces; ++i)
    {
      m_Forces[i]->Save(s);
    }
  }

  // Version 17
  s << m_fLodCrossFadeTransition;
}

void aeTreeDescriptor::Load(aeStreamIn& s)
{
  Reset();

  aeUInt32 uiVersion = 1;
  s >> uiVersion;
  s >> m_StructureDesc.m_uiRandomSeed;

  if (uiVersion >= 3)
    s >> m_StructureDesc.m_bLeafCardMode;

  if ((uiVersion >= 5) && (uiVersion <= 7))
  {
    bool m_bManualMode;
    s >> m_bManualMode;
  }

  if (uiVersion >= 2)
  {
    for (aeUInt32 i = 0; i < aeLod::ENUM_COUNT; ++i)
    {
      ResetLodDesc(m_LodData[i], (aeLod::Enum)i);
      m_LodData[i].Load(s);
    }

    ClampLodValues(aeLod::None);
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
    if (i < Kraut::BranchType::ENUM_COUNT)
      m_StructureDesc.m_BranchTypes[i].Load(s);
    else
    {
      Kraut::SpawnNodeDesc dummy;
      dummy.Load(s);
    }

    if ((uiVersion < 6) && (i == 0)) // Version 6 added 2 trunk types
      i += 2;
  }

  if (uiVersion >= 7)
  {
    s >> m_StructureDesc.m_bGrowProceduralTrunks;
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
    m_LeafCardResolution = (aeLeafCardResolution::Enum)uiLeafCardResolution;

    aeUInt8 uiLeafCardMipmaps;
    s >> uiLeafCardMipmaps;
    m_LeafCardMipmaps = (aeLeafCardMipmapResolution::Enum)uiLeafCardMipmaps;
  }

  if (uiVersion >= 13)
  {
    s >> m_bUseAO;
    s >> m_fAOSampleSize;
    s >> m_fAOContrast;
  }

  if (uiVersion >= 14)
  {
    aeAOGrid::Load(s);
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
      m_Forces.push_back(new aeForce());
      m_Forces.back()->Load(s);
    }
  }

  if (uiVersion >= 17)
  {
    s >> m_fLodCrossFadeTransition;
  }
}

static void PushLodSettingsDown(aeUInt32 uiStartLod, Kraut::LodDesc* m_LodData)
{
  for (aeUInt32 i = uiStartLod + 1; i < aeLod::ENUM_COUNT; ++i)
  {
    m_LodData[i].m_Mode = (Kraut::LodMode::Enum)aeMath::Max(m_LodData[i].m_Mode, m_LodData[i - 1].m_Mode);
    m_LodData[i].m_Mode = aeMath::Clamp(m_LodData[i].m_Mode, Kraut::LodMode::Full, Kraut::LodMode::Disabled);

    m_LodData[i].m_uiLodDistance = aeMath::Max(m_LodData[i].m_uiLodDistance, m_LodData[i - 1].m_uiLodDistance);
    m_LodData[i].m_fCurvatureThreshold = aeMath::Max(m_LodData[i].m_fCurvatureThreshold, m_LodData[i - 1].m_fCurvatureThreshold);
    m_LodData[i].m_fThicknessThreshold = aeMath::Max(m_LodData[i].m_fThicknessThreshold, m_LodData[i - 1].m_fThicknessThreshold);
    m_LodData[i].m_fTipDetail = aeMath::Max(m_LodData[i].m_fTipDetail, m_LodData[i - 1].m_fTipDetail);
    m_LodData[i].m_fVertexRingDetail = aeMath::Max(m_LodData[i].m_fVertexRingDetail, m_LodData[i - 1].m_fVertexRingDetail);

    for (aeUInt32 mt = 0; mt < Kraut::BranchGeometryType::ENUM_COUNT; ++mt)
      m_LodData[i].m_AllowTypes[(Kraut::BranchGeometryType::Enum)mt].SetFlags(m_LodData[i].m_AllowTypes[(Kraut::BranchGeometryType::Enum)mt].GetData() & m_LodData[i - 1].m_AllowTypes[(Kraut::BranchGeometryType::Enum)mt].GetData());

    m_LodData[i].m_iMaxFrondDetail = aeMath::Min(m_LodData[i].m_iMaxFrondDetail, m_LodData[i - 1].m_iMaxFrondDetail);
    m_LodData[i].m_iFrondDetailReduction = aeMath::Max(m_LodData[i].m_iFrondDetailReduction, m_LodData[i - 1].m_iFrondDetailReduction);
  }
}

static void PushLodSettingsUp(aeUInt32 uiStartLod, Kraut::LodDesc* m_LodData)
{
  for (aeInt32 i = uiStartLod - 1; i >= 0; --i)
  {
    m_LodData[i].m_Mode = (Kraut::LodMode::Enum)aeMath::Min(m_LodData[i].m_Mode, m_LodData[i + 1].m_Mode);
    m_LodData[i].m_Mode = aeMath::Clamp(m_LodData[i].m_Mode, Kraut::LodMode::Full, Kraut::LodMode::Disabled);

    m_LodData[i].m_uiLodDistance = aeMath::Min(m_LodData[i].m_uiLodDistance, m_LodData[i + 1].m_uiLodDistance);
    m_LodData[i].m_fCurvatureThreshold = aeMath::Min(m_LodData[i].m_fCurvatureThreshold, m_LodData[i + 1].m_fCurvatureThreshold);
    m_LodData[i].m_fThicknessThreshold = aeMath::Min(m_LodData[i].m_fThicknessThreshold, m_LodData[i + 1].m_fThicknessThreshold);
    m_LodData[i].m_fTipDetail = aeMath::Min(m_LodData[i].m_fTipDetail, m_LodData[i + 1].m_fTipDetail);
    m_LodData[i].m_fVertexRingDetail = aeMath::Min(m_LodData[i].m_fVertexRingDetail, m_LodData[i + 1].m_fVertexRingDetail);

    for (aeUInt32 mt = 0; mt < Kraut::BranchGeometryType::ENUM_COUNT; ++mt)
      m_LodData[i].m_AllowTypes[(Kraut::BranchGeometryType::Enum)mt].SetFlags(m_LodData[i].m_AllowTypes[(Kraut::BranchGeometryType::Enum)mt].GetData() | m_LodData[i + 1].m_AllowTypes[(Kraut::BranchGeometryType::Enum)mt].GetData());

    m_LodData[i].m_iMaxFrondDetail = aeMath::Max(m_LodData[i].m_iMaxFrondDetail, m_LodData[i + 1].m_iMaxFrondDetail);
    m_LodData[i].m_iFrondDetailReduction = aeMath::Min(m_LodData[i].m_iFrondDetailReduction, m_LodData[i + 1].m_iFrondDetailReduction);
  }
}

void aeTreeDescriptor::ClampLodValues(aeLod::Enum CurLod)
{
  if (aeUndo::IsStoringUndoOperation())
    return;

  PushLodSettingsDown(CurLod, m_LodData);
  PushLodSettingsUp(CurLod, m_LodData);

  PushLodSettingsDown(aeLod::None, m_LodData);
  PushLodSettingsUp(aeLod::None, m_LodData);
}
