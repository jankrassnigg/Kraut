// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#ifndef AE_GRAPHICS_TEXTUREMANAGER_TEXTURERESOURCE_H
#define AE_GRAPHICS_TEXTUREMANAGER_TEXTURERESOURCE_H

#include "Declarations.h"

namespace AE_NS_GRAPHICS
{
	struct aeTextureResourceDescriptor
	{
		aeTextureResourceDescriptor ()
		{
			m_uiWidth = 1;
			m_uiHeight = 1;
			m_uiDepth = 1;
			m_uiArraySlices = 1;
			m_iDefaultTextureSamplerID = -1;

			m_TextureType = AE_TYPE_UNDEFINED;
			m_TextureFormat = AE_FORMAT_UNDEFINED;

			m_bGenerateMipmaps = false;
			m_pInitialData = NULL;

			m_bAllowToReadContents = false;
		}

		aeUInt16 m_uiWidth;
		aeUInt16 m_uiHeight;
		aeUInt16 m_uiDepth;
		aeUInt16 m_uiArraySlices;
		aeInt16 m_iDefaultTextureSamplerID;

		bool m_bAllowToReadContents;
		bool m_bGenerateMipmaps;
		void* m_pInitialData;

		AE_TEXTURE_TYPE m_TextureType;
		AE_TEXTURE_FORMAT m_TextureFormat;
	};

	class AE_GRAPHICS_DLL aeTextureResource : public aeResource
	{
    AE_RESOURCE_DECLARATION (aeTextureResource)

	public:
		aeTextureResource ();

		void setDefaultTextureSampler (aeUInt16 uiSamplerID) {m_uiDefaultTextureSamplerID = uiSamplerID;}

#ifdef AE_RENDERAPI_OPENGL
		aeUInt32 getOpenGLID (void) const {return (m_uiTextureIDOGL);}
		GLenum getOpenGLTextureType (void) const {return (m_TextureTypeOGL);}
#else
		aeUInt32 getOpenGLID (void) const {return (0);}
		aeUInt32 getOpenGLTextureType (void) const {return (0);}
#endif

		aeUInt32 getTextureWidth (void) const {return (m_uiWidth);}
		aeUInt32 getTextureHeight (void) const {return (m_uiHeight);}
		aeUInt32 getTextureDepth (void) const {return (m_uiDepth);}

		void GenerateMipmaps (void);

		void ReadbackTexture (aeUInt8* pData);

	private:
		friend class aeTextureManager;

		virtual void LoadResourceFromFile (const char* szFilename);

		virtual void CreateResourceFromDescriptor (void* pDescriptor);

		virtual void UnloadResource (void);

	private:

#ifdef AE_RENDERAPI_OPENGL
		aeUInt32 m_uiTextureIDOGL;
		GLenum m_TextureTypeOGL;
#endif

		aeUInt16 m_uiDefaultTextureSamplerID;
		aeUInt32 m_uiWidth;
		aeUInt32 m_uiHeight;
		aeUInt32 m_uiDepth;

		AE_TEXTURE_FORMAT m_TextureFormat;

		bool m_bHasMipmaps;
	};



}

#endif



