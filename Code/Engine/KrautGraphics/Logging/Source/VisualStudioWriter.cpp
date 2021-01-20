#include "../VisualStudioWriter.h"

//#ifdef AE_PLATFORM_WINDOWS

#include <windows.h>

namespace AE_NS_FOUNDATION
{
  void aeLog_VisualStudioWriter::LogMessageHandler(void* pPassThrough, const void* pEventData)
  {
    aeLog::aeLoggingEvent* le = (aeLog::aeLoggingEvent*)pEventData;

    if (le->m_EventType == aeLogEventType::BeginGroup)
      OutputDebugString("\n");

    for (aeUInt32 i = 0; i < le->m_uiIntendation; ++i)
      OutputDebugString(" ");

    char sz[1024];

    switch (le->m_EventType)
    {
      case aeLogEventType::FlushToDisk:
        break;
      case aeLogEventType::BeginGroup:
        //SetConsoleColor (0x02);
        aeStringFunctions::Format(sz, 1024, "+++++ %s +++++\n", le->m_szText);
        OutputDebugString(sz);
        break;
      case aeLogEventType::EndGroup:
        //SetConsoleColor (0x02);
        aeStringFunctions::Format(sz, 1024, "----- %s -----\n\n", le->m_szText);
        OutputDebugString(sz);
        break;
      case aeLogEventType::FatalErrorMsg:
        //SetConsoleColor (0x0D);
        aeStringFunctions::Format(sz, 1024, "Fatal Error: %s\n", le->m_szText);
        OutputDebugString(sz);
        break;
      case aeLogEventType::ErrorMsg:
        //SetConsoleColor (0x0C);
        aeStringFunctions::Format(sz, 1024, "Error: %s\n", le->m_szText);
        OutputDebugString(sz);
        break;
      case aeLogEventType::SeriousWarningMsg:
        //SetConsoleColor (0x0C);
        aeStringFunctions::Format(sz, 1024, "Seriously: %s\n", le->m_szText);
        OutputDebugString(sz);
        break;
      case aeLogEventType::WarningMsg:
        //SetConsoleColor (0x0E);
        aeStringFunctions::Format(sz, 1024, "Warning: %s\n", le->m_szText);
        OutputDebugString(sz);
        break;
      case aeLogEventType::SuccessMsg:
        //SetConsoleColor (0x0A);
        aeStringFunctions::Format(sz, 1024, "%s\n", le->m_szText);
        OutputDebugString(sz);
        break;
      case aeLogEventType::LogMsg:
        //SetConsoleColor (0x07);
        aeStringFunctions::Format(sz, 1024, "%s\n", le->m_szText);
        OutputDebugString(sz);
        break;
      case aeLogEventType::DevMsg:
        //SetConsoleColor (0x08);
        aeStringFunctions::Format(sz, 1024, "%s\n", le->m_szText);
        OutputDebugString(sz);
        break;
      case aeLogEventType::DebugMsg:
      case aeLogEventType::DebugRegularMsg:
        //SetConsoleColor (0x09);
        aeStringFunctions::Format(sz, 1024, "%s\n", le->m_szText);
        OutputDebugString(sz);
        break;
      default:
        //SetConsoleColor (0x0D);
        aeStringFunctions::Format(sz, 1024, "%s\n", le->m_szText);
        OutputDebugString(sz);

        aeLog::Warning("Unknown Message Type %d", le->m_EventType);
        break;
    }

    //SetConsoleColor (0x07);
  }
} // namespace AE_NS_FOUNDATION

//#endif
