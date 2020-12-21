// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#ifndef AE_GRAPHICS_VERTEXARRAY_VERTEXARRAYDESCRIPTOR_H
#define AE_GRAPHICS_VERTEXARRAY_VERTEXARRAYDESCRIPTOR_H

#include "Declarations.h"
#include <KrautFoundation/Strings/IncludeAll.h>
#include <deque>

namespace AE_NS_GRAPHICS
{
  class AE_GRAPHICS_DLL aeVertexArrayDescriptor
	{
	public:
		aeVertexArrayDescriptor ();
    ~aeVertexArrayDescriptor ();

		//! Call this first to set the number of vertices, triangles and some other usage information.
		void SetupDescriptor (aeUInt32 uiVertexCount, aeUInt32 uiTriangleCount, AE_VA_USAGE Usage, bool bAllowBufferSplit = true, bool bInterleavedData = true);

    void BeginSettingAttributes (void);

		//! Adds one stream/attribute to the vertex-array. Define the semantics inside the shader (e.g. "TEXCOORD") and the format. Returns the index of the attribute-array. "POSITION" is always array 0.
		aeUInt8 AddVertexAttribute (const char* szShaderBinding, AE_VA_FORMAT eComponentFormat);

		//! Sets the data for one vertex in one of the attribute-arrays.
		void setVertexAttribute1f (aeUInt32 uiArray, aeUInt32 uiVertex, float d1);
		//! Sets the data for one vertex in one of the attribute-arrays.
		void setVertexAttribute2f (aeUInt32 uiArray, aeUInt32 uiVertex, float d1, float d2);
		//! Sets the data for one vertex in one of the attribute-arrays.
		void setVertexAttribute3f (aeUInt32 uiArray, aeUInt32 uiVertex, float d1, float d2, float d3);
		//! Sets the data for one vertex in one of the attribute-arrays.
		void setVertexAttribute4f (aeUInt32 uiArray, aeUInt32 uiVertex, float d1, float d2, float d3, float d4);
		//! Sets the data for one vertex in one of the attribute-arrays.
		void setVertexAttribute4b  (aeUInt32 uiArray, aeUInt32 uiVertex, aeInt8  d1, aeInt8  d2, aeInt8  d3, aeInt8  d4);
		//! Sets the data for one vertex in one of the attribute-arrays.
		void setVertexAttribute4ub (aeUInt32 uiArray, aeUInt32 uiVertex, aeUInt8 d1, aeUInt8 d2, aeUInt8 d3, aeUInt8 d4);
		
		//! Sets the three indices for one triangle.
		void setTriangle (aeUInt32 uiTriangle, aeUInt32 uiVertex1, aeUInt32 uiVertex2, aeUInt32 uiVertex3);

	private:
		friend class aeVertexArrayResource;

		void AllocateBuffer (void);

		struct aeVertexAttribute
		{
			aeStaticString<32> m_sSemantic;
			AE_VA_FORMAT m_ComponentFormat;
			aeUInt32 m_uiComponentSize;
			aeUInt32 m_uiDataByteOffset;
		};

		aeUInt32 m_uiVertexSize;
		aeUInt32 m_uiVertices;
		aeUInt32 m_uiTriangles;
		AE_VA_USAGE m_Usage;
		bool m_bAllowBufferSplit;
		bool m_bInterleavedData;

		static std::vector<aeUInt8> m_VertexData;

    std::vector<aeVertexAttribute> m_Attributes;
		static std::vector<aeUInt32> m_Indices;
	};

}

#pragma once

#endif

