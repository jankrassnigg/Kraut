// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#include "../ResourceManager.h"
#include "../ResourceHandle.h"
#include "ResourceManager_Impl.h"
#include <KrautGraphics/Logging/Log.h>



namespace AE_NS_FOUNDATION
{
  void aeResourceManager::RegisterResourceType(const char* szName, AE_FPN_RESOURCE_FACTORY pFactory, AE_FPN_RESOURCE_DESTROYER pDestroyer, const char* szFallbackResource)
  {
    aeCriticalSectionLock scope(&aeResManImpl::m_CS);

    AE_CHECK_ALWAYS(aeResManImpl::m_ResourceTypes.find(szName) == aeResManImpl::m_ResourceTypes.end(), "aeResourceManager::RegisterResourceType: The ResourceType \"%s\" has already been registered.", szName);

    aeResourceType t;
    t.m_pFactory = pFactory;
    t.m_pDestroyer = pDestroyer;
    t.m_sFallbackResource = szFallbackResource;

    aeResManImpl::m_ResourceTypes[szName] = t;

    aeLog::Log("Registered the ResourceType \"%s\".", szName);
  }

  /*!	The Descriptor can be a pointer to any struct, that describes how to create the Resource.\n
  */
  void aeResourceManager::CreateResourceOfType(aeResourceHandle& inout_ResultHandle, const char* szName, void* pDescriptor)
  {
    aeCriticalSectionLock scope(&aeResManImpl::m_CS);

    if (!inout_ResultHandle.IsValid())
      inout_ResultHandle = aeResManImpl::CreateResourceType(szName);
    else
      inout_ResultHandle.mp_Resource->UnloadResource();

    inout_ResultHandle.mp_Resource->CreateResourceFromDescriptor(pDescriptor);

    // flag it as available
    inout_ResultHandle.mp_Resource->m_eResourceFlags |= AE_RESOURCE_AVAILABLE;
  }

  void aeResourceManager::SetDeferredResourceLoading(bool b)
  {
    aeResManImpl::m_bDelayedResourceLoading = b;
  }

  void aeResourceManager::AutoDecreaseRefCount(aeResource* pResource)
  {
    aeResManImpl::m_AutoDecreaseRefCount.push_back(pResource);
  }

  void aeResourceManager::PrepareShutdown(void)
  {
    for (aeUInt32 ui = 0; ui < aeResManImpl::m_AutoDecreaseRefCount.size(); ++ui)
      aeResManImpl::m_AutoDecreaseRefCount[ui]->DecreaseReferenceCount();

    aeResManImpl::m_AutoDecreaseRefCount.clear();
  }

  void aeResourceManager::ProgramShutdown(void)
  {
    {
      aeCriticalSectionLock scope(&aeResManImpl::m_CS);

      aeLog::BeginLogBlock("ResourceManager Shutdown");

      AE_CHECK_DEV(aeResManImpl::m_ResourcesLoaded.empty(), "aeResourceManager::ProgramShutdown: EndReadingResourceFile was not called pairwise with BeginReadingResourceFile.");
      AE_CHECK_DEV(aeResManImpl::m_ResourcesSaved.empty(), "aeResourceManager::ProgramShutdown: EndWritingResourceFile was not called pairwise with BeginWritingResourceFile.");
      AE_CHECK_DEV(!aeResManImpl::m_bWritingAResourceFile, "aeResourceManager::ProgramShutdown: EndWritingResourceFile was not called pairwise with BeginWritingResourceFile.");

      PrepareShutdown();

      // delete all resources (hopefully)
      // do it several times, so that resources, that were held by other resources, will be freed, too
      FreeUnusedResources();
      FreeUnusedResources();
      FreeUnusedResources();
      FreeUnusedResources();
      FreeUnusedResources();


      // log and throw exceptions, if resources were not freed
      bool bError = false;
      RESTYPE_IT itend = aeResManImpl::m_ResourceTypes.end();
      for (RESTYPE_IT it = aeResManImpl::m_ResourceTypes.begin(); it != itend; ++it)
      {
        if (!it.value().m_AllocatedResources.empty())
        {
          aeLog::Error("aeResourceManager::ProgramShutdown: There are %d Resources of Type \"%s\", that have not been freed. Any global (evil!) RESOURCE_HANDLEs referencing them?", it.value().m_AllocatedResources.size(), it.key().c_str());

          for (aeUInt32 i = 0; i < it.value().m_AllocatedResources.size(); ++i)
          {
            aeResourceHandle h(it.value().m_AllocatedResources[i]);
            aeString sFile = GetFileResourceWasLoadedFrom(h);

            aeLog::Error("Resource \"%s\" is still active (RefCount: %d).", sFile.c_str(), (it.value().m_AllocatedResources[i]->GetReferenceCount()));
          }

          bError = true;
        }
      }

      AE_CHECK_ALWAYS(!bError, "aeResourceManager::ProgramShutdown: There were unfreed RESOURCEs. Check your app for global RESOURCE_HANDLEs, that still reference data.");

      aeLog::EndLogBlock();
    }
  }


  /*!	Checks which resources are not referenced anymore and frees that data. It might not free all
    unused resources, if a resource (A) references another resource (B). If so, A will be
    freed, B however not, since there is still a reference to it (from A). However, in a second run, 
    A does not exist anymore, so A's reference to B does not exist anymore either, and B will be freed.
    Therefore, if you want't to make sure, that all unused resources will be freed, call this function
    two or three times, depending on how deep resource-reference-chains can be.
  */
  void aeResourceManager::FreeUnusedResources(void)
  {
    aeLog::BeginLogBlock("Freeing unused Resources");

    aeCriticalSectionLock scope(&aeResManImpl::m_CS);

    // free all loaded files, with refcount == 1
    FILE_IT it2, itend1 = aeResManImpl::m_LoadedFiles.end();



    for (FILE_IT it = aeResManImpl::m_LoadedFiles.begin(); it != itend1;)
    {
      if (it.value().mp_Resource->GetReferenceCount() == 1) // only this Handle exists...
      {
        aeLog::Debug("Removing the unused ResourceFile \"%s\".", it.key().c_str());

        // delete the entry in the map => the last remaining RESOURCE_HANLDE will be deleted
        // => refcount == 0, the resource will be deleted in the next step
        it2 = it;
        ++it2;
        aeResManImpl::m_LoadedFiles.erase(it);
        it = it2;
      }
      else
      {
        ++it;
      }
    }

    // free all resources, with refcount == 0

    int countall = 0;
    aeUInt32 uiStillInUse = 0;

    RESTYPE_IT itend = aeResManImpl::m_ResourceTypes.end();
    for (RESTYPE_IT it = aeResManImpl::m_ResourceTypes.begin(); it != itend; ++it)
    {
      aeDeque<aeResource*>& pVec = it.value().m_AllocatedResources;

      int count = 0;
      int iEnd = (int)pVec.size();
      int inst = 0;
      int iRemaining = 0;

      while (inst < iEnd)
      {
        if (pVec[inst]->GetReferenceCount() == 0)
        {
          --iEnd;

          pVec[inst]->UnloadResource();
          it.value().m_pDestroyer(pVec[inst]);
          pVec[inst] = pVec[pVec.size() - 1];
          pVec.pop_back();

          ++count;

          continue;
        }

        ++iRemaining;
        ++inst;
      }

      countall += count;
      if (count > 0)
        aeLog::Debug("Deleted %d unused Resources of Type \"%s\". %d Resources still in use.", count, it.key().c_str(), iRemaining);

      uiStillInUse += iRemaining;
    }

    if (countall > 0)
      aeLog::Debug("Deleted %d unused Resources. %d Resources still in use.", countall, uiStillInUse);

    aeLog::EndLogBlock();
  }
} // namespace AE_NS_FOUNDATION
