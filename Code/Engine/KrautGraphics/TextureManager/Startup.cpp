// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#include "Main.h"
#include <KrautFoundation/Configuration/Startup.h>
#include <KrautFoundation/Communication/GlobalEvent.h>



namespace AE_NS_GRAPHICS
{
	class aeTextureManagerStartup
	{
	public:
		static void CoreStartup (void) 
		{
			//aeTextureResource::RegisterResourceType ();
			aeTextureManager::CoreStartup ();
		}

		static void EngineStartup (void) 
		{
			aeTextureManager::EngineStartup ();
		}

		static void EngineShutdown (void) 
		{
			aeTextureManager::EngineShutdown ();

			aeTextureResource::CheckResourcesOfTypeAreUnloaded ();
		}
	};

  AE_ON_GLOBAL_EVENT_ONCE (aeStartup_StartupCore_Begin)
  {
    aeStartup::RegisterModule ("aeTextureManager", aeTextureManagerStartup::CoreStartup, NULL, aeTextureManagerStartup::EngineStartup, aeTextureManagerStartup::EngineShutdown, "aeFoundation", "aeResourceManager");
  }
}


