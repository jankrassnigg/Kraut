// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#ifndef AE_CORE_RESOURCEMANAGER_RESOURCEHANDLE_INL
#define AE_CORE_RESOURCEMANAGER_RESOURCEHANDLE_INL

#include "Resource.h"
#include "../ResourceManager.h"

namespace AE_NS_FOUNDATION
{
  template<class RES>
  const RES* aeResourceHandle::GetResource (void) const
  {
    if (!IsValid ())
    {
      AE_CHECK_ALWAYS (false, "aeResourceHandle::GetResource: The ResourceHandle is invalid (\"%s\")", RES::GetResourceTypeNameStatic ());
    }

    const RES* pRes = static_cast<const RES*> (mp_Resource);
    if (!pRes->isResourceAvailable ())
      aeResourceManager::ReloadResource (*this);

    return (pRes);
  }

  template<class RES>
  RES* aeResourceHandle::GetResource (void)
  {
    if (!IsValid ())
    {
      AE_CHECK_ALWAYS (false, "aeResourceHandle::GetResource: The ResourceHandle is invalid (\"%s\")", RES::GetResourceTypeNameStatic ());
    }

    RES* pRes = static_cast<RES*> (mp_Resource);
    if (!pRes->isResourceAvailable ())
      aeResourceManager::ReloadResource (*this);

    return (pRes);
  }

  //AE_INLINE void operator<< (aeStreamOut& pFile, const aeResourceHandle& rh)
  //{
  //	rh.Save (pFile);
  //}

  //AE_INLINE void operator>> (aeStreamIn& pFile, aeResourceHandle& rh)
  //{
  //	rh.Load (pFile);
  //}

  AE_INLINE bool aeResourceHandle::operator!= (const aeResourceHandle& cc) const
  {
    return (mp_Resource != cc.mp_Resource);
  }
  
  AE_INLINE bool aeResourceHandle::operator== (const aeResourceHandle& cc) const
  {
    return (mp_Resource == cc.mp_Resource);
  }

  AE_INLINE bool aeResourceHandle::IsValid (void) const
  {
    return (mp_Resource != NULL);
  }

  AE_INLINE aeResourceHandle::aeResourceHandle (void)
  {
    mp_Resource = NULL;
  }

  AE_INLINE aeResourceHandle::aeResourceHandle (const aeResourceHandle& cc)
  {
    mp_Resource = cc.mp_Resource;

    if (mp_Resource)
      mp_Resource->IncreaseReferenceCount ();
  }

  AE_INLINE aeResourceHandle::aeResourceHandle (aeResource* pResource)
  {
    mp_Resource = pResource;

    if (mp_Resource)
      mp_Resource->IncreaseReferenceCount ();
  }

  AE_INLINE aeResourceHandle::~aeResourceHandle ()
  {
    if (mp_Resource)
      mp_Resource->DecreaseReferenceCount ();
  }

  AE_INLINE void aeResourceHandle::operator= (const aeResourceHandle& cc)
  {
    if (mp_Resource == cc.mp_Resource)
      return;

    if (mp_Resource)
      mp_Resource->DecreaseReferenceCount ();

    mp_Resource = cc.mp_Resource;

    if (mp_Resource)
      mp_Resource->IncreaseReferenceCount ();
  }

  AE_INLINE void aeResourceHandle::operator= (aeResource* cc)
  {
    if (mp_Resource == cc)
      return;

    if (mp_Resource)
      mp_Resource->DecreaseReferenceCount ();

    mp_Resource = cc;

    if (mp_Resource)
      mp_Resource->IncreaseReferenceCount ();
  }

  AE_INLINE void aeResourceHandle::Invalidate (void)
  {
    if (mp_Resource)
      mp_Resource->DecreaseReferenceCount ();

    mp_Resource = NULL;
  }

  //AE_INLINE void aeResourceHandle::Save (aeStreamOut& pFile) const
  //{
  //	aeResourceManager::StoreResourceHandle (pFile, *this);
  //}

  //AE_INLINE void aeResourceHandle::Load (aeStreamIn& pFile)
  //{
  //	(*this) = aeResourceManager::RestoreResourceHandle (pFile);
  //}

  /*! Called by aeResourceManager::->StoreResourceHandle.
  */
  AE_INLINE aeResourceSaveID aeResourceHandle::GetResourceSaveID (void) const
  {
    aeResourceSaveID id = (aeResourceSaveID) mp_Resource;
    return (id);
  }
}

#endif

