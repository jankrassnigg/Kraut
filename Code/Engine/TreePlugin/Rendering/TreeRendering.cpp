#include "PCH.h"

#include "../Basics/Plugin.h"
#include "../GUI/qtResourceEditorWidget/MaterialLibrary.h"
#include "../Tree/Tree.h"
#include "AmbientOcclusion.h"

#include <KrautGraphics/Framebuffer/Main.h>
#include <KrautGraphics/RenderAPI/Main.h>
#include <KrautGraphics/ShaderManager/Main.h>
#include <KrautGraphics/TextureManager/Main.h>
#include <KrautGraphics/VertexArray/VertexArrayResource.h>
using namespace AE_NS_GRAPHICS;

void EnsureColorRendering(void);

void aeTree::RenderBranchMesh(aeLod::Enum lod, aeUInt32 uiBranch)
{
  // TODO: store the branch type (trunk / branch / twig) in the BranchMesh structure

  const aeUInt32 type = m_TreeStructure.m_BranchStructures[uiBranch].m_Type;

  if (!m_Descriptor.m_StructureDesc.m_BranchTypes[type].m_bVisible)
    return;

  aeShaderManager::setShaderBuilderVariable("NON_EDITABLE", false); //g_Globals.s_bManualEditingActive && !m_TreeStructure.m_Branches[b]->m_bManuallyCreated);

  const aeVec3 vCamPos = aeGLCamera::getCurrent()->getPosition();
  const aeVec3 vForwards = aeGLCamera::getCurrent()->getVectorForwards().GetNormalized();
  const aeVec3 vRight = aeGLCamera::getCurrent()->getVectorRight().GetNormalized();
  const aeVec3 vUp = aeGLCamera::getCurrent()->getVectorUp().GetNormalized();
  const aeVec3 vCenter = m_BBox.GetCenter();

  aeShaderManager::setUniformFloat("unif_CameraPosition", 3, vCamPos.x, vCamPos.y, vCamPos.z);
  aeShaderManager::setUniformFloat("unif_CameraForwards", 3, vForwards.x, vForwards.y, vForwards.z);
  aeShaderManager::setUniformFloat("unif_CameraRight", 3, vRight.x, vRight.y, vRight.z);
  aeShaderManager::setUniformFloat("unif_CameraUp", 3, vUp.x, vUp.y, vUp.z);
  aeShaderManager::setUniformFloat("unif_TreeCenter", 3, vCenter.x, vCenter.y, vCenter.z);
  aeShaderManager::setUniformFloat("unif_FrondColor", 4, m_Descriptor.m_StructureDesc.m_BranchTypes[type].m_uiVariationColor[Kraut::BranchGeometryType::Frond][0] / 255.0f, m_Descriptor.m_StructureDesc.m_BranchTypes[type].m_uiVariationColor[Kraut::BranchGeometryType::Frond][1] / 255.0f, m_Descriptor.m_StructureDesc.m_BranchTypes[type].m_uiVariationColor[Kraut::BranchGeometryType::Frond][2] / 255.0f, m_Descriptor.m_StructureDesc.m_BranchTypes[type].m_uiVariationColor[Kraut::BranchGeometryType::Frond][3] / 255.0f);
  aeShaderManager::setUniformFloat("unif_LeafColor", 4, m_Descriptor.m_StructureDesc.m_BranchTypes[type].m_uiVariationColor[Kraut::BranchGeometryType::Leaf][0] / 255.0f, m_Descriptor.m_StructureDesc.m_BranchTypes[type].m_uiVariationColor[Kraut::BranchGeometryType::Leaf][1] / 255.0f, m_Descriptor.m_StructureDesc.m_BranchTypes[type].m_uiVariationColor[Kraut::BranchGeometryType::Leaf][2] / 255.0f, m_Descriptor.m_StructureDesc.m_BranchTypes[type].m_uiVariationColor[Kraut::BranchGeometryType::Leaf][3] / 255.0f);

  aeShaderResourceHandle hShaders[Kraut::BranchGeometryType::ENUM_COUNT] = {g_Globals.s_hBranchShader, g_Globals.s_hFrondShader, g_Globals.s_hLeafShader};
  AE_FACE_CULLING CullMode[Kraut::BranchGeometryType::ENUM_COUNT] = {AE_CULL_FACE_BACK, AE_CULL_FACE_NONE, AE_CULL_FACE_NONE};
  bool bRender[Kraut::BranchGeometryType::ENUM_COUNT] = {true, g_Globals.s_bRenderLeaves, g_Globals.s_bRenderLeaves};

  for (aeUInt32 mt = 0; mt < Kraut::BranchGeometryType::ENUM_COUNT; ++mt)
  {
    if (!bRender[mt])
      continue;

    if (m_TreeRenderData[lod].m_BranchRenderData[uiBranch].m_hMeshes[mt].IsValid())
    {
      auto hTexture = aeTextureResource::LoadResource(m_Descriptor.m_StructureDesc.m_BranchTypes[type].m_sTexture[mt].c_str());
      aeShaderManager::BindTexture("tex_Diffuse", hTexture, aeBranchGeometryType::TextureSampler[mt]);

      aeTreeMaterial* pMaterial = aeTreeMaterialLibrary::GetOrAddMaterial(m_Descriptor.m_StructureDesc.m_BranchTypes[type].m_sTexture[mt].c_str());

      {
        aeHybridString<128> sTextureNormal = pMaterial->m_sTextureNormal;

        aeTextureResourceHandle hTextureN;
        if (!sTextureNormal.empty())
          hTextureN = aeTextureResource::LoadResource(sTextureNormal.c_str());
        else
          hTextureN = aeTextureResource::LoadResource("Textures/NoNormals.tga");

        aeShaderManager::BindTexture("tex_Normal", hTextureN, aeBranchGeometryType::TextureSampler[mt]);
      }

      {
        aeHybridString<128> sTextureRoughness = pMaterial->m_sTextureRoughness;

        aeTextureResourceHandle hTextureR;
        if (!sTextureRoughness.empty())
          hTextureR = aeTextureResource::LoadResource(sTextureRoughness.c_str());
        else
          hTextureR = aeTextureResource::LoadResource("Textures/NoRoughness.tga");

        aeShaderManager::BindTexture("tex_Roughness", hTextureR, aeBranchGeometryType::TextureSampler[mt]);
      }

      if ((mt == Kraut::BranchGeometryType::Leaf) && (!m_Descriptor.m_StructureDesc.m_BranchTypes[type].m_bBillboardLeaves))
        aeShaderManager::setShader(hShaders[Kraut::BranchGeometryType::Frond]);
      else
        aeShaderManager::setShader(hShaders[mt]);

      aeRenderAPI::setFaceCulling(CullMode[mt]);

      m_TreeRenderData[lod].m_BranchRenderData[uiBranch].m_hMeshes[mt].GetResource()->QueueDrawcall(0, m_TreeRenderData[lod].m_BranchRenderData[uiBranch].m_hMeshes[mt].GetResource()->getNumberOfTriangles(), 0);
      m_TreeRenderData[lod].m_BranchRenderData[uiBranch].m_hMeshes[mt].GetResource()->FlushDrawcallQueues();
    }
  }
}

void aeTree::RenderSingleBranch(aeUInt32 uiBranch)
{
  EnsureColorRendering();

  CreateTreeRenderData(aeLod::Lod0);

  //bool bTemp = g_Globals.s_bManualEditingActive;
  //g_Globals.s_bManualEditingActive = false;
  RenderBranchMesh(aeLod::Lod0, uiBranch);
  //g_Globals.s_bManualEditingActive = bTemp;
}

aeLod::Enum aeTree::GetLodToRender(const aeVec3& vCameraPosition)
{
  aeLod::Enum SelLod = g_Globals.s_CurLoD;

  if (g_Globals.s_bSelectLodAutomatically)
  {
    const float fDistance = vCameraPosition.GetLength();

    SelLod = aeLod::Lod0;

    for (aeUInt32 l = aeLod::Lod0; l < aeLod::ENUM_COUNT; ++l)
    {
      if (m_Descriptor.m_LodData[l].m_Mode == Kraut::LodMode::Disabled)
        break;

      SelLod = (aeLod::Enum)l;

      if (fDistance <= (float)m_Descriptor.m_LodData[l].m_uiLodDistance)
        break;
    }
  }

  return SelLod;
}

void aeTree::Render(const aeVec3& vCameraPosition, bool bForExport, bool bForExportPreview)
{
  if (bForExport)
  {
    EnsureColorRendering();
    RenderTree(aeLod::None);
    return;
  }

  const aeLod::Enum SelLod = GetLodToRender(vCameraPosition);

  if (g_Globals.s_RenderMode == aeTreeRenderMode::Skeleton)
  {
    RenderSkeleton(SelLod);
  }
  else
  {
    aeRenderAPI::setWireframeMode(g_Globals.s_RenderMode == aeTreeRenderMode::Wireframe);
    aeShaderManager::setShaderBuilderVariable("WIREFRAME", g_Globals.s_RenderMode == aeTreeRenderMode::Wireframe);
    aeShaderManager::setShaderBuilderVariable("OUTPUT_NORMALS_ONLY", g_Globals.s_RenderMode == aeTreeRenderMode::Normals);
    aeShaderManager::setShaderBuilderVariable("OUTPUT_DIFFUSE_ONLY", g_Globals.s_RenderMode == aeTreeRenderMode::Diffuse);
    aeShaderManager::setShaderBuilderVariable("OUTPUT_LIGHT_ONLY", g_Globals.s_RenderMode == aeTreeRenderMode::Light);
    aeShaderManager::setShaderBuilderVariable("OUTPUT_AMBIENT_ONLY", g_Globals.s_RenderMode == aeTreeRenderMode::Ambient);
    aeShaderManager::setShaderBuilderVariable("OUTPUT_SKYLIGHT_ONLY", g_Globals.s_RenderMode == aeTreeRenderMode::Skylight);

    if ((g_Globals.s_bSelectLodAutomatically) && (m_Descriptor.m_fLodCrossFadeTransition > 0.0f))
    {
      aeShaderManager::setShaderBuilderVariable("CROSSFADE", true);

      for (aeInt32 lod = aeLod::Lod0; lod < aeLod::ENUM_COUNT; ++lod)
        RenderTree((aeLod::Enum)lod);
    }
    else
    {
      aeShaderManager::setShaderBuilderVariable("CROSSFADE", false);
      RenderTree(SelLod);
    }

    aeRenderAPI::setWireframeMode(false);
  }

  if (!bForExportPreview)
  {
    RenderCollisionMesh();
    RenderGroundPlane();
    RenderForces(false);
  }
}

void aeTree::RenderTreeMesh(aeLod::Enum lod)
{
  CreateTreeRenderData(lod);

  aeShaderManager::setUniformFloat("unif_CrossFadeOutDist", 1, m_Descriptor.m_LodData[lod].m_uiLodDistance);
  aeShaderManager::setUniformFloat("unif_CrossFadeOutRange", 1, m_Descriptor.m_LodData[lod].m_uiLodDistance * m_Descriptor.m_fLodCrossFadeTransition);

  if (lod == aeLod::Lod0)
  {
    aeShaderManager::setUniformFloat("unif_CrossFadeInDist", 1, -5);
    aeShaderManager::setUniformFloat("unif_CrossFadeInRange", 1, 1);
  }
  else
  {
    aeShaderManager::setUniformFloat("unif_CrossFadeInDist", 1, m_Descriptor.m_LodData[lod - 1].m_uiLodDistance);
    aeShaderManager::setUniformFloat("unif_CrossFadeInRange", 1, m_Descriptor.m_LodData[lod - 1].m_uiLodDistance * m_Descriptor.m_fLodCrossFadeTransition);
  }

  for (aeUInt32 b = 0; b < m_TreeRenderData[lod].m_BranchRenderData.size(); ++b)
  {
    RenderBranchMesh(lod, b);
  }
}

void aeTree::RenderTree(aeLod::Enum lod)
{
  if (m_Descriptor.m_LodData[lod].m_Mode == Kraut::LodMode::Disabled)
    return;

  if (m_Descriptor.m_LodData[lod].m_Mode == Kraut::LodMode::Full)
  {
    RenderTreeMesh(lod);
  }
  else
  {
    RenderTreeImpostor(lod);
  }
}

inline static aeUInt8 fToU8(float f)
{
  return (aeUInt8)aeMath::Clamp((f * 0.5f + 0.5f) * 255.0f, 0.0f, 255.0f);
}

void aeTree::CreateTreeRenderData(aeLod::Enum lod)
{
  if (m_bLodRenderDataGenerated[lod])
    return;

  m_bLodRenderDataGenerated[lod] = true;

  EnsureMeshIsGenerated(lod);

  m_BranchRenderInfo.clear();
  m_BranchRenderInfo.resize(m_TreeMesh[lod].m_BranchMeshes.size());

  m_TreeRenderData[lod].Clear();
  m_TreeRenderData[lod].m_BranchRenderData.resize(m_TreeMesh[lod].m_BranchMeshes.size());

  for (aeUInt32 b = 0; b < m_TreeStructure.m_BranchStructures.size(); ++b)
  {
    const aeUInt32 uiPickingID = aeEditorPlugin::RegisterPickableObject(&m_TreeStructure.m_BranchStructures[b], b, "aeTreeBranch");

    m_BranchRenderInfo[b].m_uiPickID = uiPickingID;

    for (aeUInt32 mt = 0; mt < Kraut::BranchGeometryType::ENUM_COUNT; ++mt)
    {
      Kraut::BranchMesh& branchMesh = m_TreeMesh[lod].m_BranchMeshes[b];
      aeBranchRenderData& branchRenderData = m_TreeRenderData[lod].m_BranchRenderData[b];

      if ((!branchRenderData.m_hMeshes[mt].IsValid()) && (branchMesh.m_Mesh[mt].GetNumTriangles() > 0))
      {
        aeVertexArrayDescriptor vad;
        vad.SetupDescriptor(branchMesh.m_Mesh[mt].m_Vertices.size(), branchMesh.m_Mesh[mt].GetNumTriangles(), AE_VAU_STATIC, false);
        vad.AddVertexAttribute("attr_TexCoords", AE_VAF_R32G32B32_FLOAT);
        vad.AddVertexAttribute("attr_Normal", AE_VAF_R8G8B8A8_UNORM);
        vad.AddVertexAttribute("attr_Tangent", AE_VAF_R8G8B8A8_UNORM);
        vad.AddVertexAttribute("attr_BiTangent", AE_VAF_R8G8B8A8_UNORM);
        vad.AddVertexAttribute("attr_PickingID", AE_VAF_R8G8B8A8_UNORM);
        vad.AddVertexAttribute("attr_PickingSubID", AE_VAF_R8G8B8A8_UNORM);
        vad.BeginSettingAttributes();

        branchMesh.m_BoundingBox[mt].SetInvalid();

        for (aeUInt32 v = 0; v < branchMesh.m_Mesh[mt].m_Vertices.size(); ++v)
        {
          const Kraut::Vertex& vtx = branchMesh.m_Mesh[mt].m_Vertices[v];

          if (vtx.m_vNormal.IsZeroVector(0.001f)) // billboard leaf
          {
            aeUInt8 uiAO[6];
            uiAO[0] = (aeUInt8)(aeAOGrid::GetAmbientOcclusionAt(vtx.m_vPosition + aeVec3::GetAxisX() * 0.2f, aeVec3::GetAxisX()) * 255.0f);
            uiAO[1] = (aeUInt8)(aeAOGrid::GetAmbientOcclusionAt(vtx.m_vPosition - aeVec3::GetAxisX() * 0.2f, -aeVec3::GetAxisX()) * 255.0f);
            uiAO[2] = (aeUInt8)(aeAOGrid::GetAmbientOcclusionAt(vtx.m_vPosition + aeVec3::GetAxisY() * 0.2f, aeVec3::GetAxisY()) * 255.0f);
            uiAO[3] = (aeUInt8)(aeAOGrid::GetAmbientOcclusionAt(vtx.m_vPosition - aeVec3::GetAxisY() * 0.2f, -aeVec3::GetAxisY()) * 255.0f);
            uiAO[4] = (aeUInt8)(aeAOGrid::GetAmbientOcclusionAt(vtx.m_vPosition + aeVec3::GetAxisZ() * 0.2f, aeVec3::GetAxisZ()) * 255.0f);
            uiAO[5] = (aeUInt8)(aeAOGrid::GetAmbientOcclusionAt(vtx.m_vPosition - aeVec3::GetAxisZ() * 0.2f, -aeVec3::GetAxisZ()) * 255.0f);

            vad.setVertexAttribute4ub(2, v, uiAO[0], uiAO[1], uiAO[2], uiAO[3]);                                                           // normal (encodes the first four ambient occlusion values)
            vad.setVertexAttribute4ub(3, v, uiAO[4], uiAO[5], (aeUInt8)(vtx.m_vTangent.x * 255.0f), (aeUInt8)(vtx.m_vTangent.y * 255.0f)); // tangent (encodes the billboard span factor, and the last two ambient occlusion factors)
            vad.setVertexAttribute4ub(4, v, 0, 0, 0, vtx.m_uiColorVariation);                                                              // bitangent (not used in shader)
          }
          else
          {
            const aeUInt8 uiAO = (aeUInt8)(aeAOGrid::GetAmbientOcclusionAt(vtx.m_vPosition, vtx.m_vNormal) * 255.0f);
            const aeUInt8 uiAO2 = (aeUInt8)(aeAOGrid::GetAmbientOcclusionAt(vtx.m_vPosition, -vtx.m_vNormal) * 255.0f);
            vad.setVertexAttribute4ub(2, v, fToU8(vtx.m_vNormal.x), fToU8(vtx.m_vNormal.y), fToU8(vtx.m_vNormal.z), uiAO);
            vad.setVertexAttribute4ub(3, v, fToU8(vtx.m_vTangent.x), fToU8(vtx.m_vTangent.y), fToU8(vtx.m_vTangent.z), uiAO2);
            vad.setVertexAttribute4ub(4, v, fToU8(vtx.m_vBiTangent.x), fToU8(vtx.m_vBiTangent.y), fToU8(vtx.m_vBiTangent.z), vtx.m_uiColorVariation);
          }

          branchMesh.m_BoundingBox[mt].ExpandToInclude(vtx.m_vPosition);

          vad.setVertexAttribute3f(0, v, vtx.m_vPosition.x, vtx.m_vPosition.y, vtx.m_vPosition.z);
          vad.setVertexAttribute3f(1, v, vtx.m_vTexCoord.x, vtx.m_vTexCoord.y, vtx.m_vTexCoord.z);
        }

        for (aeUInt32 q = 0; q < branchMesh.m_Mesh[mt].GetNumTriangles(); ++q)
        {
          const Kraut::Triangle& t = branchMesh.m_Mesh[mt].m_Triangles[q];

          for (int i = 0; i < 3; ++i)
          {
            const Kraut::Vertex& vtx = branchMesh.m_Mesh[mt].m_Vertices[t.m_uiVertexIDs[i]];

            vad.setVertexAttribute4ub(5, t.m_uiVertexIDs[i], (uiPickingID >> 0) & 0xFF, (uiPickingID >> 8) & 0xFF, (uiPickingID >> 16) & 0xFF, (uiPickingID >> 24) & 0xFF);
            vad.setVertexAttribute4ub(6, t.m_uiVertexIDs[i], (t.m_uiPickingSubID >> 0) & 0xFF, (t.m_uiPickingSubID >> 8) & 0xFF, (t.m_uiPickingSubID >> 16) & 0xFF, (t.m_uiPickingSubID >> 24) & 0xFF);
          }

          vad.setTriangle(q, t.m_uiVertexIDs[0], t.m_uiVertexIDs[1], t.m_uiVertexIDs[2]);
        }

        aeVertexArrayResource::CreateResource(branchRenderData.m_hMeshes[mt], &vad);
      }
    }
  }
}

void aeTree::ClearTreeRenderData(void)
{
  for (aeUInt32 lod = 0; lod < aeLod::ENUM_COUNT; ++lod)
  {
    m_TreeRenderData[lod].Clear();
    m_LodImpostors[lod].Clear();
  }
}
