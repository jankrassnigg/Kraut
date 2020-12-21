// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#include "Main.h"
#include "RenderAPI_Impl.h"



namespace AE_NS_GRAPHICS
{
	void aeRenderAPI::setStencilTest (bool bEnable, AE_COMPARE_FUNC StencilFunc, aeInt32 iStencilFuncRef, aeUInt32 uiStencilFuncMask,
			aeUInt8 uiStencilMask, AE_STENCIL_OP StencilOpFail, AE_STENCIL_OP StencilOpZFail, AE_STENCIL_OP StencilOpZPass, 
			AE_FACE_STENCIL_TEST face)
	{
		aeRenderAPI_Impl::m_CurrentStencilState.m_bStencilTest = bEnable;

		if ((face == AE_FRONT_FACE_STENCIL_TEST) || (face == AE_FRONT_AND_BACK_FACE_STENCIL_TEST))
		{
			aeRenderAPI_Impl::m_CurrentStencilState.m_StencilFuncFront = StencilFunc;
			aeRenderAPI_Impl::m_CurrentStencilState.m_StencilFuncMaskFront = uiStencilFuncMask;
			aeRenderAPI_Impl::m_CurrentStencilState.m_StencilFuncRefFront = iStencilFuncRef;
			aeRenderAPI_Impl::m_CurrentStencilState.m_StencilOP_FailFront = StencilOpFail;
			aeRenderAPI_Impl::m_CurrentStencilState.m_StencilOP_zFailFront = StencilOpZFail;
			aeRenderAPI_Impl::m_CurrentStencilState.m_StencilOP_zPassFront = StencilOpZPass;
			aeRenderAPI_Impl::m_CurrentStencilState.m_uiStencilMaskFront = uiStencilMask;
		}

		if ((face == AE_BACK_FACE_STENCIL_TEST) || (face == AE_FRONT_AND_BACK_FACE_STENCIL_TEST))
		{
			aeRenderAPI_Impl::m_CurrentStencilState.m_StencilFuncBack = StencilFunc;
			aeRenderAPI_Impl::m_CurrentStencilState.m_StencilFuncMaskBack = uiStencilFuncMask;
			aeRenderAPI_Impl::m_CurrentStencilState.m_StencilFuncRefBack = iStencilFuncRef;
			aeRenderAPI_Impl::m_CurrentStencilState.m_StencilOP_FailBack = StencilOpFail;
			aeRenderAPI_Impl::m_CurrentStencilState.m_StencilOP_zFailBack = StencilOpZFail;
			aeRenderAPI_Impl::m_CurrentStencilState.m_StencilOP_zPassBack = StencilOpZPass;
			aeRenderAPI_Impl::m_CurrentStencilState.m_uiStencilMaskBack = uiStencilMask;
		}
	}

#ifdef AE_RENDERAPI_OPENGL
	void aeRenderAPI_Impl::ApplyStencilStates (bool bForce)
	{
		if (m_bAutoCheckErrors)
			aeRenderAPI::CheckErrors ("(auto-check) Before ApplyStencilStates");
		
    if ((bForce) || (!aeMemory::Compare (&m_CurrentStencilState, &m_AppliedStencilState, sizeof (aeStencilState))))
		{
			if (!m_CurrentStencilState.m_bStencilTest)
			{
				m_CurrentStencilState.setToDefault ();
				glDisable (GL_STENCIL_TEST);
			}
			else
			{
				glEnable (GL_STENCIL_TEST);
				
				if (GLEW_EXT_stencil_two_side)
				{
					glEnable (GL_STENCIL_TEST_TWO_SIDE_EXT);

					glActiveStencilFaceEXT (GL_BACK);

					glStencilOp (mapStencilOpOGL (m_CurrentStencilState.m_StencilOP_FailBack),
						mapStencilOpOGL (m_CurrentStencilState.m_StencilOP_zFailBack),
						mapStencilOpOGL (m_CurrentStencilState.m_StencilOP_zPassBack));

					glStencilMask (m_CurrentStencilState.m_uiStencilMaskBack);

					glStencilFunc (mapCompareFuncOGL (m_CurrentStencilState.m_StencilFuncBack), 
						m_CurrentStencilState.m_StencilFuncRefBack,
						m_CurrentStencilState.m_StencilFuncMaskBack);

					glActiveStencilFaceEXT (GL_FRONT);

					glStencilOp (mapStencilOpOGL (m_CurrentStencilState.m_StencilOP_FailFront),
						mapStencilOpOGL (m_CurrentStencilState.m_StencilOP_zFailFront),
						mapStencilOpOGL (m_CurrentStencilState.m_StencilOP_zPassFront));

					glStencilMask (m_CurrentStencilState.m_uiStencilMaskFront);

					glStencilFunc (mapCompareFuncOGL (m_CurrentStencilState.m_StencilFuncFront), 
						m_CurrentStencilState.m_StencilFuncRefFront,
						m_CurrentStencilState.m_StencilFuncMaskFront);
				}
				else
				if (GLEW_ATI_separate_stencil)
				{
					// does not support the same features as EXT_stencil_two_side, only a subset

					// back OP
					glStencilOpSeparateATI (GL_BACK, mapStencilOpOGL (m_CurrentStencilState.m_StencilOP_FailBack),
						mapStencilOpOGL (m_CurrentStencilState.m_StencilOP_zFailBack),
						mapStencilOpOGL (m_CurrentStencilState.m_StencilOP_zPassBack));

					// front OP
					glStencilOpSeparateATI (GL_FRONT, mapStencilOpOGL (m_CurrentStencilState.m_StencilOP_FailFront),
						mapStencilOpOGL (m_CurrentStencilState.m_StencilOP_zFailFront),
						mapStencilOpOGL (m_CurrentStencilState.m_StencilOP_zPassFront));

					// front func, back func, only front ref and mask value
					glStencilFuncSeparateATI (mapCompareFuncOGL (m_CurrentStencilState.m_StencilFuncFront), 
						mapCompareFuncOGL (m_CurrentStencilState.m_StencilFuncBack), 
						m_CurrentStencilState.m_StencilFuncRefFront,
						m_CurrentStencilState.m_StencilFuncMaskFront);

					glStencilMask (m_CurrentStencilState.m_uiStencilMaskFront);
				}


				m_AppliedStencilState = m_CurrentStencilState;

				
			}
		}

		if (m_bAutoCheckErrors)
			aeRenderAPI::CheckErrors ("(auto-check) After ApplyStencilStates");
	}
#endif

	void aeRenderAPI::PushStencilStates (bool bResetToDefault)
	{
		AE_CHECK_DEV (aeRenderAPI_Impl::m_StencilStateStack.size () < 100, "aeRenderAPI::PushStencilStates: Stack Size is 100. You are doing it wrong.");

		aeRenderAPI_Impl::m_StencilStateStack.push (aeRenderAPI_Impl::m_CurrentStencilState);

		if (bResetToDefault)
			aeRenderAPI_Impl::m_CurrentStencilState.setToDefault ();
	}

	void aeRenderAPI::PopStencilStates ()
	{
		AE_CHECK_DEV (!aeRenderAPI_Impl::m_StencilStateStack.empty (), "aeRenderAPI::PopStencilStates: Stack is empty. You do not call Push/Pop in concert.");

		aeRenderAPI_Impl::m_CurrentStencilState = aeRenderAPI_Impl::m_StencilStateStack.top ();

		aeRenderAPI_Impl::m_StencilStateStack.pop ();
	}
}


