// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#include "DDSLoader.h"
#include "../RenderAPI/Main.h"
#include "../glew/glew.h"
#include <KrautGraphics/FileSystem/FileIn.h>


namespace AE_NS_GRAPHICS
{
#define DDSD_CAPS 0x00000001
#define DDSD_HEIGHT 0x00000002
#define DDSD_WIDTH 0x00000004
#define DDSD_PITCH 0x00000008
#define DDSD_PIXELFORMAT 0x00001000
#define DDSD_MIPMAPCOUNT 0x00020000
#define DDSD_LINEARSIZE 0x00080000
#define DDSD_DEPTH 0x00800000

#define DDPF_ALPHAPIXELS 0x00000001
#define DDPF_FOURCC 0x00000004
#define DDPF_RGB 0x00000040

#define DDSCAPS_COMPLEX 0x00000008
#define DDSCAPS_TEXTURE 0x00001000
#define DDSCAPS_MIPMAP 0x00400000

#define DDSCAPS2_CUBEMAP 0x00000200
#define DDSCAPS2_CUBEMAP_POSITIVEX 0x00000400
#define DDSCAPS2_CUBEMAP_NEGATIVEX 0x00000800
#define DDSCAPS2_CUBEMAP_POSITIVEY 0x00001000
#define DDSCAPS2_CUBEMAP_NEGATIVEY 0x00002000
#define DDSCAPS2_CUBEMAP_POSITIVEZ 0x00004000
#define DDSCAPS2_CUBEMAP_NEGATIVEZ 0x00008000
#define DDSCAPS2_VOLUME 0x00200000

  struct DDS_PixelFormat
  {
    aeUInt32 dwSize;
    aeUInt32 dwFlags;
    aeUInt32 dwFourCC;
    aeUInt32 dwRGBBitCount;
    aeUInt32 dwRBitMask;
    aeUInt32 dwGBitMask;
    aeUInt32 dwBBitMask;
    aeUInt32 dwRGBAlphaBitMask;
  };

  struct DDS_Caps2
  {
    aeUInt32 dwCaps1;
    aeUInt32 dwCaps2;
    aeUInt32 Reserved[2];
  };

  struct DDS_SurfaceDesc
  {
    aeUInt32 dwSize;
    aeUInt32 dwFlags;
    aeUInt32 dwHeight;
    aeUInt32 dwWidth;
    aeUInt32 dwPitchOrLinearSize;
    aeUInt32 dwDepth;
    aeUInt32 dwMipMapCount;
    aeUInt32 dwReserved1[11];
    DDS_PixelFormat ddpfPixelFormat;
    DDS_Caps2 ddsCaps;
    aeUInt32 dwReserved2;
  };


  struct DDS_Header
  {
    aeUInt32 m_uiMagic;
    DDS_SurfaceDesc m_SurfaceDesc;
  };

  aeDDSLoader::aeDDSLoader()
  {
    m_TextureType = AE_DDS_NONE;
    m_Compression = AE_COMPR_NONE;
    m_uiWidth = m_uiHeight = m_uiDepth = 1;
    m_iMipmaps = 1;
  }

  void aeDDSLoader::PrintDDSInformation(const char* szFile)
  {
    aeFileIn File;

    if (!File.Open(szFile))
    {
      aeLog::Error("aeDDSLoader::PrintDDSInformation: DDS-File \"%s\" could not be opened", szFile);
      return;
    }

    aeUInt32 uiMagic;
    File >> uiMagic;

    aeLog::Log("Magic Number is %u", uiMagic);

    if (uiMagic != 542327876) // Compare with "DDS"
    {
      aeLog::Error("Wrong Magic Number in DDS-File.");
      return;
    }

    DDS_SurfaceDesc Surf;
    File.Read(&Surf, sizeof(DDS_SurfaceDesc));



    aeLog::Log("Texture Dimensions: %d * %d", Surf.dwWidth, Surf.dwHeight);

    if (Surf.dwFlags & DDSD_LINEARSIZE)
      aeLog::Log("Linear Size is: %d", Surf.dwPitchOrLinearSize);
    if (Surf.dwFlags & DDSD_PITCH)
      aeLog::Log("Pitch is: %d", Surf.dwPitchOrLinearSize);

    if (Surf.ddsCaps.dwCaps1 & DDSCAPS_COMPLEX)
      aeLog::Log("Texture is complex.");
    if (Surf.ddsCaps.dwCaps1 & DDSCAPS_MIPMAP)
      aeLog::Log("Texture is mipmapped.");

    if (Surf.ddsCaps.dwCaps2 & DDSCAPS2_VOLUME)
      aeLog::Log("Texture is a Volume, Depth: %d", Surf.dwDepth);

    aeLog::Log("RGB BitCount is %d", Surf.ddpfPixelFormat.dwRGBBitCount);
    aeLog::Log("Bitmask: R %d G %d B %d A %d", Surf.ddpfPixelFormat.dwRBitMask, Surf.ddpfPixelFormat.dwGBitMask, Surf.ddpfPixelFormat.dwBBitMask, Surf.ddpfPixelFormat.dwRGBAlphaBitMask);

    // check, whether it is a cubemap
    if (Surf.ddsCaps.dwCaps2 & DDSCAPS2_CUBEMAP)
    {
      aeLog::Log("Texture is a Cubemap");

      if (Surf.ddsCaps.dwCaps2 & DDSCAPS2_CUBEMAP_POSITIVEX)
        aeLog::Log("Contains Positive X Face");
      if (Surf.ddsCaps.dwCaps2 & DDSCAPS2_CUBEMAP_NEGATIVEX)
        aeLog::Log("Contains Negative X Face");
      if (Surf.ddsCaps.dwCaps2 & DDSCAPS2_CUBEMAP_POSITIVEY)
        aeLog::Log("Contains Positive Y Face");
      if (Surf.ddsCaps.dwCaps2 & DDSCAPS2_CUBEMAP_NEGATIVEY)
        aeLog::Log("Contains Negative Y Face");
      if (Surf.ddsCaps.dwCaps2 & DDSCAPS2_CUBEMAP_POSITIVEZ)
        aeLog::Log("Contains Positive Z Face");
      if (Surf.ddsCaps.dwCaps2 & DDSCAPS2_CUBEMAP_NEGATIVEZ)
        aeLog::Log("Contains Negative Z Face");
    }

    if (Surf.ddsCaps.dwCaps1 & DDSCAPS_MIPMAP)
      aeLog::Log("Number of Mipmaps contained: %d", Surf.dwMipMapCount);
    else
      aeLog::Log("Mipmaps are NOT provided.");


    if (Surf.ddpfPixelFormat.dwFlags & DDPF_FOURCC)
    {
      char szFourCC[5];
      aeMemory::Copy(&Surf.ddpfPixelFormat.dwFourCC, szFourCC, 4);
      szFourCC[4] = '\0';

      aeLog::Log("Texture is FOURCC: %s (%d)", szFourCC, Surf.ddpfPixelFormat.dwFourCC);

      if (Surf.ddpfPixelFormat.dwFlags & DDPF_ALPHAPIXELS)
        aeLog::Log("Alpha Channel is provided.");
    }
    else if (Surf.ddpfPixelFormat.dwFlags & DDPF_RGB)
    {
      aeLog::Log("Texture is RGB.");

      aeLog::Log("Bit-Count: %d", Surf.ddpfPixelFormat.dwRGBBitCount);

      if (Surf.ddpfPixelFormat.dwFlags & DDPF_ALPHAPIXELS)
        aeLog::Log("Alpha Channel is provided.");

      aeLog::Log("Bit-Masks (RGBA): %u | %u | %u | %u", Surf.ddpfPixelFormat.dwRBitMask, Surf.ddpfPixelFormat.dwGBitMask, Surf.ddpfPixelFormat.dwBBitMask, Surf.ddpfPixelFormat.dwRGBAlphaBitMask);
    }
    else
    {
      aeLog::Log("Texture is neither RGB nor FOURCC.");

      aeLog::Log("Bit-Count: %d", Surf.ddpfPixelFormat.dwRGBBitCount);

      aeLog::Log("Bit-Masks (RGBA): %u | %u | %u | %u", Surf.ddpfPixelFormat.dwRBitMask, Surf.ddpfPixelFormat.dwGBitMask, Surf.ddpfPixelFormat.dwBBitMask, Surf.ddpfPixelFormat.dwRGBAlphaBitMask);

      if (Surf.ddpfPixelFormat.dwFlags & DDPF_ALPHAPIXELS)
      {
        aeLog::Log("Alpha Channel is provided.");

        if (Surf.ddpfPixelFormat.dwRBitMask != 0)
        {
          aeLog::Log("Texture is Luminance_Alpha.");

          if (Surf.ddpfPixelFormat.dwRGBBitCount == 16)
            aeLog::Log("Texture is Luminance8_Alpha8.");
        }
        else
        {
          aeLog::Log("Texture is Alpha-only.");

          if (Surf.ddpfPixelFormat.dwRGBBitCount == 8)
            aeLog::Log("Texture is Alpha8.");
        }
      }
      else if (Surf.ddpfPixelFormat.dwRBitMask != 0)
      {
        aeLog::Log("Texture is Luminance-only.");

        if (Surf.ddpfPixelFormat.dwRGBBitCount == 8)
          aeLog::Log("Texture is Luminance8.");
      }
    }
  }

  bool aeDDSLoader::LoadDDS(const char* szFile, bool bLoadForD3D)
  {
    m_bLoadForD3D = bLoadForD3D;

    aeFileIn File;

    if (!File.Open(szFile))
    {
      aeLog::Error("aeDDSLoader::LoadDDS: DDS-File \"%s\" could not be opened", szFile);
      return (false);
    }

    aeUInt32 uiMagic;
    File >> uiMagic;

    if (uiMagic != 542327876) // Compare with "DDS"
    {
      aeLog::Error("No Magic in here");
      return (false);
    }

    DDS_SurfaceDesc Surf;
    File.Read(&Surf, sizeof(DDS_SurfaceDesc));


    // retrieve the texture dimensions
    m_uiWidth = Surf.dwWidth;
    m_uiHeight = Surf.dwHeight;
    m_TextureType = AE_DDS_TEXTURE2D;

    // check, whether it is a volume-texture
    if (Surf.ddsCaps.dwCaps2 & DDSCAPS2_VOLUME)
    {
      m_uiDepth = Surf.dwDepth;
      m_TextureType = AE_DDS_TEXTURE3D;
    }

    // check, whether it is a cubemap
    if (Surf.ddsCaps.dwCaps2 & DDSCAPS2_CUBEMAP)
    {
      m_TextureType = AE_DDS_CUBEMAP;
    }

    // retrieve the number of mipmap-levels to read
    if (Surf.ddsCaps.dwCaps1 & DDSCAPS_MIPMAP)
      m_iMipmaps = Surf.dwMipMapCount;

    if (Surf.ddpfPixelFormat.dwFlags & DDPF_RGB)
    {
      switch (Surf.ddpfPixelFormat.dwRGBBitCount)
      {
        case 24:
          m_Compression = AE_COMPR_RGB8;
          break;
        case 32:
          m_Compression = AE_COMPR_ARGB8;
          break;
      }
    }
    else if (Surf.ddpfPixelFormat.dwFlags & DDPF_FOURCC)
    {
      char szFourCC[5];
      aeMemory::Copy(&Surf.ddpfPixelFormat.dwFourCC, szFourCC, 4);
      szFourCC[4] = '\0';

      if (aeStringFunctions::CompareEqual(szFourCC, "DXT1"))
      {
        if (Surf.ddpfPixelFormat.dwFlags & DDPF_ALPHAPIXELS)
        {
          m_Compression = AE_COMPR_DXT1_A;
          AE_CHECK_ALWAYS(false, "aeDDSLoader::LoadDDS: DXT1 with 1-Bit Alpha is not supported.");
        }
        else
          m_Compression = AE_COMPR_DXT1;
      }
      else if (aeStringFunctions::CompareEqual(szFourCC, "DXT3"))
        m_Compression = AE_COMPR_DXT3;
      else if (aeStringFunctions::CompareEqual(szFourCC, "DXT5"))
        m_Compression = AE_COMPR_DXT5;
      else if (aeStringFunctions::CompareEqual(szFourCC, "q"))
        m_Compression = AE_COMPR_RGBA16F;
      else if (aeStringFunctions::CompareEqual(szFourCC, "t"))
        m_Compression = AE_COMPR_RGBA32F;
    }
    else
    {
      if (Surf.ddpfPixelFormat.dwFlags & DDPF_ALPHAPIXELS)
      {
        if (Surf.ddpfPixelFormat.dwRBitMask != 0)
        {
          if (Surf.ddpfPixelFormat.dwRGBBitCount == 16)
            m_Compression = AE_COMPR_L8A8;
        }
        else
        {
          if (Surf.ddpfPixelFormat.dwRGBBitCount == 8)
            m_Compression = AE_COMPR_A8;
        }
      }
      else if (Surf.ddpfPixelFormat.dwRBitMask != 0)
      {
        if (Surf.ddpfPixelFormat.dwRGBBitCount == 8)
          m_Compression = AE_COMPR_L8;
      }
    }

    if (m_Compression == AE_COMPR_NONE)
    {
      aeLog::Error("aeDDSLoader::LoadDDS: Format of DDS-File \"%s\" is not supported.", szFile);

      PrintDDSInformation(szFile);
      return (false);
    }

    switch (m_TextureType)
    {
      case AE_DDS_TEXTURE2D:
        LoadTexture2D(File);
        break;
      case AE_DDS_TEXTURE3D:
        aeLog::Error("aeDDSLoader::LoadDDS: DDS-File \"%s\": Volume Textures are not yet supported.", szFile);

        PrintDDSInformation(szFile);

        return (false);
        //		LoadTexture3D (File);
        break;
      case AE_DDS_CUBEMAP:
        LoadCubemap(File);
        break;
    }

    std::vector<aeUInt8> Temp(1024 * 1024);
    aeUInt32 read = File.Read(&Temp[0], 1024 * 1024);

    if (read > 0)
      aeLog::Error("aeDDSLoader::LoadDDS: DDS-File \"%s\": Read %d additional bytes.", szFile, read);

    return (true);
  }

  void aeDDSLoader::LoadSlice(aeStreamIn& File, aeUInt32 uiWidth, aeUInt32 uiHeight, aeDSSCompression Compression, std::vector<aeUInt8>& out_Data)
  {
    uiWidth = aeMath::Max<aeUInt32>(uiWidth, 1);
    uiHeight = aeMath::Max<aeUInt32>(uiHeight, 1);

    aeUInt32 uiDataSize = 0;

    switch (Compression)
    {
      case AE_COMPR_L8A8:
        uiDataSize = uiWidth * uiHeight * 2;
        break;
      case AE_COMPR_L8:
      case AE_COMPR_A8:
        uiDataSize = uiWidth * uiHeight * 1;
        break;
      case AE_COMPR_RGB8:
        uiDataSize = uiWidth * uiHeight * 3;
        break;
      case AE_COMPR_ARGB8:
        uiDataSize = uiWidth * uiHeight * 4;
        break;
      case AE_COMPR_DXT1:
      case AE_COMPR_DXT1_A:
        uiDataSize = ((uiWidth + 3) >> 2) * ((uiHeight + 3) >> 2) * 8;
        break;
      case AE_COMPR_DXT3:
      case AE_COMPR_DXT5:
        uiDataSize = ((uiWidth + 3) >> 2) * ((uiHeight + 3) >> 2) * 16;
        break;

      case AE_COMPR_RGBA16F:
        uiDataSize = uiWidth * uiHeight * 8;
        break;
      case AE_COMPR_RGBA32F:
        uiDataSize = uiWidth * uiHeight * 16;
        break;

      default:
        AE_CHECK_ALWAYS(false, "aeDDSLoader::LoadSlice: Unknown Compression %d", Compression);
    }

    out_Data.resize(uiDataSize);
    aeUInt32 uiRead = File.Read(&out_Data[0], uiDataSize);
    if (uiRead != uiDataSize)
      AE_CHECK_ALWAYS(false, "aeDDSLoader::LoadSlice: Reading a DDS-Slice returned %d of %d Bytes", uiRead, uiDataSize);

    if ((!m_bLoadForD3D) || (getTextureType() != AE_DDS_CUBEMAP))
    {
      switch (Compression)
      {
        case AE_COMPR_L8:
        case AE_COMPR_A8:
          // flip vertically
          FlipSliceVertical(uiWidth, uiHeight, 1, out_Data);
          break;
        case AE_COMPR_L8A8:
          // flip vertically
          FlipSliceVertical(uiWidth, uiHeight, 2, out_Data);
          break;
        case AE_COMPR_RGB8:
          // flip vertically
          FlipSliceVertical(uiWidth, uiHeight, 3, out_Data);
          break;
        case AE_COMPR_ARGB8:
          // flip vertically
          FlipSliceVertical(uiWidth, uiHeight, 4, out_Data);
          break;
        case AE_COMPR_RGBA16F:
          // flip vertically
          FlipSliceVertical(uiWidth, uiHeight, 8, out_Data);
          break;
        case AE_COMPR_RGBA32F:
          // flip vertically
          FlipSliceVertical(uiWidth, uiHeight, 16, out_Data);
          break;

        case AE_COMPR_DXT1:
        case AE_COMPR_DXT1_A:
          // flip vertically
          FlipDXT1(uiWidth, uiHeight, out_Data);
          break;
        case AE_COMPR_DXT3:
          // flip vertically
          FlipDXT3(uiWidth, uiHeight, out_Data);
          break;
        case AE_COMPR_DXT5:
          // flip vertically
          FlipDXT5(uiWidth, uiHeight, out_Data);
          break;
      }
    }
  }

  void aeDDSLoader::Load2DMipmapChain(aeStreamIn& File, std::deque<std::vector<aeUInt8>>& out_Data)
  {
    out_Data.clear();
    out_Data.resize(m_iMipmaps);

    aeUInt32 uiWidth = m_uiWidth;
    aeUInt32 uiHeight = m_uiHeight;

    for (aeInt32 i = 0; i < m_iMipmaps; ++i)
    {
      LoadSlice(File, uiWidth, uiHeight, m_Compression, out_Data[i]);

      uiWidth /= 2;
      uiHeight /= 2;
    }
  }

  void aeDDSLoader::LoadTexture2D(aeStreamIn& File)
  {
    m_TexturesMipmaps.clear();
    m_TexturesMipmaps.resize(1);

    Load2DMipmapChain(File, m_TexturesMipmaps[0]);
  }

  void aeDDSLoader::LoadCubemap(aeStreamIn& File)
  {
    m_TexturesMipmaps.clear();
    m_TexturesMipmaps.resize(6);

    Load2DMipmapChain(File, m_TexturesMipmaps[0]);
    Load2DMipmapChain(File, m_TexturesMipmaps[1]);
    Load2DMipmapChain(File, m_TexturesMipmaps[2]);
    Load2DMipmapChain(File, m_TexturesMipmaps[3]);
    Load2DMipmapChain(File, m_TexturesMipmaps[4]);
    Load2DMipmapChain(File, m_TexturesMipmaps[5]);
  }

#ifdef AE_RENDERAPI_OPENGL
  void aeDDSLoader::UploadIntoOpenGLTexture(aeUInt32 iTextureID, aeUInt32& uiWidth, aeUInt32& uiHeight, aeUInt32& uiDepth, aeUInt8 iDropMipmaps, aeUInt32 uiMaxResolution)
  {
    switch (m_TextureType)
    {
      case AE_DDS_TEXTURE2D:
        glBindTexture(GL_TEXTURE_2D, iTextureID);
        UploadTexture2D(iDropMipmaps, uiMaxResolution, uiWidth, uiHeight);
        uiDepth = 1;
        break;
      case AE_DDS_TEXTURE3D:
        glBindTexture(GL_TEXTURE_3D, iTextureID);
        //		UploadTexture3D (iDropMipmaps, uiMaxResolution, uiWidth, uiHeight, uiDepth);
        break;
      case AE_DDS_CUBEMAP:
        glBindTexture(GL_TEXTURE_CUBE_MAP, iTextureID);
        UploadCubemap(iDropMipmaps, uiMaxResolution, uiWidth, uiHeight);
        uiDepth = 1;
        break;
    }
  }
#endif

  void aeDDSLoader::Upload2DSlice(aeInt32 iTexture, unsigned int glTexType, aeUInt8 iDropMipmaps, aeUInt32 uiMaxResolution, aeUInt32& out_uiWidth, aeUInt32& out_uiHeight)
  {
    aeUInt32 uiWidth = m_uiWidth;
    aeUInt32 uiHeight = m_uiHeight;

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    bool bSetDims = false;

    int iMaxMipmapUsed = 0;

    aeInt32 m = 0;
    for (aeInt32 i = 0; i < m_iMipmaps; ++i)
    {
      if (((i >= iDropMipmaps) && (uiWidth <= uiMaxResolution) && (uiHeight <= uiMaxResolution)) || (uiWidth <= 64) || (uiHeight <= 64) || (i == m_iMipmaps - 1))
      {
        iMaxMipmapUsed = m;

        if (!bSetDims)
        {
          bSetDims = true;
          out_uiWidth = uiWidth;
          out_uiHeight = uiHeight;
        }

        switch (m_Compression)
        {
          case AE_COMPR_L8:
            glTexImage2D(glTexType, m, GL_LUMINANCE8, uiWidth, uiHeight, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, &m_TexturesMipmaps[iTexture][i][0]);
            break;

          case AE_COMPR_A8:
            glTexImage2D(glTexType, m, GL_ALPHA8, uiWidth, uiHeight, 0, GL_ALPHA, GL_UNSIGNED_BYTE, &m_TexturesMipmaps[iTexture][i][0]);
            break;

          case AE_COMPR_L8A8:
            glTexImage2D(glTexType, m, GL_LUMINANCE8_ALPHA8, uiWidth, uiHeight, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, &m_TexturesMipmaps[iTexture][i][0]);
            break;

          case AE_COMPR_RGB8:
            glTexImage2D(glTexType, m, GL_RGB8, uiWidth, uiHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, &m_TexturesMipmaps[iTexture][i][0]);
            break;

          case AE_COMPR_ARGB8:
            glTexImage2D(glTexType, m, GL_RGBA8, uiWidth, uiHeight, 0, GL_BGRA, GL_UNSIGNED_BYTE, &m_TexturesMipmaps[iTexture][i][0]);
            break;

          case AE_COMPR_DXT1:
            glCompressedTexImage2D(glTexType, m, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, uiWidth, uiHeight, 0, (GLsizei)m_TexturesMipmaps[iTexture][i].size(), &m_TexturesMipmaps[iTexture][i][0]);
            break;

          case AE_COMPR_DXT1_A:
            glCompressedTexImage2D(glTexType, m, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, uiWidth, uiHeight, 0, (GLsizei)m_TexturesMipmaps[iTexture][i].size(), &m_TexturesMipmaps[iTexture][i][0]);
            break;

          case AE_COMPR_DXT3:
            glCompressedTexImage2D(glTexType, m, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, uiWidth, uiHeight, 0, (GLsizei)m_TexturesMipmaps[iTexture][i].size(), &m_TexturesMipmaps[iTexture][i][0]);
            break;

          case AE_COMPR_DXT5:
            glCompressedTexImage2D(glTexType, m, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, uiWidth, uiHeight, 0, (GLsizei)m_TexturesMipmaps[iTexture][i].size(), &m_TexturesMipmaps[iTexture][i][0]);
            break;

          case AE_COMPR_RGBA16F:
            glTexImage2D(glTexType, m, GL_RGBA16F, uiWidth, uiHeight, 0, GL_RGBA, GL_HALF_FLOAT, &m_TexturesMipmaps[iTexture][i][0]);
            break;
          case AE_COMPR_RGBA32F:
            glTexImage2D(glTexType, m, GL_RGBA32F, uiWidth, uiHeight, 0, GL_RGBA, GL_FLOAT, &m_TexturesMipmaps[iTexture][i][0]);
            break;
        }

        ++m;
      }

      uiWidth /= 2;
      uiHeight /= 2;

      uiWidth = aeMath::Max<aeUInt32>(uiWidth, 1);
      uiHeight = aeMath::Max<aeUInt32>(uiHeight, 1);
    }

    aeRenderAPI::CheckErrors("aeDDSLoader::Upload2DSlice: Before GL_TEXTURE_MAX_LEVEL");

    if ((glTexType == GL_TEXTURE_2D) || (glTexType == GL_TEXTURE_1D))
      glTexParameteri(glTexType, GL_TEXTURE_MAX_LEVEL, iMaxMipmapUsed);

    aeRenderAPI::CheckErrors("aeDDSLoader::Upload2DSlice: After GL_TEXTURE_MAX_LEVEL");

    glPixelStorei(GL_PACK_ALIGNMENT, 4);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  }

  void aeDDSLoader::UploadTexture2D(aeUInt8 iDropMipmaps, aeUInt32 uiMaxResolution, aeUInt32& uiWidth, aeUInt32& uiHeight)
  {
    Upload2DSlice(0, GL_TEXTURE_2D, iDropMipmaps, uiMaxResolution, uiWidth, uiHeight);
  }

  void aeDDSLoader::UploadCubemap(aeUInt8 iDropMipmaps, aeUInt32 uiMaxResolution, aeUInt32& uiWidth, aeUInt32& uiHeight)
  {
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, m_iMipmaps - 1);

    Upload2DSlice(0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, iDropMipmaps, uiMaxResolution, uiWidth, uiHeight);
    Upload2DSlice(1, GL_TEXTURE_CUBE_MAP_NEGATIVE_X, iDropMipmaps, uiMaxResolution, uiWidth, uiHeight);
    Upload2DSlice(2, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, iDropMipmaps, uiMaxResolution, uiWidth, uiHeight);
    Upload2DSlice(3, GL_TEXTURE_CUBE_MAP_POSITIVE_Y, iDropMipmaps, uiMaxResolution, uiWidth, uiHeight);
    Upload2DSlice(4, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, iDropMipmaps, uiMaxResolution, uiWidth, uiHeight);
    Upload2DSlice(5, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, iDropMipmaps, uiMaxResolution, uiWidth, uiHeight);
  }

  void aeDDSLoader::FlipSliceVertical(aeUInt32 uiWidth, aeUInt32 uiHeight, aeUInt32 iBPP, std::vector<aeUInt8>& inout_Data)
  {
    std::vector<aeUInt8> Copy(inout_Data.size());

    const aeUInt32 uiCopyMem = uiWidth * iBPP;

    for (aeUInt32 uiRowOrg = 0; uiRowOrg < uiHeight; ++uiRowOrg)
    {
      const aeUInt32 uiRowCopy = uiHeight - uiRowOrg - 1;

      aeMemory::Copy(&inout_Data[uiRowOrg * uiCopyMem], &Copy[uiRowCopy * uiCopyMem], uiCopyMem);
    }

    inout_Data = Copy;
  }

  void aeDDSLoader::FlipDXTBlockInterpolated(aeUInt8* pData, aeUInt32 uiBlockHeight)
  {
    if (uiBlockHeight <= 1)
      return;

    const aeUInt8 uiBits1 = pData[4];
    const aeUInt8 uiBits2 = pData[5];

    if (uiBlockHeight == 2)
    {
      pData[4] = uiBits2;
      pData[5] = uiBits1;
      return;
    }

    const aeUInt8 uiBits3 = pData[6];
    const aeUInt8 uiBits4 = pData[7];

    pData[4] = uiBits4;
    pData[5] = uiBits3;
    pData[6] = uiBits2;
    pData[7] = uiBits1;
  }

  void aeDDSLoader::FlipDXTBlockInterpolatedAlpha(aeUInt8* pData, aeUInt32 uiBlockHeight)
  {
    if (uiBlockHeight <= 1)
      return;

    const aeUInt32 uiBits1 = pData[2];
    const aeUInt32 uiBits2 = pData[3];
    const aeUInt32 uiBits3 = pData[4];

    if (uiBlockHeight == 2)
    {
      pData[2] = (uiBits2 >> 4) | (uiBits3 << 4);
      pData[3] = (uiBits3 >> 4) | (uiBits1 << 4);
      pData[4] = (uiBits1 >> 4) | (uiBits2 << 4);
      return;
    }

    const aeUInt32 uiBits4 = pData[5];
    const aeUInt32 uiBits5 = pData[6];
    const aeUInt32 uiBits6 = pData[7];

    pData[2] = (uiBits5 >> 4) | (uiBits6 << 4);
    pData[3] = (uiBits6 >> 4) | (uiBits4 << 4);
    pData[4] = (uiBits4 >> 4) | (uiBits5 << 4);
    pData[5] = (uiBits2 >> 4) | (uiBits3 << 4);
    pData[6] = (uiBits3 >> 4) | (uiBits1 << 4);
    pData[7] = (uiBits1 >> 4) | (uiBits2 << 4);
  }

  void aeDDSLoader::FlipDXTBlockExplicit(aeUInt8* pData, aeUInt32 uiBlockHeight)
  {
    if (uiBlockHeight <= 1)
      return;

    const aeUInt8 uiBits1 = pData[0];
    const aeUInt8 uiBits2 = pData[1];
    const aeUInt8 uiBits3 = pData[2];
    const aeUInt8 uiBits4 = pData[3];


    if (uiBlockHeight == 2)
    {
      pData[0] = uiBits3;
      pData[1] = uiBits4;
      pData[2] = uiBits1;
      pData[3] = uiBits2;
      return;
    }

    const aeUInt8 uiBits5 = pData[4];
    const aeUInt8 uiBits6 = pData[5];
    const aeUInt8 uiBits7 = pData[6];
    const aeUInt8 uiBits8 = pData[7];

    pData[0] = uiBits7;
    pData[1] = uiBits8;
    pData[2] = uiBits5;
    pData[3] = uiBits6;
    pData[4] = uiBits3;
    pData[5] = uiBits4;
    pData[6] = uiBits1;
    pData[7] = uiBits2;
  }


  void aeDDSLoader::FlipDXT1(aeUInt32 uiWidth, aeUInt32 uiHeight, std::vector<aeUInt8>& inout_Data)
  {
    if (uiHeight <= 4)
    {
      for (aeUInt32 uiBlock = 0; uiBlock < aeMath::Max<aeUInt32>((uiWidth / 4), 1); ++uiBlock)
      {
        FlipDXTBlockInterpolated(&inout_Data[uiBlock * 8], uiHeight);
      }

      return;
    }

    std::vector<aeUInt8> Copy(inout_Data.size());

    // necessary when textures are higher than they are wide to prevent incorrect flipping on lower mip levels
    uiWidth = aeMath::Max<aeUInt32>(uiWidth, 4);

    const aeUInt32 uiCopyMem = (uiWidth / 4) * 8;

    for (aeUInt32 uiRowOrg = 0; uiRowOrg < (uiHeight / 4); ++uiRowOrg)
    {
      const aeUInt32 uiRowCopy = (uiHeight / 4) - uiRowOrg - 1;

      aeMemory::Copy(&inout_Data[uiRowOrg * uiCopyMem], &Copy[uiRowCopy * uiCopyMem], uiCopyMem);

      for (aeUInt32 uiBlock = 0; uiBlock < (uiWidth / 4); ++uiBlock)
      {
        FlipDXTBlockInterpolated(&Copy[uiRowCopy * uiCopyMem + uiBlock * 8], 4);
      }
    }

    inout_Data = Copy;
  }


  void aeDDSLoader::FlipDXT3(aeUInt32 uiWidth, aeUInt32 uiHeight, std::vector<aeUInt8>& inout_Data)
  {
    if (uiHeight <= 4)
    {
      for (aeUInt32 uiBlock = 0; uiBlock < aeMath::Max<aeUInt32>((uiWidth / 4), 1); ++uiBlock)
      {
        FlipDXTBlockExplicit(&inout_Data[uiBlock * 16], uiHeight);
        FlipDXTBlockInterpolated(&inout_Data[uiBlock * 16 + 8], uiHeight);
      }

      return;
    }

    std::vector<aeUInt8> Copy(inout_Data.size());

    // necessary when textures are higher than they are wide to prevent incorrect flipping on lower mip levels
    uiWidth = aeMath::Max<aeUInt32>(uiWidth, 4);

    const aeUInt32 uiCopyMem = (uiWidth / 4) * 16;

    for (aeUInt32 uiRowOrg = 0; uiRowOrg < (uiHeight / 4); ++uiRowOrg)
    {
      const aeUInt32 uiRowCopy = (uiHeight / 4) - uiRowOrg - 1;

      aeMemory::Copy(&inout_Data[uiRowOrg * uiCopyMem], &Copy[uiRowCopy * uiCopyMem], uiCopyMem);

      for (aeUInt32 uiBlock = 0; uiBlock < (uiWidth / 4); ++uiBlock)
      {
        FlipDXTBlockExplicit(&Copy[uiRowCopy * uiCopyMem + uiBlock * 16], 4);
        FlipDXTBlockInterpolated(&Copy[uiRowCopy * uiCopyMem + uiBlock * 16 + 8], 4);
      }
    }

    inout_Data = Copy;
  }


  void aeDDSLoader::FlipDXT5(aeUInt32 uiWidth, aeUInt32 uiHeight, std::vector<aeUInt8>& inout_Data)
  {
    if (uiHeight <= 4)
    {
      for (aeUInt32 uiBlock = 0; uiBlock < aeMath::Max<aeUInt32>((uiWidth / 4), 1); ++uiBlock)
      {
        FlipDXTBlockInterpolatedAlpha(&inout_Data[uiBlock * 16], uiHeight);
        FlipDXTBlockInterpolated(&inout_Data[uiBlock * 16 + 8], uiHeight);
      }

      return;
    }

    std::vector<aeUInt8> Copy(inout_Data.size());

    // necessary when textures are higher than they are wide to prevent incorrect flipping on lower mip levels
    uiWidth = aeMath::Max<aeUInt32>(uiWidth, 4);

    const aeUInt32 uiCopyMem = (uiWidth / 4) * 16;

    for (aeUInt32 uiRowOrg = 0; uiRowOrg < (uiHeight / 4); ++uiRowOrg)
    {
      const aeUInt32 uiRowCopy = (uiHeight / 4) - uiRowOrg - 1;

      aeMemory::Copy(&inout_Data[uiRowOrg * uiCopyMem], &Copy[uiRowCopy * uiCopyMem], uiCopyMem);

      for (aeUInt32 uiBlock = 0; uiBlock < (uiWidth / 4); ++uiBlock)
      {
        FlipDXTBlockInterpolatedAlpha(&Copy[uiRowCopy * uiCopyMem + uiBlock * 16], 4);
        FlipDXTBlockInterpolated(&Copy[uiRowCopy * uiCopyMem + uiBlock * 16 + 8], 4);
      }
    }

    inout_Data = Copy;
  }
} // namespace AE_NS_GRAPHICS
