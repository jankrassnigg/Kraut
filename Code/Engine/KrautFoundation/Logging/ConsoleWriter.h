#ifndef AE_FOUNDATION_LOGGING_CONSOLEWRITER_H
#define AE_FOUNDATION_LOGGING_CONSOLEWRITER_H

#include "../Defines.h"
#include "../Logging/Log.h"

namespace AE_NS_FOUNDATION
{
  //! A simple log writer that writes out log messages using printf.
  class aeLog_ConsoleWriter
  {
  public:
    //! Register this at aeLog to write all log messages to the console using printf.
    AE_FOUNDATION_DLL static void LogMessageHandler (void* pPassThrough, const void* pEventData);

  };

}

#endif
