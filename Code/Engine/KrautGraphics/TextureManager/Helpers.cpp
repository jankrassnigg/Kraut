// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#include "TextureResource.h"
#include "TextureManager.h"

#include "DDSLoader.h"
#include "TGALoader.h"



namespace AE_NS_GRAPHICS
{
#ifdef AE_RENDERAPI_OPENGL
	GLenum getGLDataType (AE_TEXTURE_FORMAT Format)
	{
		switch (Format)
		{
		case AE_FORMAT_R32G32B32A32_FLOAT:
			return (GL_FLOAT);
		case AE_FORMAT_R32G32B32_FLOAT:
			return (GL_FLOAT);
		case AE_FORMAT_R32G32_FLOAT:		
			return (GL_FLOAT);
		case AE_FORMAT_R32_FLOAT:			
			return (GL_FLOAT);

		case AE_FORMAT_R32G32B32A32_UINT:	
			return (GL_UNSIGNED_INT);
		case AE_FORMAT_R32G32B32_UINT:	
			return (GL_UNSIGNED_INT);
		case AE_FORMAT_R32G32_UINT:		
			return (GL_UNSIGNED_INT);
		case AE_FORMAT_R32_UINT:			
			return (GL_UNSIGNED_INT);

		case AE_FORMAT_R32G32B32A32_SINT:	
			return (GL_INT);
		case AE_FORMAT_R32G32B32_SINT:	
			return (GL_INT);
		case AE_FORMAT_R32G32_SINT:		
			return (GL_INT);
		case AE_FORMAT_R32_SINT:			
			return (GL_INT);

		case AE_FORMAT_R16G16B16A16_FLOAT:
			return (GL_FLOAT);
		case AE_FORMAT_R16G16_FLOAT:		
			return (GL_FLOAT);
		case AE_FORMAT_R16_FLOAT:			
			return (GL_FLOAT);

		case AE_FORMAT_R16G16B16A16_UINT:	
			return (GL_UNSIGNED_SHORT);
		case AE_FORMAT_R16G16_UINT:		
			return (GL_UNSIGNED_SHORT);
		case AE_FORMAT_R16_UINT:			
			return (GL_UNSIGNED_SHORT);

		case AE_FORMAT_R16G16B16A16_SINT:	
			return (GL_SHORT);
		case AE_FORMAT_R16G16_SINT:		
			return (GL_SHORT);
		case AE_FORMAT_R16_SINT:			
			return (GL_SHORT);

		case AE_FORMAT_R16G16B16A16_UNORM:
			return (GL_UNSIGNED_SHORT);
		case AE_FORMAT_R16G16_UNORM:		
			return (GL_UNSIGNED_SHORT);
		case AE_FORMAT_R16_UNORM:			
			return (GL_UNSIGNED_SHORT);

		case AE_FORMAT_R8G8B8A8_UNORM:	
			return (GL_UNSIGNED_BYTE);
		case AE_FORMAT_R8G8B8A8_UINT:		
			return (GL_UNSIGNED_BYTE);
		case AE_FORMAT_R8G8B8A8_SINT:		
			return (GL_BYTE);

		case AE_FORMAT_R8G8_UNORM:		
			return (GL_UNSIGNED_BYTE);
		case AE_FORMAT_R8G8_UINT:			
			return (GL_UNSIGNED_BYTE);
		case AE_FORMAT_R8G8_SINT:			
			return (GL_BYTE);

		case AE_FORMAT_R8_UNORM:			
			return (GL_UNSIGNED_BYTE);
		case AE_FORMAT_R8_UINT:			
			return (GL_UNSIGNED_BYTE);
		case AE_FORMAT_R8_SINT:			
			return (GL_BYTE);

		case AE_FORMAT_R11G11B10_FLOAT:	
			return (GL_FLOAT);

		case AE_FORMAT_DEPTH32:			
			return (GL_FLOAT);
		case AE_FORMAT_DEPTH24_STENCIL8:	
			return (GL_FLOAT);

		case AE_FORMAT_SHADOW24:
			return (GL_FLOAT);
		case AE_FORMAT_SHADOW32:
			return (GL_FLOAT);

		case AE_FORMAT_RGBE:
			return (GL_UNSIGNED_INT_5_9_9_9_REV);
		}

		AE_CHECK_ALWAYS (false, "getGLDataType: Unknown Format %d", Format);
    return 0;
	}

	GLenum getGLDataFormat (AE_TEXTURE_FORMAT Format)
	{
		switch (Format)
		{
		case AE_FORMAT_R32G32B32A32_FLOAT:
			return (GL_RGBA);
		case AE_FORMAT_R32G32B32_FLOAT:
			return (GL_RGB);
		case AE_FORMAT_R32G32_FLOAT:		
			return (GL_RG);
		case AE_FORMAT_R32_FLOAT:			
			return (GL_RED);

		case AE_FORMAT_R32G32B32A32_UINT:	
			return (GL_RGBA_INTEGER);
		case AE_FORMAT_R32G32B32_UINT:	
			return (GL_RGB_INTEGER);
		case AE_FORMAT_R32G32_UINT:		
			return (GL_RG_INTEGER);
		case AE_FORMAT_R32_UINT:			
			return (GL_RED_INTEGER);

		case AE_FORMAT_R32G32B32A32_SINT:	
			return (GL_RGBA_INTEGER);
		case AE_FORMAT_R32G32B32_SINT:	
			return (GL_RGB_INTEGER);
		case AE_FORMAT_R32G32_SINT:		
			return (GL_RG_INTEGER);
		case AE_FORMAT_R32_SINT:			
			return (GL_RED_INTEGER);

		case AE_FORMAT_R16G16B16A16_FLOAT:
			return (GL_RGBA);
		case AE_FORMAT_R16G16_FLOAT:		
			return (GL_RG);
		case AE_FORMAT_R16_FLOAT:			
			return (GL_RED);

		case AE_FORMAT_R16G16B16A16_UINT:	
			return (GL_RGBA_INTEGER);
		case AE_FORMAT_R16G16_UINT:		
			return (GL_RG_INTEGER);
		case AE_FORMAT_R16_UINT:			
			return (GL_RED_INTEGER);

		case AE_FORMAT_R16G16B16A16_SINT:	
			return (GL_RGBA_INTEGER);
		case AE_FORMAT_R16G16_SINT:		
			return (GL_RG_INTEGER);
		case AE_FORMAT_R16_SINT:			
			return (GL_RED_INTEGER);

		case AE_FORMAT_R16G16B16A16_UNORM:
			return (GL_RGBA);
		case AE_FORMAT_R16G16_UNORM:		
			return (GL_RG);
		case AE_FORMAT_R16_UNORM:			
			return (GL_RED);

		case AE_FORMAT_R8G8B8A8_UNORM:	
			return (GL_RGBA);
		case AE_FORMAT_R8G8B8A8_UINT:		
			return (GL_RGBA_INTEGER);
		case AE_FORMAT_R8G8B8A8_SINT:		
			return (GL_RGBA_INTEGER);

		case AE_FORMAT_R8G8_UNORM:		
			return (GL_RG);
		case AE_FORMAT_R8G8_UINT:			
			return (GL_RG_INTEGER);
		case AE_FORMAT_R8G8_SINT:			
			return (GL_RG_INTEGER);

		case AE_FORMAT_R8_UNORM:			
			return (GL_RED);
		case AE_FORMAT_R8_UINT:			
			return (GL_RED_INTEGER);
		case AE_FORMAT_R8_SINT:			
			return (GL_RED_INTEGER);

		case AE_FORMAT_R11G11B10_FLOAT:	
			return (GL_RGB);

		case AE_FORMAT_DEPTH32:			
			return (GL_DEPTH_COMPONENT);
		case AE_FORMAT_DEPTH24_STENCIL8:	
			return (GL_DEPTH_COMPONENT);

		case AE_FORMAT_SHADOW24:
			return (GL_DEPTH_COMPONENT);
		case AE_FORMAT_SHADOW32:
			return (GL_DEPTH_COMPONENT);

		case AE_FORMAT_RGBE:
			return (GL_RGB);
		}

		AE_CHECK_ALWAYS (false, "getGLDataFormat: Unknown Format %d", Format);
    return 0;
	}
#endif


	aeUInt32 getFormatSize (AE_TEXTURE_FORMAT Format)
	{
		switch (Format)
		{
		case AE_FORMAT_R32G32B32A32_FLOAT:
			return (4 * 4);
		case AE_FORMAT_R32G32B32_FLOAT:
			return (4 * 3);
		case AE_FORMAT_R32G32_FLOAT:		
			return (4 * 2);
		case AE_FORMAT_R32_FLOAT:			
			return (4 * 1);

		case AE_FORMAT_R32G32B32A32_UINT:	
			return (4 * 4);
		case AE_FORMAT_R32G32B32_UINT:	
			return (4 * 3);
		case AE_FORMAT_R32G32_UINT:		
			return (4 * 2);
		case AE_FORMAT_R32_UINT:			
			return (4 * 1);

		case AE_FORMAT_R32G32B32A32_SINT:	
			return (4 * 4);
		case AE_FORMAT_R32G32B32_SINT:	
			return (4 * 3);
		case AE_FORMAT_R32G32_SINT:		
			return (4 * 2);
		case AE_FORMAT_R32_SINT:			
			return (4 * 1);

		case AE_FORMAT_R16G16B16A16_FLOAT:
			return (2 * 4);
		case AE_FORMAT_R16G16_FLOAT:		
			return (2 * 2);
		case AE_FORMAT_R16_FLOAT:			
			return (2 * 1);

		case AE_FORMAT_R16G16B16A16_UINT:	
			return (2 * 4);
		case AE_FORMAT_R16G16_UINT:		
			return (2 * 2);
		case AE_FORMAT_R16_UINT:			
			return (2 * 1);

		case AE_FORMAT_R16G16B16A16_SINT:	
			return (2 * 4);
		case AE_FORMAT_R16G16_SINT:		
			return (2 * 2);
		case AE_FORMAT_R16_SINT:			
			return (2 * 1);

		case AE_FORMAT_R16G16B16A16_UNORM:
			return (2 * 4);
		case AE_FORMAT_R16G16_UNORM:		
			return (2 * 2);
		case AE_FORMAT_R16_UNORM:			
			return (2 * 1);

		case AE_FORMAT_R8G8B8A8_UNORM:	
			return (4);
		case AE_FORMAT_R8G8B8A8_UINT:		
			return (4);
		case AE_FORMAT_R8G8B8A8_SINT:		
			return (4);

		case AE_FORMAT_R8G8_UNORM:		
			return (2);
		case AE_FORMAT_R8G8_UINT:			
			return (2);
		case AE_FORMAT_R8G8_SINT:			
			return (2);

		case AE_FORMAT_R8_UNORM:			
			return (1);
		case AE_FORMAT_R8_UINT:			
			return (1);
		case AE_FORMAT_R8_SINT:			
			return (1);

		case AE_FORMAT_R11G11B10_FLOAT:	
			return (4);

		case AE_FORMAT_DEPTH32:			
			return (4);
		case AE_FORMAT_DEPTH24_STENCIL8:	
			return (4);

		case AE_FORMAT_SHADOW24:
			return (4);
		case AE_FORMAT_SHADOW32:
			return (4);

		case AE_FORMAT_RGBE:
			return (4);

		case AE_FORMAT_COMPRESSED_RGB:
			return (4);
		case AE_FORMAT_COMPRESSED_RGBA:
			return (4);
		}

		AE_CHECK_ALWAYS (false, "getFormatSize: Unknown Format %d", Format);
    return 0;
	}
}