// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#include "ResourceManager_Impl.h"
#include "../ResourceHandle.h"


#include <string>

namespace AE_NS_FOUNDATION
{
	aeMap<aeStaticString<32>, aeResourceType> aeResManImpl::m_ResourceTypes;
	aeStack<aeResourceLoadState> aeResManImpl::m_ResourcesLoaded;
	aeMap<aeStaticString<256>, aeResourceHandle> aeResManImpl::m_LoadedFiles;
	aeSet<aeResourceSaveID> aeResManImpl::m_ResourcesSaved;
	bool aeResManImpl::m_bWritingAResourceFile = false;
	bool aeResManImpl::m_bStoreFullResources = true;
	bool aeResManImpl::m_bDelayedResourceLoading = false;
	aeDeque<aeResource*> aeResManImpl::m_AutoDecreaseRefCount;
	aeCriticalSection aeResManImpl::m_CS;

	/*! Creates a resource of the type, the name is given for, and returns a handle to it.
	*/
	const aeResourceHandle aeResManImpl::CreateResourceType (const char* szName)
	{
		std::string sResourceType = szName;

		RESTYPE_IT it = m_ResourceTypes.find (sResourceType.c_str ());
		AE_CHECK_DEV (it != m_ResourceTypes.end (), "aeResourceManager::CreateResourceType: The ResourceType \"%s\" is unknown.", sResourceType.c_str ());

		aeResource* pResource = it.value ().m_pFactory ();

		it.value ().m_AllocatedResources.push_back (pResource);

		return (aeResourceHandle (pResource));
	}
}

