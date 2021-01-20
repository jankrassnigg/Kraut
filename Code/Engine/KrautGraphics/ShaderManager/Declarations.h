// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#ifndef AE_GRAPHICS_SHADERMANAGER_DECLARATIONS_H
#define AE_GRAPHICS_SHADERMANAGER_DECLARATIONS_H


// Include this as the first Header everywhere
#include "../Base.h"

#include <KrautGraphics/ResourceManager/ResourceManager.h>
#include <KrautGraphics/TextureManager/Declarations.h>

namespace AE_NS_GRAPHICS
{
  using namespace AE_NS_GRAPHICS;
  using namespace AE_NS_GRAPHICS;

  class aeShaderObject;
  class aeShaderResource;
  class aeShaderManagerStartup;
  struct aeConstantBuffer;
  struct aeShaderVariable;


  enum AE_GS_INOUT_TYPE
  {
    AE_GS_IOT_POINTS,
    AE_GS_IOT_LINES,
    AE_GS_IOT_TRIANGLES,
    AE_GS_IOT_LINE_STRIP,
    AE_GS_IOT_TRIANGLE_STRIP,
  };

  struct aeUsedConstantBuffer
  {
    aeUInt32 m_uiTargetSlot;
    aeConstantBuffer* m_pBuffer;
  };


  struct aeBindTexture
  {
    aeTextureResourceHandle m_hTexture;
    aeInt32 m_iSampler;
  };

  struct aeShaderTypeData
  {
#ifdef AE_RENDERAPI_OPENGL
    aeUInt32 m_ShaderOGL;

    static const aeUInt32 INVALID_SHADER_OGL = 0xFFFFFFFF;
#endif

    AE_GS_INOUT_TYPE m_GSInputType;
    AE_GS_INOUT_TYPE m_GSOutputType;
    aeUInt32 m_GSOutputVertices;

    aeShaderTypeData()
    {
#ifdef AE_RENDERAPI_OPENGL
      m_ShaderOGL = INVALID_SHADER_OGL;
#endif

      m_GSInputType = AE_GS_IOT_TRIANGLES;
      m_GSOutputType = AE_GS_IOT_TRIANGLE_STRIP;
      m_GSOutputVertices = 0;
    }
  };

  inline bool operator==(const aeShaderTypeData& lhs, const aeShaderTypeData& rhs)
  {
#ifdef AE_RENDERAPI_OPENGL
    return (lhs.m_ShaderOGL == rhs.m_ShaderOGL);
#endif
  }

  inline bool operator!=(const aeShaderTypeData& lhs, const aeShaderTypeData& rhs)
  {
#ifdef AE_RENDERAPI_OPENGL
    return (lhs.m_ShaderOGL != rhs.m_ShaderOGL);
#endif
  }

  struct AE_GPU_VENDOR
  {
    enum Enum
    {
      AE_VENDOR_UNCHECKED,
      AE_VENDOR_UNKNOWN,
      AE_VENDOR_NVIDIA,
      AE_VENDOR_ATI,
    };
  };

  enum AE_SHADER_TYPE
  {
    AE_ST_VERTEX,
    AE_ST_FRAGMENT,
    AE_ST_GEOMETRY,
  };

  struct aeShaderCombi
  {
    aeShaderTypeData* m_pVertexShaderID;
    aeShaderTypeData* m_pFragmentShaderID;
    aeShaderTypeData* m_pGeometryShaderID;

    aeShaderCombi()
    {
      m_pVertexShaderID = nullptr;
      m_pFragmentShaderID = nullptr;
      m_pGeometryShaderID = nullptr;
    }

    inline bool operator<(const aeShaderCombi& cc) const
    {
      if (m_pVertexShaderID < cc.m_pVertexShaderID)
        return (true);
      if (m_pVertexShaderID > cc.m_pVertexShaderID)
        return (false);

      if (m_pFragmentShaderID < cc.m_pFragmentShaderID)
        return (true);
      if (m_pFragmentShaderID > cc.m_pFragmentShaderID)
        return (false);

      return (m_pGeometryShaderID < cc.m_pGeometryShaderID);
    }
  };

  // typedef for a Handle to a ShaderResource
  typedef aeTypedResourceHandle<aeShaderResource> aeShaderResourceHandle;
} // namespace AE_NS_GRAPHICS

#endif
