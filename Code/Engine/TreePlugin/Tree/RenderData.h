#ifndef TREEPLUGIN_BRANCHLOD_H
#define TREEPLUGIN_BRANCHLOD_H

#include <KrautGenerator/Description/DescriptionEnums.h>
#include <KrautGraphics/VertexArray/Declarations.h>
#include <TreePlugin/Basics/Enums.h>

using namespace AE_NS_GRAPHICS;

struct aeBranchRenderData
{
  aeVertexArrayResourceHandle m_hMeshes[Kraut::BranchGeometryType::ENUM_COUNT];
};

struct aeTreeRenderData
{
  void Clear()
  {
    m_BranchRenderData.clear();
  }

  aeDeque<aeBranchRenderData> m_BranchRenderData;
};

struct aeBranchRenderInfo
{
  ~aeBranchRenderInfo();

  aeUInt32 m_uiPickID = 0;
};

#endif
