#include "../Time.h"

//#ifdef AE_PLATFORM_WINDOWS
#pragma comment(lib, "winmm.lib")
#include <windows.h>
//#endif

#include <KrautFoundation/Basics/Checks.h>

namespace AE_NS_FOUNDATION
{
  static float s_fAppTime = 0.0;
  static float s_fAppTimeDiff = 0.0;
  static float s_fLastUpdate = 0.0;

  float aeTime::GetRealTime(void)
  {
    //#ifdef AE_PLATFORM_WINDOWS
    const aeUInt32 ui = timeGetTime();
    return (ui / 1000.0f);
    //#endif

    AE_CHECK_ALWAYS(false, "aeTime::GetRealTime is not implemented for this platform.");
  }

  static float s_fLastFPSUpdate = 0.0;
  static aeInt32 s_iCountedUpdates = 0;
  static aeInt32 s_iCurrentFPS = 0;

  void aeTime::Update(void)
  {
    const float fTime = GetRealTime();

    if (s_fLastUpdate == 0.0f)
    {
      s_fLastUpdate = fTime;
      return;
    }

    s_fAppTimeDiff = fTime - s_fLastUpdate;
    s_fLastUpdate = fTime;
    s_fAppTime += s_fAppTimeDiff;

    if (fTime - s_fLastFPSUpdate >= 1.0f)
    {
      s_fLastFPSUpdate = fTime;
      s_iCurrentFPS = s_iCountedUpdates;
      s_iCountedUpdates = 0;
    }

    ++s_iCountedUpdates;
  }

  aeInt32 aeTime::GetUpdatesPerSecond(void)
  {
    return (s_iCurrentFPS);
  }

  float aeTime::GetAppTime(void)
  {
    return (s_fAppTime);
  }

  float aeTime::GetAppTimeDiff(void)
  {
    return (s_fAppTimeDiff);
  }

} // namespace AE_NS_FOUNDATION
