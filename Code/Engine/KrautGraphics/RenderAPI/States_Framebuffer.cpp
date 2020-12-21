// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#include "Main.h"
#include "RenderAPI_Impl.h"
#include "../Framebuffer/Main.h"



namespace AE_NS_GRAPHICS
{
	aeUInt32 aeRenderAPI_Impl::s_uiWindowSizeX = 640;
	aeUInt32 aeRenderAPI_Impl::s_uiWindowSizeY = 480;

	void aeRenderAPI::Clear (bool bColor, bool bDepthStencil, float fRed, float fGreen, float fBlue, float fAlpha, float fDepth, aeUInt8 uiStencil)
	{
		aeFramebuffer::ClearCurrentBuffers (bColor, bDepthStencil, fRed, fGreen, fBlue, fAlpha, fDepth, uiStencil);
	}


	void aeRenderAPI::PushFramebufferStates (bool bResetToDefault)
	{
		AE_CHECK_DEV (aeRenderAPI_Impl::m_FBStateStack.size () < 100, "aeRenderAPI::PushFramebufferStates: Stack Size is 100. You are doing it wrong.");

		aeRenderAPI_Impl::m_FBStateStack.push (aeRenderAPI_Impl::m_CurrentFBState);

		if (bResetToDefault)
			aeRenderAPI_Impl::m_CurrentFBState.setToDefault ();
	}

	void aeRenderAPI::PopFramebufferStates ()
	{
		AE_CHECK_DEV (!aeRenderAPI_Impl::m_FBStateStack.empty (), "aeRenderAPI::PopFramebufferStates: Stack is empty. You do not call Push/Pop in concert.");

		aeRenderAPI_Impl::m_CurrentFBState = aeRenderAPI_Impl::m_FBStateStack.top ();

		aeRenderAPI_Impl::m_FBStateStack.pop ();

		aeRenderAPI_Impl::m_bChangeViewportOrDepthRange = true;
	}

	void aeRenderAPI::setViewport (aeUInt32 x, aeUInt32 y, aeUInt32 width, aeUInt32 height)
	{
		aeRenderAPI_Impl::m_CurrentFBState.m_uiViewportX = x;
		aeRenderAPI_Impl::m_CurrentFBState.m_uiViewportY = y;

		if (width == 0)
			aeRenderAPI_Impl::m_CurrentFBState.m_uiViewportWidth = aeRenderAPI_Impl::s_uiWindowSizeX;
		else
			aeRenderAPI_Impl::m_CurrentFBState.m_uiViewportWidth = width;

		if (height == 0)
			aeRenderAPI_Impl::m_CurrentFBState.m_uiViewportHeight = aeRenderAPI_Impl::s_uiWindowSizeY;
		else
			aeRenderAPI_Impl::m_CurrentFBState.m_uiViewportHeight = height;

		aeRenderAPI_Impl::m_bChangeViewportOrDepthRange = true;
	}

	void aeRenderAPI::getViewportAndDepthRange (aeUInt32& out_x, aeUInt32& out_y, aeUInt32& out_width, aeUInt32& out_height, float& out_fNear, float& out_fFar)
	{
		out_x = aeRenderAPI_Impl::m_CurrentFBState.m_uiViewportX;
		out_y = aeRenderAPI_Impl::m_CurrentFBState.m_uiViewportY;
		out_width = aeRenderAPI_Impl::m_CurrentFBState.m_uiViewportWidth;
		out_height= aeRenderAPI_Impl::m_CurrentFBState.m_uiViewportHeight;

		out_fNear = aeRenderAPI_Impl::m_CurrentFBState.m_fDepthRangeNear;
		out_fFar  = aeRenderAPI_Impl::m_CurrentFBState.m_fDepthRangeFar;
	}


	void aeRenderAPI::setDepthClip (bool bDepthClip)
	{
		aeRenderAPI_Impl::m_CurrentRenderState.m_bDepthClip = bDepthClip;
	}

	void aeRenderAPI::setDepthRange (float fNear, float fFar)
	{
		aeRenderAPI_Impl::m_CurrentFBState.m_fDepthRangeNear = fNear;
		aeRenderAPI_Impl::m_CurrentFBState.m_fDepthRangeFar  = fFar;

		aeRenderAPI_Impl::m_bChangeViewportOrDepthRange = true;
	}


	void aeRenderAPI::setScissorTest (bool bEnable, aeUInt32 x, aeUInt32 y, aeUInt32 width, aeUInt32 height)
	{
		if (bEnable)
		{
			aeRenderAPI_Impl::m_CurrentFBState.m_bScissorTest = true;
			aeRenderAPI_Impl::m_CurrentFBState.m_uiScissorX = x;
			aeRenderAPI_Impl::m_CurrentFBState.m_uiScissorY = y;
			aeRenderAPI_Impl::m_CurrentFBState.m_uiScissorWidth = width;
			aeRenderAPI_Impl::m_CurrentFBState.m_uiScissorHeight = height;
		}
		else
		{
			aeRenderAPI_Impl::m_CurrentFBState.m_bScissorTest = false;
			aeRenderAPI_Impl::m_CurrentFBState.m_uiScissorX = 0;
			aeRenderAPI_Impl::m_CurrentFBState.m_uiScissorY = 0;
			aeRenderAPI_Impl::m_CurrentFBState.m_uiScissorWidth = aeRenderAPI_Impl::s_uiWindowSizeX;
			aeRenderAPI_Impl::m_CurrentFBState.m_uiScissorHeight = aeRenderAPI_Impl::s_uiWindowSizeY;
		}
	}


#ifdef AE_RENDERAPI_OPENGL
	void aeRenderAPI_Impl::ApplyFBStates (bool bForce)
	{
		if (m_bAutoCheckErrors)
			aeRenderAPI::CheckErrors ("(auto-check) Before ApplyFBStates");

		if ((bForce) || (m_CurrentFBState.m_fDepthRangeNear != m_AppliedFBState.m_fDepthRangeNear)
			|| (m_CurrentFBState.m_fDepthRangeFar != m_AppliedFBState.m_fDepthRangeFar))
		{
			glDepthRange (m_CurrentFBState.m_fDepthRangeNear, m_CurrentFBState.m_fDepthRangeFar);
		}

		if ((bForce) || (m_CurrentFBState.m_bScissorTest != m_AppliedFBState.m_bScissorTest)
			|| (m_CurrentFBState.m_uiScissorX != m_AppliedFBState.m_uiScissorX)
			|| (m_CurrentFBState.m_uiScissorY != m_AppliedFBState.m_uiScissorY)
			|| (m_CurrentFBState.m_uiScissorWidth != m_AppliedFBState.m_uiScissorWidth)
			|| (m_CurrentFBState.m_uiScissorHeight != m_AppliedFBState.m_uiScissorHeight))
		{
			if (m_CurrentFBState.m_bScissorTest)
			{
				glEnable (GL_SCISSOR_TEST);
				glScissor (m_CurrentFBState.m_uiScissorX, m_CurrentFBState.m_uiScissorY,
					m_CurrentFBState.m_uiScissorWidth, m_CurrentFBState.m_uiScissorHeight);
			}
			else
			{
				glDisable (GL_SCISSOR_TEST);
			}
		}

		if ((bForce) || (m_CurrentFBState.m_uiViewportX != m_AppliedFBState.m_uiViewportX)
			|| (m_CurrentFBState.m_uiViewportY != m_AppliedFBState.m_uiViewportY)
			|| (m_CurrentFBState.m_uiViewportWidth != m_AppliedFBState.m_uiViewportWidth)
			|| (m_CurrentFBState.m_uiViewportHeight != m_AppliedFBState.m_uiViewportHeight)
      || (aeRenderAPI_Impl::m_bChangeViewportOrDepthRange))
		{
			glViewport (m_CurrentFBState.m_uiViewportX, m_CurrentFBState.m_uiViewportY,
				m_CurrentFBState.m_uiViewportWidth, m_CurrentFBState.m_uiViewportHeight);
		}

		m_AppliedFBState = m_CurrentFBState;

		if (m_bAutoCheckErrors)
			aeRenderAPI::CheckErrors ("(auto-check) After ApplyFBStates");
	}
#endif

}

