// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#include "Main.h"
#include <KrautGraphics/Communication/GlobalEvent.h>
#include <KrautGraphics/Configuration/Startup.h>


namespace AE_NS_GRAPHICS
{
  class aeRenderAPIStartup
  {
  public:
    static void EngineStartup(void)
    {
      aeRenderAPI::ProgramStartup();
    }

    static void EngineShutdown(void)
    {
      aeRenderAPI::ProgramShutdown();
    }
  };

  AE_ON_GLOBAL_EVENT_ONCE(aeStartup_StartupCore_Begin)
  {
    aeStartup::RegisterModule("aeRenderAPI", nullptr, nullptr, aeRenderAPIStartup::EngineStartup, aeRenderAPIStartup::EngineShutdown, "aeFoundation");
  }
} // namespace AE_NS_GRAPHICS
