#include "ShaderManager.h"



namespace AE_NS_GRAPHICS
{
  aeUInt32 aeShaderVariable::s_uiVarChangeCounter = 1;

#ifdef AE_RENDERAPI_OPENGL
	void aeShaderManager::ReflectShaderForConstantBuffersOGL (aeShaderTypeData& Shader)
	{
//		AE_LOG_BLOCK b ("Reflecting Shader");

		
/*
		//! \todo  a better way to do things...
		char szUniformName[256];
		aeInt32 iActiveUniforms;
		glGetObjectParameteriv (pShaderProgram->m_uiProgramID, GL_OBJECT_ACTIVE_UNIFORMS, &iActiveUniforms);

		int iUniNameLen, iUniSize;
		GLenum iUniType;

		for (aeInt32 i = 0; i < iActiveUniforms; ++i)
			glGetActiveUniform (pShaderProgram->m_uiProgramID, i, 256, &iUniNameLen, &iUniSize, &iUniType, szUniformName);
*/
	}
#endif


	void aeConstantBuffer::UploadToGPU (AE_UPLOAD_TO_SHADER Target, aeUInt32 uiSlot)
	{
		bool bSync = false;

		const aeUInt32 uiVars = (aeUInt32) m_Variables.size ();
		for (aeUInt32 v = 0; v < uiVars; ++v)
		{
			if (m_Variables[v].m_pVariable->m_uiLastGPUSync > m_uiLastGPUSync)
			{
				bSync = true;
				break;
			}
		}

		if (!bSync)
			return;


		m_uiLastGPUSync = aeShaderVariable::s_uiVarChangeCounter;
	}

#ifdef AE_RENDERAPI_OPENGL
	void aeShaderManager::UploadConstantBuffersOGL (void)
	{
	}
#endif

	void aeShaderVariable::setVariableFloat (aeUInt8 uiComponents, float a1, float a2, float a3, float a4)
	{
		++s_uiVarChangeCounter;
		m_uiLastGPUSync = s_uiVarChangeCounter;

		if (m_Data.empty ())
			m_Data.resize (uiComponents * sizeof (float));

		AE_CHECK_DEV (m_Data.size () == uiComponents * sizeof (float), "aeShaderVariable::setVariableFloat: A Shader-Variable has been initialized with a different size, than it is used.");

		float* pData = (float*) &m_Data[0];

		pData[0] = a1;
		if (uiComponents <= 1)
			return;
		pData[1] = a2;
		if (uiComponents <= 2)
			return;
		pData[2] = a3;
		if (uiComponents <= 3)
			return;
		pData[3] = a4;
	}

	void aeShaderVariable::setVariableInt (aeUInt8 uiComponents, aeInt32 a1, aeInt32 a2, aeInt32 a3, aeInt32 a4)
	{
		++s_uiVarChangeCounter;
		m_uiLastGPUSync = s_uiVarChangeCounter;

		if (m_Data.empty ())
			m_Data.resize (uiComponents * sizeof (aeInt32));

		AE_CHECK_DEV (m_Data.size () == uiComponents * sizeof (aeInt32), "aeShaderVariable::setVariableInt: A Shader-Variable has been initialized with a different size, than it is used.");

		aeInt32* pData = (aeInt32*) &m_Data[0];

		pData[0] = a1;
		if (uiComponents <= 1)
			return;
		pData[1] = a2;
		if (uiComponents <= 2)
			return;
		pData[2] = a3;
		if (uiComponents <= 3)
			return;
		pData[3] = a4;
	}

	void aeShaderVariable::setVariableFloatArray (aeUInt8 uiComponents, const float* pData, aeUInt32 uiArraySize)
	{
		++s_uiVarChangeCounter;
		m_uiLastGPUSync = s_uiVarChangeCounter;

		if (m_Data.empty ())
			m_Data.resize (uiArraySize * uiComponents * sizeof (float));

		AE_CHECK_DEV (m_Data.size () == uiArraySize * uiComponents * sizeof (float), "aeShaderVariable::setVariableFloatArray: A Shader-Variable has been initialized with a different size, than it is used.");

    aeMemory::Copy (pData, &m_Data[0], uiArraySize * uiComponents * sizeof (float));
	}

	void aeShaderVariable::setVariableIntArray (aeUInt8 uiComponents, const aeInt32* pData, aeUInt32 uiArraySize)
	{
		++s_uiVarChangeCounter;
		m_uiLastGPUSync = s_uiVarChangeCounter;

		if (m_Data.empty ())
			m_Data.resize (uiArraySize * uiComponents * sizeof (aeInt32));

		AE_CHECK_DEV (m_Data.size () == uiArraySize * uiComponents * sizeof (aeInt32), "aeShaderVariable::setVariableIntArray: A Shader-Variable has been initialized with a different size, than it is used.");

		aeMemory::Copy (pData, &m_Data[0], uiArraySize * uiComponents * sizeof (aeInt32));
	}

	void aeShaderVariable::setVariableMatrix3x3 (bool bTranspose, const float* pData, aeUInt32 uiArraySize)
	{
		uiArraySize = aeMath::Max<aeUInt32> (uiArraySize, 1);

		++s_uiVarChangeCounter;
		m_uiLastGPUSync = s_uiVarChangeCounter;

		if (m_Data.empty ())
			m_Data.resize (uiArraySize * 9 * sizeof (float));

		AE_CHECK_DEV (m_Data.size () == uiArraySize * 9 * sizeof (float), "aeShaderVariable::setVariableMatrix3x3: A Shader-Variable has been initialized with a different size, than it is used.");

		if (!bTranspose)
			aeMemory::Copy (pData, &m_Data[0], uiArraySize * 9 * sizeof (float));
		else
		{
			float* pDst = (float*) &m_Data[0];
			for (aeUInt32 ui = 0; ui < uiArraySize; ++ui)
			{
				pDst[ui * 9 + 0] = pData[ui * 9 + 0];
				pDst[ui * 9 + 1] = pData[ui * 9 + 3];
				pDst[ui * 9 + 2] = pData[ui * 9 + 6];
				pDst[ui * 9 + 3] = pData[ui * 9 + 1];
				pDst[ui * 9 + 4] = pData[ui * 9 + 4];
				pDst[ui * 9 + 5] = pData[ui * 9 + 7];
				pDst[ui * 9 + 6] = pData[ui * 9 + 2];
				pDst[ui * 9 + 7] = pData[ui * 9 + 5];
				pDst[ui * 9 + 8] = pData[ui * 9 + 8];
			}
		}
	}

	void aeShaderVariable::setVariableMatrix4x4 (bool bTranspose, const float* pData, aeUInt32 uiArraySize)
	{
		uiArraySize = aeMath::Max<aeUInt32> (uiArraySize, 1);

		++s_uiVarChangeCounter;
		m_uiLastGPUSync = s_uiVarChangeCounter;

		if (m_Data.empty ())
			m_Data.resize (uiArraySize * 16 * sizeof (float));

		AE_CHECK_DEV (m_Data.size () == uiArraySize * 16 * sizeof (float), "aeShaderVariable::setVariableMatrix4x4: A Shader-Variable has been initialized with a different size, than it is used.");

		if (!bTranspose)
			aeMemory::Copy (pData, &m_Data[0], uiArraySize * 16 * sizeof (float));
		else
		{
			float* pDst = (float*) &m_Data[0];
			for (aeUInt32 ui = 0; ui < uiArraySize; ++ui)
			{
				pDst[ui * 16 + 0] = pData[ui * 16 + 0];
				pDst[ui * 16 + 1] = pData[ui * 16 + 4];
				pDst[ui * 16 + 2] = pData[ui * 16 + 8];
				pDst[ui * 16 + 3] = pData[ui * 16 +12];
				pDst[ui * 16 + 4] = pData[ui * 16 + 1];
				pDst[ui * 16 + 5] = pData[ui * 16 + 5];
				pDst[ui * 16 + 6] = pData[ui * 16 + 9];
				pDst[ui * 16 + 7] = pData[ui * 16 +13];
				pDst[ui * 16 + 8] = pData[ui * 16 + 2];
				pDst[ui * 16 + 9] = pData[ui * 16 + 6];
				pDst[ui * 16 +10] = pData[ui * 16 +10];
				pDst[ui * 16 +11] = pData[ui * 16 +14];
				pDst[ui * 16 +12] = pData[ui * 16 + 3];
				pDst[ui * 16 +13] = pData[ui * 16 + 7];
				pDst[ui * 16 +14] = pData[ui * 16 +11];
				pDst[ui * 16 +15] = pData[ui * 16 +15];
			}
		}
	}


}

