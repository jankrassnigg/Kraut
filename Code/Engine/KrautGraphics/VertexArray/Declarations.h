// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#ifndef AE_GRAPHICS_VERTEXARRAY_DECLARATIONS_H
#define AE_GRAPHICS_VERTEXARRAY_DECLARATIONS_H

#include "../Base.h"
#include <KrautGraphics/ResourceManager/TypedResourceHandle.h>

#include <deque>
#include <vector>

namespace AE_NS_GRAPHICS
{
  using namespace AE_NS_FOUNDATION;
  using namespace AE_NS_GRAPHICS;

  class aeVertexArrayDescriptor;
  class aeVertexArrayResource;

  //! Vertex Array Usage
  enum AE_VA_USAGE
  {
    AE_VAU_NONE,
    AE_VAU_STATIC,
    AE_VAU_DYNAMIC,
    AE_VAU_STREAM,
  };

  //! All currently supported Vertex Array formats.
  enum AE_VA_FORMAT
  {
    AE_VAF_R32G32B32A32_FLOAT,
    AE_VAF_R32G32B32_FLOAT,
    AE_VAF_R32G32_FLOAT,
    AE_VAF_R32_FLOAT,
    AE_VAF_R8G8B8A8_UNORM,
    AE_VAF_R8G8B8A8_SNORM,
    AE_VAF_R8G8B8A8_UINT,
    AE_VAF_R8G8B8A8_SINT,
  };

  //! Callback-typedef for when the queued drawcalls are executed, to inform the app to bind the proper material.
  typedef void (*AE_MATERIAL_CHANGE_CALLBACK)(aeUInt32 uiMaterial, void* pPassThrough);

  // typedef for a Handle to a TextureResource
  typedef aeTypedResourceHandle<aeVertexArrayResource> aeVertexArrayResourceHandle;

} // namespace AE_NS_GRAPHICS

#pragma once

#endif
