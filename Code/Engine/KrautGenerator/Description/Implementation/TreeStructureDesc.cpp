#include "PCH.h"

#include <KrautGenerator/Description/TreeStructureDesc.h>

namespace Kraut
{
  TreeStructureDesc::TreeStructureDesc()
  {
    for (aeUInt32 i = 0; i < Kraut::BranchType::ENUM_COUNT; ++i)
    {
      m_BranchTypes[i].m_Type = (Kraut::BranchType::Enum)i;
    }
  }

  TreeStructureDesc::~TreeStructureDesc() = default;

  float TreeStructureDesc::GetBoundingBoxSizeIncrease() const
  {
    float result = 0.0f;

    for (aeUInt32 typeIdx = 0; typeIdx < Kraut::BranchType::ENUM_COUNT; ++typeIdx)
    {
      const Kraut::SpawnNodeDesc& snd = m_BranchTypes[typeIdx];

      if (!snd.m_bUsed)
        continue;

      // branch geometry is ignored here

      if (snd.m_bEnable[Kraut::BranchGeometryType::Frond])
      {
        // fronds are usually not oriented in a way that they take the maximum area (in the bounding box)
        // so scale them down by 0.5 to get a better approximation (otherwise the bbox gets too big)
        result = aeMath::Max(result, snd.m_fFrondHeight * 0.5f);
        result = aeMath::Max(result, snd.m_fFrondWidth * 0.5f);
      }

      if (snd.m_bEnable[Kraut::BranchGeometryType::Leaf])
      {
        result = aeMath::Max(result, snd.m_fLeafSize);
      }
    }

    return result;
  }


} // namespace Kraut
