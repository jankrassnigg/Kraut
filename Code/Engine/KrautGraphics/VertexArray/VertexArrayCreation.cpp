#include "VertexArrayResource.h"



namespace AE_NS_GRAPHICS
{

  AE_RESOURCE_IMPLEMENTATION (aeVertexArrayResource, "");

#ifdef AE_RENDERAPI_OPENGL

	GLenum getBufferUsageOGL (AE_VA_USAGE Usage)
	{
		switch (Usage)
		{
		case AE_VAU_STATIC:
			return (GL_STATIC_DRAW);
		case AE_VAU_DYNAMIC:
			return (GL_DYNAMIC_DRAW);
		case AE_VAU_STREAM:
			return (GL_STREAM_DRAW);
		}

		AE_CHECK_ALWAYS (false, "getBufferUsage: Unknown Usage %d", Usage);
    return GL_STATIC_DRAW;
	}

#endif

	void aeVertexArrayResource::UnloadResource (void)
	{
		for (aeUInt32 uiPart = 0; uiPart < (aeUInt32) m_BufferPartitions.size (); ++uiPart)
		{
#ifdef AE_RENDERAPI_OPENGL
			if (g_RenderAPI == AE_RA_OPENGL)
			{
				if (m_BufferPartitions[uiPart].m_uiVertexBufferID != 0)
					glDeleteBuffers (1, &m_BufferPartitions[uiPart].m_uiVertexBufferID);
				if (m_BufferPartitions[uiPart].m_uiIndexBufferID != 0)
					glDeleteBuffers (1, &m_BufferPartitions[uiPart].m_uiIndexBufferID);
			}
#endif
		}

		m_BufferPartitions.clear ();
		m_VertexAttributes.clear ();
	}

	void aeVertexArrayResource::CreateResourceFromDescriptor (void* pDescriptor)
	{
		//AE_LOG_BLOCK b ("Creating VertexArray");

		aeVertexArrayDescriptor* pDesc = (aeVertexArrayDescriptor*) pDescriptor;

		m_bBufferZeroIsBound = false;
		m_bInterleavedData = pDesc->m_bInterleavedData;
		m_uiVertexSize = pDesc->m_uiVertexSize;
		m_uiNumberOfVertices = pDesc->m_uiVertices;
		m_uiNumberOfTriangles = pDesc->m_uiTriangles;
		m_uiNumberOfIndices = (aeUInt32) pDesc->m_Indices.size ();

		//aeLog::Log ("%d Vertices, %d Triangles, VertexSize: %d bytes, %d Indices", m_uiNumberOfVertices, m_uiNumberOfTriangles, m_uiVertexSize, m_uiNumberOfIndices);

		if (pDesc->m_bAllowBufferSplit)
		{
			//aeLog::Log ("Buffer Split is allowed");

			aeUInt32 uiFirstTriangle = 0;
			aeUInt32 uiFirstVertex = 0;

			aeUInt32 uiLastVertex = 0;		// inclusive
			aeUInt32 uiLastTriangle = 0;	// inclusive

			aeUInt32 uiVert0Prev = 0;
			aeUInt32 uiVert2Prev = 0;

			const aeUInt32 uiMaxVerticesInPartition = 0xFFFF;

			for (aeUInt32 tri = 0; tri < m_uiNumberOfTriangles; ++tri)
			{
				aeUInt32 uiVert0 = pDesc->m_Indices[tri * 3];
				aeUInt32 uiVert2 = pDesc->m_Indices[tri * 3 + 2];

				if (uiVert0 > uiVert2Prev) // polygon boundary => split possible
				{
					// if the number of used vertices is below the max buffer size
					if (uiVert2Prev - uiFirstVertex < uiMaxVerticesInPartition)
					{
						uiLastVertex = uiVert2Prev;
						uiLastTriangle = tri - 1;
					}
					else
					{
						// split the buffers here...
						UploadBufferPartition (uiFirstVertex, uiLastVertex - uiFirstVertex + 1, uiFirstTriangle, uiLastTriangle - uiFirstTriangle + 1, pDesc);

						uiFirstVertex = uiLastVertex + 1;
						uiFirstTriangle = uiLastTriangle + 1;
					}
				}

				uiVert0Prev = uiVert0;
				uiVert2Prev = uiVert2;
			}

			uiLastVertex = m_uiNumberOfVertices - 1;
			uiLastTriangle = m_uiNumberOfTriangles - 1;

			UploadBufferPartition (uiFirstVertex, uiLastVertex - uiFirstVertex + 1, uiFirstTriangle, uiLastTriangle - uiFirstTriangle + 1, pDesc);
		}
		else
		{
			UploadBufferPartition (0, m_uiNumberOfVertices, 0, m_uiNumberOfTriangles, pDesc);
		}

		// copy the important attribute information
		const aeUInt32 uiAttributes = (aeUInt32) pDesc->m_Attributes.size ();
		m_VertexAttributes.resize (uiAttributes);

		for (aeUInt32 uiAttr = 0; uiAttr < uiAttributes; ++uiAttr)
		{
			m_VertexAttributes[uiAttr].m_sSemantic = pDesc->m_Attributes[uiAttr].m_sSemantic;
			m_VertexAttributes[uiAttr].m_ComponentFormat = pDesc->m_Attributes[uiAttr].m_ComponentFormat;
			m_VertexAttributes[uiAttr].m_uiComponentSize = pDesc->m_Attributes[uiAttr].m_uiComponentSize;
		}
	}

	aeVertexArrayResource::aeBufferPartition::aeBufferPartition ()
	{
#ifdef AE_RENDERAPI_OPENGL
		m_uiVertexBufferID = 0;
		m_uiIndexBufferID = 0;
#endif

		m_uiFirstVertex = 0;
		m_uiVertexCount = 0;
		m_uiFirstTriangle = 0;
		m_uiTriangleCount = 0;
	}

	void aeVertexArrayResource::UploadBufferPartition (aeUInt32 uiFirstVertex, aeUInt32 uiVertexCount, aeUInt32 uiFirstTriangle, aeUInt32 uiTriangleCount, aeVertexArrayDescriptor* pDesc)
	{
		//aeLog::Log ("Buffer Partition: First Vertex %d, Vertices %d, first triangle %d, Triangles %d", uiFirstVertex, uiVertexCount, uiFirstTriangle, uiTriangleCount);

		const aeUInt32 uiPartition = (aeUInt32) m_BufferPartitions.size ();

		// create the buffer partition
		m_BufferPartitions.push_back (aeBufferPartition ());

		// store some basic info
		m_BufferPartitions.back ().m_uiFirstVertex = uiFirstVertex;
		m_BufferPartitions.back ().m_uiVertexCount = uiVertexCount;
		m_BufferPartitions.back ().m_uiFirstTriangle = uiFirstTriangle;
		m_BufferPartitions.back ().m_uiTriangleCount = uiTriangleCount;

		// upload the vertex buffer

#ifdef AE_RENDERAPI_OPENGL
		if (g_RenderAPI == AE_RA_OPENGL)
		{
			glGenBuffers (1, &m_BufferPartitions.back ().m_uiVertexBufferID);
			glBindBuffer (GL_ARRAY_BUFFER, m_BufferPartitions.back ().m_uiVertexBufferID);

			//aeLog::Log ("Uploading %d bytes from position %d", pDesc->m_uiVertexSize * uiVertexCount, uiFirstVertex * pDesc->m_uiVertexSize);

			glBufferData (GL_ARRAY_BUFFER, pDesc->m_uiVertexSize * uiVertexCount, &pDesc->m_VertexData[uiFirstVertex * pDesc->m_uiVertexSize], getBufferUsageOGL (pDesc->m_Usage));
		}
#endif

    const aeUInt32 uiIndices = uiTriangleCount * 3;

		m_BufferPartitions.back ().m_uiBytesPerIndex = 2;	// unsigned short indices
		
		// scan index-buffer for index size
		for (aeUInt32 index = 0; index < uiIndices; ++index)
		{
			aeInt32 iTempIndex = (aeUInt32) (pDesc->m_Indices[uiFirstTriangle * 3 + index]) - (aeUInt32) (uiFirstVertex);

			AE_CHECK_DEV (iTempIndex >= 0, "aeVertexArrayResource::UploadBufferPartition: Vertex-Index in buffer-partition is smaller than 0 => Your data is not suited for buffer splitting. Disable buffer-splits to avoid this.");

			if (iTempIndex >= 0xFFFF )
				m_BufferPartitions.back ().m_uiBytesPerIndex = 4;		// unsigned int indices
		}

		//aeLog::Log ("Bytes per Index: %d", m_BufferPartitions.back ().m_uiBytesPerIndex);


		// upload the index buffer

    std::vector<aeUInt8> IndexBlob;
	
		IndexBlob.resize (uiIndices * m_BufferPartitions.back ().m_uiBytesPerIndex);

		aeUInt16* pIndex16 = (aeUInt16*) &IndexBlob[0];
		aeUInt32* pIndex32 = (aeUInt32*) &IndexBlob[0];

		for (aeUInt32 index = 0; index < uiIndices; ++index)
		{
			aeInt32 iTempIndex = (aeUInt32) (pDesc->m_Indices[uiFirstTriangle * 3 + index]) - (aeUInt32) (uiFirstVertex);

			if (m_BufferPartitions.back ().m_uiBytesPerIndex == 2)
			{
				*pIndex16 = (aeUInt16) iTempIndex;
				++pIndex16;
			}
			else
			{
				*pIndex32 = (aeUInt32) iTempIndex;
				++pIndex32;
			}
		}

#ifdef AE_RENDERAPI_OPENGL
		if (g_RenderAPI == AE_RA_OPENGL)
		{
			glGenBuffers (1, &m_BufferPartitions.back ().m_uiIndexBufferID);
			glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, m_BufferPartitions.back ().m_uiIndexBufferID);
			glBufferData (GL_ELEMENT_ARRAY_BUFFER, uiIndices * m_BufferPartitions.back ().m_uiBytesPerIndex, (void*) &IndexBlob[0], getBufferUsageOGL (pDesc->m_Usage));

			//aeLog::Log ("Uploading %d Indices, %d IndexBuffer bytes", uiIndices, uiIndices * m_BufferPartitions.back ().m_uiBytesPerIndex);
		}
#endif
	}

}

