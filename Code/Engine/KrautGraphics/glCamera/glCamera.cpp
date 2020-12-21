// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#include "Main.h"
#include "../RenderAPI/RenderAPI.h"



namespace AE_NS_GRAPHICS
{
	aeGLCamera* aeGLCamera::s_pCurrentCamera = NULL;

	aeGLCamera::aeGLCamera (const char* szName)
	{
		m_bChangedCamera = true;
		m_bViewfrustumFrozen = false;

		m_fLookAngleX = 0.0f;
		m_fMaxLookUpDownAngle = 88.0f;

		m_uiViewportX = 0;
		m_uiViewportY = 0;
		m_uiViewportWidth = 200;
		m_uiViewportHeight = 200;

		m_fViewAngleY = 90.0f;

		m_iCameraType = 0;

		m_fNearPlane = 1.0f;
		m_fFarPlane = 100.0f;

		m_fDepthRangeNear = 0.0f;
		m_fDepthRangeFar = 1.0f;

		m_fStereoEyeDistance = 0.0f;

    m_vPosition.SetZero ();

		m_vVectorForwards.SetVector (0, 0, -1);
		m_vVectorRight.SetVector (1, 0, 0);
		m_vVectorUp.SetVector (0, 1, 0);

		m_sCameraName = szName;
	}

	aeGLCamera::~aeGLCamera ()
	{
		if (s_pCurrentCamera == this)
			s_pCurrentCamera = NULL;
	}

	const aeGLCamera* aeGLCamera::getCurrent ()
	{
		//AE_CHECK_DEV (s_pCurrentCamera != NULL, "aeGLCamera::getCurrent: The current Camera is NULL, please apply a camera, before you query for the current one.");

		return (s_pCurrentCamera);
	}

	void aeGLCamera::setViewport (aeUInt32 x, aeUInt32 y, aeUInt32 iWidth, aeUInt32 iHeight, float fDepthRangeNear, float fDepthRangeFar)
	{
		m_uiViewportX = x;
		m_uiViewportY = y;
		m_uiViewportWidth = iWidth;
		m_uiViewportHeight = iHeight;

		m_fDepthRangeNear = fDepthRangeNear;
		m_fDepthRangeFar = fDepthRangeFar;
	}

	void aeGLCamera::setPerspectiveCamera (float fViewAngleY, float fNearPlane, float fFarPlane)
	{
		m_bChangedCamera = true;

		m_iCameraType = 0;

		m_fViewAngleY = fViewAngleY;

		m_fNearPlane = fNearPlane;
		m_fFarPlane = fFarPlane;
	}

	void aeGLCamera::setOrthoCamera (float fLeft, float fRight, float fBottom, float fTop, float fNear, float fFar)
	{
		m_bChangedCamera = true;

		m_iCameraType = 1;

		m_fOrthoLeft = fLeft;
		m_fOrthoRight = fRight;
		m_fOrthoBottom = fBottom;
		m_fOrthoTop = fTop;
		m_fNearPlane = fNear;
		m_fFarPlane = fFar;
	}

	void aeGLCamera::setDistortedCamera (float fLeft, float fRight, float fBottom, float fTop, float fNear, float fFar)
	{
		m_bChangedCamera = true;

		m_iCameraType = 2;

		m_fOrthoLeft = fLeft;
		m_fOrthoRight = fRight;
		m_fOrthoBottom = fBottom;
		m_fOrthoTop = fTop;
		m_fNearPlane = fNear;
		m_fFarPlane = fFar;
	}

	void aeGLCamera::setPosition (const aeVec3& vPosition)
	{
		m_bChangedCamera = true;

		m_vPosition = vPosition;
	}

	void aeGLCamera::MovePositionGlobal (const aeVec3& vMove)
	{
		m_bChangedCamera = true;

		m_vPosition += vMove;
	}

	void aeGLCamera::MovePositionLocal (const aeVec3& vMove)
	{
		m_bChangedCamera = true;

		m_vPosition += vMove.x * m_vVectorRight;
		m_vPosition += vMove.y * m_vVectorUp;
		m_vPosition += vMove.z * -m_vVectorForwards;
	}

	void aeGLCamera::ApplyCamera (bool bRightEye)
	{
		s_pCurrentCamera = this;

		aeRenderAPI::setDepthRange (m_fDepthRangeNear, m_fDepthRangeFar);
		aeRenderAPI::setViewport (m_uiViewportX, m_uiViewportY, m_uiViewportWidth, m_uiViewportHeight);

		switch (m_iCameraType)
		{
		case 0:
			{
				aeUInt32 uiPosX, uiPosY, uiWidth, uiHeight;
				float fNear, fFar;
				aeRenderAPI::getViewportAndDepthRange (uiPosX, uiPosY, uiWidth, uiHeight, fNear, fFar);

				//LimitLookUp ();
				aeRenderAPI::GetMatrix_CameraToScreen ().Perspective (m_fViewAngleY, (float) (uiWidth) / (float) (uiHeight), m_fNearPlane, m_fFarPlane);
			}
			break;
		case 1:
			aeRenderAPI::GetMatrix_CameraToScreen ().Ortho (m_fOrthoLeft, m_fOrthoRight, m_fOrthoBottom, m_fOrthoTop, m_fNearPlane, m_fFarPlane);
			break;
		case 2:
			//LimitLookUp ();
			aeRenderAPI::GetMatrix_CameraToScreen ().Frustum (m_fOrthoLeft, m_fOrthoRight, m_fOrthoBottom, m_fOrthoTop, m_fNearPlane, m_fFarPlane);
			break;
		}

		aeVec3 vPos = m_vPosition;

		if (m_fStereoEyeDistance > 0.0f)
		{
			if (bRightEye)
				vPos += m_vVectorRight * m_fStereoEyeDistance * 0.5f;
			else
				vPos -= m_vVectorRight * m_fStereoEyeDistance * 0.5f;
		}

		aeRenderAPI::GetMatrix_WorldToCamera ().LookAt (
			vPos.x, vPos.y, vPos.z,
			vPos.x + m_vVectorForwards.x, 
			vPos.y + m_vVectorForwards.y, 
			vPos.z + m_vVectorForwards.z,
			m_vVectorUp.x, m_vVectorUp.y, m_vVectorUp.z);

		//if (!m_bViewfrustumFrozen)
			//m_Viewfrustum.ExtractOpenGLFrustum (vPos);

		m_bChangedCamera = false;

		m_vStereoPosition = vPos;
	}

	void aeGLCamera::setLookAt (const aeVec3& vPos, const aeVec3& vLookAt0, const aeVec3& vUp)
	{
		m_bChangedCamera = true;

    if (vLookAt0.IsNaN ())
			AE_CHECK_ALWAYS (false, "vLookAt0 is NaN");

		aeVec3 vLookAt = vLookAt0;

		if (vLookAt == vPos)
			vLookAt += aeVec3 (0, 0, -1);

		if (vLookAt.IsNaN ())
			AE_CHECK_ALWAYS (false, "vLookAt is NaN");

		m_vPosition = vPos;
		m_vVectorForwards = (vLookAt - vPos).GetNormalized ();
		m_vVectorRight = m_vVectorForwards.Cross (vUp).GetNormalized ();
		m_vVectorUp = m_vVectorRight.Cross (m_vVectorForwards).GetNormalized ();
	}


	//const aeViewfrustum& aeGLCamera::getViewfrustum () const
	//{
	//	if (m_bChangedCamera)
	//		aeLog::Warning ("aeGLCamera::getViewfrustum: The camera \"%s\" has been changed, but not yet applied, the Viewfrustum is therefore invalid.", m_sCameraName.c_str ());

	//	return (m_Viewfrustum);
	//}

	void aeGLCamera::RotateGlobalX (float fAngle)
	{
		m_bChangedCamera = true;

		aeMatrix m;
		m.SetRotationMatrixX (-fAngle);

		m_vVectorRight = m * m_vVectorRight;
		m_vVectorUp = m * m_vVectorUp;
		m_vVectorForwards = m * m_vVectorForwards;
	}

	void aeGLCamera::RotateGlobalY (float fAngle)
	{
		m_bChangedCamera = true;

		aeMatrix m;
		m.SetRotationMatrixY (-fAngle);

		m_vVectorRight = m * m_vVectorRight;
		m_vVectorUp = m * m_vVectorUp;
		m_vVectorForwards = m * m_vVectorForwards;
	}

	void aeGLCamera::RotateGlobalZ (float fAngle)
	{
		m_bChangedCamera = true;

		aeMatrix m;
		m.SetRotationMatrixZ (-fAngle);

		m_vVectorRight = m * m_vVectorRight;
		m_vVectorUp = m * m_vVectorUp;
		m_vVectorForwards = m * m_vVectorForwards;
	}


	void aeGLCamera::RotateLocalX (float fAngle)
	{
		m_bChangedCamera = true;

		fAngle = -fAngle;

		const float fDot = m_vVectorForwards.Dot (aeVec3 (0, 1, 0));
		m_fLookAngleX = 90.0f - aeMath::ACosDeg (fDot);

		fAngle = aeMath::Clamp (m_fLookAngleX + fAngle, -85.0f, 85.0f) - m_fLookAngleX;
		m_fLookAngleX += fAngle;

		aeMatrix m;
		m.SetRotationMatrix (m_vVectorRight, fAngle);
		
		m_vVectorUp = m * m_vVectorUp;
		m_vVectorForwards = m * m_vVectorForwards;
	}

	void aeGLCamera::RotateLocalY (float fAngle)
	{
		m_bChangedCamera = true;

		aeMatrix m;
		m.SetRotationMatrix (m_vVectorUp, -fAngle);
		
		m_vVectorRight = m * m_vVectorRight;
		m_vVectorForwards = m * m_vVectorForwards;
	}

	void aeGLCamera::RotateLocalZ (float fAngle)
	{
		m_bChangedCamera = true;

		aeMatrix m;
		m.SetRotationMatrix (m_vVectorForwards, -fAngle);
		
		m_vVectorRight = m * m_vVectorRight;
		m_vVectorUp = m * m_vVectorUp;
	}

	void aeGLCamera::setStereoScopic (float fEyeDistance)
	{
		m_fStereoEyeDistance = fEyeDistance;
	}

	void aeGLCamera::FreezeViewfrustumForDebug (bool bFreeze)
	{
		m_bViewfrustumFrozen = bFreeze;
	}

	//void aeGLCamera::UpdateViewfrustum (void)
	//{
	//	//m_Viewfrustum.ExtractOpenGLFrustum (m_vPosition);
	//}

	void aeGLCamera::LimitLookUp (void)
	{
		const float m_fMaxLookUpAngle = m_fMaxLookUpDownAngle;

		const float fDot = m_vVectorForwards.Dot (aeVec3 (0, 1, 0));
		m_fLookAngleX = 90.0f - aeMath::ACosDeg (fDot);

		if (aeMath::Abs (m_fLookAngleX) > m_fMaxLookUpAngle)
		{
			float fRotateDown = (aeMath::Abs (m_fLookAngleX) - m_fMaxLookUpAngle) * aeMath::Sign (m_fLookAngleX);

			RotateLocalX (fRotateDown);
		}
	}

	void aeGLCamera::setMatrixCamera (const aeMatrix& mCamera)
	{
		m_bChangedCamera = true;

    m_vPosition.x = mCamera.m_fColumn[3][0];
		m_vPosition.y = mCamera.m_fColumn[3][1];
		m_vPosition.z = mCamera.m_fColumn[3][2];

		m_vVectorRight.x = mCamera.m_fColumn[0][0];
		m_vVectorRight.y = mCamera.m_fColumn[0][1];
		m_vVectorRight.z = mCamera.m_fColumn[0][2];

		m_vVectorUp.x = mCamera.m_fColumn[1][0];
		m_vVectorUp.y = mCamera.m_fColumn[1][1];
		m_vVectorUp.z = mCamera.m_fColumn[1][2];

		m_vVectorForwards.x = -mCamera.m_fColumn[2][0];
		m_vVectorForwards.y = -mCamera.m_fColumn[2][1];
		m_vVectorForwards.z = -mCamera.m_fColumn[2][2];
	}
}

