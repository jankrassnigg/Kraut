// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#include "TextureResource.h"
#include "Declarations.h"
#include "TextureManager.h"

#include "../RenderAPI/RenderAPI.h"
#include "DDSLoader.h"
#include "TGALoader.h"
#include <KrautGraphics/ResourceManager/ResourceManager.h>

#include <KrautGraphics/Configuration/CVar.h>



namespace AE_NS_GRAPHICS
{
  extern aeCVarInt g_CVarTextureQuality;

  AE_RESOURCE_IMPLEMENTATION(aeTextureResource, "Textures/MissingTexture.tga")

  aeUInt32 getFormatSize(AE_TEXTURE_FORMAT Format);

#ifdef AE_RENDERAPI_OPENGL
  GLenum getGLDataType(AE_TEXTURE_FORMAT Format);
  GLenum getGLDataFormat(AE_TEXTURE_FORMAT Format);
#endif

  aeTextureResource::aeTextureResource()
  {
    m_TextureFormat = AE_FORMAT_UNDEFINED;

#ifdef AE_RENDERAPI_OPENGL
    if (g_RenderAPI == AE_RA_OPENGL)
    {
      m_uiTextureIDOGL = 0;
      m_TextureTypeOGL = GL_TEXTURE_2D;
    }
#endif

    m_bHasMipmaps = true;
    m_uiDefaultTextureSamplerID = 0;
  }

  void aeTextureResource::LoadResourceFromFile(const char* szFilename)
  {

    aeHybridString<16> sEXT = aePathFunctions::GetFileExtension(szFilename);
    aeString sFile = aePathFunctions::GetFileName(szFilename);

    const aeUInt32 uiQuality = g_CVarTextureQuality;

    if ((sFile[0] == 'H') && (sFile[1] == 'D') && (sFile[2] == '_'))
      g_CVarTextureQuality = 0;

    aeRenderAPI::CheckErrors("aeTextureResource::LoadResourceFromFile: Before loading Texture");

#ifdef AE_RENDERAPI_OPENGL
    if (g_RenderAPI == AE_RA_OPENGL)
    {
      AE_TEXTURE_FILE_LOADER_OGL pLoader = aeTextureManager::getFileLoaderOGL(sEXT.c_str());

      aeTextureManager::MakeTextureUnitCurrent(15);

      glGenTextures(1, &m_uiTextureIDOGL);
      aeLog::Dev("OpenGL Texture ID: %i", m_uiTextureIDOGL);
      if (!pLoader(szFilename, m_uiTextureIDOGL, m_TextureTypeOGL, m_uiWidth, m_uiHeight, m_uiDepth))
      {
        glDeleteTextures(1, &m_uiTextureIDOGL);
        AE_CHECK_ALWAYS(false, "aeTextureResource::LoadResourceFromFile: Could not load \"%s\"", szFilename);
      }

      if (m_TextureTypeOGL == GL_TEXTURE_CUBE_MAP)
        m_uiDefaultTextureSamplerID = aeTextureManager::getTextureSamplerID("DefaultCubemap");
    }
#endif

    g_CVarTextureQuality = uiQuality;

    aeRenderAPI::CheckErrors("aeTextureResource::LoadResourceFromFile: After loading Texture");
  }

  void aeTextureResource::CreateResourceFromDescriptor(void* pDescriptor)
  {
    aeTextureResourceDescriptor* pTexDesc = (aeTextureResourceDescriptor*)(pDescriptor);
    m_TextureFormat = pTexDesc->m_TextureFormat;

    if (pTexDesc->m_iDefaultTextureSamplerID >= 0)
      m_uiDefaultTextureSamplerID = pTexDesc->m_iDefaultTextureSamplerID;
    else
    {
      if ((pTexDesc->m_TextureType == AE_TYPE_TEXTURE_CUBE) || (pTexDesc->m_TextureType == AE_TYPE_TEXTURE_CUBE_ARRAY))
        m_uiDefaultTextureSamplerID = aeTextureManager::getTextureSamplerID("DefaultCubemap");
      else
        m_uiDefaultTextureSamplerID = 0;
    }

    m_uiWidth = pTexDesc->m_uiWidth;
    m_uiHeight = pTexDesc->m_uiHeight;
    m_uiDepth = pTexDesc->m_uiDepth;

#ifdef AE_RENDERAPI_OPENGL
    if (g_RenderAPI == AE_RA_OPENGL)
    {
      m_TextureTypeOGL = mapTextureTypeOGL(pTexDesc->m_TextureType);

      aeRenderAPI::CheckErrors("aeTextureResource::CreateResourceFromDescriptor 1");

      aeTextureManager::MakeTextureUnitCurrent(15);

      glGenTextures(1, &m_uiTextureIDOGL);
      glBindTexture(m_TextureTypeOGL, m_uiTextureIDOGL);

      aeRenderAPI::CheckErrors("aeTextureResource::CreateResourceFromDescriptor 2");

      glPixelStorei(GL_PACK_ALIGNMENT, 1);
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

      switch (pTexDesc->m_TextureType)
      {
        case AE_TYPE_TEXTURE_2D:
        case AE_TYPE_TEXTURE_RECTANGLE:
          if (isCompressed(pTexDesc->m_TextureFormat))
            glCompressedTexImage2D(m_TextureTypeOGL, 0, mapTextureFormatOGL(pTexDesc->m_TextureFormat), pTexDesc->m_uiWidth, pTexDesc->m_uiHeight, 0, getCompressedSize(pTexDesc->m_TextureFormat, pTexDesc->m_uiWidth, pTexDesc->m_uiHeight), pTexDesc->m_pInitialData);
          else
            glTexImage2D(m_TextureTypeOGL, 0, mapTextureFormatOGL(pTexDesc->m_TextureFormat), pTexDesc->m_uiWidth, pTexDesc->m_uiHeight, 0, getGLDataFormat(pTexDesc->m_TextureFormat), getGLDataType(pTexDesc->m_TextureFormat), pTexDesc->m_pInitialData);
          break;
        case AE_TYPE_TEXTURE_2D_ARRAY:
          if (isCompressed(pTexDesc->m_TextureFormat))
            glCompressedTexImage3D(m_TextureTypeOGL, 0, mapTextureFormatOGL(pTexDesc->m_TextureFormat), pTexDesc->m_uiWidth, pTexDesc->m_uiHeight, pTexDesc->m_uiArraySlices, 0, getCompressedSize(pTexDesc->m_TextureFormat, pTexDesc->m_uiWidth, pTexDesc->m_uiHeight, pTexDesc->m_uiArraySlices), pTexDesc->m_pInitialData);
          else
            glTexImage3D(m_TextureTypeOGL, 0, mapTextureFormatOGL(pTexDesc->m_TextureFormat), pTexDesc->m_uiWidth, pTexDesc->m_uiHeight, pTexDesc->m_uiArraySlices, 0, getGLDataFormat(pTexDesc->m_TextureFormat), getGLDataType(pTexDesc->m_TextureFormat), pTexDesc->m_pInitialData);
          break;
        case AE_TYPE_TEXTURE_3D:
          if (isCompressed(pTexDesc->m_TextureFormat))
            glCompressedTexImage3D(m_TextureTypeOGL, 0, mapTextureFormatOGL(pTexDesc->m_TextureFormat), pTexDesc->m_uiWidth, pTexDesc->m_uiHeight, pTexDesc->m_uiDepth, 0, getCompressedSize(pTexDesc->m_TextureFormat, pTexDesc->m_uiWidth, pTexDesc->m_uiHeight, pTexDesc->m_uiDepth), pTexDesc->m_pInitialData);
          else
            glTexImage3D(m_TextureTypeOGL, 0, mapTextureFormatOGL(pTexDesc->m_TextureFormat), pTexDesc->m_uiWidth, pTexDesc->m_uiHeight, pTexDesc->m_uiDepth, 0, getGLDataFormat(pTexDesc->m_TextureFormat), getGLDataType(pTexDesc->m_TextureFormat), pTexDesc->m_pInitialData);
          break;
        case AE_TYPE_TEXTURE_CUBE:
          glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB, 0, mapTextureFormatOGL(pTexDesc->m_TextureFormat), pTexDesc->m_uiWidth, pTexDesc->m_uiWidth, 0, getGLDataFormat(pTexDesc->m_TextureFormat), getGLDataType(pTexDesc->m_TextureFormat), nullptr);
          glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB, 0, mapTextureFormatOGL(pTexDesc->m_TextureFormat), pTexDesc->m_uiWidth, pTexDesc->m_uiWidth, 0, getGLDataFormat(pTexDesc->m_TextureFormat), getGLDataType(pTexDesc->m_TextureFormat), nullptr);
          glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB, 0, mapTextureFormatOGL(pTexDesc->m_TextureFormat), pTexDesc->m_uiWidth, pTexDesc->m_uiWidth, 0, getGLDataFormat(pTexDesc->m_TextureFormat), getGLDataType(pTexDesc->m_TextureFormat), nullptr);
          glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB, 0, mapTextureFormatOGL(pTexDesc->m_TextureFormat), pTexDesc->m_uiWidth, pTexDesc->m_uiWidth, 0, getGLDataFormat(pTexDesc->m_TextureFormat), getGLDataType(pTexDesc->m_TextureFormat), nullptr);
          glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB, 0, mapTextureFormatOGL(pTexDesc->m_TextureFormat), pTexDesc->m_uiWidth, pTexDesc->m_uiWidth, 0, getGLDataFormat(pTexDesc->m_TextureFormat), getGLDataType(pTexDesc->m_TextureFormat), nullptr);
          glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB, 0, mapTextureFormatOGL(pTexDesc->m_TextureFormat), pTexDesc->m_uiWidth, pTexDesc->m_uiWidth, 0, getGLDataFormat(pTexDesc->m_TextureFormat), getGLDataType(pTexDesc->m_TextureFormat), nullptr);
          break;
        default:
          AE_CHECK_ALWAYS(false, "aeTextureResource::CreateResourceFromDescriptor: The Texture Type %d is currently not supported.", pTexDesc->m_TextureType);
      }

      glPixelStorei(GL_PACK_ALIGNMENT, 4);
      glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

      aeRenderAPI::CheckErrors("aeTextureResource::CreateResourceFromDescriptor 3");

      glTexParameteri(m_TextureTypeOGL, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(m_TextureTypeOGL, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(m_TextureTypeOGL, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

      glTexParameteri(m_TextureTypeOGL, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(m_TextureTypeOGL, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameterf(m_TextureTypeOGL, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1.0f);

      m_bHasMipmaps = false;

      if (pTexDesc->m_bGenerateMipmaps)
      {
        m_bHasMipmaps = true;

        // this is required to get mipmap generation working on ATI cards
        glEnable(m_TextureTypeOGL);
        glTexParameteri(m_TextureTypeOGL, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glGenerateMipmap(m_TextureTypeOGL);
        glTexParameteri(m_TextureTypeOGL, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      }
    }
#endif

    aeRenderAPI::CheckErrors("aeTextureResource::CreateResourceFromDescriptor 5");
  }

  void aeTextureResource::UnloadResource(void)
  {
#ifdef AE_RENDERAPI_OPENGL
    if (g_RenderAPI == AE_RA_OPENGL)
    {
      glDeleteTextures(1, &m_uiTextureIDOGL);
    }
#endif
  }

  void aeTextureResource::GenerateMipmaps(void)
  {
    AE_CHECK_ALWAYS(m_bHasMipmaps, "aeTextureResource::GenerateMipmaps: Cannot generate Mipmaps on a Texture that was created without them.");

#ifdef AE_RENDERAPI_OPENGL
    if (g_RenderAPI == AE_RA_OPENGL)
    {
      aeTextureManager::BindTexture(15, aeTextureResourceHandle(this));
      glGenerateMipmap(m_TextureTypeOGL);
    }
#endif
  }

  void aeTextureResource::ReadbackTexture(aeUInt8* pData)
  {
#ifdef AE_RENDERAPI_OPENGL
    if (g_RenderAPI == AE_RA_OPENGL)
    {
      aeTextureManager::BindTexture(15, aeTextureResourceHandle(this));
      glGetTexImage(m_TextureTypeOGL, 0, getGLDataFormat(m_TextureFormat), getGLDataType(m_TextureFormat), pData);
    }
#endif
  }


} // namespace AE_NS_GRAPHICS
