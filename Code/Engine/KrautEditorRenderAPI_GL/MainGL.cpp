#include "Main.h"
#include <KrautGraphics/Logging/Log.h>


namespace AE_NS_EDITORRENDERAPIGL
{
  aeEditorRenderAPI_GL g_RenderAPI;

  void aePlugin_Init(void)
  {
    aeLog::Log("RenderAPI_GL Plugin Init.");
  }

  void aePlugin_DeInit(void)
  {
    aeLog::Log("RenderAPI_GL Plugin DeInit");
  }

  aeEditorRenderAPI_GL::aeEditorRenderAPI_GL()
  {
    m_bCreatedContext = false;
    m_hDC = nullptr;
    m_hRC = nullptr;
    m_hWnd = nullptr;
  }
} // namespace AE_NS_EDITORRENDERAPIGL
