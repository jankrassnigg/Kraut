// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#ifndef AE_GRAPHICS_TEXTUREMANAGER_TEXTUREMANAGER_H
#define AE_GRAPHICS_TEXTUREMANAGER_TEXTUREMANAGER_H

#include "Declarations.h"
#include <map>
#include <vector>
#include "TextureResource.h"
#include "SamplerState.h"

namespace AE_NS_GRAPHICS
{
  class AE_GRAPHICS_DLL aeTextureManager
	{
	public:

		//! Returns the number of texture units available.
		static aeUInt8 getNumberOfTextureUnits (void);
		//! Returns the number of texture switches since the last call to this function.
		static aeUInt32 getAndResetNumberOfTextureSwitches (void);

		//! Binds a texture to the given unit.
		static void BindTexture (aeUInt8 uiUnit, aeTextureResourceHandle& hTexture, aeInt32 iTextureSamplerID = -1, bool bVertexShader = false);
		//! Sets the given texture unit as current, for other OpenGL commands to execute on it.
		static void MakeTextureUnitCurrent (aeUInt8 uiUnit);

#ifdef AE_RENDERAPI_OPENGL
		//! Registers a loader-function for files with a certain file-extension.
		static void RegisterFileLoader (const char* szFileExtension, AE_TEXTURE_FILE_LOADER_OGL pTextureLoader);
#endif

    //! Sets the state of the texture-sampler with the given name. Overwrites it, if it already exists.
		static void setTextureSampler (const char* szName, const aeSamplerState& State);
		//! Returns the ID of the texture sampler with the given ID, throws an exception, if it does not exist.
		static aeUInt32 getTextureSamplerID (const char* szName);

		//! Sets the default filter to use on textures.
		static void setDefaultTextureFilter (AE_TEXTURE_FILTER DefaultFilter);

	private:
		friend class aeTextureManagerStartup;
		friend class aeTextureResource;

		static void CoreStartup (void);
		static void EngineStartup (void);
		static void EngineShutdown (void);

#ifdef AE_RENDERAPI_OPENGL
		//! Returns a loader-function for files with a certain file-extension.
		static AE_TEXTURE_FILE_LOADER_OGL getFileLoaderOGL (const char* szFileExtension);
#endif

		static void BindSampler (aeUInt32 uiTextureUnit, aeUInt32 uiSamplerID, aeTextureResource* pTexture, bool bVertexShader);
		static void CreateTextureSampler (aeUInt32 uiSamplerID);
		static void DestroyTextureSamplers (void);

	};

	struct aeTexManImpl
	{
	private:
		friend class aeTextureManager;

		static aeUInt32 m_uiTextureSwitches;

		static std::vector<aeTextureResourceHandle> m_BoundTextures;
		static std::vector<aeInt32> m_BoundSamplers;

#ifdef AE_RENDERAPI_OPENGL
		static std::map<aeStaticString<16>, AE_TEXTURE_FILE_LOADER_OGL> m_FileLoadersOGL;
#endif

    static std::map<aeStaticString<24>, aeUInt16> m_SamplerIDs;
		static std::vector<aeSamplerState> m_TextureSamplers;

		static AE_TEXTURE_FILTER m_DefaultFilter;
	};

}

#endif



