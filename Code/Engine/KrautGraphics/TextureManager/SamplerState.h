#ifndef AE_GRAPHICS_TEXTUREMANAGER_SAMPLERSTATE_H
#define AE_GRAPHICS_TEXTUREMANAGER_SAMPLERSTATE_H

#include "Declarations.h"

namespace AE_NS_GRAPHICS
{
	struct aeSamplerState
	{
		aeSamplerState ()
		{
			m_TextureFilter = AE_FILTER_DEFAULT;
			m_TextureWrapU = AE_WRAP_REPEAT;
			m_TextureWrapV = AE_WRAP_REPEAT;
			m_TextureWrapW = AE_WRAP_REPEAT;
			//m_TextureCompare = AE_COMPARISON_DISABLED;
		}

		AE_TEXTURE_FILTER m_TextureFilter;
		AE_TEXTURE_WRAP m_TextureWrapU;
		AE_TEXTURE_WRAP m_TextureWrapV;
		AE_TEXTURE_WRAP m_TextureWrapW;
		//AE_TEXTURE_COMPARE m_TextureCompare;
	};

}

#endif

