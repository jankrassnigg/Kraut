#include "VertexArrayDescriptor.h"



namespace AE_NS_GRAPHICS
{
  std::vector<aeUInt8> aeVertexArrayDescriptor::m_VertexData;
  std::vector<aeUInt32> aeVertexArrayDescriptor::m_Indices;

	static aeUInt32 getComponentSize (AE_VA_FORMAT eComponentFormat)
	{
		switch (eComponentFormat)
		{
		case AE_VAF_R32G32B32A32_FLOAT:
			return (4 * sizeof (float));
		case AE_VAF_R32G32B32_FLOAT:
			return (3 * sizeof (float));
		case AE_VAF_R32G32_FLOAT:
			return (2 * sizeof (float));
		case AE_VAF_R32_FLOAT:
			return (1 * sizeof (float));
		case AE_VAF_R8G8B8A8_UNORM:
		case AE_VAF_R8G8B8A8_SNORM:
		case AE_VAF_R8G8B8A8_UINT:
		case AE_VAF_R8G8B8A8_SINT:
			return (4 * sizeof (aeInt8));

		default:
			AE_CHECK_ALWAYS (false, "getComponentSize: Format %d is unknown", eComponentFormat);
      break;
		}

		return (0);
	}

	aeVertexArrayDescriptor::aeVertexArrayDescriptor ()
	{
		m_uiVertices = 0;
		m_uiTriangles = 0;
		m_Usage = AE_VAU_NONE;
		m_bAllowBufferSplit = true;
		m_bInterleavedData = true;
		m_uiVertexSize = 0;
	}

  aeVertexArrayDescriptor::~aeVertexArrayDescriptor ()
  {
  }

	/*! Usage: static / dynamic / streaming\n
		BufferSplit: If true, tries to create n buffers with 16-bit indices, rather than 1 buffer with 32-bit indices.\n
		Not possible when the vertex-array is supposed to be used for instanced rendering.\n
		Also only possible if the triangle-indices index the vertices mostly in sequence, not too random.\n
		Interleaved: If set to true data will be interleaved, otherwise n streams will be used.\n
	*/
	void aeVertexArrayDescriptor::SetupDescriptor (aeUInt32 uiVertexCount, aeUInt32 uiTriangleCount, AE_VA_USAGE Usage, bool bAllowBufferSplit, bool bInterleavedData)
	{
		m_uiVertices = uiVertexCount;
		m_uiTriangles = uiTriangleCount;
		m_Indices.resize (uiTriangleCount * 3);
		m_Usage = Usage;
		m_bAllowBufferSplit = bAllowBufferSplit;
		m_bInterleavedData = bInterleavedData;

		// otherwise one would need one signature for every buffer partition
		// also uploading data would make trouble
		if (!bInterleavedData)
			m_bAllowBufferSplit = false;

		m_Attributes.clear ();
    m_Attributes.reserve (6);
		
		AddVertexAttribute ("attr_Position", AE_VAF_R32G32B32_FLOAT);
	}

	void aeVertexArrayDescriptor::AllocateBuffer (void)
	{
		//if (m_VertexData.empty ())
    m_VertexData.reserve (1024 * 1024 * 256);
			m_VertexData.resize (m_uiVertexSize * m_uiVertices);
	}

	aeUInt8 aeVertexArrayDescriptor::AddVertexAttribute (const char* szShaderBinding, AE_VA_FORMAT eComponentFormat)
	{
		//AE_CHECK_DEV (m_VertexData.empty (), "aeVertexArrayDescriptor::AddVertexAttribute: Vertex-data has already been set.");

		const aeUInt8 uiID = (aeUInt8) (m_Attributes.size ());

		aeVertexAttribute attr;
		attr.m_ComponentFormat = eComponentFormat;
		attr.m_sSemantic = szShaderBinding;
		attr.m_uiComponentSize = getComponentSize (eComponentFormat);

		if (m_bInterleavedData)
			attr.m_uiDataByteOffset = m_uiVertexSize;
		else
			attr.m_uiDataByteOffset = m_uiVertexSize * m_uiVertices;

		m_Attributes.push_back (attr);

		m_uiVertexSize += attr.m_uiComponentSize;

		return (uiID);
	}

  void aeVertexArrayDescriptor::BeginSettingAttributes (void)
  {
    AllocateBuffer ();

  }

	void aeVertexArrayDescriptor::setVertexAttribute1f (aeUInt32 uiArray, aeUInt32 uiVertex, float d1)
	{
		AE_CHECK_DEV (uiArray < m_Attributes.size (), "aeVertexArrayDescriptor::setVertexAttribute1f: Cannot access attribute-array %d, only %d arrays were created.", uiArray, m_Attributes.size ());
		AE_CHECK_DEV (uiVertex < m_uiVertices, "aeVertexArrayDescriptor::setVertexAttribute1f: Cannot set vertex %d, only %d vertices were created.", uiVertex, m_uiVertices);

		const aeUInt32 uiCompSize = m_Attributes[uiArray].m_uiComponentSize;

		float* pData = nullptr;
		if (m_bInterleavedData)
			pData = (float*) &(m_VertexData[m_Attributes[uiArray].m_uiDataByteOffset + m_uiVertexSize * uiVertex]);
		else
			pData = (float*) &(m_VertexData[m_Attributes[uiArray].m_uiDataByteOffset + uiCompSize * uiVertex]);

		*(pData + 0) = d1;
	}

	void aeVertexArrayDescriptor::setVertexAttribute2f (aeUInt32 uiArray, aeUInt32 uiVertex, float d1, float d2)
	{
		AE_CHECK_DEV (uiArray < m_Attributes.size (), "aeVertexArrayDescriptor::setVertexAttribute2f: Cannot access attribute-array %d, only %d arrays were created.", uiArray, m_Attributes.size ());
		AE_CHECK_DEV (uiVertex < m_uiVertices, "aeVertexArrayDescriptor::setVertexAttribute2f: Cannot set vertex %d, only %d vertices were created.", uiVertex, m_uiVertices);

		const aeUInt32 uiCompSize = m_Attributes[uiArray].m_uiComponentSize;

		float* pData = nullptr;
		if (m_bInterleavedData)
			pData = (float*) &(m_VertexData[m_Attributes[uiArray].m_uiDataByteOffset + m_uiVertexSize * uiVertex]);
		else
			pData = (float*) &(m_VertexData[m_Attributes[uiArray].m_uiDataByteOffset + uiCompSize * uiVertex]);


		*(pData + 0) = d1;
		*(pData + 1) = d2;
	}

	void aeVertexArrayDescriptor::setVertexAttribute3f (aeUInt32 uiArray, aeUInt32 uiVertex, float d1, float d2, float d3)
	{
		AE_CHECK_DEV (uiArray < m_Attributes.size (), "aeVertexArrayDescriptor::setVertexAttribute3f: Cannot access attribute-array %d, only %d arrays were created.", uiArray, m_Attributes.size ());
		AE_CHECK_DEV (uiVertex < m_uiVertices, "aeVertexArrayDescriptor::setVertexAttribute3f: Cannot set vertex %d, only %d vertices were created.", uiVertex, m_uiVertices);

		const aeUInt32 uiCompSize = m_Attributes[uiArray].m_uiComponentSize;

		float* pData = nullptr;
		if (m_bInterleavedData)
			pData = (float*) &(m_VertexData[m_Attributes[uiArray].m_uiDataByteOffset + m_uiVertexSize * uiVertex]);
		else
			pData = (float*) &(m_VertexData[m_Attributes[uiArray].m_uiDataByteOffset + uiCompSize * uiVertex]);

		*(pData + 0) = d1;
		*(pData + 1) = d2;
		*(pData + 2) = d3;
	}

	void aeVertexArrayDescriptor::setVertexAttribute4f (aeUInt32 uiArray, aeUInt32 uiVertex, float d1, float d2, float d3, float d4)
	{
		AE_CHECK_DEV (uiArray < m_Attributes.size (), "aeVertexArrayDescriptor::setVertexAttribute4f: Cannot access attribute-array %d, only %d arrays were created.", uiArray, m_Attributes.size ());
		AE_CHECK_DEV (uiVertex < m_uiVertices, "aeVertexArrayDescriptor::setVertexAttribute4f: Cannot set vertex %d, only %d vertices were created.", uiVertex, m_uiVertices);

		const aeUInt32 uiCompSize = m_Attributes[uiArray].m_uiComponentSize;

		float* pData = nullptr;
		if (m_bInterleavedData)
			pData = (float*) &(m_VertexData[m_Attributes[uiArray].m_uiDataByteOffset + m_uiVertexSize * uiVertex]);
		else
			pData = (float*) &(m_VertexData[m_Attributes[uiArray].m_uiDataByteOffset + uiCompSize * uiVertex]);

		*(pData + 0) = d1;
		*(pData + 1) = d2;
		*(pData + 2) = d3;
		*(pData + 3) = d4;
	}

	void aeVertexArrayDescriptor::setVertexAttribute4b  (aeUInt32 uiArray, aeUInt32 uiVertex, aeInt8  d1, aeInt8  d2, aeInt8  d3, aeInt8  d4)
	{
		AE_CHECK_DEV (uiArray < m_Attributes.size (), "aeVertexArrayDescriptor::setVertexAttribute4b: Cannot access attribute-array %d, only %d arrays were created.", uiArray, m_Attributes.size ());
		AE_CHECK_DEV (uiVertex < m_uiVertices, "aeVertexArrayDescriptor::setVertexAttribute4b: Cannot set vertex %d, only %d vertices were created.", uiVertex, m_uiVertices);

		const aeUInt32 uiCompSize = m_Attributes[uiArray].m_uiComponentSize;

		aeInt8* pData = nullptr;
		if (m_bInterleavedData)
			pData = (aeInt8*) &(m_VertexData[m_Attributes[uiArray].m_uiDataByteOffset + m_uiVertexSize * uiVertex]);
		else
			pData = (aeInt8*) &(m_VertexData[m_Attributes[uiArray].m_uiDataByteOffset + uiCompSize * uiVertex]);

		*(pData + 0) = d1;
		*(pData + 1) = d2;
		*(pData + 2) = d3;
		*(pData + 3) = d4;
	}

	void aeVertexArrayDescriptor::setVertexAttribute4ub (aeUInt32 uiArray, aeUInt32 uiVertex, aeUInt8 d1, aeUInt8 d2, aeUInt8 d3, aeUInt8 d4)
	{
		AE_CHECK_DEV (uiArray < m_Attributes.size (), "aeVertexArrayDescriptor::setVertexAttribute4ub: Cannot access attribute-array %d, only %d arrays were created.", uiArray, m_Attributes.size ());
		AE_CHECK_DEV (uiVertex < m_uiVertices, "aeVertexArrayDescriptor::setVertexAttribute4ub: Cannot set vertex %d, only %d vertices were created.", uiVertex, m_uiVertices);

		const aeUInt32 uiCompSize = m_Attributes[uiArray].m_uiComponentSize;

		aeInt8* pData = nullptr;
		if (m_bInterleavedData)
			pData = (aeInt8*) &(m_VertexData[m_Attributes[uiArray].m_uiDataByteOffset + m_uiVertexSize * uiVertex]);
		else
			pData = (aeInt8*) &(m_VertexData[m_Attributes[uiArray].m_uiDataByteOffset + uiCompSize * uiVertex]);

		*(pData + 0) = d1;
		*(pData + 1) = d2;
		*(pData + 2) = d3;
		*(pData + 3) = d4;
	}

	void aeVertexArrayDescriptor::setTriangle (aeUInt32 uiTriangle, aeUInt32 uiVertex1, aeUInt32 uiVertex2, aeUInt32 uiVertex3)
	{
		AE_CHECK_DEV (uiTriangle < m_uiTriangles, "aeVertexArrayDescriptor::setTriangle: Cannot set triangle %d, only %d were created.", uiTriangle, m_uiTriangles);

		m_Indices[uiTriangle * 3 + 0] = uiVertex1;
		m_Indices[uiTriangle * 3 + 1] = uiVertex2;
		m_Indices[uiTriangle * 3 + 2] = uiVertex3;
	}


}

