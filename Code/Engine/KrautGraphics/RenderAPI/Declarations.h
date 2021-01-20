// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#ifndef AE_GRAPHICS_RENDERAPI_DECLARATIONS_H
#define AE_GRAPHICS_RENDERAPI_DECLARATIONS_H

// Include this as the first Header everywhere
#include "../Base.h"
#include <KrautGraphics/Logging/Log.h>
#include <KrautFoundation/Math/Declarations.h>

namespace AE_NS_GRAPHICS
{
  using namespace AE_NS_FOUNDATION;

  class aeRenderAPI;
  struct aeRenderAPI_Impl;
  class aeOcclusionQueryObject;
  class aeMatrixStack;
  class aeRenderAPIStartup;

  struct aeRenderState;
  struct aeStencilState;
  struct aeFramebufferState;

  enum AE_RENDERAPI
  {
    AE_RA_NONE,
    AE_RA_OPENGL,
    AE_RA_D3D11,
  };

  extern AE_RENDERAPI g_RenderAPI;

  enum AE_RA_VENDOR
  {
    AE_VENDOR_UNKNOWN = 0,
    AE_VENDOR_ATI = 1,
    AE_VENDOR_NVIDIA = 2,
  };

  enum AE_FACE_STENCIL_TEST
  {
    AE_FRONT_FACE_STENCIL_TEST,
    AE_BACK_FACE_STENCIL_TEST,
    AE_FRONT_AND_BACK_FACE_STENCIL_TEST,
  };

  // only necessary for the OpenGL port
  enum AE_INDEX_TYPE
  {
    AE_UNKNOWN,
    AE_UNSIGNED_SHORT,
    AE_UNSIGNED_INT,
  };

  enum AE_RENDER_MODE
  {
    AE_RENDER_UNKNOWN,
    AE_RENDER_POINTS,
    AE_RENDER_LINES,
    AE_RENDER_TRIANGLES,
  };

  enum AE_COMPARE_FUNC
  {
    AE_COMPARE_NEVER,
    AE_COMPARE_LESS,
    AE_COMPARE_EQUAL,
    AE_COMPARE_LEQUAL,
    AE_COMPARE_GREATER,
    AE_COMPARE_NOTEQUAL,
    AE_COMPARE_GEQUAL,
    AE_COMPARE_ALWAYS,
  };

  enum AE_FACE_CULLING
  {
    AE_CULL_FACE_NONE,
    AE_CULL_FACE_FRONT,
    AE_CULL_FACE_BACK,
  };


  enum AE_BLEND_FUNC
  {
    AE_FUNC_ADD,
    AE_FUNC_SUBTRACT,
    AE_FUNC_REV_SUBTRACT,
    AE_FUNC_MIN,
    AE_FUNC_MAX,
  };

  enum AE_BLEND_FACTOR
  {
    AE_FACTOR_ZERO,
    AE_FACTOR_ONE,
    AE_FACTOR_SRC_COLOR,
    AE_FACTOR_INV_SRC_COLOR,
    AE_FACTOR_SRC_ALPHA,
    AE_FACTOR_INV_SRC_ALPHA,
    AE_FACTOR_DST_COLOR,
    AE_FACTOR_INV_DST_COLOR,
    AE_FACTOR_DST_ALPHA,
    AE_FACTOR_INV_DST_ALPHA,
    AE_FACTOR_BLEND_COLOR,
    AE_FACTOR_INV_BLEND_COLOR,
  };

  enum AE_STENCIL_OP
  {
    AE_STENCIL_KEEP,
    AE_STENCIL_ZERO,
    AE_STENCIL_REPLACE,
    AE_STENCIL_INCR_SAT,
    AE_STENCIL_DECR_SAT,
    AE_STENCIL_INCR_WRAP,
    AE_STENCIL_DECR_WRAP,
    AE_STENCIL_INVERT,
  };

  AE_GRAPHICS_DLL bool ProjectOnScreen(const aeMatrix& TransformationMatrix, const aeUInt32 uiViewport[4], const aeVec3& vPoint, aeVec3& out_vScreenPos);
  AE_GRAPHICS_DLL bool ProjectOnScreen(const aeMatrix& TransformationMatrix, const aeUInt32 uiViewport[4], const aeVec3* vPoints, aeVec3* out_vScreenPos, aeUInt32 uiPoints);
  AE_GRAPHICS_DLL bool UnProjectFromScreen(const aeMatrix& InverseTransformationMatrix, const aeUInt32 uiViewport[4], const aeVec3& vScreenPos, aeVec3& out_Point);
  AE_GRAPHICS_DLL bool UnProjectFromScreen(const aeMatrix& InverseTransformationMatrix, const aeUInt32 uiViewport[4], const aeVec3* vScreenPos, aeVec3* out_Points, aeUInt32 uiPoints);
} // namespace AE_NS_GRAPHICS

#include "DeclarationsD3D11.h"
#include "DeclarationsOGL.h"

#endif
