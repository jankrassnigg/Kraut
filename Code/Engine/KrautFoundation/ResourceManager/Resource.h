// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#ifndef AE_CORE_RESOURCEMANAGER_RESOURCE_H
#define AE_CORE_RESOURCEMANAGER_RESOURCE_H

#include "Declarations.h"
#include <KrautFoundation/Streams/Streams.h>

namespace AE_NS_FOUNDATION
{
  /*!	This is the base-class for all Resources to be used by the engine. It provides a sophisticated
    mechanisms to handle resourcemanagement as much behind the scenes, as possible.\n
    DON'T derive your Resource directly from this class, but instead use the aeResourceTemplate.\n
    Using the template nearly all the code-overhead will be auto-generated. The only thing left to do
    is to call the auto-generated functions "aeResourceTemplate<custom>::RegisterResourceType" 
    to complete integration into the system.\n
    RESOURCEs are reference counted, which is done automatically by the aeResourceHandle class.
    \note NEVER reference a aeResource directly through a pointer, but instead use a aeResourceHandle.
    Use the aeResourceHandle to Get access to the aeResource, and to make sure reference-counting is
    used correctly.\n
    RESOURCEs are automatically destroyed, when there are no RESOURCE_HANDLEs referencing them
    anymore.\n
    To make a resource be loadable from a file (ie. a Texture, Shader, Object, ...), provide the
    virtual function "LoadResourceFromFile".\n
    To make a resource be createable from the application, using a descriptor-struct (if necessary),
    provide the virtual function "CreateResourceFromDescriptor". In this case, if your resource is
    supposed to be embedable into a savegame/file (ie. a Mesh in a level-file), additionally provide
    the virtual functions "WriteResourceToFile" and "ReadResourceFromFile". The aeResourceManager
    takes care of calling those functions, when a aeResourceHandle is written to a file, which
    references a aeResource, that needs to be embedded into the file.
  */
  class AE_FOUNDATION_DLL aeResource
  {
  public:
    aeResource (void);
    virtual ~aeResource ();

    //! Returns the number of aeResourceHandles that hold a reference to this resource.
    aeInt32 GetReferenceCount (void) const {return (m_iReferenceCount);}

  protected:
    //! Used by the aeResourceTemplate to Get access to the private register-functions of the Manager.
    static void RegisterResourceToManager (const char* szResourceTypeName, AE_FPN_RESOURCE_FACTORY pFactory, AE_FPN_RESOURCE_DESTROYER pDestroyer, const char* szFallbackResource);

    static void LoadResourceFile (aeResourceHandle& inout_ResultHandle, const char* szResourceType, const char* szFilename);

    static void CreateResourceFromDesc (aeResourceHandle& inout_ResultHandle, const char* szResourceType, void* pResourceDescriptor);

    static void ReloadResourcesOfType (const char* szTypeName);

  private:
    friend class aeResourceHandle;

    //! Called by the aeResourceHandle.
    void IncreaseReferenceCount (void);
    //! Called by the aeResourceHandle.
    void DecreaseReferenceCount (void);

  private:
    friend class aeResourceManager;

    //! Returns true, if the resource is fully loaded, or false, if it is thus far only a place-holder.
    bool isResourceAvailable (void) const {return ((m_eResourceFlags & AE_RESOURCE_AVAILABLE) != 0);}
    //! Whether this aeResource was loaded from a file and can thus be restored automatically.
    bool isLoadedFromFile (void) const {return ((m_eResourceFlags & AE_RESOURCE_LOADED_FROM_FILE) != 0);}

    //! Needs to return EXACTLY the name, that the Resource is registered under.
    virtual const char* GetResourceTypeName (void) const = 0;

    //! Overwrite this, if the resource should be loadable from a file.
    virtual void LoadResourceFromFile (const char* szFilename);
    //! Overwrite this, if the resource should be createable using a descriptor-struct.
    virtual void CreateResourceFromDescriptor (void* pDescriptor);

    //! Overwrite this, if the resource should be embeddable into a file (scenegraph...)
    virtual void WriteResourceToFile (aeStreamOut& File) const;
    //! Overwrite this, if the resource should be embeddable into a file (scenegraph...)
    virtual void ReadResourceFromFile (aeStreamIn& File);

  private:

    //! Overwrite this to unload a resource. Will be called before destroying a resource, and when a resources should be reloaded.
    virtual void UnloadResource (void) = 0;
    //! Overwrite this to implement a custom determination, whether a resource needs to be reloaded, or can be kept as is.
    virtual bool isResourceUpToDate (void) {return (false);}

  private:
    //! Every Resource is (automatically) reference counted, through the aeResourceHandles.
    aeInt16 m_iReferenceCount;

    //! Whether this resource was loaded from a file.
    aeUInt32 m_eResourceFlags;

  private:
    aeResource (const aeResource& cc);
    const aeResource& operator= (const aeResource& cc);
  };
}

#include "Inlines/ResourceMacros.inl"

#endif
