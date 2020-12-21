// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#ifndef AE_UTILITY_CULLING_AABBCOMPUTER_H
#define AE_UTILITY_CULLING_AABBCOMPUTER_H

#include <KrautFoundation/Math/Vec3.h>
#include <KrautFoundation/Math/Plane.h>

namespace AE_NS_GRAPHICS
{
	//! Small helper struct to quickly compute the AABB of a set of points.
	struct aeAABBComputer
	{
		bool m_bAnyPointsAdded;
		float m_fMinX;
		float m_fMaxX;
		float m_fMinY;
		float m_fMaxY;
		float m_fMinZ;
		float m_fMaxZ;

		aeAABBComputer () 
		{
			Reset ();
		}

		void Reset (void)
		{
			m_bAnyPointsAdded = false;
      m_fMinX = aeMath::Infinity ();
			m_fMaxX =-aeMath::Infinity ();
			m_fMinY = aeMath::Infinity ();
			m_fMaxY =-aeMath::Infinity ();
			m_fMinZ = aeMath::Infinity ();
			m_fMaxZ =-aeMath::Infinity ();
		}

		void AddPoint (const aeVec3& v)
		{
			m_bAnyPointsAdded = true;
			m_fMinX = aeMath::Min (m_fMinX, v.x);
			m_fMaxX = aeMath::Max (m_fMaxX, v.x);
			m_fMinY = aeMath::Min (m_fMinY, v.y);
			m_fMaxY = aeMath::Max (m_fMaxY, v.y);
			m_fMinZ = aeMath::Min (m_fMinZ, v.z);
			m_fMaxZ = aeMath::Max (m_fMaxZ, v.z);
		}

		//! Returns false, if not a single point has been added to the AABB so far.
		bool isValid (void) const
		{
			return (m_bAnyPointsAdded);
		}

		void AddPoints (const aeVec3* v, aeUInt32 uiPoints)
		{
			for (aeUInt32 ui = 0; ui < uiPoints; ++ui)
				AddPoint (v[ui]);
		}

		void GetAABBCenterAndExtents (aeVec3& out_vCenter, aeVec3& out_vHalfExtents) const
		{
			aeVec3 vExt (m_fMaxX - m_fMinX, m_fMaxY - m_fMinY, m_fMaxZ - m_fMinZ);

			out_vHalfExtents = vExt * 0.5f;

			out_vCenter = aeVec3 (m_fMinX, m_fMinY, m_fMinZ) + out_vHalfExtents;
		}
	};


}

#pragma once

#endif


