#ifndef AE_FOUNDATION_CONFIGURATION_STARTUP_H
#define AE_FOUNDATION_CONFIGURATION_STARTUP_H

#include "../Defines.h"

namespace AE_NS_FOUNDATION
{
  typedef void (*AE_STARTUP_CALLBACK)(void);

  class AE_FOUNDATION_DLL aeStartup
  {
  public:

    //! Registers a Module by Name, defines its startup/shutdown callbacks, and also, by name, which other modules it depends on (thus they should be initialized first).
    static void RegisterModule (const char* szModuleName, 
      AE_STARTUP_CALLBACK CoreStartup, AE_STARTUP_CALLBACK CoreShutdown, 
      AE_STARTUP_CALLBACK EngineStartup, AE_STARTUP_CALLBACK EngineShutdown,
      const char* szDependsOn1 = NULL, const char* szDependsOn2 = NULL, const char* szDependsOn3 = NULL, const char* szDependsOn4 = NULL, 
      const char* szDependsOn5 = NULL, const char* szDependsOn6 = NULL, const char* szDependsOn7 = NULL, const char* szDependsOn8 = NULL);

    //! Initializes all core modules in their proper order. Sends the global event "aeStartup::StartupCore" first.
    static void StartupCore (void);
    //! Shuts down all core modules in the reversed order in which they were initialized. Sends the global event "aeStartup::ShutdownCore" first.
    static void ShutdownCore (void);

    //! Initializes all engine modules in their proper order. Sends the global event "aeStartup::StartupEngine" first.
    static void StartupEngine (void);
    //! Shuts down all engine modules in the reversed order in which they were initialized. Sends the global event "aeStartup::ShutdownEngine" first.
    static void ShutdownEngine (void);

    //! Removes all modules from the internal list. For testing-purposes.
    static void ClearAllStartupModules (void);
  };

}

#endif

