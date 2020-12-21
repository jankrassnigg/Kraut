#ifndef AE_EDITORBASICS_PLUGIN_H
#define AE_EDITORBASICS_PLUGIN_H

#include "Base.h"

#include <KrautFoundation/Communication/Event.h>
#include <KrautFoundation/Strings/HybridString.h>
#include <KrautFoundation/Strings/String.h>

class QMouseEvent;
class QWheelEvent;
class QKeyEvent;

using namespace AE_NS_FOUNDATION;

namespace AE_NS_EDITORBASICS
{
  struct aeEditorPluginEvent
  {
    enum aeEventType
    {
      MouseMove,
      MousePress,
      MouseRelease,
      MouseWheel,
      KeyPress,
      KeyRelease,
    };

    union aeEventData
    {
      QMouseEvent* m_MouseEvent;
      QWheelEvent* m_WheelEvent;
      QKeyEvent* m_KeyEvent;
    };

    aeEventType m_Type;
    aeEventData m_Data;
  };

  struct aePickableObject
  {
    void* m_pObject;
    aeUInt32 m_uiSubID;
    const char* m_szType;
  };

  //! Abstract Interface for a plugin that initializes some render API.
  class AE_EDITORBASICS_DLL aeEditorPlugin
  {
  public:
    //! The editor will notify all plugins of certain events through this object.
    static aeEvent s_EditorPluginEvent;

    static aeUInt32 RegisterPickableObject (void* pObject, aeUInt32 uiSubID, const char* szType);

    static void UnregisterPickableObject (aeUInt32 uiObjectID);

    static const aePickableObject& GetPickableObject (aeUInt32 uiObjectID);

    static aeString s_Stylesheet;

    static aeHybridString<16> s_EditorName;

  private:
  };
}


#endif

