// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#ifndef AE_GRAPHICS_RENDERAPI_RENDERAPI_H
#define AE_GRAPHICS_RENDERAPI_RENDERAPI_H

#include "Declarations.h"
#include "MatrixStack.h"
#include "StatesD3D.h"
#include <KrautFoundation/Math/Declarations.h>
#include <KrautGraphics/Strings/StaticString.h>

//#ifdef AE_PLATFORM_WINDOWS
#include <windows.h>
//#endif

namespace AE_NS_GRAPHICS
{
  class AE_GRAPHICS_DLL aeRenderAPI
  {
  public:
    static void ConfigureBeforeStartup(bool bAutoCheckForErrors, bool bExceptionOnError);

    //! Tells the RenderAPI the current width and height of the window in pixels.
    static void SetWindowSize(aeUInt32 uiWindowSizeX, aeUInt32 uiWindowSizeY);
    //! Returns the width/height of the window in pixels.
    static void GetWindowSize(aeUInt32& out_uiWindowSizeX, aeUInt32& out_uiWindowSizeY);

    //! Enables or disables vertical synchronization.
    static void SetVSync(bool bEnable);
    //! Returns whether vsync is enabled.
    static bool GetVSync(void);

    //! Returns the vendor of the graphics-card.
    static AE_RA_VENDOR GetVendor();

    //! Checks for GL errors, if found logs them using the section-name to identify the code position.
    static void CheckErrors(const char* szSectionName);

    //! Resets all internal counters about state-changes, etc.
    static void StartFrame(void);


    // ****** DRAWCALLS ******

    static void Draw(AE_RENDER_MODE Mode, aeUInt32 uiFirstVertex, aeUInt32 uiVertexCount);
    static void DrawIndexed(AE_RENDER_MODE Mode, aeUInt32 uiFirstIndex, aeUInt32 uiIndexCount, AE_INDEX_TYPE IndexType = AE_UNKNOWN);
    static void DrawInstanced(AE_RENDER_MODE Mode, aeUInt32 uiFirstVertex, aeUInt32 uiVertexCount, aeUInt32 uiInstanceCount);
    static void DrawIndexedInstanced(AE_RENDER_MODE Mode, aeUInt32 uiFirstIndex, aeUInt32 uiIndexCount, aeUInt32 uiInstanceCount, AE_INDEX_TYPE IndexType = AE_UNKNOWN);
    static void DrawIndexedWithBase(AE_RENDER_MODE Mode, aeUInt32 uiFirstIndex, aeUInt32 uiIndexCount, aeUInt32 uiBase, AE_INDEX_TYPE IndexType = AE_UNKNOWN);


    // ****** STATES ******

    //! Applies all cached states immediately. Automatically called by all drawcalls, so usually no need to call it yourself.
    static void ApplyStates(void);

    //! Pushes the current state-configuration onto a stack for reuse later, resets all states to their default
    static void PushRenderStates(bool bResetToDefault);
    //! Takes the state-configuration from the stacks top, discards the current state-configuration.
    static void PopRenderStates();
    //! Sets all render states to their default values.
    static void SetDefaultRenderStates();

    //! Pushes the current state-configuration onto a stack for reuse later, resets all states to their default
    static void PushFramebufferStates(bool bResetToDefault);
    //! Takes the state-configuration from the stacks top, discards the current state-configuration.
    static void PopFramebufferStates();

    //! Pushes the current state-configuration onto a stack for reuse later, resets all states to their default
    static void PushStencilStates(bool bResetToDefault);
    //! Takes the state-configuration from the stacks top, discards the current state-configuration.
    static void PopStencilStates();



    // ****** RENDER STATES ******

    //! Whether to clip (cull) fragments at the near/far plane.
    static void setDepthClip(bool bDepthClip);

    static void setFaceCulling(AE_FACE_CULLING cull);
    static void setWireframeMode(bool bEnable);
    static void setColorMask(bool bRed, bool bGreen, bool bBlue, bool bAlpha, aeInt8 iRenderTarget = -1);
    static void setDepthMask(bool bEnable);
    static void setDepthTest(bool bEnable, AE_COMPARE_FUNC DepthFunc = AE_COMPARE_ALWAYS);

    static void setBlendColor(float fRed, float fGreen, float fBlue, float fAlpha);
    static void setBlending(bool bEnable, AE_BLEND_FACTOR srcfactorRGB = AE_FACTOR_ONE, AE_BLEND_FACTOR dstfactorRGB = AE_FACTOR_ONE, AE_BLEND_FUNC BlendFuncRGB = AE_FUNC_ADD,
      AE_BLEND_FACTOR srcfactorAlpha = AE_FACTOR_ONE, AE_BLEND_FACTOR dstfactorAlpha = AE_FACTOR_ZERO, AE_BLEND_FUNC BlendFuncAlpha = AE_FUNC_ADD, aeInt8 iRenderTarget = -1);


    // ****** STENCIL STATES ******
    static void setStencilTest(bool bEnable, AE_COMPARE_FUNC StencilFunc = AE_COMPARE_ALWAYS, aeInt32 iStencilFuncRef = 0, aeUInt32 uiStencilFuncMask = ~0,
      aeUInt8 uiStencilMask = ~0, AE_STENCIL_OP StencilOpFail = AE_STENCIL_KEEP, AE_STENCIL_OP StencilOpZFail = AE_STENCIL_KEEP, AE_STENCIL_OP StencilOpZPass = AE_STENCIL_KEEP,
      AE_FACE_STENCIL_TEST face = AE_FRONT_AND_BACK_FACE_STENCIL_TEST);


    // ****** FRAMEBUFFER ******

    //! Clears the buffers of the currently active FBO.
    static void Clear(bool bColor, bool bDepthStencil, float fRed = 0.0f, float fGreen = 0.0f, float fBlue = 0.0f, float fAlpha = 0.0f, float fDepth = 1.0f, aeUInt8 uiStencil = 0);

    //! Sets the viewport dimensions.
    static void setViewport(aeUInt32 x = 0, aeUInt32 y = 0, aeUInt32 width = 0, aeUInt32 height = 0);
    //! Returns current viewport and depth-range.
    static void getViewportAndDepthRange(aeUInt32& out_x, aeUInt32& out_y, aeUInt32& out_width, aeUInt32& out_height, float& out_fNear, float& out_fFar);

    //! Sets the depth-range (near / far) and whether z values shall be clipped to or clamped to the depth-range.
    static void setDepthRange(float fNear, float fFar);
    //! Sets the scissor rect.
    static void setScissorTest(bool bEnable, aeUInt32 x = 0, aeUInt32 y = 0, aeUInt32 width = 0, aeUInt32 height = 0);


    // ****** MATRIX STACKS ******

    //! Accesses the Matrix that transforms from object space into world space (part of "Modelview Matrix")
    static aeMatrixStack& GetMatrix_ObjectToWorld(void);

    //! Accesses the Matrix that transforms from world space into camera space (part of "Modelview Matrix")
    static aeMatrixStack& GetMatrix_WorldToCamera(void);

    //! Accesses the Matrix that transforms from camera space into screen space ("Projection Matrix")
    static aeMatrixStack& GetMatrix_CameraToScreen(void);


    // ****** STATISTICS ******
    static void ResetStatistics(void);
    static aeUInt32 getNumberOfDrawcalls(void);
    static aeUInt32 getNumberOfStateChanges(void);
    static aeUInt32 getNumberOfRenderedVertices(void);
    static aeUInt32 getNumberOfRenderedTriangles(void);
    static aeUInt32 getNumberOfRenderedInstances(void);

  private:
    friend class aeRenderAPIStartup;

    // ****** startup / shutdown / basic stuff ******
    static void ProgramStartup(void);
    static void ProgramShutdown(void);

    static void SetInitialState(void);

    static void CheckExtensions(void);
  };

  //! RAII Helper struct for automatic block error checking.
  struct AE_RENDERAPI_CHECK
  {
  public:
    AE_RENDERAPI_CHECK(const char* szBlockName);
    ~AE_RENDERAPI_CHECK();

  private:
    aeStaticString<32> m_sBlockName;
  };
} // namespace AE_NS_GRAPHICS

#endif
