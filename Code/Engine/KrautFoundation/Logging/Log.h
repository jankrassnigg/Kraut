#ifndef AE_FOUNDATION_LOGGING_LOG_H
#define AE_FOUNDATION_LOGGING_LOG_H

#include "../Defines.h"
#include "../Communication/Event.h"
#include "../Containers/Stack.h"
#include "../Strings/HybridString.h"

namespace AE_NS_FOUNDATION
{
  //! Describes the types of events that aeLog sends.
  struct aeLogEventType
  {
    enum Enum
    {
      FlushToDisk         = -3,
      BeginGroup          = -2,
      EndGroup            = -1,
      None                = 0,
      FatalErrorMsg       = 1,
      ErrorMsg            = 2,
      SeriousWarningMsg   = 3,
      WarningMsg          = 4,
      SuccessMsg          = 5,
      LogMsg              = 6,
      DevMsg              = 7,
      DebugMsg            = 8,
      DebugRegularMsg     = 9,
      All                 = 9,
    };
  };

  //! Static class that allows to write out logging information.
  /*! This class takes logging information, prepares it and then broadcasts it to all interested code
      via the event interface. It does not write anything on disk or somewhere else, itself. Instead it
      allows to register custom log writers that can then write it to disk, to console, send it over a
      network or pop up a message box. Whatever suits the current situation.
      Since event handlers can be registered only temporarily, it is also possible to just gather all
      errors that occur during some operation and then unregister the event handler again.
  */
  class AE_FOUNDATION_DLL aeLog
  {
  public:
    //! LogLevel is between aeLogEventType::None and aeLogEventType::All and defines which messages will be logged and which will be filtered out.
    static void SetLogLevel (aeUInt8 LogLevel);

    //! An error that results in program termination.
    static void FatalError (const char* szFormat, ...);
    //! An error that needs to be fixed as soon as possible.
    static void Error (const char* szFormat, ...);
    //! Not an error, but definitely a big problem, that should be looked into very soon.
    static void SeriousWarning (const char* szFormat, ...);
    //! A potential problem or a performance warning. Might be possible to ignore it.
    static void Warning (const char* szFormat, ...);
    //! Status information that something was completed successfully.
    static void Success (const char* szFormat, ...);
    //! Status information that is important.
    static void Log (const char* szFormat, ...);
    //! Status information that is nice to have during development.
    static void Dev (const char* szFormat, ...);
    //! Status information during debugging. Very verbose. Usually only temporarily added to the code.
    static void Debug (const char* szFormat, ...);
    //! Status information during debugging. Very verbose, happening every frame. Usually only temporarily added to the code.
    static void DebugRegular (const char* szFormat, ...);

    //! Begins grouping log messages. Can be nested.
    static void BeginLogBlock (const char* szName);
    //! Ends grouping log messages.
    static void EndLogBlock (void);

    //! Can be called at any time to make sure the log is written to disk.
    static void FlushToDisk ();

    //! The data that is sent through the event interface.
    struct aeLoggingEvent
    {
      //! The type of information that is sent.
      aeLogEventType::Enum m_EventType;
      //! How many "levels" to indent.
      aeUInt32 m_uiIntendation;
      //! The information text.
      const char* m_szText;
      //! An optional tag extracted from the log-string (if it started with "[SomeTag]Logging String.") Can be used by log-writers for additional configuration, or simply be ignored.
      const char* m_szTag;
    };

    //! Allows to register a function as an event receiver. All receivers will be notified in the order that they registered.
    static void RegisterEventReceiver (AE_EVENT_NOTIFICATION_CALLBACK callback, void* pPassThrough)   { m_LoggingEvent.RegisterEventReceiver   (callback, pPassThrough); }
    //! Unregisters a previously registered receiver. It is an error to unregister a receiver that was not registered.
    static void UnregisterEventReceiver (AE_EVENT_NOTIFICATION_CALLBACK callback, void* pPassThrough) { m_LoggingEvent.UnregisterEventReceiver (callback, pPassThrough); }

    //! Returns how many errors occured.
    static aeUInt32 GetErrorCount (void)						{ return (s_uiHadErrors); }
    //! Returns how many serious warnings occured.
    static aeUInt32 GetSeriousWarningCount (void)		{ return (s_uiHadSeriousWarnings); }
    //! Returns how many warnings occured.
    static aeUInt32 GetWarningCount (void)					{ return (s_uiHadWarnings); }

  private:
    struct aeBlockData
    {
      aeHybridString<32> m_sName;
      bool m_bWritten;
    };

    static aeUInt32 s_uiHadErrors;
    static aeUInt32 s_uiHadSeriousWarnings;
    static aeUInt32 s_uiHadWarnings;

    static aeEvent m_LoggingEvent;
    static aeArray<aeBlockData> m_LogBlocks;

    static void WriteString (aeLogEventType::Enum type, const char* szString);
  };

  class AE_FOUNDATION_DLL AE_LOG_BLOCK_OBJECT
  {
  public:
    AE_LOG_BLOCK_OBJECT (const char* szName)
    {
      aeLog::BeginLogBlock (szName);
    }

    ~AE_LOG_BLOCK_OBJECT ()
    {
      aeLog::EndLogBlock ();
    }
  };

  #define AE_LOG_BLOCK AE_LOG_BLOCK_OBJECT _lb_
}

#endif
