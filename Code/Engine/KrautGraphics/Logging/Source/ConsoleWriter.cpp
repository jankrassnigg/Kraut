#include "../ConsoleWriter.h"


//#ifdef AE_PLATFORM_WINDOWS
  #include <windows.h>

  void SetConsoleColor (WORD ui)
  {
    SetConsoleTextAttribute (GetStdHandle (STD_OUTPUT_HANDLE), ui);
  }
//#else
//  void SetConsoleColor (aeUInt8 ui) { }
//#endif

namespace AE_NS_FOUNDATION
{
  void aeLog_ConsoleWriter::LogMessageHandler (void* pPassThrough, const void* pEventData)
  {
    aeLog::aeLoggingEvent* le = (aeLog::aeLoggingEvent*) pEventData;

    if (le->m_EventType == aeLogEventType::BeginGroup)
      printf ("\n");

    for (aeUInt32 i = 0; i < le->m_uiIntendation; ++i)
      printf (" ");

    switch (le->m_EventType)
    {
    case aeLogEventType::FlushToDisk:
      break;
    case aeLogEventType::BeginGroup:
      SetConsoleColor (0x02);
      printf ("+++++ %s +++++\n", le->m_szText);
      break;
    case aeLogEventType::EndGroup:
      SetConsoleColor (0x02);
      printf ("----- %s -----\n\n", le->m_szText);
      break;
    case aeLogEventType::FatalErrorMsg:
      SetConsoleColor (0x0D);
      printf ("Fatal Error: %s\n", le->m_szText);
      break;
    case aeLogEventType::ErrorMsg:
      SetConsoleColor (0x0C);
      printf ("Error: %s\n", le->m_szText);
      break;
    case aeLogEventType::SeriousWarningMsg:
      SetConsoleColor (0x0C);
      printf ("Seriously: %s\n", le->m_szText);
      break;
    case aeLogEventType::WarningMsg:
      SetConsoleColor (0x0E);
      printf ("Warning: %s\n", le->m_szText);
      break;
    case aeLogEventType::SuccessMsg:
      SetConsoleColor (0x0A);
      printf ("%s\n", le->m_szText);
      break;
    case aeLogEventType::LogMsg:
      SetConsoleColor (0x07);
      printf ("%s\n", le->m_szText);
      break;
    case aeLogEventType::DevMsg:
      SetConsoleColor (0x08);
      printf ("%s\n", le->m_szText);
      break;
    case aeLogEventType::DebugMsg:
    case aeLogEventType::DebugRegularMsg:
      SetConsoleColor (0x09);
      printf ("%s\n", le->m_szText);
      break;
    default:
      SetConsoleColor (0x0D);
      printf ("%s\n", le->m_szText);

      aeLog::Warning ("Unknown Message Type %d", le->m_EventType);
      break;
    }

    SetConsoleColor (0x07);
  }
}




