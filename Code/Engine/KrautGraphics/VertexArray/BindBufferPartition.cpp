#include "VertexArrayResource.h"

#ifdef AE_RENDERAPI_OPENGL
#  include <KrautGraphics/ShaderManager/Main.h>
#endif


namespace AE_NS_GRAPHICS
{
  void aeVertexArrayResource::BindBufferPartition(aeUInt32 uiPartition)
  {
    if (uiPartition == 0)
    {
      //if (m_bBufferZeroIsBound)
      //	return;

      m_bBufferZeroIsBound = true;
    }
    else
      m_bBufferZeroIsBound = false;

#ifdef AE_RENDERAPI_OPENGL
    if (g_RenderAPI == AE_RA_OPENGL)
    {
      BindBufferPartitionOGL(uiPartition);
    }
#endif
  }

#ifdef AE_RENDERAPI_OPENGL

  void glVertexAttribPointerOGL(aeInt32 iBindPoint, AE_VA_FORMAT Format, aeUInt32 uiStride, aeUInt32 uiOffset)
  {
    switch (Format)
    {
      case AE_VAF_R32G32B32A32_FLOAT:
        glVertexAttribPointer(iBindPoint, 4, GL_FLOAT, false, uiStride, (void*)(uiOffset));
        return;
      case AE_VAF_R32G32B32_FLOAT:
        glVertexAttribPointer(iBindPoint, 3, GL_FLOAT, false, uiStride, (void*)(uiOffset));
        return;
      case AE_VAF_R32G32_FLOAT:
        glVertexAttribPointer(iBindPoint, 2, GL_FLOAT, false, uiStride, (void*)(uiOffset));
        return;
      case AE_VAF_R32_FLOAT:
        glVertexAttribPointer(iBindPoint, 1, GL_FLOAT, false, uiStride, (void*)(uiOffset));
        return;
      case AE_VAF_R8G8B8A8_UNORM:
        glVertexAttribPointer(iBindPoint, 4, GL_UNSIGNED_BYTE, true, uiStride, (void*)(uiOffset));
        return;
      case AE_VAF_R8G8B8A8_SNORM:
        glVertexAttribPointer(iBindPoint, 4, GL_BYTE, true, uiStride, (void*)(uiOffset));
        return;
      case AE_VAF_R8G8B8A8_UINT:
        glVertexAttribPointer(iBindPoint, 4, GL_UNSIGNED_BYTE, false, uiStride, (void*)(uiOffset));
        return;
      case AE_VAF_R8G8B8A8_SINT:
        glVertexAttribPointer(iBindPoint, 4, GL_BYTE, false, uiStride, (void*)(uiOffset));
        return;
    }
  }

  void aeVertexArrayResource::BindBufferPartitionOGL(aeUInt32 uiPartition)
  {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_BufferPartitions[uiPartition].m_uiIndexBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, m_BufferPartitions[uiPartition].m_uiVertexBufferID);

    aeUInt32 uiOffset = 0;
    aeUInt32 uiSize;

    aeVertexAttribute va = m_VertexAttributes[0];

    if (m_bInterleavedData)
      uiSize = va.m_uiComponentSize;
    else
      uiSize = va.m_uiComponentSize * m_BufferPartitions[uiPartition].m_uiVertexCount;

    aeShaderManager::PrepareForRendering();

    const aeUInt32 uiAttributes = (aeUInt32)m_VertexAttributes.size();
    for (aeUInt32 attr = 1; attr < uiAttributes; ++attr)
    {
      va = m_VertexAttributes[attr];
      const aeInt32 iBind = aeShaderManager::getAttributeBindPoint(va.m_sSemantic.c_str());

      uiOffset += uiSize;

      if (m_bInterleavedData)
        uiSize = va.m_uiComponentSize;
      else
        uiSize = va.m_uiComponentSize * m_BufferPartitions[uiPartition].m_uiVertexCount;

      if (iBind > 0)
      {
        glEnableVertexAttribArray(iBind);

        if (m_bInterleavedData)
          glVertexAttribPointerOGL(iBind, va.m_ComponentFormat, m_uiVertexSize, uiOffset);
        else
          glVertexAttribPointerOGL(iBind, va.m_ComponentFormat, 0, uiOffset);
      }
    }


    glEnableVertexAttribArray(0);
    if (m_bInterleavedData)
      glVertexAttribPointer(0, 3, GL_FLOAT, false, m_uiVertexSize, 0);
    else
      glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
  }

#endif

} // namespace AE_NS_GRAPHICS
