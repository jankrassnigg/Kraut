#ifndef AE_FOUNDATION_LOGGING_CONSOLEWRITER_H
#define AE_FOUNDATION_LOGGING_CONSOLEWRITER_H

#include <KrautFoundation/Defines.h>
#include <KrautGraphics/Base.h>
#include <KrautGraphics/Logging/Log.h>

namespace AE_NS_FOUNDATION
{
  //! A simple log writer that writes out log messages using printf.
  class aeLog_ConsoleWriter
  {
  public:
    //! Register this at aeLog to write all log messages to the console using printf.
    AE_GRAPHICS_DLL static void LogMessageHandler(void* pPassThrough, const void* pEventData);
  };

} // namespace AE_NS_FOUNDATION

#endif
