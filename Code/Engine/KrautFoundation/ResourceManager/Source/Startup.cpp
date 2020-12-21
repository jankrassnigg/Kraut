// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#include "../IncludeAll.h"
#include <KrautFoundation/Configuration/Startup.h>
#include <KrautFoundation/Communication/GlobalEvent.h>



namespace AE_NS_FOUNDATION
{
	class aeResourceManagerStartup
	{
	public:
		static void CoreShutdown (void) 
		{
			aeResourceManager::ProgramShutdown ();
		}
	};

  AE_ON_GLOBAL_EVENT_ONCE (aeStartup_StartupCore_Begin)
  {
    aeStartup::RegisterModule ("aeResourceManager", NULL, aeResourceManagerStartup::CoreShutdown, NULL, NULL, "aeFoundation");
  }
}

