#ifndef AE_FOUNDATION_LOGGING_VISUALSTUDIOWRITER_H
#define AE_FOUNDATION_LOGGING_VISUALSTUDIOWRITER_H

#include "../Defines.h"
#include "../Logging/Log.h"

namespace AE_NS_FOUNDATION
{
  //! A simple log writer that outputs all log messages to visual studios output window
  class aeLog_VisualStudioWriter
  {
  public:
    //! Register this at aeLog to write all log messages to visual studios output window.
    AE_FOUNDATION_DLL static void LogMessageHandler (void* pPassThrough, const void* pEventData);

  };

}

#endif
