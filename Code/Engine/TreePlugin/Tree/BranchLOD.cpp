#include "PCH.h"
#include "BranchLOD.h"

aeBranchLod::aeBranchLod ()
{
  Reset ();
}

void aeBranchLod::Reset (void)
{
  for (aeUInt32 i = 0; i < aeMeshType::ENUM_COUNT; ++i)
    m_bMeshGenerated[i] = false;

  m_bMeshGeneratedTrunkCaps = false;
  m_bMeshGeneratedBranchCaps = false;
  m_bSkeletonGenerated = false;

  m_NodeIDs.clear ();
  m_TipNodes.clear ();

  for (aeUInt32 mt = 0; mt < aeMeshType::ENUM_COUNT; ++mt)
  {
    m_SubMesh[mt].clear ();
    m_hMeshes[mt].Invalidate ();
    m_MeshBBox[mt].SetInvalid ();
  }
}


aeLodData::aeLodData ()
{
  aeMemory::FillWithZeros (this, sizeof (aeLodData));

  Reset (aeLod::Lod0);
}

void aeLodData::Reset (aeLod::Enum lod)
{
  m_Mode = aeLodMode::Full;
  m_fTipDetail = 0.04f;
  m_fCurvatureThreshold = 5.0f;
  m_fThicknessThreshold = 0.2f;
  m_fVertexRingDetail = 0.2f;

  for (aeUInt32 i = 0; i < aeMeshType::ENUM_COUNT; ++i)
    m_AllowTypes[(aeMeshType::Enum) i].RaiseFlags (0xFFFFFFFF);

  m_iMaxFrondDetail = 32;
  m_iFrondDetailReduction = 0;
  m_uiLodDistance = 0;

  switch (lod)
  {
  case aeLod::None:
    m_fCurvatureThreshold = 0.0f;
    m_fThicknessThreshold = 1.0f / 100.0f;
    m_fTipDetail = 0.03f;
    m_fVertexRingDetail = 0.04f;
    break;
  case aeLod::Lod0:
    m_fCurvatureThreshold = 2.0f;
    m_fThicknessThreshold = 5.0f / 100.0f;
    m_fTipDetail = 0.04f;
    m_fVertexRingDetail = 0.2f;
    m_uiLodDistance = 10;
    break;
  case aeLod::Lod1:
    m_fCurvatureThreshold = 5.0f;
    m_fThicknessThreshold = 10.0f / 100.0f;
    m_fTipDetail = 0.10f;
    m_fVertexRingDetail = 0.4f;
    m_iMaxFrondDetail = 6;
    m_iFrondDetailReduction = 1;
    m_uiLodDistance = 20;
    break;
  case aeLod::Lod2:
    m_fCurvatureThreshold = 10.0f;
    m_fThicknessThreshold = 15.0f / 100.0f;
    m_fTipDetail = 0.20f;
    m_fVertexRingDetail = 0.6f;
    m_iMaxFrondDetail = 4;
    m_iFrondDetailReduction = 2;
    m_uiLodDistance = 30;
    break;
  case aeLod::Lod3:
    m_Mode = aeLodMode::TwoQuads;
    m_fCurvatureThreshold = 15.0f;
    m_fThicknessThreshold = 20.0f / 100.0f;
    m_fTipDetail = 0.30f;
    m_fVertexRingDetail = 0.8f;
    m_iMaxFrondDetail = 2;
    m_iFrondDetailReduction = 3;
    m_uiLodDistance = 40;
    break;
  case aeLod::Lod4:
    m_Mode = aeLodMode::Disabled;
    m_fCurvatureThreshold = 20.0f;
    m_fThicknessThreshold = 50.0f / 200.0f;
    m_fTipDetail = 0.40f;
    m_fVertexRingDetail = 1.0f;
    m_iMaxFrondDetail = 0;
    m_iFrondDetailReduction = 4;
    m_uiLodDistance = 1000;
    break;
  }
}

void aeLodData::Save (aeStreamOut& s)
{
  aeUInt8 uiVersion = 7;

  s << uiVersion;

  s << m_fTipDetail;
  s << m_fCurvatureThreshold;
  s << m_fThicknessThreshold;
  s << m_fVertexRingDetail;

  // Version 3
  s << m_AllowTypes[aeMeshType::Branch].GetData ();

  // Version 4
  s << m_iMaxFrondDetail;
  s << m_iFrondDetailReduction;

  // Version 5
  s << m_AllowTypes[aeMeshType::Frond].GetData ();
  s << m_uiLodDistance;

  // Version 6
  s << m_AllowTypes[aeMeshType::Leaf].GetData ();

  // Version 7
  {
    aeInt8 uiLodMode = m_Mode;
    s << uiLodMode;
  }
}


void aeLodData::Load (aeStreamIn& s, aeLod::Enum lod)
{
  Reset (lod);

  aeUInt8 uiVersion = 1;

  s >> uiVersion;

  s >> m_fTipDetail;
  s >> m_fCurvatureThreshold;
  s >> m_fThicknessThreshold;

  if (uiVersion >= 2)
    s >> m_fVertexRingDetail;

  if (uiVersion >= 3)
  {
    aeUInt32 l;
    s >> l;
    m_AllowTypes[aeMeshType::Branch].SetFlags (l);
  }

  if (uiVersion >= 4)
  {
    s >> m_iMaxFrondDetail;
    s >> m_iFrondDetailReduction;
  }

  if (uiVersion >= 5)
  {
    aeUInt32 l;
    s >> l;
    m_AllowTypes[aeMeshType::Frond].SetFlags (l);
    s >> m_uiLodDistance;
  }

  if (uiVersion >= 6)
  {
    aeUInt32 l;
    s >> l;
    m_AllowTypes[aeMeshType::Leaf].SetFlags (l);
  }

  if (uiVersion >= 7)
  {
    aeInt8 uiLodMode;
    s >> uiLodMode;
    m_Mode = (aeLodMode::Enum) uiLodMode;
  }
}

