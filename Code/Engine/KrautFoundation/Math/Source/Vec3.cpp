#include "../Vec3.h"
#include "../Quaternion.h"
#include "../Math.h"
#include "../../Basics/Checks.h"



namespace AE_NS_FOUNDATION
{
  const aeVec3 aeVec3::GetRefractedVector (const aeVec3& vNormal, float fRefIndex1, float fRefIndex2) const
  {
    const float n = fRefIndex1 / fRefIndex2;
    const float cosI = this->Dot (vNormal);
    const float sinT2 = n * n * (1.0f - (cosI * cosI));

    //invalid refraction
    if (sinT2 > 1.0f)
      return (*this);

    return ((n * (*this)) - (n + aeMath::Sqrt (1.0f - sinT2)) * vNormal);
  }

  /*! Normal needs to be normalized to yield correct results. iMaxAlpha is in degree. */
  void aeVec3::CreateRandomVector (const aeVec3 &vNormal, aeUInt8 iMaxAlpha)
  {
    AE_CHECK_DEV (iMaxAlpha <= 180, "aeVec3::CreateRandomVector: iMaxAlpha (%d) is > 180", iMaxAlpha);

    if (iMaxAlpha == 0)
    {
      *this = vNormal;
      return;
    }

    aeVec3 vUp = aeVec3::GetAxisY ();

    if (aeMath::Abs (vNormal.Dot (aeVec3::GetAxisY ())) > 0.7f) //45°
      vUp = aeVec3::GetAxisX ();


    const aeVec3 vOrtho = vNormal.Cross (vUp);//.GetNormalized ();

    const float fr1 = (float) aeMath::Rand (iMaxAlpha);
    const float fr2 = (float) aeMath::Rand (360);

    aeQuaternion qR1, qR2;
    qR1.CreateQuaternion (vOrtho, fr1);
    qR2.CreateQuaternion (vNormal, fr2);
    *this = qR2 * (qR1 * vNormal);
  }

  /*! Normal needs to be normalized to yield correct results. iMinAlpha and iMaxAlpha are in degree. */
  void aeVec3::CreateRandomVector (const aeVec3 &vNormal, aeUInt8 iMinAlpha, aeUInt8 iMaxAlpha)
  {
    AE_CHECK_DEV (iMaxAlpha <= 180, "aeVec3::CreateRandomVector: iMaxAlpha (%d) is > 180", iMaxAlpha);

    if (iMaxAlpha == 0)
    {
      *this = vNormal;
      return;
    }

    aeVec3 vUp = aeVec3::GetAxisY ();

    if (aeMath::Abs (vNormal.Dot (aeVec3::GetAxisY ())) > 0.7f) //45°
      vUp = aeVec3::GetAxisX ();


    const aeVec3 vOrtho = vNormal.Cross (vUp);

    aeQuaternion qR1, qR2;
    qR1.CreateQuaternion (vOrtho, (float) aeMath::Rand (iMinAlpha, iMaxAlpha));
    qR2.CreateQuaternion (vNormal, (float) aeMath::Rand (360));
    *this = qR2 * (qR1 * vNormal);
  }
}


