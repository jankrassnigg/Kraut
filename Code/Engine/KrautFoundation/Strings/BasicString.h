#ifndef AE_FOUNDATION_STRINGS_BASICSTRING_H
#define AE_FOUNDATION_STRINGS_BASICSTRING_H

#include "../Strings/StringFunctions.h"
#include "Declarations.h"

namespace AE_NS_FOUNDATION
{
  //! This simple base class for all string classes is needed to make them compatible to each other without needing thousands of overloads.
  /*! This class should never be instanciated directly. Instead it only serves as the base-class for several
      other string classes.
  */
  class AE_FOUNDATION_DLL aeBasicString
  {
  public:
    aeBasicString(const char* szString);

    //! Returns the length of the string.
    aeUInt32 length(void) const { return (m_uiLength); }

    //! Returns whether the string has a length of zero.
    bool empty(void) const { return (m_uiLength == 0); }

    //! Allows read-access to the string.
    const char* c_str(void) const { return (m_pDataToRead); }

    //! Allows read-access to the character with the given index
    const char operator[](aeUInt32 index) const;

    //! Returns true if this string and the given string are completely identical.
    bool CompareEqual(const char* szString2) const;
    //! Returns true if the first n characters of this string and the given string are identical.
    bool CompareEqual(const char* szString2, aeUInt32 uiCharsToCompare) const;
    //! Returns true if this string and the given string are completely identical, except for lower/upper case.
    bool CompareEqual_NoCase(const char* szString2) const;
    //! Returns true if the first n characters of this string and the given string are identical, except for lower/upper case.
    bool CompareEqual_NoCase(const char* szString2, aeUInt32 uiCharsToCompare) const;

    //! Returns zero if this string and the given string are equal, a negative value if this string is lexicographically "smaller", otherwise a positive value.
    aeInt32 CompareAlphabetically(const char* szString2) const;
    //! Returns zero if the first n characters of this string and the given string are equal, a negative value if this string is lexicographically "smaller", otherwise a positive value.
    aeInt32 CompareAlphabetically(const char* szString2, aeUInt32 uiCharsToCompare) const;
    //! Returns zero if this string and the given string are equal, a negative value if this string is lexicographically "smaller", otherwise a positive value. Ignores case.
    aeInt32 CompareAlphabetically_NoCase(const char* szString2) const;
    //! Returns zero if the first n characters of this string and the given string are equal, a negative value if this string is lexicographically "smaller", otherwise a positive value. Ignores case.
    aeInt32 CompareAlphabetically_NoCase(const char* szString2, aeUInt32 uiCharsToCompare) const;

    //! Returns the first position after uiStartPos where szStringToFind is found. -1 if nothing is found.
    aeInt32 FindFirstStringPos(const char* szStringToFind, aeUInt32 uiStartPos = 0) const;
    //! Searches for the first occurance of szStringToFind. Returns the index or -1 if nothing is found. Ignores case.
    aeInt32 FindFirstStringPos_NoCase(const char* szStringToFind, aeUInt32 uiStartPos = 0) const;
    //! Returns the last position before uiStartPos where szStringToFind is found. -1 if nothing is found.
    aeInt32 FindLastStringPos(const char* szStringToFind, aeUInt32 uiStartPos = 0x0FFFFFFF) const;
    //! Searches for the last occurance of szStringToFind. Returns the index or -1 if nothing is found. Ignores case.
    aeInt32 FindLastStringPos_NoCase(const char* szStringToFind, aeUInt32 uiStartPos = 0x0FFFFFFF) const;

    //! Returns true if this string starts with the string given in szStartsWith.
    bool StartsWith(const char* szStartsWith) const;
    //! Returns true if this string starts with the string given in szStartsWith. Ignores case.
    bool StartsWith_NoCase(const char* szStartsWith) const;
    //! Returns true if this string ends with the string given in szEndsWith.
    bool EndsWith(const char* szEndsWith) const;
    //! Returns true if this string ends with the string given in szEndsWith. Ignores case.
    bool EndsWith_NoCase(const char* szEndsWith) const;

    //! Searches this string after position uiStartPos for the word szSearchFor. If IsDelimiterCB returns true for both characters in front and back of the word, the position is returned. Otherwise -1.
    aeInt32 FindWholeWord(const char* szSearchFor, aeStringFunctions::AE_IS_WORD_DELIMITER IsDelimiterCB, aeUInt32 uiStartPos = 0) const;
    //! Searches this string after position uiStartPos for the word szSearchFor. If IsDelimiterCB returns true for both characters in front and back of the word, the position is returned. Otherwise -1. Ignores case.
    aeInt32 FindWholeWord_NoCase(const char* szSearchFor, aeStringFunctions::AE_IS_WORD_DELIMITER IsDelimiterCB, aeUInt32 uiStartPos = 0) const;


  protected:
    //! Non-instantiable from outside.
    explicit aeBasicString(char* pReadData, aeUInt32 uiLength)
      : m_pDataToRead(pReadData)
      , m_uiLength(uiLength)
    {
      // The empty string "" is simply stored inside the executable (static data)
      // By initializing all pointers with it, c_str will always return a valid zero-terminated string
      // even if the string-class has not allocated any data so far

      AE_CHECK_DEV(pReadData != nullptr, "aeBasicString::Constructor: pReadData must not be nullptr. Initialize it with \"\" instead.");
    }

    //! A pointer to the data. Differently initialized/modified by each derived string class.
    char* m_pDataToRead;
    //! The cached length of the string.
    aeUInt32 m_uiLength;

  private:
    aeBasicString(void);
    aeBasicString(const aeBasicString& cc);
    void operator=(const aeBasicString& rhs);
  };

  // *** Comparison Operators that work on all combinations of aeBasicString-classes and C-Strings ***

  AE_FOUNDATION_DLL bool operator==(const aeBasicString& lhs, const aeBasicString& rhs);
  AE_FOUNDATION_DLL bool operator==(const char* lhs, const aeBasicString& rhs);
  AE_FOUNDATION_DLL bool operator==(const aeBasicString& lhs, const char* rhs);

  AE_FOUNDATION_DLL bool operator!=(const aeBasicString& lhs, const aeBasicString& rhs);
  AE_FOUNDATION_DLL bool operator!=(const char* lhs, const aeBasicString& rhs);
  AE_FOUNDATION_DLL bool operator!=(const aeBasicString& lhs, const char* rhs);

  AE_FOUNDATION_DLL bool operator<(const aeBasicString& lhs, const aeBasicString& rhs);
  AE_FOUNDATION_DLL bool operator<(const char* lhs, const aeBasicString& rhs);
  AE_FOUNDATION_DLL bool operator<(const aeBasicString& lhs, const char* rhs);

  AE_FOUNDATION_DLL bool operator>(const aeBasicString& lhs, const aeBasicString& rhs);
  AE_FOUNDATION_DLL bool operator>(const char* lhs, const aeBasicString& rhs);
  AE_FOUNDATION_DLL bool operator>(const aeBasicString& lhs, const char* rhs);

  AE_FOUNDATION_DLL bool operator<=(const aeBasicString& lhs, const aeBasicString& rhs);
  AE_FOUNDATION_DLL bool operator<=(const char* lhs, const aeBasicString& rhs);
  AE_FOUNDATION_DLL bool operator<=(const aeBasicString& lhs, const char* rhs);

  AE_FOUNDATION_DLL bool operator>=(const aeBasicString& lhs, const aeBasicString& rhs);
  AE_FOUNDATION_DLL bool operator>=(const char* lhs, const aeBasicString& rhs);
  AE_FOUNDATION_DLL bool operator>=(const aeBasicString& lhs, const char* rhs);
} // namespace AE_NS_FOUNDATION


#include "Inline/BasicString.inl"

#endif
