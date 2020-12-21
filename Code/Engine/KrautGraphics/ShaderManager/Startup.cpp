// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#include "Main.h"
#include <KrautFoundation/Configuration/Startup.h>
#include <KrautFoundation/Communication/GlobalEvent.h>



namespace AE_NS_GRAPHICS
{
	class aeShaderManagerStartup
	{
	public:
		static void CoreStartup (void) 
		{
			//aeShaderResource::RegisterResourceType ();
		}

		static void EngineStartup (void) 
		{
			aeShaderManager::ProgramStartup ();
		}

		static void EngineShutdown (void)
		{
			aeShaderManager::ProgramShutdown ();

			aeShaderResource::CheckResourcesOfTypeAreUnloaded ();
		}

	};

  AE_ON_GLOBAL_EVENT_ONCE (aeStartup_StartupCore_Begin)
  {
    aeStartup::RegisterModule ("aeShaderManager", aeShaderManagerStartup::CoreStartup, NULL, aeShaderManagerStartup::EngineStartup, aeShaderManagerStartup::EngineShutdown, "aeFoundation", "aeResourceManager", "aeTextureManager");
  }
}


