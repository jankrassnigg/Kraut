#include "../Log.h"
#include "../../Strings/String.h"
#include "../../Configuration/CVar.h"



namespace AE_NS_FOUNDATION
{
  #ifdef AE_COMPILE_FOR_DEVELOPMENT
    static aeCVarInt CVarLogLevel ("log_Level", aeLogEventType::DebugMsg, aeCVarFlags::Save, "[0 - 8] Filter to reduce the amount of logging messages. 0 == no messages ; 8 == all messages.");
  #else
    static aeCVarInt CVarLogLevel ("log_Level", aeLogEventType::LogMsg, aeCVarFlags::None, "[0 - 8] Filter to reduce the amount of logging messages. 0 == no messages ; 8 == all messages.");
  #endif

  aeEvent aeLog::m_LoggingEvent;
  aeArray<aeLog::aeBlockData> aeLog::m_LogBlocks;
  aeUInt32 aeLog::s_uiHadErrors = 0;
  aeUInt32 aeLog::s_uiHadSeriousWarnings = 0;
  aeUInt32 aeLog::s_uiHadWarnings = 0;

  void aeLog::SetLogLevel (aeUInt8 LogLevel)
  {
    CVarLogLevel = aeMath::Clamp<aeInt32> (LogLevel, aeLogEventType::None, aeLogEventType::All);
  }

  void aeLog::BeginLogBlock (const char* szName)
  {
    aeBlockData bd;
    bd.m_sName = szName;
    bd.m_bWritten = false;

    m_LogBlocks.push_back (bd);
  }

  void aeLog::EndLogBlock (void)
  {
    AE_CHECK_DEV (!m_LogBlocks.empty (), "aeLog::EndLogBlock: All log blocks are already finished.");

    if (m_LogBlocks.back ().m_bWritten)
    {
      aeLoggingEvent le;
      le.m_EventType = aeLogEventType::EndGroup;
      le.m_szText = m_LogBlocks.back ().m_sName.c_str ();
      le.m_uiIntendation = m_LogBlocks.size () - 1;
      le.m_szTag = "";

      m_LoggingEvent.RaiseEvent ((void*) &le);
    }

    m_LogBlocks.pop_back ();
  }

  void aeLog::FlushToDisk ()
  {
    aeLoggingEvent le;
    le.m_EventType = aeLogEventType::FlushToDisk;
    le.m_szText = "";
    le.m_uiIntendation = m_LogBlocks.size ();
    le.m_szTag = "";

    m_LoggingEvent.RaiseEvent ((void*) &le);
  }

  void aeLog::WriteString (aeLogEventType::Enum type, const char* szString)
  {
    for (aeUInt32 ui = 0; ui < m_LogBlocks.size (); ++ui)
    {
      if (m_LogBlocks[ui].m_bWritten)
        continue;

      m_LogBlocks[ui].m_bWritten = true;

      aeLoggingEvent le;
      le.m_EventType = aeLogEventType::BeginGroup;
      le.m_szText = m_LogBlocks[ui].m_sName.c_str ();
      le.m_uiIntendation = ui;
      le.m_szTag = "";

      m_LoggingEvent.RaiseEvent ((void*) &le);
    }

    aeHybridString<16> sTag;

    if (aeStringFunctions::StartsWith (szString, "["))
    {
      aeInt32 iPos = 1;
      while ((szString[iPos] != '\0') && (szString[iPos] != '[') && (szString[iPos] != ']'))
      {
        sTag += szString[iPos];
        ++iPos;
      }

      if (szString[iPos] == ']')
        ++iPos;

      szString = &szString[iPos];
    }

    aeLoggingEvent le;
    le.m_EventType = type;
    le.m_szText = szString;
    le.m_uiIntendation = m_LogBlocks.size ();
    le.m_szTag = sTag.c_str ();

    m_LoggingEvent.RaiseEvent ((void*) &le);
  }

  void aeLog::FatalError (const char* szFormat, ...)
  {
    ++s_uiHadErrors;

    if (CVarLogLevel < aeLogEventType::FatalErrorMsg)
      return;

    aeString s;

    va_list ap;
    va_start (ap, szFormat);

    s.FormatArgs (szFormat, ap);

    va_end (ap);

    WriteString (aeLogEventType::FatalErrorMsg, s.c_str ());
  }

  void aeLog::Error (const char* szFormat, ...)
  {
    ++s_uiHadErrors;

    if (CVarLogLevel < aeLogEventType::ErrorMsg)
      return;

    aeString s;

    va_list ap;
    va_start (ap, szFormat);

    s.FormatArgs (szFormat, ap);

    va_end (ap);

    WriteString (aeLogEventType::ErrorMsg, s.c_str ());
  }

  void aeLog::SeriousWarning (const char* szFormat, ...)
  {
    ++s_uiHadSeriousWarnings;

    if (CVarLogLevel < aeLogEventType::SeriousWarningMsg)
      return;

    aeString s;

    va_list ap;
    va_start (ap, szFormat);

    s.FormatArgs (szFormat, ap);

    va_end (ap);

    WriteString (aeLogEventType::SeriousWarningMsg, s.c_str ());

  }

  void aeLog::Warning (const char* szFormat, ...)
  {
    ++s_uiHadWarnings;

    if (CVarLogLevel < aeLogEventType::WarningMsg)
      return;

    aeString s;

    va_list ap;
    va_start (ap, szFormat);

    s.FormatArgs (szFormat, ap);

    va_end (ap);

    WriteString (aeLogEventType::WarningMsg, s.c_str ());
  }

  void aeLog::Success (const char* szFormat, ...)
  {
    if (CVarLogLevel < aeLogEventType::SuccessMsg)
      return;

    aeString s;

    va_list ap;
    va_start (ap, szFormat);

    s.FormatArgs (szFormat, ap);

    va_end (ap);

    WriteString (aeLogEventType::SuccessMsg, s.c_str ());
  }

  void aeLog::Log (const char* szFormat, ...)
  {
    if (CVarLogLevel < aeLogEventType::LogMsg)
      return;

    aeString s;

    va_list ap;
    va_start (ap, szFormat);

    s.FormatArgs (szFormat, ap);

    va_end (ap);

    WriteString (aeLogEventType::LogMsg, s.c_str ());
  }

  void aeLog::Dev (const char* szFormat, ...)
  {
    if (CVarLogLevel < aeLogEventType::DevMsg)
      return;

    aeString s;

    va_list ap;
    va_start (ap, szFormat);

    s.FormatArgs (szFormat, ap);

    va_end (ap);

    WriteString (aeLogEventType::DevMsg, s.c_str ());
  }

  void aeLog::Debug (const char* szFormat, ...)
  {
    if (CVarLogLevel < aeLogEventType::DebugMsg)
      return;

    aeString s;

    va_list ap;
    va_start (ap, szFormat);

    s.FormatArgs (szFormat, ap);

    va_end (ap);

    WriteString (aeLogEventType::DebugMsg, s.c_str ());
  }

  void aeLog::DebugRegular (const char* szFormat, ...)
  {
    if (CVarLogLevel < aeLogEventType::DebugRegularMsg)
      return;

    aeString s;

    va_list ap;
    va_start (ap, szFormat);

    s.FormatArgs (szFormat, ap);

    va_end (ap);

    WriteString (aeLogEventType::DebugRegularMsg, s.c_str ());
  }

}




