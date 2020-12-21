#include "TextureManager.h"


namespace AE_NS_GRAPHICS
{
	void aeTextureManager::setDefaultTextureFilter (AE_TEXTURE_FILTER DefaultFilter)
	{
		aeTexManImpl::m_DefaultFilter = aeMath::Clamp (DefaultFilter, AE_FILTER_NEAREST, AE_FILTER_ANISOTROPIC_16X);

		// recreate all samplers, when the default texture filter is changed
		for (aeUInt32 uiFilter = 0; uiFilter < aeTexManImpl::m_TextureSamplers.size (); ++uiFilter)
			CreateTextureSampler (uiFilter);
	}

	void aeTextureManager::setTextureSampler (const char* szName, const aeSamplerState& State)
	{
		aeUInt32 uiSamplerID;

		// if not yet existing, create it
		if (aeTexManImpl::m_SamplerIDs.find (szName) == aeTexManImpl::m_SamplerIDs.end ())
		{
			uiSamplerID = (aeUInt32) aeTexManImpl::m_TextureSamplers.size ();
			aeTexManImpl::m_SamplerIDs[szName] = uiSamplerID;
			aeTexManImpl::m_TextureSamplers.push_back (State);

			aeLog::Log ("aeTextureManager::setTextureSampler: Create \"%s\"", szName);
		}
		else // otherwise overwrite it
		{
			uiSamplerID = aeTexManImpl::m_SamplerIDs[szName];
			aeTexManImpl::m_TextureSamplers[uiSamplerID] = State;

			aeLog::Log ("aeTextureManager::setTextureSampler: Update \"%s\"", szName);
		}

		CreateTextureSampler (uiSamplerID);
	}

	aeUInt32 aeTextureManager::getTextureSamplerID (const char* szName)
	{
    std::map<aeStaticString<24>, aeUInt16>::const_iterator it = aeTexManImpl::m_SamplerIDs.find (szName);

		if (it == aeTexManImpl::m_SamplerIDs.end ())
			AE_CHECK_ALWAYS (false, "aeTextureManager::getTextureSamplerID: Texture Sampler \"%s\" does not exist (yet).", szName);

		return (it->second);
	}

	void aeTextureManager::BindSampler (aeUInt32 uiTextureUnit, aeUInt32 uiSamplerID, aeTextureResource* pTexture, bool bVertexShader)
	{
		aeTexManImpl::m_BoundSamplers[uiTextureUnit] = uiSamplerID;

#ifdef AE_RENDERAPI_OPENGL
		if (g_RenderAPI == AE_RA_OPENGL)
		{
			aeSamplerState state = aeTexManImpl::m_TextureSamplers[uiSamplerID];

			MakeTextureUnitCurrent (uiTextureUnit);

			//if (pTexture->m_TextureType == GL_TEXTURE_CUBE_MAP)
			//	aeLog::Log ("Binding cubemap.");

			//glTexParameteri (pTexture->m_TextureType, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			//glTexParameteri (pTexture->m_TextureType, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			//glTexParameteri (pTexture->m_TextureType, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

			glTexParameteri (pTexture->m_TextureTypeOGL, GL_TEXTURE_WRAP_S, state.m_TextureWrapU == AE_WRAP_CLAMP ? GL_CLAMP_TO_EDGE : GL_REPEAT);
			if ((pTexture->m_TextureTypeOGL != GL_TEXTURE_1D)  && (pTexture->m_TextureTypeOGL != GL_TEXTURE_1D_ARRAY))
				glTexParameteri (pTexture->m_TextureTypeOGL, GL_TEXTURE_WRAP_T, state.m_TextureWrapV == AE_WRAP_CLAMP ? GL_CLAMP_TO_EDGE : GL_REPEAT);
			if (pTexture->m_TextureTypeOGL == GL_TEXTURE_3D)
				glTexParameteri (pTexture->m_TextureTypeOGL, GL_TEXTURE_WRAP_R, state.m_TextureWrapW == AE_WRAP_CLAMP ? GL_CLAMP_TO_EDGE : GL_REPEAT);

			const bool bMipmapping = pTexture->m_bHasMipmaps;

			aeInt32 iFilter = state.m_TextureFilter;

			switch (state.m_TextureFilter)
			{
			case AE_FILTER_DEFAULT_MINUS_TWO:
				iFilter = aeTexManImpl::m_DefaultFilter - 2;
				break;
			case AE_FILTER_DEFAULT_MINUS_ONE:
				iFilter = aeTexManImpl::m_DefaultFilter - 1;
				break;
			case AE_FILTER_DEFAULT_PLUS_ONE:
				iFilter = aeTexManImpl::m_DefaultFilter + 1;
				break;
			case AE_FILTER_DEFAULT_PLUS_TWO:
				iFilter = aeTexManImpl::m_DefaultFilter + 2;
				break;
			default:
				break;
			}

			const AE_TEXTURE_FILTER Filter = (AE_TEXTURE_FILTER) aeMath::Clamp<aeInt32> (iFilter, AE_FILTER_NEAREST, AE_FILTER_ANISOTROPIC_16X);

			switch (Filter)
			{
			case AE_FILTER_NEAREST:
				glTexParameteri (pTexture->m_TextureTypeOGL, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameteri (pTexture->m_TextureTypeOGL, GL_TEXTURE_MIN_FILTER, bMipmapping ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST);
				glTexParameterf (pTexture->m_TextureTypeOGL, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0f);
				break;
			case AE_FILTER_BILINEAR:
				glTexParameteri (pTexture->m_TextureTypeOGL, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri (pTexture->m_TextureTypeOGL, GL_TEXTURE_MIN_FILTER, bMipmapping ? GL_LINEAR_MIPMAP_NEAREST : GL_LINEAR);
				glTexParameterf (pTexture->m_TextureTypeOGL, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0f);
				break;
			case AE_FILTER_TRILINEAR:
				glTexParameteri (pTexture->m_TextureTypeOGL, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri (pTexture->m_TextureTypeOGL, GL_TEXTURE_MIN_FILTER, bMipmapping ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
				glTexParameterf (pTexture->m_TextureTypeOGL, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0f);
				break;
			case AE_FILTER_ANISOTROPIC_2X:
				glTexParameteri (pTexture->m_TextureTypeOGL, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri (pTexture->m_TextureTypeOGL, GL_TEXTURE_MIN_FILTER, bMipmapping ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
				glTexParameterf (pTexture->m_TextureTypeOGL, GL_TEXTURE_MAX_ANISOTROPY_EXT, 2.0f);
				break;
			case AE_FILTER_ANISOTROPIC_4X:
				glTexParameteri (pTexture->m_TextureTypeOGL, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri (pTexture->m_TextureTypeOGL, GL_TEXTURE_MIN_FILTER, bMipmapping ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
				glTexParameterf (pTexture->m_TextureTypeOGL, GL_TEXTURE_MAX_ANISOTROPY_EXT, 4.0f);
				break;
			case AE_FILTER_ANISOTROPIC_8X:
				glTexParameteri (pTexture->m_TextureTypeOGL, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri (pTexture->m_TextureTypeOGL, GL_TEXTURE_MIN_FILTER, bMipmapping ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
				glTexParameterf (pTexture->m_TextureTypeOGL, GL_TEXTURE_MAX_ANISOTROPY_EXT, 8.0f);
				break;
			case AE_FILTER_ANISOTROPIC_16X:
				glTexParameteri (pTexture->m_TextureTypeOGL, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri (pTexture->m_TextureTypeOGL, GL_TEXTURE_MIN_FILTER, bMipmapping ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
				glTexParameterf (pTexture->m_TextureTypeOGL, GL_TEXTURE_MAX_ANISOTROPY_EXT, 16.0f);
				break;
			}
		}
#endif

	}

	void aeTextureManager::DestroyTextureSamplers (void)
	{
	}


	void aeTextureManager::CreateTextureSampler (aeUInt32 uiSamplerID)
	{
	}
}

