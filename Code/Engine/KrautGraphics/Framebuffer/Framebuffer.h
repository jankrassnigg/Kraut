// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// Author:	Christopher Manthei (Neelix8472@gmx.net)
// >>> Information <<<

#ifndef AE_GRAPHICS_FRAMEBUFFER_FRAMEBUFFER_H
#define AE_GRAPHICS_FRAMEBUFFER_FRAMEBUFFER_H

#include "Declarations.h"
#include "../TextureManager/Declarations.h"

namespace AE_NS_GRAPHICS
{
	//! A class that encapsulates the use of OpenGL Framebuffer Objects for Render-To-Texture.
	/*!	The class uses aeTextureResource's as render-targets. OpenGL's Renderbuffers are not supported, because the same functionality
		can be achieved with textures, plus textures can be sampled later on, Renderbuffers not.
		A texture can be attached to any number of aeFramebuffer's (though only to one attachment point each, of course).
		When a Framebuffer is not needed anymore, call "Shutdown". It can be reused thereafter if needed.
		BindFramebuffer only needs to be called, when one wants to render TO it. To copy color or depth data between buffers (textures),
		it is not necessary to bind any framebuffers.
	*/
	class AE_GRAPHICS_DLL aeFramebuffer
	{
	public:
		aeFramebuffer ();
		~aeFramebuffer ();

		static void BindBackbuffer (void);

		static void ClearCurrentBuffers (bool bColor, bool bDepthStencil, float fRed = 0.0f, float fGreen = 0.0f, float fBlue = 0.0f, float fAlpha = 0.0f, float fDepth = 1.0f, aeUInt8 uiStencil = 0);

		//! Deletes all used resources, including textures, if those are exclusively used by this Framebuffer. Can be reused afterwards.
		void Shutdown (void);

		//! Binds all render-targets to their respective bind points.
		void BindFramebuffer (void);

		//! Sets the to-be-used depth texture. Takes affect at next BindFramebuffer.
    void setDepthAttachment (const aeTextureResourceHandle& Texture);
		//! Sets the to-be-used depth texture (creates it first). Takes affect at next BindFramebuffer.
		void setDepthAttachment (const aeTextureResourceDescriptor& TexDisc);
		//! Returns the handle to the currently used depth-texture. Handle might be invalid.
		aeTextureResourceHandle getDepthAttachment (void) const;


		//! Sets the to-be-used color texture for a given bind point. Takes affect at next BindFramebuffer.
		void setColorAttachmentCubemapFace (aeUInt8 uiAttachmentPoint, const aeTextureResourceHandle& Texture, AE_CUBEMAP_FACE Face);

		//! Sets the to-be-used color texture for a given bind point. Takes affect at next BindFramebuffer.
		void setColorAttachment (aeUInt8 uiAttachmentPoint, const aeTextureResourceHandle& Texture);
		//! Sets the to-be-used color texture for a given bind point (creates it first). Takes affect at next BindFramebuffer.
		void setColorAttachmentDesc (aeUInt8 uiAttachmentPoint, const aeTextureResourceDescriptor& TexDisc);
		//! Returns the handle to the currently texture on that bind-point. Handle might be invalid.
		aeTextureResourceHandle getColorAttachment (aeUInt8 uiAttachmentPoint) const;

		//! Copies the depth-buffer from the given Framebuffer into the depth-texture of this framebuffer.
		void CopyDepth (aeFramebuffer& SrcFB);
		//! Copies the color-buffer at bind point "dst" from the given Framebuffer into the color-texture of this framebuffer at bind point "src".
		void CopyColor (aeFramebuffer& SrcFB, aeUInt8 iSrcAttachmentPoint, aeUInt8 iDstAttachmentPoint);


#ifdef AE_RENDERAPI_OPENGL
		//! Returns OpenGLs FBO ID.
		aeUInt32 getOpenGLID (void) const {return (m_uiFBO);}

		//! Sets the to-be-used depth render buffer. Takes effect at the next BindFramebuffer.
		void setDepthAttachmentRenderBuffer (aeUInt32 uiRenderBufferID, aeUInt32 uiWidth, aeUInt32 uiHeight);
		//! Sets the to-be-used color render buffer for a given bind point. Takes affect at next BindFramebuffer.
		void setColorAttachmentRenderBuffer (aeUInt8 uiAttachmentPoint, aeUInt32 uiRenderBufferID, aeUInt32 uiWidth, aeUInt32 uiHeight);
#endif

		//! Returns whether the currently bound buffer provides a depth-buffer.
		static bool isDepthBufferAvailable (void) {return (s_bDepthBufferAvailable);}

	private:
		static const aeUInt32 GC_MAX_RENDERTARGETS = 8;

		//! Sets the Framebuffer up, if things have changed.
		void SetupFramebuffer (void);

#ifdef AE_RENDERAPI_OPENGL

		aeUInt32 m_uiFBO;
		bool m_bSetupFBO;

		aeUInt32 m_uiDepthStencilRT_ID;
		bool m_bUseDepthStencilRenderBuffer;
		aeUInt32 m_uiDepthStencilWidth;
		aeUInt32 m_uiDepthStencilHeight;

		aeUInt32 m_uiColorRT_ID[GC_MAX_RENDERTARGETS];
		bool m_bUseColorRenderBuffer[GC_MAX_RENDERTARGETS];
		aeUInt32 m_uiColorWidth[GC_MAX_RENDERTARGETS];
		aeUInt32 m_uiColorHeight[GC_MAX_RENDERTARGETS];

		GLenum m_ColorBuffers[GC_MAX_RENDERTARGETS];
#else
  #error "RenderAPI undefined"
#endif

		static bool s_bDepthBufferAvailable;

		static aeFramebuffer* s_pCurrentlyBoundFramebuffer;

		// which face of the given texture to bind
		AE_CUBEMAP_FACE m_ColorBufferCubemapFace[GC_MAX_RENDERTARGETS];

		aeTextureResourceHandle m_DepthStencilRT;
		aeTextureResourceHandle m_ColorRT[GC_MAX_RENDERTARGETS];

		aeInt32 m_iColorBuffers;
	};


}

#endif


