#ifndef AE_FOUNDATION_STRINGS_STRING_INL
#define AE_FOUNDATION_STRINGS_STRING_INL

#include "../Strings/StringFunctions.h"
#include "../Math/Math.h"
#include "../Memory/Memory.h"
#include "../Memory/MemoryManagement.h"

namespace AE_NS_FOUNDATION
{
  AE_INLINE aeUInt32 aeString::ComputeStringCapacity (aeUInt32 uiStringLength)
  {
    // for n characters one needs n+1 bytes, round that to the next higher multiple of 32 to prevent excessive reallocations on small changes
    return (aeMath::FloatToInt (aeMath::Ceil ((float) uiStringLength + 1.0f, 32.0f)));
  }

  AE_INLINE aeString::aeString (void) : aeBasicString ("", 0), m_uiCapacity (1)
  {
  }

  AE_INLINE aeString::aeString (const aeBasicString& cc) : aeBasicString ("", 0), m_uiCapacity (1)
  {
    if (!cc.empty ())
    {
      m_uiCapacity = ComputeStringCapacity (cc.length ());

      m_pDataToRead = AE_MEMORY_ALLOCATE_TYPE_NODEBUG (char, m_uiCapacity, true);
      aeStringFunctions::Copy (m_pDataToRead, m_uiCapacity, cc.c_str ());

      m_uiLength = cc.length ();
      m_pDataToRead[m_uiLength] = '\0';
    }
  }

  AE_INLINE aeString::aeString (const aeString& cc) : aeBasicString ("", 0), m_uiCapacity (1)
  {
    if (!cc.empty ())
    {
      m_uiCapacity = ComputeStringCapacity (cc.length ());

      m_pDataToRead = AE_MEMORY_ALLOCATE_TYPE_NODEBUG (char, m_uiCapacity, true);
      aeStringFunctions::Copy (m_pDataToRead, m_uiCapacity, cc.c_str ());

      m_uiLength = cc.length ();
      m_pDataToRead[m_uiLength] = '\0';
    }
  }

  AE_INLINE aeString::aeString (const char* cc) : aeBasicString ("", 0), m_uiCapacity (1)
  {
    m_uiLength = aeStringFunctions::GetLength (cc);

    if (m_uiLength > 0)
    {
      m_uiCapacity = ComputeStringCapacity (m_uiLength);

      m_pDataToRead = AE_MEMORY_ALLOCATE_TYPE_NODEBUG (char, m_uiCapacity, true);
      aeStringFunctions::Copy (m_pDataToRead, m_uiCapacity, cc, m_uiLength);

      m_pDataToRead[m_uiLength] = '\0';
    }
  }

  AE_INLINE aeString::aeString (const char* cc, aeUInt32 uiMaxChar) : aeBasicString ("", 0), m_uiCapacity (1)
  {
    m_uiLength = aeMath::Min (uiMaxChar, aeStringFunctions::GetLength (cc));

    if (m_uiLength > 0)
    {
      m_uiCapacity = ComputeStringCapacity (m_uiLength);

      m_pDataToRead = AE_MEMORY_ALLOCATE_TYPE_NODEBUG (char, m_uiCapacity, true);
      aeStringFunctions::Copy (m_pDataToRead, m_uiCapacity, cc, m_uiLength);
    }

    m_pDataToRead[m_uiLength] = '\0';
  }

  AE_INLINE aeString::~aeString ()
  {
    clear (true);
  }

  AE_INLINE void aeString::reserve (aeUInt32 uiCapacity)
  {
    uiCapacity += 1;

    if (uiCapacity <= m_uiCapacity)
      return;

    const aeUInt32 uiNewCapacity = ComputeStringCapacity (uiCapacity);
    char* pNewData = AE_MEMORY_ALLOCATE_TYPE_NODEBUG (char, uiNewCapacity, true);

    // copy the old data over to the new array
    if (m_uiLength > 0)
    {
      aeStringFunctions::Copy (pNewData, uiNewCapacity, m_pDataToRead, m_uiLength);
    }

    if (m_uiCapacity > 1)
      AE_MEMORY_DEALLOCATE_NODEBUG (m_pDataToRead, true);

    m_pDataToRead = pNewData;
    m_uiCapacity = uiNewCapacity;
    m_pDataToRead[m_uiLength] = '\0';
  }

  AE_INLINE void aeString::resize (aeUInt32 uiNewSize)
  {
    // Handles the case that m_pDataToRead is NULL/"" AND some idiot resizes to 0 length.
    if (uiNewSize == m_uiLength)
      return;

    // In this case m_pDataToRead cannot be NULL/"", so accessing element 0 is safe.
    if (uiNewSize < m_uiLength)
    {
      m_uiLength = uiNewSize;
      m_pDataToRead[uiNewSize] = '\0';
      return;
    }

    // Handles the case, that the string is grown, but it has enough capacity
    if (uiNewSize < m_uiCapacity)
    {
      // the newly accessible characters are not initialized
      // in fact the previous \0 is not overwritten, so unless someone puts something useful in the
      // now availble characters, the string will not change, at all

      m_uiLength = uiNewSize;
      m_pDataToRead[m_uiLength] = '\0';
      return;
    }

    // now handle the case that the string has to grow, but there is not enough capacity
    reserve (uiNewSize);

    m_uiLength = uiNewSize;
    m_pDataToRead[m_uiLength] = '\0';
  }

  AE_INLINE aeUInt32 aeString::capacity (void) const
  {
    return (m_uiCapacity - 1);
  }

  AE_INLINE void aeString::clear (bool bDeallocateData)
  {
    if (bDeallocateData)
    {
      if (m_uiCapacity > 1)
      {
        AE_MEMORY_DEALLOCATE_NODEBUG (m_pDataToRead, true);
        m_pDataToRead = "";

        m_uiCapacity = 1;
        m_uiLength = 0;
      }

      return;
    }

    // If no deallocation is desired, just resize the string to zero elements.
    resize (0);
  }


  AE_INLINE void aeString::Concatenate (const char* szSource)
  {
    if (aeStringFunctions::IsNullOrEmpty (szSource))
      return;

    Concatenate (szSource, aeStringFunctions::GetLength (szSource));
  }

  AE_INLINE void aeString::Concatenate (const char* szSource, aeUInt32 uiCharsToCopy)
  {
    if (aeStringFunctions::IsNullOrEmpty (szSource))
      return;

    const aeUInt32 Length = uiCharsToCopy;
    const aeUInt32 uiPrevLength = m_uiLength;

    resize (m_uiLength + Length);

    aeStringFunctions::Copy (&m_pDataToRead[uiPrevLength], m_uiCapacity - uiPrevLength, szSource, Length);
  }


  AE_INLINE void aeString::Format (const char* szFormat, ...)
  {
    va_list ap;
    va_start (ap, szFormat);

    FormatArgs (szFormat, ap);

    va_end (ap);
  }

  AE_INLINE void aeString::FormatArgs (const char* szFormat, va_list ap)
  {
    // TODO: Test this function thoroughly, especially on different platforms !

    AE_CHECK_DEV (szFormat != NULL, "aeString::FormatArgs: Cannot work with a NULL-String as Format.");

    const aeUInt32 uiLength = aeStringFunctions::GetRequiredSizeForFormat (szFormat, ap);
    resize (uiLength);

    m_uiLength = aeStringFunctions::FormatArgs (&m_pDataToRead[0], uiLength, szFormat, ap);
    m_pDataToRead[m_uiLength] = '\0';
  }

  AE_INLINE void aeString::operator+= (const aeBasicString& str)
  {
    Concatenate (str.c_str (), str.length ());
  }

  AE_INLINE void aeString::operator+= (const char* s)
  {
    Concatenate (s);
  }

  AE_INLINE void aeString::operator+= (char c)
  {
    char temp[2];
    temp[0] = c;
    temp[1] = '\0';

    Concatenate (temp, 1);
  }

  AE_INLINE void aeString::operator= (const aeString& str)
  {
    operator= ((const aeBasicString&) str);
  }

  AE_INLINE void aeString::operator= (const aeBasicString& str)
  {
    resize (str.length ());

    // Copy automatically places a \0 at the end
    if (m_uiLength > 0)
      aeStringFunctions::Copy (m_pDataToRead, m_uiCapacity, str.c_str (), m_uiLength);

    // resize automatically places a \0 at the end
  }

  AE_INLINE void aeString::operator= (const char* s)
  {
    const aeUInt32 Length = aeStringFunctions::GetLength (s);

    resize (Length);

    // Copy automatically places a \0 at the end
    if (m_uiLength > 0)
      aeStringFunctions::Copy (m_pDataToRead, m_uiCapacity, s, m_uiLength);

    // resize automatically places a \0 at the end
  }

  AE_INLINE void aeString::operator= (char c)
  {
    // If someone actually "clears" a string this way, make sure no memory will be allocated
    if ((c == '\0') && (m_uiCapacity == 0))
        return;

    resize (1);
    m_pDataToRead[0] = c;
    m_pDataToRead[1] = '\0';
  }

  AE_INLINE const aeString operator+ (const aeBasicString& lhs, const char* rhs)
  {
    aeString res;
    res.reserve (lhs.length () + aeStringFunctions::GetLength (rhs));
    res = lhs;
    res.Concatenate (rhs);

    return (res);
  }

  AE_INLINE const aeString operator+ (const char* lhs, const aeBasicString& rhs)
  {
    aeString res;
    res.reserve (rhs.length () + aeStringFunctions::GetLength (lhs));
    res = lhs;
    res.Concatenate (rhs.c_str (), rhs.length ());

    return (res);
  }

  AE_INLINE const aeString operator+ (const aeBasicString& lhs, const aeBasicString& rhs)
  {
    aeString res;
    res.reserve (lhs.length () + rhs.length ());
    res = lhs;
    res.Concatenate (rhs.c_str (), rhs.length ());

    return (res);
  }

  AE_INLINE char& aeString::operator[](aeUInt32 index)
  {
    AE_CHECK_DEV (index < m_uiLength, "aeString::operator[]: Cannot access character %d, the string only has a length of %d.", index, m_uiLength);

    return (m_pDataToRead[index]);
  }

  AE_INLINE void aeString::ToUpperCase (void)
  {
    aeStringFunctions::ToUpperCase (m_pDataToRead);
  }

  AE_INLINE void aeString::ToLowerCase (void)
  {
    aeStringFunctions::ToLowerCase (m_pDataToRead);
  }
}



#endif


