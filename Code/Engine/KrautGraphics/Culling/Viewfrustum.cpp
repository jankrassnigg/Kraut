// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#include "Viewfrustum.h"
#include <KrautGraphics/RenderAPI/Main.h>


namespace AE_NS_GRAPHICS
{
	using namespace AE_NS_FOUNDATION;

	/*! Constructs a non-existing (empty) Viewfrustum. */
	aeViewfrustum::aeViewfrustum ()
	{
		m_iUsedPlanes = 0;
	}

	/*! Gets OpenGLs Camera-Planes and sets the Viewfrustum with those planes. The Cameraposition is necessary to
		set the Near-Plane back to that Position, instead of leaving it at OpenGLs Position. Else something that is
		in front of the camera, but not inside the Viewfrustum (because of the Nearplane) might suddenly disappear. */
	void aeViewfrustum::ExtractOpenGLFrustum (const aeVec3& vCameraPos, float fMaxFarPlaneDist)
	{
		m_vCamPosition = vCameraPos;

		float   proj[16];
		float   modl[16];
		float   clip[16];
		float   t;
		float frustum[6][4];

		// Get the current PROJECTION matrix from OpenGL
		aeRenderAPI::GetMatrix_CameraToScreen ().GetMatrix ().GetAsOpenGL4x4Matrix (proj);

		// Get the current MODELVIEW matrix from OpenGL
		aeRenderAPI::GetMatrix_WorldToCamera ().GetMatrix ().GetAsOpenGL4x4Matrix (modl);

		// Combine the two matrices (multiply projection by modelview)
		clip[ 0] = modl[ 0] * proj[ 0] + modl[ 1] * proj[ 4] + modl[ 2] * proj[ 8] + modl[ 3] * proj[12];
		clip[ 1] = modl[ 0] * proj[ 1] + modl[ 1] * proj[ 5] + modl[ 2] * proj[ 9] + modl[ 3] * proj[13];
		clip[ 2] = modl[ 0] * proj[ 2] + modl[ 1] * proj[ 6] + modl[ 2] * proj[10] + modl[ 3] * proj[14];
		clip[ 3] = modl[ 0] * proj[ 3] + modl[ 1] * proj[ 7] + modl[ 2] * proj[11] + modl[ 3] * proj[15];

		clip[ 4] = modl[ 4] * proj[ 0] + modl[ 5] * proj[ 4] + modl[ 6] * proj[ 8] + modl[ 7] * proj[12];
		clip[ 5] = modl[ 4] * proj[ 1] + modl[ 5] * proj[ 5] + modl[ 6] * proj[ 9] + modl[ 7] * proj[13];
		clip[ 6] = modl[ 4] * proj[ 2] + modl[ 5] * proj[ 6] + modl[ 6] * proj[10] + modl[ 7] * proj[14];
		clip[ 7] = modl[ 4] * proj[ 3] + modl[ 5] * proj[ 7] + modl[ 6] * proj[11] + modl[ 7] * proj[15];

		clip[ 8] = modl[ 8] * proj[ 0] + modl[ 9] * proj[ 4] + modl[10] * proj[ 8] + modl[11] * proj[12];
		clip[ 9] = modl[ 8] * proj[ 1] + modl[ 9] * proj[ 5] + modl[10] * proj[ 9] + modl[11] * proj[13];
		clip[10] = modl[ 8] * proj[ 2] + modl[ 9] * proj[ 6] + modl[10] * proj[10] + modl[11] * proj[14];
		clip[11] = modl[ 8] * proj[ 3] + modl[ 9] * proj[ 7] + modl[10] * proj[11] + modl[11] * proj[15];

		clip[12] = modl[12] * proj[ 0] + modl[13] * proj[ 4] + modl[14] * proj[ 8] + modl[15] * proj[12];
		clip[13] = modl[12] * proj[ 1] + modl[13] * proj[ 5] + modl[14] * proj[ 9] + modl[15] * proj[13];
		clip[14] = modl[12] * proj[ 2] + modl[13] * proj[ 6] + modl[14] * proj[10] + modl[15] * proj[14];
		clip[15] = modl[12] * proj[ 3] + modl[13] * proj[ 7] + modl[14] * proj[11] + modl[15] * proj[15];

		// Extract the numbers for the RIGHT plane
		frustum[0][0] = clip[ 3] - clip[ 0];
		frustum[0][1] = clip[ 7] - clip[ 4];
		frustum[0][2] = clip[11] - clip[ 8];
		frustum[0][3] = clip[15] - clip[12];

		// Normalize the result
		t = 1.0f / aeMath::Sqrt (frustum[0][0] * frustum[0][0] + frustum[0][1] * frustum[0][1] + frustum[0][2] * frustum[0][2]);
		frustum[0][0] *= t;
		frustum[0][1] *= t;
		frustum[0][2] *= t;
		frustum[0][3] *= t;

		// Extract the numbers for the LEFT plane
		frustum[1][0] = clip[ 3] + clip[ 0];
		frustum[1][1] = clip[ 7] + clip[ 4];
		frustum[1][2] = clip[11] + clip[ 8];
		frustum[1][3] = clip[15] + clip[12];

		// Normalize the result
		t = 1.0f / aeMath::Sqrt (frustum[1][0] * frustum[1][0] + frustum[1][1] * frustum[1][1] + frustum[1][2] * frustum[1][2]);
		frustum[1][0] *= t;
		frustum[1][1] *= t;
		frustum[1][2] *= t;
		frustum[1][3] *= t;

		/* Extract the BOTTOM plane */
		frustum[2][0] = clip[ 3] + clip[ 1];
		frustum[2][1] = clip[ 7] + clip[ 5];
		frustum[2][2] = clip[11] + clip[ 9];
		frustum[2][3] = clip[15] + clip[13];

		/* Normalize the result */
		t = 1.0f / aeMath::Sqrt (frustum[2][0] * frustum[2][0] + frustum[2][1] * frustum[2][1] + frustum[2][2] * frustum[2][2]);
		frustum[2][0] *= t;
		frustum[2][1] *= t;
		frustum[2][2] *= t;
		frustum[2][3] *= t;

		/* Extract the TOP plane */
		frustum[3][0] = clip[ 3] - clip[ 1];
		frustum[3][1] = clip[ 7] - clip[ 5];
		frustum[3][2] = clip[11] - clip[ 9];
		frustum[3][3] = clip[15] - clip[13];

		/* Normalize the result */
		t = 1.0f / aeMath::Sqrt (frustum[3][0] * frustum[3][0] + frustum[3][1] * frustum[3][1] + frustum[3][2] * frustum[3][2]);
		frustum[3][0] *= t;
		frustum[3][1] *= t;
		frustum[3][2] *= t;
		frustum[3][3] *= t;

		/* Extract the FAR plane */
		frustum[4][0] = clip[ 3] - clip[ 2];
		frustum[4][1] = clip[ 7] - clip[ 6];
		frustum[4][2] = clip[11] - clip[10];
		frustum[4][3] = clip[15] - clip[14];

		/* Normalize the result */
		t = 1.0f / aeMath::Sqrt (frustum[4][0] * frustum[4][0] + frustum[4][1] * frustum[4][1] + frustum[4][2] * frustum[4][2]);
		frustum[4][0] *= t;
		frustum[4][1] *= t;
		frustum[4][2] *= t;
		frustum[4][3] *= t;

		/* Extract the NEAR plane */
		frustum[5][0] = clip[ 3] + clip[ 2];
		frustum[5][1] = clip[ 7] + clip[ 6];
		frustum[5][2] = clip[11] + clip[10];
		frustum[5][3] = clip[15] + clip[14];

		/* Normalize the result */
		t = 1.0f / aeMath::Sqrt (frustum[5][0] * frustum[5][0] + frustum[5][1] * frustum[5][1] + frustum[5][2] * frustum[5][2]);
		frustum[5][0] *= t;
		frustum[5][1] *= t;
		frustum[5][2] *= t;
		frustum[5][3] *= t;

		m_Planes[0].m_vNormal.x = frustum[5][0];
		m_Planes[0].m_vNormal.y = frustum[5][1];
		m_Planes[0].m_vNormal.z = frustum[5][2];

		// The near-plane has to be at the camera´s position, so that no portals in front of you, get culled away
		m_Planes[0].CreatePlane (m_Planes[0].m_vNormal, vCameraPos);

		m_Planes[1].m_vNormal.x = frustum[0][0];
		m_Planes[1].m_vNormal.y = frustum[0][1];
		m_Planes[1].m_vNormal.z = frustum[0][2];
		m_Planes[1].m_fDistance = -frustum[0][3];

		m_Planes[2].m_vNormal.x = frustum[1][0];
		m_Planes[2].m_vNormal.y = frustum[1][1];
		m_Planes[2].m_vNormal.z = frustum[1][2];
		m_Planes[2].m_fDistance = -frustum[1][3];

		m_Planes[3].m_vNormal.x = frustum[2][0];
		m_Planes[3].m_vNormal.y = frustum[2][1];
		m_Planes[3].m_vNormal.z = frustum[2][2];
		m_Planes[3].m_fDistance = -frustum[2][3];

		m_Planes[4].m_vNormal.x = frustum[3][0];
		m_Planes[4].m_vNormal.y = frustum[3][1];
		m_Planes[4].m_vNormal.z = frustum[3][2];
		m_Planes[4].m_fDistance = -frustum[3][3];

		m_Planes[5].m_vNormal.x = frustum[4][0];
		m_Planes[5].m_vNormal.y = frustum[4][1];
		m_Planes[5].m_vNormal.z = frustum[4][2];
		m_Planes[5].m_fDistance = -frustum[4][3];

		const float fTemp = m_Planes[5].GetDistanceToPoint (vCameraPos);
		if (fTemp > fMaxFarPlaneDist)
			m_Planes[5].m_fDistance -= fMaxFarPlaneDist - fTemp;
		
		m_iUsedPlanes = 6;
	}

	void aeViewfrustum::CreateFrustum (const aeVec3& vCamPos, const aeVec3& vCamDir, const aeVec3& vCamUp, float fFovX, float fFovY, float fNearPlane, float fFarPlane)
	{
		m_vCamPosition = vCamPos;

		//! \todo implementieren
	}

	/*! Takes a frustum and a polygon and from the silouethe of the poly, it creates a new, smaller frustum, 
		that is goes through the Polygon. Useful to create a Viewfrustum for a Sector, by taking the cameras
		Viewfrustum and reducing it through a Portalpolygon. */
	void aeViewfrustum::CreateFrustum (const aeVec3& vCamPos, const aeViewfrustum &Frustum, const aeVec3 *vVertices, aeUInt32 uiVertices)
	{
		//! \todo checken

		m_vCamPosition = vCamPos;

		if (uiVertices > 16)
			uiVertices = 16;

		m_iUsedPlanes = 0;

		aePlane Plane (vVertices);

		aeVec3 vTemp[2];
		bool bPlanes[16];
		for (int i=0; i < Frustum.m_iUsedPlanes; ++i)
			bPlanes[i] = false;

		//case 1: Camera is behind the poly
    if (Plane.GetPointPosition (vCamPos) == aePositionOnPlane::Back)
		{
			int prev = uiVertices - 1;
			for (aeUInt32 i = 0; i < uiVertices; i++)
			{
				vTemp[0] = vVertices[prev];
				vTemp[1] = vVertices[i];
				prev = i;

				for (int p = 0; p < Frustum.m_iUsedPlanes; p++)
				{
					switch (Frustum.m_Planes[p].GetObjectPosition (vTemp, 2))
					{
          case aePositionOnPlane::Back:
						goto ende1;
          case aePositionOnPlane::Spanning:
						bPlanes[p] = true;
					}
				}

				if (m_iUsedPlanes > 15)
					m_iUsedPlanes = 15;

				m_Planes[m_iUsedPlanes].CreatePlane (vCamPos, vTemp[0], vTemp[1]);

				++m_iUsedPlanes;

	ende1:;
			}
		}
		//case 2: Camera is in front of the poly
		else
		{
			int prev = 0;
			for (int i = uiVertices - 1; i >= 0; i--)
			{
				vTemp[0] = vVertices[prev];
				vTemp[1] = vVertices[i];
				prev = i;

				for (int p = 0; p < Frustum.m_iUsedPlanes; p++)
				{
					switch (Frustum.m_Planes[p].GetObjectPosition (vTemp, 2))
					{
          case aePositionOnPlane::Back:
						goto ende2;
          case aePositionOnPlane::Spanning:
						bPlanes[p] = true;
					}
				}

				if (m_iUsedPlanes > 15)
					m_iUsedPlanes = 15;

				m_Planes[m_iUsedPlanes].CreatePlane (vCamPos, vTemp[0], vTemp[1]);

				++m_iUsedPlanes;

	ende2:;
			}
		}

		for (int i = 0; i < Frustum.m_iUsedPlanes; i++)
		{
			if (bPlanes[i])
			{
				if (m_iUsedPlanes > 15)
					m_iUsedPlanes = 15;

				m_Planes[m_iUsedPlanes] = Frustum.m_Planes[i];
				++m_iUsedPlanes;
			}
		}
	}

	AE_CULL_VOLUME_CHECK aeViewfrustum::isPointInside (const aeVec3& vPoint) const
	{
		for (int i = 0; i < m_iUsedPlanes; i++)
		{
      if (m_Planes[i].GetPointPosition (vPoint) == aePositionOnPlane::Back)
				return (AE_OBJECT_OUTSIDE_VOLUME);
		}

		return (AE_OBJECT_INSIDE_VOLUME);
	}

	/*! if an object is behind any one plane, it is outside the (convex) frustum */
	AE_CULL_VOLUME_CHECK aeViewfrustum::isObjectInside (const aeVec3* vVertices, aeUInt32 iVertices) const
	{
		bool bOnSomePlane = false;

		for (int i = 0; i < m_iUsedPlanes; i++)
		{
			switch (m_Planes[i].GetObjectPosition (vVertices, iVertices))
			{
      case aePositionOnPlane::Back:
				return (AE_OBJECT_OUTSIDE_VOLUME);
      case aePositionOnPlane::Spanning:
				bOnSomePlane = true;
			}
		}

		return (bOnSomePlane ? AE_OBJECT_INTERSECTS_VOLUME : AE_OBJECT_INSIDE_VOLUME);
	}

	AE_CULL_VOLUME_CHECK aeViewfrustum::isObjectInside (const aeVec3* vVertices, aeUInt32 uiVertices, const aeMatrix& Transformation) const
	{
		bool bOnSomePlane = false;

    std::vector<aeVec3> vTemp (uiVertices);
		for (aeUInt32 v = 0; v < uiVertices; ++v)
			vTemp[v] = Transformation * vVertices[v];

		for (int i = 0; i < m_iUsedPlanes; i++)
		{
			switch (m_Planes[i].GetObjectPosition (&vTemp[0], uiVertices))
			{
      case aePositionOnPlane::Back:
				return (AE_OBJECT_OUTSIDE_VOLUME);
      case aePositionOnPlane::Spanning:
				bOnSomePlane = true;
			}
		}

		return (bOnSomePlane ? AE_OBJECT_INTERSECTS_VOLUME : AE_OBJECT_INSIDE_VOLUME);
	}

	AE_CULL_VOLUME_CHECK aeViewfrustum::isSphereInside (const aeVec3 &vPosition, float fRadius) const
	{
		float d;
		bool bOnSomePlane = false;

		for (int i = 0; i < m_iUsedPlanes; i++)
		{
			d = m_Planes[i].GetDistanceToPoint (vPosition);

			if (d <= -fRadius)
				return (AE_OBJECT_OUTSIDE_VOLUME);

			if (d < fRadius)
				bOnSomePlane = true;
		}

		return (bOnSomePlane ? AE_OBJECT_INTERSECTS_VOLUME : AE_OBJECT_INSIDE_VOLUME);
	}

	/*! Useful to transform objects and Viewfrustums in the same space to be able to do Viewfrustumculling. */
	void aeViewfrustum::TransformViewFrustum (const aeMatrix& Transform)
	{
		m_vCamPosition = Transform * m_vCamPosition;

		for (int i = 0; i < m_iUsedPlanes; i++)
			m_Planes[i].TransformPlane (Transform);
	}

}


