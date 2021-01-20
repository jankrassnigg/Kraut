#include "Base.h"
#include "Plugin.h"
#include "RenderAPI.h"

#include <KrautFoundation/Containers/Deque.h>
#include <KrautFoundation/Containers/Stack.h>
#include <KrautGraphics/Communication/GlobalEvent.h>

namespace AE_NS_EDITORBASICS
{
  AE_SINGLETON_CLASS_CODE(aeEditorRenderAPI);

  aeEvent aeEditorPlugin::s_EditorPluginEvent;
  aeString aeEditorPlugin::s_Stylesheet;
  aeHybridString<16> aeEditorPlugin::s_EditorName = "aeEditor";


  static aeDeque<aePickableObject> s_PickableObjects;
  static aeStack<aeUInt32> s_FreePickableObjects;

  aeUInt32 aeEditorPlugin::RegisterPickableObject(void* pObject, aeUInt32 uiSubID, const char* szType)
  {
    if (s_PickableObjects.empty())
    {
      s_PickableObjects.push_back(aePickableObject());
      aePickableObject& po = s_PickableObjects[0];
      po.m_pObject = nullptr;
      po.m_uiSubID = 0;
      po.m_szType = "none";
    }

    aeUInt32 uiObjectID;

    if (!s_FreePickableObjects.empty())
    {
      uiObjectID = s_FreePickableObjects.top();
      s_FreePickableObjects.pop();
    }
    else
    {
      uiObjectID = s_PickableObjects.size();
      s_PickableObjects.push_back(aePickableObject());
    }

    aePickableObject& po = s_PickableObjects[uiObjectID];

    po.m_pObject = pObject;
    po.m_uiSubID = uiSubID;
    po.m_szType = szType;

    return uiObjectID;
  }

  void aeEditorPlugin::UnregisterPickableObject(aeUInt32 uiObjectID)
  {
    if (uiObjectID == 0)
      return;

    s_FreePickableObjects.push(uiObjectID);
  }

  const aePickableObject& aeEditorPlugin::GetPickableObject(aeUInt32 uiObjectID)
  {
    return s_PickableObjects[uiObjectID];
  }
} // namespace AE_NS_EDITORBASICS
