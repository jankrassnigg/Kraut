#include "PCH.h"

#include "../Tree/Tree.h"
#include "../Basics/Plugin.h"
#include "../GUI/qtResourceEditorWidget/MaterialLibrary.h"
#include "AmbientOcclusion.h"

#include <KrautGraphics/RenderAPI/Main.h>
#include <KrautGraphics/TextureManager/Main.h>
#include <KrautGraphics/ShaderManager/Main.h>
#include <KrautGraphics/Framebuffer/Main.h>
#include <KrautGraphics/VertexArray/VertexArrayResource.h>
using namespace AE_NS_GRAPHICS;

void EnsureColorRendering (void);

void aeTree::RenderBranchMesh (aeLod::Enum lod, aeUInt32 uiBranch)
{
  aeUInt32 b = uiBranch;

  if (m_Branches[b].IsDeleted ())
    return;

  aeUInt32 n = m_Branches[b].m_Type;

  if (!m_Descriptor.m_BranchTypes[n].m_bVisible)
    return;

  aeShaderManager::setShaderBuilderVariable ("NON_EDITABLE", g_Globals.s_bManualEditingActive && !m_Branches[b].m_bManuallyCreated);

  const aeVec3 vCamPos   = aeGLCamera::getCurrent ()->getPosition ();
  const aeVec3 vForwards = aeGLCamera::getCurrent ()->getVectorForwards ().GetNormalized();
  const aeVec3 vRight    = aeGLCamera::getCurrent ()->getVectorRight ().GetNormalized();
  const aeVec3 vUp       = aeGLCamera::getCurrent ()->getVectorUp ().GetNormalized();
  const aeVec3 vCenter   = m_BBox.GetCenter ();

  aeShaderManager::setUniformFloat ("unif_CameraPosition", 3, vCamPos.x, vCamPos.y, vCamPos.z);
  aeShaderManager::setUniformFloat ("unif_CameraForwards", 3, vForwards.x, vForwards.y, vForwards.z);
  aeShaderManager::setUniformFloat ("unif_CameraRight", 3, vRight.x, vRight.y, vRight.z);
  aeShaderManager::setUniformFloat ("unif_CameraUp", 3, vUp.x, vUp.y, vUp.z);
  aeShaderManager::setUniformFloat ("unif_TreeCenter", 3, vCenter.x, vCenter.y, vCenter.z);
  aeShaderManager::setUniformFloat ("unif_FrondColor", 4, m_Descriptor.m_BranchTypes[n].m_uiVariationColor[aeMeshType::Frond][0] / 255.0f, m_Descriptor.m_BranchTypes[n].m_uiVariationColor[aeMeshType::Frond][1] / 255.0f, m_Descriptor.m_BranchTypes[n].m_uiVariationColor[aeMeshType::Frond][2] / 255.0f, m_Descriptor.m_BranchTypes[n].m_uiVariationColor[aeMeshType::Frond][3] / 255.0f);
  aeShaderManager::setUniformFloat ("unif_LeafColor",  4, m_Descriptor.m_BranchTypes[n].m_uiVariationColor[aeMeshType::Leaf][0] / 255.0f,  m_Descriptor.m_BranchTypes[n].m_uiVariationColor[aeMeshType::Leaf][1] / 255.0f,  m_Descriptor.m_BranchTypes[n].m_uiVariationColor[aeMeshType::Leaf][2] / 255.0f,  m_Descriptor.m_BranchTypes[n].m_uiVariationColor[aeMeshType::Leaf][3] / 255.0f);

  aeShaderResourceHandle hShaders[aeMeshType::ENUM_COUNT] = { g_Globals.s_hBranchShader, g_Globals.s_hFrondShader, g_Globals.s_hLeafShader };
  AE_FACE_CULLING CullMode[aeMeshType::ENUM_COUNT] = { AE_CULL_FACE_BACK, AE_CULL_FACE_NONE, AE_CULL_FACE_NONE };
  bool bRender[aeMeshType::ENUM_COUNT] = { true, g_Globals.s_bRenderLeaves, g_Globals.s_bRenderLeaves };

  for (aeUInt32 mt = 0; mt < aeMeshType::ENUM_COUNT; ++mt)
  {
    if (!bRender[mt])
      continue;

    if (m_Branches[b].m_LODs[lod].m_hMeshes[mt].IsValid ())
    {
      m_Descriptor.m_BranchTypes[n].m_hTexture[mt] = aeTextureResource::LoadResource (m_Descriptor.m_BranchTypes[n].m_sTexture[mt].c_str ());
      aeShaderManager::BindTexture ("tex_Diffuse", m_Descriptor.m_BranchTypes[n].m_hTexture[mt], aeMeshType::TextureSampler[mt]);

      aeTreeMaterial* pMaterial = aeTreeMaterialLibrary::GetOrAddMaterial (m_Descriptor.m_BranchTypes[n].m_sTexture[mt].c_str ());
      aeHybridString<128> sTextureNormal = pMaterial->m_sTextureNormal;

      if (!sTextureNormal.empty ())
        m_Descriptor.m_BranchTypes[n].m_hTextureN[mt] = aeTextureResource::LoadResource (sTextureNormal.c_str ());
      else
        m_Descriptor.m_BranchTypes[n].m_hTextureN[mt] = aeTextureResource::LoadResource ("Textures/NoNormals.tga");

      aeShaderManager::BindTexture ("tex_Normal", m_Descriptor.m_BranchTypes[n].m_hTextureN[mt], aeMeshType::TextureSampler[mt]);

      if ((mt == aeMeshType::Leaf) && (!m_Descriptor.m_BranchTypes[n].m_bBillboardLeaves))
        aeShaderManager::setShader (hShaders[aeMeshType::Frond]);
      else
        aeShaderManager::setShader (hShaders[mt]);

      aeRenderAPI::setFaceCulling (CullMode[mt]);

      m_Branches[b].m_LODs[lod].m_hMeshes[mt].GetResource ()->QueueDrawcall (0, m_Branches[b].m_LODs[lod].m_hMeshes[mt].GetResource ()->getNumberOfTriangles (), 0);
      m_Branches[b].m_LODs[lod].m_hMeshes[mt].GetResource ()->FlushDrawcallQueues ();
    }
  }
}

void aeTree::RenderSingleBranch (aeUInt32 uiBranch)
{
  EnsureColorRendering ();

  CreateTreeRenderData (aeLod::Lod0);

  bool bTemp = g_Globals.s_bManualEditingActive;
  g_Globals.s_bManualEditingActive = false;
  RenderBranchMesh (aeLod::Lod0, uiBranch);
  g_Globals.s_bManualEditingActive = bTemp;
}

aeLod::Enum aeTree::GetLodToRender (const aeVec3& vCameraPosition)
{
  aeLod::Enum SelLod = g_Globals.s_CurLoD;

  if (g_Globals.s_bSelectLodAutomatically)
  {
    const float fDistance = vCameraPosition.GetLength ();

    SelLod = aeLod::Lod0;

    for (aeUInt32 l = aeLod::Lod0; l < aeLod::ENUM_COUNT; ++l)
    {
      if (m_Descriptor.m_LodData[l].m_Mode == aeLodMode::Disabled)
        break;

      SelLod = (aeLod::Enum) l;

      if (fDistance <= (float) m_Descriptor.m_LodData[l].m_uiLodDistance)
        break;
    }
  }

  return SelLod;
}

void aeTree::Render (const aeVec3& vCameraPosition, bool bForExport, bool bForExportPreview)
{
  if (bForExport)
  {
    EnsureColorRendering ();
    RenderTree (aeLod::None);
    return;
  }

  const aeLod::Enum SelLod = GetLodToRender (vCameraPosition);

  if (g_Globals.s_RenderMode == aeTreeRenderMode::Skeleton)
  {
    RenderSkeleton (SelLod);
  }
  else
  {
    aeRenderAPI::setWireframeMode (g_Globals.s_RenderMode == aeTreeRenderMode::Wireframe);
    aeShaderManager::setShaderBuilderVariable ("WIREFRAME", g_Globals.s_RenderMode == aeTreeRenderMode::Wireframe);
    aeShaderManager::setShaderBuilderVariable ("OUTPUT_NORMALS_ONLY", g_Globals.s_RenderMode == aeTreeRenderMode::Normals);
    aeShaderManager::setShaderBuilderVariable ("OUTPUT_DIFFUSE_ONLY", g_Globals.s_RenderMode == aeTreeRenderMode::Diffuse);
    aeShaderManager::setShaderBuilderVariable ("OUTPUT_LIGHT_ONLY", g_Globals.s_RenderMode == aeTreeRenderMode::Light);
    aeShaderManager::setShaderBuilderVariable ("OUTPUT_AMBIENT_ONLY", g_Globals.s_RenderMode == aeTreeRenderMode::Ambient);
    aeShaderManager::setShaderBuilderVariable ("OUTPUT_SKYLIGHT_ONLY", g_Globals.s_RenderMode == aeTreeRenderMode::Skylight);

    if ((g_Globals.s_bSelectLodAutomatically) && (m_Descriptor.m_fLodCrossFadeTransition > 0.0f))
    {
      aeShaderManager::setShaderBuilderVariable ("CROSSFADE", true);

      for (aeInt32 lod = aeLod::Lod0; lod < aeLod::ENUM_COUNT; ++lod)
        RenderTree ((aeLod::Enum) lod);
    }
    else
    {
      aeShaderManager::setShaderBuilderVariable ("CROSSFADE", false);
      RenderTree (SelLod);
    }

    aeRenderAPI::setWireframeMode (false);
  }

  if (!bForExportPreview)
  {
    RenderCollisionMesh ();
    RenderGroundPlane ();
    RenderForces (false);
  }
}

void aeTree::RenderTreeMesh (aeLod::Enum lod)
{
  CreateTreeRenderData (lod);

  aeShaderManager::setUniformFloat ("unif_CrossFadeOutDist", 1, m_Descriptor.m_LodData[lod].m_uiLodDistance);
  aeShaderManager::setUniformFloat ("unif_CrossFadeOutRange", 1, m_Descriptor.m_LodData[lod].m_uiLodDistance * m_Descriptor.m_fLodCrossFadeTransition);

  if (lod == aeLod::Lod0)
  {
    aeShaderManager::setUniformFloat ("unif_CrossFadeInDist", 1, -5);
    aeShaderManager::setUniformFloat ("unif_CrossFadeInRange", 1, 1);
  }
  else
  {
    aeShaderManager::setUniformFloat ("unif_CrossFadeInDist", 1, m_Descriptor.m_LodData[lod-1].m_uiLodDistance);
    aeShaderManager::setUniformFloat ("unif_CrossFadeInRange", 1, m_Descriptor.m_LodData[lod-1].m_uiLodDistance * m_Descriptor.m_fLodCrossFadeTransition);
  }

  for (aeUInt32 b = 0; b < m_Branches.size (); ++b)
    RenderBranchMesh (lod, b);
}

void aeTree::RenderTree (aeLod::Enum lod)
{
  if (m_Descriptor.m_LodData[lod].m_Mode == aeLodMode::Disabled)
    return;

  if (m_Descriptor.m_LodData[lod].m_Mode == aeLodMode::Full)
  {
    RenderTreeMesh (lod);
  }
  else
  {
    RenderTreeImpostor (lod);
  }
}

inline static aeUInt8 fToU8 (float f)
{
  return (aeUInt8) aeMath::Clamp ((f * 0.5f + 0.5f) * 255.0f, 0.0f, 255.0f);
}

void aeTree::CreateTreeRenderData (aeLod::Enum lod)
{
  AE_PREVENT_RECURSION;

  EnsureMeshIsGenerated (lod);

  for (aeUInt32 b = 0; b < m_Branches.size (); ++b)
  {
    if (m_Branches[b].IsDeleted ())
      continue;

    aeUInt32 n = m_Branches[b].m_Type;

    for (aeUInt32 mt = 0; mt < aeMeshType::ENUM_COUNT; ++mt)
    {
      if ((!m_Branches[b].m_LODs[lod].m_hMeshes[mt].IsValid ()) && (m_Branches[b].m_LODs[lod].m_SubMesh[mt].GetNumTriangles () > 0))
      {
        aeVertexArrayDescriptor vad;
        vad.SetupDescriptor (m_Branches[b].m_LODs[lod].m_SubMesh[mt].m_Vertices.size (), m_Branches[b].m_LODs[lod].m_SubMesh[mt].GetNumTriangles (), AE_VAU_STATIC, false);
        vad.AddVertexAttribute ("attr_TexCoords", AE_VAF_R32G32B32_FLOAT);
        vad.AddVertexAttribute ("attr_Normal", AE_VAF_R8G8B8A8_UNORM);
        vad.AddVertexAttribute ("attr_Tangent", AE_VAF_R8G8B8A8_UNORM);
        vad.AddVertexAttribute ("attr_BiTangent", AE_VAF_R8G8B8A8_UNORM);
        vad.AddVertexAttribute ("attr_PickingID", AE_VAF_R8G8B8A8_UNORM);
        vad.AddVertexAttribute ("attr_PickingSubID", AE_VAF_R8G8B8A8_UNORM);
        vad.BeginSettingAttributes ();

        m_Branches[b].m_LODs[lod].m_MeshBBox[mt].SetInvalid ();

        for (aeUInt32 v = 0; v < m_Branches[b].m_LODs[lod].m_SubMesh[mt].m_Vertices.size (); ++v)
        {
          const aeMeshVertex& vtx = m_Branches[b].m_LODs[lod].m_SubMesh[mt].m_Vertices[v];

          if (vtx.m_vNormal.IsZeroVector (0.001f)) // billboard leaf
          {
            aeUInt8 uiAO[6];
            uiAO[0] = (aeUInt8) (aeAOGrid::GetAmbientOcclusionAt (vtx.m_vPosition + aeVec3::GetAxisX() * 0.2f, aeVec3::GetAxisX()) * 255.0f);
            uiAO[1] = (aeUInt8) (aeAOGrid::GetAmbientOcclusionAt (vtx.m_vPosition - aeVec3::GetAxisX() * 0.2f,-aeVec3::GetAxisX()) * 255.0f);
            uiAO[2] = (aeUInt8) (aeAOGrid::GetAmbientOcclusionAt (vtx.m_vPosition + aeVec3::GetAxisY() * 0.2f, aeVec3::GetAxisY()) * 255.0f);
            uiAO[3] = (aeUInt8) (aeAOGrid::GetAmbientOcclusionAt (vtx.m_vPosition - aeVec3::GetAxisY() * 0.2f,-aeVec3::GetAxisY()) * 255.0f);
            uiAO[4] = (aeUInt8) (aeAOGrid::GetAmbientOcclusionAt (vtx.m_vPosition + aeVec3::GetAxisZ() * 0.2f, aeVec3::GetAxisZ()) * 255.0f);
            uiAO[5] = (aeUInt8) (aeAOGrid::GetAmbientOcclusionAt (vtx.m_vPosition - aeVec3::GetAxisZ() * 0.2f,-aeVec3::GetAxisZ()) * 255.0f);

            vad.setVertexAttribute4ub (2, v, uiAO[0], uiAO[1], uiAO[2], uiAO[3]); // normal (encodes the first four ambient occlusion values)
            vad.setVertexAttribute4ub (3, v, uiAO[4], uiAO[5], (aeUInt8) (vtx.m_vTangent.x * 255.0f), (aeUInt8) (vtx.m_vTangent.y * 255.0f)); // tangent (encodes the billboard span factor, and the last two ambient occlusion factors)
            vad.setVertexAttribute4ub (4, v, 0, 0, 0, vtx.m_uiColorVariation); // bitangent (not used in shader)
          }
          else
          {
            const aeUInt8 uiAO = (aeUInt8) (aeAOGrid::GetAmbientOcclusionAt (vtx.m_vPosition, vtx.m_vNormal) * 255.0f);
            const aeUInt8 uiAO2= (aeUInt8) (aeAOGrid::GetAmbientOcclusionAt (vtx.m_vPosition,-vtx.m_vNormal) * 255.0f);
            vad.setVertexAttribute4ub (2, v, fToU8 (vtx.m_vNormal.x), fToU8 (vtx.m_vNormal.y), fToU8 (vtx.m_vNormal.z), uiAO);
            vad.setVertexAttribute4ub (3, v, fToU8 (vtx.m_vTangent.x),   fToU8 (vtx.m_vTangent.y), fToU8 (vtx.m_vTangent.z), uiAO2);
            vad.setVertexAttribute4ub (4, v, fToU8 (vtx.m_vBiTangent.x), fToU8 (vtx.m_vBiTangent.y), fToU8 (vtx.m_vBiTangent.z), vtx.m_uiColorVariation);
          }

          m_Branches[b].m_LODs[lod].m_MeshBBox[mt].ExpandToInclude (vtx.m_vPosition);

          vad.setVertexAttribute3f  (0, v, vtx.m_vPosition.x, vtx.m_vPosition.y, vtx.m_vPosition.z);
          vad.setVertexAttribute3f  (1, v, vtx.m_vTexCoord.x, vtx.m_vTexCoord.y, vtx.m_vTexCoord.z);
        }

        for (aeUInt32 q = 0; q < m_Branches[b].m_LODs[lod].m_SubMesh[mt].GetNumTriangles (); ++q)
        {
          const aeMeshTriangle& t = m_Branches[b].m_LODs[lod].m_SubMesh[mt].m_Triangles[q];

          for (int i = 0; i < 3; ++i)
          {
            const aeMeshVertex& vtx = m_Branches[b].m_LODs[lod].m_SubMesh[mt].m_Vertices[t.m_uiVertexIDs[i]];
            
            vad.setVertexAttribute4ub (5, t.m_uiVertexIDs[i], (t.m_uiPickingID >> 0) & 0xFF, (t.m_uiPickingID >> 8) & 0xFF, (t.m_uiPickingID >> 16) & 0xFF, (t.m_uiPickingID >> 24) & 0xFF);
            vad.setVertexAttribute4ub (6, t.m_uiVertexIDs[i], (t.m_uiPickingSubID >> 0) & 0xFF, (t.m_uiPickingSubID >> 8) & 0xFF, (t.m_uiPickingSubID >> 16) & 0xFF, (t.m_uiPickingSubID >> 24) & 0xFF);
          }

          vad.setTriangle (q, t.m_uiVertexIDs[0], t.m_uiVertexIDs[1], t.m_uiVertexIDs[2]);
        }

        aeVertexArrayResource::CreateResource (m_Branches[b].m_LODs[lod].m_hMeshes[mt], &vad);
      }
    }
  }
}

void aeTree::ClearBranchRenderData (aeUInt32 uiBranch)
{
  for (aeUInt32 i = 0; i < aeLod::ENUM_COUNT; ++i)
  {
    m_Branches[uiBranch].m_LODs[i].m_bMeshGeneratedBranchCaps = false;
    m_Branches[uiBranch].m_LODs[i].m_bMeshGeneratedTrunkCaps = false;

    for (aeUInt32 mt = 0; mt < aeMeshType::ENUM_COUNT; ++mt)
    {
      m_Branches[uiBranch].m_LODs[i].m_bMeshGenerated[mt] = false;
      m_Branches[uiBranch].m_LODs[i].m_hMeshes[mt].Invalidate ();
      m_Branches[uiBranch].m_LODs[i].m_MeshBBox[mt].SetInvalid ();
    }
  }
}

void aeTree::ClearTreeRenderData (void)
{
  for (aeUInt32 b = 0; b < m_Branches.size (); ++b)
    ClearBranchRenderData (b);

  for (aeUInt32 lod = 0; lod < aeLod::ENUM_COUNT; ++lod)
  {
    m_LodImpostors[lod].Clear ();
  }
}


