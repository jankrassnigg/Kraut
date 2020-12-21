// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#include "TextureManager.h"
#include <KrautFoundation/Math/IncludeAll.h>
#include <KrautFoundation/Configuration/CVar.h>



namespace AE_NS_GRAPHICS
{
	void Cvars_OnChange (void);

	aeCVarInt g_CVarTextureQuality ("r_TextureQuality", 0, aeCVarFlags::Save | aeCVarFlags::Restart, "How many high-res mipmap levels to drop.");//, Cvars_OnChange);
	aeCVarInt g_CVarMaxTextureResolution ("r_MaxTextureResolution", 2048, aeCVarFlags::Save | aeCVarFlags::Restart, "The upper limit for texture resolutions.");//, Cvars_OnChange);

	void Cvars_OnChange (void)
	{
		//*g_CVarTextureQuality = aeMath::Clamp<aeUInt8> (*g_CVarTextureQuality, 0, 5);
		//*g_CVarMaxTextureResolution = aeMath::Clamp<aeUInt16> (*g_CVarMaxTextureResolution, 64, 8192);
	}

	aeUInt8 aeTextureManager::getNumberOfTextureUnits (void)
	{
		return (aeUInt8 (aeTexManImpl::m_BoundTextures.size ()));
	}

	void aeTextureManager::MakeTextureUnitCurrent (aeUInt8 uiUnit)
	{
#ifdef AE_RENDERAPI_OPENGL
		if (g_RenderAPI == AE_RA_OPENGL)
		{
			glActiveTexture (GL_TEXTURE0 + uiUnit);
		}
#endif
	}


	void aeTextureManager::BindTexture (aeUInt8 uiUnit, aeTextureResourceHandle& hTexture, aeInt32 iTextureSamplerID, bool bVertexShader)
	{
		AE_CHECK_DEV (uiUnit < getNumberOfTextureUnits (), "aeTextureManager::BindTexture: Cannot bind Texture to unit %d, there are only %d units.", uiUnit, getNumberOfTextureUnits ());

		if (aeTexManImpl::m_BoundTextures[uiUnit] == hTexture)
		{
			if (!hTexture.IsValid ())
				return;

			aeInt32 iSampler = iTextureSamplerID;
			if (iSampler == -1)
				iSampler = hTexture.GetResource ()->m_uiDefaultTextureSamplerID;

			if (iSampler == aeTexManImpl::m_BoundSamplers[uiUnit])
				return;
		}

		++aeTexManImpl::m_uiTextureSwitches;

		aeTexManImpl::m_BoundTextures[uiUnit] = hTexture;

		aeTextureResource* pRes = NULL;
		
		if (hTexture.IsValid ())
			pRes = hTexture.GetResource ();

#ifdef AE_RENDERAPI_OPENGL
		if (g_RenderAPI == AE_RA_OPENGL)
		{
			MakeTextureUnitCurrent (uiUnit);

			if (pRes)
				glBindTexture (pRes->getOpenGLTextureType (), pRes->getOpenGLID ());
			else
			{
				glBindTexture (mapTextureTypeOGL (AE_TYPE_TEXTURE_2D), 0);
				glBindTexture (mapTextureTypeOGL (AE_TYPE_TEXTURE_2D_ARRAY), 0);
				glBindTexture (mapTextureTypeOGL (AE_TYPE_TEXTURE_3D), 0);
				glBindTexture (mapTextureTypeOGL (AE_TYPE_TEXTURE_CUBE), 0);
				//glBindTexture (mapTextureTypeOGL (AE_TYPE_TEXTURE_CUBE_ARRAY), 0);
				//glBindTexture (mapTextureTypeOGL (AE_TYPE_TEXTURE_RECTANGLE), 0);
			}
		}
#endif

		if (pRes)
		{
			if (iTextureSamplerID < 0)
				BindSampler (uiUnit, pRes->m_uiDefaultTextureSamplerID, pRes, bVertexShader);
			else
				BindSampler (uiUnit, iTextureSamplerID, pRes, bVertexShader);
		}
	}

	aeUInt32 aeTextureManager::getAndResetNumberOfTextureSwitches (void)
	{
		aeUInt32 uiTemp = aeTexManImpl::m_uiTextureSwitches;
		aeTexManImpl::m_uiTextureSwitches = 0;
		return (uiTemp);
	}

#ifdef AE_RENDERAPI_OPENGL
	void aeTextureManager::RegisterFileLoader (const char* szFileExtension, AE_TEXTURE_FILE_LOADER_OGL pTextureLoader)
	{
		aeStaticString<16> sExt = szFileExtension;
    sExt.ToUpperCase ();

		aeTexManImpl::m_FileLoadersOGL[sExt] = pTextureLoader;
	}

	AE_TEXTURE_FILE_LOADER_OGL aeTextureManager::getFileLoaderOGL (const char* szFileExtension)
	{
		aeStaticString<16> sExt = szFileExtension;
    sExt.ToUpperCase ();

		//! \todo Wenn extension nicht bekannt: error ausgeben, und default loader (leere Textur) zurueckgeben.

		AE_CHECK_DEV (aeTexManImpl::m_FileLoadersOGL[sExt] != NULL, "aeTextureManager: There is no Loader for \"%s\"-files.", sExt.c_str ());

		return (aeTexManImpl::m_FileLoadersOGL[sExt]);
	}
#endif
}

