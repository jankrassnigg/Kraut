#include "VertexArrayResource.h"



namespace AE_NS_GRAPHICS
{
	bool aeVertexArrayResource::s_bIgnoreMaterialChange = false;

	aeVertexArrayResource::aeVertexArrayResource ()
	{
		m_MaterialChangeCallback = NULL;
		m_pMaterialChangePassThrough = NULL;

		m_uiNumberOfVertices = 0;
		m_uiNumberOfTriangles = 0;
		m_uiNumberOfIndices = 0;
	}

	void aeVertexArrayResource::EnableMaterialChangeCallback (bool bEnable)
	{
		s_bIgnoreMaterialChange = !bEnable;
	}

	void aeVertexArrayResource::setMaterialChangeCallback (AE_MATERIAL_CHANGE_CALLBACK cb, void* pPassThrough)
	{
		m_MaterialChangeCallback = cb;
		m_pMaterialChangePassThrough = pPassThrough;
	}

	aeUInt32 aeVertexArrayResource::getNumberOfVertices (void) const
	{
		return (m_uiNumberOfVertices);
	}

	aeUInt32 aeVertexArrayResource::getNumberOfTriangles (void) const
	{
		return (m_uiNumberOfTriangles);
	}

	aeUInt32 aeVertexArrayResource::getNumberOfIndices (void) const
	{
		return (m_uiNumberOfIndices);
	}
}

