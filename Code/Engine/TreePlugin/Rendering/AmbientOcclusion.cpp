#include "PCH.h"

#include "../Tree/Tree.h"
#include "../Basics/Plugin.h"
#include "../GUI/ProgressBar.h"
#include "AmbientOcclusion.h"

#include "../GUI/qtTreeEditWidget/qtTreeEditWidget.moc.h"

#include <KrautFoundation/Configuration/VariableRegistry.h>

#include <KrautGraphics/RenderAPI/Main.h>
#include <KrautGraphics/TextureManager/Main.h>
#include <KrautGraphics/ShaderManager/Main.h>
#include <KrautGraphics/Framebuffer/Main.h>
using namespace AE_NS_GRAPHICS;

aeBBox aeAOGrid::m_BBox;
aeUInt32 aeAOGrid::m_uiCells[3];
aeArray<aeAOGrid::aeDirection> aeAOGrid::m_Cells;
float aeAOGrid::m_fCellSize;
aeArray<aeUInt8> aeAOGrid::m_RenderedImage;

static aeFramebuffer g_AmbientOccFB;
static aeTextureResourceHandle g_hAmbientOccDiffuseRT;
static aeTextureResourceHandle g_hAmbientOccDepthRT;
static aeUInt32 g_uiAmbientOccRTSize = 16;
static aeUInt32 g_uiCubeFaces = 6;
static double g_FormFactorSum = 0.0;
static aeArray<double> g_FormFactors;


AE_ON_GLOBAL_EVENT_ONCE (aeStartup_ShutdownEngine_Begin)
{
  g_hAmbientOccDiffuseRT.Invalidate ();
  g_hAmbientOccDepthRT.Invalidate ();
  g_AmbientOccFB.Shutdown ();
}

static void SetupAmbientOccFBs ()
{
  AE_EXECUTE_ONLY_ONCE;

  aeTextureResourceDescriptor trd;
  trd.m_bAllowToReadContents = true;
  trd.m_bGenerateMipmaps = false;
  trd.m_TextureFormat = AE_FORMAT_R8G8B8A8_UNORM;
  trd.m_TextureType = AE_TYPE_TEXTURE_2D;
  trd.m_uiWidth = g_uiAmbientOccRTSize;
  trd.m_uiHeight = g_uiAmbientOccRTSize * g_uiCubeFaces;

  aeTextureResource::CreateResource (g_hAmbientOccDiffuseRT, &trd);

  trd.m_bAllowToReadContents = false;
  trd.m_TextureFormat = AE_FORMAT_DEPTH24_STENCIL8;

  aeTextureResource::CreateResource (g_hAmbientOccDepthRT, &trd);

  g_AmbientOccFB.setDepthAttachment (g_hAmbientOccDepthRT);
  g_AmbientOccFB.setColorAttachment (0, g_hAmbientOccDiffuseRT);
}

AE_ON_GLOBAL_EVENT (aeTreePlugin_TreeUnloaded)
{
  aeAOGrid::Clear ();
}

aeAOGrid::aeDirection aeAOGrid::SampleGrid (const aeVec3& vPos)
{
  if (m_Cells.empty ())
    return 1.0f;

  const aeVec3 vRelative = (vPos - m_BBox.m_vMin) / m_fCellSize;

  const aeInt32 iCell[3] = 
  {
    (aeInt32)aeMath::Floor (vRelative.x),
    (aeInt32)aeMath::Floor (vRelative.y),
    (aeInt32)aeMath::Floor (vRelative.z),
  };

  const float fInterplation[3] = 
  {
    vRelative.x - (float) iCell[0],
    vRelative.y - (float) iCell[1],
    vRelative.z - (float) iCell[2],
  };

  aeInt32 iCellL[3];
  aeInt32 iCellH[3];

  for (aeInt32 i = 0; i < 3; ++i)
  {
    iCellL[i] = aeMath::Clamp<aeInt32> (iCell[i], 0, m_uiCells[i] - 1);
    iCellH[i] = aeMath::Clamp<aeInt32> (iCellL[i] + 1, 0, m_uiCells[i] - 1);
  }

  const aeDirection fValues8[8] =
  {
    GetCell (iCellL[0], iCellL[1], iCellL[2]),
    GetCell (iCellL[0], iCellL[1], iCellH[2]),
    GetCell (iCellL[0], iCellH[1], iCellL[2]),
    GetCell (iCellL[0], iCellH[1], iCellH[2]),
    GetCell (iCellH[0], iCellL[1], iCellL[2]),
    GetCell (iCellH[0], iCellL[1], iCellH[2]),
    GetCell (iCellH[0], iCellH[1], iCellL[2]),
    GetCell (iCellH[0], iCellH[1], iCellH[2]),
  };

  const aeDirection fValues4[4] =
  {
    aeMath::Lerp(fValues8[0], fValues8[4], fInterplation[0]),
    aeMath::Lerp(fValues8[1], fValues8[5], fInterplation[0]),
    aeMath::Lerp(fValues8[2], fValues8[6], fInterplation[0]),
    aeMath::Lerp(fValues8[3], fValues8[7], fInterplation[0]),
  };

  const aeDirection fValues2[2] =
  {
    aeMath::Lerp(fValues4[0], fValues4[2], fInterplation[1]),
    aeMath::Lerp(fValues4[1], fValues4[3], fInterplation[1]),
  };

  const aeDirection fValue = aeMath::Lerp(fValues2[0], fValues2[1], fInterplation[2]);

  return fValue;
}

static double ComputeFormFactor (aeUInt32 uiTexel, aeUInt32 uiRenderTargetWidth)
{
  const aeUInt32 y = uiTexel / uiRenderTargetWidth;
  const aeUInt32 x = uiTexel % uiRenderTargetWidth;

  const double fX = (x / (double) uiRenderTargetWidth) * 2.0 - 1.0;
  const double fY = (y / (double) uiRenderTargetWidth) * 2.0 - 1.0;

  const double fFormFactor = 1.0 / aeMath::Sqrt (fX * fX + fY * fY + 1.0);

  return fFormFactor;
}

void aeAOGrid::Update (aeBBox box, float fCellSize)
{
  box.AddBoundary (aeVec3 (0.1f));

  aeUInt32 uiCells[3];

  uiCells[0] = aeMath::Ceil ((box.m_vMax.x - box.m_vMin.x) / fCellSize) + 1;
  uiCells[1] = aeMath::Ceil ((box.m_vMax.y - box.m_vMin.y) / fCellSize) + 1;
  uiCells[2] = aeMath::Ceil ((box.m_vMax.z - box.m_vMin.z) / fCellSize) + 1;

  const aeUInt32 uiMaxCells = uiCells[0] * uiCells[1] * uiCells[2];

  QMainWindow* pMainWindow = NULL;
  aeVariableRegistry::RetrieveRaw ("system/qt/mainwidget", &pMainWindow, sizeof (QMainWindow*));

  aeString s;
  s.Format ("With the current settings %i samples will be computed.\nYou can reduce the number of samples by increasing the sample size. Also please make sure that the bounding box of the tree is as tightly fitting as possible. You can modify the bounding box in the LeafCard panel (enable 'Preview Leafcard' to see how well the bounding box fits.\n\nDo you want to continue?", uiMaxCells);
  if (QMessageBox::question (pMainWindow, "Kraut", s.c_str (), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) != QMessageBox::Yes)
    return;

  aeProgressBar pb ("Computing Ambient Occlusion", uiMaxCells + 1);

  m_uiCells[0] = uiCells[0];
  m_uiCells[1] = uiCells[1];
  m_uiCells[2] = uiCells[2];

  m_Cells.resize (uiMaxCells);

  m_fCellSize = fCellSize;
  m_BBox = box;

  SetupAmbientOccFBs ();
  g_AmbientOccFB.BindFramebuffer ();
  m_RenderedImage.resize (g_uiAmbientOccRTSize * g_uiCubeFaces * g_uiAmbientOccRTSize * 4);

  aeShaderManager::setShaderBuilderVariable ("EXPORT_LEAFCARD", true);

  for (aeUInt32 i = 0; i < m_Cells.size (); ++i)
    m_Cells[i] = 0.0f;

  aeProgressBar::Update ();

  g_FormFactorSum = 0.0;
  g_FormFactors.resize (g_uiAmbientOccRTSize * g_uiAmbientOccRTSize);
  for (aeUInt32 texel = 0; texel < g_FormFactors.size (); ++texel)
  {
    const double dFormFactor = ComputeFormFactor (texel, g_uiAmbientOccRTSize);
    g_FormFactors[texel] = dFormFactor / 255.0;
    g_FormFactorSum += dFormFactor;
  }

  for (aeUInt32 z = 0; z < m_uiCells[2]; ++z)
  {
    for (aeUInt32 y = 0; y < m_uiCells[1]; ++y)
    {
      for (aeUInt32 x = 0; x < m_uiCells[0]; ++x)
      {
        UpdatePosition (x, y, z);
        aeProgressBar::Update ();

        if (aeProgressBar::WasProgressBarCanceled ())
          goto aborted;
      }
    }
  }

  goto end;

aborted:
  Clear ();

end:
  aeShaderManager::setShaderBuilderVariable ("EXPORT_LEAFCARD", true);
}

void aeAOGrid::RenderCubeFace (aeInt32 iFace, const aeVec3& vFromPos, const aeVec3& vDir, const aeVec3& vUp)
{
  g_Globals.m_Camera.setLookAt (vFromPos, vFromPos + vDir, vUp);
  g_Globals.m_Camera.setPerspectiveCamera (90, 0.001f, 20.0f);
  g_Globals.m_Camera.setViewport (0, g_uiAmbientOccRTSize * iFace, g_uiAmbientOccRTSize, g_uiAmbientOccRTSize);
  g_Globals.m_Camera.ApplyCamera ();

  g_Tree.RenderTreeMesh (aeLod::Lod0);
}

aeAOGrid::aeDirection aeAOGrid::GetRenderedResult (void)
{
  g_hAmbientOccDiffuseRT.GetResource ()->ReadbackTexture (m_RenderedImage.data ());
  const aeUInt32 uiTexels = m_RenderedImage.size () / g_uiCubeFaces;

  aeDirection res;

  for (aeUInt32 face = 0; face < g_uiCubeFaces; ++face)
  {
    double uiSummedAlpha = 0;

    aeUInt32 uiCurTexel = 0;
    for (aeUInt32 i = uiTexels * face + 3; i < uiTexels * (1 + face); i += 4, ++uiCurTexel)
      uiSummedAlpha += m_RenderedImage[i] * g_FormFactors[uiCurTexel];

    res.m_fDir[face] = 1.0 - (uiSummedAlpha / g_FormFactorSum);
  }

  return res;
}

void aeAOGrid::UpdatePosition (aeUInt32 x, aeUInt32 y, aeUInt32 z)
{
  const aeVec3 vPos = m_BBox.m_vMin + aeVec3 (x, y, z) * m_fCellSize;

  g_AmbientOccFB.ClearCurrentBuffers (true, true);

  RenderCubeFace (0, vPos, aeVec3 ( 1, 0, 0), aeVec3 ( 0, 1, 0));
  RenderCubeFace (1, vPos, aeVec3 (-1, 0, 0), aeVec3 ( 0, 1, 0));
  RenderCubeFace (2, vPos, aeVec3 ( 0, 1, 0), aeVec3 ( 1, 0, 0));
  RenderCubeFace (3, vPos, aeVec3 ( 0,-1, 0), aeVec3 ( 1, 0, 0));
  RenderCubeFace (4, vPos, aeVec3 ( 0, 0, 1), aeVec3 ( 0, 1, 0));
  RenderCubeFace (5, vPos, aeVec3 ( 0, 0,-1), aeVec3 ( 0, 1, 0));

  SetCell (x, y, z, GetRenderedResult ());
}

void ComputeAmbientOcclusion (void)
{
  aeAOGrid::Update (g_Tree.m_BBox, g_Tree.m_Descriptor.m_fAOSampleSize);
}

float aeAOGrid::GetAmbientOcclusionAt (const aeVec3& vPos, const aeVec3& vNormal0)
{
  if (!g_Tree.m_Descriptor.m_bUseAO)
    return 1.0f;

  aeVec3 vNormal = vNormal0;

  if (vNormal.IsZeroVector (0.001f))
  {
    vNormal = (vPos - g_Tree.m_BBox.GetCenter ()).GetNormalized ();
    //return 1.0f;
  }

  const float fOffset = aeMath::Min (0.2f, g_Tree.m_Descriptor.m_fAOSampleSize);

  const aeAOGrid::aeDirection vAO = aeAOGrid::SampleGrid (vPos + vNormal * fOffset);

  float fPX = aeMath::Square (aeMath::Max (0.0f, aeVec3::GetAxisX ().Dot (vNormal))) * vAO.m_fDir[0];
  float fNX = aeMath::Square (aeMath::Max (0.0f,-aeVec3::GetAxisX ().Dot (vNormal))) * vAO.m_fDir[1];
  float fPY = aeMath::Square (aeMath::Max (0.0f, aeVec3::GetAxisY ().Dot (vNormal))) * vAO.m_fDir[2];
  float fNY = aeMath::Square (aeMath::Max (0.0f,-aeVec3::GetAxisY ().Dot (vNormal))) * vAO.m_fDir[3];
  float fPZ = aeMath::Square (aeMath::Max (0.0f, aeVec3::GetAxisZ ().Dot (vNormal))) * vAO.m_fDir[4];
  float fNZ = aeMath::Square (aeMath::Max (0.0f,-aeVec3::GetAxisZ ().Dot (vNormal))) * vAO.m_fDir[5];

  float fAO = fPX + fNX + fPY + fNY + fPZ + fNZ;

  return aeMath::Pow (fAO, g_Tree.m_Descriptor.m_fAOContrast);
}

static aeUInt8 g_uiAOGridVersion = 1;

void aeAOGrid::Save (aeStreamOut& stream)
{
  stream << g_uiAOGridVersion;

  stream << m_BBox.m_vMin;
  stream << m_BBox.m_vMax;
  stream << m_uiCells[0];
  stream << m_uiCells[1];
  stream << m_uiCells[2];
  stream << m_fCellSize;

  const aeUInt32 uiCells = m_Cells.size ();
  stream << uiCells;

  for (aeUInt32 c = 0; c < uiCells; c++)
  {
    stream << m_Cells[c].m_fDir[0];
    stream << m_Cells[c].m_fDir[1];
    stream << m_Cells[c].m_fDir[2];
    stream << m_Cells[c].m_fDir[3];
    stream << m_Cells[c].m_fDir[4];
    stream << m_Cells[c].m_fDir[5];
  }
}

void aeAOGrid::Load (aeStreamIn& stream)
{
  aeUInt8 uiVersion;
  stream >> uiVersion;

  AE_CHECK_ALWAYS (uiVersion <= g_uiAOGridVersion, "The file to be loaded is of version %i, and thus either corrupted or newer than what this version of Kraut can load (%i).", uiVersion, g_uiAOGridVersion);

  stream >> m_BBox.m_vMin;
  stream >> m_BBox.m_vMax;
  stream >> m_uiCells[0];
  stream >> m_uiCells[1];
  stream >> m_uiCells[2];
  stream >> m_fCellSize;

  aeUInt32 uiCells = 0;
  stream >> uiCells;

  m_Cells.resize (uiCells);

  for (aeUInt32 c = 0; c < uiCells; c++)
  {
    stream >> m_Cells[c].m_fDir[0];
    stream >> m_Cells[c].m_fDir[1];
    stream >> m_Cells[c].m_fDir[2];
    stream >> m_Cells[c].m_fDir[3];
    stream >> m_Cells[c].m_fDir[4];
    stream >> m_Cells[c].m_fDir[5];
  }
}


