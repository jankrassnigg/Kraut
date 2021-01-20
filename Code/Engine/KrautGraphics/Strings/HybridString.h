#ifndef AE_FOUNDATION_STRINGS_HYBRIDSTRING_H
#define AE_FOUNDATION_STRINGS_HYBRIDSTRING_H

#include <KrautFoundation/Strings/BasicString.h>
#include <KrautGraphics/Base.h>

namespace AE_NS_FOUNDATION
{
  //! A string that has an internal buffer to handle strings with a limited length. Dynamically allocates a larger buffer on demand.
  /*! A hybrid string has an internal buffer that can hold SIZE-1 characters (plus \0 terminator).
      As long as it stores strings shorter than that, no dynamic allocations are done.

  */
  template <aeUInt32 SIZE>
  class aeHybridString : public aeBasicString
  {
  public:
    //! Initializes the string to be empty.
    aeHybridString(void);
    //! Initializes the string with a copy of the given string.
    aeHybridString(const aeBasicString& cc);
    //! Initializes the string with a copy of the given string.
    aeHybridString(const aeHybridString<SIZE>& cc);
    //! Initializes the string with a copy of the given string.
    aeHybridString(const char* cc);
    //! Initializes the string with a copy of the given string but only up to uiMaxChars.
    aeHybridString(const char* cc, aeUInt32 uiMaxChars);
    //! Deallocates all data, if anything was allocated.
    ~aeHybridString();

    //! Makes sure the string has room for at least uiCapacity characters. Thus subsequent string-modifications might become more efficient.
    void reserve(aeUInt32 uiCapacity);

    //! Resizes the string, but NEVER deallocates data when shrinking the string. Only reallocates when growing above the current capacity.
    void resize(aeUInt32 uiNewSize);

    //! Returns how many characters the string can hold without need for reallocation.
    aeUInt32 capacity(void) const { return (m_uiCapacity - 1); }

    //! Resets the string to "". Does NOT deallocate any data, unless bDeallocateData is specifically set to true.
    void clear(bool bDeallocateData = false);

    //! Appends the given string to this string.
    void Concatenate(const char* szSource);
    //! Appends up to uiCharsToCopy characters of the given string to this string.
    void Concatenate(const char* szSource, aeUInt32 uiCharsToCopy);

    //! Creates a formatted string. See the C-function sprintf for details.
    void Format(const char* szFormat, ...);
    //! Creates a formatted string. See the C-function sprintf for details.
    void FormatArgs(const char* szFormat, va_list ap);

    // get operator[] from aeBasicString into this class
    using aeBasicString::operator[];
    //! Allows read/write-access to the character with the given index
    char& operator[](aeUInt32 index);

    //! Appends the given string to this string.
    void operator+=(const aeBasicString& str);
    //! Appends the given string to this string.
    void operator+=(const char* s);
    //! Appends the given character to this string.
    void operator+=(char c);

    //! Copies the given string into this one.
    void operator=(const aeHybridString<SIZE>& str);
    //! Copies the given string into this one.
    void operator=(const aeBasicString& str);
    //! Copies the given string into this one.
    void operator=(const char* s);
    //! Copies the given character into this one.
    void operator=(char c);

    //! Replaces the sub-string between [uiPos; uiPos+uiLength] with szReplaceWith.
    void ReplaceSubString(aeUInt32 uiPos, aeUInt32 uiLength, const char* szReplaceWith);
    //! Inserts szInsert at/before uiPos.
    void Insert(aeUInt32 uiPos, const char* szInsert);

    //! Replaces the first occurance of szSearchFor after index uiStartPos with szReplaceWith. Returns true if anything was replaced.
    bool Replace(const char* szSearchFor, const char* szReplaceWith, aeUInt32 uiStartPos = 0);
    //! Replaces the first occurance of szSearchFor after index uiStartPos with szReplaceWith. Returns true if anything was replaced. Ignores case.
    bool Replace_NoCase(const char* szSearchFor, const char* szReplaceWith, aeUInt32 uiStartPos = 0);
    //! Replaces the first occurance of szSearchFor after index uiStartPos with szReplaceWith. Returns how many occurances were found and replaced.
    aeUInt32 ReplaceAll(const char* szSearchFor, const char* szReplaceWith, aeUInt32 uiStartPos = 0);
    //! Replaces the first occurance of szSearchFor after index uiStartPos with szReplaceWith. Returns how many occurances were found and replaced. Ignores case.
    aeUInt32 ReplaceAll_NoCase(const char* szSearchFor, const char* szReplaceWith, aeUInt32 uiStartPos = 0);

    //! Searches for a whole word and replaces it. Returns true, if anyting was replaced.
    bool ReplaceWholeWord(const char* szSearchFor, const char* szReplaceWith, aeStringFunctions::AE_IS_WORD_DELIMITER IsDelimiterCB, aeUInt32 uiStartPos = 0);
    //! Searches for a whole word and replaces it. Returns true, if anyting was replaced. Ignores case.
    bool ReplaceWholeWord_NoCase(const char* szSearchFor, const char* szReplaceWith, aeStringFunctions::AE_IS_WORD_DELIMITER IsDelimiterCB, aeUInt32 uiStartPos = 0);
    //! Searches for a whole word and replaces it. Returns how many words were replaced.
    aeUInt32 ReplaceWholeWordAll(const char* szSearchFor, const char* szReplaceWith, aeStringFunctions::AE_IS_WORD_DELIMITER IsDelimiterCB, aeUInt32 uiStartPos = 0);
    //! Searches for a whole word and replaces it. Returns how many words were replaced. Ignores case.
    aeUInt32 ReplaceWholeWordAll_NoCase(const char* szSearchFor, const char* szReplaceWith, aeStringFunctions::AE_IS_WORD_DELIMITER IsDelimiterCB, aeUInt32 uiStartPos = 0);

    //! Converts all characters in this string to upper case.
    void ToUpperCase(void);
    //! Converts all characters in this string to lower case.
    void ToLowerCase(void);

    // TODO
    // split string (delimiters)

  private:
    //! Computes the next best capacity size to use for reallocations. Rounds to the next higher multiple of 32.
    static aeUInt32 ComputeStringCapacity(aeUInt32 uiStringLength);

    //! The current maximum of characters, that this string can hold.
    aeUInt32 m_uiCapacity;
    //! The internal buffer for characters. Used as long as the string fits in there. If necessary a larger buffer is allocated and used instead.
    char m_String[SIZE];
  };

} // namespace AE_NS_FOUNDATION

#include "Inline/HybridString.inl"

#endif
