#ifndef AE_FOUNDATION_TIME_TIME_H
#define AE_FOUNDATION_TIME_TIME_H

#include <KrautGraphics/Base.h>

namespace AE_NS_FOUNDATION
{

  //! A static class that provides the basic functions to query the elapsed time.
  /*! aeTime provides functions to query the current time as reported by the OS and how much time has passed since
      application startup. These timers can be used to update user-interfaces, FPS-counters (which is also provided),
      and other things that are dependent on the "real" time that has passed.
      aeGameTime on the other hand provides timers to handle game times. They allow to slow down and speed up the 
      elapsed time, pause and unpause, etc. There can be any number of aeGameTime instances for various purposes,
      but there is always only one central application time.
      Code can either access an aeGameTime object directly, or it can be set as "active" in aeTime, which allows to
      make it the "current" timer to use for subsequent operations.
      A default aeGameTime is provided, for easier handling, if one global game timer is sufficient.
  */
  class AE_GRAPHICS_DLL aeTime
  {
  public:

    //! Returns the current time as the OS reports it at this moment (in seconds).
    static float GetRealTime (void);

    //! Returns the application time. Useful for updating UI and other elements that are independent of the game-speed.
    static float GetAppTime (void);
    //! Returns the time-difference to the last app-time.
    static float GetAppTimeDiff (void);

    //! Updates the app-tme. Has to be called once each frame. Is automatically called if the global event "aeFrameBegin" is sent.
    static void Update (void);

    //! Returns how many updates were done in the last second. Can be used to show an FPS counter.
    static aeInt32 GetUpdatesPerSecond (void);
  };
}

#endif
