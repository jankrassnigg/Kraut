// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#include "../glim/glim.h"
#include "Main.h"
#include "RenderAPI_Impl.h"

#ifdef AE_PLATFORM_WINDOWS
#  include "../Glew/wglew.h"
#endif



namespace AE_NS_GRAPHICS
{
  using namespace NS_GLIM;

#ifdef AE_RENDERAPI_OPENGL
  AE_RENDERAPI g_RenderAPI = AE_RA_OPENGL;
#endif

  void aeRenderAPI::ConfigureBeforeStartup(bool bAutoCheckForErrors, bool bExceptionOnError)
  {
    aeRenderAPI_Impl::m_bAutoCheckErrors = bAutoCheckForErrors;
    aeRenderAPI_Impl::m_bExceptionOnError = bExceptionOnError;
  }

  void aeRenderAPI::ProgramStartup(void)
  {
    AE_LOG_BLOCK("Starting RenderAPI");

#ifdef AE_RENDERAPI_OPENGL
    if (g_RenderAPI == AE_RA_OPENGL)
    {
      // check the vendor
      char renderer[256] = "";
      aeStringFunctions::Copy(renderer, 256, (const char*)glGetString(GL_VENDOR));
      aeStringFunctions::ToUpperCase(renderer);

      if (aeStringFunctions::FindFirstStringPos(renderer, "MICROSOFT") != -1)
        AE_CHECK_ALWAYS(false, "Your graphics-card does not support OpenGL.\nPlease update your display-driver.");

      // store the vendor
      aeRenderAPI_Impl::m_Vendor = AE_VENDOR_UNKNOWN;
      if (aeStringFunctions::FindFirstStringPos(renderer, "NVIDIA") != -1)
        aeRenderAPI_Impl::m_Vendor = AE_VENDOR_NVIDIA;
      else if (aeStringFunctions::FindFirstStringPos(renderer, "ATI ") != -1) // note the space
        aeRenderAPI_Impl::m_Vendor = AE_VENDOR_ATI;


      // initialize all extensions
      glewInit();
      glGetError();

      CheckErrors("After Extension Initialization");

      aeInt32 i;
      glGetIntegerv(GL_MAX_DRAW_BUFFERS, &i);
      aeLog::Log("Max Render Targets: %d", i);

      CheckExtensions();

      CheckErrors("Before State Initialization");

      // set all default states (even the weird ones)
      SetInitialState();

      CheckErrors("Before VSync Initialization");

      SetVSync(true);

      CheckErrors("After VSync Initialization");

      aeRenderAPI_Impl::ApplyRenderStates(true);

      CheckErrors("After RenderState Initialization");

      aeRenderAPI_Impl::ApplyStencilStates(true);

      CheckErrors("After StencilState Initialization");

      aeRenderAPI_Impl::ApplyFBStates(true);

      CheckErrors("After FB State Initialization");
    }
#endif

    switch (g_RenderAPI)
    {
#ifdef AE_RENDERAPI_OPENGL
      case AE_RA_OPENGL:
        g_GlimAPI = GLIM_OPENGL;
        aeLog::Log("Using OpenGL as RenderAPI.");
        break;
#endif
      default:
        AE_CHECK_ALWAYS(false, "The RenderAPI has not been selected (OpenGL / Direct3D11).");
        break;
    }
  }

  void aeRenderAPI::ProgramShutdown(void)
  {
    AE_LOG_BLOCK("aeRenderAPI::ProgramShutdown");

    aeLog::Log("Shutting down aeRenderAPI");

    Sleep(200);
  }

  AE_RA_VENDOR aeRenderAPI::GetVendor()
  {
    return (aeRenderAPI_Impl::m_Vendor);
  }

  void aeRenderAPI::CheckErrors(const char* szSectionName)
  {
#ifdef AE_RENDERAPI_OPENGL
    if (g_RenderAPI == AE_RA_OPENGL)
    {
      GLenum err = glGetError();
      GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);

      if (err != GL_NO_ERROR)
      {
        aeLog::Error("OpenGL Error (%s):\n%s (%d)\n", szSectionName, gluErrorString(err), err);

        if (aeRenderAPI_Impl::m_bExceptionOnError)
          AE_CHECK_ALWAYS(false, "OpenGL Error (%s):\n%s (%d)\n", szSectionName, gluErrorString(err), err);
      }

      switch (status)
      {
        case GL_FRAMEBUFFER_COMPLETE:
          break;
        case GL_FRAMEBUFFER_UNSUPPORTED:
          aeLog::Error("OpenGL Error (%s):\n FBO configuration unsupported\n", szSectionName);

          if (aeRenderAPI_Impl::m_bExceptionOnError)
            AE_CHECK_ALWAYS(false, "OpenGL Error (%s):\n FBO configuration unsupported\n", szSectionName);
          break;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
          aeLog::Warning("OpenGL Error (%s):\n FRAMEBUFFER_INCOMPLETE_ATTACHMENT\n", szSectionName);

          //if (aeRenderAPI_Impl::m_bExceptionOnError)
          //	AE_CHECK_ALWAYS (false, "OpenGL Error (%s):\n FRAMEBUFFER_INCOMPLETE_ATTACHMENT\n", szSectionName);
          break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
          aeLog::Warning("OpenGL Error (%s):\n FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT\n", szSectionName);

          //if (aeRenderAPI_Impl::m_bExceptionOnError)
          //	AE_CHECK_ALWAYS (false, "OpenGL Error (%s):\n FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT\n", szSectionName);
          break;
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
          aeLog::Warning("OpenGL Error (%s):\n FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER\n", szSectionName);

          //if (aeRenderAPI_Impl::m_bExceptionOnError)
          //	AE_CHECK_ALWAYS (false, "OpenGL Error (%s):\n FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER\n", szSectionName);
          break;
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
          aeLog::Warning("OpenGL Error (%s):\n FRAMEBUFFER_INCOMPLETE_READ_BUFFER\n", szSectionName);

          //if (aeRenderAPI_Impl::m_bExceptionOnError)
          //	AE_CHECK_ALWAYS (false, "OpenGL Error (%s):\n FRAMEBUFFER_INCOMPLETE_READ_BUFFER\n", szSectionName);
          break;
        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
          aeLog::Warning("OpenGL Error (%s):\n FRAMEBUFFER_INCOMPLETE_MULTISAMPLE\n", szSectionName);

          //if (aeRenderAPI_Impl::m_bExceptionOnError)
          //	AE_CHECK_ALWAYS (false, "OpenGL Error (%s):\n FRAMEBUFFER_INCOMPLETE_MULTISAMPLE\n", szSectionName);
          break;

        default:
          aeLog::Error("OpenGL Error (%s):\n Unknown FBO error\n", szSectionName);

          if (aeRenderAPI_Impl::m_bExceptionOnError)
            AE_CHECK_ALWAYS(false, "OpenGL Error (%s):\n Unknown FBO error\n", szSectionName);
          break;
      }
    }
#endif
  }

  void aeRenderAPI::StartFrame(void)
  {
    if (aeRenderAPI_Impl::m_bAutoCheckErrors)
      CheckErrors("auto-check: StartFrame");

    //AE_CHECK_ALWAYS (aeRenderAPI_Impl::m_RenderStateStack.empty (), "You need to call PushRenderStates / PopRenderStates equally often each frame.");

    aeRenderAPI_Impl::m_CurrentRenderState.setToDefault();
    aeRenderAPI_Impl::m_CurrentStencilState.setToDefault();
    aeRenderAPI_Impl::m_CurrentFBState.setToDefault();

    aeRenderAPI_Impl::m_MatrixObjectToWorld.LoadIdentity();
    aeRenderAPI_Impl::m_MatrixWorldToCamera.LoadIdentity();
  }

  void aeRenderAPI::Draw(AE_RENDER_MODE Mode, aeUInt32 uiFirstVertex, aeUInt32 uiVertexCount)
  {
    ApplyStates();

    ++aeRenderAPI_Impl::m_uiNumberOfDrawcalls;
    aeRenderAPI_Impl::m_uiNumberOfRenderedVertices += uiVertexCount;

    if (Mode == AE_RENDER_TRIANGLES)
      aeRenderAPI_Impl::m_uiNumberOfRenderedTriangles += uiVertexCount / 3;

#ifdef AE_RENDERAPI_OPENGL
    if (g_RenderAPI == AE_RA_OPENGL)
    {
      glDrawArrays(mapRenderModeOGL(Mode), uiFirstVertex, uiVertexCount);
    }
#endif
  }

  void aeRenderAPI::DrawIndexed(AE_RENDER_MODE Mode, aeUInt32 uiFirstIndex, aeUInt32 uiIndexCount, AE_INDEX_TYPE IndexType)
  {
    ApplyStates();

    ++aeRenderAPI_Impl::m_uiNumberOfDrawcalls;
    aeRenderAPI_Impl::m_uiNumberOfRenderedVertices += uiIndexCount;

    if (Mode == AE_RENDER_TRIANGLES)
      aeRenderAPI_Impl::m_uiNumberOfRenderedTriangles += uiIndexCount / 3;

#ifdef AE_RENDERAPI_OPENGL
    if (g_RenderAPI == AE_RA_OPENGL)
    {
      if (IndexType == AE_UNSIGNED_SHORT)
        glDrawElements(mapRenderModeOGL(Mode), uiIndexCount, mapIndexTypeOGL(IndexType), (void*)(uiFirstIndex * 2));
      if (IndexType == AE_UNSIGNED_INT)
        glDrawElements(mapRenderModeOGL(Mode), uiIndexCount, mapIndexTypeOGL(IndexType), (void*)(uiFirstIndex * 4));
    }
#endif
  }

  void aeRenderAPI::DrawIndexedWithBase(AE_RENDER_MODE Mode, aeUInt32 uiFirstIndex, aeUInt32 uiIndexCount, aeUInt32 uiBase, AE_INDEX_TYPE IndexType)
  {
    ApplyStates();

    ++aeRenderAPI_Impl::m_uiNumberOfDrawcalls;
    aeRenderAPI_Impl::m_uiNumberOfRenderedVertices += uiIndexCount;

    if (Mode == AE_RENDER_TRIANGLES)
      aeRenderAPI_Impl::m_uiNumberOfRenderedTriangles += uiIndexCount / 3;

#ifdef AE_RENDERAPI_OPENGL
    if (g_RenderAPI == AE_RA_OPENGL)
    {
      if (IndexType == AE_UNSIGNED_SHORT)
        glDrawElementsBaseVertex(mapRenderModeOGL(Mode), uiIndexCount, mapIndexTypeOGL(IndexType), (void*)(uiFirstIndex * 2), uiBase);
      if (IndexType == AE_UNSIGNED_INT)
        glDrawElementsBaseVertex(mapRenderModeOGL(Mode), uiIndexCount, mapIndexTypeOGL(IndexType), (void*)(uiFirstIndex * 4), uiBase);
    }
#endif
  }

  void aeRenderAPI::DrawInstanced(AE_RENDER_MODE Mode, aeUInt32 uiFirstVertex, aeUInt32 uiVertexCount, aeUInt32 uiInstanceCount)
  {
    ApplyStates();

    ++aeRenderAPI_Impl::m_uiNumberOfDrawcalls;
    aeRenderAPI_Impl::m_uiNumberOfRenderedInstances += uiInstanceCount;
    aeRenderAPI_Impl::m_uiNumberOfRenderedVertices += uiVertexCount;

    if (Mode == AE_RENDER_TRIANGLES)
      aeRenderAPI_Impl::m_uiNumberOfRenderedTriangles += uiVertexCount / 3;

#ifdef AE_RENDERAPI_OPENGL
    if (g_RenderAPI == AE_RA_OPENGL)
    {
      glDrawArraysInstancedEXT(mapRenderModeOGL(Mode), uiFirstVertex, uiVertexCount, uiInstanceCount);
    }
#endif
  }

  void aeRenderAPI::DrawIndexedInstanced(AE_RENDER_MODE Mode, aeUInt32 uiFirstIndex, aeUInt32 uiIndexCount, aeUInt32 uiInstanceCount, AE_INDEX_TYPE IndexType)
  {
    ApplyStates();

    ++aeRenderAPI_Impl::m_uiNumberOfDrawcalls;
    aeRenderAPI_Impl::m_uiNumberOfRenderedInstances += uiInstanceCount;
    aeRenderAPI_Impl::m_uiNumberOfRenderedVertices += uiIndexCount;

    if (Mode == AE_RENDER_TRIANGLES)
      aeRenderAPI_Impl::m_uiNumberOfRenderedTriangles += uiIndexCount / 3;

#ifdef AE_RENDERAPI_OPENGL
    if (g_RenderAPI == AE_RA_OPENGL)
    {
      if (IndexType == AE_UNSIGNED_SHORT)
        glDrawElementsInstanced(mapRenderModeOGL(Mode), uiIndexCount, mapIndexTypeOGL(IndexType), (void*)(uiFirstIndex * 2), uiInstanceCount);
      if (IndexType == AE_UNSIGNED_INT)
        glDrawElementsInstanced(mapRenderModeOGL(Mode), uiIndexCount, mapIndexTypeOGL(IndexType), (void*)(uiFirstIndex * 4), uiInstanceCount);
    }
#endif
  }

  void aeRenderAPI::SetInitialState(void)
  {
#ifdef AE_RENDERAPI_OPENGL
    if (g_RenderAPI == AE_RA_OPENGL)
    {
      aeRenderAPI::CheckErrors("Before SetInitialState");

      glLineWidth(1);
      glPointSize(1);

      aeRenderAPI::CheckErrors("SetInitialState 3");

      //glMatrixMode (GL_TEXTURE);
      //glLoadIdentity ();
      //glMatrixMode (GL_PROJECTION);
      //glLoadIdentity ();
      //glMatrixMode (GL_MODELVIEW);
      //glLoadIdentity ();

      //aeRenderAPI::CheckErrors ("SetInitialState 4");


      //Fog
      //glDisable (GL_FOG);

      aeRenderAPI::CheckErrors("SetInitialState 5");

      //Smoothing
      //glDisable (GL_LINE_SMOOTH);
      //glDisable (GL_POINT_SMOOTH);
      //glDisable (GL_POLYGON_SMOOTH);

      aeRenderAPI::CheckErrors("SetInitialState 6");

      //Color
      //glShadeModel (GL_FLAT);
      //glDisable (GL_COLOR_MATERIAL);

      aeRenderAPI::CheckErrors("SetInitialState 7");

      //Alpha Test
      //glDisable (GL_ALPHA_TEST);
      //glAlphaFunc (GL_ALWAYS, 0.0f);

      aeRenderAPI::CheckErrors("SetInitialState 9");

      //Normals
      //glDisable (GL_AUTO_NORMAL);
      //glDisable (GL_NORMALIZE);

      aeRenderAPI::CheckErrors("SetInitialState 13");

      //Culling
      glFrontFace(GL_CCW);

      aeRenderAPI::CheckErrors("SetInitialState 15");

      //Logic
      //glDisable (GL_COLOR_LOGIC_OP);
      //glDisable (GL_INDEX_LOGIC_OP);
      //glDisable (GL_LOGIC_OP);

      aeRenderAPI::CheckErrors("SetInitialState 17");

      //Lighting
      //glDisable (GL_LIGHTING);

      aeRenderAPI::CheckErrors("SetInitialState 18");

      //Polygonoffset
      glDisable(GL_POLYGON_OFFSET_FILL);
      glDisable(GL_POLYGON_OFFSET_LINE);
      glDisable(GL_POLYGON_OFFSET_POINT);

      aeRenderAPI::CheckErrors("After SetInitialState");
    }
#endif
  }

  void aeRenderAPI::CheckExtensions(void)
  {
#ifdef AE_RENDERAPI_OPENGL
    if (g_RenderAPI == AE_RA_OPENGL)
    {
    }
#endif
  }

  void aeRenderAPI::SetWindowSize(aeUInt32 uiWindowSizeX, aeUInt32 uiWindowSizeY)
  {
    aeRenderAPI_Impl::s_uiWindowSizeX = uiWindowSizeX;
    aeRenderAPI_Impl::s_uiWindowSizeY = uiWindowSizeY;
  }

  void aeRenderAPI::GetWindowSize(aeUInt32& out_uiWindowSizeX, aeUInt32& out_uiWindowSizeY)
  {
    out_uiWindowSizeX = aeRenderAPI_Impl::s_uiWindowSizeX;
    out_uiWindowSizeY = aeRenderAPI_Impl::s_uiWindowSizeY;
  }

  void aeRenderAPI::SetVSync(bool bEnable)
  {
#ifdef AE_RENDERAPI_OPENGL
    if (g_RenderAPI == AE_RA_OPENGL)
    {

#  ifdef AE_PLATFORM_WINDOWS
      if (WGLEW_EXT_swap_control)
      {
        wglSwapIntervalEXT(bEnable ? 1 : 0);
        aeRenderAPI_Impl::m_bVSync = bEnable;
      }
      else
#  endif
        aeRenderAPI_Impl::m_bVSync = false;
    }
#endif
  }

  bool aeRenderAPI::GetVSync(void)
  {
    return (aeRenderAPI_Impl::m_bVSync);
  }

#ifdef AE_COMPILE_FOR_DEVELOPMENT
  AE_RENDERAPI_CHECK::AE_RENDERAPI_CHECK(const char* szBlockName)
  {
    aeLog::BeginLogBlock(szBlockName);

    m_sBlockName = szBlockName;

    char szText[64];
    aeStringFunctions::Format(szText, 64, "Before %s", szBlockName);

    aeRenderAPI::CheckErrors(szText);
  }

  AE_RENDERAPI_CHECK::~AE_RENDERAPI_CHECK()
  {
    char szText[64];
    aeStringFunctions::Format(szText, 64, "After %s", m_sBlockName.c_str());

    aeRenderAPI::CheckErrors(szText);

    aeLog::EndLogBlock();
  }
#endif

  void aeRenderAPI::ResetStatistics(void)
  {
    aeRenderAPI_Impl::m_uiNumberOfDrawcalls = 0;
    aeRenderAPI_Impl::m_uiNumberOfStateChanges = 0;
    aeRenderAPI_Impl::m_uiNumberOfRenderedVertices = 0;
    aeRenderAPI_Impl::m_uiNumberOfRenderedTriangles = 0;
    aeRenderAPI_Impl::m_uiNumberOfRenderedInstances = 0;
  }

  aeUInt32 aeRenderAPI::getNumberOfDrawcalls(void)
  {
    return (aeRenderAPI_Impl::m_uiNumberOfDrawcalls);
  }

  aeUInt32 aeRenderAPI::getNumberOfStateChanges(void)
  {
    return (aeRenderAPI_Impl::m_uiNumberOfStateChanges);
  }

  aeUInt32 aeRenderAPI::getNumberOfRenderedVertices(void)
  {
    return (aeRenderAPI_Impl::m_uiNumberOfRenderedVertices);
  }

  aeUInt32 aeRenderAPI::getNumberOfRenderedTriangles(void)
  {
    return (aeRenderAPI_Impl::m_uiNumberOfRenderedTriangles);
  }

  aeUInt32 aeRenderAPI::getNumberOfRenderedInstances(void)
  {
    return (aeRenderAPI_Impl::m_uiNumberOfRenderedInstances);
  }
} // namespace AE_NS_GRAPHICS
