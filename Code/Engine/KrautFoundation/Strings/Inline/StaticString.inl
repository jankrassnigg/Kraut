#ifndef AE_FOUNDATION_STRINGS_STATICSTRING_INL
#define AE_FOUNDATION_STRINGS_STATICSTRING_INL

#include "../Strings/StringFunctions.h"

namespace AE_NS_FOUNDATION
{
  template<aeUInt16 SIZE>
  aeStaticString<SIZE>::aeStaticString (void) : aeBasicString (&m_String[0], 0)
  {
    clear ();
  }

  template<aeUInt16 SIZE>
  aeStaticString<SIZE>::aeStaticString (const aeBasicString& cc) : aeBasicString (&m_String[0], 0)
  {
    operator= (cc);
  }

  template<aeUInt16 SIZE>
  aeStaticString<SIZE>::aeStaticString (const aeStaticString<SIZE>& cc) : aeBasicString (&m_String[0], 0)
  {
    operator= (cc);
  }

  template<aeUInt16 SIZE>
  aeStaticString<SIZE>::aeStaticString (const char* cc) : aeBasicString (&m_String[0], 0)
  {
    operator= (cc);
  }

  template<aeUInt16 SIZE>
  aeStaticString<SIZE>::aeStaticString (const char* cc, aeUInt32 uiMaxChars) : aeBasicString (&m_String[0], 0)
  {
    aeStringFunctions::Copy (&m_String[0], SIZE, cc, aeMath::Min<aeInt32> (SIZE - 1, uiMaxChars));
    m_uiLength = aeMath::Min (uiMaxChars, aeStringFunctions::GetLength (c_str ()));
  }

  template<aeUInt16 SIZE>
  void aeStaticString<SIZE>::operator= (const aeStaticString<SIZE>& str)
  {
    operator= ((const aeBasicString&) str);
  }

  template<aeUInt16 SIZE>
  void aeStaticString<SIZE>::operator= (const aeBasicString& str)
  {
    m_uiLength = aeMath::Min<aeUInt16> (str.length (), SIZE - 1);
    aeStringFunctions::Copy (&m_String[0], SIZE, str.c_str (), m_uiLength);
  }

  template<aeUInt16 SIZE>
  void aeStaticString<SIZE>::operator= (const char* s)
  {
    aeStringFunctions::Copy (&m_String[0], SIZE, s);
    m_uiLength = aeStringFunctions::GetLength (c_str ());
  }

  template<aeUInt16 SIZE>
  void aeStaticString<SIZE>::operator= (char c)
  {
    // bad way to clear a string
    if (c == '\0')
    {
      clear ();
      return;
    }

    m_String[0] = c;
    m_String[1] = '\0';
    m_uiLength = 1;
  }


  /*! uiNewSize has to be in the range [0; SIZE-1].
      The SIZE-1-th element is always \0.
  */
  template<aeUInt16 SIZE>
  void aeStaticString<SIZE>::resize (aeUInt16 uiNewSize)
  {
    AE_CHECK_DEV (uiNewSize < SIZE, "Cannot resize to %d elements, the string can only hold %d characters.", uiNewSize, SIZE - 1);

    m_uiLength = uiNewSize;
    m_String[uiNewSize] = '\0';
  }

  /*! The resulting string is clamped to SIZE-1 characters.
  */
  template<aeUInt16 SIZE>
  void aeStaticString<SIZE>::Concatenate (const char* szSource)
  {
    m_uiLength = aeStringFunctions::Concatenate (&m_String[0], SIZE, length (), szSource);
  }

  template<aeUInt16 SIZE>
  void aeStaticString<SIZE>::Concatenate (const char* szSource, aeUInt32 uiCharsToCopy)
  {
    m_uiLength = aeStringFunctions::Concatenate (&m_String[0], SIZE, length (), szSource, uiCharsToCopy);
  }

  template<aeUInt16 SIZE>
  void aeStaticString<SIZE>::Format (const char* szFormat, ...)
  {
    va_list ap;
    va_start (ap, szFormat);

    FormatArgs (szFormat, ap);

    va_end (ap);
  }

  template<aeUInt16 SIZE>
  void aeStaticString<SIZE>::FormatArgs (const char* szFormat, va_list ap)
  {
    m_uiLength = aeStringFunctions::FormatArgs (&m_String[0], SIZE, szFormat, ap);
  }

  template<aeUInt16 SIZE>
  void aeStaticString<SIZE>::operator+= (const aeBasicString& str)
  {
    Concatenate (str.c_str (), str.length ());
  }

  template<aeUInt16 SIZE>
  void aeStaticString<SIZE>::operator+= (const char* s)
  {
    Concatenate (s);
  }

  template<aeUInt16 SIZE>
  void aeStaticString<SIZE>::operator+= (char c)
  {
    if (m_uiLength < SIZE - 1)
    {
      m_String[m_uiLength] = c;
      ++m_uiLength;
      m_String[m_uiLength] = '\0';
    }
  }

  template<aeUInt16 SIZE>
  AE_INLINE char& aeStaticString<SIZE>::operator[](aeUInt32 index)
  {
    AE_CHECK_DEV (index < m_uiLength, "aeStaticString::operator[]: Cannot access character %d, the string only has a length of %d.", index, m_uiLength);

    return (m_pDataToRead[index]);
  }

  template<aeUInt16 SIZE>
  AE_INLINE void aeStaticString<SIZE>::ToUpperCase (void)
  {
    aeStringFunctions::ToUpperCase (m_pDataToRead);
  }

  template<aeUInt16 SIZE>
  AE_INLINE void aeStaticString<SIZE>::ToLowerCase (void)
  {
    aeStringFunctions::ToLowerCase (m_pDataToRead);
  }

}

#endif


