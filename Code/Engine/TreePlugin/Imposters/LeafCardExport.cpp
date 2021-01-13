#include "PCH.h"

#include "../Basics/Plugin.h"
#include "../GUI/ProgressBar.h"
#include "../Tree/Tree.h"
#include <KrautGraphics/Framebuffer/Main.h>
#include <KrautGraphics/RenderAPI/Main.h>
#include <KrautGraphics/ShaderManager/Main.h>
#include <KrautGraphics/TextureManager/Main.h>


using namespace AE_NS_GRAPHICS;

const aeUInt32 g_uiLeafCardRenderTargetSize = 2048;

aeFramebuffer g_LeafCardFB;
aeFramebuffer g_LeafCardClearColorFB;
aeFramebuffer g_LeafCardClearNormalFB;
aeTextureResourceHandle g_hLeafCardDiffuseRT;
aeTextureResourceHandle g_hLeafCardNormalRT;
aeTextureResourceHandle g_hLeafCardDepthRT;

bool ConvertTGAtoDDS(const char* szFile, bool bNormalMap);
void GetSubImage(const aeArray<aeUInt8>& Image, aeUInt32 uiWidth, aeUInt32 uiHeight, aeArray<aeUInt8>& out_SubImage, aeUInt32 uiSubWidth, aeUInt32 uiSubHeight);
void ClearAlpha(aeArray<aeUInt8>& Image, aeUInt8 uiAlphaThreshold);
void DilateColors(aeArray<aeUInt8>& Image, aeUInt32 uiWidth, aeUInt32 uiHeight, aeUInt8 uiPass);
void FillAvgImageColor(aeArray<aeUInt8>& Image);


AE_ON_GLOBAL_EVENT_ONCE(aeStartup_ShutdownEngine_Begin)
{
  g_LeafCardFB.Shutdown();
  g_LeafCardClearColorFB.Shutdown();
  g_LeafCardClearNormalFB.Shutdown();
  g_hLeafCardDiffuseRT.Invalidate();
  g_hLeafCardNormalRT.Invalidate();
  g_hLeafCardDepthRT.Invalidate();
}

static void SetupLeafCardFBs()
{
  AE_EXECUTE_ONLY_ONCE;

  aeTextureResourceDescriptor trd;
  trd.m_bAllowToReadContents = true;
  trd.m_bGenerateMipmaps = false;
  trd.m_TextureFormat = AE_FORMAT_R8G8B8A8_UNORM;
  trd.m_TextureType = AE_TYPE_TEXTURE_2D;
  trd.m_uiWidth = g_uiLeafCardRenderTargetSize;
  trd.m_uiHeight = g_uiLeafCardRenderTargetSize;

  aeTextureResource::CreateResource(g_hLeafCardDiffuseRT, &trd);
  aeTextureResource::CreateResource(g_hLeafCardNormalRT, &trd);

  trd.m_bAllowToReadContents = false;
  trd.m_TextureFormat = AE_FORMAT_DEPTH24_STENCIL8;

  aeTextureResource::CreateResource(g_hLeafCardDepthRT, &trd);

  g_LeafCardFB.setDepthAttachment(g_hLeafCardDepthRT);
  g_LeafCardFB.setColorAttachment(0, g_hLeafCardDiffuseRT);
  g_LeafCardFB.setColorAttachment(1, g_hLeafCardNormalRT);

  g_LeafCardClearColorFB.setColorAttachment(0, g_hLeafCardDiffuseRT);
  g_LeafCardClearNormalFB.setColorAttachment(0, g_hLeafCardNormalRT);
}

void aeTreePlugin::RenderLeafCard(aeUInt32 uiRenderSize, bool bExportNow)
{
  if (!bExportNow)
    aeRenderAPI::Clear(true, true, 0.2f, 0.2f, 0.2f);

  float fSize;

  aeMatrix mTBN;
  mTBN.SetIdentity();

  if (g_Tree.m_Descriptor.m_bSnapshotFromAbove)
  {
    const aeVec3 vCenter = g_Tree.m_BBox.GetCenter();

    fSize = aeMath::Max(g_Tree.m_BBox.GetSize().x, g_Tree.m_BBox.GetSize().z);

    g_Globals.m_OrthoCamera.setViewport(0, 0, uiRenderSize, uiRenderSize);
    g_Globals.m_OrthoCamera.setLookAt(aeVec3(vCenter.x, 1000, vCenter.z + 0.1f), aeVec3(vCenter.x, 0, vCenter.z));
    g_Globals.m_OrthoCamera.setOrthoCamera(-fSize * 0.5f, fSize * 0.5f, -fSize * 0.5f, fSize * 0.5f, -1000, 1000);

    g_Globals.m_OrthoCamera.ApplyCamera();

    if (!bExportNow)
    {
      const float fSize2 = (fSize / 2.0f) - 0.01f;
      aeShaderManager::setShader(g_Globals.s_hSkeletonShader);
      glim.BeginBatch();
      glim.Attribute4ub("attr_Color", 100, 100, 255);
      glim.Begin(GLIM_LINE_LOOP);
      glim.Vertex(vCenter.x - fSize2, 1, vCenter.z - fSize2);
      glim.Vertex(vCenter.x + fSize2, 1, vCenter.z - fSize2);
      glim.Vertex(vCenter.x + fSize2, 1, vCenter.z + fSize2);
      glim.Vertex(vCenter.x - fSize2, 1, vCenter.z + fSize2);
      glim.End();
      glim.EndBatch();
      glim.RenderBatch();
    }

    mTBN.SetColumn(0, 1, 0, 0, 0);
    mTBN.SetColumn(1, 0, 0, -1, 0);
    mTBN.SetColumn(2, 0, 1, 0, 0);
  }
  else
  {
    const float fSizeX = aeMath::Max(g_Tree.m_BBox.m_vMax.x, aeMath::Abs(g_Tree.m_BBox.m_vMin.x)) * 2.0f;
    fSize = aeMath::Max(fSizeX, g_Tree.m_BBox.GetSize().y);

    g_Globals.m_OrthoCamera.setViewport(0, 0, uiRenderSize, uiRenderSize);
    g_Globals.m_OrthoCamera.setLookAt(aeVec3(0, 0, 100), aeVec3(0, 0, 0));
    g_Globals.m_OrthoCamera.setOrthoCamera(-fSize * 0.5f, fSize * 0.5f, g_Tree.m_BBox.m_vMin.y, g_Tree.m_BBox.m_vMin.y + fSize, -1000, 1000);

    g_Globals.m_OrthoCamera.ApplyCamera();

    if (!bExportNow)
    {
      const float fSize2 = (fSize / 2.0f) - 0.01f;
      aeShaderManager::setShader(g_Globals.s_hSkeletonShader);
      glim.BeginBatch();
      glim.Attribute4ub("attr_Color", 100, 100, 255);
      glim.Begin(GLIM_LINE_LOOP);
      glim.Vertex(-fSize2, g_Tree.m_BBox.m_vMin.y + 0.01f, 0);
      glim.Vertex(+fSize2, g_Tree.m_BBox.m_vMin.y + 0.01f, 0);
      glim.Vertex(+fSize2, g_Tree.m_BBox.m_vMin.y + fSize - 0.01f, 0);
      glim.Vertex(-fSize2, g_Tree.m_BBox.m_vMin.y + fSize - 0.01f, 0);
      glim.End();
      glim.EndBatch();
      glim.RenderBatch();
    }
  }

  g_Globals.m_OrthoCamera.ApplyCamera();

  aeShaderManager::setShaderBuilderVariable("EXPORT_LEAFCARD", true);

  float fInvTBN[9];
  mTBN.GetInverse().GetAsOpenGL3x3Matrix(fInvTBN);
  aeShaderManager::setUniformMatrix3x3("unif_InverseTBN", false, fInvTBN);

  g_Globals.s_vSunLightDir.Normalize();
  aeShaderManager::setUniformFloat("unif_LightPos", 3, g_Globals.s_vPointLightPos.x, g_Globals.s_vPointLightPos.y, g_Globals.s_vPointLightPos.z);
  aeShaderManager::setUniformFloat("unif_SunDir", 3, g_Globals.s_vSunLightDir.x, g_Globals.s_vSunLightDir.y, g_Globals.s_vSunLightDir.z);

  g_Tree.Render(aeVec3::ZeroVector(), bExportNow, true);

  aeShaderManager::setShaderBuilderVariable("EXPORT_LEAFCARD", false);
}

static void PrepareRenderTargets(void)
{
  SetupLeafCardFBs();

  g_LeafCardClearColorFB.BindFramebuffer();
  g_LeafCardClearColorFB.ClearCurrentBuffers(true, false, 0, 0, 0, 0);

  g_LeafCardClearNormalFB.BindFramebuffer();
  g_LeafCardClearNormalFB.ClearCurrentBuffers(true, false, 0.5f, 0.5f, 1.0f, 0.0f);

  g_LeafCardFB.BindFramebuffer();
  aeRenderAPI::Clear(false, true);
}

bool aeTreePlugin::ExportLeafCard(aeUInt32 uiRenderSize, const char* szFilePath, bool bDDS)
{
  aeProgressBar pb("Exporting LeafCard", bDDS ? 4 : 2);

  PrepareRenderTargets();

  uiRenderSize = aeMath::Min(uiRenderSize, g_uiLeafCardRenderTargetSize);

  RenderLeafCard(uiRenderSize, true);

  aeHybridString<256> sText;

  {
    aeFilePath sPathDiffuse;
    sPathDiffuse.Format("%s.tga", szFilePath);
    aeFilePath sPathNormal;
    sPathNormal.Format("%s_N.tga", szFilePath);

    aeArray<aeUInt8> Image, SubImage;
    Image.resize(g_uiLeafCardRenderTargetSize * g_uiLeafCardRenderTargetSize * 4);

    {
      sText.Format("Writing TGA:\n'%s'", sPathDiffuse.c_str());
      aeProgressBar::Update(sText.c_str()); // 1

      g_hLeafCardDiffuseRT.GetResource()->ReadbackTexture(&Image[0]);
      GetSubImage(Image, g_uiLeafCardRenderTargetSize, g_uiLeafCardRenderTargetSize, SubImage, uiRenderSize, uiRenderSize);

      FillAvgImageColor(SubImage);

      for (aeInt32 d = g_Tree.m_Descriptor.m_uiLeafCardTexelDilation * 2; d > 0; --d)
        DilateColors(SubImage, uiRenderSize, uiRenderSize, d);

      ClearAlpha(SubImage, g_Tree.m_Descriptor.m_uiLeafCardTexelDilation * 2);

      if (!ExportTGA(sPathDiffuse.c_str(), uiRenderSize, uiRenderSize, 4, &SubImage[0], true))
        return false;

      if (aeProgressBar::WasProgressBarCanceled())
        return true;

      if (bDDS)
      {
        sText.Format("Converting to DDS:\n'%s'", sPathDiffuse.c_str());
        aeProgressBar::Update(sText.c_str()); // 2

        if (!ConvertTGAtoDDS(sPathDiffuse.c_str(), false))
          return false;
        if (aeProgressBar::WasProgressBarCanceled())
          return true;
      }
    }

    {
      sText.Format("Writing TGA:\n'%s'", sPathNormal.c_str());
      aeProgressBar::Update(sText.c_str()); // 3

      g_hLeafCardNormalRT.GetResource()->ReadbackTexture(&Image[0]);
      GetSubImage(Image, g_uiLeafCardRenderTargetSize, g_uiLeafCardRenderTargetSize, SubImage, uiRenderSize, uiRenderSize);

      for (aeInt32 d = g_Tree.m_Descriptor.m_uiLeafCardTexelDilation; d > 0; --d)
        DilateColors(SubImage, uiRenderSize, uiRenderSize, d);

      ClearAlpha(SubImage, g_Tree.m_Descriptor.m_uiLeafCardTexelDilation * 2);

      if (!ExportTGA(sPathNormal.c_str(), uiRenderSize, uiRenderSize, 4, &SubImage[0], true))
        return false;
      if (aeProgressBar::WasProgressBarCanceled())
        return true;

      if (bDDS)
      {
        sText.Format("Converting to DDS:\n'%s'", sPathDiffuse.c_str());
        aeProgressBar::Update(sText.c_str()); // 4

        if (!ConvertTGAtoDDS(sPathNormal.c_str(), true))
          return false;
        if (aeProgressBar::WasProgressBarCanceled())
          return true;
      }
    }
  }

  return true;
}

bool aeTreePlugin::ExportLeafCards(const char* szFilePath, bool bDDS)
{
  aeUInt32 uiMaxRenderSize = aeLeafCardResolution::Values[g_Tree.m_Descriptor.m_LeafCardResolution];
  aeUInt32 uiMinRenderSize = aeLeafCardMipmapResolution::Values[g_Tree.m_Descriptor.m_LeafCardMipmaps];

  aeInt32 iNumFiles = 0;

  {
    aeUInt32 rs = uiMaxRenderSize;
    while (rs >= uiMinRenderSize)
    {
      iNumFiles++;
      rs /= 2;
    }
  }

  aeProgressBar pb("Exporting LeafCards", iNumFiles);

  uiMaxRenderSize = aeMath::PowerOfTwo_Floor(uiMaxRenderSize);

  uiMaxRenderSize = aeMath::Clamp<aeInt32>(uiMaxRenderSize, 4, g_uiLeafCardRenderTargetSize);
  uiMinRenderSize = aeMath::Clamp<aeInt32>(uiMinRenderSize, 1, uiMaxRenderSize);

  aeInt32 iLod = 0;
  aeFilePath sNoExt = aePathFunctions::ChangeExistingFileExtension(szFilePath, "");
  aeFilePath sPath = sNoExt;

  while (uiMaxRenderSize >= uiMinRenderSize)
  {
    if (!ExportLeafCard(uiMaxRenderSize, sPath.c_str(), bDDS))
      return false;

    aeProgressBar::Update(); // 1 to n

    if (aeProgressBar::WasProgressBarCanceled())
    {
      QMessageBox::warning(nullptr, "Kraut", "LeafCard Export was canceled.");
      return true;
    }

    uiMaxRenderSize /= 2;
    ++iLod;

    sPath.Format("%s_LOD%i.tga", sNoExt.c_str(), iLod);
  }

  return true;
}
