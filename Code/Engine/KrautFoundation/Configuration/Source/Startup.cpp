#include "../Startup.h"
#include "../../Containers/Map.h"
#include "../../Containers/Array.h"
#include "../../Strings/HybridString.h"
#include "../../Communication/GlobalEvent.h"
#include "../../Logging/Log.h"

namespace AE_NS_FOUNDATION
{
  struct aeModule
  {
    aeHybridString<16> m_sName;
    AE_STARTUP_CALLBACK m_CoreStartup;
    AE_STARTUP_CALLBACK m_CoreShutdown;
    AE_STARTUP_CALLBACK m_EngineStartup;
    AE_STARTUP_CALLBACK m_EngineShutdown;
    aeHybridString<16> m_sDependsOn[8];

    bool m_bCoreStartupDone;
    bool m_bEngineStartupDone;
  };

  static aeDeque<aeModule> g_Modules;

  void aeStartup::ClearAllStartupModules (void)
  {
    g_Modules.clear ();
  }

  // Checks whether there are dependencies to modules which do not exist.
  static void VerifyStartupList (void)
  {
    // go through all modules
    for (aeUInt32 mod = 0; mod < g_Modules.size (); ++mod)
    {
      // for each module go through all dependencies

      for (aeUInt32 dep = 0; dep < 8; ++dep)
      {
        if (!g_Modules[mod].m_sDependsOn[dep].empty ())
        {
          bool bDependencyExists = false;

          // check whether this module exists

          for (aeUInt32 mod2 = 0; mod2 < g_Modules.size (); ++mod2)
          {
            if (g_Modules[mod2].m_sName.CompareEqual_NoCase (g_Modules[mod].m_sDependsOn[dep].c_str ()))
            {
              bDependencyExists = true;
              break;
            }
          }

          AE_CHECK_ALWAYS (bDependencyExists, "The Module \"%s\" has a dependency on Module \"%s\". However this module has not been registered to the system.", g_Modules[mod].m_sName.c_str (), g_Modules[mod].m_sDependsOn[dep].c_str ());
        }
      }
    }
  }

  static aeDeque<aeInt32> CreateSortedStartupList (void)
  {
    VerifyStartupList ();

    aeDeque<aeInt32> Result;


    aeArray<bool> bInserted (g_Modules.size ());
    for (aeUInt32 ui = 0; ui < g_Modules.size (); ++ui)
      bInserted[ui] = false;

    bool bAddedAny = true;

    while (bAddedAny)
    {
      bAddedAny = false;

      for (aeUInt32 module = 0; module < g_Modules.size (); ++module)
      {
        if (bInserted[module])
          continue;

        bool bHasUnfullfilledDep = false;

        for (aeUInt32 dep = 0; dep < 8; ++dep)
        {
          if (g_Modules[module].m_sDependsOn[dep].empty ())
            continue;

          bool bThisDepFullfilled = false;

          for (aeUInt32 ins = 0; ins < Result.size (); ++ins)
          {
            if (g_Modules[Result[ins]].m_sName.CompareEqual_NoCase (g_Modules[module].m_sDependsOn[dep].c_str ()))
            {
              bThisDepFullfilled = true;
              break;
            }
          }

          if (!bThisDepFullfilled)
          {
            bHasUnfullfilledDep = true;
            break;
          }
        }

        if (bHasUnfullfilledDep)
          continue;

        bAddedAny = true;
        bInserted[module] = true;
        Result.push_back (module);
      }
    }

    AE_CHECK_ALWAYS (Result.size () == g_Modules.size (), "Could not sort the startup-dependencies. There seem to be circular dependencies.");

    return (Result);
  }

  void aeStartup::RegisterModule (const char* szModuleName, AE_STARTUP_CALLBACK CoreStartup, AE_STARTUP_CALLBACK CoreShutdown, AE_STARTUP_CALLBACK EngineStartup, AE_STARTUP_CALLBACK EngineShutdown, const char* szDependsOn1, const char* szDependsOn2, const char* szDependsOn3, const char* szDependsOn4, const char* szDependsOn5, const char* szDependsOn6, const char* szDependsOn7, const char* szDependsOn8)
  {
    aeModule m;
    m.m_sName = szModuleName;

    m.m_CoreStartup    = CoreStartup;
    m.m_CoreShutdown   = CoreShutdown;
    m.m_EngineStartup  = EngineStartup;
    m.m_EngineShutdown = EngineShutdown;

    m.m_sDependsOn[0] = szDependsOn1;
    m.m_sDependsOn[1] = szDependsOn2;
    m.m_sDependsOn[2] = szDependsOn3;
    m.m_sDependsOn[3] = szDependsOn4;
    m.m_sDependsOn[4] = szDependsOn5;
    m.m_sDependsOn[5] = szDependsOn6;
    m.m_sDependsOn[6] = szDependsOn7;
    m.m_sDependsOn[7] = szDependsOn8;

    m.m_bCoreStartupDone   = false;
    m.m_bEngineStartupDone = false;

    for (aeUInt32 ui = 0; ui < g_Modules.size (); ++ui)
    {
      if (g_Modules[ui].m_sName == m.m_sName)
      {
        for (aeUInt32 d = 0; d < 8; ++d)
        {
          if (m.m_sDependsOn[d] != g_Modules[ui].m_sDependsOn[d])
          {
            aeLog::Error ("aeStartup::RegisterModule: Module \"%s\" is already registered, but with different dependencies.", m.m_sName.c_str ());
            return;
          }
        }

        //aeLog::Debug ("aeStartup::RegisterModule: \"%s\" already registered", m.m_sName.c_str ());
        return;
      }
    }

    g_Modules.push_back (m);

    aeLog::Debug ("aeStartup::RegisterModule: \"%s\"", szModuleName);
  }

  void aeStartup::StartupCore (void)
  {
    AE_LOG_BLOCK ("aeStartup::StartupCore");

    AE_BROADCAST_EVENT (aeStartup_StartupCore_Begin);

    aeDeque<aeInt32> List = CreateSortedStartupList ();

    aeLog::Dev ("Executing Startup Procedures.");

    for (aeUInt32 ui = 0; ui < List.size (); ++ui)
    {
      aeModule& Mod = g_Modules[List[ui]];

      if (Mod.m_bCoreStartupDone)
        continue;

      Mod.m_bCoreStartupDone = true;

      if (Mod.m_CoreStartup)
      {
        aeLog::Log ("Starting up Module \"%s\"", Mod.m_sName.c_str ());
        Mod.m_CoreStartup ();
      }
    }

    AE_BROADCAST_EVENT (aeStartup_StartupCore_End);
  }

  void aeStartup::ShutdownCore (void)
  {
    AE_LOG_BLOCK ("aeStartup::ShutdownCore");

    ShutdownEngine ();

    AE_BROADCAST_EVENT (aeStartup_ShutdownCore_Begin);

    aeDeque<aeInt32> List = CreateSortedStartupList ();

    aeLog::Dev ("Executing Shutdown Procedures.");

    for (aeInt32 ui = (aeInt32) List.size () - 1; ui >= 0; --ui)
    {
      aeModule& Mod = g_Modules[List[ui]];

      if (!Mod.m_bCoreStartupDone)
        continue;

      Mod.m_bCoreStartupDone = false;

      if (Mod.m_CoreShutdown)
      {
        aeLog::Log ("Shutting down Module \"%s\"", Mod.m_sName.c_str ());
        Mod.m_CoreShutdown ();
      }
    }

    g_Modules.clear ();

    AE_BROADCAST_EVENT (aeStartup_ShutdownCore_End);
  }

  void aeStartup::StartupEngine (void)
  {
    AE_LOG_BLOCK ("aeStartup::StartupEngine");

    StartupCore ();

    AE_BROADCAST_EVENT (aeStartup_StartupEngine_Begin);

    aeDeque<aeInt32> List = CreateSortedStartupList ();

    aeLog::Dev ("Executing Startup Procedures.");

    for (aeUInt32 ui = 0; ui < List.size (); ++ui)
    {
      aeModule& Mod = g_Modules[List[ui]];

      if (Mod.m_bEngineStartupDone)
        continue;

      Mod.m_bEngineStartupDone = true;

      if (Mod.m_EngineStartup)
      {
        aeLog::Log ("Starting up Module \"%s\"", Mod.m_sName.c_str ());
        Mod.m_EngineStartup ();
      }
    }

    AE_BROADCAST_EVENT (aeStartup_StartupEngine_End);
  }

  void aeStartup::ShutdownEngine (void)
  {
    AE_LOG_BLOCK ("aeStartup::ShutdownEngine");

    AE_BROADCAST_EVENT (aeStartup_ShutdownEngine_Begin);

    aeDeque<aeInt32> List = CreateSortedStartupList ();

    aeLog::Dev ("Executing Shutdown Procedures.");

    for (aeInt32 ui = (aeInt32) List.size () - 1; ui >= 0; --ui)
    {
      aeModule& Mod = g_Modules[List[ui]];

      if (!Mod.m_bEngineStartupDone)
        continue;

      Mod.m_bEngineStartupDone = false;

      if (Mod.m_EngineShutdown)
      {
        aeLog::Log ("Shutting down Module \"%s\"", Mod.m_sName.c_str ());
        Mod.m_EngineShutdown ();
      }
    }

    AE_BROADCAST_EVENT (aeStartup_ShutdownEngine_End);
  }
}

