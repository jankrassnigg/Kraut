// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#include "Main.h"
#include <KrautGraphics/RenderAPI/Declarations.h>



namespace AE_NS_GRAPHICS
{
	void aeShaderManager::ProgramStartup (void)
	{
		getGPUVendor ();
	
#ifdef AE_RENDERAPI_OPENGL
		if (g_RenderAPI == AE_RA_OPENGL)
		{
			aeLog::Log ("RenderAPI is OpenGL -> defining LANGUAGE_GLSL.");

			aeShaderManager::setShaderBuilderVariable ("LANGUAGE_GLSL", true, true);
			aeShaderManager::setShaderBuilderVariable ("LANGUAGE_HLSL", false, true);
		}
#endif
	}

}



