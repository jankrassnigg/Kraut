// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#ifndef AE_GRAPHICS_GLCAMERA_GLCAMERA_H
#define AE_GRAPHICS_GLCAMERA_GLCAMERA_H

#include "../Base.h"
#include <KrautFoundation/Math/Vec3.h>
#include <KrautFoundation/Math/Matrix.h>
#include <KrautFoundation/Strings/Declarations.h>
#include <KrautFoundation/Strings/StaticString.h>


namespace AE_NS_GRAPHICS
{
  using namespace AE_NS_FOUNDATION;

	class AE_GRAPHICS_DLL aeGLCamera
	{
	public:
		aeGLCamera (const char* szName);
		~aeGLCamera ();

		//! Returns the currently active camera. That is the camera which was "applied" last.
		static const aeGLCamera* getCurrent ();

		//! Sets the viewport-dimensions to use with this camera. Implies the view-angle in X-direction (due to the aspect ratio)
		void setViewport (aeUInt32 x, aeUInt32 y, aeUInt32 iWidth, aeUInt32 iHeight, float fDepthRangeNear = 0.0f, float fDepthRangeFar = 1.0f);

		//! Sets this camera to be a perspective camera, with a given view-angle in the Y-direction and an implicit view-angle in X-direction, derived from the aspect-ratio of the viewport. 
		void setPerspectiveCamera (float fViewAngleY, float fNearPlane, float fFarPlane);
		//! Sets this camera to be an orthogonal camera.
		void setOrthoCamera (float fLeft, float fRight, float fBottom, float fTop, float fNear, float fFar);
		//! Allows you to specify manually the frustum to be used
		void setDistortedCamera (float fLeft, float fRight, float fBottom, float fTop, float fNear, float fFar);

		//! Defines the eye-distance to use, when stereo-rendering is enabled
		void setStereoScopic (float fEyeDistance);

		//! Applies the camera's state to OpenGL. Makes this one the "current" camera.
		void ApplyCamera (bool bRightEye = false);

		//! Returns the viewfrustum of this camera, to be used for culling.
		//const aeViewfrustum& getViewfrustum () const;

		//! Returns the current vector orthogonal to the forwards and up vector
		const aeVec3& getVectorRight () const;
		//! Returns the current "up" vector 
		const aeVec3& getVectorUp () const;
		//! Returns the current direction, the camera is looking at
		const aeVec3& getVectorForwards () const;

		//! Returns the camera's current EYE position (ie. in stereo the position of the left or right eye)
		const aeVec3& getPosition () const;
		//! Returns the camera's CENTER position (ie. in stereo the position between the left and right eye)
		const aeVec3& getCenterPosition () const;

		//! Returns the camera's current near plane, far plane and view angle
		void getPerspectiveCamera (float& fViewAngleY, float& fNearPlane, float& fFarPlane) const;

		//! Sets the Camera's position
		void setPosition (const aeVec3& vPosition);

		//! Sets the Camera to look at the given point
		void setLookAt (const aeVec3& vPos, const aeVec3& vLookAt, const aeVec3& vUp = aeVec3::GetAxisY ());

		//! Rotates the camera around the global X-axis, not really useful
		void RotateGlobalX (float fAngle);
		//! Rotates the camera around the global Y-axis, useful for turning the camera around
		void RotateGlobalY (float fAngle);
		//! Rotates the camera around the global Z-axis, not really useful
		void RotateGlobalZ (float fAngle);

		//! Rotates the camera around its local X-axis, useful for looking up/down
		void RotateLocalX (float fAngle);
		//! Rotates the camera around its local Y-axis, rarely useful
		void RotateLocalY (float fAngle);
		//! Rotates the camera around its local Z-axis, sometimes useful
		void RotateLocalZ (float fAngle);

		//! Moves the camera globally by the given vector, useful for moving the camera up/down or along a fixed axis
		void MovePositionGlobal (const aeVec3& vMove);
		//! Moves the camera locally by the given vector, useful for moving "forwards" and "strafing"
		void MovePositionLocal (const aeVec3& vMove);

		//! The viewfrustum will not be updated, as long as it is frozen, such that although the camera changes, culling will be done with that frozen frustum.
		void FreezeViewfrustumForDebug (bool bFreeze);

		//! Makes sure the viewfrustum incorporates the latest changes, even if they were made outside of the aeGLCamera. It even works, when the frustum is frozen.
		//void UpdateViewfrustum (void);

		void setMaxLookUpDownAngle (float fMaxAngle) {m_fMaxLookUpDownAngle = fMaxAngle;}

		//! Sets the camera defined through the given matrix.
		void setMatrixCamera (const aeMatrix& mCamera);

	private:
		void LimitLookUp (void);

		static aeGLCamera* s_pCurrentCamera;

		bool m_bChangedCamera;
		bool m_bViewfrustumFrozen;

		aeUInt32 m_uiViewportX;
		aeUInt32 m_uiViewportY;
		aeUInt32 m_uiViewportWidth;
		aeUInt32 m_uiViewportHeight;

		aeInt32 m_iCameraType;	//! < 0 = perspective, 1 = ortho, 2 = distorted

		float m_fViewAngleY;
		float m_fMaxLookUpDownAngle;

		float m_fLookAngleX;

		float m_fOrthoLeft;
		float m_fOrthoRight;
		float m_fOrthoBottom;
		float m_fOrthoTop;

		float m_fNearPlane;
		float m_fFarPlane;
		float m_fDepthRangeNear;
		float m_fDepthRangeFar;

		float m_fStereoEyeDistance;

		aeVec3 m_vPosition;
		aeVec3 m_vStereoPosition;

		aeVec3 m_vVectorRight;
		aeVec3 m_vVectorUp;
		aeVec3 m_vVectorForwards;

		//aeViewfrustum m_Viewfrustum;

		aeStaticString<32> m_sCameraName;
	};

	inline const aeVec3& aeGLCamera::getPosition () const
	{
		return (m_vStereoPosition);
	}

	inline const aeVec3& aeGLCamera::getCenterPosition () const
	{
		return (m_vPosition);
	}

	inline void aeGLCamera::getPerspectiveCamera (float& fViewAngleY, float& fNearPlane, float& fFarPlane) const
	{
		fViewAngleY = m_fViewAngleY;
		fNearPlane = m_fNearPlane;
		fFarPlane = m_fFarPlane;
	}

	inline const aeVec3& aeGLCamera::getVectorRight () const
	{
		return (m_vVectorRight);
	}

	inline const aeVec3& aeGLCamera::getVectorUp () const
	{
		return (m_vVectorUp);
	}

	inline const aeVec3& aeGLCamera::getVectorForwards () const
	{
		return (m_vVectorForwards);
	}
}

#pragma once

#endif



