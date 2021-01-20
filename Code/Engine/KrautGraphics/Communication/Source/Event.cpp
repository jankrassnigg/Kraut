#include "../Event.h"


namespace AE_NS_FOUNDATION
{
  aeInt32 aeEvent::m_iRecursionDepth = 0;

  /*! A callback can be registered multiple times with different pass-through data (or even with the same,
      though that is less useful).
  */
  void aeEvent::RegisterEventReceiver (AE_EVENT_NOTIFICATION_CALLBACK callback, void* pPassThrough)
  {
    AE_CHECK_DEV (callback != nullptr, "aeEvent::RegisterEventReceiver: Callback may not be nullptr."); 

    m_EventReceivers.SetSizeIncrementPolicy_Add (4);

    aeEventReceiver er;
    er.m_Callback = callback;
    er.m_pPassThrough = pPassThrough;

    m_EventReceivers.push_back (er);
  }

  /*! Use exactly the same combination of callback/pass-through-data to unregister a receiver.
      Otherwise an error occurs.
  */
  void aeEvent::UnregisterEventReceiver (AE_EVENT_NOTIFICATION_CALLBACK callback, void* pPassThrough)
  {
    AE_CHECK_DEV (callback != nullptr, "aeEvent::UnregisterEventReceiver: Callback may not be nullptr.");

    aeUInt32 ui = 0;
    for (; ui < m_EventReceivers.size (); ++ui)
    {
      if ((m_EventReceivers[ui].m_Callback == callback) && (m_EventReceivers[ui].m_pPassThrough == pPassThrough))
        goto found;
    }

    AE_CHECK_DEV (false, "aeEvent::UnregisterEventReceiver: Callback %p with pass-through data %p has not been registered or already been unregistered.", callback, pPassThrough);
    return;

found:

    for (; ui < m_EventReceivers.size () - 1; ++ui)
      m_EventReceivers[ui] = m_EventReceivers[ui + 1];

    m_EventReceivers.pop_back ();
  }

  /*! The notification is sent to all receivers in the order that they were registered.
  */
  void aeEvent::RaiseEvent (const void* pEventData)
  {
    AE_CHECK_DEV (m_iRecursionDepth < 16, "aeEvent::RaiseEvent: Recursion depth has reached %d.", m_iRecursionDepth);

    ++m_iRecursionDepth;

    for (aeUInt32 ui = 0; ui < m_EventReceivers.size (); ++ui)
      m_EventReceivers[ui].m_Callback (m_EventReceivers[ui].m_pPassThrough, pEventData);

    --m_iRecursionDepth;
  }
}



