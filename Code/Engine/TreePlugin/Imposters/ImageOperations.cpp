#include "PCH.h"

#include "../Tree/Tree.h"
#include "../Basics/Plugin.h"
#include <KrautGraphics/RenderAPI/Main.h>
#include <KrautGraphics/TextureManager/Main.h>
#include <KrautGraphics/ShaderManager/Main.h>
#include <KrautGraphics/Framebuffer/Main.h>


using namespace AE_NS_GRAPHICS;

bool ConvertTGAtoDDS (const char* szFile, bool bNormalMap);

void GetSubImage (const aeArray<aeUInt8>& Image, aeUInt32 uiWidth, aeUInt32 uiHeight, aeArray<aeUInt8>& out_SubImage, aeUInt32 uiSubWidth, aeUInt32 uiSubHeight)
{
  if ((uiWidth == uiSubWidth) && (uiHeight == uiSubHeight))
  {
    out_SubImage = Image;
    return;
  }

  out_SubImage.resize (uiSubWidth * uiSubHeight * 4);

  for (aeUInt32 h = 0; h < uiSubHeight; ++h)
  {
    for (aeUInt32 w = 0; w < uiSubWidth; ++w)
    {
      const aeUInt32 uiSubIndex = (h * uiSubWidth + w) * 4;
      const aeUInt32 uiIndex = (h * uiWidth + w) * 4;

      out_SubImage[uiSubIndex + 0] = Image[uiIndex + 0];
      out_SubImage[uiSubIndex + 1] = Image[uiIndex + 1];
      out_SubImage[uiSubIndex + 2] = Image[uiIndex + 2];
      out_SubImage[uiSubIndex + 3] = Image[uiIndex + 3];
    }
  }
}

inline static aeColor GetPixel (const aeArray<aeUInt8>& Image, aeUInt32 uiWidth, aeUInt32 x, aeUInt32 y)
{
  aeUInt32 uiIndex = (y * uiWidth + x) * 4;
  aeColor c;

  c.r = Image[uiIndex + 0];
  c.g = Image[uiIndex + 1];
  c.b = Image[uiIndex + 2];
  c.a = Image[uiIndex + 3];

  return c;
}

inline static void SetPixel (aeArray<aeUInt8>& Image, aeUInt32 uiWidth, aeUInt32 x, aeUInt32 y, const aeColor& c)
{
  aeUInt32 uiIndex = (y * uiWidth + x) * 4;

  Image[uiIndex + 0] = c.r;
  Image[uiIndex + 1] = c.g;
  Image[uiIndex + 2] = c.b;
  Image[uiIndex + 3] = c.a;
}

void ClearAlpha (aeArray<aeUInt8>& Image, aeUInt8 uiAlphaThreshold)
{
  #pragma omp parallel for 
  for (int i = 0; i < (int) Image.size (); i += 4)
  {
    if (Image[i + 3] <= uiAlphaThreshold)
      Image[i + 3] = 0;
  }
}

void EnsureBlackAlphaBorder (aeArray<aeUInt8>& Image, aeUInt32 uiWidth, aeUInt32 uiHeight, aeUInt32 uiStartX, aeUInt32 uiRectWidth)
{
  for (aeUInt32 y = 0; y < uiHeight; ++y)
  {
    Image[((uiWidth * y) + uiStartX) * 4 + 3] = 0;
    Image[((uiWidth * y) + uiStartX + uiRectWidth - 1) * 4 + 3] = 0;
  }
}

static aeColor GetAvgColor (const aeArray<aeUInt8>& Image, aeUInt32 uiWidth, aeUInt32 uiHeight, aeInt32 x, aeInt32 y, aeUInt8 uiPass)
{
  aeColor ret = GetPixel (Image, uiWidth, x, y);

  if (ret.a > 0)
    return ret;

  float r = 0, g = 0, b = 0, a = 0;

  const aeInt32 iRadius = 1;

  for (aeInt32 cy = aeMath::Max<aeInt32> (0, y - iRadius); cy <= aeMath::Min<aeInt32> (y + iRadius, uiHeight-1); ++cy)
  {
    for (aeInt32 cx = aeMath::Max<aeInt32> (0, x - iRadius); cx <= aeMath::Min<aeInt32> (x + iRadius, uiWidth-1); ++cx)
    {
      const aeColor col = GetPixel (Image, uiWidth, cx, cy);

      if (col.a > uiPass)
      {
        r += (float) col.r;
        g += (float) col.g;
        b += (float) col.b;
        a += 1.0f;
      }
    }
  }

  if (a > 0.0f)
  {
    r /= a;
    g /= a;
    b /= a;
    
    ret.r = (aeUInt8) r;
    ret.g = (aeUInt8) g;
    ret.b = (aeUInt8) b;
    ret.a = uiPass;
  }

  return ret;
}

void DilateColors (aeArray<aeUInt8>& Image, aeUInt32 uiWidth, aeUInt32 uiHeight, aeUInt8 uiPass)
{
  #pragma omp parallel for 
  for (int y = 0; y < (int) uiHeight; ++y)
  {
    for (aeUInt32 x = 0; x < uiWidth; ++x)
    {
      aeColor col = GetAvgColor (Image, uiWidth, uiHeight, x, y, uiPass);

      SetPixel (Image, uiWidth, x, y, col);
    }
  }
}

void FillAvgImageColor (aeArray<aeUInt8>& Image)
{
  aeVec3 c (0.0f);
  float fCount = 0.0f;

  for (aeUInt32 i = 0; i < Image.size (); i += 4)
  {
    if (Image[i + 3] > 0)
    {
      c.x += Image[i + 0] / 255.0f;
      c.y += Image[i + 1] / 255.0f;
      c.z += Image[i + 2] / 255.0f;

      fCount += 1.0f;
    }
  }

  c /= fCount;

  aeColor col;
  col.r = (aeUInt8) aeMath::Clamp ((aeInt32) (c.x * 255.0f), 0, 255);
  col.g = (aeUInt8) aeMath::Clamp ((aeInt32) (c.y * 255.0f), 0, 255);
  col.b = (aeUInt8) aeMath::Clamp ((aeInt32) (c.z * 255.0f), 0, 255);

  for (aeUInt32 i = 0; i < Image.size (); i += 4)
  {
    if (Image[i + 3] == 0)
    {
      Image[i + 0] = col.r;
      Image[i + 1] = col.g;
      Image[i + 2] = col.b;
    }
  }
}
