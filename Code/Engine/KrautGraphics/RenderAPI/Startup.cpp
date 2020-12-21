// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#include "Main.h"
#include <KrautFoundation/Configuration/Startup.h>
#include <KrautFoundation/Communication/GlobalEvent.h>




namespace AE_NS_GRAPHICS
{
  class aeRenderAPIStartup
  {
  public:

    static void EngineStartup (void) 
    {
      aeRenderAPI::ProgramStartup ();
    }

    static void EngineShutdown (void) 
    {
      aeRenderAPI::ProgramShutdown ();
    }

  };

  AE_ON_GLOBAL_EVENT_ONCE (aeStartup_StartupCore_Begin)
  {
    aeStartup::RegisterModule ("aeRenderAPI", NULL, NULL, aeRenderAPIStartup::EngineStartup, aeRenderAPIStartup::EngineShutdown, "aeFoundation");
  }
}


