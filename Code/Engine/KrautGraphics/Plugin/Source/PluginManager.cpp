#include "../PluginManager.h"
#include "../../FileSystem/FileSystem.h"
#include <KrautFoundation/Containers/Map.h>
#include <KrautGraphics/Logging/Log.h>

//#ifdef AE_PLATFORM_WINDOWS
#include <windows.h>
//#endif

namespace AE_NS_FOUNDATION
{
//#ifdef AE_PLATFORM_WINDOWS

  static aeMap<aeString, HMODULE> g_LoadedPlugins;
  static aeEvent g_PluginEvent;

  void aePluginManager::RegisterEventReceiver(AE_EVENT_NOTIFICATION_CALLBACK callback, void* pPassThrough)
  {
    g_PluginEvent.RegisterEventReceiver(callback, pPassThrough);
  }

  void aePluginManager::UnregisterEventReceiver(AE_EVENT_NOTIFICATION_CALLBACK callback, void* pPassThrough)
  {
    g_PluginEvent.UnregisterEventReceiver(callback, pPassThrough);
  }

  static aeMap<aeString, void*> g_RegisteredFunctions;

  void aePluginManager::RegisterPluginFunction(const char* szFunctionName, void* pFunc)
  {
    g_RegisteredFunctions[szFunctionName] = pFunc;
  }

  void* aePluginManager::GetRegisteredFunction(const char* szFunctionName)
  {
    return g_RegisteredFunctions[szFunctionName];
  }

  void aePluginManager::LoadPlugin(const char* szPluginName, const char* szDebugModeSuffix)
  {
    aeString sPlugin = szPluginName;

#ifdef _DEBUG
    sPlugin += szDebugModeSuffix;
#endif

    AE_LOG_BLOCK("Loading Plugin");
    aeLog::Log("Plugin: \"%s\"", sPlugin.c_str());

    aePluginEvent e;
    e.m_szPluginName = sPlugin.c_str();
    e.m_EventType = aePluginEventType::BeforeLoaded;

    g_PluginEvent.RaiseEvent((void*)&e);

    GetLastError();
    HMODULE hModule = LoadLibrary(sPlugin.c_str());

    if (hModule == nullptr)
    {
      DWORD err = GetLastError();

      LPVOID lpMsgBuf = nullptr;

      FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr,
        err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, nullptr);

      aeLog::Error("Could not load Plugin '%s'. Error-Code %u (\"%s\")", sPlugin.c_str(), err, lpMsgBuf);

      LocalFree(lpMsgBuf);
      return;
    }

    g_LoadedPlugins[sPlugin] = hModule;

    e.m_EventType = aePluginEventType::AfterLoaded;
    g_PluginEvent.RaiseEvent((void*)&e);

    AE_PLUGINFUNC_INIT Init = (AE_PLUGINFUNC_INIT)GetProcAddress(hModule, "aePlugin_Init");

    if (Init)
      Init();
    else
      aeLog::Dev("Plugin has no 'aePlugin_Init' function.");

    aeLog::Success("Plugin loaded.");
  }

  void aePluginManager::UnloadPlugin(const char* szPluginName, const char* szDebugModeSuffix)
  {
    aeString sPlugin = szPluginName;

#ifdef _DEBUG
    sPlugin += szDebugModeSuffix;
#endif

    AE_LOG_BLOCK("Unloading Plugin");
    aeLog::Log("Plugin: \"%s\"", sPlugin.c_str());

    if (g_LoadedPlugins.find(sPlugin) == g_LoadedPlugins.end())
    {
      aeLog::Warning("Plugin is not loaded.");
      return;
    }

    aePluginEvent e;
    e.m_szPluginName = sPlugin.c_str();
    e.m_EventType = aePluginEventType::BeforeUnloaded;

    g_PluginEvent.RaiseEvent((void*)&e);

    AE_PLUGINFUNC_INIT DeInit = (AE_PLUGINFUNC_INIT)GetProcAddress(g_LoadedPlugins[sPlugin], "aePlugin_DeInit");

    if (DeInit)
      DeInit();
    else
      aeLog::Dev("Plugin has no 'aePlugin_DeInit' function.");

    FreeLibrary(g_LoadedPlugins[sPlugin]);
    g_LoadedPlugins.erase(sPlugin);

    e.m_EventType = aePluginEventType::AfterUnloaded;
    g_PluginEvent.RaiseEvent((void*)&e);

    aeLog::Success("Plugin unloaded.");
  }

  //#endif

} // namespace AE_NS_FOUNDATION
