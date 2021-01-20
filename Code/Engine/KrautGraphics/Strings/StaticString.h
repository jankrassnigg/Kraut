#ifndef AE_FOUNDATION_STRINGS_STATICSTRING_H
#define AE_FOUNDATION_STRINGS_STATICSTRING_H

#include <KrautFoundation/Strings/BasicString.h>

namespace AE_NS_FOUNDATION
{
  //! A string class with a fixed number of characters. No dynamic allocations are done.
  /*! Fixed strings can hold exactly SIZE-1 characters plus the \0 terminator.
      The memory is static and no dynamic allocations are done. Thus fixed strings are
      useful when one wants to trade more memory consumption for less memory allocations.
      For example fixed strings can be embedded in classes to store a "Name" or similar
      attributes without the need to do dynamic allocations. All strings assigned to it
      will be clamped in length to the maximum possible length. In return no dynamic
      memory allocations are done at all, which can often be desirable.\n
      \n
      Since fixed strings cannot grow indefinitely in size, they do not implement certain
      functionality, such as search & replace, as they make little sense under these conditions.
  */
  template <aeUInt16 SIZE>
  class aeStaticString : public aeBasicString
  {
  public:
    //! Initializes the string to be empty.
    aeStaticString(void);
    //! Initializes the string with a copy of the given string.
    aeStaticString(const aeBasicString& cc);
    //! Initializes the string with a copy of the given string.
    aeStaticString(const aeStaticString<SIZE>& cc);
    //! Initializes the string with a copy of the given string.
    aeStaticString(const char* cc);
    //! Initializes the string with a copy of the given string, but only up to uiMaxChars characters.
    aeStaticString(const char* cc, aeUInt32 uiMaxChars);

    //! Resizes the string to the given number of characters.
    void resize(aeUInt16 uiNewSize);

    //! Returns how many characters the string can hold. Will always return SIZE-1.
    aeUInt16 capacity(void) const { return (SIZE - 1); }

    //! Resets the string to be empty.
    void clear(void) { resize(0); }

    //! Appends the given string to this string.
    void Concatenate(const char* szSource);
    //! Appends up to uiCharsToCopy characters of the given string to this string.
    void Concatenate(const char* szSource, aeUInt32 uiCharsToCopy);

    //! Creates a formatted string. See the C-function sprintf for details.
    void Format(const char* szFormat, ...);
    //! Creates a formatted string. See the C-function sprintf for details.
    void FormatArgs(const char* szFormat, va_list ap);

    //! Converts all characters in this string to upper case.
    void ToUpperCase(void);
    //! Converts all characters in this string to lower case.
    void ToLowerCase(void);

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
    void operator=(const aeStaticString<SIZE>& str);
    //! Copies the given string into this one.
    void operator=(const aeBasicString& str);
    //! Copies the given string into this one.
    void operator=(const char* s);
    //! Copies the given character into this one.
    void operator=(char c);


  private:
    //! The fixed-size string buffer.
    char m_String[SIZE];
  };

} // namespace AE_NS_FOUNDATION

#include "Inline/StaticString.inl"

#endif
