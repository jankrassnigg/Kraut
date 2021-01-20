// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#ifndef AE_CORE_RESOURCEMANAGER_RESOURCEMACROS_INL
#define AE_CORE_RESOURCEMANAGER_RESOURCEMACROS_INL

#include <KrautGraphics/Communication/GlobalEvent.h>
#include <KrautGraphics/Strings/StaticString.h>

/*! The aeResourceTemplate is a Helper-sandwich-class, which implements type-specific functions,
  that are necessary for the aeResourceManager to work with a aeResource. It is a template, which
  allows the compiler to auto-generate these type-specific functions, removing this responsibility
  from the programmer. When creating a resource-type, derive from this class, instead of directly
  from aeResource. At application-startup you still need to call "RegisterResourceType" once for 
  every resource-type. But that's about it.\n
  ResourceType-Names are auto-generated from c++ class-names, so don't change the class-name later,
  if save games (files with embedded resources) need to stay compatible.\n
  To correctly derive from aeResourceTemplate, provide the name of your new resource as the template
  parameter:\n
  class MY_RESOURCE : public aeResourceTemplate<MY_RESOURCE> {...};\n
  \n
  For further information, how to program a aeResource-type, check out the documentation of aeResource.
*/

#define AE_RESOURCE_DECLARATION(CLASS)                                                                                                       \
                                                                                                                                             \
public:                                                                                                                                      \
  static aeTypedResourceHandle<CLASS> LoadResource(const char* szFile);                                                                      \
                                                                                                                                             \
  static void LoadIntoResource(aeTypedResourceHandle<CLASS>& inout_ResultHandle, const char* szFile);                                        \
                                                                                                                                             \
  /*! Creates a Resource of this type*/                                                                                                      \
  static void CreateResource(aeTypedResourceHandle<CLASS>& inout_ResultHandle, void* pResourceDescriptor, bool bOverwriteIfExisting = true); \
                                                                                                                                             \
  /*! Call this to register this aeResource-type to the system. */                                                                           \
  static void RegisterResourceType(const char* szFallbackResource = "");                                                                     \
                                                                                                                                             \
  /*! Unloads unused resources and checks that all resources of this type are really freed. */                                               \
  static void CheckResourcesOfTypeAreUnloaded(void);                                                                                         \
                                                                                                                                             \
  /*! Tells the ResourceManager to reload all resources of this type. */                                                                     \
  static void ReloadResourceType(void) { ReloadResourcesOfType(s_ResourceTypeName.c_str()); }                                                \
                                                                                                                                             \
  /*! returns the auto-generated class-name of this resource-type */                                                                         \
  inline static const char* GetResourceTypeNameStatic(void) { return s_ResourceTypeName.c_str(); }                                           \
                                                                                                                                             \
private:                                                                                                                                     \
  /*! returns the auto-generated class-name of this resource-type */                                                                         \
  virtual const char* GetResourceTypeName(void) const { return s_ResourceTypeName.c_str(); }                                                 \
                                                                                                                                             \
  static aeResource* ResourceCreator(void);                                                                                                  \
                                                                                                                                             \
  static void ResourceDestroyer(aeResource*);                                                                                                \
                                                                                                                                             \
  static aeStaticString<32> s_ResourceTypeName;


#define AE_RESOURCE_IMPLEMENTATION(CLASS, FALLBACK_RESOURCE_FILE)                                                                                                                                     \
                                                                                                                                                                                                      \
  aeStaticString<32> CLASS::s_ResourceTypeName(#CLASS);                                                                                                                                               \
                                                                                                                                                                                                      \
  aeResource* CLASS::ResourceCreator()                                                                                                                                                                \
  {                                                                                                                                                                                                   \
    return new CLASS();                                                                                                                                                                               \
  }                                                                                                                                                                                                   \
                                                                                                                                                                                                      \
  void CLASS::ResourceDestroyer(aeResource* pResource)                                                                                                                                                \
  {                                                                                                                                                                                                   \
    delete pResource;                                                                                                                                                                                 \
  }                                                                                                                                                                                                   \
                                                                                                                                                                                                      \
  void CLASS::RegisterResourceType(const char* szFallbackResource)                                                                                                                                    \
  {                                                                                                                                                                                                   \
    RegisterResourceToManager(s_ResourceTypeName.c_str(), ResourceCreator, ResourceDestroyer, szFallbackResource);                                                                                    \
  }                                                                                                                                                                                                   \
                                                                                                                                                                                                      \
  aeTypedResourceHandle<CLASS> CLASS::LoadResource(const char* szFile)                                                                                                                                \
  {                                                                                                                                                                                                   \
    aeResourceHandle hTemp;                                                                                                                                                                           \
    LoadResourceFile(hTemp, s_ResourceTypeName.c_str(), szFile);                                                                                                                                      \
    return (aeTypedResourceHandle<CLASS>(hTemp));                                                                                                                                                     \
  }                                                                                                                                                                                                   \
                                                                                                                                                                                                      \
  void CLASS::LoadIntoResource(aeTypedResourceHandle<CLASS>& inout_ResultHandle, const char* szFile)                                                                                                  \
  {                                                                                                                                                                                                   \
    aeResourceHandle hTemp = inout_ResultHandle.GetAsUntypedHandle();                                                                                                                                 \
    LoadResourceFile(hTemp, s_ResourceTypeName.c_str(), szFile);                                                                                                                                      \
    inout_ResultHandle = hTemp;                                                                                                                                                                       \
  }                                                                                                                                                                                                   \
                                                                                                                                                                                                      \
  /* If bOverwriteIfExisting and inout_ResultHandle points to an existing resource, that resource will be overwritten, */                                                                             \
  /*  meaning all handles pointing to it will stay valid, but its content will change. */                                                                                                             \
  /* If not bOverwriteIfExisting, a new resource is created and a handle to it is stored in inout_ResultHandle, independent of whether that handle was */                                             \
  /* pointing to a valid resource or not. That means all other handles pointing to that resource will still point to the same, unchanged resource. */                                                 \
  void CLASS::CreateResource(aeTypedResourceHandle<CLASS>& inout_ResultHandle, void* pResourceDescriptor, bool bOverwriteIfExisting)                                                                  \
  {                                                                                                                                                                                                   \
    aeResourceHandle hTemp;                                                                                                                                                                           \
                                                                                                                                                                                                      \
    if (bOverwriteIfExisting)                                                                                                                                                                         \
      hTemp = inout_ResultHandle.GetAsUntypedHandle();                                                                                                                                                \
                                                                                                                                                                                                      \
    CreateResourceFromDesc(hTemp, s_ResourceTypeName.c_str(), pResourceDescriptor);                                                                                                                   \
    inout_ResultHandle = hTemp;                                                                                                                                                                       \
  }                                                                                                                                                                                                   \
                                                                                                                                                                                                      \
  void CLASS::CheckResourcesOfTypeAreUnloaded(void)                                                                                                                                                   \
  {                                                                                                                                                                                                   \
    aeResourceManager::PrepareShutdown();                                                                                                                                                             \
    aeResourceManager::FreeUnusedResources();                                                                                                                                                         \
    aeResourceManager::FreeUnusedResources();                                                                                                                                                         \
    aeResourceManager::FreeUnusedResources();                                                                                                                                                         \
                                                                                                                                                                                                      \
    /*AE_CHECK_ALWAYS (GetResourcesAllocatedOfType () == 0, "There are still %d Resources of Type \"%s\" when all should be freed.", GetResourcesAllocatedOfType (), GetResourceTypeNameStatic ());*/ \
  }                                                                                                                                                                                                   \
                                                                                                                                                                                                      \
  AE_ON_GLOBAL_EVENT_ONCE(aeStartup_StartupCore_Begin)                                                                                                                                                \
  {                                                                                                                                                                                                   \
    CLASS::RegisterResourceType(FALLBACK_RESOURCE_FILE);                                                                                                                                              \
  }


#endif
