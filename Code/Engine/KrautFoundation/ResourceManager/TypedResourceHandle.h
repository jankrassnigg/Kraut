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

    AE_INLINE aeTypedResourceHandle (void);
    AE_INLINE aeTypedResourceHandle (const aeResourceHandle& cc);
    AE_INLINE aeTypedResourceHandle (const aeTypedResourceHandle<RES>& cc);
    AE_INLINE explicit aeTypedResourceHandle (aeResource* pResource);

    AE_INLINE void operator= (const aeResourceHandle& cc);
    AE_INLINE void operator= (const aeTypedResourceHandle<RES>& cc);
    AE_INLINE void operator= (aeResource* cc);
    AE_INLINE bool operator== (const aeTypedResourceHandle<RES>& cc) const;
    AE_INLINE bool operator!= (const aeTypedResourceHandle<RES>& cc) const;
    
    //! Returns a pointer to the Resource, this Handle references. Casts it to the desired type.
    AE_INLINE const RES* GetResource (void) const;
    //! Returns a pointer to the Resource, this Handle references. Casts it to the desired type.
    AE_INLINE RES* GetResource (void);

    //! Checks, whether the aeResourceHandle references a valid aeResource.
    AE_INLINE bool IsValid (void) const;
    //! Makes the aeResourceHandle invalid, meaning the aeResource it references is not used any longer.
    AE_INLINE void Invalidate (void);

    //! Saves the Handle to disk.
    //AE_INLINE void Save (aeStreamOut& pFile) const;
    //! Loads the Handle from disk.
    //AE_INLINE void Load (aeStreamIn& pFile);

    AE_INLINE aeResourceHandle GetAsUntypedHandle (void) const;

  private:
    aeResourceHandle m_Handle;
  };
}

#include "Inlines/TypedResourceHandle.inl"

#endif


