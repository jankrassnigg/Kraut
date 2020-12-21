#include "ShaderManager.h"
#include <KrautGraphics/TextureManager/Main.h>
#include <KrautGraphics/RenderAPI/Declarations.h>



namespace AE_NS_GRAPHICS
{
	void aeShaderManager::BindTexture (const char* szUniform, const aeTextureResourceHandle& hTexture, aeInt32 iTextureSamplerID)
	{
		++aeShaderManImpl::m_uiLastBoundTextureChange;

		aeBindTexture bt;
		bt.m_hTexture = hTexture;
		bt.m_iSampler = iTextureSamplerID;

		aeShaderManImpl::m_TexturesToBind[szUniform] = bt;
	}

	void aeShaderManager::BindUsedTextures (void)
	{
#ifdef AE_RENDERAPI_OPENGL
		if (g_RenderAPI == AE_RA_OPENGL)
		{
			AE_LOG_BLOCK ("BindUsedTextures");

			for (aeUInt32 ui = 0; ui < 16; ++ui)
			{
				if (aeShaderManImpl::m_pCurrentProgram->m_UsedTextures[ui] != aeShaderManImpl::m_TexturesToBind.end ())
				{
					aeTextureManager::BindTexture (ui, aeShaderManImpl::m_pCurrentProgram->m_UsedTextures[ui]->second.m_hTexture,
						aeShaderManImpl::m_pCurrentProgram->m_UsedTextures[ui]->second.m_iSampler);
				}
				else
					aeTextureManager::BindTexture (ui, aeTextureResourceHandle ());
			}
		}
#endif
	}
}

