#include "PCH.h"
#include <KrautEditorBasics/Plugin.h>
#include "Foundation.h"
#include <QApplication>
#include "qtMainWindow.moc.h"
#include <windows.h>
#include "LastInclude.h"

using namespace AE_NS_EDITORBASICS;

// state of the HTML Log Writer
static aeLog_HTMLWriter LogWriter;

aeCVarString CVarMainPlugin("app_MainPlugin", "ezTreePlugin", aeCVarFlags::Restart | aeCVarFlags::Save, "Which Renderer-DLL to use.");

// Returns the directory in which the exe is located
static aeString GetBinaryDirectory (void)
{
  char szTemp[512] = "";
  GetModuleFileName (NULL, szTemp, 512);

  return (aePathFunctions::GetFileDirectory (szTemp));
}

void StartupMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
  // store the HINSTANCE globally
  aeVariableRegistry::StoreRaw ("system/windows/hinstance", &hInstance, sizeof (HINSTANCE));

  // send the event that we are now starting up
  AE_BROADCAST_EVENT (aeStartup_main_begin);

  aeString temp = aePathFunctions::GetParentDirectory(GetBinaryDirectory());

  // setup the file system
  //aeFileSystem::AddDataDirectory (GetBinaryDirectory ().c_str (), "BIN", false);
  aeFileSystem::AddDataDirectory ((temp + "../../Data/aeEditor").c_str (), "", false);
  aeFileSystem::AddDataDirectory ((temp + "../../Data/aeEditor").c_str (), "APP", true);
  aeFileSystem::AddDataDirectory ((temp + "../../Data/Content").c_str (), "", true);

  // setup the logging system
  aeLog::RegisterEventReceiver (aeLog_HTMLWriter::LogMessageHandler, &LogWriter);
  aeLog::RegisterEventReceiver (aeLog_ConsoleWriter::LogMessageHandler, NULL);
  aeLog::RegisterEventReceiver (aeLog_VisualStudioWriter::LogMessageHandler, NULL);

  // start logging to the HTML log
  LogWriter.BeginLog ("<APP>Log.htm", "aeEditor");

  // write some info
  aeLog::Log ("Directory: \"%s\"", GetBinaryDirectory ().c_str ());

  // Load the CVars
  if (!aeCVar::LoadCVars ("<APP>CVars.cfg"))
    aeLog::Warning ("Could not load CVars.");

  aePluginManager::LoadPlugin (CVarMainPlugin.GetValue (), "");

  // Load the CVars again, after we have initialized the plugins
  if (!aeCVar::LoadCVars ("<APP>CVars.cfg"))
    aeLog::Warning ("Could not load CVars.");

  AE_BROADCAST_EVENT (aeStartup_main_end);

  // Startup all core modules
  aeStartup::StartupCore ();
}

void ShutdownMain ()
{
  AE_BROADCAST_EVENT (aeShutdown_main_begin);

  // Save all CVars
  if (!aeCVar::SaveCVars ("<APP>CVars.cfg"))
    aeLog::Warning ("Could not save CVars.");

  // Shutdown all core modules
  aeStartup::ShutdownCore ();

  // Stop logging
  LogWriter.EndLog ();

  AE_BROADCAST_EVENT (aeShutdown_main_end);
}

void LoadStylesheet (QApplication& app)
{
  aeFileIn File;
  if (File.Open ("App.stylesheet"))
  {
    aeString sSS;
    aeString sLine;

    while (!File.IsEndOfStream ())
    {
      sLine.clear ();
      File.ReadLine (sLine);
      sSS += sLine;
    }

    app.setStyle ("plastique");

    aeEditorPlugin::s_Stylesheet = sSS;
    app.setStyleSheet (sSS.c_str ());
  }
}

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
  int argc = 0;
  QApplication app(argc, NULL);

  StartupMain (hInstance, hPrevInstance, lpCmdLine, nShowCmd);

  LoadStylesheet (app);

  qtMainWindow MainWindow;
  MainWindow.show ();

  int iResult = app.exec();

  ShutdownMain ();

  return iResult;
}
