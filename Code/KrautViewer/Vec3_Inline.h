// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#ifndef MATHS_VEC3INLINE_H
#define MATHS_VEC3INLINE_H

// *** INLINE IMPLEMENTATION OF VEC3 *** 

#include <math.h>

//namespace NS_MATHS
//{
	inline VEC3::VEC3 (float X, float Y, float Z) : x (X), y (Y), z (Z)
	{
	}

	inline void VEC3::SetVector (float X, float Y, float Z)
	{
		x = X;
		y = Y;
		z = Z;
	}

	inline void VEC3::SetToZeroVector (void)
	{
		x = y = z = 0.0f;
	}

	inline float VEC3::GetLength (void) const
	{
		return (sqrtf (x * x + y * y + z * z));
	}

	inline float VEC3::GetLengthSquared (void) const
	{
		return (x * x + y * y + z * z);
	}

	inline const VEC3 VEC3::GetNormalized (void) const
	{
		const float fLengthInv = 1.0f / GetLength ();
		return (VEC3 (x * fLengthInv, y * fLengthInv, z * fLengthInv));
	}

	inline void VEC3::Normalize (void)
	{
		const float fLengthInv = 1.0f / GetLength ();

		// multiplication is faster than division
		x *= fLengthInv;
		y *= fLengthInv;
		z *= fLengthInv;
	}

	inline bool VEC3::IsZeroVector (void) const
	{
		return ((x == 0.0f) && (y == 0.0f) && (z == 0.0f));
	}

	inline bool VEC3::IsNormalized (void) const
	{
		return (GetLengthSquared () == 1.0f);
	}

	inline const VEC3 VEC3::operator- (void) const
	{
		return (VEC3 (-x, -y, -z));
	}

	inline const VEC3& VEC3::operator+= (const VEC3& cc)
	{
		x += cc.x;
		y += cc.y;
		z += cc.z;

		return (*this);
	}

	inline const VEC3& VEC3::operator-= (const VEC3& cc)
	{
		x -= cc.x;
		y -= cc.y;
		z -= cc.z;

		return (*this);
	}

	inline const VEC3& VEC3::operator*= (float f)
	{
		x *= f;
		y *= f;
		z *= f;

		return (*this);
	}

	inline const VEC3& VEC3::operator/= (float f)
	{
		const float f_inv = 1.0f / f;

		x *= f_inv;
		y *= f_inv;
		z *= f_inv;

		return (*this);
	}

	inline const VEC3 VEC3::GetZeroVector (void)
	{
		return (VEC3 (0.0f, 0.0f, 0.0f));
	}

	inline void VEC3::CalculateNormal (const VEC3& v1, const VEC3& v2, const VEC3& v3)
	{
		*this = Cross ((v3 - v2), (v1 - v2));
		Normalize ();
	}

	inline float Dot (const VEC3& v1, const VEC3& v2)
	{
		return ((v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z));
	}

	inline const VEC3 Cross (const VEC3& v1, const VEC3& v2)
	{
		return (VEC3 (v1.y * v2.z - v1.z * v2.y,
					  v1.z * v2.x - v1.x * v2.z,
					  v1.x * v2.y - v1.y * v2.x));
	}

	inline bool Compare (const VEC3& v1, const VEC3& v2, float fDiff)
	{
		if ((v1.x + fDiff > v2.x) && (v1.x - fDiff < v2.x))
		{
			if ((v1.y + fDiff > v2.y) && (v1.y - fDiff < v2.y))
			{
				if ((v1.z + fDiff > v2.z) && (v1.z - fDiff < v2.z))
					return (true);
			}
		}

		return (false);
	}

	inline const VEC3 operator+ (const VEC3& v1, const VEC3& v2)
	{
		return (VEC3 (v1.x + v2.x, v1.y + v2.y, v1.z + v2.z));
	}

	inline const VEC3 operator- (const VEC3& v1, const VEC3& v2)
	{
		return (VEC3 (v1.x - v2.x, v1.y - v2.y, v1.z - v2.z));
	}

	inline const VEC3 operator* (float f, const VEC3& v)
	{
		return (VEC3 (v.x * f, v.y * f, v.z * f));
	}

	inline const VEC3 operator* (const VEC3& v, float f)
	{
		return (VEC3 (v.x * f, v.y * f, v.z * f));
	}

	inline const VEC3 operator/ (const VEC3& v, float f)
	{
		// multiplication is much faster than division
		const float f_inv = 1.0f / f;
		return (VEC3 (v.x * f_inv, v.y * f_inv, v.z * f_inv));
	}

	inline float operator* (const VEC3& v1, const VEC3& v2)
	{
		return (Dot (v1, v2));
	}

	inline bool operator== (const VEC3& v1, const VEC3& v2)
	{
		return ((v1.x == v2.x) && (v1.y == v2.y) && (v1.z == v2.z));
	}

	inline bool operator!= (const VEC3& v1, const VEC3& v2)
	{
		return ((v1.x != v2.x) || (v1.y != v2.y) || (v1.z != v2.z));
	}
//}

#pragma once

#endif



