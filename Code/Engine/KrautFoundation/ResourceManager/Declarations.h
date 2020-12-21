// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#ifndef AE_CORE_RESOURCEMANAGER_DECLARATIONS_H
#define AE_CORE_RESOURCEMANAGER_DECLARATIONS_H

#include "../Defines.h"

namespace AE_NS_FOUNDATION
{
  class aeResource;
  class aeResourceHandle;
  class aeResourceManager;
  class aeResourceManagerStartup;

  template<class RES>
  class aeTypedResourceHandle;

  //! Type used to store the ID of a Resource in a file.
  typedef aeUInt64 aeResourceSaveID;

  //! aeResource* ResourceCreator (void);
  typedef aeResource* (*AE_FPN_RESOURCE_FACTORY) (void);
  //! void ResourceDestroyer (aeResource* pResourceToDestroy);
  typedef void (*AE_FPN_RESOURCE_DESTROYER) (aeResource*);

  enum aeResourceFlags 
  {
    AE_RESOURCE_LOADED_FROM_FILE  = 1 << 0,
    AE_RESOURCE_AVAILABLE         = 1 << 1,
  };


}

#endif




