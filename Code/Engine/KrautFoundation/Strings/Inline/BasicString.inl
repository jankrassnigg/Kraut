#ifndef AE_FOUNDATION_STRINGS_BASICSTRING_INL
#define AE_FOUNDATION_STRINGS_BASICSTRING_INL

#include "../Strings/StringFunctions.h"

namespace AE_NS_FOUNDATION
{
  AE_INLINE aeBasicString::aeBasicString (const char* szString) : m_pDataToRead ((char*) szString)
  {
    if (szString == NULL)
      m_pDataToRead = "";

    m_uiLength = aeStringFunctions::GetLength (szString);
  }

  AE_INLINE const char aeBasicString::operator[](aeUInt32 index) const
  {
    AE_CHECK_DEV (index < m_uiLength, "aeBasicString::operator[]: Cannot access character %d, the string only has a length of %d.", index, m_uiLength);
    
    return (m_pDataToRead[index]);
  }

  AE_INLINE bool aeBasicString::CompareEqual (const char* szString2) const
  {
    return (aeStringFunctions::CompareEqual (m_pDataToRead, szString2));
  }

  AE_INLINE bool aeBasicString::CompareEqual (const char* szString2, aeUInt32 uiCharsToCompare) const
  {
    return (aeStringFunctions::CompareEqual (m_pDataToRead, szString2, uiCharsToCompare));
  }

  AE_INLINE bool aeBasicString::CompareEqual_NoCase (const char* szString2) const
  {
    return (aeStringFunctions::CompareEqual_NoCase (m_pDataToRead, szString2));
  }

  AE_INLINE bool aeBasicString::CompareEqual_NoCase (const char* szString2, aeUInt32 uiCharsToCompare) const
  {
    return (aeStringFunctions::CompareEqual_NoCase (m_pDataToRead, szString2, uiCharsToCompare));
  }

  AE_INLINE aeInt32 aeBasicString::CompareAlphabetically (const char* szString2) const
  {
    return (aeStringFunctions::CompareAlphabetically (m_pDataToRead, szString2));
  }

  AE_INLINE aeInt32 aeBasicString::CompareAlphabetically (const char* szString2, aeUInt32 uiCharsToCompare) const
  {
    return (aeStringFunctions::CompareAlphabetically (m_pDataToRead, szString2, uiCharsToCompare));
  }

  AE_INLINE aeInt32 aeBasicString::CompareAlphabetically_NoCase (const char* szString2) const
  {
    return (aeStringFunctions::CompareAlphabetically_NoCase (m_pDataToRead, szString2));
  }

  AE_INLINE aeInt32 aeBasicString::CompareAlphabetically_NoCase (const char* szString2, aeUInt32 uiCharsToCompare) const
  {
    return (aeStringFunctions::CompareAlphabetically_NoCase (m_pDataToRead, szString2, uiCharsToCompare));
  }

  AE_INLINE aeInt32 aeBasicString::FindFirstStringPos (const char* szStringToFind, aeUInt32 uiStartPos) const
  {
    return (aeStringFunctions::FindFirstStringPos (m_pDataToRead, szStringToFind, uiStartPos));
  }

  AE_INLINE aeInt32 aeBasicString::FindFirstStringPos_NoCase (const char* szStringToFind, aeUInt32 uiStartPos) const
  {
    return (aeStringFunctions::FindFirstStringPos_NoCase (m_pDataToRead, szStringToFind, uiStartPos));
  }

  AE_INLINE aeInt32 aeBasicString::FindLastStringPos  (const char* szStringToFind, aeUInt32 uiStartPos) const
  {
    return (aeStringFunctions::FindLastStringPos (m_pDataToRead, szStringToFind, uiStartPos));
  }

  AE_INLINE aeInt32 aeBasicString::FindLastStringPos_NoCase  (const char* szStringToFind, aeUInt32 uiStartPos) const
  {
    return (aeStringFunctions::FindLastStringPos_NoCase (m_pDataToRead, szStringToFind, uiStartPos));
  }

  AE_INLINE bool aeBasicString::StartsWith (const char* szStartsWith) const
  {
    return (aeStringFunctions::StartsWith (c_str (), szStartsWith));
  }

  AE_INLINE bool aeBasicString::StartsWith_NoCase (const char* szStartsWith) const
  {
    return (aeStringFunctions::StartsWith_NoCase (c_str (), szStartsWith));
  }

  AE_INLINE bool aeBasicString::EndsWith (const char* szEndsWith) const
  {
    return (aeStringFunctions::EndsWith (c_str (), szEndsWith));
  }

  AE_INLINE bool aeBasicString::EndsWith_NoCase (const char* szEndsWith) const
  {
    return (aeStringFunctions::EndsWith_NoCase (c_str (), szEndsWith));
  }

  AE_INLINE aeInt32 aeBasicString::FindWholeWord (const char* szSearchFor, aeStringFunctions::AE_IS_WORD_DELIMITER IsDelimiterCB, aeUInt32 uiStartPos) const
  {
    return (aeStringFunctions::FindWholeWord (c_str (), szSearchFor, IsDelimiterCB, uiStartPos));
  }

  AE_INLINE aeInt32 aeBasicString::FindWholeWord_NoCase (const char* szSearchFor, aeStringFunctions::AE_IS_WORD_DELIMITER IsDelimiterCB, aeUInt32 uiStartPos) const
  {
    return (aeStringFunctions::FindWholeWord_NoCase (c_str (), szSearchFor, IsDelimiterCB, uiStartPos));
  }

  AE_INLINE bool operator== (const aeBasicString& lhs, const aeBasicString& rhs)
  {
    if (lhs.length () != rhs.length ())
      return (false);

    return (aeStringFunctions::CompareEqual (lhs.c_str (), rhs.c_str ()));
  }

  AE_INLINE bool operator== (const char* lhs, const aeBasicString& rhs)
  {
    return (aeStringFunctions::CompareEqual (lhs, rhs.c_str ()));
  }

  AE_INLINE bool operator== (const aeBasicString& lhs, const char* rhs)
  {
    return (aeStringFunctions::CompareEqual (lhs.c_str (), rhs));
  }


  AE_INLINE bool operator!= (const aeBasicString& lhs, const aeBasicString& rhs)
  {
    return (! (lhs == rhs) );
  }

  AE_INLINE bool operator!= (const char* lhs, const aeBasicString& rhs)
  {
    return (! (lhs == rhs) );
  }

  AE_INLINE bool operator!= (const aeBasicString& lhs, const char* rhs)
  {
    return (! (lhs == rhs) );
  }


  AE_INLINE bool operator< (const aeBasicString& lhs, const aeBasicString& rhs)
  {
    return (aeStringFunctions::CompareAlphabetically (lhs.c_str (), rhs.c_str ()) < 0);
  }

  AE_INLINE bool operator< (const char* lhs, const aeBasicString& rhs)
  {
    return (aeStringFunctions::CompareAlphabetically (lhs, rhs.c_str ()) < 0);
  }

  AE_INLINE bool operator< (const aeBasicString& lhs, const char* rhs)
  {
    return (aeStringFunctions::CompareAlphabetically (lhs.c_str (), rhs) < 0);
  }


  AE_INLINE bool operator> (const aeBasicString& lhs, const aeBasicString& rhs)
  {
    return (aeStringFunctions::CompareAlphabetically (lhs.c_str (), rhs.c_str ()) > 0);
  }

  AE_INLINE bool operator> (const char* lhs, const aeBasicString& rhs)
  {
    return (aeStringFunctions::CompareAlphabetically (lhs, rhs.c_str ()) > 0);
  }

  AE_INLINE bool operator> (const aeBasicString& lhs, const char* rhs)
  {
    return (aeStringFunctions::CompareAlphabetically (lhs.c_str (), rhs) > 0);
  }


  AE_INLINE bool operator<= (const aeBasicString& lhs, const aeBasicString& rhs)
  {
    return (aeStringFunctions::CompareAlphabetically (lhs.c_str (), rhs.c_str ()) <= 0);
  }

  AE_INLINE bool operator<= (const char* lhs, const aeBasicString& rhs)
  {
    return (aeStringFunctions::CompareAlphabetically (lhs, rhs.c_str ()) <= 0);
  }

  AE_INLINE bool operator<= (const aeBasicString& lhs, const char* rhs)
  {
    return (aeStringFunctions::CompareAlphabetically (lhs.c_str (), rhs) <= 0);
  }


  AE_INLINE bool operator>= (const aeBasicString& lhs, const aeBasicString& rhs)
  {
    return (aeStringFunctions::CompareAlphabetically (lhs.c_str (), rhs.c_str ()) >= 0);
  }

  AE_INLINE bool operator>= (const char* lhs, const aeBasicString& rhs)
  {
    return (aeStringFunctions::CompareAlphabetically (lhs, rhs.c_str ()) >= 0);
  }

  AE_INLINE bool operator>= (const aeBasicString& lhs, const char* rhs)
  {
    return (aeStringFunctions::CompareAlphabetically (lhs.c_str (), rhs) >= 0);
  }


}

#endif
