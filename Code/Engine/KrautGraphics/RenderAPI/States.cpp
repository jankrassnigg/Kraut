// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#include "Main.h"
#include "RenderAPI_Impl.h"
#include <KrautGraphics/ShaderManager/Main.h>




namespace AE_NS_GRAPHICS
{
	void aeRenderAPI::ApplyStates (void)
	{
		if (aeRenderAPI_Impl::m_bAutoCheckErrors)
			CheckErrors ("auto-check: Before ApplyStates");

		++aeRenderAPI_Impl::m_uiNumberOfStateChanges;

		
		float f[16];

		const bool bChangedO2W = !aeRenderAPI_Impl::m_MatrixObjectToWorld.IsStateFlagged ();
		const bool bChangedW2C = !aeRenderAPI_Impl::m_MatrixWorldToCamera.IsStateFlagged ();
		const bool bChangedC2S = !aeRenderAPI_Impl::m_MatrixCameraToScreen.IsStateFlagged ();

		if ((bChangedO2W) || (bChangedW2C) || (bChangedC2S))
		{
			aeMatrix mCtS = aeRenderAPI_Impl::m_MatrixCameraToScreen.GetMatrix ();

			if (aeRenderAPI_Impl::m_bFlipRenderedImage)
			{
        mCtS.m_fColumn[1][1] *= -1.0f;
				mCtS.m_fColumn[2][1] *= -1.0f;
				mCtS.m_fColumn[3][1] *= -1.0f;
			}

			static aeMatrix mCtSInv;
			if (bChangedC2S)
				mCtSInv = mCtS.GetInverse ();			

			if (bChangedO2W)
			{
				aeRenderAPI_Impl::m_MatrixObjectToWorld.GetMatrix ().GetAsOpenGL4x4Matrix (f);
				aeShaderManager::setUniformMatrix4x4 ("ra_ObjectToWorldMatrix", false, f);

				aeRenderAPI_Impl::m_MatrixObjectToWorld.GetInverse ().GetAsOpenGL4x4Matrix (f);
				aeShaderManager::setUniformMatrix4x4 ("ra_ObjectToWorldMatrix_Inverse", false, f);
			}

			if (bChangedW2C)
			{
				aeMatrix mCam = aeRenderAPI_Impl::m_MatrixWorldToCamera.GetMatrix ().GetInverse ();

				const aeVec3 vPos = aeVec3 ( mCam.m_fColumn[3][0],  mCam.m_fColumn[3][1],  mCam.m_fColumn[3][2]);
				const aeVec3 vDir = aeVec3 (-mCam.m_fColumn[2][0], -mCam.m_fColumn[2][1], -mCam.m_fColumn[2][2]);
				const aeVec3 vRight=aeVec3 ( mCam.m_fColumn[0][0],  mCam.m_fColumn[0][1],  mCam.m_fColumn[0][2]);
				const aeVec3 vUp  = aeVec3 ( mCam.m_fColumn[1][0],  mCam.m_fColumn[1][1],  mCam.m_fColumn[2][1]);

				aeShaderManager::setUniformFloat ("ra_CameraPosition", 3, vPos.x, vPos.y, vPos.z);
				aeShaderManager::setUniformFloat ("ra_CameraForwards", 3, vDir.x, vDir.y, vDir.z);
				aeShaderManager::setUniformFloat ("ra_CameraRight", 3, vRight.x, vRight.y, vRight.z);
				aeShaderManager::setUniformFloat ("ra_CameraUp", 3, vUp.x, vUp.y, vUp.z);

				aeRenderAPI_Impl::m_MatrixWorldToCamera.GetMatrix ().GetAsOpenGL4x4Matrix (f);
				aeShaderManager::setUniformMatrix4x4 ("ra_WorldToCameraMatrix", false, f);

				aeRenderAPI_Impl::m_MatrixWorldToCamera.GetInverse ().GetAsOpenGL4x4Matrix (f);
				aeShaderManager::setUniformMatrix4x4 ("ra_WorldToCameraMatrix_Inverse", false, f);
			}

			if (bChangedC2S)
			{
				mCtS.GetAsOpenGL4x4Matrix (f);
				aeShaderManager::setUniformMatrix4x4 ("ra_CameraToScreenMatrix", false, f);

				mCtSInv.GetAsOpenGL4x4Matrix (f);
				aeShaderManager::setUniformMatrix4x4 ("ra_CameraToScreenMatrix_Inverse", false, f);
			}

			if ((bChangedO2W) || (bChangedW2C))
			{
				const aeMatrix mObjectToCamera = aeRenderAPI_Impl::m_MatrixWorldToCamera.GetMatrix () * aeRenderAPI_Impl::m_MatrixObjectToWorld.GetMatrix ();

				mObjectToCamera.GetAsOpenGL4x4Matrix (f);
				aeShaderManager::setUniformMatrix4x4 ("ra_ObjectToCameraMatrix", false, f);

				mObjectToCamera.GetInverse ().GetAsOpenGL4x4Matrix (f);
				aeShaderManager::setUniformMatrix4x4 ("ra_ObjectToCameraMatrix_Inverse", false, f);
			}

			if ((bChangedO2W) || (bChangedW2C) || (bChangedC2S))
			{
				const aeMatrix mWorldToScreen = mCtS * aeRenderAPI_Impl::m_MatrixWorldToCamera.GetMatrix ();
				const aeMatrix mObjectToScreen = mWorldToScreen * aeRenderAPI_Impl::m_MatrixObjectToWorld.GetMatrix ();

				mWorldToScreen.GetAsOpenGL4x4Matrix (f);
				aeShaderManager::setUniformMatrix4x4 ("ra_WorldToScreenMatrix", false, f);

				mObjectToScreen.GetAsOpenGL4x4Matrix (f);
				aeShaderManager::setUniformMatrix4x4 ("ra_ObjectToScreenMatrix", false, f);


				mWorldToScreen.GetInverse ().GetAsOpenGL4x4Matrix (f);
				aeShaderManager::setUniformMatrix4x4 ("ra_WorldToScreenMatrix_Inverse", false, f);

				mObjectToScreen.GetInverse ().GetAsOpenGL4x4Matrix (f);
				aeShaderManager::setUniformMatrix4x4 ("ra_ObjectToScreenMatrix_Inverse", false, f);
			}

			aeRenderAPI_Impl::m_MatrixObjectToWorld.FlagCurrentState ();
			aeRenderAPI_Impl::m_MatrixWorldToCamera.FlagCurrentState ();
			aeRenderAPI_Impl::m_MatrixCameraToScreen.FlagCurrentState ();
		}

		if (aeRenderAPI_Impl::m_bChangeViewportOrDepthRange)
		{
			aeShaderManager::setUniformFloat ("ra_Viewport", 4, aeRenderAPI_Impl::m_CurrentFBState.m_uiViewportX, aeRenderAPI_Impl::m_CurrentFBState.m_uiViewportY, 
			  aeRenderAPI_Impl::m_CurrentFBState.m_uiViewportWidth, aeRenderAPI_Impl::m_CurrentFBState.m_uiViewportHeight); 

			aeShaderManager::setUniformFloat ("ra_DepthRange", 2, aeRenderAPI_Impl::m_CurrentFBState.m_fDepthRangeNear, aeRenderAPI_Impl::m_CurrentFBState.m_fDepthRangeFar);
		}


		//! \todo NormalMatrix

		if (aeRenderAPI_Impl::m_bAutoCheckErrors)
			CheckErrors ("auto-check: Before ShaderManager->PrepareForRendering");

		aeShaderManager::PrepareForRendering ();

		if (aeRenderAPI_Impl::m_bAutoCheckErrors)
			CheckErrors ("auto-check: After ShaderManager->PrepareForRendering");

#ifdef AE_RENDERAPI_OPENGL
		if (g_RenderAPI == AE_RA_OPENGL)
		{
			aeRenderAPI_Impl::ApplyRenderStates (false);

			if (aeRenderAPI_Impl::m_bAutoCheckErrors)
				CheckErrors ("auto-check: After ApplyRenderStates");

			aeRenderAPI_Impl::ApplyStencilStates (false);

			if (aeRenderAPI_Impl::m_bAutoCheckErrors)
				CheckErrors ("auto-check: Before ApplyStencilStates");

			aeRenderAPI_Impl::ApplyFBStates (false);

			if (aeRenderAPI_Impl::m_bAutoCheckErrors)
				CheckErrors ("auto-check: Before ApplyFBStates");
		}
#endif

	  aeRenderAPI_Impl::m_bChangeViewportOrDepthRange = false;
	}

}

