// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#include "Main.h"
#include "RenderAPI_Impl.h"



namespace AE_NS_GRAPHICS
{
	void aeRenderAPI::PushRenderStates (bool bResetToDefault)
	{
		AE_CHECK_DEV (aeRenderAPI_Impl::m_RenderStateStack.size () < 100, "aeRenderAPI::PushRenderStates: Stack Size is 100. You are doing it wrong.");

		aeRenderAPI_Impl::m_RenderStateStack.push (aeRenderAPI_Impl::m_CurrentRenderState);

		if (bResetToDefault)
			aeRenderAPI_Impl::m_CurrentRenderState.setToDefault ();
	}

	void aeRenderAPI::PopRenderStates ()
	{
		AE_CHECK_DEV (!aeRenderAPI_Impl::m_RenderStateStack.empty (), "aeRenderAPI::PopRenderStates: Stack is empty. You do not call Push/Pop in concert.");

		aeRenderAPI_Impl::m_CurrentRenderState = aeRenderAPI_Impl::m_RenderStateStack.top ();

		aeRenderAPI_Impl::m_RenderStateStack.pop ();
	}

	void aeRenderAPI::SetDefaultRenderStates ()
	{
		aeRenderAPI_Impl::m_CurrentRenderState.setToDefault ();
	}

#ifdef AE_RENDERAPI_OPENGL
	void aeRenderAPI_Impl::ApplyRenderStates (bool bForce)
	{
		if (m_bAutoCheckErrors)
			aeRenderAPI::CheckErrors ("(auto-check) Before ApplyRenderStates");

		if ((bForce) || (m_CurrentRenderState.m_bWireframe != m_AppliedRenderState.m_bWireframe))
		{
			if (m_CurrentRenderState.m_bWireframe)
				glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
			else
				glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
		}

		if ((bForce) || (m_CurrentRenderState.m_bDepthClip != m_AppliedRenderState.m_bDepthClip))
		{
			if (GLEW_ARB_depth_clamp)
			{
				if (m_CurrentRenderState.m_bDepthClip)
					glDisable (GL_DEPTH_CLAMP);
				else
					glEnable (GL_DEPTH_CLAMP);
			}
			else
			if (GLEW_NV_depth_clamp)
			{
				if (m_CurrentRenderState.m_bDepthClip)
					glDisable (GL_DEPTH_CLAMP_NV);
				else
					glEnable (GL_DEPTH_CLAMP_NV);					
			}
		}

		if ((bForce) || (m_CurrentRenderState.m_FaceCulling != m_AppliedRenderState.m_FaceCulling))
		{
			switch (m_CurrentRenderState.m_FaceCulling)
			{
			case AE_CULL_FACE_NONE:
				glDisable (GL_CULL_FACE);
				break;

			case AE_CULL_FACE_FRONT:
				glEnable (GL_CULL_FACE);
				glCullFace (GL_FRONT);
				break;

			case AE_CULL_FACE_BACK:
			default:
				glEnable (GL_CULL_FACE);
				glCullFace (GL_BACK);
				break;
			}
		}

		if ((bForce) || (m_CurrentRenderState.m_bDepthTest != m_AppliedRenderState.m_bDepthTest))
		{
			if (m_CurrentRenderState.m_bDepthTest)
				glEnable (GL_DEPTH_TEST);
			else
				glDisable (GL_DEPTH_TEST);
		}

		if ((bForce) || (m_CurrentRenderState.m_bDepthMask != m_AppliedRenderState.m_bDepthMask))
		{
			glDepthMask (m_CurrentRenderState.m_bDepthMask);
		}

		if ((bForce) || (m_CurrentRenderState.m_DepthFunc != m_AppliedRenderState.m_DepthFunc))
		{
			glDepthFunc (mapCompareFuncOGL (m_CurrentRenderState.m_DepthFunc));
		}

		//if (GLEW_EXT_draw_buffers2)
		{
			for (int rt = 0; rt < GC_MAX_RENDERTARGETS; ++rt)
			{
				if ((bForce) || (m_CurrentRenderState.m_bColorMaskRed[rt] != m_AppliedRenderState.m_bColorMaskRed[rt])
							 || (m_CurrentRenderState.m_bColorMaskGreen[rt] != m_AppliedRenderState.m_bColorMaskGreen[rt])
							 || (m_CurrentRenderState.m_bColorMaskBlue[rt] != m_AppliedRenderState.m_bColorMaskBlue[rt])
							 || (m_CurrentRenderState.m_bColorMaskAlpha[rt] != m_AppliedRenderState.m_bColorMaskAlpha[rt]))
				{
					glColorMaskIndexedEXT (rt, m_CurrentRenderState.m_bColorMaskRed[rt], m_CurrentRenderState.m_bColorMaskGreen[rt],
						m_CurrentRenderState.m_bColorMaskBlue[rt], m_CurrentRenderState.m_bColorMaskAlpha[rt]);
				}

				if ((bForce) || (m_CurrentRenderState.m_bBlending[rt] != m_AppliedRenderState.m_bBlending[rt])
							 || (m_CurrentRenderState.m_BlendFuncRGB[rt] != m_AppliedRenderState.m_BlendFuncRGB[rt])
							 || (m_CurrentRenderState.m_BlendFuncAlpha[rt] != m_AppliedRenderState.m_BlendFuncAlpha[rt])
							 || (m_CurrentRenderState.m_SrcFactorRGB[rt] != m_AppliedRenderState.m_SrcFactorRGB[rt])
							 || (m_CurrentRenderState.m_SrcFactorAlpha[rt] != m_AppliedRenderState.m_SrcFactorAlpha[rt])
							 || (m_CurrentRenderState.m_DstFactorRGB[rt] != m_AppliedRenderState.m_DstFactorRGB[rt])
							 || (m_CurrentRenderState.m_DstFactorAlpha[rt] != m_AppliedRenderState.m_DstFactorAlpha[rt]))
				{
					if (m_CurrentRenderState.m_bBlending[rt])
					{
						glEnableIndexedEXT (GL_BLEND, rt);

						glBlendEquationSeparate (mapBlendFuncOGL (m_CurrentRenderState.m_BlendFuncRGB[rt]), mapBlendFuncOGL (m_CurrentRenderState.m_BlendFuncAlpha[rt]));
						glBlendFuncSeparate (mapBlendFactorOGL (m_CurrentRenderState.m_SrcFactorRGB[rt]), mapBlendFactorOGL (m_CurrentRenderState.m_DstFactorRGB[rt]),
							mapBlendFactorOGL (m_CurrentRenderState.m_SrcFactorAlpha[rt]), mapBlendFactorOGL (m_CurrentRenderState.m_DstFactorAlpha[rt]));
					}
					else
					{
						glDisableIndexedEXT (GL_BLEND, rt);
					}
				}
			}
		}

		if ((bForce) || (m_CurrentRenderState.m_fBlendColorRed != m_AppliedRenderState.m_fBlendColorRed)
					 || (m_CurrentRenderState.m_fBlendColorGreen != m_AppliedRenderState.m_fBlendColorGreen)
					 || (m_CurrentRenderState.m_fBlendColorBlue != m_AppliedRenderState.m_fBlendColorBlue)
					 || (m_CurrentRenderState.m_fBlendColorAlpha != m_AppliedRenderState.m_fBlendColorAlpha))
		{
			glBlendColor (m_CurrentRenderState.m_fBlendColorRed, m_CurrentRenderState.m_fBlendColorGreen, 
				m_CurrentRenderState.m_fBlendColorBlue, m_CurrentRenderState.m_fBlendColorAlpha);
		}

		m_AppliedRenderState = m_CurrentRenderState;
	}
#endif

	void aeRenderAPI::setFaceCulling (AE_FACE_CULLING cull)
	{
		aeRenderAPI_Impl::m_CurrentRenderState.m_FaceCulling = cull;
	}

	void aeRenderAPI::setWireframeMode (bool bEnable)
	{
		aeRenderAPI_Impl::m_CurrentRenderState.m_bWireframe = bEnable;
	}

	void aeRenderAPI::setColorMask (bool bRed, bool bGreen, bool bBlue, bool bAlpha, aeInt8 iRenderTarget)
	{
		if (iRenderTarget != -1)
		{
			aeRenderAPI_Impl::m_CurrentRenderState.m_bColorMaskRed[iRenderTarget] = bRed;
			aeRenderAPI_Impl::m_CurrentRenderState.m_bColorMaskGreen[iRenderTarget] = bGreen;
			aeRenderAPI_Impl::m_CurrentRenderState.m_bColorMaskBlue[iRenderTarget] = bBlue;
			aeRenderAPI_Impl::m_CurrentRenderState.m_bColorMaskAlpha[iRenderTarget] = bAlpha;
		}
		else
		{
			for (aeInt8 iRenderTarget = 0; iRenderTarget < GC_MAX_RENDERTARGETS; ++iRenderTarget)
			{
				aeRenderAPI_Impl::m_CurrentRenderState.m_bColorMaskRed[iRenderTarget] = bRed;
				aeRenderAPI_Impl::m_CurrentRenderState.m_bColorMaskGreen[iRenderTarget] = bGreen;
				aeRenderAPI_Impl::m_CurrentRenderState.m_bColorMaskBlue[iRenderTarget] = bBlue;
				aeRenderAPI_Impl::m_CurrentRenderState.m_bColorMaskAlpha[iRenderTarget] = bAlpha;
			}
		}
	}

	void aeRenderAPI::setDepthMask (bool bEnable)
	{
		aeRenderAPI_Impl::m_CurrentRenderState.m_bDepthMask = bEnable;
	}

	void aeRenderAPI::setDepthTest (bool bEnable, AE_COMPARE_FUNC DepthFunc)
	{
		aeRenderAPI_Impl::m_CurrentRenderState.m_bDepthTest = bEnable;
		aeRenderAPI_Impl::m_CurrentRenderState.m_DepthFunc = DepthFunc;
	}

	void aeRenderAPI::setBlendColor (float fRed, float fGreen, float fBlue, float fAlpha)
	{
		aeRenderAPI_Impl::m_CurrentRenderState.m_fBlendColorRed = fRed;
		aeRenderAPI_Impl::m_CurrentRenderState.m_fBlendColorGreen = fGreen;
		aeRenderAPI_Impl::m_CurrentRenderState.m_fBlendColorBlue = fBlue;
		aeRenderAPI_Impl::m_CurrentRenderState.m_fBlendColorAlpha = fAlpha;
	}
	
	void aeRenderAPI::setBlending (bool bEnable, AE_BLEND_FACTOR srcfactorRGB, AE_BLEND_FACTOR dstfactorRGB, AE_BLEND_FUNC BlendFuncRGB,
					AE_BLEND_FACTOR srcfactorAlpha, AE_BLEND_FACTOR dstfactorAlpha, AE_BLEND_FUNC BlendFuncAlpha, aeInt8 iRenderTarget)
	{
		if (iRenderTarget != -1)
		{
			//AE_CHECK_ALWAYS (false, "OpenGL does not yet support different blend modes per render target.");

			aeRenderAPI_Impl::m_CurrentRenderState.m_bBlending[iRenderTarget] = bEnable;
			aeRenderAPI_Impl::m_CurrentRenderState.m_BlendFuncRGB[iRenderTarget] = BlendFuncRGB;
			aeRenderAPI_Impl::m_CurrentRenderState.m_SrcFactorRGB[iRenderTarget] = srcfactorRGB;
			aeRenderAPI_Impl::m_CurrentRenderState.m_DstFactorRGB[iRenderTarget] = dstfactorRGB;
			aeRenderAPI_Impl::m_CurrentRenderState.m_BlendFuncAlpha[iRenderTarget] = BlendFuncAlpha;
			aeRenderAPI_Impl::m_CurrentRenderState.m_SrcFactorAlpha[iRenderTarget] = srcfactorAlpha;
			aeRenderAPI_Impl::m_CurrentRenderState.m_DstFactorAlpha[iRenderTarget] = dstfactorAlpha;
		}
		else
		{
			for (iRenderTarget = 0; iRenderTarget < GC_MAX_RENDERTARGETS; ++iRenderTarget)
			{
				aeRenderAPI_Impl::m_CurrentRenderState.m_bBlending[iRenderTarget] = bEnable;
				aeRenderAPI_Impl::m_CurrentRenderState.m_BlendFuncRGB[iRenderTarget] = BlendFuncRGB;
				aeRenderAPI_Impl::m_CurrentRenderState.m_SrcFactorRGB[iRenderTarget] = srcfactorRGB;
				aeRenderAPI_Impl::m_CurrentRenderState.m_DstFactorRGB[iRenderTarget] = dstfactorRGB;
				aeRenderAPI_Impl::m_CurrentRenderState.m_BlendFuncAlpha[iRenderTarget] = BlendFuncAlpha;
				aeRenderAPI_Impl::m_CurrentRenderState.m_SrcFactorAlpha[iRenderTarget] = srcfactorAlpha;
				aeRenderAPI_Impl::m_CurrentRenderState.m_DstFactorAlpha[iRenderTarget] = dstfactorAlpha;
			}
		}
	}
}

