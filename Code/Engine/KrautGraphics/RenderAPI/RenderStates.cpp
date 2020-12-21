// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#include "RenderStates.h"
#include "RenderAPI.h"



namespace AE_NS_GRAPHICS
{
	aeRenderState::aeRenderState ()
	{
		setToDefault ();
	}


	void aeRenderState::setToDefault ()
	{
		m_bWireframe = false;
		m_FaceCulling = AE_CULL_FACE_BACK;
		m_bDepthClip = true;
		m_bDepthTest = true;
		m_bDepthMask = true;
		m_DepthFunc = AE_COMPARE_LEQUAL;

		m_fBlendColorRed = 1.0;
		m_fBlendColorGreen = 1.0;
		m_fBlendColorBlue = 1.0;
		m_fBlendColorAlpha = 1.0;

		for (aeInt32 i = 0; i < GC_MAX_RENDERTARGETS; ++i)
		{
			m_bColorMaskRed[i] = true;
			m_bColorMaskGreen[i] = true;
			m_bColorMaskBlue[i] = true;
			m_bColorMaskAlpha[i] = true;

			m_bBlending[i] = false;

			m_BlendFuncRGB[i] = AE_FUNC_ADD;
			m_SrcFactorRGB[i] = AE_FACTOR_ONE;
			m_DstFactorRGB[i] = AE_FACTOR_ZERO;

			m_BlendFuncAlpha[i] = AE_FUNC_ADD;
			m_SrcFactorAlpha[i] = AE_FACTOR_ONE;
			m_DstFactorAlpha[i] = AE_FACTOR_ZERO;
		}
	}

	aeStencilState::aeStencilState ()
	{
		setToDefault ();
	}

	void aeStencilState::setToDefault ()
	{
		m_bStencilTest = false;

		m_uiStencilMaskFront = ~0;
		m_StencilFuncFront = AE_COMPARE_ALWAYS;
		m_StencilOP_FailFront = AE_STENCIL_KEEP;
		m_StencilOP_zFailFront = AE_STENCIL_KEEP;
		m_StencilOP_zPassFront = AE_STENCIL_KEEP;
		m_StencilFuncRefFront = 0;
		m_StencilFuncMaskFront = ~0;

		m_uiStencilMaskBack = ~0;
		m_StencilFuncBack = AE_COMPARE_ALWAYS;
		m_StencilOP_FailBack = AE_STENCIL_KEEP;
		m_StencilOP_zFailBack = AE_STENCIL_KEEP;
		m_StencilOP_zPassBack = AE_STENCIL_KEEP;
		m_StencilFuncRefBack = 0;
		m_StencilFuncMaskBack = ~0;
	}

	aeFramebufferState::aeFramebufferState ()
	{
		setToDefault ();
	}

	void aeFramebufferState::setToDefault ()
	{
		aeUInt32 w, h;
		aeRenderAPI::GetWindowSize (w, h);

		m_uiViewportX = 0;
		m_uiViewportY = 0;
		m_uiViewportWidth = w;
		m_uiViewportHeight = h;

		m_fDepthRangeNear = 0.0f;
		m_fDepthRangeFar = 1.0f;

		m_bScissorTest = false;
		m_uiScissorX = 0;
		m_uiScissorY = 0;
		m_uiScissorWidth = m_uiViewportWidth;
		m_uiScissorHeight = m_uiViewportHeight;
	}


}

