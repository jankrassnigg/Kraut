#ifndef AE_FOUNDATION_GENERAL_DEFINES_H
#define AE_FOUNDATION_GENERAL_DEFINES_H

  // put all YOUR configuration into this header
  #include "UserConfig.h"

  // class 'type' needs to have dll-interface to be used by clients of class 'type2' -> dll export / import issues (mostly with templates)
  #pragma warning(disable: 4251)

  namespace AE_NS_FOUNDATION
  {
    // just so it exists
  }

  // The following defines are defined by the build-system (cmake)
  // AE_PLATFORM_32BIT | AE_PLATFORM_64BIT
  // AE_PLATFORM_WINDOWS | AE_PLATFORM_LINUX
  // AE_BUILDING_FOUNDATION_DLL (set on all projects in the "Foundation"-Project
  // AE_BUILDING_ENGINE_DLL (set on all projects in the "Engine"-Project
  // AE_COMPILE_FOR_DEVELOPMENT (selected by user)
  // AE_COMPILE_ENGINE_AS_DLL (selected by user)

  // Configure the DLL Import/Export Define
  #ifdef AE_COMPILE_ENGINE_AS_DLL
    #ifdef BUILDSYSTEM_BUILDING_KRAUTFOUNDATION_LIB
      #define AE_FOUNDATION_DLL __declspec(dllexport)
      #define AE_FOUNDATION_TEMPLATE
    #else
      #define AE_FOUNDATION_DLL __declspec(dllimport)
      #define AE_FOUNDATION_TEMPLATE extern
    #endif
  #else
    #define AE_FOUNDATION_DLL
    #define AE_FOUNDATION_TEMPLATE
  #endif

  // Use this as the "linkage" parameter to some macros, if you do not use DLLs as plugins (static linking).
  #define AE_STATIC_LINKAGE


  #define AE_EXPIMP_TEMPLATE_CLASS AE_FOUNDATION_TEMPLATE template class AE_FOUNDATION_DLL
  #define AE_EXPIMP_TEMPLATE_STRUCT AE_FOUNDATION_TEMPLATE template struct AE_FOUNDATION_DLL

  // Configure some convenience defines (32/64 Bit)
  #ifdef AE_PLATFORM_32BIT
    #ifdef AE_PLATFORM_WINDOWS
      #define AE_PLATFORM_WINDOWS_32BIT
    #endif
    #ifdef AE_PLATFORM_LINUX
      #define AE_PLATFORM_LINUX_32BIT
    #endif
  #endif

  // Configure some convenience defines (32/64 Bit)
  #ifdef AE_PLATFORM_64BIT
    #ifdef AE_PLATFORM_WINDOWS
      #define AE_PLATFORM_WINDOWS_64BIT
    #endif
    #ifdef AE_PLATFORM_LINUX
      #define AE_PLATFORM_LINUX_64BIT
    #endif
  #endif

  // AE_COMPILE_FOR_DEBUG is a platform-independent define for the DEBUG-build
  #ifdef _DEBUG
    #define AE_COMPILE_FOR_DEBUG
  #endif

  // not yet used
  //#define AE_COMPILE_FOR_PROFILING


// ***** low level stuff *****

  // make sure NULL is defined
  #ifndef NULL
    #define NULL 0
  #endif

#define AE_INLINE __forceinline
#define AE_RESTRICT __restrict


// ****** Useful Macros ******

  // Macro to do compile-time checks, such as to ensure sizes of types
  #define AE_CHECK_COMPILETIME(exp)	extern char AE_CompileTimeAssert[ (unsigned int)((exp) ? 1 : -1) ]

  #ifdef AE_COMPILE_FOR_DEVELOPMENT
    #define AE_IF_DEVELOPING if (true) 
  #else
    #define AE_IF_DEVELOPING if (false)
  #endif

  #ifdef AE_COMPILE_FOR_PROFILING
    #define AE_IF_PROFILING if (true) 
  #else
    #define AE_IF_PROFILING if (false)
  #endif

  #ifdef AE_COMPILE_FOR_DEBUG
    #define AE_IF_DEBUGGING if (true) 
  #else
    #define AE_IF_DEBUGGING if (false)
  #endif

#include "Types.h"

#endif

