#ifndef AE_FOUNDATION_STRINGS_HYBRIDSTRING_INL
#define AE_FOUNDATION_STRINGS_HYBRIDSTRING_INL

#include "../Strings/StringFunctions.h"
#include "../Memory/MemoryManagement.h"

namespace AE_NS_FOUNDATION
{
  template<aeUInt32 SIZE>
  AE_INLINE aeUInt32 aeHybridString<SIZE>::ComputeStringCapacity (aeUInt32 uiStringLength)
  {
    // for n characters one needs n+1 bytes, round that to the next higher multiple of 32 to prevent excessive reallocations on small changes
    return (aeMath::FloatToInt (aeMath::Ceil ((float) uiStringLength + 1.0f, 32.0f)));
  }

  template<aeUInt32 SIZE>
  aeHybridString<SIZE>::aeHybridString (void) : aeBasicString (&m_String[0], 0), m_uiCapacity (SIZE)
  {
    m_pDataToRead[0] = '\0';
  }

  template<aeUInt32 SIZE>
  aeHybridString<SIZE>::aeHybridString (const aeBasicString& cc) : aeBasicString (&m_String[0], 0), m_uiCapacity (SIZE)
  {
    m_pDataToRead[0] = '\0';

    operator= (cc);
  }

  template<aeUInt32 SIZE>
  aeHybridString<SIZE>::aeHybridString (const aeHybridString<SIZE>& cc) : aeBasicString (&m_String[0], 0), m_uiCapacity (SIZE)
  {
    m_pDataToRead[0] = '\0';

    operator= (cc);
  }

  template<aeUInt32 SIZE>
  aeHybridString<SIZE>::aeHybridString (const char* cc) : aeBasicString (&m_String[0], 0), m_uiCapacity (SIZE)
  {
    m_pDataToRead[0] = '\0';

    operator= (cc);
  }

  template<aeUInt32 SIZE>
  aeHybridString<SIZE>::aeHybridString (const char* cc, aeUInt32 uiMaxChars) : aeBasicString (&m_String[0], 0), m_uiCapacity (SIZE)
  {
    m_pDataToRead[0] = '\0';

    const aeUInt32 len = aeMath::Min (uiMaxChars, aeStringFunctions::GetLength (cc));
    resize (len);

    aeStringFunctions::Copy (m_pDataToRead, m_uiCapacity, cc, len);
  }

  template<aeUInt32 SIZE>
  aeHybridString<SIZE>::~aeHybridString ()
  {
    clear (true);
  }

  template<aeUInt32 SIZE>
  void aeHybridString<SIZE>::operator= (const aeHybridString<SIZE>& str)
  {
    operator= ((const aeBasicString&) str);
  }

  template<aeUInt32 SIZE>
  void aeHybridString<SIZE>::operator= (const aeBasicString& str)
  {
    resize (str.length ());

    aeStringFunctions::Copy (m_pDataToRead, m_uiCapacity, str.c_str (), str.length ());
  }

  template<aeUInt32 SIZE>
  void aeHybridString<SIZE>::operator= (const char* s)
  {
    const aeUInt32 len = aeStringFunctions::GetLength (s);
    resize (len);

    aeStringFunctions::Copy (m_pDataToRead, m_uiCapacity, s, len);
  }

  template<aeUInt32 SIZE>
  void aeHybridString<SIZE>::operator= (char c)
  {
    // bad way to clear a string
    if (c == '\0')
    {
      clear ();
      return;
    }

    m_pDataToRead[0] = c;
    m_pDataToRead[1] = '\0';
    m_uiLength = 1;
  }

  template<aeUInt32 SIZE>
  void aeHybridString<SIZE>::clear (bool bDeallocateData)
  {
    if (bDeallocateData)
    {
      if (m_uiCapacity > SIZE)
        AE_MEMORY_DEALLOCATE_NODEBUG (m_pDataToRead, true);

       m_pDataToRead = &m_String[0];
       m_uiCapacity = SIZE;
    }

    m_pDataToRead[0] = '\0';
    m_uiLength = 0;
  }

  template<aeUInt32 SIZE>
  void aeHybridString<SIZE>::reserve (aeUInt32 uiCapacity)
  {
    uiCapacity += 1;

    if (uiCapacity < m_uiCapacity)
      return;

    const aeUInt32 uiNewCapacity = ComputeStringCapacity (uiCapacity);
    char* pNewData = AE_MEMORY_ALLOCATE_TYPE_NODEBUG (char, uiNewCapacity, true);

    if (m_uiLength > 0)
      aeStringFunctions::Copy (pNewData, uiNewCapacity, m_pDataToRead, m_uiLength);

    // not using the internal buffer anymore
    if (m_uiCapacity > SIZE)
      AE_MEMORY_DEALLOCATE_NODEBUG (m_pDataToRead, true);

    m_uiCapacity = uiNewCapacity;
    m_pDataToRead = pNewData;

      // make sure the last character is properly initialized
    m_pDataToRead[m_uiLength] = '\0';
  }

  template<aeUInt32 SIZE>
  void aeHybridString<SIZE>::resize (aeUInt32 uiNewSize)
  {
    // no change
    if (uiNewSize == m_uiLength)
      return;

    // shrinking
    if (uiNewSize < m_uiLength)
    {
      m_uiLength = uiNewSize;
      m_pDataToRead[uiNewSize] = '\0';
      return;
    }

    // growing, but not above the capacity
    if (uiNewSize < m_uiCapacity)
    {
      m_uiLength = uiNewSize;
      m_pDataToRead[uiNewSize] = '\0';
      return;
    }

    // growing, but above the current capacity
    reserve (uiNewSize);

    m_uiLength = uiNewSize;
    m_pDataToRead[uiNewSize] = '\0';
  }

  template<aeUInt32 SIZE>
  void aeHybridString<SIZE>::Concatenate (const char* szSource)
  {
    Concatenate (szSource, aeStringFunctions::GetLength (szSource));
  }

  template<aeUInt32 SIZE>
  void aeHybridString<SIZE>::Concatenate (const char* szSource, aeUInt32 uiCharsToCopy)
  {
    const aeUInt32 uiPrevLen = m_uiLength;

    resize (m_uiLength + uiCharsToCopy);

    m_uiLength = aeStringFunctions::Concatenate (&m_pDataToRead[0], m_uiCapacity, uiPrevLen, szSource, uiCharsToCopy);
  }

  template<aeUInt32 SIZE>
  void aeHybridString<SIZE>::Format (const char* szFormat, ...)
  {
    va_list ap;
    va_start (ap, szFormat);

    FormatArgs (szFormat, ap);

    va_end (ap);
  }

  template<aeUInt32 SIZE>
  void aeHybridString<SIZE>::FormatArgs (const char* szFormat, va_list ap)
  {
    // TODO: Test this function thoroughly, especially on different platforms !

    AE_CHECK_DEV (szFormat != NULL, "aeHybridString::FormatArgs: Cannot work with a NULL-String as Format.");

    const aeUInt32 uiLength = aeStringFunctions::GetRequiredSizeForFormat (szFormat, ap);
    resize (uiLength);

    m_uiLength = aeStringFunctions::FormatArgs (&m_pDataToRead[0], uiLength, szFormat, ap);
    m_pDataToRead[m_uiLength] = '\0';
  }

  template<aeUInt32 SIZE>
  void aeHybridString<SIZE>::operator+= (const aeBasicString& str)
  {
    Concatenate (str.c_str (), str.length ());
  }

  template<aeUInt32 SIZE>
  void aeHybridString<SIZE>::operator+= (const char* s)
  {
    Concatenate (s);
  }

  template<aeUInt32 SIZE>
  void aeHybridString<SIZE>::operator+= (char c)
  {
    char temp[2];
    temp[0] = c;
    temp[1] = '\0';

    Concatenate (temp, 1);
  }


  template<aeUInt32 SIZE>
  void aeHybridString<SIZE>::ReplaceSubString (aeUInt32 uiPos, aeUInt32 uiLength, const char* szReplaceWith)
  {
    AE_CHECK_DEV (szReplaceWith != NULL, "aeHybridString::ReplaceSubString: Cannot replace something with a NULL-string.");
    AE_CHECK_DEV (uiPos + uiLength <= m_uiLength, "aeHybridString::ReplaceSubString: The given range is invalid. Cannot replace [%d to %d], the String is only %d characters long.", uiPos, uiPos + uiLength, m_uiLength);

    const aeUInt32 uiWordLen = aeStringFunctions::GetLength (szReplaceWith);

    // most simple case, just replace characters
    if (uiLength == uiWordLen)
    {
      for (aeUInt32 ui = 0; ui < uiWordLen; ++ui)
        m_pDataToRead[uiPos + ui] = szReplaceWith[ui];

      return;
    }

    // the replacement is shorter than the existing stuff -> move characters to the left, no reallocation needed
    if (uiWordLen < uiLength)
    {
      // first copy the replacement to the correct position
      aeStringFunctions::Copy (&m_pDataToRead[uiPos], uiWordLen+1, szReplaceWith, uiWordLen);

      const aeUInt32 uiDifference = uiLength - uiWordLen;

      // now move all the characters from behind the replaced string to the correct position
      for (aeUInt32 ui = uiPos + uiWordLen; ui < m_uiLength - uiDifference; ++ui)
        m_pDataToRead[ui] = m_pDataToRead[ui + uiDifference];

      m_uiLength -= uiDifference;
      m_pDataToRead[m_uiLength] = '\0';
      return;
    }

    // else the replacement is longer than the existing word
    {
      resize (m_uiLength - uiLength + uiWordLen);

      const aeUInt32 uiDifference = uiWordLen - uiLength;

      // first move the characters to the proper position from back to front
      for (aeInt32 ui = (aeInt32) m_uiLength - 1; ui >= (aeInt32) (uiPos + uiWordLen); --ui)
        m_pDataToRead[ui] = m_pDataToRead[ui - uiDifference];

      // now copy the replacement to the correct position
      for (aeUInt32 ui = 0; ui < uiWordLen; ++ui)
        m_pDataToRead[uiPos + ui] = szReplaceWith[ui];
    }
  }

  template<aeUInt32 SIZE>
  void aeHybridString<SIZE>::Insert (aeUInt32 uiPos, const char* szInsert)
  {
    ReplaceSubString (uiPos, 0, szInsert);
  }

  template<aeUInt32 SIZE>
  bool aeHybridString<SIZE>::Replace (const char* szSearchFor, const char* szReplaceWith, aeUInt32 uiStartPos)
  {
    const aeInt32 iPos = FindFirstStringPos (szSearchFor, uiStartPos);
    if (iPos < 0)
      return (false);

    ReplaceSubString (iPos, aeStringFunctions::GetLength (szSearchFor), szReplaceWith);
    return (true);
  }

  template<aeUInt32 SIZE>
  bool aeHybridString<SIZE>::Replace_NoCase (const char* szSearchFor, const char* szReplaceWith, aeUInt32 uiStartPos)
  {
    const aeInt32 iPos = FindFirstStringPos_NoCase (szSearchFor, uiStartPos);
    if (iPos < 0)
      return (false);

    ReplaceSubString (iPos, aeStringFunctions::GetLength (szSearchFor), szReplaceWith);
    return (true);
  }

  template<aeUInt32 SIZE>
  aeUInt32 aeHybridString<SIZE>::ReplaceAll (const char* szSearchFor, const char* szReplaceWith, aeUInt32 uiStartPos)
  {
    const aeUInt32 uiSearchLen = aeStringFunctions::GetLength (szSearchFor);
    const aeUInt32 uiReplaceLen = aeStringFunctions::GetLength (szReplaceWith);

    aeUInt32 uiCounter = 0;

    aeInt32 iFoundPos = FindFirstStringPos (szSearchFor, uiStartPos);
    while (iFoundPos >= 0)
    {
      ReplaceSubString (iFoundPos, uiSearchLen, szReplaceWith);
      ++uiCounter;

      uiStartPos = iFoundPos + uiReplaceLen;

      iFoundPos = FindFirstStringPos (szSearchFor, uiStartPos);

      AE_CHECK_DEV (uiCounter < 100000, "aeHybridString::ReplaceAll: Too many replacements! Probably an endless loop.");
    }

    return (uiCounter);
  }

  template<aeUInt32 SIZE>
  aeUInt32 aeHybridString<SIZE>::ReplaceAll_NoCase (const char* szSearchFor, const char* szReplaceWith, aeUInt32 uiStartPos)
  {
    const aeUInt32 uiSearchLen = aeStringFunctions::GetLength (szSearchFor);
    const aeUInt32 uiReplaceLen = aeStringFunctions::GetLength (szReplaceWith);

    aeUInt32 uiCounter = 0;

    aeInt32 iFoundPos = FindFirstStringPos_NoCase (szSearchFor, uiStartPos);
    while (iFoundPos >= 0)
    {
      ReplaceSubString (iFoundPos, uiSearchLen, szReplaceWith);
      ++uiCounter;

      uiStartPos = iFoundPos + uiReplaceLen;

      iFoundPos = FindFirstStringPos_NoCase (szSearchFor, uiStartPos);

      AE_CHECK_DEV (uiCounter < 100000, "aeHybridString::ReplaceAll: Too many replacements! Probably an endless loop.");
    }

    return (uiCounter);
  }

  template<aeUInt32 SIZE>
  bool aeHybridString<SIZE>::ReplaceWholeWord (const char* szSearchFor, const char* szReplaceWith, aeStringFunctions::AE_IS_WORD_DELIMITER IsDelimiterCB, aeUInt32 uiStartPos)
  {
    aeInt32 iFoundPos = FindWholeWord (szSearchFor, IsDelimiterCB, uiStartPos);

    if (iFoundPos < 0)
      return (false);

    ReplaceSubString (iFoundPos, aeStringFunctions::GetLength (szSearchFor), szReplaceWith);
    return (true);
  }

  template<aeUInt32 SIZE>
  bool aeHybridString<SIZE>::ReplaceWholeWord_NoCase (const char* szSearchFor, const char* szReplaceWith, aeStringFunctions::AE_IS_WORD_DELIMITER IsDelimiterCB, aeUInt32 uiStartPos)
  {
    aeInt32 iFoundPos = FindWholeWord_NoCase (szSearchFor, IsDelimiterCB, uiStartPos);

    if (iFoundPos < 0)
      return (false);

    ReplaceSubString (iFoundPos, aeStringFunctions::GetLength (szSearchFor), szReplaceWith);
    return (true);
  }

  template<aeUInt32 SIZE>
  aeUInt32 aeHybridString<SIZE>::ReplaceWholeWordAll (const char* szSearchFor, const char* szReplaceWith, aeStringFunctions::AE_IS_WORD_DELIMITER IsDelimiterCB, aeUInt32 uiStartPos)
  {
    const aeUInt32 uiSearchLen = aeStringFunctions::GetLength (szSearchFor);
    const aeUInt32 uiReplaceLen = aeStringFunctions::GetLength (szReplaceWith);

    aeUInt32 uiCounter = 0;

    aeInt32 iFoundPos = FindWholeWord (szSearchFor, IsDelimiterCB, uiStartPos);
    while (iFoundPos > 0)
    {
      ReplaceSubString (iFoundPos, uiSearchLen, szReplaceWith);
      ++uiCounter;

      uiStartPos = iFoundPos + uiReplaceLen;

      iFoundPos = FindWholeWord (szSearchFor, IsDelimiterCB, uiStartPos);

      AE_CHECK_DEV (uiCounter < 100000, "aeHybridString::ReplaceWholeWordAll: Too many replacements! Probably an endless loop.");
    }

    return (uiCounter);
  }

  template<aeUInt32 SIZE>
  aeUInt32 aeHybridString<SIZE>::ReplaceWholeWordAll_NoCase (const char* szSearchFor, const char* szReplaceWith, aeStringFunctions::AE_IS_WORD_DELIMITER IsDelimiterCB, aeUInt32 uiStartPos)
  {
    const aeUInt32 uiSearchLen = aeStringFunctions::GetLength (szSearchFor);
    const aeUInt32 uiReplaceLen = aeStringFunctions::GetLength (szReplaceWith);

    aeUInt32 uiCounter = 0;

    aeInt32 iFoundPos = FindWholeWord_NoCase (szSearchFor, IsDelimiterCB, uiStartPos);
    while (iFoundPos > 0)
    {
      ReplaceSubString (iFoundPos, uiSearchLen, szReplaceWith);
      ++uiCounter;

      uiStartPos = iFoundPos + uiReplaceLen;

      iFoundPos = FindWholeWord_NoCase (szSearchFor, IsDelimiterCB, uiStartPos);

      AE_CHECK_DEV (uiCounter < 100000, "aeHybridString::ReplaceWholeWordAll_NoCase: Too many replacements! Probably an endless loop.");
    }

    return (uiCounter);
  }

  template<aeUInt32 SIZE>
  AE_INLINE char& aeHybridString<SIZE>::operator[](aeUInt32 index)
  {
    AE_CHECK_DEV (index < m_uiLength, "aeHybridString::operator[]: Cannot access character %d, the string only has a length of %d.", index, m_uiLength);

    return (m_pDataToRead[index]);
  }

  template<aeUInt32 SIZE>
  AE_INLINE void aeHybridString<SIZE>::ToUpperCase (void)
  {
    aeStringFunctions::ToUpperCase (m_pDataToRead);
  }

  template<aeUInt32 SIZE>
  AE_INLINE void aeHybridString<SIZE>::ToLowerCase (void)
  {
    aeStringFunctions::ToLowerCase (m_pDataToRead);
  }
}

#endif


