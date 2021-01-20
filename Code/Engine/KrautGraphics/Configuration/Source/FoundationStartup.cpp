#include "../../Communication/GlobalEvent.h"
#include "../../Logging/Log.h"
#include "../Startup.h"
#include <KrautFoundation/Containers/Array.h>
#include <KrautFoundation/Containers/Map.h>
#include <time.h>



namespace AE_NS_FOUNDATION
{
  static void CoreStartup(void)
  {
    aeLog::Debug("aeFoundation::CoreStartup");
  }

  static void CoreShutdown(void)
  {
    aeLog::Debug("aeFoundation::CoreShutdown");
  }

  static void EngineStartup(void)
  {
    aeLog::Debug("aeFoundation::EngineStartup");
  }

  static void EngineShutdown(void)
  {
    aeLog::Debug("aeFoundation::EngineShutdown");
  }

  AE_ON_GLOBAL_EVENT_ONCE(aeStartup_StartupCore_Begin)
  {
    aeStartup::RegisterModule("aeFoundation", CoreStartup, CoreShutdown, EngineStartup, EngineShutdown);
  }


} // namespace AE_NS_FOUNDATION
