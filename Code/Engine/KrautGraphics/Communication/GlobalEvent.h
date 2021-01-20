#ifndef AE_FOUNDATION_COMMUNICATION_GLOBALEVENT_H
#define AE_FOUNDATION_COMMUNICATION_GLOBALEVENT_H

#include <KrautGraphics/Base.h>
#include <KrautFoundation/Containers/Array.h>

namespace AE_NS_FOUNDATION
{
  //! One message parameter. Allows to package several pieces of data.
  union AE_GRAPHICS_DLL aeGlobalEventParam
  {
    aeGlobalEventParam(void) { Ptr = nullptr; }
    aeGlobalEventParam(void* p) { Ptr = p; }
    aeGlobalEventParam(aeInt32 i1) { iInt32 = i1; }
    aeGlobalEventParam(aeInt16 i1, aeInt16 i2)
    {
      iInt16[0] = i1;
      iInt16[1] = i2;
    }
    aeGlobalEventParam(float f1) { Float = f1; }
    aeGlobalEventParam(const char* sz) { Text = sz; }
    aeGlobalEventParam(aeUInt8 i1, aeUInt8 i2, aeUInt8 i3, aeUInt8 i4)
    {
      uiByte[0] = i1;
      uiByte[1] = i2;
      uiByte[2] = i3;
      uiByte[3] = i4;
    }

    void* Ptr;
    const char* Text;
    float Float;
    aeInt32 iInt32;
    aeInt16 iInt16[2];
    aeUInt8 uiByte[4];
  };


  //! Function-Pointer for a function that is called whenever a global event is sent throughout the system.
  typedef void (*AE_GLOBAL_EVENT_RECEIVER)(aeGlobalEventParam param0, aeGlobalEventParam param1, aeGlobalEventParam param2, aeGlobalEventParam param3);

  //! A class to handle global (system-wide) events.
  /*! A global event is an event that will be sent to all instances of aeGlobalEvent (or better, their
      respective receiver functions), without the need to first register these event-handlers anywhere.
      Thus they are very useful to notify sub-systems of certain important events, such as that some kind of
      initialization will be done shortly, which means they can react by preparing properly.
      For example the aeStartup-class allows to do initialization of sub-systems in the proper order by first
      gathering dependencies and then executing initialization steps in the proper order. However to implement
      that, all sub-systems have to register themselves first.
      This registration can be done manually, but it can also be automated, by using an aeGlobalEvent to listen
      for aeStartup's startup-event and then registering all objects to aeStartup that should be initialized by it.
      aeGlobalEvent's should be used when there is a kind of event that should be propagated throughout the entire
      engine, without knowledge which systems might want to know about it. These systems can then use an
      aeGlobalEvent-instance to hook themselves into the global-event pipeline and react accordingly.
      Global events should mostly be used for startup / configuration / shutdown procedures, and never for
      runtime events, since all events are passed unfiltered to all receivers, which will do string-comparisons
      to filter out unwanted events.
  */
  class AE_GRAPHICS_DLL aeGlobalEvent
  {
  public:
    //! Instances of this type are initialized with a callback function that is called whenever a global event occurs.
    aeGlobalEvent(const char* szEventName, AE_GLOBAL_EVENT_RECEIVER Receiver, bool bHandleEventOnlyOnce);
    //! Destructor.
    ~aeGlobalEvent();

    //! Static function to send a global event to all aeGlobalEvent-instances.
    static void BroadcastEvent(const char* szEvent,
      aeGlobalEventParam param0 = aeGlobalEventParam(),
      aeGlobalEventParam param1 = aeGlobalEventParam(),
      aeGlobalEventParam param2 = aeGlobalEventParam(),
      aeGlobalEventParam param3 = aeGlobalEventParam());
  };

// Use this macro to broadcast an event. Pass 0 to 4 parameters of type aeGlobalEventParam to it.
#define AE_BROADCAST_EVENT(name, ...) aeGlobalEvent::BroadcastEvent(#name, __VA_ARGS__);

// Use this macro to handle an event (place function code in curly brackets after it)
#define AE_ON_GLOBAL_EVENT(name)                                                                                                               \
  static void EventHandler_##name(aeGlobalEventParam param0, aeGlobalEventParam param1, aeGlobalEventParam param2, aeGlobalEventParam param3); \
  static aeGlobalEvent s_RegisterEventHandler_##name(#name, EventHandler_##name, false);                                                       \
  static void EventHandler_##name(aeGlobalEventParam param0, aeGlobalEventParam param1, aeGlobalEventParam param2, aeGlobalEventParam param3)


#define AE_ON_GLOBAL_EVENT_ONCE(name)                                                                                                          \
  static void EventHandler_##name(aeGlobalEventParam param0, aeGlobalEventParam param1, aeGlobalEventParam param2, aeGlobalEventParam param3); \
  static aeGlobalEvent s_RegisterEventHandler_##name(#name, EventHandler_##name, true);                                                        \
  static void EventHandler_##name(aeGlobalEventParam param0, aeGlobalEventParam param1, aeGlobalEventParam param2, aeGlobalEventParam param3)
} // namespace AE_NS_FOUNDATION

#endif
