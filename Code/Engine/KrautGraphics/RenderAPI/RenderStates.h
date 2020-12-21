// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#ifndef AE_GRAPHICS_RENDERAPI_RENDERSTATES_H
#define AE_GRAPHICS_RENDERAPI_RENDERSTATES_H

#include "Declarations.h"
#include <stack>

namespace AE_NS_GRAPHICS
{
	const aeUInt8 GC_MAX_RENDERTARGETS = 8;

	struct AE_GRAPHICS_DLL aeRenderState
	{
		aeRenderState ();
		void setToDefault ();

		bool m_bWireframe;
		AE_FACE_CULLING m_FaceCulling;		

		bool m_bDepthClip;
		bool m_bDepthTest;
		bool m_bDepthMask;
		AE_COMPARE_FUNC m_DepthFunc;

		bool m_bColorMaskRed[GC_MAX_RENDERTARGETS];
		bool m_bColorMaskGreen[GC_MAX_RENDERTARGETS];
		bool m_bColorMaskBlue[GC_MAX_RENDERTARGETS];
		bool m_bColorMaskAlpha[GC_MAX_RENDERTARGETS];

		float m_fBlendColorRed;
		float m_fBlendColorGreen;
		float m_fBlendColorBlue;
		float m_fBlendColorAlpha;

		bool m_bBlending[GC_MAX_RENDERTARGETS];
		AE_BLEND_FUNC   m_BlendFuncRGB[GC_MAX_RENDERTARGETS];
		AE_BLEND_FACTOR m_SrcFactorRGB[GC_MAX_RENDERTARGETS];
		AE_BLEND_FACTOR m_DstFactorRGB[GC_MAX_RENDERTARGETS];
		AE_BLEND_FUNC   m_BlendFuncAlpha[GC_MAX_RENDERTARGETS];
		AE_BLEND_FACTOR m_SrcFactorAlpha[GC_MAX_RENDERTARGETS];
		AE_BLEND_FACTOR m_DstFactorAlpha[GC_MAX_RENDERTARGETS];
	};

	struct AE_GRAPHICS_DLL aeStencilState
	{		
		aeStencilState ();
		void setToDefault ();

		bool m_bStencilTest;

		aeUInt8 m_uiStencilMaskFront;
		AE_COMPARE_FUNC m_StencilFuncFront;
		AE_STENCIL_OP m_StencilOP_FailFront;
		AE_STENCIL_OP m_StencilOP_zFailFront;
		AE_STENCIL_OP m_StencilOP_zPassFront;
		aeInt32 m_StencilFuncRefFront;
		aeUInt32 m_StencilFuncMaskFront;

		aeUInt8 m_uiStencilMaskBack;
		AE_COMPARE_FUNC m_StencilFuncBack;
		AE_STENCIL_OP m_StencilOP_FailBack;
		AE_STENCIL_OP m_StencilOP_zFailBack;
		AE_STENCIL_OP m_StencilOP_zPassBack;
		aeInt32 m_StencilFuncRefBack;
		aeUInt32 m_StencilFuncMaskBack;
	};

	struct AE_GRAPHICS_DLL aeFramebufferState
	{
		aeFramebufferState ();
		void setToDefault ();

		aeUInt16 m_uiViewportX;
		aeUInt16 m_uiViewportY;
		aeUInt16 m_uiViewportWidth;
		aeUInt16 m_uiViewportHeight;
		
		float m_fDepthRangeNear;
		float m_fDepthRangeFar;

		bool m_bScissorTest;
		aeUInt16 m_uiScissorX;
		aeUInt16 m_uiScissorY;
		aeUInt16 m_uiScissorWidth;
		aeUInt16 m_uiScissorHeight;

	};
}

#endif




