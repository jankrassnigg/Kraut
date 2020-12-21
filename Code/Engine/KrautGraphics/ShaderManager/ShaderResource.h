// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#ifndef AE_GRAPHICS_SHADERMANAGER_SHADERRESOURCE_H
#define AE_GRAPHICS_SHADERMANAGER_SHADERRESOURCE_H

#include "Declarations.h"
#include "ShaderObject.h"

namespace AE_NS_GRAPHICS
{
  using namespace AE_NS_GRAPHICS;

	class AE_GRAPHICS_DLL aeShaderResource : public aeResource
	{
    AE_RESOURCE_DECLARATION (aeShaderResource)

    aeShaderResource () {} 

	public:
		static aeStaticString<128>& getCurrentlyLoadedShader (void) {return (s_CurrentlyLoadedShader);}

		aeShaderTypeData* getVertexShaderID (void);
		aeShaderTypeData* getFragmentShaderID (void);
		aeShaderTypeData* getGeometryShaderID (void);


	private:
		virtual void LoadResourceFromFile (const char* szFilename);

		virtual void UnloadResource (void);

		static aeStaticString<128> s_CurrentlyLoadedShader;


	private:
		bool m_bHasVertexShader;
		bool m_bHasFragmentShader;
		bool m_bHasGeometryShader;

		aeShaderObject m_VertexShader;
		aeShaderObject m_FragmentShader;
		aeShaderObject m_GeometryShader;
	};
}

#endif


