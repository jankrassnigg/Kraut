// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#ifndef AE_CORE_RESOURCEMANAGER_RESOURCEMANAGER_H
#define AE_CORE_RESOURCEMANAGER_RESOURCEMANAGER_H

#include "Declarations.h"
#include <KrautFoundation/Strings/Declarations.h>
#include <KrautFoundation/Strings/PathFunctions.h>

namespace AE_NS_FOUNDATION
{
	/*!	The one-and-only global Manager for all Resources used inside the Engine.\n
		Call "CreateResource" to create a aeResource-type. Call "LoadResourceFile" to load a resource-file.
		Most of the initialization-code is inside the aeResource and the aeResourceTemplate.
	*/
	class AE_FOUNDATION_DLL aeResourceManager
	{
	public:
		//! Deletes all Resources, that are currently not referenced by any aeResourceHandle.
		static void FreeUnusedResources (void);

		//! If true, resources from files will be loaded at first use, not immediately.
		static void SetDeferredResourceLoading (bool b);

		//! The specified resource will Get one ref-count decrease on shutdown for each such registration -> allows global resource-handles that do not need to be invalidated.
		static void AutoDecreaseRefCount (aeResource* pResource);

		static void PrepareShutdown (void);

	private:
		//! Deletes all still allocated Resources, throws Exceptions, if data is still referenced.
		static void ProgramShutdown (void);

		//! Creates a aeResource of the given Type.
		static void CreateResourceOfType (aeResourceHandle& inout_ResultHandle, const char* szName, void* pDescriptor);
		//! Uses a Loader, depending on the Files Extension, to load it and returns a aeResourceHandle to it.
		static void LoadResourceFile (aeResourceHandle& inout_ResultHandle, const char* szResourceType, const char* szFilename);

	public:
		//! Returns the path to the file, the resource was loaded from. "" if it wasn't loaded from any file.
		static const aeFilePath GetFileResourceWasLoadedFrom (const aeResourceHandle& hRes);
		//! The resource with the given handle will be loaded from the given file, instead of the currently registered one.
		static void ReplaceLoadedResourceFile (const aeResourceHandle& hRes, const char* szNewFile);

	private:
		//! Loads all resources, that are known to the Manager, but not available right now (for example due to reloading).
		static void LoadAllMissingResources (void);

	public:

		//! Call this just before reading a file, that stores RESOURCEs and RESOURCE_HANDLEs.
		static void BeginReadingResourceFile (bool bCopyResources);
		//! Call this after reading a file, that stores RESOURCEs and RESOURCE_HANDLEs.
		static void EndReadingResourceFile (void);

		//! Call this just before writing a file, that stores RESOURCEs and RESOURCE_HANDLEs.
		static void BeginWritingResourceFile (bool bCopyResources);
		//! Call this after writing a file, that stores RESOURCEs and RESOURCE_HANDLEs.
		static void EndWritingResourceFile (void);


	private:

		//! Registers a aeResource-type by a name.
		static void RegisterResourceType (const char* szResourceTypeName, AE_FPN_RESOURCE_FACTORY pFactory, AE_FPN_RESOURCE_DESTROYER pDestroyer, const char* szFallbackResource);

		//! Makes sure all resources of this type are up-to-date or will be reloaded.
		static void ReloadResourcesOfType (const char* szTypeName);

	private:
		friend class aeResource;
		friend class aeResourceHandle;
		friend class aeResourceManagerStartup;

		//! Writes a Handle and all necessary Resourcedata to the file.
		//static void StoreResourceHandle (aeStreamOut& pFile, const aeResourceHandle& hResource);
		//! Restores a Handle and all necessary Resourcedata from the file.
		//static aeResourceHandle RestoreResourceHandle (aeStreamIn& pFile);

		//! Reloads the resource pointed to by the handle.
		static void ReloadResource (const aeResourceHandle& Handle);

	};


}

#endif



