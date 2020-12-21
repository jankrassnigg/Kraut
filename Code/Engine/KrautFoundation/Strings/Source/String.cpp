#include "../String.h"



namespace AE_NS_FOUNDATION
{
  void aeString::ReplaceSubString (aeUInt32 uiPos, aeUInt32 uiLength, const char* szReplaceWith)
  {
    AE_CHECK_DEV (szReplaceWith != NULL, "aeString::ReplaceSubString: Cannot replace something with a NULL-string.");
    AE_CHECK_DEV (uiPos + uiLength <= m_uiLength, "aeString::ReplaceSubString: The given range is invalid. Cannot replace [%d to %d], the String is only %d characters long.", uiPos, uiPos + uiLength, m_uiLength);

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
      for (aeInt32 ui = (aeInt32) m_uiLength - 1; ui >= (aeInt32) (uiPos + uiLength); --ui)
        m_pDataToRead[ui] = m_pDataToRead[ui - uiDifference];

      // now copy the replacement to the correct position
      for (aeUInt32 ui = 0; ui < uiWordLen; ++ui)
        m_pDataToRead[uiPos + ui] = szReplaceWith[ui];
    }
  }

  void aeString::Insert (aeUInt32 uiPos, const char* szInsert)
  {
    ReplaceSubString (uiPos, 0, szInsert);
  }

  bool aeString::Replace (const char* szSearchFor, const char* szReplaceWith, aeUInt32 uiStartPos)
  {
    const aeInt32 iPos = FindFirstStringPos (szSearchFor, uiStartPos);
    if (iPos < 0)
      return (false);

    ReplaceSubString (iPos, aeStringFunctions::GetLength (szSearchFor), szReplaceWith);
    return (true);
  }

  bool aeString::Replace_NoCase (const char* szSearchFor, const char* szReplaceWith, aeUInt32 uiStartPos)
  {
    const aeInt32 iPos = FindFirstStringPos_NoCase (szSearchFor, uiStartPos);
    if (iPos < 0)
      return (false);

    ReplaceSubString (iPos, aeStringFunctions::GetLength (szSearchFor), szReplaceWith);
    return (true);
  }

  aeUInt32 aeString::ReplaceAll (const char* szSearchFor, const char* szReplaceWith, aeUInt32 uiStartPos)
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

      AE_CHECK_DEV (uiCounter < 100000, "aeString::ReplaceAll: Too many replacements! Probably an endless loop.");
    }

    return (uiCounter);
  }

  aeUInt32 aeString::ReplaceAll_NoCase (const char* szSearchFor, const char* szReplaceWith, aeUInt32 uiStartPos)
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

      AE_CHECK_DEV (uiCounter < 100000, "aeString::ReplaceAll: Too many replacements! Probably an endless loop.");
    }

    return (uiCounter);
  }

  bool aeString::ReplaceWholeWord (const char* szSearchFor, const char* szReplaceWith, aeStringFunctions::AE_IS_WORD_DELIMITER IsDelimiterCB, aeUInt32 uiStartPos)
  {
    aeInt32 iFoundPos = FindWholeWord (szSearchFor, IsDelimiterCB, uiStartPos);

    if (iFoundPos < 0)
      return (false);

    ReplaceSubString (iFoundPos, aeStringFunctions::GetLength (szSearchFor), szReplaceWith);
    return (true);
  }

  bool aeString::ReplaceWholeWord_NoCase (const char* szSearchFor, const char* szReplaceWith, aeStringFunctions::AE_IS_WORD_DELIMITER IsDelimiterCB, aeUInt32 uiStartPos)
  {
    aeInt32 iFoundPos = FindWholeWord_NoCase (szSearchFor, IsDelimiterCB, uiStartPos);

    if (iFoundPos < 0)
      return (false);

    ReplaceSubString (iFoundPos, aeStringFunctions::GetLength (szSearchFor), szReplaceWith);
    return (true);
  }

  aeUInt32 aeString::ReplaceWholeWordAll (const char* szSearchFor, const char* szReplaceWith, aeStringFunctions::AE_IS_WORD_DELIMITER IsDelimiterCB, aeUInt32 uiStartPos)
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

      AE_CHECK_DEV (uiCounter < 100000, "aeString::ReplaceWholeWordAll: Too many replacements! Probably an endless loop.");
    }

    return (uiCounter);
  }

  aeUInt32 aeString::ReplaceWholeWordAll_NoCase (const char* szSearchFor, const char* szReplaceWith, aeStringFunctions::AE_IS_WORD_DELIMITER IsDelimiterCB, aeUInt32 uiStartPos)
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

      AE_CHECK_DEV (uiCounter < 100000, "aeString::ReplaceWholeWordAll_NoCase: Too many replacements! Probably an endless loop.");
    }

    return (uiCounter);
  }
}

