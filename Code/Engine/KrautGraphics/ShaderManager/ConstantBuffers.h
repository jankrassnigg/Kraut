#ifndef AE_GRAPHICS_SHADERMANAGER_CONSTANTBUFFERS_H
#define AE_GRAPHICS_SHADERMANAGER_CONSTANTBUFFERS_H

#include "Declarations.h"

namespace AE_NS_GRAPHICS
{
	struct aeCBVariable
	{
		aeUInt16 m_uiOffset;

		aeShaderVariable* m_pVariable;
	};

	enum AE_UPLOAD_TO_SHADER
	{
		AE_UTS_VERTEX,
		AE_UTS_FRAGMENT,
		AE_UTS_GEOMETRY,
	};

	struct aeConstantBuffer
	{
		aeConstantBuffer () : m_uiLastGPUSync (0), m_uiDataSize (0)
    {
    }

		void UploadToGPU (AE_UPLOAD_TO_SHADER Target, aeUInt32 uiSlot);

		aeUInt32 m_uiLastGPUSync;
		aeUInt32 m_uiDataSize;
    std::vector<aeCBVariable> m_Variables;
	};

	struct aeShaderVariable
	{
		static aeUInt32 s_uiVarChangeCounter;

		aeShaderVariable () : m_uiLastGPUSync (0) {}

		aeUInt32 m_uiLastGPUSync;

		//! Temporary data to store the variable
    std::vector<aeUInt8> m_Data;

		void setVariableFloat		(aeUInt8 uiComponents, float a1, float a2 = 0, float a3 = 0, float a4 = 0);
		void setVariableInt			(aeUInt8 uiComponents, aeInt32 a1, aeInt32 a2 = 0, aeInt32 a3 = 0, aeInt32 a4 = 0);
		void setVariableFloatArray	(aeUInt8 uiComponents, const float* pData, aeUInt32 uiArraySize);
		void setVariableIntArray	(aeUInt8 uiComponents, const aeInt32* pData, aeUInt32 uiArraySize);
		void setVariableMatrix3x3	(bool bTranspose,	   const float* pData, aeUInt32 uiArraySize = 1);
		void setVariableMatrix4x4	(bool bTranspose,	   const float* pData, aeUInt32 uiArraySize = 1);
	};


}

#endif

