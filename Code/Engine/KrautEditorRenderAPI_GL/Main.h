#ifndef AE_EDITORRENDERAPIGL_MAIN_H
#define AE_EDITORRENDERAPIGL_MAIN_H

#include "Base.h"
#include <KrautEditorBasics/RenderAPI.h>
#include <windows.h>
//#include <gl/gl.h>

namespace AE_NS_EDITORRENDERAPIGL
{
  using namespace AE_NS_EDITORBASICS;

  extern "C"
  {
    AE_EDITORRENDERAPIGL_DLL void aePlugin_Init (void);
    AE_EDITORRENDERAPIGL_DLL void aePlugin_DeInit (void);
  }

  class aeEditorRenderAPI_GL : public aeEditorRenderAPI
  {
  public:
    aeEditorRenderAPI_GL ();
    virtual void CreateContext (aeUInt32 uiWidth, aeUInt32 uiHeight);
    virtual void DestroyContext ();
    virtual void ResizeFramebuffer (aeUInt32 uiWidth, aeUInt32 uiHeight);
    virtual void Swap ();

  private:
    bool m_bCreatedContext;
    HDC m_hDC;
    HGLRC m_hRC;
    HWND m_hWnd;
  };
}

#endif

