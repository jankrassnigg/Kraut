// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#ifndef AE_CORE_RESOURCEMANAGER_RESOURCEMANAGERIMPL_H
#define AE_CORE_RESOURCEMANAGER_RESOURCEMANAGERIMPL_H

#include "../Declarations.h"
#include "../ResourceHandle.h"

#include <KrautFoundation/Containers/Deque.h>
#include <KrautFoundation/Containers/Map.h>
#include <KrautFoundation/Containers/Stack.h>
#include <KrautGraphics/Containers/Set.h>
#include <KrautGraphics/Strings/PathFunctions.h>
#include <KrautGraphics/Strings/StaticString.h>
#include <KrautGraphics/Threading/CriticalSection.h>

namespace AE_NS_FOUNDATION
{
  struct AE_GRAPHICS_DLL aeResourceType
  {
    //! The Factory to create the resource
    AE_FPN_RESOURCE_FACTORY m_pFactory;
    //! The Destroyer to delete the resource
    AE_FPN_RESOURCE_DESTROYER m_pDestroyer;

    //! All Resources of this type, that have been created.
    aeDeque<aeResource*> m_AllocatedResources;

    aeString m_sFallbackResource;
  };

  typedef aeMap<aeStaticString<32>, aeResourceType>::iterator RESTYPE_IT;

  typedef aeMap<aeStaticString<256>, aeResourceHandle>::iterator FILE_IT;

  struct AE_GRAPHICS_DLL aeResourceLoadState
  {
    bool m_bCopyResources;
    aeMap<aeResourceSaveID, aeResourceHandle> m_SaveIDs;
  };


  struct AE_GRAPHICS_DLL aeResManImpl
  {
  public:
    //! Creates a blank Resource of the given type.
    static const aeResourceHandle CreateResourceType(const char* szName);

    //! all registered resources and types
    static aeMap<aeStaticString<32>, aeResourceType> m_ResourceTypes;

    //! stores which pointers need to be mapped to which current pointers.
    static aeStack<aeResourceLoadState> m_ResourcesLoaded;

    //! The map that knows, which files are already loaded.
    static aeMap<aeStaticString<256>, aeResourceHandle> m_LoadedFiles;

    //! Which resources where already saved, during writing the current file.
    static aeSet<aeResourceSaveID> m_ResourcesSaved;

    //! Whether one is currently writing a ResourceFile.
    static bool m_bWritingAResourceFile;

    //! Whether the resource-handles that are written/read to/from file will be copied or shared.
    static bool m_bStoreFullResources;

    //! If true, resources loaded from file will not be loaded immediately, but at first use. Practical for programs like Editors etc. that do not access certain resources at all.
    static bool m_bDelayedResourceLoading;

    //! The Critical Section used for access to vital data.
    static aeCriticalSection m_CS;


    static aeDeque<aeResource*> m_AutoDecreaseRefCount;
  };
} // namespace AE_NS_FOUNDATION

#endif
