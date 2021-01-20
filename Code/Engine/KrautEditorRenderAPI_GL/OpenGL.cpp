#include "Main.h"

#include <KrautGraphics/Communication/GlobalEvent.h>
#include <KrautGraphics/Configuration/VariableRegistry.h>
#include <KrautGraphics/Glew/glew.h>
#include <KrautGraphics/Logging/Log.h>

namespace AE_NS_EDITORRENDERAPIGL
{
  void aeEditorRenderAPI_GL::CreateContext(aeUInt32 uiWidth, aeUInt32 uiHeight)
  {
    if (m_bCreatedContext)
      return;

    AE_LOG_BLOCK("CreateContext");

    aeLog::Log("Creating OpenGL Context...");

    int iColorBits = 24;
    int iDepthBits = 24;
    int iBPC = 8;

    PIXELFORMATDESCRIPTOR pfd =
      {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,                                                                              // Version
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_SWAP_EXCHANGE, // Flags
        PFD_TYPE_RGBA,                                                                  // Pixeltype
        iColorBits,                                                                     // Color Bits
        iBPC, 0, iBPC, 0, iBPC, 0, iBPC, 0,                                             // Red Bits / Red Shift, Green Bits / Shift, Blue Bits / Shift, Alpha Bits / Shift
        0, 0, 0, 0, 0,                                                                  // Accum Bits (total), Accum Bits Red, Green, Blue, Alpha
        iDepthBits, 8,                                                                  // Depth, Stencil Bits
        0,                                                                              // Aux Buffers
        PFD_MAIN_PLANE,                                                                 // Layer Type (ignored)
        0, 0, 0, 0                                                                      // ignored deprecated flags
      };

    aeVariableRegistry::RetrieveRaw("system/windows/hwnd", &m_hWnd, sizeof(HWND));

    m_hDC = GetDC(m_hWnd);

    AE_CHECK_ALWAYS(m_hDC != nullptr, "CreateContext: Could not get the Windows hDC.");

    int iPixelformat = ChoosePixelFormat(m_hDC, &pfd);
    AE_CHECK_ALWAYS(iPixelformat != 0, "CreateContext: Could not choose a PixelFormat.");

    AE_CHECK_ALWAYS(SetPixelFormat(m_hDC, iPixelformat, &pfd), "CreateContext: Could not set a PixelFormat.");

    m_hRC = wglCreateContext(m_hDC);
    AE_CHECK_ALWAYS(m_hRC != nullptr, "CreateContext: Could not create a (wgl) Context.");

    AE_CHECK_ALWAYS(wglMakeCurrent(m_hDC, m_hRC), "CreateContext: Could not make the (wgl) Context current.");

    aeVariableRegistry::StoreRaw("system/graphics/opengl/hdc", &m_hDC, sizeof(HDC));
    aeVariableRegistry::StoreRaw("system/graphics/opengl/hrc", &m_hRC, sizeof(HGLRC));

    aeLog::Success("Created the OpenGL Context.");

    m_bCreatedContext = true;

    ResizeFramebuffer(uiWidth, uiHeight);
  }

  void aeEditorRenderAPI_GL::DestroyContext(void)
  {
    if (!m_bCreatedContext)
      return;

    m_bCreatedContext = false;

    AE_LOG_BLOCK("DestroyContext");

    aeLog::Log("Destroying the OpenGL Context...");

    if (m_hRC)
    {
      wglMakeCurrent(nullptr, nullptr);

      wglDeleteContext(m_hRC);

      m_hRC = nullptr;

      aeVariableRegistry::StoreRaw("system/graphics/opengl/hrc", &m_hRC, sizeof(HGLRC));
    }

    if (m_hDC)
    {
      ReleaseDC(m_hWnd, m_hDC);
      m_hDC = nullptr;

      aeVariableRegistry::StoreRaw("system/graphics/opengl/hdc", &m_hDC, sizeof(HDC));
    }

    aeLog::Success("Destroyed the OpenGL Context.");
  }

  void aeEditorRenderAPI_GL::Swap()
  {
    if (!m_bCreatedContext)
      return;

    SwapBuffers(m_hDC);
  }

  void aeEditorRenderAPI_GL::ResizeFramebuffer(aeUInt32 uiWidth, aeUInt32 uiHeight)
  {
    if (!m_bCreatedContext)
      return;

    aeVariableRegistry::StoreInt("system/graphics/resolution_x", uiWidth);
    aeVariableRegistry::StoreInt("system/graphics/resolution_y", uiHeight);

    glViewport(0, 0, uiWidth, uiHeight);
    AE_BROADCAST_EVENT(aeRenderAPI_Resize);
  }
} // namespace AE_NS_EDITORRENDERAPIGL
