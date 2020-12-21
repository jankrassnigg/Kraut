// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#include "Main.h"
#include "RenderAPI_Impl.h"



namespace AE_NS_GRAPHICS
{

  bool ProjectOnScreen (const aeMatrix& TransformationMatrix, const aeUInt32 uiViewport[4], const aeVec3& vPoint, aeVec3& out_vScreenPos)
  {
    float w = 1.0f;
    aeVec3 vProjected = TransformationMatrix.TransformWithWComponent (vPoint, &w);

    if (w == 0.0f) 
      return (false);

    vProjected /= w;
    vProjected *= 0.5f;
    vProjected += aeVec3 (0.5f);

    out_vScreenPos.x = vProjected.x * uiViewport[2] + uiViewport[0];
    out_vScreenPos.y = vProjected.y * uiViewport[3] + uiViewport[1];
    out_vScreenPos.z = vProjected.z;

    return(true);
  }

  bool ProjectOnScreen (const aeMatrix& TransformationMatrix, const aeUInt32 uiViewport[4], const aeVec3* vPoints, aeVec3* out_vScreenPos, aeUInt32 uiPoints)
  {
    aeVec3 vProjected;
    bool bNoError = true;

    for (aeUInt32 i = 0; i < uiPoints; ++i)
    {
      float w = 1.0f;
      vProjected = TransformationMatrix.TransformWithWComponent (vPoints[i], &w);

      if (w == 0.0f) 
      {
        bNoError = false;
        out_vScreenPos[i] = aeVec3 (0.0f);
        continue;
      }				

      vProjected /= w;
      vProjected *= 0.5f;
      vProjected += aeVec3 (0.5f);

      out_vScreenPos[i].x = vProjected.x * uiViewport[2] + uiViewport[0];
      out_vScreenPos[i].y = vProjected.y * uiViewport[3] + uiViewport[1];
      out_vScreenPos[i].z = vProjected.z;
    }

    return (bNoError);
  }

  bool UnProjectFromScreen (const aeMatrix& InverseTransformationMatrix, const aeUInt32 uiViewport[4], const aeVec3& vScreenPos, aeVec3& out_Point)
  {
    aeVec3 vSP = vScreenPos;
    float wSP = 1.0f;

    /* Map x and y from window coordinates */
    vSP.x = (vSP.x - uiViewport[0]) / uiViewport[2];
    vSP.y = (vSP.y - uiViewport[1]) / uiViewport[3];

    /* Map to range -1 to 1 */
    vSP = (vSP * 2.0f) - aeVec3 (1.0f);

    aeVec3 vPoint = InverseTransformationMatrix.TransformWithWComponent (vSP, &wSP);

    if (wSP == 0.0f)
      return (false);

    out_Point = vPoint / wSP;

    return (true);
  }

  bool UnProjectFromScreen (const aeMatrix& InverseTransformationMatrix, const aeUInt32 uiViewport[4], const aeVec3* vScreenPos, aeVec3* out_Points, aeUInt32 uiPoints)
  {
    aeVec3 vSP;
    float wSP;

    bool bNoError = true;

    for (aeUInt32 i = 0; i < uiPoints; ++i)
    {
      vSP = vScreenPos[uiPoints];
      wSP = 1.0f;

      /* Map x and y from window coordinates */
      vSP.x = (vSP.x - uiViewport[0]) / uiViewport[2];
      vSP.y = (vSP.y - uiViewport[1]) / uiViewport[3];

      /* Map to range -1 to 1 */
      vSP = (vSP * 2.0f) - aeVec3 (1.0f);

      aeVec3 vPoint = InverseTransformationMatrix.TransformWithWComponent (vSP, &wSP);

      if (wSP == 0.0f)
      {
        bNoError = false;
        continue;
      }

      out_Points[uiPoints] = vPoint / wSP;

    }

    return (bNoError);
  }
}