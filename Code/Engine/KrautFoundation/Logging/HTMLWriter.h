#ifndef AE_FOUNDATION_LOGGING_HTMLWRITER_H
#define AE_FOUNDATION_LOGGING_HTMLWRITER_H

#include "../Defines.h"
#include "../Logging/Log.h"
#include "../FileSystem/FileOut.h"

namespace AE_NS_FOUNDATION
{
  //! A log writer that writes out log messages to an HTML file.
  class AE_FOUNDATION_DLL aeLog_HTMLWriter
  {
  public:
    ~aeLog_HTMLWriter ();

    //! Register this at aeLog to write all log messages to the console using printf.
    static void LogMessageHandler (void* pPassThrough, const void* pEventData);

    void BeginLog (const char* szFile, const char* szAppTitle);

    void EndLog (void);

    //! Returns the name of the log-file that was really opened. Might be slightly different than what was given to BeginLog, to allow parallel execution of the same application.
    const char* GetOpenedLogFile (void) const;

  private:
    void MessageHandler (const void* pEventData);

    void WriteString (const aeBasicString& s, aeUInt32 uiColor);

    aeFileOut m_File;

  };

}

#endif
