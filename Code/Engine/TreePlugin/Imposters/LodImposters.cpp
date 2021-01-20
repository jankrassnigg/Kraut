#include "PCH.h"

#include "../Basics/Plugin.h"
#include "../GUI/ProgressBar.h"
#include "../Tree/Tree.h"
#include <KrautGenerator/Mesh/Mesh.h>
#include <KrautGraphics/Framebuffer/Main.h>
#include <KrautGraphics/RenderAPI/Main.h>
#include <KrautGraphics/ShaderManager/Main.h>
#include <KrautGraphics/TextureManager/Main.h>


using namespace AE_NS_GRAPHICS;

bool ConvertTGAtoDDS(const char* szFile, bool bNormalMap);
void GetSubImage(const aeArray<aeUInt8>& Image, aeUInt32 uiWidth, aeUInt32 uiHeight, aeArray<aeUInt8>& out_SubImage, aeUInt32 uiSubWidth, aeUInt32 uiSubHeight);
void ClearAlpha(aeArray<aeUInt8>& Image, aeUInt8 uiAlphaThreshold);
void DilateColors(aeArray<aeUInt8>& Image, aeUInt32 uiWidth, aeUInt32 uiHeight, aeUInt8 uiPass);
void FillAvgImageColor(aeArray<aeUInt8>& Image);
void EnsureBlackAlphaBorder(aeArray<aeUInt8>& Image, aeUInt32 uiWidth, aeUInt32 uiHeight, aeUInt32 uiStartX, aeUInt32 uiRectWidth);

aeFramebuffer g_ImpostorFB;
aeFramebuffer g_ImpostorClearColorFB;
aeFramebuffer g_ImpostorClearNormalFB;
aeTextureResourceHandle g_ImpostorDepthRT;

const aeUInt32 g_uiImpostorRenderTargetSize = 1024; // must be the maximum possible resolution atm, otherwise export cannot create larger textures

AE_ON_GLOBAL_EVENT_ONCE(aeStartup_ShutdownEngine_Begin)
{
  g_ImpostorFB.Shutdown();
  g_ImpostorClearColorFB.Shutdown();
  g_ImpostorClearNormalFB.Shutdown();
  g_ImpostorDepthRT.Invalidate();
}

void EnsureColorRendering(void)
{
  aeRenderAPI::setWireframeMode(false);
  aeShaderManager::setShaderBuilderVariable("OUTPUT_NORMALS_ONLY", false);
  aeShaderManager::setShaderBuilderVariable("OUTPUT_DIFFUSE_ONLY", false);
  aeShaderManager::setShaderBuilderVariable("OUTPUT_LIGHT_ONLY", false);
  aeShaderManager::setShaderBuilderVariable("OUTPUT_AMBIENT_ONLY", false);
  aeShaderManager::setShaderBuilderVariable("OUTPUT_SKYLIGHT_ONLY", false);
  aeShaderManager::setShaderBuilderVariable("WIREFRAME", false);
  aeShaderManager::setShaderBuilderVariable("CROSSFADE", false);
}

static void SetupImpostorFBs(aeTextureResourceHandle& pDiffuse, aeTextureResourceHandle& pNormal)
{
  AE_EXECUTE_ONLY_ONCE;

  aeTextureResourceDescriptor trd;
  trd.m_bGenerateMipmaps = false;
  trd.m_TextureType = AE_TYPE_TEXTURE_2D;
  trd.m_uiWidth = g_uiImpostorRenderTargetSize * 4;
  trd.m_uiHeight = g_uiImpostorRenderTargetSize;
  trd.m_bAllowToReadContents = false;
  trd.m_TextureFormat = AE_FORMAT_DEPTH24_STENCIL8;

  aeTextureResource::CreateResource(g_ImpostorDepthRT, &trd);


  aeArray<aeUInt32> Data(trd.m_uiWidth * trd.m_uiHeight);

  for (aeUInt32 d = 0; d < Data.size(); ++d)
    Data[d] = 0xFF00FFFF;

  trd.m_bGenerateMipmaps = true;
  trd.m_TextureFormat = AE_FORMAT_R8G8B8A8_UNORM;
  trd.m_bAllowToReadContents = true;
  trd.m_pInitialData = Data.data();

  {
    aeSamplerState ss;
    ss.m_TextureWrapU = AE_WRAP_CLAMP;
    ss.m_TextureWrapV = AE_WRAP_CLAMP;
    aeTextureManager::setTextureSampler("Clamped", ss);
    trd.m_iDefaultTextureSamplerID = aeTextureManager::getTextureSamplerID("Clamped");

    aeTextureResource::CreateResource(pDiffuse, &trd);
    aeTextureResource::CreateResource(pNormal, &trd);

    g_ImpostorFB.setDepthAttachment(g_ImpostorDepthRT);
    g_ImpostorFB.setColorAttachment(0, pDiffuse);
    g_ImpostorFB.setColorAttachment(1, pNormal);

    g_ImpostorClearColorFB.setColorAttachment(0, pDiffuse);
    g_ImpostorClearNormalFB.setColorAttachment(0, pNormal);
  }

  // due to strange ATI driver behavior, generating the mipmaps first, and later again, ensures correct mipmaps
  pDiffuse.GetResource()->GenerateMipmaps();
  pNormal.GetResource()->GenerateMipmaps();
}

static void PrepareRenderTargets(aeTextureResourceHandle& pDiffuse, aeTextureResourceHandle& pNormal)
{
  SetupImpostorFBs(pDiffuse, pNormal);

  g_ImpostorClearColorFB.BindFramebuffer();
  g_ImpostorClearColorFB.ClearCurrentBuffers(true, false, 120.0f / 255.0f, 150.0f / 255.0f, 60.0f / 255.0f, 0);

  g_ImpostorClearNormalFB.BindFramebuffer();
  g_ImpostorClearNormalFB.ClearCurrentBuffers(true, false, 0.5f, 0.5f, 1.0f, 0.0f);

  g_ImpostorFB.BindFramebuffer();
  aeRenderAPI::Clear(false, true);
}

AE_ON_GLOBAL_EVENT(aeStartup_StartupEngine_End)
{
  PrepareRenderTargets(g_Tree.m_hLodTextures, g_Tree.m_hLodTexturesN);
}

aeLod::Enum GetNonImpostorLod(aeTreeDescriptor& desc, aeLod::Enum lod)
{
  while ((desc.m_LodData[lod].m_Mode == Kraut::LodMode::Disabled) || Kraut::LodMode::IsImpostorMode(desc.m_LodData[lod].m_Mode))
    lod = (aeLod::Enum)((aeInt32)(lod)-1);

  return lod;
}

static bool SaveImpostorTexture(const char* szFile, bool bDDS, aeTextureResourceHandle& hTexture, bool bNormalMap, aeUInt32 uiImpostorResolution)
{
  aeHybridString<256> sText;
  sText.Format("Exporting LOD Impostor (%s).", bNormalMap ? "normalmap" : "color");
  aeProgressBar pb(sText.c_str(), bDDS ? 4 : 3);

  aeArray<aeUInt8> Image0(g_uiImpostorRenderTargetSize * g_uiImpostorRenderTargetSize * 4 * 4);

  hTexture.GetResource()->ReadbackTexture(Image0.data());

  aeArray<aeUInt8> Image;
  GetSubImage(Image0, g_uiImpostorRenderTargetSize * 4, g_uiImpostorRenderTargetSize, Image, uiImpostorResolution * 4, uiImpostorResolution);

  if (!bNormalMap)
    FillAvgImageColor(Image);

  aeProgressBar::Update(nullptr); // 1

  for (aeInt32 d = g_Tree.m_Descriptor.m_uiLeafCardTexelDilation * 2; d > 0; --d)
    DilateColors(Image, uiImpostorResolution * 4, uiImpostorResolution, d);

  ClearAlpha(Image, g_Tree.m_Descriptor.m_uiLeafCardTexelDilation * 2);

  EnsureBlackAlphaBorder(Image, uiImpostorResolution * 4, uiImpostorResolution, 0 * uiImpostorResolution, uiImpostorResolution);
  EnsureBlackAlphaBorder(Image, uiImpostorResolution * 4, uiImpostorResolution, 1 * uiImpostorResolution, uiImpostorResolution);
  EnsureBlackAlphaBorder(Image, uiImpostorResolution * 4, uiImpostorResolution, 2 * uiImpostorResolution, uiImpostorResolution);
  EnsureBlackAlphaBorder(Image, uiImpostorResolution * 4, uiImpostorResolution, 3 * uiImpostorResolution, uiImpostorResolution);


  sText.Format("Writing TGA:\n'%s'", szFile);
  aeProgressBar::Update(sText.c_str()); // 2

  if (!ExportTGA(szFile, uiImpostorResolution * 4, uiImpostorResolution, 4, Image.data(), true))
    return false;

  if (aeProgressBar::WasProgressBarCanceled())
    return true;

  if (bDDS)
  {
    sText.Format("Converting to DDS:\n'%s'", szFile);
    aeProgressBar::Update(sText.c_str()); // 3

    if (!ConvertTGAtoDDS(szFile, false))
      return false;
  }

  aeProgressBar::Update(nullptr); // 4

  return true;
}

bool aeTree::SaveLodImpostors(const char* szFile, bool bDDS, aeUInt32 uiImpostorResolution)
{
  aeProgressBar pb("Saving Impostor Textures", 3);

  CreateLodImposters(uiImpostorResolution);

  aeString sPath = aePathFunctions::GetFileDirectory(szFile);
  aeString sName = aePathFunctions::GetFileName(szFile);
  aeString sDiffuse = aePathFunctions::CombinePaths(sPath, sName.c_str()) + ".tga";
  aeString sNormal = aePathFunctions::CombinePaths(sPath, sName.c_str()) + "_N.tga";

  aeProgressBar::Update(nullptr); // 1

  SaveImpostorTexture(sDiffuse.c_str(), bDDS, m_hLodTextures, false, uiImpostorResolution);
  aeProgressBar::Update(nullptr); // 2

  SaveImpostorTexture(sNormal.c_str(), bDDS, m_hLodTexturesN, true, uiImpostorResolution);
  aeProgressBar::Update(nullptr); // 3

  return true;
}

void aeTree::UpdateImpostorSize(void)
{
  Kraut::BoundingBox bbox = m_BBox;
  bbox.ExpandToInclude(aeVec3::ZeroVector());
  bbox.ExpandToInclude(aeVec3(-m_BBox.m_vMax.x, 0, -m_BBox.m_vMax.z));
  bbox.ExpandToInclude(aeVec3(-m_BBox.m_vMin.x, 0, -m_BBox.m_vMin.z));

  m_fImposterWidth = aeMath::Max(bbox.GetSize().x, bbox.GetSize().z) * 0.5f;
  m_fImposterHeight = bbox.GetSize().y;
}

bool aeTree::CreateLodImposters(aeUInt32 uiImpostorResolution)
{
  if (uiImpostorResolution == 0)
    uiImpostorResolution = g_uiImpostorRenderTargetSize;

  const bool bIsDefaultRenderResolution = (uiImpostorResolution == g_uiImpostorRenderTargetSize);

  if (bIsDefaultRenderResolution && m_bLodImpostorsAreUpToDate)
    return false;

  m_bLodImpostorsAreUpToDate = true;

  aeRenderAPI::PushFramebufferStates(false);
  aeRenderAPI::PushRenderStates(false);
  aeRenderAPI::PushStencilStates(false);

  CreateTreeRenderData(GetNonImpostorLod(m_Descriptor, aeLod::Lod4));

  PrepareRenderTargets(m_hLodTextures, m_hLodTexturesN);

  EnsureColorRendering();

  aeShaderManager::setShaderBuilderVariable("EXPORT_LEAFCARD", true);

  UpdateImpostorSize();

  const aeUInt32 uiSnapshots = 4;
  const float fSnapshotRotation = 180.0f / uiSnapshots;
  const aeVec3 vCenter = aeVec3(0, m_fImposterHeight / 2.0f, 0);
  float fHalfWidth = m_fImposterWidth;
  float fHalfHeight = m_fImposterHeight / 2.0f;

  for (aeUInt32 snapshot = 0; snapshot < uiSnapshots; ++snapshot)
  {
    aeMatrix mTBN;
    mTBN.SetIdentity();

    const float fTreeRotation = snapshot * fSnapshotRotation;
    const float fCameraRotation = fTreeRotation - 90;
    const aeVec3 vCameraDir = aeVec3(aeMath::CosDeg(fCameraRotation), 0, -aeMath::SinDeg(fCameraRotation)) * 100.0f;

    mTBN.SetColumn(0, aeMath::CosDeg(fTreeRotation), 0, -aeMath::SinDeg(fTreeRotation), 0);
    mTBN.SetColumn(1, 0, -1, 0, 0);
    mTBN.SetColumn(2, aeMath::CosDeg(fCameraRotation), 0, -aeMath::SinDeg(fCameraRotation), 0);

    float fInvTBN[9];
    mTBN.GetInverse().GetAsOpenGL3x3Matrix(fInvTBN);
    aeShaderManager::setUniformMatrix3x3("unif_InverseTBN", false, fInvTBN);

    g_Globals.m_OrthoCamera.setViewport(snapshot * uiImpostorResolution, 0, uiImpostorResolution, uiImpostorResolution);
    g_Globals.m_OrthoCamera.setLookAt(vCenter + vCameraDir, vCenter);
    g_Globals.m_OrthoCamera.setOrthoCamera(-fHalfWidth, fHalfWidth, -fHalfHeight, fHalfHeight, 0, 1000);

    g_Globals.m_OrthoCamera.ApplyCamera();

    RenderTreeMesh(GetNonImpostorLod(m_Descriptor, aeLod::Lod4));
  }

  aeShaderManager::setShaderBuilderVariable("EXPORT_LEAFCARD", false);

  aeRenderAPI::PopFramebufferStates();
  aeRenderAPI::PopRenderStates();
  aeRenderAPI::PopStencilStates();

  m_hLodTextures.GetResource()->GenerateMipmaps();
  m_hLodTexturesN.GetResource()->GenerateMipmaps();

  // ensure that in the next update the default resolution will be restored
  if (!bIsDefaultRenderResolution)
    m_bLodImpostorsAreUpToDate = false;

  return true;
}

inline static aeUInt8 fToU8(float f)
{
  return (aeUInt8)aeMath::Clamp((f * 0.5f + 0.5f) * 255.0f, 0.0f, 255.0f);
}

static void AddQuad(Kraut::Mesh& SubMesh, GLIM_BATCH& gb, const aeVec3 vPos, float fHalfWidth, float fHeight, float fRotation, float fTexCoordU, float fTexWidth)
{
  const aeVec3 vDir = aeVec3(aeMath::CosDeg(fRotation), 0, -aeMath::SinDeg(fRotation)) * fHalfWidth;
  const aeVec3 vUp(0, fHeight, 0);
  aeVec3 v;

  aeUInt32 uiVertices[4];

  Kraut::Vertex vtx;
  vtx.m_vNormal.SetVector(aeMath::CosDeg(fRotation - 90), 0, -aeMath::SinDeg(fRotation - 90));
  vtx.m_vTangent.SetVector(aeMath::CosDeg(fRotation), 0, -aeMath::SinDeg(fRotation));
  vtx.m_vTangent = -vtx.m_vTangent;
  vtx.m_vBiTangent.SetVector(0, 1, 0);

  gb.Attribute4ub("attr_Normal", fToU8(aeMath::CosDeg(fRotation - 90)), fToU8(0), fToU8(-aeMath::SinDeg(fRotation - 90)), 0);
  gb.Attribute4ub("attr_Tangent", fToU8(aeMath::CosDeg(fRotation)), fToU8(0), fToU8(-aeMath::SinDeg(fRotation)));
  gb.Attribute4ub("attr_BiTangent", fToU8(0), fToU8(1), fToU8(0));

  vtx.m_vTexCoord.SetVector(fTexCoordU, 0, 1);
  gb.Attribute3f("attr_TexCoords", fTexCoordU, 0, 1);
  v = vPos - vDir;
  vtx.m_vPosition = v;
  uiVertices[0] = SubMesh.AddVertex(vtx);
  gb.Vertex(v.x, v.y, v.z);

  vtx.m_vTexCoord.SetVector(fTexCoordU + fTexWidth, 0, 1);
  gb.Attribute3f("attr_TexCoords", fTexCoordU + fTexWidth, 0, 1);
  v = vPos + vDir;
  vtx.m_vPosition = v;
  uiVertices[1] = SubMesh.AddVertex(vtx);
  gb.Vertex(v.x, v.y, v.z);

  vtx.m_vTexCoord.SetVector(fTexCoordU + fTexWidth, 1, 1);
  gb.Attribute3f("attr_TexCoords", fTexCoordU + fTexWidth, 1, 1);
  v = vPos + vDir + vUp;
  vtx.m_vPosition = v;
  uiVertices[2] = SubMesh.AddVertex(vtx);
  gb.Vertex(v.x, v.y, v.z);

  vtx.m_vTexCoord.SetVector(fTexCoordU, 1, 1);
  gb.Attribute3f("attr_TexCoords", fTexCoordU, 1, 1);
  v = vPos - vDir + vUp;
  vtx.m_vPosition = v;
  uiVertices[3] = SubMesh.AddVertex(vtx);
  gb.Vertex(v.x, v.y, v.z);

  Kraut::Triangle t;
  t.m_uiVertexIDs[0] = uiVertices[0];
  t.m_uiVertexIDs[1] = uiVertices[1];
  t.m_uiVertexIDs[2] = uiVertices[2];
  SubMesh.m_Triangles.push_back(t);

  t.m_uiVertexIDs[0] = uiVertices[0];
  t.m_uiVertexIDs[1] = uiVertices[2];
  t.m_uiVertexIDs[2] = uiVertices[3];
  SubMesh.m_Triangles.push_back(t);
}

static void AddBillboard(Kraut::Mesh& SubMesh, GLIM_BATCH& gb, const aeVec3 vPos, float fHalfWidth, float fHeight, float fTexCoordU, float fTexWidth)
{
  Kraut::Vertex vtx;

  const aeVec3 vUp(0, fHeight, 0);
  aeVec3 v;

  aeUInt32 uiVertices[4];

  vtx.m_vTexCoord.SetVector(fTexCoordU, 0, -fHalfWidth);
  gb.Attribute3f("attr_TexCoords", fTexCoordU, 0, -fHalfWidth);
  v = vPos;
  vtx.m_vPosition = v;
  gb.Vertex(v.x, v.y, v.z);

  uiVertices[0] = SubMesh.AddVertex(vtx);

  vtx.m_vTexCoord.SetVector(fTexCoordU + fTexWidth, 0, fHalfWidth);
  gb.Attribute3f("attr_TexCoords", fTexCoordU + fTexWidth, 0, fHalfWidth);
  v = vPos;
  vtx.m_vPosition = v;
  gb.Vertex(v.x, v.y, v.z);

  uiVertices[1] = SubMesh.AddVertex(vtx);

  vtx.m_vTexCoord.SetVector(fTexCoordU + fTexWidth, 1, fHalfWidth);
  gb.Attribute3f("attr_TexCoords", fTexCoordU + fTexWidth, 1, fHalfWidth);
  v = vPos + vUp;
  vtx.m_vPosition = v;
  gb.Vertex(v.x, v.y, v.z);

  uiVertices[2] = SubMesh.AddVertex(vtx);

  vtx.m_vTexCoord.SetVector(fTexCoordU, 1, -fHalfWidth);
  gb.Attribute3f("attr_TexCoords", fTexCoordU, 1, -fHalfWidth);
  v = vPos + vUp;
  vtx.m_vPosition = v;
  gb.Vertex(v.x, v.y, v.z);

  uiVertices[3] = SubMesh.AddVertex(vtx);

  Kraut::Triangle t;
  t.m_uiVertexIDs[0] = uiVertices[0];
  t.m_uiVertexIDs[1] = uiVertices[1];
  t.m_uiVertexIDs[2] = uiVertices[2];
  SubMesh.m_Triangles.push_back(t);

  t.m_uiVertexIDs[0] = uiVertices[0];
  t.m_uiVertexIDs[1] = uiVertices[2];
  t.m_uiVertexIDs[2] = uiVertices[3];
  SubMesh.m_Triangles.push_back(t);
}

void aeTree::CreateTreeImpostorMesh(aeLod::Enum lod)
{
  if (!m_LodImpostors[lod].IsCleared())
    return;

  UpdateImpostorSize();

  m_LodImpostorMesh[lod].Clear();

  m_LodImpostors[lod].BeginBatch();

  m_LodImpostors[lod].Attribute3f("attr_TexCoords", 0, 0, 1);
  m_LodImpostors[lod].Attribute4ub("attr_Normal", fToU8(0), fToU8(0), fToU8(1), 0);
  m_LodImpostors[lod].Attribute4ub("attr_Tangent", fToU8(1), fToU8(0), fToU8(0));
  m_LodImpostors[lod].Attribute4ub("attr_BiTangent", fToU8(0), fToU8(1), fToU8(0));
  m_LodImpostors[lod].Attribute4ub("attr_PickingID", 0, 0, 0, 0);
  m_LodImpostors[lod].Attribute4ub("attr_PickingSubID", 0, 0, 0, 0);

  m_LodImpostors[lod].Begin(GLIM_QUADS);

  const float fTexWidth = 1.0f / 4;

  switch (m_Descriptor.m_LodData[lod].m_Mode)
  {
    case Kraut::LodMode::FourQuads:
    {
      AddQuad(m_LodImpostorMesh[lod], m_LodImpostors[lod], aeVec3::ZeroVector(), m_fImposterWidth, m_fImposterHeight, 0, 0 * fTexWidth, fTexWidth);
      AddQuad(m_LodImpostorMesh[lod], m_LodImpostors[lod], aeVec3::ZeroVector(), m_fImposterWidth, m_fImposterHeight, 45, 1 * fTexWidth, fTexWidth);
      AddQuad(m_LodImpostorMesh[lod], m_LodImpostors[lod], aeVec3::ZeroVector(), m_fImposterWidth, m_fImposterHeight, 90, 2 * fTexWidth, fTexWidth);
      AddQuad(m_LodImpostorMesh[lod], m_LodImpostors[lod], aeVec3::ZeroVector(), m_fImposterWidth, m_fImposterHeight, 135, 3 * fTexWidth, fTexWidth);
    }
    break;
    case Kraut::LodMode::TwoQuads:
    {
      AddQuad(m_LodImpostorMesh[lod], m_LodImpostors[lod], aeVec3::ZeroVector(), m_fImposterWidth, m_fImposterHeight, 0, 0 * fTexWidth, fTexWidth);
      AddQuad(m_LodImpostorMesh[lod], m_LodImpostors[lod], aeVec3::ZeroVector(), m_fImposterWidth, m_fImposterHeight, 90, 2 * fTexWidth, fTexWidth);
    }
    break;
    case Kraut::LodMode::Billboard:
    {
      AddBillboard(m_LodImpostorMesh[lod], m_LodImpostors[lod], aeVec3::ZeroVector(), m_fImposterWidth, m_fImposterHeight, 0, fTexWidth);
    }
    break;
  }

  m_LodImpostors[lod].End();
  m_LodImpostors[lod].EndBatch(false);
}

void aeTree::RenderTreeImpostor(aeLod::Enum lod)
{
  if (CreateLodImposters())
  {
    aeFramebuffer::BindBackbuffer();
    AE_BROADCAST_EVENT(aeEditor_QueueRedraw);
    return;
  }

  CreateTreeImpostorMesh(lod);

  aeShaderManager::BindTexture("tex_Diffuse", m_hLodTextures, aeBranchGeometryType::ImpostorTextureSampler);
  aeShaderManager::BindTexture("tex_Normal", m_hLodTexturesN, aeBranchGeometryType::ImpostorTextureSampler);
  aeRenderAPI::setFaceCulling(AE_CULL_FACE_NONE);

  if (m_Descriptor.m_LodData[lod].m_Mode == Kraut::LodMode::Billboard)
    aeShaderManager::setShader(g_Globals.s_hBillboardShader);
  else
    aeShaderManager::setShader(g_Globals.s_hImpostorShader);

  const aeVec3 vCamPos = aeGLCamera::getCurrent()->getPosition();
  const aeVec3 vForwards = aeGLCamera::getCurrent()->getVectorForwards().GetNormalized();
  const aeVec3 vRight = aeGLCamera::getCurrent()->getVectorRight().GetNormalized();
  const aeVec3 vUp = aeGLCamera::getCurrent()->getVectorUp().GetNormalized();

  aeShaderManager::setShaderBuilderVariable("EXPORT_LEAFCARD", false);
  aeShaderManager::setUniformFloat("unif_CameraPosition", 3, vCamPos.x, vCamPos.y, vCamPos.z);
  aeShaderManager::setUniformFloat("unif_CameraForwards", 3, vForwards.x, vForwards.y, vForwards.z);
  aeShaderManager::setUniformFloat("unif_CameraRight", 3, vRight.x, vRight.y, vRight.z);
  aeShaderManager::setUniformFloat("unif_CameraUp", 3, vUp.x, vUp.y, vUp.z);

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

  m_LodImpostors[lod].RenderBatch();
}
