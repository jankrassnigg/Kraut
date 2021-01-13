#pragma once

#include <KrautFoundation/Math/Vec3.h>
#include <KrautGenerator/KrautGeneratorDLL.h>

namespace Kraut
{
  using namespace AE_NS_FOUNDATION;

  struct KRAUT_DLL BoundingBox
  {
    BoundingBox();

    void SetInvalid(void);
    bool IsInvalid(void) const;
    aeVec3 GetSize(void) const;
    void AddBoundary(const aeVec3& vAdd);

    void ExpandToInclude(const aeVec3& v);
    aeVec3 GetCenter(void) const;

    aeVec3 m_vMin;
    aeVec3 m_vMax;
  };

} // namespace Kraut
