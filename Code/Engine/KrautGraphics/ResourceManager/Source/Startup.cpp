// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#include <KrautGraphics/Communication/GlobalEvent.h>
#include <KrautGraphics/Configuration/Startup.h>
#include <KrautGraphics/ResourceManager/ResourceManager.h>

namespace AE_NS_FOUNDATION
{
  class aeResourceManagerStartup
  {
  public:
    static void CoreShutdown(void)
    {
      aeResourceManager::ProgramShutdown();
    }
  };

  AE_ON_GLOBAL_EVENT_ONCE(aeStartup_StartupCore_Begin)
  {
    aeStartup::RegisterModule("aeResourceManager", nullptr, aeResourceManagerStartup::CoreShutdown, nullptr, nullptr, "aeFoundation");
  }
} // namespace AE_NS_FOUNDATION
