#pragma once

#include <KrautGenerator/Description/DescriptionEnums.h>
#include <KrautGenerator/Infrastructure/BoundingBox.h>
#include <KrautGenerator/Mesh/Mesh.h>

namespace Kraut
{
  struct KRAUT_DLL BranchMesh
  {
    Kraut::BoundingBox m_BoundingBox[Kraut::BranchGeometryType::ENUM_COUNT];
    Kraut::Mesh m_Mesh[Kraut::BranchGeometryType::ENUM_COUNT];
  };

} // namespace Kraut
