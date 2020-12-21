// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#include "../ResourceManager.h"
#include "../ResourceHandle.h"
#include "ResourceManager_Impl.h"
#include <KrautFoundation/Logging/Log.h>
#include <KrautFoundation/FileSystem/FileSystem.h>


namespace AE_NS_FOUNDATION
{
	/*!	From the file-extension this function knows which aeResource to create and load the data into.
	*/
	void aeResourceManager::LoadResourceFile (aeResourceHandle& inout_ResultHandle, const char* szResourceType, const char* szFilename)
	{
		aeCriticalSectionLock scope (&aeResManImpl::m_CS);

		bool bValidAlready = inout_ResultHandle.IsValid ();

		if (!bValidAlready)
		{
			aeStaticString<256> sKey = aeStaticString<256> (szResourceType) + "$$" + aeStaticString<256> (szFilename);

			// first check, whether the file was loaded before
			aeMap<aeStaticString<256>, aeResourceHandle>::iterator it = aeResManImpl::m_LoadedFiles.find (sKey);
			if (it != aeResManImpl::m_LoadedFiles.end ())
			{
				inout_ResultHandle = it.value ();
				return;
			}

			// create the resource, let it load the file
			inout_ResultHandle = aeResManImpl::CreateResourceType (szResourceType);

			// store it as being loaded
			aeResManImpl::m_LoadedFiles[sKey] = inout_ResultHandle;
		}
		else
		{
			inout_ResultHandle.mp_Resource->UnloadResource ();
		}

		if (!aeResManImpl::m_bDelayedResourceLoading)
		{
			aeLog::Dev ("aeResourceManager: Loading %s from \"%s\".", szResourceType, szFilename);

			if ((!aeResManImpl::m_ResourceTypes[szResourceType].m_sFallbackResource.empty ()) &&
        (!aeFileSystem::ExistsFile (szFilename)))
			{
				aeLog::Error ("Resource \"%s\" not available.\nUsing Fallback-Resource \"%s\".", szFilename, aeResManImpl::m_ResourceTypes[szResourceType].m_sFallbackResource.c_str ());

				inout_ResultHandle.mp_Resource->LoadResourceFromFile (aeResManImpl::m_ResourceTypes[szResourceType].m_sFallbackResource.c_str ());
			}
			else
				inout_ResultHandle.mp_Resource->LoadResourceFromFile (szFilename);

			// flag it as available
			inout_ResultHandle.mp_Resource->m_eResourceFlags |= AE_RESOURCE_AVAILABLE;
		}

		// flag it as being restorable (read from a file)
		inout_ResultHandle.mp_Resource->m_eResourceFlags |= AE_RESOURCE_LOADED_FROM_FILE;
	}
}