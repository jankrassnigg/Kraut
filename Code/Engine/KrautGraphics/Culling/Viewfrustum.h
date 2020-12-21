// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#ifndef AE_UTILITY_CULLING_VIEWFRUSTUM_H
#define AE_UTILITY_CULLING_VIEWFRUSTUM_H

#include "Declarations.h"
#include <KrautFoundation/Math/Vec3.h>
#include <KrautFoundation/Math/Plane.h>

namespace AE_NS_GRAPHICS
{
  using namespace AE_NS_FOUNDATION;

	/*! This class allows to create and manipulate a Viewfrustum. */
	class aeViewfrustum
	{
	public:
		aeViewfrustum ();

		//! \todo implementieren
		//! Creates a Frustum from the given camera settings.
		void CreateFrustum (const aeVec3& vCamPos, const aeVec3& vCamDir, const aeVec3& vCamUp, float fFovX, float fFovY, float fNearPlane, float fFarPlane);
		//! Takes an existing Frustum and a polygon and creates a new, smaller frustum, which lies entirely in the old frustum
		//! \todo checken
		void CreateFrustum (const aeVec3& vCamPos, const aeViewfrustum& Frustum, const aeVec3 *vVertices, aeUInt32 uiVertices);

		//! Extracts the ViewFrustum Information from OpenGL´s Projectionmatrix
		void ExtractOpenGLFrustum (const aeVec3& vCameraPos, float fMaxFarPlaneDist = 999999.0f);

		aeVec3 getCameraPosition (void) const {return (m_vCamPosition);}

		//! Checks, whether the given point is inside the Viewfrustum.
		AE_CULL_VOLUME_CHECK isPointInside (const aeVec3& vPoint) const;
		//! Checks, whether the Object is inside the viewfrustum (or partly).
		AE_CULL_VOLUME_CHECK isObjectInside (const aeVec3* vVertices, aeUInt32 uiVertices) const;
		//! Checks, whether the Object is inside the viewfrustum (or partly).
		AE_CULL_VOLUME_CHECK isObjectInside (const aeVec3* vVertices, aeUInt32 uiVertices, const aeMatrix& Transformation) const;
		//! Checks, whether a sphere is fully or partly inside the Viewfrustum.
		AE_CULL_VOLUME_CHECK isSphereInside (const aeVec3& vPosition, float fRadiusSquared) const;

		//! Transforms the Viewfrustum by the given Matrix (Translation, Rotation).
		void TransformViewFrustum (const aeMatrix& Transform);


		//! Returns a value between 0 and 16.
		aeUInt32 getNumberOfUsedPlanes (void) const {return (m_iUsedPlanes);}

		//! Returns the nth plane of the Frustum.
		aePlane getFrustumPlane (aeUInt32 uiPlane) const {return (m_Planes[uiPlane]);}
		
	private:
		//! up to 16 planes form the frustum
		aePlane m_Planes[16];	

		//! How many planes are in use at the moment.
		aeInt8 m_iUsedPlanes;

		aeVec3 m_vCamPosition;
	};

}

#pragma once

#endif

