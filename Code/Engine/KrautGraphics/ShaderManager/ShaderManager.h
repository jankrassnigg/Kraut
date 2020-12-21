// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#ifndef AE_GRAPHICS_SHADERMANAGER_SHADERMANAGER_H
#define AE_GRAPHICS_SHADERMANAGER_SHADERMANAGER_H

#include "Declarations.h"
#include "ShaderResource.h"
#include "ShaderResource.h"
#include "Uniforms.h"
#include <map>
#include "ConstantBuffers.h"

namespace AE_NS_GRAPHICS
{
	/*! The ShaderManager manages all important aspects of shaders. You can set "ShaderBuilder-Variables", which affect how shaders are combined from
		text-fragments. You can set uniforms through this manager and they will be applied to every shader, when needed.
		Shaders are loaded through the ResourceManager. Before loading any shaders, the function aeSHADERMANAGER::ProgramStartup needs to be called,
		to register the ShaderResource to the ResourceManager. After that all files with the extension ".shader" can be loaded through the ResourceManager,
		and bound to this ShaderManager.
	*/
	class AE_GRAPHICS_DLL aeShaderManager
	{
	public:

		static void BindTexture (const char* szUniform, const aeTextureResourceHandle& hTexture, aeInt32 iTextureSamplerID = -1);

		//! Sets a Variable used for building shader-combinations. Use $if (szName), $ifnot (szName), $elseif (szName) or $elseifnot (szName) in a Shader to build combinations.
		static void setShaderBuilderVariable (const char* szName, bool bFlag, bool bSetAsDefaultValue = true);
		//! Sets a Variable used for building shader-combinations. Use $val (szName) in a Shader to insert the value as text in the code.
		static void setShaderBuilderVariable (const char* szName, aeInt32 iValue, bool bSetAsDefaultValue = true);
		//! Sets a Variable used for building shader-combinations. Use $val (szName) in a Shader to insert the value as text in the code.
		static void setShaderBuilderVariable (const char* szName, float fValue, bool bSetAsDefaultValue = true);
		//! Sets a Variable used for building shader-combinations. Use $val (szName) in a Shader to insert the value as text in the code.
		static void setShaderBuilderVariable (const char* szName, const char* szReplacement, bool bSetAsDefaultValue = true);

		//! Sets all ShaderBuilderVariables to the last value, that was specified as "default".
		static void setShaderBuilderVariablesToDefault (void);

		//! Sets the ShaderResource to be used. If any of the bools is false, the ShaderResource previously bound for that stage will continue to be used there.
		static void setShader (aeShaderResourceHandle hShader, bool bVertex = true, bool bFragment = true, bool bGeometry = true);

		//! Shortcut of setUniformFloat for colors (unsigned bytes).
		static void setUniformUB (const aeStaticString<64>& sUniform, aeUInt8 f1, aeUInt8 f2, aeUInt8 f3, aeUInt8 f4 = 255);

		//! Sets a uniform float/vec2/vec3/vec4 (depending on the component-count) for all shaders.
		static void setUniformFloat		(const aeStaticString<64>& sUniform, aeUInt32 uiComponentCount, float f1, float f2 = 0.0f, float f3 = 0.0f, float f4 = 0.0f);
		//! Sets a uniform array of float/vec2/vec3/vec4 (depending on the component-count) for all shaders.
		static void setUniformFloatArray	(const aeStaticString<64>& sUniform, aeUInt32 uiComponentCount, float* pElements, aeUInt32 uiArraySize);

		//! Sets a uniform int/ivec2/ivec3/ivec4 (depending on the component-count) for all shaders.
		static void setUniformInt			(const aeStaticString<64>& sUniform, aeUInt32 uiComponentCount, aeInt32 i1, aeInt32 i2 = 0, aeInt32 i3 = 0, aeInt32 i4 = 0);
		//! Sets a uniform array of int/ivec2/ivec3/ivec4 (depending on the component-count) for all shaders.
		static void setUniformIntArray	(const aeStaticString<64>& sUniform, aeUInt32 uiComponentCount, aeInt32* pElements, aeUInt32 uiArraySize);

		//! Sets a uniform mat3, if uiArraySize == 0, or an array of mat3's, if uiArraySize > 0 (including arrays of size 1). If bTranspose is true, the matrices will be transpose.
		static void setUniformMatrix3x3	(const aeStaticString<64>& sUniform, bool bTranspose, const float* fMatrix, aeUInt32 uiArraySize = 0);
		//! Sets a uniform mat4, if uiArraySize == 0, or an array of mat4's, if uiArraySize > 0 (including arrays of size 1). If bTranspose is true, the matrices will be transpose.
		static void setUniformMatrix4x4	(const aeStaticString<64>& sUniform, bool bTranspose, const float* fMatrix, aeUInt32 uiArraySize = 0);

		//! For vertex-arrays, that need to know at which attribute-array to bind a data-stream. Not compatible with the immediate-mode shader (will throw an exception, if used together).
		static aeInt32 getAttributeBindPoint (const char* szAttributeName);

		//! Call this, before any kind of rendering. It will build shader-combinations as needed and set all uniforms in the shader to their current values. Can be called multiple times, without affecting performance.
		static void PrepareForRendering (void);

		//! Returns the vendor of the GPU, if detectable.
    static AE_GPU_VENDOR::Enum getGPUVendor (void);

	private:
		friend class aeShaderManagerStartup;

		//! Call this before using the shader-manager the first time
		static void ProgramStartup (void);
		//! Call this before shutting down, to release all currently bound shaders and to unload uniforms, etc.
		static void ProgramShutdown (void);

	private:

		static void UnloadProgramsUsingShader (const aeShaderTypeData& ShaderID);


	private:
		friend class aeShaderObject;

#ifdef AE_RENDERAPI_OPENGL
		static void ReflectShaderForConstantBuffersOGL (aeShaderTypeData& Shader);
		static void BindShaderProgramOGL (void);
		static void UploadConstantBuffersOGL (void);

#endif

		static void setImmediateModeShader (void);

		
		static void setUsedUniforms (void);


		static void BindUsedTextures (void);

	};

//#ifdef AE_BUILDING_SHADERMANAGER_DLL

	struct aeShaderManImpl
	{
	private:
		friend class aeShaderManager;
		friend class aeShaderObject;
		
		static bool m_bVertexShaderChanged;
		static bool m_bFragmentShaderChanged;
		static bool m_bGeometryShaderChanged;
		static bool m_bRelinkShaders;
		static bool m_bImmediateModeShader;
		
		static aeShaderResourceHandle m_hCurrentVertexShader;
		static aeShaderResourceHandle m_hCurrentFragmentShader;
		static aeShaderResourceHandle m_hCurrentGeometryShader;

		static aeShaderCombi m_ShaderIDs;

#ifdef AE_RENDERAPI_OPENGL
		struct aeUniformTuple
		{
			aeUniformTuple () : m_uiLastUpdate (0) {}

			aeInt32 m_iBindPoint;
			aeUniform* m_pUniform;
			aeUInt32 m_uiLastUpdate;
		};

		struct aeShaderProgram
		{
			aeUInt32 m_uiProgramID;
      std::vector<aeUniformTuple> m_Uniforms;

      std::map<aeStaticString<32>, aeBindTexture>::iterator m_UsedTextures[16];
		};

		static aeUInt32 m_uiCurrentProgramID;
		static std::map<aeStaticString<64>, aeUniform> m_Uniforms;
		static std::map<aeShaderCombi, aeShaderProgram> m_ShaderPrograms;
		static aeShaderProgram* m_pCurrentProgram;

		static void FindUsedUniforms (aeShaderProgram* pShaderProgram);
#endif

		static aeUInt32 m_uiConstantBufferUpdate;
		static aeUInt32 m_uiBoundTextureUpdate;
		static aeUInt32 m_uiLastBoundTextureChange;

		static std::map<aeStaticString<32>, aeConstantBuffer> m_ConstantBuffers;
		static std::map<aeStaticString<64>, aeShaderVariable> m_ShaderVariables;

		static std::map<aeStaticString<32>, aeBindTexture> m_TexturesToBind;

    static AE_GPU_VENDOR::Enum m_GPUVendor;
	};

//#endif

}

#endif


