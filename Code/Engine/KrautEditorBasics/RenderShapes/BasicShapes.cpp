#include "RenderShapes.h"

namespace AE_NS_EDITORBASICS
{
  void aeRenderShapes::CreateCylinder (GLIM_BATCH& glim, const aeMatrix& mTransform, float fLength, float fRadius, aeUInt16 uiSegments)
  {
    glim.Begin (GLIM_QUADS);

    const float fAngleStep = 360.0f / uiSegments;
    const float fHalfLength = fLength * 0.5f;

    for (aeUInt32 s = 0; s < uiSegments; ++s)
    {
      aeVec3 vPos[4];
      vPos[0] = aeVec3 (aeMath::CosDeg ((s+0) * fAngleStep) * fRadius, fHalfLength, aeMath::SinDeg ((s+0) * fAngleStep) * fRadius);
      vPos[1] = aeVec3 (aeMath::CosDeg ((s+1) * fAngleStep) * fRadius, fHalfLength, aeMath::SinDeg ((s+1) * fAngleStep) * fRadius);
      vPos[2] = vPos[1] + aeVec3 (0, -fLength, 0);
      vPos[3] = vPos[0] + aeVec3 (0, -fLength, 0);

      for (int i = 0; i < 4; ++i)
      {
        vPos[i] = mTransform * vPos[i];
        glim.Vertex (vPos[i].x, vPos[i].y, vPos[i].z);
      }
    }

    glim.End ();

    glim.Begin (GLIM_POLYGON);

    for (aeUInt32 s = 0; s < uiSegments; ++s)
    {
      aeVec3 vPos = mTransform * aeVec3 (aeMath::CosDeg ((s+0) * fAngleStep) * fRadius, -fHalfLength, aeMath::SinDeg ((s+0) * fAngleStep) * fRadius);
      glim.Vertex (vPos.x, vPos.y, vPos.z);
    }

    glim.End ();

   glim.Begin (GLIM_POLYGON);

    for (aeInt32 s = uiSegments-1; s >= 0; --s)
    {
      aeVec3 vPos = mTransform * aeVec3 (aeMath::CosDeg ((s+0) * fAngleStep) * fRadius, +fHalfLength, aeMath::SinDeg ((s+0) * fAngleStep) * fRadius);
      glim.Vertex (vPos.x, vPos.y, vPos.z);
    }

    glim.End ();
  }

  void aeRenderShapes::CreateCone (GLIM_BATCH& glim, const aeMatrix& mTransform, float fLength, float fRadius, aeUInt16 uiSegments)
  {
    glim.Begin (GLIM_TRIANGLES);

    const float fAngleStep = 360.0f / uiSegments;
    const float fHalfLength = fLength * 0.5f;

    for (aeUInt32 s = 0; s < uiSegments; ++s)
    {
      aeVec3 vPos[3];
      vPos[0] = aeVec3 (aeMath::CosDeg ((s+1) * fAngleStep) * fRadius, -fHalfLength, aeMath::SinDeg ((s+1) * fAngleStep) * fRadius);
      vPos[1] = aeVec3 (aeMath::CosDeg ((s+0) * fAngleStep) * fRadius, -fHalfLength, aeMath::SinDeg ((s+0) * fAngleStep) * fRadius);
      vPos[2] = aeVec3 (0, fHalfLength, 0);;

      for (int i = 0; i < 3; ++i)
      {
        vPos[i] = mTransform * vPos[i];
        glim.Vertex (vPos[i].x, vPos[i].y, vPos[i].z);
      }
    }

    glim.End ();

    glim.Begin (GLIM_POLYGON);

    for (aeUInt32 s = 0; s < uiSegments; ++s)
    {
      aeVec3 vPos = mTransform * aeVec3 (aeMath::CosDeg ((s+0) * fAngleStep) * fRadius, -fHalfLength, aeMath::SinDeg ((s+0) * fAngleStep) * fRadius);
      glim.Vertex (vPos.x, vPos.y, vPos.z);
    }

    glim.End ();
  }

  void aeRenderShapes::CreateArrow (GLIM_BATCH& glim, const aeMatrix& mTransform, float fLength, float fRadius, aeUInt16 uiSegments)
  {
    CreateCylinder (glim, mTransform, fLength, fRadius, uiSegments);

    const float fTipRadius = fRadius * 2.0f;
    const float fTipLength = fTipRadius * 2.0f;
    
    aeMatrix mTip;
    mTip.SetTranslationMatrix (aeVec3 (0, (fLength + fTipLength) * 0.5f, 0));

    CreateCone (glim, mTransform * mTip, fTipLength, fTipRadius, uiSegments);
  }

  void aeRenderShapes::CreateTorus (GLIM_BATCH& glim, const aeMatrix& mTransform, float fRadius, float fRingRadius, aeUInt16 uiSegments, aeUInt16 uiRingSegments)
  {
    glim.Begin (GLIM_QUADS);

    const float fSegmentsStep = 360.0f / uiSegments;
    const float fRingStep = 360.0f / uiRingSegments;

    aeMatrix mCenter[2];
    aeVec3 vPos[4];

    const aeVec3 vRingCenter (fRadius, 0, 0);

    for (aeUInt32 r0 = 0; r0 < uiSegments; ++r0)
    {
      const float fSegAngle1 = (r0 + 0) * fSegmentsStep;
      const float fSegAngle2 = (r0 + 1) * fSegmentsStep;

      mCenter[0].SetRotationMatrixZ (fSegAngle1);
      mCenter[1].SetRotationMatrixZ (fSegAngle2);

      for (aeUInt32 r1 = 0; r1 < uiRingSegments; ++r1)
      {
        const float fRingAngle1 = (r1 + 0) * fRingStep;
        const float fRingAngle2 = (r1 + 1) * fRingStep;

        vPos[0] = vRingCenter + aeVec3 (aeMath::CosDeg (fRingAngle2), 0, aeMath::SinDeg (fRingAngle2)) * fRingRadius;
        vPos[1] = vRingCenter + aeVec3 (aeMath::CosDeg (fRingAngle1), 0, aeMath::SinDeg (fRingAngle1)) * fRingRadius;
        vPos[2] = vPos[1];
        vPos[3] = vPos[0];

        vPos[0] = mCenter[0] * vPos[0];
        vPos[1] = mCenter[0] * vPos[1];
        vPos[2] = mCenter[1] * vPos[2];
        vPos[3] = mCenter[1] * vPos[3];

        for (int i = 0; i < 4; ++i)
        {
          vPos[i] = mTransform * vPos[i];
          glim.Vertex (vPos[i].x, vPos[i].y, vPos[i].z);
        }
      }
    }

    glim.End ();
  }

}

