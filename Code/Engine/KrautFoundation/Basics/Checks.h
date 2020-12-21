#ifndef AE_FOUNDATION_BASICS_CHECKS_H
#define AE_FOUNDATION_BASICS_CHECKS_H

#include "../Defines.h"

namespace AE_NS_FOUNDATION
{
  // Set Warnings as Errors: Too few/many parameters given for Macro
  #pragma warning (error : 4002 4003)

  // Executing AE_DEBUG_BREAK will trigger a debug-break in debug-builds
  // In non-debug builds it will (or should at least) just crash the app
  #ifdef AE_PLATFORM_WINDOWS_64BIT
    #define AE_DEBUG_BREAK { __debugbreak (); }
  #elif defined AE_PLATFORM_WINDOWS_32BIT
    #define AE_DEBUG_BREAK { __asm { int 3 } }
  #else
    #error "AE_DEBUG_BREAK is not yet defined for this platform."
  #endif

  // some compilers do not support this macro
  #ifndef __FUNCTION__
    #define __FUNCTION__ "unknown"
  #endif

  //! Called by the AE_CHECK_ALWAYS Macros whenever a check failed.
  void AE_FOUNDATION_DLL aeFailedCheck (const char* szSourceFile, aeUInt32 uiLine, const char* szFunction, const char* szExpression, const char* szErrorMsg, ...);

  //! Macro to raise an error, if a condition is not met. Allows to write a message using printf style. Is executed even in non-development builds.
  #define AE_CHECK_ALWAYS(bCondition, szErrorMsg, ...)                               if (bCondition == false) aeFailedCheck (__FILE__, __LINE__, __FUNCTION__, #bCondition, szErrorMsg, __VA_ARGS__);

  // Occurances of AE_CHECK_DEV are compiled out in non-development builds
  #ifdef AE_COMPILE_FOR_DEVELOPMENT
    //! Macro to raise an error, if a condition is not met. Allows to write a message using printf style. Compiled out in non-development builds.
    #define AE_CHECK_DEV(bCondition, szErrorMsg, ...)                         if (bCondition == false) aeFailedCheck (__FILE__, __LINE__, __FUNCTION__, #bCondition, szErrorMsg, __VA_ARGS__);
  #else
    //! Macro to raise an error, if a condition is not met. Allows to write a message using printf style. Compiled out in non-development builds.
    #define AE_CHECK_DEV(bCondition, szErrorMsg, ...)                         {}
  #endif
}

#endif
