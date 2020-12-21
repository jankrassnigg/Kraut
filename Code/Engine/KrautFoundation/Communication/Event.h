#ifndef AE_FOUNDATION_COMMUNICATION_EVENT_H
#define AE_FOUNDATION_COMMUNICATION_EVENT_H

#include "../Containers/Array.h"

namespace AE_NS_FOUNDATION
{
  //! Notification callback type for events.
  typedef void (*AE_EVENT_NOTIFICATION_CALLBACK)(void* pPassThrough, const void* pEventData);

  struct aeEventReceiver
  {
    AE_EVENT_NOTIFICATION_CALLBACK m_Callback;
    void* m_pPassThrough;
  };

  // Export these instanciations of the template from the DLL
  #ifdef AE_COMPILE_ENGINE_AS_DLL
    #pragma warning (push)
    #pragma warning (disable : 4231)

    AE_EXPIMP_TEMPLATE_CLASS aeArray<aeEventReceiver>;

    #pragma warning (pop)
  #endif



  //! This class allows to propagate events to code that might be interested in them.
  /*! An event is can be anything that "happens" that might be of interest for other code, such
      that it can react on it in some way. It may just log the event or create statistics, or it may purge
      cashes and recompute data. However that code reacts on the event is of no interest to the code
      that produced the event.
      Just create an instance of aeEvent and call "RaiseEvent" on it. Other intersted code needs access to
      the variable (or at least to RegisterEventReceiver / UnregisterEventReceiver) such that it can
      register itself as an interested party. To pass information to the receivers, create your own
      custom struct to package that information and then pass a pointer to that data through RaiseEvent.
      The receivers just need to cast the void-pointer to the proper struct and thus can get all the detailed
      information about the event.
  */
  class AE_FOUNDATION_DLL aeEvent
  {
  public:
    //! This function will broadcast to all registered users, that this event has just happened.
    void RaiseEvent (const void* pEventData);

    //! Allows to register a function as an event receiver. All receivers will be notified in the order that they registered.
    void RegisterEventReceiver (AE_EVENT_NOTIFICATION_CALLBACK callback, void* pPassThrough);
    //! Unregisters a previously registered receiver. It is an error to unregister a receiver that was not registered.
    void UnregisterEventReceiver (AE_EVENT_NOTIFICATION_CALLBACK callback, void* pPassThrough);

  private:
    static aeInt32 m_iRecursionDepth;
    aeArray<aeEventReceiver> m_EventReceivers;
  };
}

#endif
