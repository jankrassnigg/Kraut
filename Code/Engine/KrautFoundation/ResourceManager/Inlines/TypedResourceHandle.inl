// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#ifndef AE_CORE_RESOURCEMANAGER_TYPEDRESOURCEHANDLE_INL
#define AE_CORE_RESOURCEMANAGER_TYPEDRESOURCEHANDLE_INL

namespace AE_NS_FOUNDATION
{
  template<class RES>
  AE_INLINE aeTypedResourceHandle<RES>::aeTypedResourceHandle () : m_Handle ()
  {
  }

  template<class RES>
  AE_INLINE aeTypedResourceHandle<RES>::aeTypedResourceHandle (const aeResourceHandle& cc) : m_Handle (cc)
  {
  }

  template<class RES>
  AE_INLINE aeTypedResourceHandle<RES>::aeTypedResourceHandle (const aeTypedResourceHandle<RES>& cc) : m_Handle (cc.m_Handle)
  {
  }

  template<class RES>
  AE_INLINE aeTypedResourceHandle<RES>::aeTypedResourceHandle (aeResource* pResource) : m_Handle (pResource)
  {
  }

  template<class RES>
  AE_INLINE void aeTypedResourceHandle<RES>::operator= (const aeResourceHandle& cc)
  {
    m_Handle.operator= (cc);
  }

  template<class RES>
  AE_INLINE void aeTypedResourceHandle<RES>::operator= (const aeTypedResourceHandle<RES>& cc)
  {
    m_Handle.operator= (cc.m_Handle);
  }

  template<class RES>
  AE_INLINE void aeTypedResourceHandle<RES>::operator= (aeResource* cc)
  {
    m_Handle.operator= (cc);
  }

  template<class RES>
  AE_INLINE bool aeTypedResourceHandle<RES>::operator== (const aeTypedResourceHandle<RES>& cc) const
  {
    return (m_Handle.operator== (cc.m_Handle));
  }

  template<class RES>
  AE_INLINE bool aeTypedResourceHandle<RES>::operator!= (const aeTypedResourceHandle<RES>& cc) const
  {
    return (m_Handle.operator!= (cc.m_Handle));
  }

  template<class RES>
  AE_INLINE const RES* aeTypedResourceHandle<RES>::GetResource (void) const
  {
    return (m_Handle.GetResource<RES> ());
  }

  template<class RES>
  AE_INLINE RES* aeTypedResourceHandle<RES>::GetResource (void)
  {
    return (m_Handle.GetResource<RES> ());
  }

  template<class RES>
  AE_INLINE bool aeTypedResourceHandle<RES>::IsValid (void) const
  {
    return (m_Handle.IsValid ());
  }

  template<class RES>
  AE_INLINE void aeTypedResourceHandle<RES>::Invalidate (void)
  {
    m_Handle.Invalidate ();
  }

  //template<class RES>
  //AE_INLINE void aeTypedResourceHandle<RES>::Save (aeStreamOut& pFile) const
  //{
  //  m_Handle.Save (pFile);
  //}

  //template<class RES>
  //AE_INLINE void aeTypedResourceHandle<RES>::Load (aeStreamIn& pFile)
  //{
  //  m_Handle.Load (pFile);
  //}

  template<class RES>
  AE_INLINE aeResourceHandle aeTypedResourceHandle<RES>::GetAsUntypedHandle (void) const
  {
    return (m_Handle);
  }

  //template<class RES>
  //AE_INLINE void operator<< (aeStreamOut& pFile, const aeTypedResourceHandle<RES>& rh)
  //{
  //  rh.Save (pFile);
  //}

  //template<class RES>
  //AE_INLINE void operator>> (aeStreamIn& pFile, aeTypedResourceHandle<RES>& rh)
  //{
  //  rh.Load (pFile);
  //}

}

#endif

