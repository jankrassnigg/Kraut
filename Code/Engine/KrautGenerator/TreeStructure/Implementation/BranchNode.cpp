#include <PCH.h>

#include <KrautGenerator/TreeStructure/BranchNode.h>
#include <KrautGenerator/TreeStructure/BranchStats.h>

namespace Kraut
{
  void BranchNode::Save(aeStreamOut& s)
  {
    aeUInt8 uiVersion = 1;
    s << uiVersion;

    aeUInt8 uiColor = 255;
    aeInt16 m_iSegments = -1;

    s << m_vPosition;
    s << m_fThickness;
    s << m_iSegments;
    s << m_bHasChildBranches;
    s << uiColor;
    s << uiColor;
    s << uiColor;
    s << m_fTexCoordV;
    aeVec3 m_vWindFactor(0.0f);
    s << m_vWindFactor;
  }

  void BranchNode::Load(aeStreamIn& s)
  {
    aeUInt8 uiVersion;
    s >> uiVersion;

    aeUInt8 uiColor;
    aeInt16 m_iSegments = -1;

    s >> m_vPosition;
    s >> m_fThickness;
    s >> m_iSegments;
    s >> m_bHasChildBranches;
    s >> uiColor;
    s >> uiColor;
    s >> uiColor;
    s >> m_fTexCoordV;
    aeVec3 m_vWindFactor(0.0f);
    s >> m_vWindFactor;
  }
} // namespace Kraut
