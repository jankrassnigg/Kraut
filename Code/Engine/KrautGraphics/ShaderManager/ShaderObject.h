// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#ifndef AE_GRAPHICS_SHADERMANAGER_SHADEROBJECT_H
#define AE_GRAPHICS_SHADERMANAGER_SHADEROBJECT_H

#include "Declarations.h"
#include "Condition.h"
#include <KrautFoundation/ResourceManager/ResourceHandle.h>
#include <KrautFoundation/Containers/Set.h>
#include <KrautFoundation/Containers/Map.h>

namespace AE_NS_GRAPHICS
{
	class AE_GRAPHICS_DLL aeShaderObject
	{
	public:
		aeShaderObject ();

		void setShaderCode (const char* szSource);

		aeShaderTypeData* getShaderID (AE_SHADER_TYPE ShaderType, aeResourceHandle hShader);

		void UnloadAllShaders (void);

	private:
		aeString BuildShaderCode (void);
		

		aeCondition m_RootCondition;
    aeSet<aeStaticString<32>> m_UsedVariables;

#ifdef AE_RENDERAPI_OPENGL
		aeShaderTypeData CreateShaderOGL (AE_SHADER_TYPE ShaderType, aeResourceHandle hShader);
#endif

    aeMap<aeString, aeShaderTypeData> m_ShaderCombos;
	};



}

#endif


