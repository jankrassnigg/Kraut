// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// Author:	Christopher Manthei (Neelix8472@gmx.net)
// >>> Information <<<

#include "Framebuffer.h"
#include "../RenderAPI/Main.h"
#include "../TextureManager/TextureResource.h"


namespace AE_NS_GRAPHICS
{
	aeFramebuffer* aeFramebuffer::s_pCurrentlyBoundFramebuffer = nullptr;
	bool aeFramebuffer::s_bDepthBufferAvailable = false;

	aeFramebuffer::aeFramebuffer ()
	{
		m_iColorBuffers = 0;

#ifdef AE_RENDERAPI_OPENGL
		if (g_RenderAPI == AE_RA_OPENGL)
		{
			m_uiFBO = 0;
			m_bSetupFBO = true;

			m_bUseDepthStencilRenderBuffer = false;
      m_uiDepthStencilRT_ID = 0;
      m_uiDepthStencilWidth = 0;
      m_uiDepthStencilHeight = 0;

			for (aeInt32 i = 0; i < GC_MAX_RENDERTARGETS; ++i)
			{
				m_ColorBufferCubemapFace[i] = AE_CUBEMAP_NONE;
				m_bUseColorRenderBuffer[i] = false;
        m_uiColorRT_ID[i] = 0;
        m_uiColorWidth[i] = 0;
        m_uiColorHeight[i] = 0;
			}
		}
#endif
	}

	aeFramebuffer::~aeFramebuffer ()
	{
#ifdef AE_RENDERAPI_OPENGL
		if (g_RenderAPI == AE_RA_OPENGL)
		{
			AE_CHECK_DEV (m_uiFBO == 0, "aeFramebuffer::~aeFramebuffer: You need to call \"Shutdown\" on each aeFramebuffer, before destroying it.");
		}
#endif
	}

	void aeFramebuffer::Shutdown (void)
	{
#ifdef AE_RENDERAPI_OPENGL
		if (g_RenderAPI == AE_RA_OPENGL)
		{
			m_bSetupFBO = true;

			if (m_uiFBO != 0)
			{
				glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, 0);

				glDeleteFramebuffersEXT (1, &m_uiFBO);
				m_uiFBO = 0;
			}
		}
#endif

		m_DepthStencilRT.Invalidate ();

		for (aeUInt32 i = 0; i < GC_MAX_RENDERTARGETS; ++i)
			m_ColorRT[i].Invalidate ();
	}

	void aeFramebuffer::BindBackbuffer (void)
	{
		s_pCurrentlyBoundFramebuffer = nullptr;
		s_bDepthBufferAvailable = false;

#ifdef AE_RENDERAPI_OPENGL
		if (g_RenderAPI == AE_RA_OPENGL)
		{
			glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, 0);
		}
#endif
	}

	void aeFramebuffer::SetupFramebuffer (void)
	{
#ifdef AE_RENDERAPI_OPENGL
		if (g_RenderAPI == AE_RA_OPENGL)
		{
			if (!m_bSetupFBO)
				return;

			m_bSetupFBO = false;

			if (m_uiFBO == 0)
			{
				glGenFramebuffersEXT (1, &m_uiFBO);
			}

			glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, m_uiFBO);

			if ((m_bUseDepthStencilRenderBuffer) || (m_DepthStencilRT.IsValid ()))
			{
				if (m_bUseDepthStencilRenderBuffer)
					glFramebufferRenderbufferEXT (GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, m_uiDepthStencilRT_ID);
				else
					glFramebufferTexture2DEXT (GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, m_uiDepthStencilRT_ID, 0);
			}

			m_iColorBuffers = 0;

			for (aeUInt32 i = 0; i < GC_MAX_RENDERTARGETS; ++i)
			{
				//! \todo Handle 3D Textures and Cubemaps and Texture Arrays too

				if ((m_bUseColorRenderBuffer[i]) || (m_ColorRT[i].IsValid ()))
				{
					m_ColorBuffers[m_iColorBuffers] = GL_COLOR_ATTACHMENT0_EXT + i;
					++m_iColorBuffers;

					if (m_bUseColorRenderBuffer[i])
						glFramebufferRenderbufferEXT (GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT + i,  GL_RENDERBUFFER_EXT, m_uiColorRT_ID[i]);
					else
					if (m_ColorBufferCubemapFace[i] != AE_CUBEMAP_NONE)
						glFramebufferTexture2DEXT (GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT + i,  mapCubemapFaceOGL (m_ColorBufferCubemapFace[i]), m_uiColorRT_ID[i], 0);
					else
						glFramebufferTexture2DEXT (GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT + i,  m_ColorRT[i].GetResource ()->getOpenGLTextureType(), m_uiColorRT_ID[i], 0);
				}
				else
				{
					// detach anything that might be bound here
					glFramebufferTexture2DEXT (GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT + i, GL_TEXTURE_2D, 0, 0);
				}
			}

			if (m_iColorBuffers == 0)
			{
				glReadBuffer (GL_NONE);
				glDrawBuffer (GL_NONE);
			}
			else
			{
				glReadBuffer (m_ColorBuffers[0]);
				glDrawBuffers (m_iColorBuffers, m_ColorBuffers);
			}
		}
#endif
	}

	void aeFramebuffer::BindFramebuffer (void)
	{
		s_pCurrentlyBoundFramebuffer = this;
		s_bDepthBufferAvailable = m_DepthStencilRT.IsValid ();

		SetupFramebuffer ();

#ifdef AE_RENDERAPI_OPENGL
		if (g_RenderAPI == AE_RA_OPENGL)
		{
			AE_RENDERAPI_CHECK b ("aeFramebuffer::BindFramebuffer");

			glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, m_uiFBO);

			if (m_iColorBuffers == 0)
			{
				glReadBuffer (GL_NONE);
				glDrawBuffer (GL_NONE);
			}
			else
			{
				glReadBuffer (m_ColorBuffers[0]);
				glDrawBuffers (m_iColorBuffers, m_ColorBuffers);
			}
		}
#endif
	}

	void aeFramebuffer::setDepthAttachment (const aeTextureResourceHandle& Texture)
	{
		m_DepthStencilRT = Texture;

#ifdef AE_RENDERAPI_OPENGL
    if (g_RenderAPI == AE_RA_OPENGL)
    {
      m_bSetupFBO = true;

      m_uiDepthStencilRT_ID = m_DepthStencilRT.GetResource()->getOpenGLID();
      m_bUseDepthStencilRenderBuffer = false;

      m_uiDepthStencilWidth = m_DepthStencilRT.GetResource()->getTextureWidth();
      m_uiDepthStencilHeight = m_DepthStencilRT.GetResource()->getTextureHeight();
    }
#endif		
	}

	void aeFramebuffer::setDepthAttachment (const aeTextureResourceDescriptor& TexDisc)
	{
		aeTextureResourceHandle hTexTemp;
		aeTextureResource::CreateResource (hTexTemp, (void*) &TexDisc, false);

		setDepthAttachment (hTexTemp);
	}

#ifdef AE_RENDERAPI_OPENGL
	void aeFramebuffer::setDepthAttachmentRenderBuffer (aeUInt32 uiRenderBufferID, aeUInt32 uiWidth, aeUInt32 uiHeight)
	{
		m_bSetupFBO = true;

		m_DepthStencilRT.Invalidate ();
		m_uiDepthStencilRT_ID = uiRenderBufferID;
		m_bUseDepthStencilRenderBuffer = true;

		m_uiDepthStencilWidth = uiWidth;
		m_uiDepthStencilHeight = uiHeight;
	}
#endif

	aeTextureResourceHandle aeFramebuffer::getDepthAttachment (void) const
	{
		return (m_DepthStencilRT);
	}

	void aeFramebuffer::setColorAttachmentCubemapFace (aeUInt8 uiAttachmentPoint, const aeTextureResourceHandle& Texture, AE_CUBEMAP_FACE Face)
	{
		AE_CHECK_DEV (uiAttachmentPoint < GC_MAX_RENDERTARGETS, "aeFramebuffer::setColorAttachment: Attachment Point %d is invalid.", uiAttachmentPoint);

		m_ColorRT[uiAttachmentPoint] = Texture;
		m_ColorBufferCubemapFace[uiAttachmentPoint] = Face;

#ifdef AE_RENDERAPI_OPENGL
		if (g_RenderAPI == AE_RA_OPENGL)
		{
			m_bSetupFBO = true;

			m_uiColorRT_ID[uiAttachmentPoint] = Texture.GetResource ()->getOpenGLID ();
			m_bUseColorRenderBuffer[uiAttachmentPoint] = false;

			m_uiColorWidth[uiAttachmentPoint] = Texture.GetResource ()->getTextureWidth ();
			m_uiColorHeight[uiAttachmentPoint] = Texture.GetResource ()->getTextureHeight ();
		}
#endif
	}

	void aeFramebuffer::setColorAttachment (aeUInt8 uiAttachmentPoint, const aeTextureResourceHandle& Texture)
	{
		//AE_CHECK_DEV (uiAttachmentPoint < GC_MAX_RENDERTARGETS, "aeFramebuffer::setColorAttachment: Attachment Point %d is invalid.", uiAttachmentPoint);

		m_ColorRT[uiAttachmentPoint] = Texture;
		m_ColorBufferCubemapFace[uiAttachmentPoint] = AE_CUBEMAP_NONE;

#ifdef AE_RENDERAPI_OPENGL
		if (g_RenderAPI == AE_RA_OPENGL)
		{
			m_bSetupFBO = true;

			m_uiColorRT_ID[uiAttachmentPoint] = Texture.GetResource ()->getOpenGLID ();
			m_bUseColorRenderBuffer[uiAttachmentPoint] = false;

			m_uiColorWidth[uiAttachmentPoint] = Texture.GetResource ()->getTextureWidth ();
			m_uiColorHeight[uiAttachmentPoint] = Texture.GetResource ()->getTextureHeight ();
		}
#endif
	}

	void aeFramebuffer::setColorAttachmentDesc(aeUInt8 uiAttachmentPoint, const aeTextureResourceDescriptor& TexDisc)
	{
		aeTextureResourceHandle hTexTemp;
		aeTextureResource::CreateResource (hTexTemp, (void*) &TexDisc, false);

		setColorAttachment (uiAttachmentPoint, hTexTemp);
	}

#ifdef AE_RENDERAPI_OPENGL
	void aeFramebuffer::setColorAttachmentRenderBuffer (aeUInt8 uiAttachmentPoint, aeUInt32 uiRenderBufferID, aeUInt32 uiWidth, aeUInt32 uiHeight)
	{
		m_bSetupFBO = true;

		m_ColorRT[uiAttachmentPoint].Invalidate ();
		m_uiColorRT_ID[uiAttachmentPoint] = uiRenderBufferID;
		m_bUseColorRenderBuffer[uiAttachmentPoint] = true;

		m_uiColorWidth[uiAttachmentPoint] = uiWidth;
		m_uiColorHeight[uiAttachmentPoint] = uiHeight;
	}
#endif

	aeTextureResourceHandle aeFramebuffer::getColorAttachment (aeUInt8 uiAttachmentPoint) const
	{
		AE_CHECK_DEV (uiAttachmentPoint < GC_MAX_RENDERTARGETS, "aeFramebuffer::getColorAttachment: Attachment Point %d is invalid.", uiAttachmentPoint);

		return (m_ColorRT[uiAttachmentPoint]);
	}

	void aeFramebuffer::CopyDepth (aeFramebuffer& SrcFB)
	{
#ifdef AE_RENDERAPI_OPENGL
		if (g_RenderAPI == AE_RA_OPENGL)
		{
			// make sure all FBOs are setup
			SetupFramebuffer ();
			SrcFB.SetupFramebuffer ();

			// to apply depth mask etc.
			aeRenderAPI::PushRenderStates (true);
			aeRenderAPI::setDepthMask (true);
			aeRenderAPI::ApplyStates ();

			glBindFramebufferEXT (GL_READ_FRAMEBUFFER_EXT, SrcFB.m_uiFBO);
			glBindFramebufferEXT (GL_DRAW_FRAMEBUFFER_EXT, m_uiFBO);

			const aeUInt32 uiWidth  = m_uiDepthStencilWidth;
			const aeUInt32 uiHeight = m_uiDepthStencilHeight;

			glBlitFramebufferEXT (0, 0, uiWidth, uiHeight,
								  0, 0, uiWidth, uiHeight,
									GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT,
									GL_NEAREST);


			aeRenderAPI::CheckErrors ("aeFramebuffer::CopyDepth");

			aeRenderAPI::PopRenderStates ();
		}
#endif
	}

	void aeFramebuffer::CopyColor (aeFramebuffer& SrcFB, aeUInt8 iSrcAttachmentPoint, aeUInt8 iDstAttachmentPoint)
	{
#ifdef AE_RENDERAPI_OPENGL
		if (g_RenderAPI == AE_RA_OPENGL)
		{
			// make sure all FBOs are setup
			SetupFramebuffer ();
			SrcFB.SetupFramebuffer ();

			// to apply color masks etc.
			aeRenderAPI::ApplyStates ();

			glBindFramebufferEXT (GL_READ_FRAMEBUFFER_EXT, SrcFB.m_uiFBO);
			glBindFramebufferEXT (GL_DRAW_FRAMEBUFFER_EXT, m_uiFBO);

			// define from which layer to which other to copy the data
			glReadBuffer (GL_COLOR_ATTACHMENT0_EXT + iSrcAttachmentPoint);
			glDrawBuffer (GL_COLOR_ATTACHMENT0_EXT + iDstAttachmentPoint);

			const aeUInt32 uiSrcWidth  = SrcFB.m_uiColorWidth[iSrcAttachmentPoint];
			const aeUInt32 uiSrcHeight = SrcFB.m_uiColorHeight[iSrcAttachmentPoint];

			const aeUInt32 uiDstWidth  = m_uiColorWidth[iDstAttachmentPoint];
			const aeUInt32 uiDstHeight = m_uiColorHeight[iDstAttachmentPoint];

			const bool bEqualDim = ((uiSrcWidth == uiDstWidth) && (uiSrcHeight == uiDstHeight));

			glBlitFramebufferEXT (0, 0, uiSrcWidth, uiSrcHeight,
								  0, 0, uiDstWidth, uiDstHeight, 
								  GL_COLOR_BUFFER_BIT, 
								  bEqualDim ? GL_NEAREST : GL_LINEAR);

			aeRenderAPI::CheckErrors ("aeFramebuffer::CopyColor");
		}
#endif
	}

	void aeFramebuffer::ClearCurrentBuffers (bool bColor, bool bDepthStencil, float fRed, float fGreen, float fBlue, float fAlpha, float fDepth, aeUInt8 uiStencil)
	{
		aeRenderAPI::ApplyStates ();

#ifdef AE_RENDERAPI_OPENGL
		if (g_RenderAPI == AE_RA_OPENGL)
		{
			aeUInt32 uiBits = 0;

			if (bColor)
			{
				uiBits |= GL_COLOR_BUFFER_BIT;
				glClearColorIuiEXT (0, 0, 0, 0);
				glClearColorIiEXT (0, 0, 0, 0);

				glClearColor (fRed, fGreen, fBlue, fAlpha);
			}

			if (bDepthStencil)
			{
				uiBits |= GL_DEPTH_BUFFER_BIT;
				uiBits |= GL_STENCIL_BUFFER_BIT;
				glClearDepth (fDepth);
				glClearStencil (uiStencil);
			}

			glClear (uiBits);
		}
#endif
	}
}

