// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#ifndef AE_GRAPHICS_RENDERAPI_RENDERAPI_IMPL_H
#define AE_GRAPHICS_RENDERAPI_RENDERAPI_IMPL_H

#include "Declarations.h"
#include "RenderStates.h"
#include <stack>
#include <KrautFoundation/Math/Matrix.h>
#include "MatrixStack.h"

namespace AE_NS_GRAPHICS
{
	struct aeRenderAPI_Impl
	{

#ifdef AE_RENDERAPI_OPENGL
		static void ApplyRenderStates (bool bForce);
		static void ApplyStencilStates (bool bForce);
		static void ApplyFBStates (bool bForce);
#endif


		static AE_RA_VENDOR m_Vendor;
		static bool m_bAutoCheckErrors;
		static bool m_bExceptionOnError;
		static bool m_bChangeViewportOrDepthRange;

		static bool m_bVSync;
		static bool m_bFlipRenderedImage;

		static std::stack<aeRenderState> m_RenderStateStack;
		static std::stack<aeStencilState> m_StencilStateStack;
		static std::stack<aeFramebufferState> m_FBStateStack;

		static aeRenderState m_CurrentRenderState;
		static aeRenderState m_AppliedRenderState;

		static aeStencilState m_CurrentStencilState;
		static aeStencilState m_AppliedStencilState;

		static aeFramebufferState m_CurrentFBState;
		static aeFramebufferState m_AppliedFBState;


		static aeMatrixStack m_MatrixObjectToWorld;
		static aeMatrixStack m_MatrixWorldToCamera;
		static aeMatrixStack m_MatrixCameraToScreen;

		static AE_RENDER_MODE m_CurrentRenderMode;

		static aeUInt32 m_uiNumberOfDrawcalls;
		static aeUInt32 m_uiNumberOfStateChanges;
		static aeUInt32 m_uiNumberOfRenderedVertices;
		static aeUInt32 m_uiNumberOfRenderedTriangles;
		static aeUInt32 m_uiNumberOfRenderedInstances;

		static aeUInt32 s_uiWindowSizeX;
		static aeUInt32 s_uiWindowSizeY;
	};


}

#endif


