#ifndef AE_FOUNDATION_STRINGS_CACHEDSTRING_INL
#define AE_FOUNDATION_STRINGS_CACHEDSTRING_INL

namespace AE_NS_FOUNDATION
{
  AE_INLINE aeCachedString::~aeCachedString ()
  {
    // reduce the reference count of the currently referenced string
    --m_Data.value ();
  }

  AE_INLINE void aeCachedString::operator= (char c)
  {
    char s[2];
    s[0] = c;
    s[1] = '\0';

    operator= (s);
  }

  AE_INLINE bool aeCachedString::IsEqual (const char* str) const
  {
    return (m_Data == s_StringCache.find (str));
  }

  AE_INLINE bool aeCachedString::IsEqual (const aeBasicString& str) const
  {
    return (m_Data == s_StringCache.find (str));
  }

  AE_INLINE bool operator== (const aeCachedString& lhs, const aeCachedString& rhs)
  {
    return (lhs.m_Data == rhs.m_Data);
  }

  AE_INLINE bool operator== (const aeCachedString& lhs, const aeBasicString& rhs)
  {
    return (lhs.IsEqual (rhs));
  }

  AE_INLINE bool operator== (const aeBasicString& lhs, const aeCachedString& rhs)
  {
    return (rhs.IsEqual (lhs));
  }

  AE_INLINE bool operator== (const aeCachedString& lhs, const char* rhs)
  {
    return (lhs.IsEqual (rhs));
  }

  AE_INLINE bool operator== (const char* lhs, const aeCachedString& rhs)
  {
    return (rhs.IsEqual (lhs));
  }

  AE_INLINE bool operator!= (const aeCachedString& lhs, const aeCachedString& rhs)
  {
    return (lhs.m_Data != rhs.m_Data);
  }

  AE_INLINE bool operator!= (const aeCachedString& lhs, const aeBasicString& rhs)
  {
    return (!lhs.IsEqual (rhs));
  }

  AE_INLINE bool operator!= (const aeBasicString& lhs, const aeCachedString& rhs)
  {
    return (!rhs.IsEqual (lhs));
  }

  AE_INLINE bool operator!= (const aeCachedString& lhs, const char* rhs)
  {
    return (!lhs.IsEqual (rhs));
  }

  AE_INLINE bool operator!= (const char* lhs, const aeCachedString& rhs)
  {
    return (!rhs.IsEqual (lhs));
  }
}

#endif
