// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#ifndef AE_GRAPHICS_TEXTUREMANAGER_DECLARATIONSOGL_H
#define AE_GRAPHICS_TEXTUREMANAGER_DECLARATIONSOGL_H

// Include this as the first Header everywhere
#include "../Base.h"

#ifdef AE_RENDERAPI_OPENGL

namespace AE_NS_GRAPHICS
{
	inline GLenum mapCubemapFaceOGL (AE_CUBEMAP_FACE f)
	{
		switch (f)
		{
		case AE_CUBEMAP_NONE:
			return (0);
		case AE_CUBEMAP_POS_X:
			return (GL_TEXTURE_CUBE_MAP_POSITIVE_X);
		case AE_CUBEMAP_NEG_X:
			return (GL_TEXTURE_CUBE_MAP_NEGATIVE_X);
		case AE_CUBEMAP_POS_Y:
			return (GL_TEXTURE_CUBE_MAP_POSITIVE_Y);
		case AE_CUBEMAP_NEG_Y:
			return (GL_TEXTURE_CUBE_MAP_NEGATIVE_Y);
		case AE_CUBEMAP_POS_Z:
			return (GL_TEXTURE_CUBE_MAP_POSITIVE_Z);
		case AE_CUBEMAP_NEG_Z:
			return (GL_TEXTURE_CUBE_MAP_NEGATIVE_Z);
		}

		AE_CHECK_DEV (false, "mapCubemapFaceOGL: Invalid data %d", f);
		return (0);
	}

	inline GLenum mapTextureTypeOGL (AE_TEXTURE_TYPE f)
	{
		switch (f)
		{
		case AE_TYPE_UNDEFINED:
			return (0);
		//case AE_TYPE_TEXTURE_1D:
		//	return (GL_TEXTURE_1D);
		//case AE_TYPE_TEXTURE_1D_ARRAY:
		//	return (GL_TEXTURE_1D_ARRAY);
		case AE_TYPE_TEXTURE_2D:
			return (GL_TEXTURE_2D);
		case AE_TYPE_TEXTURE_2D_ARRAY:
			return (GL_TEXTURE_2D_ARRAY);
		case AE_TYPE_TEXTURE_3D:
			return (GL_TEXTURE_3D);
		case AE_TYPE_TEXTURE_CUBE:
			return (GL_TEXTURE_CUBE_MAP);
		//case AE_TYPE_TEXTURE_CUBE_ARRAY:
		//	return (GL_TEXTURE_CUBE_MAP_ARRAY_ARB);
		//case AE_TYPE_TEXTURE_RECTANGLE:
		//	return (GL_TEXTURE_RECTANGLE);
		}

		AE_CHECK_DEV (false, "mapTextureTypeOGL: Invalid data %d (or unsupported format).", f);
		return (0);
	}

	inline GLenum mapTextureFormatOGL (AE_TEXTURE_FORMAT f)
	{
		switch (f)
		{
		case AE_FORMAT_UNDEFINED:
			return (0);

		case AE_FORMAT_R32G32B32A32_FLOAT:
			return (GL_RGBA32F);
		case AE_FORMAT_R32G32B32_FLOAT:
			return (GL_RGB32F);
		case AE_FORMAT_R32G32_FLOAT:
			return (GL_RG32F);
		case AE_FORMAT_R32_FLOAT:
			return (GL_R32F);

		case AE_FORMAT_R32G32B32A32_UINT:
			return (GL_RGBA32UI);
		case AE_FORMAT_R32G32B32_UINT:
			return (GL_RGB32UI);
		case AE_FORMAT_R32G32_UINT:
			return (GL_RG32UI);
		case AE_FORMAT_R32_UINT:
			return (GL_R32UI);

		case AE_FORMAT_R32G32B32A32_SINT:
			return (GL_RGBA32I);
		case AE_FORMAT_R32G32B32_SINT:
			return (GL_RGB32I);
		case AE_FORMAT_R32G32_SINT:
			return (GL_RG32I);
		case AE_FORMAT_R32_SINT:
			return (GL_R32I);

		case AE_FORMAT_R16G16B16A16_FLOAT:
			return (GL_RGBA16F);
		case AE_FORMAT_R16G16_FLOAT:
			return (GL_RG16F);
		case AE_FORMAT_R16_FLOAT:
			return (GL_R16F);

		case AE_FORMAT_R16G16B16A16_UINT:
			return (GL_RGBA16UI);
		case AE_FORMAT_R16G16_UINT:
			return (GL_RG16UI);
		case AE_FORMAT_R16_UINT:
			return (GL_R16UI);

		case AE_FORMAT_R16G16B16A16_SINT:
			return (GL_RGBA16I);
		case AE_FORMAT_R16G16_SINT:
			return (GL_RG16I);
		case AE_FORMAT_R16_SINT:
			return (GL_R16I);

		case AE_FORMAT_R16G16B16A16_UNORM:
			return (GL_RGBA16);
		case AE_FORMAT_R16G16_UNORM:
			return (GL_RG16);
		case AE_FORMAT_R16_UNORM:
			return (GL_R16);

		case AE_FORMAT_R8G8B8A8_UNORM:
			return (GL_RGBA8);
		case AE_FORMAT_R8G8B8A8_UINT:
			return (GL_RGBA8UI);
		case AE_FORMAT_R8G8B8A8_SINT:
			return (GL_RGBA8I);

		case AE_FORMAT_R8G8_UNORM:
			return (GL_RG8);
		case AE_FORMAT_R8G8_UINT:
			return (GL_RG8UI);
		case AE_FORMAT_R8G8_SINT:
			return (GL_RG8I);

		case AE_FORMAT_R8_UNORM:
			return (GL_R8);
		case AE_FORMAT_R8_UINT:
			return (GL_R8UI);
		case AE_FORMAT_R8_SINT:
			return (GL_R8I);

		case AE_FORMAT_R11G11B10_FLOAT:
			return (GL_R11F_G11F_B10F);

		case AE_FORMAT_DEPTH24_STENCIL8:
			return (GL_DEPTH_COMPONENT24);
		case AE_FORMAT_DEPTH32:
			return (GL_DEPTH_COMPONENT32F);

		case AE_FORMAT_SHADOW24:
			return (GL_DEPTH_COMPONENT24);
		case AE_FORMAT_SHADOW32:
			return (GL_DEPTH_COMPONENT32F);

		case AE_FORMAT_RGBE:
			return (GL_RGB9_E5);

		case AE_FORMAT_COMPRESSED_RGB:
			return (GL_COMPRESSED_RGBA_S3TC_DXT3_EXT);
		case AE_FORMAT_COMPRESSED_RGBA:
			return (GL_COMPRESSED_RGBA_S3TC_DXT5_EXT);
		}

		AE_CHECK_DEV (false, "mapTextureFormatOGL: Invalid data %d (or unsupported format).", f);
		return (0);
	}

	inline aeInt32 getCompressedSize (AE_TEXTURE_FORMAT f, aeUInt32 uiWidth, aeUInt32 uiHeight, aeUInt32 uiDepth = 1)
	{
		switch (f)
		{
		case AE_FORMAT_COMPRESSED_RGB:
			return (uiWidth * uiHeight * uiDepth * 4 / 8);
		case AE_FORMAT_COMPRESSED_RGBA:
			return (uiWidth * uiHeight * uiDepth * 4 / 4);
		}

		AE_CHECK_DEV (false, "getCompressedSize: Invalid data %d (or unsupported format).", f);
		return (0);
	}

	typedef bool (*AE_TEXTURE_FILE_LOADER_OGL) (const char* szFile, aeUInt32 uiOpenGLTextureID, aeUInt32& uiOpenGLTextureType, aeUInt32& uiWidth, aeUInt32& uiHeight, aeUInt32& uiDepth);
}

#endif

#endif


