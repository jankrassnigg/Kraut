// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#ifndef AE_CORE_RESOURCEMANAGER_TYPEDRESOURCEHANDLE_H
#define AE_CORE_RESOURCEMANAGER_TYPEDRESOURCEHANDLE_H

#include "ResourceHandle.h"

namespace AE_NS_FOUNDATION
{
  template<class RES>
  class aeTypedResourceHandle
  {
  public:

    inline aeTypedResourceHandle (void);
    inline aeTypedResourceHandle (const aeResourceHandle& cc);
    inline aeTypedResourceHandle (const aeTypedResourceHandle<RES>& cc);
    inline explicit aeTypedResourceHandle (aeResource* pResource);

    inline void operator= (const aeResourceHandle& cc);
    inline void operator= (const aeTypedResourceHandle<RES>& cc);
    inline void operator= (aeResource* cc);
    inline bool operator== (const aeTypedResourceHandle<RES>& cc) const;
    inline bool operator!= (const aeTypedResourceHandle<RES>& cc) const;
    
    //! Returns a pointer to the Resource, this Handle references. Casts it to the desired type.
    inline const RES* GetResource (void) const;
    //! Returns a pointer to the Resource, this Handle references. Casts it to the desired type.
    inline RES* GetResource (void);

    //! Checks, whether the aeResourceHandle references a valid aeResource.
    inline bool IsValid (void) const;
    //! Makes the aeResourceHandle invalid, meaning the aeResource it references is not used any longer.
    inline void Invalidate (void);

    //! Saves the Handle to disk.
    //inline void Save (aeStreamOut& pFile) const;
    //! Loads the Handle from disk.
    //inline void Load (aeStreamIn& pFile);

    inline aeResourceHandle GetAsUntypedHandle (void) const;

  private:
    aeResourceHandle m_Handle;
  };
}

#include "Inlines/TypedResourceHandle.inl"

#endif


