// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#include "ShaderManager.h"
#include <KrautFoundation/ResourceManager/ResourceManager.h>
#include <KrautGraphics/RenderAPI/Main.h>



namespace AE_NS_GRAPHICS
{
	void LogErrorString (const char* szString);

	bool aeShaderManImpl::m_bVertexShaderChanged = true;
	bool aeShaderManImpl::m_bFragmentShaderChanged = true;
	bool aeShaderManImpl::m_bGeometryShaderChanged = true;
	bool aeShaderManImpl::m_bRelinkShaders = true;
	bool aeShaderManImpl::m_bImmediateModeShader = true;
	
	aeShaderResourceHandle aeShaderManImpl::m_hCurrentVertexShader;
	aeShaderResourceHandle aeShaderManImpl::m_hCurrentFragmentShader;
	aeShaderResourceHandle aeShaderManImpl::m_hCurrentGeometryShader;

	aeShaderCombi aeShaderManImpl::m_ShaderIDs;

	aeUInt32 aeShaderManImpl::m_uiConstantBufferUpdate = 0;
	aeUInt32 aeShaderManImpl::m_uiBoundTextureUpdate = 0;
	aeUInt32 aeShaderManImpl::m_uiLastBoundTextureChange = 0;

  std::map<aeStaticString<32>, aeConstantBuffer> aeShaderManImpl::m_ConstantBuffers;
  std::map<aeStaticString<64>, aeShaderVariable> aeShaderManImpl::m_ShaderVariables;

  std::map<aeStaticString<32>, aeBindTexture> aeShaderManImpl::m_TexturesToBind;

  AE_GPU_VENDOR::Enum aeShaderManImpl::m_GPUVendor = AE_GPU_VENDOR::AE_VENDOR_UNCHECKED;

#ifdef AE_RENDERAPI_OPENGL
	aeUInt32 aeShaderManImpl::m_uiCurrentProgramID = 0;
  std::map<aeStaticString<64>, aeUniform> aeShaderManImpl::m_Uniforms;
  std::map<aeShaderCombi, aeShaderManImpl::aeShaderProgram> aeShaderManImpl::m_ShaderPrograms;
	aeShaderManImpl::aeShaderProgram* aeShaderManImpl::m_pCurrentProgram = NULL;
#endif

	void aeShaderManager::ProgramShutdown (void)
	{
    aeShaderManImpl::m_TexturesToBind.clear ();
    aeShaderManImpl::m_ConstantBuffers.clear ();
    aeShaderManImpl::m_ShaderVariables.clear ();

    aeShaderManImpl::m_bVertexShaderChanged = true;
    aeShaderManImpl::m_bFragmentShaderChanged = true;
    aeShaderManImpl::m_bGeometryShaderChanged = true;
    aeShaderManImpl::m_bRelinkShaders = true;
    aeShaderManImpl::m_bImmediateModeShader = true;

#ifdef AE_RENDERAPI_OPENGL
		if (g_RenderAPI == AE_RA_OPENGL)
		{
			// disable shaders
			glUseProgram (0);

      std::map<aeShaderCombi, aeShaderManImpl::aeShaderProgram>::iterator it = aeShaderManImpl::m_ShaderPrograms.begin ();
			const std::map<aeShaderCombi, aeShaderManImpl::aeShaderProgram>::iterator itend = aeShaderManImpl::m_ShaderPrograms.end ();

			for (; it != itend; ++it)
				glDeleteProgram (it->second.m_uiProgramID);

			aeShaderManImpl::m_Uniforms.clear ();

			aeShaderManImpl::m_ShaderPrograms.clear ();
		}
#endif

		

		// delete the used shaders
		aeShaderManImpl::m_hCurrentVertexShader.Invalidate ();
		aeShaderManImpl::m_hCurrentFragmentShader.Invalidate ();
		aeShaderManImpl::m_hCurrentGeometryShader.Invalidate ();
	}

	void aeShaderManager::setShaderBuilderVariablesToDefault (void)
	{
		aeCondition::setShaderBuilderVarsToDefault ();
	}

	void aeShaderManager::setShaderBuilderVariable (const char* szName, bool bFlag, bool bSetAsDefaultValue)
	{
		aeCondition::setConditionVariable (szName, bFlag ? "1" : "", bSetAsDefaultValue);
	}

	void aeShaderManager::setShaderBuilderVariable (const char* szName, aeInt32 iValue, bool bSetAsDefaultValue)
	{
		char szTemp[16] = "";
		aeStringFunctions::Format (szTemp, 16, "%d", iValue);
		//itoa (iValue, szTemp, 10);
		aeCondition::setConditionVariable (szName, szTemp, bSetAsDefaultValue);
	}

	void aeShaderManager::setShaderBuilderVariable (const char* szName, float fValue, bool bSetAsDefaultValue)
	{
		char szTemp[16] = "";
		aeStringFunctions::Format (szTemp, 16, "%.6f", fValue);
		aeCondition::setConditionVariable (szName, szTemp, bSetAsDefaultValue);
	}

	void aeShaderManager::setShaderBuilderVariable (const char* szName, const char* szReplacement, bool bSetAsDefaultValue)
	{
		aeCondition::setConditionVariable (szName, szReplacement, bSetAsDefaultValue);
	}

	void aeShaderManager::setShader (aeShaderResourceHandle hShader, bool bVertex, bool bFragment, bool bGeometry)
	{
		aeShaderManImpl::m_bImmediateModeShader = false;

		if (bVertex)
		{
			if (aeShaderManImpl::m_hCurrentVertexShader != hShader)
			{
				aeShaderManImpl::m_hCurrentVertexShader = hShader;
				aeShaderManImpl::m_bVertexShaderChanged = true;
			}
		}

		if (bFragment)
		{
			if (aeShaderManImpl::m_hCurrentFragmentShader != hShader)
			{
				aeShaderManImpl::m_hCurrentFragmentShader = hShader;
				aeShaderManImpl::m_bFragmentShaderChanged = true;
			}
		}

		if (bGeometry)
		{
			if (aeShaderManImpl::m_hCurrentGeometryShader != hShader)
			{
				aeShaderManImpl::m_hCurrentGeometryShader = hShader;
				aeShaderManImpl::m_bGeometryShaderChanged = true;
			}
		}
	}

	void aeShaderManager::setImmediateModeShader (void)
	{
		if (aeShaderManImpl::m_bImmediateModeShader)
			return;

		aeShaderManImpl::m_bImmediateModeShader = true;
		aeShaderManImpl::m_bVertexShaderChanged = true;
		aeShaderManImpl::m_bFragmentShaderChanged = true;
		aeShaderManImpl::m_bGeometryShaderChanged = true;
		aeShaderManImpl::m_bRelinkShaders = true;
		
#ifdef AE_RENDERAPI_OPENGL
		if (g_RenderAPI == AE_RA_OPENGL)
		{
			aeShaderManImpl::m_uiCurrentProgramID = 0;
			aeShaderManImpl::m_pCurrentProgram = NULL;
			glUseProgram (0);
		}
#endif
	}

  AE_GPU_VENDOR::Enum aeShaderManager::getGPUVendor (void)
	{
    if (aeShaderManImpl::m_GPUVendor == AE_GPU_VENDOR::AE_VENDOR_UNCHECKED)
		{
			aeShaderManImpl::m_GPUVendor = AE_GPU_VENDOR::AE_VENDOR_UNKNOWN;

#ifdef AE_RENDERAPI_OPENGL
			if (g_RenderAPI == AE_RA_OPENGL)
			{
				const char* szStr = (const char*) glGetString (GL_VENDOR);

				aeString s = szStr;
        s.ToUpperCase ();
				if (aeStringFunctions::FindFirstStringPos (s.c_str (), "NVIDIA") != -1)
				{
					aeShaderManImpl::m_GPUVendor = AE_GPU_VENDOR::AE_VENDOR_NVIDIA;

					setShaderBuilderVariable ("VENDOR_NVIDIA", true, true);
					aeLog::Dev ("ShaderManager: Vendor is nVidia");
				}
				else
				if (aeStringFunctions::FindFirstStringPos (s.c_str (), "ATI ") != -1)
				{
					aeShaderManImpl::m_GPUVendor = AE_GPU_VENDOR::AE_VENDOR_ATI;

					setShaderBuilderVariable ("VENDOR_ATI", true, true);
					aeLog::Dev ("ShaderManager: Vendor is ATI");
				}
				else
					aeLog::Warning ("ShaderManager: Vendor is unknown");
			}
#endif
		}

		return (aeShaderManImpl::m_GPUVendor);
	}

	void aeShaderManager::PrepareForRendering (void)
	{
		if (aeShaderManImpl::m_bImmediateModeShader)
			return;

		bool bShaderBuilderVarsChanged = aeCondition::HaveConditionsChanged ();

		if ((aeShaderManImpl::m_bVertexShaderChanged) || (bShaderBuilderVarsChanged))
		{
			aeShaderManImpl::m_bVertexShaderChanged = false;
			aeShaderTypeData* uiNewID = aeShaderManImpl::m_hCurrentVertexShader.GetResource ()->getVertexShaderID ();

			if (aeShaderManImpl::m_ShaderIDs.m_pVertexShaderID != uiNewID)
			{
				aeShaderManImpl::m_ShaderIDs.m_pVertexShaderID = uiNewID;
				aeShaderManImpl::m_bRelinkShaders = true;
			}
		}

		if ((aeShaderManImpl::m_bFragmentShaderChanged) || (bShaderBuilderVarsChanged))
		{
			aeShaderManImpl::m_bFragmentShaderChanged = false;
			aeShaderTypeData* uiNewID = aeShaderManImpl::m_hCurrentFragmentShader.GetResource ()->getFragmentShaderID ();

			if (aeShaderManImpl::m_ShaderIDs.m_pFragmentShaderID != uiNewID)
			{
				aeShaderManImpl::m_ShaderIDs.m_pFragmentShaderID = uiNewID;
				aeShaderManImpl::m_bRelinkShaders = true;
			}
		}

		if ((aeShaderManImpl::m_bGeometryShaderChanged) || (bShaderBuilderVarsChanged))
		{
			aeShaderManImpl::m_bGeometryShaderChanged = false;
			aeShaderTypeData* uiNewID = aeShaderManImpl::m_hCurrentGeometryShader.GetResource ()->getGeometryShaderID ();

			if (aeShaderManImpl::m_ShaderIDs.m_pGeometryShaderID != uiNewID)
			{
				aeShaderManImpl::m_ShaderIDs.m_pGeometryShaderID = uiNewID;
				aeShaderManImpl::m_bRelinkShaders = true;
			}
		}

		if (aeShaderManImpl::m_bRelinkShaders)
		{
			aeShaderManImpl::m_uiConstantBufferUpdate = 0;
			aeShaderManImpl::m_uiBoundTextureUpdate = 0;
			aeShaderManImpl::m_bRelinkShaders = false;

#ifdef AE_RENDERAPI_OPENGL
			if (g_RenderAPI == AE_RA_OPENGL)
			{
				BindShaderProgramOGL ();
			}
#endif
		}

		if (aeShaderManImpl::m_uiConstantBufferUpdate < aeShaderVariable::s_uiVarChangeCounter)
		{
#ifdef AE_RENDERAPI_OPENGL
			if (g_RenderAPI == AE_RA_OPENGL)
			{
				UploadConstantBuffersOGL ();
			}
#endif

			aeShaderManImpl::m_uiConstantBufferUpdate = aeShaderVariable::s_uiVarChangeCounter;
		}

		if (aeShaderManImpl::m_uiBoundTextureUpdate < aeShaderManImpl::m_uiLastBoundTextureChange)
		{
			aeShaderManImpl::m_uiBoundTextureUpdate = aeShaderManImpl::m_uiLastBoundTextureChange;
			BindUsedTextures ();
		}

#ifdef AE_RENDERAPI_OPENGL
		if (g_RenderAPI == AE_RA_OPENGL)
		{
			setUsedUniforms ();
		}
#endif
	}

#ifdef AE_RENDERAPI_OPENGL

	GLenum getGSIinputOutputType (AE_GS_INOUT_TYPE type)
	{
		switch (type)
		{
		case AE_GS_IOT_POINTS:
			return (GL_POINTS);
		case AE_GS_IOT_LINES:
			return (GL_LINES);
		case AE_GS_IOT_TRIANGLES:
			return (GL_TRIANGLES);
		case AE_GS_IOT_LINE_STRIP:
			return (GL_LINE_STRIP);
		case AE_GS_IOT_TRIANGLE_STRIP:
			return (GL_TRIANGLE_STRIP);
		}

		return (GL_TRIANGLES);
	}

	void aeShaderManager::BindShaderProgramOGL (void)
	{
    std::map<aeShaderCombi, aeShaderManImpl::aeShaderProgram>::iterator it = aeShaderManImpl::m_ShaderPrograms.find (aeShaderManImpl::m_ShaderIDs);

		if (it != aeShaderManImpl::m_ShaderPrograms.end ())
		{
			aeShaderManImpl::m_uiCurrentProgramID = it->second.m_uiProgramID;
			glUseProgram (aeShaderManImpl::m_uiCurrentProgramID);

			aeShaderManImpl::m_pCurrentProgram = &it->second;
			return;
		}

		aeShaderManImpl::m_uiCurrentProgramID = glCreateProgram ();

		if (aeShaderManImpl::m_ShaderIDs.m_pVertexShaderID->m_ShaderOGL != aeShaderTypeData::INVALID_SHADER_OGL)
			glAttachShader (aeShaderManImpl::m_uiCurrentProgramID, aeShaderManImpl::m_ShaderIDs.m_pVertexShaderID->m_ShaderOGL);

		if (aeShaderManImpl::m_ShaderIDs.m_pFragmentShaderID->m_ShaderOGL != aeShaderTypeData::INVALID_SHADER_OGL)
			glAttachShader (aeShaderManImpl::m_uiCurrentProgramID, aeShaderManImpl::m_ShaderIDs.m_pFragmentShaderID->m_ShaderOGL);

		if (aeShaderManImpl::m_ShaderIDs.m_pGeometryShaderID->m_ShaderOGL != aeShaderTypeData::INVALID_SHADER_OGL)
		{
			glAttachShader (aeShaderManImpl::m_uiCurrentProgramID, aeShaderManImpl::m_ShaderIDs.m_pGeometryShaderID->m_ShaderOGL);

			glProgramParameteriARB (aeShaderManImpl::m_uiCurrentProgramID, GL_GEOMETRY_VERTICES_OUT_ARB, aeShaderManImpl::m_ShaderIDs.m_pGeometryShaderID->m_GSOutputVertices);
			glProgramParameteriARB (aeShaderManImpl::m_uiCurrentProgramID, GL_GEOMETRY_INPUT_TYPE_ARB, getGSIinputOutputType (aeShaderManImpl::m_ShaderIDs.m_pGeometryShaderID->m_GSInputType));
			glProgramParameteriARB (aeShaderManImpl::m_uiCurrentProgramID, GL_GEOMETRY_OUTPUT_TYPE_ARB, getGSIinputOutputType (aeShaderManImpl::m_ShaderIDs.m_pGeometryShaderID->m_GSOutputType));
		}

		glLinkProgram (aeShaderManImpl::m_uiCurrentProgramID);

		int iStatus;
		glGetProgramiv (aeShaderManImpl::m_uiCurrentProgramID, GL_LINK_STATUS, &iStatus);

		if (iStatus == GL_FALSE)
		{
			aeInt32 iLength;
			glGetProgramiv (aeShaderManImpl::m_uiCurrentProgramID, GL_INFO_LOG_LENGTH, &iLength);

			aeLog::BeginLogBlock ("Shader Linking Error");

				aeLog::Error ("Linking of shaders failed.");

				aeLog::Log ("");

				if (aeShaderManImpl::m_hCurrentVertexShader.IsValid ())
					aeLog::Log ("Vertex Shader: \"%s\"", aeResourceManager::GetFileResourceWasLoadedFrom (aeShaderManImpl::m_hCurrentVertexShader.GetAsUntypedHandle ()).c_str ());
				if (aeShaderManImpl::m_hCurrentFragmentShader.IsValid ())
					aeLog::Log ("Fragment Shader: \"%s\"", aeResourceManager::GetFileResourceWasLoadedFrom (aeShaderManImpl::m_hCurrentFragmentShader.GetAsUntypedHandle ()).c_str ());
				if (aeShaderManImpl::m_hCurrentGeometryShader.IsValid ())
					aeLog::Log ("Geometry Shader: \"%s\"", aeResourceManager::GetFileResourceWasLoadedFrom (aeShaderManImpl::m_hCurrentGeometryShader.GetAsUntypedHandle ()).c_str ());

				aeLog::Log ("");

				char* szLog = new char[iLength + 1];
				glGetProgramInfoLog (aeShaderManImpl::m_uiCurrentProgramID, iLength, &iLength, szLog);
				

				LogErrorString (szLog);
				delete[] szLog;

				aeLog::Log ("");

			aeLog::EndLogBlock ();

			// set the ID to zero, to disable shaders
			aeShaderManImpl::m_uiCurrentProgramID = 0;
		}

		aeShaderManImpl::m_ShaderPrograms[aeShaderManImpl::m_ShaderIDs] = aeShaderManImpl::aeShaderProgram ();

		// insert the ID into the known/linked programs map
		aeShaderManImpl::m_ShaderPrograms[aeShaderManImpl::m_ShaderIDs].m_uiProgramID = aeShaderManImpl::m_uiCurrentProgramID;

		// bind the new shader
		glUseProgram (aeShaderManImpl::m_uiCurrentProgramID);

		// now find the uniforms
		aeShaderManImpl::FindUsedUniforms (&aeShaderManImpl::m_ShaderPrograms[aeShaderManImpl::m_ShaderIDs]);
		aeShaderManImpl::m_pCurrentProgram = &aeShaderManImpl::m_ShaderPrograms[aeShaderManImpl::m_ShaderIDs];
	}
#endif

	void aeShaderManager::UnloadProgramsUsingShader (const aeShaderTypeData& ShaderID)
	{
#ifdef AE_RENDERAPI_OPENGL
		if (g_RenderAPI == AE_RA_OPENGL)
		{
      std::map<aeShaderCombi, aeShaderManImpl::aeShaderProgram>::iterator it = aeShaderManImpl::m_ShaderPrograms.begin ();
			const std::map<aeShaderCombi, aeShaderManImpl::aeShaderProgram>::iterator itend = aeShaderManImpl::m_ShaderPrograms.end ();

			for (; it != itend;)
			{
				if (((*it).first.m_pVertexShaderID->m_ShaderOGL == ShaderID.m_ShaderOGL) ||
					((*it).first.m_pFragmentShaderID->m_ShaderOGL == ShaderID.m_ShaderOGL) ||
					((*it).first.m_pGeometryShaderID->m_ShaderOGL == ShaderID.m_ShaderOGL))
				{
					if (aeShaderManImpl::m_pCurrentProgram == &(it->second))
						setImmediateModeShader ();

					glDeleteProgram (it->second.m_uiProgramID);

          std::map<aeShaderCombi, aeShaderManImpl::aeShaderProgram>::iterator ittemp = it;
					++it;


					aeShaderManImpl::m_ShaderPrograms.erase (ittemp);
				}
				else
					++it;
			}
		}
#endif
	}

	aeInt32 aeShaderManager::getAttributeBindPoint (const char* szAttributeName)
	{
#ifdef AE_RENDERAPI_OPENGL
		if (g_RenderAPI == AE_RA_OPENGL)
		{
			AE_CHECK_ALWAYS (!aeShaderManImpl::m_bImmediateModeShader, "aeShaderManager::getAttributeBindPoint: Cannot render a mesh with vertex-arrays, when the immediate mode shader is set. Use a pass-through shader instead.");

			return (glGetAttribLocation (aeShaderManImpl::m_uiCurrentProgramID, szAttributeName));
		}
#endif

		return (0);
	}
}



