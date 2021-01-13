#include <PCH.h>

#include <KrautGenerator/Description/LodDesc.h>

namespace Kraut
{
  LodDesc::LodDesc()
  {
    for (aeUInt32 i = 0; i < Kraut::BranchGeometryType::ENUM_COUNT; ++i)
    {
      m_AllowTypes[(Kraut::BranchGeometryType::Enum)i].RaiseFlags(0xFFFFFFFF);
    }
  }

  void LodDesc::Save(aeStreamOut& s)
  {
    aeUInt8 uiVersion = 7;

    s << uiVersion;

    s << m_fTipDetail;
    s << m_fCurvatureThreshold;
    s << m_fThicknessThreshold;
    s << m_fVertexRingDetail;

    // Version 3
    s << m_AllowTypes[Kraut::BranchGeometryType::Branch].GetData();

    // Version 4
    s << m_iMaxFrondDetail;
    s << m_iFrondDetailReduction;

    // Version 5
    s << m_AllowTypes[Kraut::BranchGeometryType::Frond].GetData();
    s << m_uiLodDistance;

    // Version 6
    s << m_AllowTypes[Kraut::BranchGeometryType::Leaf].GetData();

    // Version 7
    {
      aeInt8 uiLodMode = m_Mode;
      s << uiLodMode;
    }
  }

  void LodDesc::Load(aeStreamIn& s)
  {
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
      m_AllowTypes[Kraut::BranchGeometryType::Branch].SetFlags(l);
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
      m_AllowTypes[Kraut::BranchGeometryType::Frond].SetFlags(l);
      s >> m_uiLodDistance;
    }

    if (uiVersion >= 6)
    {
      aeUInt32 l;
      s >> l;
      m_AllowTypes[Kraut::BranchGeometryType::Leaf].SetFlags(l);
    }

    if (uiVersion >= 7)
    {
      aeInt8 uiLodMode;
      s >> uiLodMode;
      m_Mode = (Kraut::LodMode::Enum)uiLodMode;
    }
  }
} // namespace Kraut
