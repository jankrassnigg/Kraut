#ifndef AE_FOUNDATION_MATH_QUATERNION_INL
#define AE_FOUNDATION_MATH_QUATERNION_INL

#include "../Math/Vec3.h"
#include "../Math/Matrix.h"

namespace AE_NS_FOUNDATION
{
  AE_INLINE aeQuaternion::aeQuaternion (Initialization Init)
  {
    switch (Init)
    {
    case Identity:
      SetIdentity ();
      return;
    default:
      AE_CHECK_DEV (false, "Unknown initialization type %i.", Init);
      return;
    }
  }

  AE_INLINE aeQuaternion::aeQuaternion (float X, float Y, float Z, float W) : v (X, Y, Z), w (W)
  {
  }

  AE_INLINE aeQuaternion::aeQuaternion (const aeVec3& V, float W) : v (V), w (W)
  {
  }

  AE_INLINE void aeQuaternion::SetQuaternion (const aeVec3& V, float W)
  {
    v = V;
    w = W;
  }

  AE_INLINE void aeQuaternion::SetQuaternion (float X, float Y, float Z, float W)
  {
    v.SetVector (X, Y, Z);
    w = W;
  }

  AE_INLINE void aeQuaternion::SetIdentity (void)
  {
    v.SetZero ();
    w = 1.0f;
  }

  AE_INLINE void aeQuaternion::CreateQuaternion (const aeVec3& vRotationAxis, float fAngle)
  {
    const float d = fAngle * aeMath_DegToRad * 0.5f;

    v = aeMath::SinRad (d) * vRotationAxis;
    w = aeMath::CosRad (d);

    Normalize ();
  }

  AE_INLINE void aeQuaternion::Normalize (void)
  {
    float n = v.x * v.x + v.y * v.y + v.z * v.z + w * w;
    
    if (n == 1.0f)
      return;

    n = aeMath::Invert (aeMath::Sqrt (n));

    v *= n;
    w *= n;
  }

  AE_INLINE const aeQuaternion aeQuaternion::GetNormalized (void) const
  {
    float n = v.x * v.x + v.y * v.y + v.z * v.z + w * w;
    
    if (n == 1.0f)
      return (*this);

    n = aeMath::Invert (aeMath::Sqrt (n));

    return (aeQuaternion (v.x * n, v.y * n, v.z * n, w * n));
  }

  AE_INLINE const aeVec3 aeQuaternion::GetRotationAxis (void) const
  {
    const float d = aeMath::SinRad (aeMath::ACosRad (w));

    if (d == 0.0f)
      return (aeVec3 (0, 1, 0));

    return (v / d);
  }

  AE_INLINE float aeQuaternion::GetRotationAngle (void) const
  {
    return (aeMath::ACosDeg (w) * 2.0f);
  }

  AE_INLINE const aeQuaternion aeQuaternion::operator- (void) const
  {
    return (aeQuaternion (-v.x, -v.y, -v.z, w));
  }

  AE_INLINE const aeVec3 operator* (const aeQuaternion& q, const aeVec3& v)
  {
    aeQuaternion qt (v.x, v.y, v.z, 0.0f);
    return (((q * qt) * (-q)).v);
  }

  AE_INLINE const aeQuaternion operator* (const aeQuaternion& q1, const aeQuaternion& q2)
  {
    aeQuaternion q;

    q.w = q1.w * q2.w - q1.v.Dot (q2.v);
    q.v = q1.w * q2.v + q2.w * q1.v + q1.v.Cross (q2.v);

    return (q);
  }
}

#endif


