#include "../GlobalEvent.h"
#include "../../Logging/Log.h"
#include "../../Containers/Map.h"
#include "../../Containers/Deque.h"
#include "../../Threading/CriticalSection.h"



namespace AE_NS_FOUNDATION
{
  static aeMap<aeHybridString<32>, aeDeque<AE_GLOBAL_EVENT_RECEIVER> >* s_pGlobalEvents = NULL;
  static aeMap<aeHybridString<32>, aeDeque<AE_GLOBAL_EVENT_RECEIVER> >* s_pGlobalEventsOnce = NULL;

  aeGlobalEvent::aeGlobalEvent (const char* szEventName, AE_GLOBAL_EVENT_RECEIVER Receiver, bool bHandleEventOnlyOnce)
  {
    if (bHandleEventOnlyOnce)
    {
      if (s_pGlobalEventsOnce == NULL)
        s_pGlobalEventsOnce = new aeMap<aeHybridString<32>, aeDeque<AE_GLOBAL_EVENT_RECEIVER> > ();

      (*s_pGlobalEventsOnce)[szEventName].push_back (Receiver);
    }
    else
    {
      if (s_pGlobalEvents == NULL)
        s_pGlobalEvents = new aeMap<aeHybridString<32>, aeDeque<AE_GLOBAL_EVENT_RECEIVER> > ();

      (*s_pGlobalEvents)[szEventName].push_back (Receiver);
    }
  }

  aeGlobalEvent::~aeGlobalEvent ()
  {
  }

  static aeCriticalSection s_BroadcastEvent;

  void aeGlobalEvent::BroadcastEvent (const char* szEvent, aeGlobalEventParam param0, aeGlobalEventParam param1, aeGlobalEventParam param2, aeGlobalEventParam param3)
  {
    AE_CRITICALSECTION_BLOCK (s_BroadcastEvent);

    static aeInt32 iRecursionDepth = 0;
    ++iRecursionDepth;

    AE_CHECK_DEV (iRecursionDepth < 4, "aeGlobalEvent::BroadcastEvent: The recursion depth of global events has become too complex.");

    AE_LOG_BLOCK (szEvent);
    //aeLog::DebugRegular ("GlobalEvent: \"%s\"", szEvent);

    if (s_pGlobalEventsOnce)
    {
      aeDeque<AE_GLOBAL_EVENT_RECEIVER>& rec = (*s_pGlobalEventsOnce)[szEvent];

      const aeUInt32 uiSize = rec.size ();
      for (aeUInt32 i = 0; i < uiSize; ++i)
        rec[i] (param0, param1, param2, param3);

      rec.clear ();
    }

    if (!s_pGlobalEvents)
      return;

    aeDeque<AE_GLOBAL_EVENT_RECEIVER>& rec = (*s_pGlobalEvents)[szEvent];

    const aeUInt32 uiSize = rec.size ();
    for (aeUInt32 i = 0; i < uiSize; ++i)
      rec[i] (param0, param1, param2, param3);

    --iRecursionDepth;
  }


}



