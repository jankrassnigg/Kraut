// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#include "ShaderManager.h"



namespace AE_NS_GRAPHICS
{
	void aeShaderManager::setUniformUB (const aeStaticString<64>& sUniform, aeUInt8 f1, aeUInt8 f2, aeUInt8 f3, aeUInt8 f4)
	{
		setUniformFloat (sUniform, 4, f1 / 255.0f, f2 / 255.0f, f3 / 255.0f, f4 / 255.0f);
	}

	void aeShaderManager::setUniformFloat (const aeStaticString<64>& sUniform, aeUInt32 uiComponentCount, float f1, float f2, float f3, float f4)
	{
#ifdef AE_RENDERAPI_OPENGL
		if (g_RenderAPI == AE_RA_OPENGL)
		{
			AE_CHECK_DEV ((uiComponentCount >= 1) && (uiComponentCount <= 4), "setUniformFloat (%s, %d, %.2f, %.2f, %.2f, %.2f): The component-count is invalid, it needs to be between 1 and 4.", sUniform.c_str (), uiComponentCount, f1, f2, f3, f4);

      std::map<aeStaticString<64>, aeUniform>::iterator it = aeShaderManImpl::m_Uniforms.find (sUniform);

			if (it == aeShaderManImpl::m_Uniforms.end ())
			{
				aeUniform u;
				u.m_bFloatValues = true;
				u.m_bUniformArray = false;
				u.m_bMatrix3x3 = false;
				u.m_bMatrix4x4 = false;
				u.m_uiArraySize = 0;
				u.m_Values.resize (uiComponentCount);

				u.m_Values[0].m_fValue = f1;

				if (uiComponentCount > 1)
					u.m_Values[1].m_fValue = f2;
				if (uiComponentCount > 2)
					u.m_Values[2].m_fValue = f3;
				if (uiComponentCount > 3)
					u.m_Values[3].m_fValue = f4;

				aeShaderManImpl::m_Uniforms[sUniform] = u;
				return;
			}

			AE_CHECK_DEV (it->second.m_bFloatValues, "setUniformFloat (%s, %d, %.2f, %.2f, %.2f, %.2f): The same variable was used with integer-values before. You need to be consistent with names and types for uniforms across ALL shaders.", sUniform.c_str (), uiComponentCount, f1, f2, f3, f4);
			AE_CHECK_DEV (it->second.m_Values.size () == uiComponentCount, "setUniformFloat (%s, %d, %.2f, %.2f, %.2f, %.2f): The same variable was used with a component-count of %d. You need to be consistent with names and types for uniforms across ALL shaders.", sUniform.c_str (), uiComponentCount, f1, f2, f3, f4, it->second.m_Values.size ());
			AE_CHECK_DEV (!(it->second.m_bUniformArray), "setUniformFloat (%s, %d, %.2f, %.2f, %.2f, %.2f): The same variable was used as an array before. You need to be consistent with names and types for uniforms across ALL shaders.", sUniform.c_str (), uiComponentCount, f1, f2, f3, f4);

			if (it->second.m_Values[0].m_fValue != f1)
			{
				it->second.m_Values[0].m_fValue = f1;
				it->second.m_uiLastChange++;
			}

			if ((uiComponentCount > 1) && (it->second.m_Values[1].m_fValue != f2))
			{
				it->second.m_Values[1].m_fValue = f2;
				it->second.m_uiLastChange++;
			}
			if ((uiComponentCount > 2) && (it->second.m_Values[2].m_fValue != f3))
			{
				it->second.m_Values[2].m_fValue = f3;
				it->second.m_uiLastChange++;
			}
			if ((uiComponentCount > 3) && (it->second.m_Values[3].m_fValue != f4))
			{
				it->second.m_Values[3].m_fValue = f4;
				it->second.m_uiLastChange++;
			}
		}
#endif

	}

	void aeShaderManager::setUniformInt (const aeStaticString<64>& sUniform, aeUInt32 uiComponentCount, aeInt32 i1, aeInt32 i2, aeInt32 i3, aeInt32 i4)
	{
#ifdef AE_RENDERAPI_OPENGL
		if (g_RenderAPI == AE_RA_OPENGL)
		{
			AE_CHECK_DEV ((uiComponentCount >= 1) && (uiComponentCount <= 4), "setUniformInt (%s, %d, %d, %d, %d, %d): The component-count is invalid, it needs to be between 1 and 4.", sUniform.c_str (), uiComponentCount, i1, i2, i3, i4);

      std::map<aeStaticString<64>, aeUniform>::iterator it = aeShaderManImpl::m_Uniforms.find (sUniform);

			if (it == aeShaderManImpl::m_Uniforms.end ())
			{
				aeUniform u;
				u.m_bFloatValues = false;
				u.m_bUniformArray = false;
				u.m_bMatrix3x3 = false;
				u.m_bMatrix4x4 = false;
				u.m_uiArraySize = 0;
				u.m_Values.resize (uiComponentCount);

				u.m_Values[0].m_iValue = i1;

				if (uiComponentCount > 1)
					u.m_Values[1].m_iValue = i2;
				if (uiComponentCount > 2)
					u.m_Values[2].m_iValue = i3;
				if (uiComponentCount > 3)
					u.m_Values[3].m_iValue = i4;

				aeShaderManImpl::m_Uniforms[sUniform] = u;
				return;
			}

			AE_CHECK_DEV (!it->second.m_bFloatValues, "setUniformInt (%s, %d, %d, %d, %d, %d): The same variable was used with float-values before. You need to be consistent with names and types for uniforms across ALL shaders.", sUniform.c_str (), uiComponentCount, i1, i2, i3, i4);
			AE_CHECK_DEV (it->second.m_Values.size () == uiComponentCount, "setUniformInt (%s, %d, %d, %d, %d, %d): The same variable was used with a component-count of %d. You need to be consistent with names and types for uniforms across ALL shaders.", sUniform.c_str (), uiComponentCount, i1, i2, i3, i4, it->second.m_Values.size ());
			AE_CHECK_DEV (!it->second.m_bUniformArray, "setUniformInt (%s, %d, %d, %d, %d, %d): The same variable was used as an array before. You need to be consistent with names and types for uniforms across ALL shaders.", sUniform.c_str (), uiComponentCount, i1, i2, i3, i4);

			if (it->second.m_Values[0].m_iValue != i1)
			{
				it->second.m_Values[0].m_iValue = i1;
				it->second.m_uiLastChange++;
			}

			if ((uiComponentCount > 1) && (it->second.m_Values[1].m_iValue != i2))
			{
				it->second.m_Values[1].m_iValue = i2;
				it->second.m_uiLastChange++;
			}
			if ((uiComponentCount > 2) && (it->second.m_Values[2].m_iValue != i3))
			{
				it->second.m_Values[2].m_iValue = i3;
				it->second.m_uiLastChange++;
			}
			if ((uiComponentCount > 3) && (it->second.m_Values[3].m_iValue != i4))
			{
				it->second.m_Values[3].m_iValue = i4;
				it->second.m_uiLastChange++;
			}
		}
#endif

	}

	void aeShaderManager::setUniformFloatArray (const aeStaticString<64>& sUniform, aeUInt32 uiComponentCount, float* pElements, aeUInt32 uiArraySize)
	{
#ifdef AE_RENDERAPI_OPENGL
		if (g_RenderAPI == AE_RA_OPENGL)
		{
			AE_CHECK_DEV ((uiComponentCount >= 1) && (uiComponentCount <= 4), "setUniformFloatArray (%s, %d, floats, %d): The component-count is invalid, it needs to be between 1 and 4.", sUniform.c_str (), uiComponentCount, uiArraySize);

      std::map<aeStaticString<64>, aeUniform>::iterator it = aeShaderManImpl::m_Uniforms.find (sUniform);

			if (it == aeShaderManImpl::m_Uniforms.end ())
			{
				aeUniform u;
				u.m_bFloatValues = true;
				u.m_bUniformArray = true;
				u.m_bMatrix3x3 = false;
				u.m_bMatrix4x4 = false;
				u.m_uiArraySize = uiArraySize;
				u.m_Values.resize (uiComponentCount * uiArraySize);

				for (aeUInt32 i = 0; i < uiComponentCount * uiArraySize; ++i)
					u.m_Values[i].m_fValue = pElements[i];

				aeShaderManImpl::m_Uniforms[sUniform] = u;
				return;
			}

			it->second.m_uiArraySize = uiArraySize;
			it->second.m_Values.resize (uiComponentCount * uiArraySize);

			AE_CHECK_DEV (it->second.m_bUniformArray, "setUniformFloatArray (%s, %d, floats, %d): The same variable was used as an array before. You need to be consistent with names and types for uniforms across ALL shaders.", sUniform.c_str (), uiComponentCount, uiArraySize);
	//		AE_CHECK_DEV (it->second.m_uiArraySize == uiArraySize, "setUniformFloatArray (%s, %d, floats, %d): The same variable was used with an array-size of %d before. You need to be consistent with names and types for uniforms across ALL shaders.", sUniform.c_str (), uiComponentCount, uiArraySize, it->second.m_uiArraySize);
			AE_CHECK_DEV (it->second.m_bFloatValues, "setUniformFloatArray (%s, %d, floats, %d): The same variable was used with integer-values before. You need to be consistent with names and types for uniforms across ALL shaders.", sUniform.c_str (), uiComponentCount, uiArraySize);
	//		AE_CHECK_DEV (it->second.m_Values.size () == uiComponentCount * uiArraySize, "setUniformFloatArray (%s, %d, floats, %d): The same variable was used with a component-count of %d. You need to be consistent with names and types for uniforms across ALL shaders.", sUniform.c_str (), uiComponentCount, uiArraySize, it->second.m_Values.size () / uiArraySize);
			
			for (aeUInt32 i = 0; i < uiComponentCount * uiArraySize; ++i)
			{
				if (it->second.m_Values[i].m_fValue != pElements[i])
				{
					it->second.m_Values[i].m_fValue = pElements[i];
					it->second.m_uiLastChange++;
				}
			}
		}
#endif

	}

	void aeShaderManager::setUniformIntArray (const aeStaticString<64>& sUniform, aeUInt32 uiComponentCount, aeInt32* pElements, aeUInt32 uiArraySize)
	{
#ifdef AE_RENDERAPI_OPENGL
		if (g_RenderAPI == AE_RA_OPENGL)
		{
			AE_CHECK_DEV ((uiComponentCount >= 1) && (uiComponentCount <= 4), "setUniformIntArray (%s, %d, ints, %d): The component-count is invalid, it needs to be between 1 and 4.", sUniform.c_str (), uiComponentCount, uiArraySize);

      std::map<aeStaticString<64>, aeUniform>::iterator it = aeShaderManImpl::m_Uniforms.find (sUniform);

			if (it == aeShaderManImpl::m_Uniforms.end ())
			{
				aeUniform u;
				u.m_bFloatValues = false;
				u.m_bUniformArray = true;
				u.m_bMatrix3x3 = false;
				u.m_bMatrix4x4 = false;
				u.m_uiArraySize = uiArraySize;
				u.m_Values.resize (uiComponentCount * uiArraySize);

				for (aeUInt32 i = 0; i < uiComponentCount * uiArraySize; ++i)
					u.m_Values[i].m_iValue = pElements[i];

				aeShaderManImpl::m_Uniforms[sUniform] = u;
				return;
			}

			it->second.m_uiArraySize = uiArraySize;
			it->second.m_Values.resize (uiComponentCount * uiArraySize);

			AE_CHECK_DEV (it->second.m_bUniformArray, "setUniformIntArray (%s, %d, ints, %d): The same variable was used as an array before. You need to be consistent with names and types for uniforms across ALL shaders.", sUniform.c_str (), uiComponentCount, uiArraySize);
	//		AE_CHECK_DEV (it->second.m_uiArraySize == uiArraySize, "setUniformIntArray (%s, %d, ints, %d): The same variable was used with an array-size of %d before. You need to be consistent with names and types for uniforms across ALL shaders.", sUniform.c_str (), uiComponentCount, uiArraySize, it->second.m_uiArraySize);
			AE_CHECK_DEV (!it->second.m_bFloatValues, "setUniformIntArray (%s, %d, ints, %d): The same variable was used with float-values before. You need to be consistent with names and types for uniforms across ALL shaders.", sUniform.c_str (), uiComponentCount, uiArraySize);
	//		AE_CHECK_DEV (it->second.m_Values.size () == uiComponentCount * uiArraySize, "setUniformIntArray (%s, %d, ints, %d): The same variable was used with a component-count of %d. You need to be consistent with names and types for uniforms across ALL shaders.", sUniform.c_str (), uiComponentCount, uiArraySize, it->second.m_Values.size () / uiArraySize);
			
			for (aeUInt32 i = 0; i < uiComponentCount * uiArraySize; ++i)
			{
				if (it->second.m_Values[i].m_iValue != pElements[i])
				{
					it->second.m_Values[i].m_iValue = pElements[i];
					it->second.m_uiLastChange++;
				}
			}
		}
#endif

	}

	void aeShaderManager::setUniformMatrix3x3 (const aeStaticString<64>& sUniform, bool bTranspose, const float* fMatrix, aeUInt32 uiArraySize)
	{
#ifdef AE_RENDERAPI_OPENGL
		if (g_RenderAPI == AE_RA_OPENGL)
		{
			bool bArray = (uiArraySize > 0);
			if (uiArraySize == 0)
				uiArraySize = 1;

      std::map<aeStaticString<64>, aeUniform>::iterator it = aeShaderManImpl::m_Uniforms.find (sUniform);

			if (it == aeShaderManImpl::m_Uniforms.end ())
			{
				aeUniform u;
				u.m_bFloatValues = true;
				u.m_bUniformArray = bArray;
				u.m_uiArraySize = uiArraySize;
				u.m_Values.resize (9 * uiArraySize);
				u.m_bMatrix3x3 = true;
				u.m_bMatrix4x4 = false;

				if (!bTranspose)
				{
					for (aeUInt32 i = 0; i < uiArraySize * 9; ++i)
						u.m_Values[i].m_fValue = fMatrix[i];
				}
				else
				{
					for (aeUInt32 i = 0; i < uiArraySize; ++i)
					{
						u.m_Values[i * 9 + 0].m_fValue = fMatrix[i * 9 + 0];
						u.m_Values[i * 9 + 1].m_fValue = fMatrix[i * 9 + 3];
						u.m_Values[i * 9 + 2].m_fValue = fMatrix[i * 9 + 6];
						u.m_Values[i * 9 + 3].m_fValue = fMatrix[i * 9 + 1];
						u.m_Values[i * 9 + 4].m_fValue = fMatrix[i * 9 + 4];
						u.m_Values[i * 9 + 5].m_fValue = fMatrix[i * 9 + 7];
						u.m_Values[i * 9 + 6].m_fValue = fMatrix[i * 9 + 2];
						u.m_Values[i * 9 + 7].m_fValue = fMatrix[i * 9 + 5];
						u.m_Values[i * 9 + 8].m_fValue = fMatrix[i * 9 + 8];
					}
				}

				aeShaderManImpl::m_Uniforms[sUniform] = u;
				return;
			}

			it->second.m_uiArraySize = uiArraySize;
			it->second.m_Values.resize (9 * uiArraySize);

	//		AE_CHECK_DEV (it->second.m_uiArraySize == uiArraySize, "setUniformMatrix3x3 (%s): The same variable was used with an array-size of %d before. You need to be consistent with names and types for uniforms across ALL shaders.", sUniform.c_str (), it->second.m_uiArraySize);
			AE_CHECK_DEV (it->second.m_bFloatValues, "setUniformMatrix3x3 (%s): The same variable was used with integer-values before. You need to be consistent with names and types for uniforms across ALL shaders.", sUniform.c_str ());
	//		AE_CHECK_DEV (it->second.m_Values.size () == 9 * uiArraySize, "setUniformMatrix3x3 (%s): The same variable was used with a component-count of %d. You need to be consistent with names and types for uniforms across ALL shaders.", sUniform.c_str (), it->second.m_Values.size () / uiArraySize);

			// when a matrix is set, it is always expected to be different???
			it->second.m_uiLastChange++;
			
			if (!bTranspose)
			{
				for (aeUInt32 i = 0; i < uiArraySize * 9; ++i)
					it->second.m_Values[i].m_fValue = fMatrix[i];
			}
			else
			{
				for (aeUInt32 i = 0; i < uiArraySize; ++i)
				{
					it->second.m_Values[i * 9 + 0].m_fValue = fMatrix[i * 9 + 0];
					it->second.m_Values[i * 9 + 1].m_fValue = fMatrix[i * 9 + 3];
					it->second.m_Values[i * 9 + 2].m_fValue = fMatrix[i * 9 + 6];
					it->second.m_Values[i * 9 + 3].m_fValue = fMatrix[i * 9 + 1];
					it->second.m_Values[i * 9 + 4].m_fValue = fMatrix[i * 9 + 4];
					it->second.m_Values[i * 9 + 5].m_fValue = fMatrix[i * 9 + 7];
					it->second.m_Values[i * 9 + 6].m_fValue = fMatrix[i * 9 + 2];
					it->second.m_Values[i * 9 + 7].m_fValue = fMatrix[i * 9 + 5];
					it->second.m_Values[i * 9 + 8].m_fValue = fMatrix[i * 9 + 8];
				}
			}
		}
#endif
		
	}

	void aeShaderManager::setUniformMatrix4x4 (const aeStaticString<64>& sUniform, bool bTranspose, const float* fMatrix, aeUInt32 uiArraySize)
	{
#ifdef AE_RENDERAPI_OPENGL
		if (g_RenderAPI == AE_RA_OPENGL)
		{
			bool bArray = (uiArraySize > 0);
			if (uiArraySize == 0)
				uiArraySize = 1;

      std::map<aeStaticString<64>, aeUniform>::iterator it = aeShaderManImpl::m_Uniforms.find (sUniform);

			if (it == aeShaderManImpl::m_Uniforms.end ())
			{
				aeUniform u;
				u.m_bFloatValues = true;
				u.m_bUniformArray = bArray;
				u.m_uiArraySize = uiArraySize;
				u.m_Values.resize (16 * uiArraySize);
				u.m_bMatrix4x4 = true;
				u.m_bMatrix3x3 = false;

				if (!bTranspose)
				{
					for (aeUInt32 i = 0; i < uiArraySize * 16; ++i)
						u.m_Values[i].m_fValue = fMatrix[i];
				}
				else
				{
					for (aeUInt32 i = 0; i < uiArraySize; ++i)
					{
						u.m_Values[i * 16 + 0].m_fValue = fMatrix[i * 16 + 0];
						u.m_Values[i * 16 + 1].m_fValue = fMatrix[i * 16 + 4];
						u.m_Values[i * 16 + 2].m_fValue = fMatrix[i * 16 + 8];
						u.m_Values[i * 16 + 3].m_fValue = fMatrix[i * 16 +12];

						u.m_Values[i * 16 + 4].m_fValue = fMatrix[i * 16 + 1];
						u.m_Values[i * 16 + 5].m_fValue = fMatrix[i * 16 + 5];
						u.m_Values[i * 16 + 6].m_fValue = fMatrix[i * 16 + 9];
						u.m_Values[i * 16 + 7].m_fValue = fMatrix[i * 16 +13];

						u.m_Values[i * 16 + 8].m_fValue = fMatrix[i * 16 + 2];
						u.m_Values[i * 16 + 9].m_fValue = fMatrix[i * 16 + 6];
						u.m_Values[i * 16 +10].m_fValue = fMatrix[i * 16 +10];
						u.m_Values[i * 16 +11].m_fValue = fMatrix[i * 16 +14];

						u.m_Values[i * 16 +12].m_fValue = fMatrix[i * 16 + 3];
						u.m_Values[i * 16 +13].m_fValue = fMatrix[i * 16 + 7];
						u.m_Values[i * 16 +14].m_fValue = fMatrix[i * 16 +11];
						u.m_Values[i * 16 +15].m_fValue = fMatrix[i * 16 +15];
					}
				}

				aeShaderManImpl::m_Uniforms[sUniform] = u;
				return;
			}

			it->second.m_uiArraySize = uiArraySize;
			it->second.m_Values.resize (16 * uiArraySize);

	//		AE_CHECK_DEV (it->second.m_uiArraySize == uiArraySize, "setUniformMatrix4x4 (%s): The same variable was used with an array-size of %d before. You need to be consistent with names and types for uniforms across ALL shaders.", sUniform.c_str (), it->second.m_uiArraySize);
			AE_CHECK_DEV (it->second.m_bFloatValues, "setUniformMatrix4x4 (%s): The same variable was used with integer-values before. You need to be consistent with names and types for uniforms across ALL shaders.", sUniform.c_str ());
	//		AE_CHECK_DEV (it->second.m_Values.size () == 16 * uiArraySize, "setUniformMatrix4x4 (%s): The same variable was used with a component-count of %d. You need to be consistent with names and types for uniforms across ALL shaders.", sUniform.c_str (), it->second.m_Values.size () / uiArraySize);
			

			// when a matrix is set, it is always expected to be different???
			it->second.m_uiLastChange++;

			if (!bTranspose)
			{
				for (aeUInt32 i = 0; i < uiArraySize * 16; ++i)
					it->second.m_Values[i].m_fValue = fMatrix[i];
			}
			else
			{
				for (aeUInt32 i = 0; i < uiArraySize; ++i)
				{
					it->second.m_Values[i * 16 + 0].m_fValue = fMatrix[i * 16 + 0];
					it->second.m_Values[i * 16 + 1].m_fValue = fMatrix[i * 16 + 4];
					it->second.m_Values[i * 16 + 2].m_fValue = fMatrix[i * 16 + 8];
					it->second.m_Values[i * 16 + 3].m_fValue = fMatrix[i * 16 +12];

					it->second.m_Values[i * 16 + 4].m_fValue = fMatrix[i * 16 + 1];
					it->second.m_Values[i * 16 + 5].m_fValue = fMatrix[i * 16 + 5];
					it->second.m_Values[i * 16 + 6].m_fValue = fMatrix[i * 16 + 9];
					it->second.m_Values[i * 16 + 7].m_fValue = fMatrix[i * 16 +13];

					it->second.m_Values[i * 16 + 8].m_fValue = fMatrix[i * 16 + 2];
					it->second.m_Values[i * 16 + 9].m_fValue = fMatrix[i * 16 + 6];
					it->second.m_Values[i * 16 +10].m_fValue = fMatrix[i * 16 +10];
					it->second.m_Values[i * 16 +11].m_fValue = fMatrix[i * 16 +14];

					it->second.m_Values[i * 16 +12].m_fValue = fMatrix[i * 16 + 3];
					it->second.m_Values[i * 16 +13].m_fValue = fMatrix[i * 16 + 7];
					it->second.m_Values[i * 16 +14].m_fValue = fMatrix[i * 16 +11];
					it->second.m_Values[i * 16 +15].m_fValue = fMatrix[i * 16 +15];
				}
			}
		}
#endif

	}


#ifdef AE_RENDERAPI_OPENGL
	void aeShaderManImpl::FindUsedUniforms (aeShaderProgram* pShaderProgram)
	{
		if (pShaderProgram == NULL)
			return;

		pShaderProgram->m_Uniforms.clear ();

		AE_LOG_BLOCK ("FindUsedUniforms");

		aeInt32 iActiveUniforms;
		glGetProgramiv (pShaderProgram->m_uiProgramID, GL_ACTIVE_UNIFORMS, &iActiveUniforms);

		aeLog::Debug ("Shader has %d active Uniforms.", iActiveUniforms);

		aeUInt32 uiSamplers = 0;

		for (aeUInt32 ui = 0; ui < 16; ++ui)
		{
			pShaderProgram->m_UsedTextures[ui] = aeShaderManImpl::m_TexturesToBind.end ();
		}

		for (aeInt32 ui = 0; ui < iActiveUniforms; ++ui)
		{
			aeUniform Uniform;
			Uniform.m_bMatrix3x3 = false;
			Uniform.m_bMatrix4x4 = false;

			aeUInt32 uiComponents = 1;

			char szTemp[256];
			GLsizei uiLen;

			GLint iSize;
			GLenum Type;

			glGetActiveUniform (pShaderProgram->m_uiProgramID, ui, 256, &uiLen, &iSize, &Type, szTemp);

			const aeInt32 iLoc = glGetUniformLocation (pShaderProgram->m_uiProgramID, szTemp);

			aeLog::Debug ("Uniform %d is named \"%s\", Location = %d, Size = %d", ui, szTemp, iLoc, iSize);

			bool bSampler = false;

			switch (Type)
			{
			case GL_FLOAT:
				aeLog::Debug ("Type is GL_FLOAT");
				Uniform.m_bFloatValues = true;
				uiComponents = 1;
				break;
			case GL_FLOAT_VEC2:
				aeLog::Debug ("Type is GL_FLOAT_VEC2");
				Uniform.m_bFloatValues = true;
				uiComponents = 2;
				break;
			case GL_FLOAT_VEC3:
				aeLog::Debug ("Type is GL_FLOAT_VEC3");
				Uniform.m_bFloatValues = true;
				uiComponents = 3;
				break;
			case GL_FLOAT_VEC4:
				aeLog::Debug ("Type is GL_FLOAT_VEC4");
				Uniform.m_bFloatValues = true;
				uiComponents = 4;
				break;
			case GL_INT:
				aeLog::Debug ("Type is GL_INT");
				Uniform.m_bFloatValues = false;
				uiComponents = 1;
				break;
			case GL_INT_VEC2:
				aeLog::Debug ("Type is GL_INT_VEC2");
				Uniform.m_bFloatValues = false;
				uiComponents = 2;
				break;
			case GL_INT_VEC3:
				aeLog::Debug ("Type is GL_INT_VEC3");
				Uniform.m_bFloatValues = false;
				uiComponents = 3;
				break;
			case GL_INT_VEC4:
				aeLog::Debug ("Type is GL_INT_VEC4");
				Uniform.m_bFloatValues = false;
				uiComponents = 4;
				break;
			case GL_FLOAT_MAT2:
				aeLog::Debug ("Type is GL_FLOAT_MAT2");
				Uniform.m_bFloatValues = true;
				uiComponents = 4;
				break;
			case GL_FLOAT_MAT3:
				aeLog::Debug ("Type is GL_FLOAT_MAT3");
				Uniform.m_bFloatValues = true;
				Uniform.m_bMatrix3x3 = true;
				uiComponents = 9;
				break;
			case GL_FLOAT_MAT4:
				aeLog::Debug ("Type is GL_FLOAT_MAT4");
				Uniform.m_bFloatValues = true;
				Uniform.m_bMatrix4x4 = true;
				uiComponents = 16;
				break;

			case GL_BOOL:
			case GL_BOOL_VEC2:
			case GL_BOOL_VEC3:
			case GL_BOOL_VEC4:
			case GL_FLOAT_MAT2x3:
			case GL_FLOAT_MAT2x4:
			case GL_FLOAT_MAT3x2:
			case GL_FLOAT_MAT3x4:
			case GL_FLOAT_MAT4x2:
			case GL_FLOAT_MAT4x3:
				AE_CHECK_ALWAYS (false, "A Shader contains a Uniform of an unsupported Type (bool, MatrixNxM)");
        break;

			case GL_SAMPLER_1D:
			case GL_SAMPLER_1D_SHADOW:
			case GL_SAMPLER_1D_ARRAY:
			case GL_SAMPLER_1D_ARRAY_SHADOW:
			case GL_SAMPLER_2D:
			case GL_SAMPLER_2D_SHADOW:
			case GL_SAMPLER_2D_ARRAY:
			case GL_SAMPLER_2D_ARRAY_SHADOW:
			case GL_SAMPLER_2D_RECT:
			case GL_SAMPLER_2D_RECT_SHADOW:
			case GL_SAMPLER_2D_MULTISAMPLE:
			case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:
			case GL_INT_SAMPLER_2D_MULTISAMPLE:
			case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
			case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
			case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
			case GL_SAMPLER_3D:
			case GL_SAMPLER_CUBE:
			case GL_SAMPLER_CUBE_MAP_ARRAY_ARB:
			case GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW_ARB:
			case GL_SAMPLER_BUFFER:
			case GL_SAMPLER_CUBE_SHADOW:
			case GL_INT_SAMPLER_1D:
			case GL_INT_SAMPLER_2D:
			case GL_INT_SAMPLER_3D:
			case GL_INT_SAMPLER_CUBE:
			case GL_INT_SAMPLER_2D_RECT:
			case GL_INT_SAMPLER_1D_ARRAY:
			case GL_INT_SAMPLER_2D_ARRAY:
			case GL_INT_SAMPLER_BUFFER:
			case GL_UNSIGNED_INT_SAMPLER_1D:
			case GL_UNSIGNED_INT_SAMPLER_2D:
			case GL_UNSIGNED_INT_SAMPLER_3D:
			case GL_UNSIGNED_INT_SAMPLER_CUBE:
			case GL_UNSIGNED_INT_SAMPLER_2D_RECT:
			case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY:
			case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
			case GL_UNSIGNED_INT_SAMPLER_BUFFER:
				aeLog::Debug ("Type is SAMPLER");
				Uniform.m_bFloatValues = false;
				uiComponents = 1;
				bSampler = true;
				break;

			default:
				AE_CHECK_ALWAYS (false, "Uniform has unknown Type %d", Type);
        break;
			}

			if (bSampler)
			{
        szTemp[aeStringFunctions::GetLength (szTemp) - 3] = '\0';

				aeShaderManImpl::m_TexturesToBind[szTemp];
				pShaderProgram->m_UsedTextures[uiSamplers] = aeShaderManImpl::m_TexturesToBind.find (szTemp);

				glUniform1i (iLoc, uiSamplers);

				aeLog::Debug ("Binding Uniform Sampler to Texture Unit %d", uiSamplers);

				++uiSamplers;

				continue;
			}

			if (aeShaderManImpl::m_Uniforms.find (szTemp) == aeShaderManImpl::m_Uniforms.end ())
			{
				Uniform.m_uiArraySize = iSize;
				Uniform.m_bUniformArray = (iSize > 1);
				Uniform.m_Values.resize (iSize * uiComponents);

				aeShaderManImpl::m_Uniforms[szTemp] = Uniform;
			}

			aeUniformTuple uTuple;
			uTuple.m_iBindPoint = iLoc;
			uTuple.m_pUniform = &aeShaderManImpl::m_Uniforms.find (szTemp)->second;

			pShaderProgram->m_Uniforms.push_back (uTuple);
		}
	}
#endif

	void aeShaderManager::setUsedUniforms (void)
	{
#ifdef AE_RENDERAPI_OPENGL
		if (g_RenderAPI == AE_RA_OPENGL)
		{
			if ((aeShaderManImpl::m_uiCurrentProgramID == 0) || (aeShaderManImpl::m_pCurrentProgram == NULL))
				return;

      std::vector<aeShaderManImpl::aeUniformTuple>::iterator it = aeShaderManImpl::m_pCurrentProgram->m_Uniforms.begin ();
			const std::vector<aeShaderManImpl::aeUniformTuple>::iterator itend = aeShaderManImpl::m_pCurrentProgram->m_Uniforms.end ();

			for (; it != itend; ++it)
			{
				if (it->m_uiLastUpdate >= it->m_pUniform->m_uiLastChange)
					continue;

				it->m_uiLastUpdate = it->m_pUniform->m_uiLastChange;

				const aeInt32 iLoc = it->m_iBindPoint;
				const aeUniform* pUniform = it->m_pUniform;

				// non-arrays
				if (!pUniform->m_bUniformArray)
				{
					// 3x3 Matrices
					if (pUniform->m_bMatrix3x3)
					{
						glUniformMatrix3fv (iLoc, pUniform->m_uiArraySize, false, &(pUniform->m_Values[0].m_fValue));
					}
					else
					// 4x4 Matrices
					if (pUniform->m_bMatrix4x4)
					{
						glUniformMatrix4fv (iLoc, pUniform->m_uiArraySize, false, &(pUniform->m_Values[0].m_fValue));
					}
					else
					// float values
					if (pUniform->m_bFloatValues)
					{
						switch (pUniform->m_Values.size ())
						{
						case 1:
							glUniform1f (iLoc, pUniform->m_Values[0].m_fValue);
							break;
						case 2:
							glUniform2f (iLoc, pUniform->m_Values[0].m_fValue, pUniform->m_Values[1].m_fValue);
							break;
						case 3:
							glUniform3f (iLoc, pUniform->m_Values[0].m_fValue, pUniform->m_Values[1].m_fValue, pUniform->m_Values[2].m_fValue);
							break;
						case 4:
							glUniform4f (iLoc, pUniform->m_Values[0].m_fValue, pUniform->m_Values[1].m_fValue, pUniform->m_Values[2].m_fValue, pUniform->m_Values[3].m_fValue);
							break;
						default:
							AE_CHECK_ALWAYS (false, "aeShaderManager::setUsedUniforms: Default case 1");
              break;
						}
					}
					// integer values
					else
					{
						switch (pUniform->m_Values.size ())
						{
						case 1:
							glUniform1i (iLoc, pUniform->m_Values[0].m_iValue);
							break;
						case 2:
							glUniform2i (iLoc, pUniform->m_Values[0].m_iValue, pUniform->m_Values[1].m_iValue);
							break;
						case 3:
							glUniform3i (iLoc, pUniform->m_Values[0].m_iValue, pUniform->m_Values[1].m_iValue, pUniform->m_Values[2].m_iValue);
							break;
						case 4:
							glUniform4i (iLoc, pUniform->m_Values[0].m_iValue, pUniform->m_Values[1].m_iValue, pUniform->m_Values[2].m_iValue, pUniform->m_Values[3].m_iValue);
							break;
						default:
							AE_CHECK_ALWAYS (false, "aeShaderManager::setUsedUniforms: Default case 2");
              break;
						}
					}
				}
				// arrays
				else
				{
					// 3x3 Matrices
					if (pUniform->m_bMatrix3x3)
					{
						glUniformMatrix3fv (iLoc, pUniform->m_uiArraySize, false, &(pUniform->m_Values[0].m_fValue));
					}
					else
					// 4x4 Matrices
					if (pUniform->m_bMatrix4x4)
					{
						glUniformMatrix4fv (iLoc, pUniform->m_uiArraySize, false, &(pUniform->m_Values[0].m_fValue));
					}
					else
					// float arrays
					if (pUniform->m_bFloatValues)
					{
						switch (pUniform->m_Values.size () / pUniform->m_uiArraySize)
						{
						case 1:
							glUniform1fv (iLoc, pUniform->m_uiArraySize, &pUniform->m_Values[0].m_fValue);
							break;
						case 2:
							glUniform2fv (iLoc, pUniform->m_uiArraySize, &pUniform->m_Values[0].m_fValue);
							break;
						case 3:
							glUniform3fv (iLoc, pUniform->m_uiArraySize, &pUniform->m_Values[0].m_fValue);
							break;
						case 4:
							glUniform4fv (iLoc, pUniform->m_uiArraySize, &pUniform->m_Values[0].m_fValue);
							break;
						default:
							AE_CHECK_ALWAYS (false, "aeShaderManager::setUsedUniforms: Default case 3");
              break;
						}
					}
					else
					// int arrays
					{
						switch (pUniform->m_Values.size () / pUniform->m_uiArraySize)
						{
						case 1:
							glUniform1iv (iLoc, pUniform->m_uiArraySize, &pUniform->m_Values[0].m_iValue);
							break;
						case 2:
							glUniform2iv (iLoc, pUniform->m_uiArraySize, &pUniform->m_Values[0].m_iValue);
							break;
						case 3:
							glUniform3iv (iLoc, pUniform->m_uiArraySize, &pUniform->m_Values[0].m_iValue);
							break;
						case 4:
							glUniform4iv (iLoc, pUniform->m_uiArraySize, &pUniform->m_Values[0].m_iValue);
							break;
						default:
							AE_CHECK_ALWAYS (false, "aeShaderManager::setUsedUniforms: Default case 4");
              break;
						}
					}
				}
			}
		}
#endif
	}
}









