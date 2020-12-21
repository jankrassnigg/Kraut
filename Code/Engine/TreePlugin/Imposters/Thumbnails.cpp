#include "PCH.h"

#include "../Tree/Tree.h"
#include "../Basics/Plugin.h"
#include <KrautGraphics/RenderAPI/Main.h>
#include <KrautGraphics/TextureManager/Main.h>
#include <KrautGraphics/ShaderManager/Main.h>
#include <KrautGraphics/Framebuffer/Main.h>


using namespace AE_NS_GRAPHICS;

extern aeUInt32 g_uiThumbnailSize;

aeFramebuffer g_ThumbnailFB;
aeTextureResourceHandle g_hThumbnailDiffuseRT;
aeTextureResourceHandle g_hThumbnailDepthRT;

AE_ON_GLOBAL_EVENT_ONCE (aeStartup_ShutdownEngine_Begin)
{
  g_hThumbnailDiffuseRT.Invalidate ();
  g_hThumbnailDepthRT.Invalidate ();
  g_ThumbnailFB.Shutdown ();
}

static void SetupThumbnailFBs ()
{
  AE_EXECUTE_ONLY_ONCE;

  aeTextureResourceDescriptor trd;
  trd.m_bAllowToReadContents = true;
  trd.m_bGenerateMipmaps = false;
  trd.m_TextureFormat = AE_FORMAT_R8G8B8A8_UNORM;
  trd.m_TextureType = AE_TYPE_TEXTURE_2D;
  trd.m_uiWidth = g_uiThumbnailSize;
  trd.m_uiHeight = g_uiThumbnailSize;

  aeTextureResource::CreateResource (g_hThumbnailDiffuseRT, &trd);

  trd.m_bAllowToReadContents = false;
  trd.m_TextureFormat = AE_FORMAT_DEPTH24_STENCIL8;

  aeTextureResource::CreateResource (g_hThumbnailDepthRT, &trd);

  g_ThumbnailFB.setDepthAttachment (g_hThumbnailDepthRT);
  g_ThumbnailFB.setColorAttachment (0, g_hThumbnailDiffuseRT);
}

static aeBBox GetBranchBBox (aeInt32 iBranch)
{
  aeUInt32 uiType = g_Tree.m_Branches[iBranch].m_Type;
  const aeSpawnNodeDesc& bnd = g_Tree.m_Descriptor.m_BranchTypes[uiType];

  const aeBBox bb1 = g_Tree.m_Branches[iBranch].m_LODs[aeLod::Lod0].m_MeshBBox[aeMeshType::Branch];
  const aeBBox bb2 = g_Tree.m_Branches[iBranch].m_LODs[aeLod::Lod0].m_MeshBBox[aeMeshType::Frond];
  const aeBBox bb3 = g_Tree.m_Branches[iBranch].m_LODs[aeLod::Lod0].m_MeshBBox[aeMeshType::Leaf];

  aeBBox bf;
  bf.SetInvalid ();
  if (!bb1.IsInvalid ())
  {
    bf.ExpandToInclude (bb1.m_vMin);
    bf.ExpandToInclude (bb1.m_vMax);
  }

  if (!bb2.IsInvalid ())
  {
    bf.ExpandToInclude (bb2.m_vMin);
    bf.ExpandToInclude (bb2.m_vMax);
  }

  if (!bb3.IsInvalid ())
  {
    const aeVec3 vSize (bnd.m_fLeafSize);

    bf.ExpandToInclude (bb3.m_vMax + vSize);
    bf.ExpandToInclude (bb3.m_vMin - vSize);
  }

  return bf;
}

static aeInt32 FindLeastProjectionAxis (aeInt32 iBranch, const aeBBox& bf)
{
  const aeVec3 vAxis = bf.m_vMax - bf.m_vMin;

  aeInt32 iLeastAxis = 0;
  if ((vAxis.x <= vAxis.y) && (vAxis.x <= vAxis.z))
    iLeastAxis = 0;
  if ((vAxis.y <= vAxis.x) && (vAxis.y <= vAxis.z))
    iLeastAxis = 1;
  if ((vAxis.z <= vAxis.x) && (vAxis.z <= vAxis.y))
    iLeastAxis = 2;

  return iLeastAxis;
}

static aeInt32 FindBranchOfType (aeUInt32 uiBranchType)
{
  aeInt32 iBranch = -1;
  aeUInt32 uiBranchScore = 0;

  for (aeUInt32 b = 0; b < g_Tree.m_Branches.size (); ++b)
  {
    if (g_Tree.m_Branches[b].IsDeleted ())
      continue;
    if (g_Tree.m_Branches[b].m_Type != uiBranchType)
      continue;

    aeUInt32 uiScore = 0;
    for (aeUInt32 mt = 0; mt < aeMeshType::ENUM_COUNT; ++mt)
      uiScore += g_Tree.m_Branches[b].m_LODs[aeLod::Lod0].m_SubMesh[mt].GetNumTriangles ();

    if (uiScore > uiBranchScore)
    {
      iBranch = b;
      uiBranchScore = uiScore;
    }
  }

  return iBranch;
}

bool aeTreePlugin::RenderBranchOfType (aeUInt32 uiRenderSize, aeUInt32 uiBranchType)
{
  const aeInt32 iBranch = FindBranchOfType (uiBranchType);

  if (iBranch < 0)
    return false;

  aeShaderManager::setShaderBuilderVariable ("EXPORT_LEAFCARD", false);

  g_Globals.s_vSunLightDir.Normalize ();
  aeShaderManager::setUniformFloat ("unif_LightPos", 3, g_Globals.s_vPointLightPos.x, g_Globals.s_vPointLightPos.y, g_Globals.s_vPointLightPos.z);
  aeShaderManager::setUniformFloat ("unif_SunDir", 3, g_Globals.s_vSunLightDir.x, g_Globals.s_vSunLightDir.y, g_Globals.s_vSunLightDir.z);

  const aeBBox bf = GetBranchBBox (iBranch);

  if (bf.IsInvalid ())
    return false;

  const aeInt32 iLeastAxis = FindLeastProjectionAxis (iBranch, bf);
  const aeVec3 vCenter = bf.GetCenter ();

  if (iLeastAxis == 2) 
  {
    const float fSizeX = bf.m_vMax.x - bf.m_vMin.x;
    const float fSizeY = bf.m_vMax.y - bf.m_vMin.y;
    const float fSize = aeMath::Max (fSizeX, fSizeY) * 0.5f;
    g_Globals.m_OrthoCamera.setOrthoCamera (-fSize, fSize, -fSize, fSize, -1000, 1000);
    g_Globals.m_OrthoCamera.setLookAt (vCenter + aeVec3 (0, 0, 100), vCenter);
  }
  if (iLeastAxis == 1)
  {
    const float fSizeX = bf.m_vMax.x - bf.m_vMin.x;
    const float fSizeY = bf.m_vMax.z - bf.m_vMin.z;
    const float fSize = aeMath::Max (fSizeX, fSizeY) * 0.5f;
    g_Globals.m_OrthoCamera.setOrthoCamera (-fSize, fSize, -fSize, fSize, -1000, 1000);
    g_Globals.m_OrthoCamera.setLookAt (vCenter + aeVec3 (0, 100, 1), vCenter);
  }
  if (iLeastAxis == 0)
  {
    const float fSizeX = bf.m_vMax.z - bf.m_vMin.z;
    const float fSizeY = bf.m_vMax.y - bf.m_vMin.y;
    const float fSize = aeMath::Max (fSizeX, fSizeY) * 0.5f;
    g_Globals.m_OrthoCamera.setOrthoCamera (-fSize, fSize, -fSize, fSize, -1000, 1000);
    g_Globals.m_OrthoCamera.setLookAt (vCenter + aeVec3 (100, 0, 0), vCenter);
  }

  g_Globals.m_OrthoCamera.setViewport (0, 0, uiRenderSize, uiRenderSize);
  g_Globals.m_OrthoCamera.ApplyCamera ();

  g_Tree.RenderSingleBranch (iBranch);

  return true;
}

static void PrepareRenderTargets (void)
{
  SetupThumbnailFBs ();

  g_ThumbnailFB.BindFramebuffer ();
  aeRenderAPI::Clear (true, true);
}

static void RGBAtoBGRA (aeArray<aeUInt8>& inout_Image)
{
  for (aeUInt32 i = 0; i < inout_Image.size (); i += 4)
  {
    aeUInt8 r,g,b,a;
    r = inout_Image[i + 0];
    g = inout_Image[i + 1];
    b = inout_Image[i + 2];
    a = inout_Image[i + 3];
    inout_Image[i + 0] = b;
    inout_Image[i + 1] = g;
    inout_Image[i + 2] = r;
    inout_Image[i + 3] = a;
  }
}

bool aeTreePlugin::CreateBranchTypeThumbnail (aeUInt32 uiBranch, aeArray<aeUInt8>& out_Thumbnail)
{
  PrepareRenderTargets ();

  if (!RenderBranchOfType (g_uiThumbnailSize, uiBranch))
    return false;

  out_Thumbnail.resize (g_uiThumbnailSize * g_uiThumbnailSize * 4);
  g_hThumbnailDiffuseRT.GetResource ()->ReadbackTexture (out_Thumbnail.data ());

  RGBAtoBGRA (out_Thumbnail);

  return true;
}
