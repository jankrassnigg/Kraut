// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#include "../IncludeAll.h"
#include "ResourceManager_Impl.h"
#include <KrautFoundation/Logging/Log.h>
#include <KrautFoundation/FileSystem/FileSystem.h>



namespace AE_NS_FOUNDATION
{
	const aeFilePath aeResourceManager::GetFileResourceWasLoadedFrom (const aeResourceHandle& hRes)
	{
		if (!hRes.IsValid ())
			return ("");

		const aeResource* pRes = hRes.mp_Resource;

		if (!pRes->isLoadedFromFile ())
			return ("");


		aeCriticalSectionLock scope (&aeResManImpl::m_CS);

		aeMap<aeStaticString<256>, aeResourceHandle>::iterator it, itend;
		itend = aeResManImpl::m_LoadedFiles.end ();

		for (it = aeResManImpl::m_LoadedFiles.begin (); it != itend; ++it)
		{
			if (it.value () == hRes)
			{
        int iMarker = aeStringFunctions::FindFirstStringPos (it.key ().c_str (), "$$") + 2;
				return (&(it.key ().c_str ()[iMarker]));
			}
		}

		AE_CHECK_ALWAYS (false, "aeResourceManager::GetFileResourceWasLoadedFrom: Resource was loaded, but file is unknown.");

    return ("");
	}

	void aeResourceManager::ReplaceLoadedResourceFile (const aeResourceHandle& hRes, const char* szNewFile)
	{
		AE_CHECK_ALWAYS (hRes.IsValid (), "aeResourceManager::ReplaceLoadedResourceFile: This can only be done with valid resource handles (New File: \"%s\").", szNewFile);

		const aeResource* pRes = hRes.mp_Resource;

		aeCriticalSectionLock scope (&aeResManImpl::m_CS);

		aeStaticString<256> sKey = aeStaticString<256> (pRes->GetResourceTypeName ()) + "$$" + aeStaticString<256> (szNewFile);

		AE_CHECK_ALWAYS (pRes->isLoadedFromFile (), "aeResourceManager::ReplaceLoadedResourceFile: The resource needs to be loaded from file, to replace that file.");

		aeMap<aeStaticString<256>, aeResourceHandle>::iterator it, itend;
		itend = aeResManImpl::m_LoadedFiles.end ();
		for (it = aeResManImpl::m_LoadedFiles.begin (); it != itend; ++it)
		{
			if (it.value () == hRes)
			{
				aeResManImpl::m_LoadedFiles.erase (it);

				aeResManImpl::m_LoadedFiles[sKey] = hRes;
				return;
			}
		}

		AE_CHECK_ALWAYS (false, "aeResourceManager::ReplaceLoadedResourceFile: Resource was loaded, but file is unknown.");
	}

	/*! It IS possible to recursively load files, and thus call this function more than once before
		calling the corresponding "EndReadingResourceFile". */
	void aeResourceManager::BeginReadingResourceFile (bool bCopyResources)
	{
		// restrict access to the resource-manager during file-loading to a single thread
		aeResManImpl::m_CS.Enter ();

		aeResourceLoadState state;
		state.m_bCopyResources = bCopyResources;

		aeResManImpl::m_ResourcesLoaded.push (state);
	}

	void aeResourceManager::EndReadingResourceFile (void)
	{
		AE_CHECK_DEV (!aeResManImpl::m_ResourcesLoaded.empty (), "aeResourceManager::EndReadingResourceFile: Called too often!");

		// now load all resources from other files
		LoadAllMissingResources ();

		aeResManImpl::m_ResourcesLoaded.pop ();
		
		aeResManImpl::m_CS.Leave ();
	}

	/*! It is NOT allowed to call this function recursively. You can only write to one file at a time. */
	void aeResourceManager::BeginWritingResourceFile (bool bCopyResources)
	{
		aeResManImpl::m_CS.Enter ();

		AE_CHECK_DEV (!aeResManImpl::m_bWritingAResourceFile, "aeResourceManager::BeginWritingResourceFile: You are already writing a ResourceFile right now.");

		aeResManImpl::m_bStoreFullResources = bCopyResources;
		aeResManImpl::m_bWritingAResourceFile = true;

		aeResManImpl::m_ResourcesSaved.clear ();
	}

	void aeResourceManager::EndWritingResourceFile (void)
	{
		AE_CHECK_DEV (aeResManImpl::m_bWritingAResourceFile, "aeResourceManager::EndWritingResourceFile: You were not writing a ResourceFile at the moment.");
		aeResManImpl::m_bWritingAResourceFile = false;

		aeResManImpl::m_ResourcesSaved.clear ();

		aeResManImpl::m_CS.Leave ();
	}

	/*! Called by aeResourceHandle::Save.\n
		Either stores only a filename, to the file containing the resource, this handle was loaded from,
		or it embeds the resource-data into the file. If the same handle is written twice to a file,
		the second time only a reference is written, instead of the same data again.
	*/
	//void aeResourceManager::StoreResourceHandle (aeStreamOut& pFile, const aeResourceHandle& res)
	//{
	//	AE_CHECK_DEV (aeResManImpl::m_bWritingAResourceFile, "A aeResourceHandle can only be saved to file after calling \"BeginWritingResourceFile\".");

	//	// this is for intra-program copying of pointers (!!), ie. at runtime, so sizeof(void*) will always be equal, when reading it (milliseconds later)
	//	// therefore, don't store this as a aeResourceSaveID here
	//	if (!aeResManImpl::m_bStoreFullResources)
	//	{
	//		aeResource* pRes = res.mp_Resource;
	//		pFile.Write (&pRes, sizeof (aeResource*));
	//		return;
	//	}

	//	
	//	aeResourceSaveID id = res.GetResourceSaveID ();
	//	pFile << id;

	//	if (!res.IsValid ())
	//		return;

	//	// if the handle has been written to this file before, don't write more information again
	//	if (aeResManImpl::m_ResourcesSaved.find (id) != aeResManImpl::m_ResourcesSaved.end ())
	//		return;

	//	// if not, write the information necessary to restore the resource, that this handle references
	//	aeResManImpl::m_ResourcesSaved.insert (id);
	//	const aeResource* pRes = res.mp_Resource;

	//	if (pRes->isLoadedFromFile ())
	//	{
	//		bool b = true;
	//		pFile << b;

	//		pFile << pRes->GetResourceTypeName ();


	//		// write the filename of the resource to this file

	//		map<aeStaticString<256>, aeResourceHandle>::iterator it, itend;
	//		itend = aeResManImpl::m_LoadedFiles.end ();
	//		for (it = aeResManImpl::m_LoadedFiles.begin (); it != itend; ++it)
	//		{
	//			if (it->second == res)
	//			{
	//				aeFilePath sRelative;

	//				//! \todo hier string bearbeiten, statt funktion aufzurufen
	//				aeStaticString<256> sFile = GetFileResourceWasLoadedFrom (res);

	//				AE_CHECK_ALWAYS (aeFileSystem::GetFolderRelativePath ("$CONTENT$", sFile.c_str (), sRelative), "aeResourceManager::StoreResourceHandle: Could not convert the path \"%s\" into a relative path to the $CONTENT$-folder (\"%s\").", sFile.c_str (), aeFileSystem::CreateValidPath ("$CONTENT$", false).c_str ());
	//				sRelative = "$CONTENT$/" + sRelative;

	//				pFile << sRelative;
	//				return;
	//			}
	//		}

	//		AE_CHECK_ALWAYS (false, "aeResourceManager::StoreResourceHandle: For a restoreable aeResourceHandle the file is unknown.");
	//	}
	//	else
	//	{
	//		bool b = false;
	//		pFile << b;

	//		// embed the resource itself in this file

	//		//! \todo write position in the other stream

	//		pFile << pRes->GetResourceTypeName ();
	//		pRes->WriteResourceToFile (pFile);
	//	}
	//}

	///*! Called by aeResourceHandle::Load
	//*/
	//aeResourceHandle aeResourceManager::RestoreResourceHandle (aeStreamIn& pFile)
	//{
	//	AE_CHECK_DEV (!aeResManImpl::m_ResourcesLoaded.empty (), "A aeResourceHandle can only be loaded from file after calling \"BeginReadingResourceFile\".");

	//	// read the pointer directly, this comes usually not from a file, but a memory buffer, when copying objects in an editor or ingame
	//	if (!aeResManImpl::m_ResourcesLoaded.top ().m_bCopyResources)
	//	{
	//		aeResource* pRes;
	//		pFile.Read (&pRes, sizeof (aeResource*));
	//		return (aeResourceHandle (pRes));
	//	}

	//	aeResourceSaveID id;
	//	pFile >> id;

	//	// if an invalid resource-handle was stored, restore it as invalid
	//	if (id == 0)
	//		return (aeResourceHandle ());

	//	map<aeResourceSaveID, aeResourceHandle>::iterator it;
	//	it = aeResManImpl::m_ResourcesLoaded.top ().m_SaveIDs.find (id);

	//	// if this handle was not read before, from this file
	//	if (it == aeResManImpl::m_ResourcesLoaded.top ().m_SaveIDs.end ())
	//	{
	//		aeResourceHandle hRes;

	//		bool bRestoreable;
	//		pFile >> bRestoreable;

	//		if (bRestoreable)
	//		{
	//			// the resource needs to be read from a file

	//			aeString sType;
	//			pFile >> sType;

	//			aeString sFilename;
	//			pFile >> sFilename;

	//			LoadResourceFile (hRes, sType.c_str (), sFilename.c_str ());
	//		}
	//		else
	//		{
	//			// the resource is embedded into this file

	//			//! \todo read position in the other stream

	//			aeString sType;
	//			pFile >> sType;

	//			hRes = aeResManImpl::CreateResourceType (sType.c_str ());
	//			hRes.mp_Resource->ReadResourceFromFile (pFile);
	//			hRes.mp_Resource->m_eResourceFlags |= AE_RESOURCE_AVAILABLE;
	//		}

	//		aeResManImpl::m_ResourcesLoaded.top ().m_SaveIDs[id] = hRes;
	//		return (hRes);
	//	}

	//	// else return the already known handle
	//	return (it->second);
	//}

	void aeResourceManager::ReloadResource (const aeResourceHandle& Handle)
	{
		aeCriticalSectionLock scope (&aeResManImpl::m_CS);

		aeResource* pRes = Handle.mp_Resource;

		aeStaticString<256> sFile = GetFileResourceWasLoadedFrom (Handle);

		aeLog::Dev ("Loading Resource \"%s\"", sFile.c_str ());

		if ((!aeResManImpl::m_ResourceTypes[pRes->GetResourceTypeName ()].m_sFallbackResource.empty ()) &&
      (!aeFileSystem::ExistsFile (sFile.c_str ())))
		{
			aeLog::Error ("Resource \"%s\" not available.\nUsing Fallback-Resource \"%s\".", sFile.c_str (), aeResManImpl::m_ResourceTypes[pRes->GetResourceTypeName ()].m_sFallbackResource.c_str ());

			sFile = aeResManImpl::m_ResourceTypes[pRes->GetResourceTypeName ()].m_sFallbackResource;
		}

		pRes->LoadResourceFromFile (sFile.c_str ());
		pRes->m_eResourceFlags |= AE_RESOURCE_AVAILABLE;
	}

	void aeResourceManager::ReloadResourcesOfType (const char* szTypeName)
	{
		aeCriticalSectionLock scope (&aeResManImpl::m_CS);
		{
			AE_LOG_BLOCK ("Reloading Resources");

			aeLog::Dev ("ResourceType to reload: \"%s\"", szTypeName);

			for (aeUInt32 i = 0; i < aeResManImpl::m_ResourceTypes[szTypeName].m_AllocatedResources.size(); ++i)
			{
        auto ref = aeResManImpl::m_ResourceTypes[szTypeName].m_AllocatedResources[i];

				if ((ref->isLoadedFromFile ()) && (!ref->isResourceUpToDate ()))
				{
          ref->UnloadResource ();
          ref->m_eResourceFlags &= ~AE_RESOURCE_AVAILABLE;
					
					aeResourceHandle Handle (ref);

					ReloadResource (Handle);
				}
			}
		}
	}

	void aeResourceManager::LoadAllMissingResources (void)
	{
		if (aeResManImpl::m_bDelayedResourceLoading)
			return;

		aeCriticalSectionLock scope (&aeResManImpl::m_CS);
		{

			AE_LOG_BLOCK ("Loading missing Resources");

			aeMap<aeStaticString<256>, aeResourceHandle>::iterator it, itend;
			itend = aeResManImpl::m_LoadedFiles.end ();


			//! \todo This function has a bug, it usually crashes the app at startup.

			// PROBLEM: Resources that are loaded, might insert other resources to be loaded -> map iterators become invalid -> crash in loop

			//for (it = aeResManImpl::m_LoadedFiles.begin (); it != itend; ++it)
			//{
			//	if (!it->second.mp_Resource->isResourceAvailable ())
			//		ReloadResource (it->second);
			//}

		}
	}

}






