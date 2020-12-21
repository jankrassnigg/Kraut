// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#include "../IncludeAll.h"



namespace AE_NS_FOUNDATION
{
	aeResource::aeResource () : m_iReferenceCount (0), m_eResourceFlags (0)
	{
	}

	aeResource::~aeResource ()
	{
	}

	void aeResource::IncreaseReferenceCount (void)
	{
		#pragma omp atomic
		++m_iReferenceCount;
	}

	void aeResource::DecreaseReferenceCount (void)
	{
		#pragma omp atomic
		--m_iReferenceCount;
	}

	void aeResource::RegisterResourceToManager (const char* szResourceTypeName, AE_FPN_RESOURCE_FACTORY pFactory, AE_FPN_RESOURCE_DESTROYER pDestroyer, const char* szFallbackResource)
	{
		aeResourceManager::RegisterResourceType (szResourceTypeName, pFactory, pDestroyer, szFallbackResource);
	}

	void aeResource::WriteResourceToFile (aeStreamOut& pFile) const
	{
    AE_CHECK_ALWAYS (false, "aeResource::WriteResourceToFile: This virtual function was not provided by the ResourceType \"%s\".", GetResourceTypeName ());
	}

	void aeResource::ReadResourceFromFile (aeStreamIn& pFile)
	{
    AE_CHECK_ALWAYS (false, "aeResource::ReadResourceFromFile: This virtual function was not provided by the ResourceType \"%s\".", GetResourceTypeName ());
	}

	/*! \note It is not checked, whether the given File exists. LoadResourceFromFile needs to check this
		itself. This way it might actually load the data from another file, if the desired file does not
		exists. For example, if the *.mesh file does not exist, it could try to load the Mesh from the
		corresponding *.dae (Collada) file and then create the *.mesh file afterwards.
	*/
	void aeResource::LoadResourceFromFile (const char* szFilename)
	{
    AE_CHECK_ALWAYS (false, "aeResource::LoadResourceFromFile: This virtual function was not provided by the ResourceType \"%s\".", GetResourceTypeName ());
	}

	/*! \note The Descriptor might be NULL. The function is completely responsible to know what kind of
		descriptor it Gets and how to use it. */
	void aeResource::CreateResourceFromDescriptor (void* pDescriptor)
	{
    AE_CHECK_ALWAYS (false, "aeResource::CreateResourceFromDescriptor: This virtual function was not provided by the ResourceType \"%s\".", GetResourceTypeName ());
	}

	void aeResource::LoadResourceFile (aeResourceHandle& inout_ResultHandle, const char* szResourceType, const char* szFilename)
	{
		aeResourceManager::LoadResourceFile (inout_ResultHandle, szResourceType, szFilename);
	}

	void aeResource::CreateResourceFromDesc (aeResourceHandle& inout_ResultHandle, const char* szResourceType, void* pResourceDescriptor)
	{
		return (aeResourceManager::CreateResourceOfType (inout_ResultHandle, szResourceType, pResourceDescriptor));
	}

	void aeResource::ReloadResourcesOfType (const char* szTypeName)
	{
		aeResourceManager::ReloadResourcesOfType (szTypeName);
	}
}