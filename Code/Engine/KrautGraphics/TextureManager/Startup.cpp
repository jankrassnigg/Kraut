// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#include "Main.h"
#include <KrautGraphics/Communication/GlobalEvent.h>
#include <KrautGraphics/Configuration/Startup.h>

namespace AE_NS_GRAPHICS
{
  class aeTextureManagerStartup
  {
  public:
    static void CoreStartup(void)
    {
      //aeTextureResource::RegisterResourceType ();
      aeTextureManager::CoreStartup();
    }

    static void EngineStartup(void)
    {
      aeTextureManager::EngineStartup();
    }

    static void EngineShutdown(void)
    {
      aeTextureManager::EngineShutdown();

      aeTextureResource::CheckResourcesOfTypeAreUnloaded();
    }
  };

  AE_ON_GLOBAL_EVENT_ONCE(aeStartup_StartupCore_Begin)
  {
    aeStartup::RegisterModule("aeTextureManager", aeTextureManagerStartup::CoreStartup, nullptr, aeTextureManagerStartup::EngineStartup, aeTextureManagerStartup::EngineShutdown, "aeFoundation", "aeResourceManager");
  }
} // namespace AE_NS_GRAPHICS
