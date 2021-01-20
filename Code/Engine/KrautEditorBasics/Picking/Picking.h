#ifndef AE_EDITORBASICS_PICKING_H
#define AE_EDITORBASICS_PICKING_H

#include <KrautEditorBasics/Base.h>
#include <KrautFoundation/Math/Vec3.h>

namespace AE_NS_EDITORBASICS
{
  using namespace AE_NS_FOUNDATION;

  class AE_EDITORBASICS_DLL aeEditorPicking
  {
  public:
    static const aeVec3 GetPickingDirection(aeInt32 x, aeInt32 y);
  };

} // namespace AE_NS_EDITORBASICS

#endif
