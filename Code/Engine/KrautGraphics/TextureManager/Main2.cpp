// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#include "Main.h"
#include "TGALoader.h"
#include "DDSLoader.h"
#include <KrautGraphics/RenderAPI/RenderAPI.h>
#include <KrautFoundation/Configuration/CVar.h>
#include <KrautFoundation/FileSystem/FileSystem.h>


namespace AE_NS_GRAPHICS
{
	extern aeCVarInt g_CVarTextureQuality;
	extern aeCVarInt g_CVarMaxTextureResolution;

#ifdef AE_RENDERAPI_OPENGL
	AE_GRAPHICS_DLL bool DDSLoaderOGL (const char* szFile, aeUInt32 uiOpenGLTextureID, aeUInt32& uiOpenGLTextureType, aeUInt32& uiWidth, aeUInt32& uiHeight, aeUInt32& uiDepth);
	AE_GRAPHICS_DLL bool TGALoaderOGL (const char* szFile, aeUInt32 uiOpenGLTextureID, aeUInt32& uiOpenGLTextureType, aeUInt32& uiWidth, aeUInt32& uiHeight, aeUInt32& uiDepth);
#endif

	aeUInt32 aeTexManImpl::m_uiTextureSwitches = 0;

  std::vector<aeTextureResourceHandle> aeTexManImpl::m_BoundTextures;
  std::vector<aeInt32> aeTexManImpl::m_BoundSamplers;

#ifdef AE_RENDERAPI_OPENGL
  std::map<aeStaticString<16>, AE_TEXTURE_FILE_LOADER_OGL> aeTexManImpl::m_FileLoadersOGL;
#endif
  std::map<aeStaticString<24>, aeUInt16> aeTexManImpl::m_SamplerIDs;
  std::vector<aeSamplerState> aeTexManImpl::m_TextureSamplers;

	AE_TEXTURE_FILTER aeTexManImpl::m_DefaultFilter = AE_FILTER_ANISOTROPIC_2X;


	void aeTextureManager::CoreStartup (void)
	{
		aeInt32 iMaxTextureUnits;

		//! \todo Query this value
		iMaxTextureUnits = 16;

#ifdef AE_RENDERAPI_OPENGL
		if (g_RenderAPI == AE_RA_OPENGL)
		{
			RegisterFileLoader ("DDS", DDSLoaderOGL);
			RegisterFileLoader ("TGA", TGALoaderOGL);
			RegisterFileLoader ("RGBE", TGALoaderOGL);
		}
#endif
		//RegisterFileLoader ("BMP", D3DXLoader);
		//RegisterFileLoader ("JPG", D3DXLoader);
		//RegisterFileLoader ("PNG", D3DXLoader);
		//RegisterFileLoader ("JPEG", D3DXLoader);

		aeTexManImpl::m_BoundTextures.resize (iMaxTextureUnits);
		aeTexManImpl::m_BoundSamplers.resize (iMaxTextureUnits);
	}

	void aeTextureManager::EngineStartup (void)
	{
#ifdef AE_RENDERAPI_OPENGL
		if (g_RenderAPI == AE_RA_OPENGL)
		{
			aeInt32 iMaxTextureUnits;

			//aeInt32 iMaxAnisotropy;
			//glGetIntegerv (GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &iMaxAnisotropy);

			glGetIntegerv (GL_MAX_TEXTURE_IMAGE_UNITS, (GLint*) &iMaxTextureUnits);

			aeTexManImpl::m_BoundTextures.resize (iMaxTextureUnits);
			aeTexManImpl::m_BoundSamplers.resize (iMaxTextureUnits);
		}
#endif		

		aeSamplerState Default;
		setTextureSampler ("Default", Default);

		//Default.m_TextureFilter = AE_FILTER_NEAREST;
		Default.m_TextureWrapU = AE_WRAP_CLAMP;
		Default.m_TextureWrapV = AE_WRAP_CLAMP;
		Default.m_TextureWrapW = AE_WRAP_CLAMP;

		setTextureSampler ("DefaultCubemap", Default);
	}

	void aeTextureManager::EngineShutdown (void)
	{
		DestroyTextureSamplers ();
		aeTexManImpl::m_BoundTextures.clear ();
		aeTexManImpl::m_BoundSamplers.clear ();
	}

#ifdef AE_RENDERAPI_OPENGL
	bool DDSLoaderOGL (const char* szFile, aeUInt32 uiOpenGLTextureID, aeUInt32& uiOpenGLTextureType, aeUInt32& uiWidth, aeUInt32& uiHeight, aeUInt32& uiDepth)
	{
		aeDDSLoader Loader;
		if (!Loader.LoadDDS (szFile, false))
			return (false);

		aeRenderAPI::CheckErrors ("DDSLoader: After LoadDDS"); 

		aeUInt8 uiQuality = g_CVarTextureQuality;
		aeUInt16 uiMaxResolution = g_CVarMaxTextureResolution;

		Loader.UploadIntoOpenGLTexture (uiOpenGLTextureID, uiWidth, uiHeight, uiDepth, uiQuality, uiMaxResolution);

		aeRenderAPI::CheckErrors ("DDSLoader: After UploadIntoOpenGLTexture");

		uiWidth = Loader.getWidth ();
		uiHeight= Loader.getHeight ();
		uiDepth = 1;

		switch (Loader.getTextureType ())
		{
		case AE_DDS_TEXTURE2D:
			uiOpenGLTextureType = GL_TEXTURE_2D;
			break;
		case AE_DDS_TEXTURE3D:
			uiOpenGLTextureType = GL_TEXTURE_3D;
			break;
		case AE_DDS_CUBEMAP:
			uiOpenGLTextureType = GL_TEXTURE_CUBE_MAP;
			break;
		}

		return (true);
	}

	bool TGALoaderOGL (const char* szFile, aeUInt32 uiOpenGLTextureID, aeUInt32& uiOpenGLTextureType, aeUInt32& uiWidth, aeUInt32& uiHeight, aeUInt32& uiDepth)
	{
		aeTGALoader Loader;

    bool bRGBE = (aeStringFunctions::FindFirstStringPos (szFile, ".rgbe") != -1);

		if (bRGBE)
			aeLog::Dev ("Loading an RGBE Texture.");
		//else
		//	aeLog::Warning ("Loading a TGA instead of DDS: \"%s\"", szFile, 0);

		if ((aeStringFunctions::FindFirstStringPos (szFile, "%d") != -1) ||
			(aeStringFunctions::FindFirstStringPos (szFile, "%02d") != -1) ||
			(aeStringFunctions::FindFirstStringPos (szFile, "%03d") != -1))
		{
			aeLog::Dev ("Loading an Array Texture \"%s\"", szFile, 0);
			aeUInt32 uiLayers = 0;

			for (int i = 1; i < 1024; ++i)
			{
				char szNewPath[256] = "";
				aeStringFunctions::Format (szNewPath, 256, szFile, i);

        if (!aeFileSystem::ExistsFile (szNewPath))
					break;

				++uiLayers;
			}
			
			if (uiLayers == 0)
			{
				aeLog::Error ("Load TGA Array Texture: There are no layers available.");
				return (false);
			}

			aeLog::Dev ("Array Texture has %d Layers", uiLayers);

			char szNewPath[256] = "";
			aeUInt32 uiZero = 1;
			aeStringFunctions::Format (szNewPath, 256, szFile, uiZero);

			if (!Loader.LoadTGA (szNewPath))
			{
				aeLog::Error ("Error loading first layer");
				return (false);
			}

			uiWidth = Loader.getWidth ();
			uiHeight= Loader.getHeight ();
			uiDepth = uiLayers;

			uiOpenGLTextureType = GL_TEXTURE_2D_ARRAY;

			glBindTexture (uiOpenGLTextureType, uiOpenGLTextureID);

			if (bRGBE)
			{
				glTexImage3D (uiOpenGLTextureType, 0, GL_RGB9_E5, uiWidth, uiHeight, uiDepth, 0, GL_RGB, GL_UNSIGNED_INT_5_9_9_9_REV, NULL);
			}
			else
			{
				glTexImage3D (uiOpenGLTextureType, 0, GL_RGBA8, uiWidth, uiHeight, uiDepth, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
			}

			glTexParameteri (uiOpenGLTextureType, GL_TEXTURE_MAX_LEVEL, 0);

			for (aeUInt32 i = 1; i < uiLayers; ++i)
			{
				char szNewPath[256] = "";
				aeStringFunctions::Format (szNewPath, 256, szFile, i);

				if (!Loader.LoadTGA (szNewPath))
					return (false);

				if (bRGBE)
				{
					glTexSubImage3D (uiOpenGLTextureType, 0, 0, 0, i, uiWidth, uiHeight, 1, GL_RGB, GL_UNSIGNED_INT_5_9_9_9_REV, Loader.getDataForOpenGL ());
				}
				else
				{
					glTexSubImage3D (uiOpenGLTextureType, 0, 0, 0, i, uiWidth, uiHeight, 1, GL_BGRA, GL_UNSIGNED_BYTE, Loader.getDataForOpenGL ());
				}
			}
		}
		else
		{
			if (!Loader.LoadTGA (szFile))
				return (false);

			uiWidth = Loader.getWidth ();
			uiHeight= Loader.getHeight ();
			uiDepth = 1;

			uiOpenGLTextureType = GL_TEXTURE_2D;

			glBindTexture (uiOpenGLTextureType, uiOpenGLTextureID);
			glTexParameteri (uiOpenGLTextureType, GL_TEXTURE_MAX_LEVEL, 0);

			if (bRGBE)
			{
				glTexImage2D (uiOpenGLTextureType, 0, GL_RGB9_E5, Loader.getWidth (), Loader.getHeight (), 0, GL_RGB, GL_UNSIGNED_INT_5_9_9_9_REV, Loader.getDataForOpenGL  ());
			}
			else
			{
				glPixelStorei (GL_PACK_ALIGNMENT, 1);
				glPixelStorei (GL_UNPACK_ALIGNMENT, 1);

				//aeLog::Warning ("Compressing on the fly.");

				//aeInt32 iSize = 0;
				//vector<aeUInt8> Temp (Loader.getWidth () * Loader.getHeight () * 4);
				//CompressImageDXT5 (Loader.getDataForOpenGL  (), &Temp[0], Loader.getWidth (), Loader.getHeight (), iSize);

				//glCompressedTexImage2D (uiOpenGLTextureType, 0, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, Loader.getWidth (), Loader.getHeight (), 0, (GLsizei) iSize, &Temp[0]);

        aeRenderAPI::CheckErrors ("Upload");
        aeLog::Dev ("Uploading Texture ID = %i (%i * %i) Data = %p", uiOpenGLTextureID, Loader.getWidth (), Loader.getHeight (), Loader.getDataForOpenGL  ());
				glTexImage2D (uiOpenGLTextureType, 0, GL_RGBA8, Loader.getWidth (), Loader.getHeight (), 0, GL_BGRA, GL_UNSIGNED_BYTE, Loader.getDataForOpenGL  ());
        aeRenderAPI::CheckErrors ("Upload 2");
			}
		}

		return (true);
	}
#endif
}



