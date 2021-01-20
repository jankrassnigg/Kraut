// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#ifndef AE_GRAPHICS_SHADERMANAGER_UNIFORMS_H
#define AE_GRAPHICS_SHADERMANAGER_UNIFORMS_H

#include "Declarations.h"

namespace AE_NS_GRAPHICS
{
#ifdef AE_RENDERAPI_OPENGL
	union aeUniformValue
	{
		float m_fValue;
		aeInt32 m_iValue;
	};

	struct aeUniform
	{
		aeUniform () : m_uiLastChange (1) {}

		aeUInt32 m_uiLastChange;

		bool m_bFloatValues;
		bool m_bUniformArray;
		bool m_bMatrix3x3;
		bool m_bMatrix4x4;
//		bool m_bTransposeMatrix;
		aeUInt16 m_uiArraySize;

    std::vector<aeUniformValue> m_Values;
	};
#endif
}

#endif

