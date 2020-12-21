// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#ifndef MATHS_VEC3_H
#define MATHS_VEC3_H

//namespace NS_MATHS
//{
	//! Simple class for a 3-component vector
	class VEC3
	{
	public:			// Data

		float x;
		float y;
		float z;


	public:			// Constructors

		VEC3 () {}	// default-constructed vector is uninitialized (for speed)
		VEC3 (float x, float y, float z);
		// no copy-constructor and operator= since the default-generated ones will be faster


	public:			// Functions to set the vector to specific values

		//! Sets the vector to these values.
		void SetVector (float x, float y, float z);
		//! Sets the vector to all 0.
		void SetToZeroVector (void);

	public:			// Functions dealing with length

		//! Returns the length of the vector.
		float GetLength (void) const;
		//! Returns the squared length. Faster, since no square-root is taken. Useful, if one only wants to Compare the lengths of two vectors.
		float GetLengthSquared (void) const;

		//! Returns a normalized version of this vector, leaves the vector itself unchanged.
		const VEC3 GetNormalized (void) const;
		//! Normalizes this vector.
		void Normalize (void);
		
		//! Returns, whether this vector is (0, 0, 0).
		bool IsZeroVector (void) const;
		//! Returns, whether the length of this vector is 1.0.
		bool IsNormalized (void) const;


	public:			// Operators

		//! Returns the negation of this vector.
		const VEC3 operator- (void) const;

		//! Adds cc component-wise to this vector
		const VEC3& operator+= (const VEC3& cc);
		//! Subtracts cc component-wise from this vector
		const VEC3& operator-= (const VEC3& cc);
		//! Multiplies all components of this vector with f
		const VEC3& operator*= (float f);
		//! Divides all components of this vector by f
		const VEC3& operator/= (float f);
		

	public:			// static vector-functions

		//! Returns the zero-vector.
		static const VEC3 GetZeroVector (void);


	public:			// Other common operations

		//! Calculates the normal of the triangle defined by the three vertices. Vertices are assumed to be ordered counter-clockwise.
		void CalculateNormal (const VEC3& v1, const VEC3& v2, const VEC3& v3);
	};


	// Common vector operations

	//! Returns the Dot-product of the two vectors (commutative, order does not matter)
	float Dot (const VEC3& v1, const VEC3& v2);
	//! Returns the Cross-product of the two vectors (NOT commutative, order DOES matter)
	const VEC3 Cross (const VEC3& v1, const VEC3& v2);
	//! Compares all componenets of the two vectors, for equality, allowing a certain deviation
	bool Compare (const VEC3& v1, const VEC3& v2, float fMaxComponentDifference);


	// Operators

	const VEC3 operator+ (const VEC3& v1, const VEC3& v2);
	const VEC3 operator- (const VEC3& v1, const VEC3& v2);

	const VEC3 operator* (float f, const VEC3& v);
	const VEC3 operator* (const VEC3& v, float f);

	const VEC3 operator/ (const VEC3& v, float f);

	//! Dot-product of two vectors, NOT component-wise multiplication
	float operator* (const VEC3& v1, const VEC3& v2);

	bool operator== (const VEC3& v1, const VEC3& v2);
	bool operator!= (const VEC3& v1, const VEC3& v2);

//}

#pragma once

#include "Vec3_Inline.h"

#endif



