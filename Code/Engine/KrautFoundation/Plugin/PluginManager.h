#ifndef AE_FOUNDATION_PLUGIN_PLUGINMANAGER_H
#define AE_FOUNDATION_PLUGIN_PLUGINMANAGER_H

#include "../Defines.h"
#include "../Communication/Event.h"
#include "../Containers/Stack.h"
#include "../Strings/HybridString.h"

namespace AE_NS_FOUNDATION
{
  typedef void (*AE_PLUGINFUNC_INIT)(void);
  typedef void (*AE_PLUGINFUNC_DEINIT)(void);

    //! Describes the events that are sent during plugin loading / unloading
  struct aePluginEventType
  {
    enum Enum
    {
      BeforeLoaded,    //!< Sent shortly before a new plugin is loaded
      AfterLoaded,     //!< Sent shortly after a new plugin has been loaded
      BeforeUnloaded,  //!< Sent before a plugin is going to be unloaded
      AfterUnloaded,   //!< Sent after a plugin has been unloaded
    };
  };

  //! This manager allows to dynamically load plugins (DLLs / shared-objects) into the application.
  class AE_FOUNDATION_DLL aePluginManager
  {
  public:
    //! Loads a plugin by the given name.
    static void LoadPlugin (const char* szPluginName, const char* szDebugModeSuffix);
    //! Unloads the plugin with the given name, if it is currently loaded.
    static void UnloadPlugin (const char* szPluginName, const char* szDebugModeSuffix);

    static void RegisterPluginFunction (const char* szFunctionName, void* pFunc);

    static void* GetRegisteredFunction (const char* szFunctionName);

    //! The data that is sent through the event interface.
    struct aePluginEvent
    {
      //! The type of this event.
      aePluginEventType::Enum m_EventType;
      //! The (file-) name of the plugin that is affected.
      const char* m_szPluginName;
    };

    //! Allows to register a function as an event receiver.
    static void RegisterEventReceiver (AE_EVENT_NOTIFICATION_CALLBACK callback, void* pPassThrough);
    //! Unregisters a previously registered receiver.
    static void UnregisterEventReceiver (AE_EVENT_NOTIFICATION_CALLBACK callback, void* pPassThrough);

  private:

  };
}

#endif
