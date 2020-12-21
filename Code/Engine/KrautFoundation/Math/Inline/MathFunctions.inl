#ifndef AE_FOUNDATION_MATH_MATH_INL
#define AE_FOUNDATION_MATH_MATH_INL

#include "../Math/Math.h"

#include <math.h>
#include <cstdlib>

namespace AE_NS_FOUNDATION
{
  AE_INLINE float aeMath::SinDeg (float f)
  {
    return (sinf (f * aeMath_DegToRad));
  }

  AE_INLINE float aeMath::CosDeg (float f)
  {
    return (cosf (f * aeMath_DegToRad));
  }

  AE_INLINE float aeMath::SinRad (float f)
  {
    return (sinf (f));
  }

  AE_INLINE float aeMath::CosRad (float f)
  {
    return (cosf (f));
  }

  AE_INLINE float aeMath::TanDeg (float f)
  {
    return (tanf (f * aeMath_DegToRad));
  }

  AE_INLINE float aeMath::TanRad (float f)
  {
    return (tanf (f));
  }

  AE_INLINE float aeMath::ASinDeg (float f)
  {
    return (asinf (f) * aeMath_RadToDeg);
  }

  AE_INLINE float aeMath::ACosDeg  (float f)
  {
    return (acosf (f) * aeMath_RadToDeg);
  }

  AE_INLINE float aeMath::ASinRad (float f)
  {
    return (asinf (f));
  }

  AE_INLINE float aeMath::ACosRad (float f)
  {
    return (acosf (f));
  }

  AE_INLINE float aeMath::ATanDeg (float f)
  {
    return (atanf (f) * aeMath_RadToDeg);
  }

  AE_INLINE float aeMath::ATanRad (float f)
  {
    return (atanf (f));
  }

  AE_INLINE float aeMath::ATan2Deg (float x, float y)
  {
    return (atan2f (x, y) * aeMath_RadToDeg);
  }

  AE_INLINE float aeMath::ATan2Rad (float x, float y)
  {
    return (atan2f (x, y));
  }

  AE_INLINE float aeMath::Exp (float f)
  {
    return (expf (f));
  }

  AE_INLINE float aeMath::Ln (float f)
  {
    return (logf (f));
  }

  AE_INLINE float aeMath::Log2 (float f)
  {
    return (log10f (f) / log10f (2.0f));
  }

  AE_INLINE float aeMath::Log10 (float f)
  {
    return (log10f (f));
  }

  AE_INLINE float aeMath::Log (float fBase, float f)
  {
    return (log10f (f) / log10f (fBase));
  }

  AE_INLINE float aeMath::Pow2 (float f)
  {
    return (powf (2.0f, f));
  }

  AE_INLINE float aeMath::Pow (float base, float exp)
  {
    return (powf (base, exp));
  }

  AE_INLINE int aeMath::Pow2 (int i)
  {
    return (1 << i);
  }

  AE_INLINE int aeMath::Pow (int base, int exp)
  {
    int res = 1;
    while (exp > 0)
    {
      res *= base;
      --exp;
    }
    return (res);
  }

  AE_INLINE float aeMath::Root (float f, float NthRoot)
  {
    return (powf (f, 1.0f / NthRoot));
  }

  AE_INLINE float aeMath::Sqrt (float f)
  {
    return (sqrtf (f));
  }

  AE_INLINE float aeMath::Floor (float f)
  {
    return floorf (f);
  }

  AE_INLINE float aeMath::Ceil (float f)
  {
    return ceilf (f);
  }

  AE_INLINE float aeMath::Floor (float f, float fMultiple)
  {
    float fDivides = f / fMultiple;
    float fFactor = Floor (fDivides);
    return (fFactor * fMultiple);
  }

  AE_INLINE float aeMath::Ceil (float f, float fMultiple)
  {
    float fDivides = f / fMultiple;
    float fFactor = Ceil (fDivides);
    return (fFactor * fMultiple);
  }

  AE_INLINE aeInt32 aeMath::Floor (aeInt32 i, aeUInt32 uiMultiple)
  {
    if (i < 0)
    {
      const aeInt32 iDivides = (i+1) / (aeInt32) uiMultiple;

      return ((iDivides-1) * uiMultiple);
    }

    const aeInt32 iDivides = i / (aeInt32) uiMultiple;
    return (iDivides * uiMultiple);
  }

  AE_INLINE aeInt32 aeMath::Ceil (aeInt32 i, aeUInt32 uiMultiple)
  {
    if (i < 0)
    {
      const aeInt32 iDivides = i / (aeInt32) uiMultiple;

      return (iDivides * uiMultiple);
    }

    aeInt32 iDivides = (i-1) / (aeInt32) uiMultiple;
    return ((iDivides+1) * uiMultiple);
  }

  AE_INLINE float aeMath::Trunc (float f)
  {
    if (f > 0.0f)
      return Floor (f);

    return Ceil (f);
  }

  AE_INLINE float aeMath::Fraction (float f)
  {
    return (f - Trunc (f));
  }

  AE_INLINE aeInt32 aeMath::FloatToInt (float f)
  {
    return ((aeInt32) Trunc (f)); // same as trunc
  }

  AE_INLINE float aeMath::Mod (float f, float div)
  {
    return (fmodf (f, div));
  }

  AE_INLINE float aeMath::Invert (float f)
  {
    return (1.0f / f);
  }

  AE_INLINE bool aeMath::IsOdd (aeInt32 i)
  {
    return ((i & 1) != 0);
  }

  AE_INLINE bool aeMath::IsEven (aeInt32 i)
  {
    return ((i & 1) == 0);
  }

  AE_INLINE float aeMath::SmoothStep (float x, float edge1, float edge2)
  {
    x = (x - edge1) / (edge2 - edge1);

    if (x <= 0.0f)
      return (0.0f);
    if (x >= 1.0f)
      return (1.0f);

    return (x * x * (3.0f - (2.0f * x)));
  }

  AE_INLINE bool aeMath::IsPowerOf2 (aeInt32 value)
  {
    if (value < 1) 
      return (false);

    return ((value & (value - 1)) == 0);
  }

  AE_INLINE void aeMath::SetRandSeed (aeUInt32 seed)
  {
    m_Rand.m_uiSeedValue = seed;
  }

  AE_INLINE aeUInt32 aeMath::Rand (void)
  {
    return (m_Rand.GetRandomNumber ());
  }

  AE_INLINE aeUInt32 aeMath::Rand (aeUInt32 max)
  {
     return (m_Rand.GetRandomNumber () % max);
  }

  AE_INLINE aeInt32 aeMath::Rand (aeInt32 min, aeInt32 max)
  {
    return (Rand_Range (min, max - min));
  }

  AE_INLINE aeInt32 aeMath::Rand_Range (aeInt32 min, aeUInt32 range)
  {
    if (range == 0)
      return (min);
  
    return (min + Rand (range));
  }

  AE_INLINE float aeMath::Randf (float max)
  {
    return ((float (m_Rand.GetRandomNumber () % RAND_MAX-1) / float (RAND_MAX-2)) * max);
  }

  AE_INLINE float aeMath::Randf (float min, float max)
  {
    return (Rand_Rangef (min, max - min));
  }

  AE_INLINE float aeMath::Rand_Rangef (float min, float range)
  {
    return (min + Randf (range));
  }

  AE_INLINE bool aeMath::IsFloatEqual (float rhs, float lhs, float fEpsilon)
  {
    return ((rhs >= lhs - fEpsilon) && (rhs <= lhs + fEpsilon));
  }

  AE_INLINE bool aeMath::IsNaN (float f)
  {
    // all comparisons fail, if a float is NaN, even equality
    // volatile to make sure the compiler doesn't optimize this away
    volatile float f2 = f;
    volatile bool equal = (f2 == f2);
    return !equal;
  }

  AE_INLINE bool aeMath::IsFinite (float f)
  {
    // if f is NaN both comparisons would fail
    // Thus returning false -> NaN is not a finite number

    // If f is Infinity or -Infinity the comparisons would also fail (it cannot be decided, but its not clearly smaller)
    return (f < aeMath::Infinity ()) && (f > -aeMath::Infinity ());
  }

  union aeIntToFloat
  {
    int i;
    float f;
  };

  AE_INLINE float aeMath::Infinity () 
  {
    aeIntToFloat i2f;
    i2f.i = 0x7f800000; // bitwise representation of float infinity

    return i2f.f;
  } 
}

#endif



