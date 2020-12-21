#ifndef AE_FOUNDATION_GENERAL_TYPES_H
#define AE_FOUNDATION_GENERAL_TYPES_H

  #ifndef AE_FOUNDATION_GENERAL_DEFINES_H
    #error "Please do not include this file directly, but include 'Defines.h' instead."
  #endif

  // ***** Definition of types *****

  typedef unsigned char     aeUInt8;
  typedef unsigned short    aeUInt16;
  typedef unsigned int      aeUInt32;
  typedef unsigned __int64  aeUInt64;

  typedef char              aeInt8;
  typedef short             aeInt16;
  typedef int               aeInt32;
  typedef __int64           aeInt64;

  // no float-types, since those are well portable


  // Do some compile-time checks on the types
  AE_CHECK_COMPILETIME (sizeof (bool)     == 1);
  AE_CHECK_COMPILETIME (sizeof (char)     == 1);
  AE_CHECK_COMPILETIME (sizeof (float)    == 4);
  AE_CHECK_COMPILETIME (sizeof (double)   == 8);
  AE_CHECK_COMPILETIME (sizeof (aeUInt8)  == 1);
  AE_CHECK_COMPILETIME (sizeof (aeUInt16) == 2);
  AE_CHECK_COMPILETIME (sizeof (aeUInt32) == 4);
  AE_CHECK_COMPILETIME (sizeof (aeUInt64) == 8);
  AE_CHECK_COMPILETIME (sizeof (aeInt8)   == 1);
  AE_CHECK_COMPILETIME (sizeof (aeInt16)  == 2);
  AE_CHECK_COMPILETIME (sizeof (aeInt32)  == 4);
  AE_CHECK_COMPILETIME (sizeof (aeInt64)  == 8);

  #ifdef AE_PLATFORM_64BIT
    AE_CHECK_COMPILETIME (sizeof (void*)  == 8);
  #else
    AE_CHECK_COMPILETIME (sizeof (void*)  == 4);
  #endif

  enum aeResultEnum
  {
    AE_FAILURE,
    AE_SUCCESS
  };

  //! Default enum for returning failure or success, instead of using a bool.
  struct aeResult
  {
    aeResult (aeResultEnum res) : e (res) {}

    bool operator== (aeResultEnum cmp) const { return e == cmp; }
    bool operator!= (aeResultEnum cmp) const { return e != cmp; }

  private:
    aeResultEnum e;
  };

#endif

