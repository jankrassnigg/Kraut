#ifndef AE_FOUNDATION_UTILITY_ENUM_H
#define AE_FOUNDATION_UTILITY_ENUM_H

#include <KrautFoundation/Defines.h>

namespace AE_NS_FOUNDATION
{
  //! A simple class to store an enum value and enforce its size in bytes.
  /*! Whenever you want to store an enum somewhere, simply use this class instead of the enum type directly.
      It allows you to additionally define the base type that represents the enum, ie. whether it is stored
      with 8 bit (unsigned char / aeUInt8), 16 bit (unsigned short / aeUInt16) or 32 bit (unsigned int / aeUInt32).
      For enums that can have negative values a signed type should be used, though.

      The class provides constructors and operators to seamlessly replace an enum type and interact with code
      without being noticed.

      Usage:

      enum ABC { A, B, C };

      aeEnum<ABC, aeUInt8>      MyEnum;   // Will be initialized with A
      aeEnum<ABC, aeUInt16, B>  MyEnum2;  // Will be initialized with B
  */
  template <class ENUM, class STORAGE, ENUM enum_default = (ENUM)0>
  class aeEnum
  {
  public:
    //! Default constructor initializes the data to the template-default.
    inline aeEnum()
      : m_Enum(enum_default)
    {
    }

    //! Copy constructor(s)
    template <ENUM def>
    inline aeEnum(const aeEnum<ENUM, STORAGE, def>& rhs)
      : m_Enum(rhs.m_Enum)
    {
    }

    //! Initializes the enum with the given value.
    inline aeEnum(ENUM value)
      : m_Enum(value)
    {
    }

    //! Assignment operator from another aeEnum
    template <ENUM def>
    inline void operator=(const aeEnum<ENUM, STORAGE, def>& rhs)
    {
      m_Enum = rhs.m_Enum;
    }

    //! Assignment operator from an ENUM
    inline void operator=(ENUM value)
    {
      m_Enum = value;
    }

    //! Operator to implicitly cast to an ENUM.
    inline operator ENUM() const
    {
      return (ENUM)m_Enum;
    }

    //! Operator to implicitly cast to an int.
    inline operator STORAGE() const
    {
      return m_Enum;
    }

  private:
    //! The data is stored as some fixed-size int type.
    STORAGE m_Enum;
  };
} // namespace AE_NS_FOUNDATION

#endif
