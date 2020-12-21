#ifndef AE_FOUNDATION_USERCONFIG_H
#define AE_FOUNDATION_USERCONFIG_H

  // This could be used through cmake or something, to make sure the user-config is not used
  #ifndef AE_IGNORE_USER_CONFIG

  // define whatever you want in this header
  // it allows you to configure the foundation library as you like
  // this is especially useful if you are not using cmake to set it up


  // *** Platform Defines ***

    #define AE_PLATFORM_WINDOWS
    // #define AE_PLATFORM_LINUX

#ifdef _WIN64
    #define AE_PLATFORM_64BIT
#else
    #define AE_PLATFORM_32BIT
#endif

  // *** Compiling ***

    #define AE_COMPILE_FOR_DEVELOPMENT
    //#define AE_COMPILE_FOR_DEBUG
    // #define AE_COMPILE_FOR_PROFILING

  // *** Linking ***

    #define AE_COMPILE_ENGINE_AS_DLL

  #endif

#endif

