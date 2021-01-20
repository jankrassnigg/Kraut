#include "Picking.h"
#include <KrautGraphics/RenderAPI/Main.h>
#include <KrautGraphics/Configuration/VariableRegistry.h>



namespace AE_NS_EDITORBASICS
{
  using namespace AE_NS_GRAPHICS;

  const aeVec3 aeEditorPicking::GetPickingDirection (aeInt32 x, aeInt32 y)
  {
    aeInt32 uiResolutionX = 0;
    aeInt32 uiResolutionY = 0;

    aeVariableRegistry::RetrieveInt ("system/graphics/resolution_x", uiResolutionX);
    aeVariableRegistry::RetrieveInt ("system/graphics/resolution_y", uiResolutionY);

    float fN, fF;
    const aeMatrix mC = aeRenderAPI::GetMatrix_WorldToCamera ().GetMatrix ();
    const aeMatrix mP = aeRenderAPI::GetMatrix_CameraToScreen ().GetMatrix ();

    aeUInt32 g_3DWindowViewport[4];
    aeRenderAPI::getViewportAndDepthRange (g_3DWindowViewport[0], g_3DWindowViewport[1], g_3DWindowViewport[2], g_3DWindowViewport[3], fN, fF);

    const aeMatrix mWindowTransformation = mP * mC;
    const aeMatrix mInvOCP = mWindowTransformation.GetInverse ();

	  const aeVec3 vScreenPos0 ((float) x, (float) (uiResolutionY - y), 0.2f);
    const aeVec3 vScreenPos1 ((float) x, (float) (uiResolutionY - y), 0.5f);

    aeVec3 vPos0;
    aeVec3 vPos1;

    if (!UnProjectFromScreen (mInvOCP, g_3DWindowViewport, vScreenPos0, vPos0))
      return aeVec3 (0.0f);
    if (!UnProjectFromScreen (mInvOCP, g_3DWindowViewport, vScreenPos1, vPos1))
      return aeVec3 (0.0f);

    return (vPos1 - vPos0).GetNormalized ();
  }
}
