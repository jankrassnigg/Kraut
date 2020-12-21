#include "../Base.h"
#include <KrautFoundation/Math/IncludeAll.h>
#include <KrautGraphics/glim/glim.h>

namespace AE_NS_EDITORBASICS
{
  using namespace NS_GLIM;
  using namespace AE_NS_FOUNDATION;

  class AE_EDITORBASICS_DLL aeRenderShapes
  {
  public:

    // Basic Shapes
    static void CreateCylinder (GLIM_BATCH& glim, const aeMatrix& mTransform, float fLength, float fRadius, aeUInt16 uiSegments);
    static void CreateCone (GLIM_BATCH& glim, const aeMatrix& mTransform, float fLength, float fRadius, aeUInt16 uiSegments);
    static void CreateArrow (GLIM_BATCH& glim, const aeMatrix& mTransform, float fLength, float fRadius, aeUInt16 uiSegments);
    static void CreateTorus (GLIM_BATCH& glim, const aeMatrix& mTransform, float fRadius, float fRingRadius, aeUInt16 uiSegments, aeUInt16 uiRingSegments);

    // Gizmo Shapes
    static void CreateTranslationGizmoCorner (GLIM_BATCH& glim, const aeMatrix& mTransform, float fSize);
    static void CreateTranslationGizmo (GLIM_BATCH& glim, const aeMatrix& mTransform, float fArrowLength, float fArrowThickness, aeUInt32 uiPickID, bool bX, bool bY, bool bZ);
    static void CreateRotationGizmo (GLIM_BATCH& glim, const aeMatrix& mTransform, float fRadius, float fRingRadius, aeUInt32 uiPickID, bool bX, bool bY, bool bZ);
  };

}

