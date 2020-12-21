#include "../HTMLWriter.h"

namespace AE_NS_FOUNDATION
{
  aeLog_HTMLWriter::~aeLog_HTMLWriter ()
  {
    EndLog ();
  }

  void aeLog_HTMLWriter::BeginLog (const char* szFile, const char* szAppTitle)
  {
    m_File;
    if (!m_File.Open (szFile, 1024 * 1024))
    {
      for (aeUInt32 i = 1; i < 32; ++i)
      {
        const aeFileName sName = aePathFunctions::GetFileName (szFile);

        aeFileName sNewName;
        sNewName.Format ("%s_%i", sName.c_str (), i);

        aeFilePath sPath = aePathFunctions::ChangeFileName (szFile, sNewName.c_str ());

        if (m_File.Open (sPath.c_str (), 1024 * 1024))
          break;
      }
    }

    if (!m_File.IsStreamOpen ())
    {
      aeLog::Error ("Could not open Log-File \"%s\".", szFile);
      return;
    }

    aeHybridString<256> sText;
    sText.Format ("<HTML><HEAD><TITLE>Log - %s</TITLE></HEAD><BODY>", szAppTitle);
    m_File.Write (sText.c_str (), sizeof (char) * sText.length ());
  }

  void aeLog_HTMLWriter::EndLog (void)
  {
    if (!m_File.IsStreamOpen ())
      return;

    aeLog::Log ("");
    aeLog::Log (" <<< HTML-Log End >>> ");
    aeLog::Log ("");
    aeLog::Log ("");

    aeHybridString<256> sText;
    sText.Format ("</BODY></HTML>");
    m_File.Write (sText.c_str (), sizeof (char) * sText.length ());

    m_File.Close ();
  }

  const char* aeLog_HTMLWriter::GetOpenedLogFile (void) const
  {
    return (m_File.GetStreamName ());
  }

  void aeLog_HTMLWriter::LogMessageHandler (void* pPassThrough, const void* pEventData)
  {
    aeLog_HTMLWriter* pLog = (aeLog_HTMLWriter*) pPassThrough;

    pLog->MessageHandler (pEventData);
  }

  void aeLog_HTMLWriter::WriteString (const aeBasicString& s, aeUInt32 uiColor)
  {
    aeHybridString<64> sTemp;
    sTemp.Format ("<font color=\"#%X\">", uiColor);

    m_File.Write (sTemp.c_str (), sizeof (char) * sTemp.length ());
    m_File.Write (s.c_str (), sizeof (char) * s.length ());

    sTemp = "</font>";
    m_File.Write (sTemp.c_str (), sizeof (char) * sTemp.length ());
    m_File.FlushWriteCache();
  }

  void aeLog_HTMLWriter::MessageHandler (const void* pEventData)
  {
    if (!m_File.IsStreamOpen ())
      return;

    aeLog::aeLoggingEvent* le = (aeLog::aeLoggingEvent*) pEventData;

    aeHybridString<512> sText;
    aeHybridString<512> sOriginalText = le->m_szText;

    // Cannot write <, > or & to HTML, must be escaped
    if (!sOriginalText.empty ())
    {
      sOriginalText.ReplaceAll ("&", "&amp;");
      sOriginalText.ReplaceAll ("<", "&lt;");
      sOriginalText.ReplaceAll (">", "&gt;");
    }

    bool bFlushWriteCache = false;

    switch (le->m_EventType)
    {
    case aeLogEventType::FlushToDisk:
      bFlushWriteCache = true;
      break;
    case aeLogEventType::BeginGroup:
      sText.Format ("<br><font color=\"#8080FF\"><b> <<< <u>%s</u> >>> </b></font><br><table width=100%% border=0><tr width=100%%><td width=10></td><td width=*>\n", sOriginalText.c_str ());
      break;
    case aeLogEventType::EndGroup:
      sText.Format ("</td></tr></table><font color=\"#8080FF\"><b> <<< %s >>> </b></font><br><br>\n", sOriginalText.c_str ());
      break;
    case aeLogEventType::FatalErrorMsg:
      bFlushWriteCache = true;
      sText.Format ("<font color=\"#FF0000\"><b><u>Fatal Error: %s</u></b></font><br>\n", sOriginalText.c_str ());
      break;
    case aeLogEventType::ErrorMsg:
      bFlushWriteCache = true;
      sText.Format ("<font color=\"#FF0000\"><b><u>Error:</u> %s</b></font><br>\n", sOriginalText.c_str ());
      break;
    case aeLogEventType::SeriousWarningMsg:
      bFlushWriteCache = true;
      sText.Format ("<font color=\"#FF4000\"><b><u>Seriously:</u> %s</b></font><br>\n", sOriginalText.c_str ());
      break;
    case aeLogEventType::WarningMsg:
      sText.Format ("<font color=\"#FF8000\"><u>Warning:</u> %s</font><br>\n", sOriginalText.c_str ());
      break;
    case aeLogEventType::SuccessMsg:
      sText.Format ("<font color=\"#009000\">%s</font><br>\n", sOriginalText.c_str ());
      break;
    case aeLogEventType::LogMsg:
      sText.Format ("<font color=\"#000000\">%s</font><br>\n", sOriginalText.c_str ());
      break;
    case aeLogEventType::DevMsg:
      sText.Format ("<font color=\"#3030F0\">%s</font><br>\n", sOriginalText.c_str ());
      break;
    case aeLogEventType::DebugMsg:
    case aeLogEventType::DebugRegularMsg:
      sText.Format ("<font color=\"#A000FF\">%s</font><br>\n", sOriginalText.c_str ());
      break;
    default:
      sText.Format ("<font color=\"#A0A0A0\">%s</font><br>\n", sOriginalText.c_str ());

      aeLog::Warning ("Unknown Message Type %d", le->m_EventType);
      break;
    }

    if (!sText.empty ())
      m_File.Write (sText.c_str (), sizeof (char) * sText.length ());

    if (bFlushWriteCache)
    {
      aeFilePath sFile = m_File.GetStreamName ();

      m_File.Close ();
      m_File.Open (sFile.c_str (), 1024 * 1024);
      //m_File.FlushWriteCache ();
    }
  }
}




