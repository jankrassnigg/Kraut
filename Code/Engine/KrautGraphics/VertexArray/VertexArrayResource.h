// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#ifndef AE_GRAPHICS_VERTEXARRAY_VERTEXARRAYRESOURCE_H
#define AE_GRAPHICS_VERTEXARRAY_VERTEXARRAYRESOURCE_H

#pragma once

#include "Declarations.h"
#include "VertexArrayDescriptor.h"
#include <KrautGraphics/ResourceManager/ResourceManager.h>
#include <KrautFoundation/Containers/Map.h>
#include <KrautGraphics/RenderAPI/Main.h>
#include <deque>
#include <map>

namespace AE_NS_GRAPHICS
{
  class AE_GRAPHICS_DLL aeVertexArrayResource : public aeResource
  {
    AE_RESOURCE_DECLARATION(aeVertexArrayResource);

  public:
    aeVertexArrayResource();

    static void EnableMaterialChangeCallback(bool bEnable);

    void setMaterialChangeCallback(AE_MATERIAL_CHANGE_CALLBACK cb, void* pPassThrough);

    void QueueDrawcall(aeUInt32 uiFirstTriangle, aeUInt32 uiTriangleCount, aeUInt32 uiMaterial, aeOcclusionQueryObject* pOcclusionQuery = nullptr);
    void FlushDrawcallQueues(void);

    void RenderInstances(aeUInt32 uiInstances, aeUInt32 uiFirstTriangle, aeUInt32 uiTriangleCount);

    void RenderWithBaseIndex(aeUInt32 uiFirstTriangle, aeUInt32 uiTriangleCount, aeUInt32 uiBaseIndex);

  public:
    aeUInt32 getNumberOfVertices(void) const;
    aeUInt32 getNumberOfTriangles(void) const;
    aeUInt32 getNumberOfIndices(void) const;


  private:
    //virtual void LoadResourceFromFile (const char* szFilename);

    //! Creates the Resource from a descriptor
    virtual void CreateResourceFromDescriptor(void* pDescriptor);

    //! Frees the Resource's data
    virtual void UnloadResource(void);

  private:
    void BindBufferPartition(aeUInt32 uiPartition);
    void UploadBufferPartition(aeUInt32 uiFirstVertex, aeUInt32 uiVertexCount, aeUInt32 uiFirstTriangle, aeUInt32 uiTriangleCount, aeVertexArrayDescriptor* pDesc);

    static bool s_bIgnoreMaterialChange;

    AE_MATERIAL_CHANGE_CALLBACK m_MaterialChangeCallback;
    void* m_pMaterialChangePassThrough;

    aeUInt32 m_uiNumberOfVertices;
    aeUInt32 m_uiNumberOfTriangles;
    aeUInt32 m_uiNumberOfIndices;
    aeUInt32 m_uiVertexSize;
    bool m_bInterleavedData;
    bool m_bBufferZeroIsBound;

    struct aeBufferPartition
    {
      aeBufferPartition();

#ifdef AE_RENDERAPI_OPENGL
      aeUInt32 m_uiVertexBufferID;
      aeUInt32 m_uiIndexBufferID;
#endif

      aeUInt32 m_uiFirstVertex;
      aeUInt32 m_uiVertexCount;

      aeUInt32 m_uiFirstTriangle;
      aeUInt32 m_uiTriangleCount;

      aeUInt8 m_uiBytesPerIndex;
    };

    aeArray<aeBufferPartition> m_BufferPartitions;

    struct aeVertexAttribute
    {
      aeStaticString<32> m_sSemantic;
      AE_VA_FORMAT m_ComponentFormat;
      aeUInt32 m_uiComponentSize;
    };

    aeArray<aeVertexAttribute> m_VertexAttributes;

    struct aeDrawcall
    {
      aeUInt32 m_uiFirstTriangle;
      aeUInt32 m_uiTriangleCount;

      aeOcclusionQueryObject* m_pConditionalRender;
    };

    void StoreDrawcall(aeUInt32 uiFirstTriangle, aeUInt32 uiTriangleCount, std::deque<aeDrawcall>& Queue, aeUInt32 uiMaterial, bool bReplace, aeOcclusionQueryObject* pOcclusionQuery);
    void FlushMaterialQueue(std::deque<aeDrawcall>& Queue, aeUInt32 uiMaterial);
    void IssueDrawcall(const aeDrawcall& dc);

    // one queue per material index
    aeMap<aeUInt32, std::deque<aeDrawcall>> m_DrawcallQueues;

#ifdef AE_RENDERAPI_OPENGL
    void BindBufferPartitionOGL(aeUInt32 uiPartition);
#endif
  };



} // namespace AE_NS_GRAPHICS

#endif
