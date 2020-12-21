#include "RenderShapes.h"

namespace AE_NS_EDITORBASICS
{
  void aeRenderShapes::CreateTranslationGizmoCorner (GLIM_BATCH& glim, const aeMatrix& mTransform, float fSize)
  {
    glim.Begin (GLIM_QUADS);
    
    aeVec3 vPos[4];
    vPos[0] = aeVec3 (0, 0, 0);
    vPos[1] = aeVec3 (fSize, 0, 0);
    vPos[2] = aeVec3 (fSize, fSize, 0);
    vPos[3] = aeVec3 (0, fSize, 0);

    for (int i = 0; i < 4; ++i)
    {
      vPos[i] = mTransform * vPos[i];
      glim.Vertex (vPos[i].x, vPos[i].y, vPos[i].z);
    }

    for (int i = 3; i >= 0; --i)
      glim.Vertex (vPos[i].x, vPos[i].y, vPos[i].z);

    glim.End ();
  }

  inline static void SetPickID (GLIM_BATCH& glim, aeUInt32 uiPickID, aeUInt32 uiPickSubID)
  {
    glim.Attribute4ub ("attr_PickingID", (uiPickID >> 0) & 0xFF, (uiPickID >> 8) & 0xFF, (uiPickID >> 16) & 0xFF, (uiPickID >> 24) & 0xFF);
    glim.Attribute4ub ("attr_PickingSubID", (uiPickSubID >> 0) & 0xFF, (uiPickSubID >> 8) & 0xFF, (uiPickSubID >> 16) & 0xFF, (uiPickSubID >> 24) & 0xFF);
  }

  const aeUInt8 uiGizmoColor = 150;

  void aeRenderShapes::CreateTranslationGizmo (GLIM_BATCH& glim, const aeMatrix& mTransform, float fArrowLength, float fArrowThickness, aeUInt32 uiPickID, bool bX, bool bY, bool bZ)
  {
    aeMatrix mR, mT;
    mT.SetTranslationMatrix (aeVec3 (0, fArrowLength * 0.5f, 0));

    if (bY && bZ)
    {
      glim.Attribute4ub ("attr_Color", uiGizmoColor, uiGizmoColor, 0);
      SetPickID (glim, uiPickID, 3);
      CreateTranslationGizmoCorner (glim, mTransform, fArrowLength / 2.0f);
    }

    if (bX && bZ)
    {
      glim.Attribute4ub ("attr_Color", uiGizmoColor, uiGizmoColor, 0);
      SetPickID (glim, uiPickID, 4);
      mR.SetRotationMatrixX (90);
      CreateTranslationGizmoCorner (glim, mTransform * mR, fArrowLength / 2.0f);
    }

    if (bX && bY)
    {
      glim.Attribute4ub ("attr_Color", uiGizmoColor, uiGizmoColor, 0);
      SetPickID (glim, uiPickID, 5);
      mR.SetRotationMatrixY (-90);
      CreateTranslationGizmoCorner (glim, mTransform * mR, fArrowLength / 2.0f);
    }

    if (bY)
    {
      glim.Attribute4ub ("attr_Color", 0, uiGizmoColor, 0);
      SetPickID (glim, uiPickID, 1);
      CreateArrow (glim, mTransform * mT, fArrowLength, fArrowThickness, 6);
    }

    if (bX)
    {
      mR.SetRotationMatrixZ (-90);
      glim.Attribute4ub ("attr_Color", uiGizmoColor, 0, 0);
      SetPickID (glim, uiPickID, 0);
      CreateArrow (glim, mTransform * mR * mT, fArrowLength, fArrowThickness, 6);
    }

    if (bZ)
    {
      mR.SetRotationMatrixX (90);
      glim.Attribute4ub ("attr_Color", 0, 0, uiGizmoColor);
      SetPickID (glim, uiPickID, 2);
      CreateArrow (glim, mTransform * mR * mT, fArrowLength, fArrowThickness, 6);
    }
  }

  void aeRenderShapes::CreateRotationGizmo (GLIM_BATCH& glim, const aeMatrix& mTransform, float fRadius, float fRingRadius, aeUInt32 uiPickID, bool bX, bool bY, bool bZ)
  {
    aeMatrix mR;

    if (bZ)
    {
      glim.Attribute4ub ("attr_Color", 0, 0, uiGizmoColor);
      SetPickID (glim, uiPickID, 3);
      CreateTorus (glim, mTransform, fRadius, fRingRadius, 32, 6);
    }

    if (bX)
    {
      mR.SetRotationMatrixY (-90);
      glim.Attribute4ub ("attr_Color", uiGizmoColor, 0, 0);
      SetPickID (glim, uiPickID, 5);
      CreateTorus (glim, mTransform * mR, fRadius, fRingRadius, 32, 6);
    }

    if (bY)
    {
      mR.SetRotationMatrixX (90);
      glim.Attribute4ub ("attr_Color", 0, uiGizmoColor, 0);
      SetPickID (glim, uiPickID, 4);
      CreateTorus (glim, mTransform * mR, fRadius, fRingRadius, 32, 6);
    }
  }

}

