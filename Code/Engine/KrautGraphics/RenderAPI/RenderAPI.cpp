// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#include "RenderAPI.h"
#include "RenderAPI_Impl.h"



namespace AE_NS_GRAPHICS
{
	AE_RA_VENDOR aeRenderAPI_Impl::m_Vendor = AE_VENDOR_UNKNOWN;
	bool aeRenderAPI_Impl::m_bAutoCheckErrors = false;
	bool aeRenderAPI_Impl::m_bExceptionOnError = false;
	bool aeRenderAPI_Impl::m_bChangeViewportOrDepthRange = true;

	bool aeRenderAPI_Impl::m_bVSync = false;
	bool aeRenderAPI_Impl::m_bFlipRenderedImage = false;

	std::stack<aeRenderState> aeRenderAPI_Impl::m_RenderStateStack;
	std::stack<aeStencilState> aeRenderAPI_Impl::m_StencilStateStack;
	std::stack<aeFramebufferState> aeRenderAPI_Impl::m_FBStateStack;

	aeRenderState aeRenderAPI_Impl::m_CurrentRenderState;
	aeRenderState aeRenderAPI_Impl::m_AppliedRenderState;

	aeStencilState aeRenderAPI_Impl::m_CurrentStencilState;
	aeStencilState aeRenderAPI_Impl::m_AppliedStencilState;

	aeFramebufferState aeRenderAPI_Impl::m_CurrentFBState;
	aeFramebufferState aeRenderAPI_Impl::m_AppliedFBState;


	aeMatrixStack aeRenderAPI_Impl::m_MatrixObjectToWorld;
	aeMatrixStack aeRenderAPI_Impl::m_MatrixWorldToCamera;
	aeMatrixStack aeRenderAPI_Impl::m_MatrixCameraToScreen;

	AE_RENDER_MODE aeRenderAPI_Impl::m_CurrentRenderMode = AE_RENDER_UNKNOWN;

	aeUInt32 aeRenderAPI_Impl::m_uiNumberOfDrawcalls = 0;
	aeUInt32 aeRenderAPI_Impl::m_uiNumberOfStateChanges = 0;
	aeUInt32 aeRenderAPI_Impl::m_uiNumberOfRenderedVertices = 0;
	aeUInt32 aeRenderAPI_Impl::m_uiNumberOfRenderedTriangles = 0;
	aeUInt32 aeRenderAPI_Impl::m_uiNumberOfRenderedInstances = 0;
}


