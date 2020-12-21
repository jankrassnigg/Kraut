// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#ifndef AE_GRAPHICS_TEXTUREMANAGER_DECLARATIONS_H
#define AE_GRAPHICS_TEXTUREMANAGER_DECLARATIONS_H

// Include this as the first Header everywhere
#include "../Base.h"

#include <KrautFoundation/Math/Declarations.h>
#include <KrautFoundation/Logging/Log.h>
#include <KrautFoundation/Strings/Declarations.h>

#include <KrautFoundation/ResourceManager/TypedResourceHandle.h>
#include "../RenderAPI/Declarations.h"
#include <KrautFoundation/Strings/PathFunctions.h>

namespace AE_NS_GRAPHICS
{
  using namespace AE_NS_GRAPHICS;
  using namespace AE_NS_GRAPHICS;

	class aeTextureManager;
	class aeTextureResource;
	class aeTextureManagerStartup;
	struct aeSamplerState;
  struct aeTextureResourceDescriptor;

	enum AE_TEXTURE_FILTER	
	{
		// absolute texture filters
		AE_FILTER_NEAREST,
		AE_FILTER_BILINEAR,
		AE_FILTER_TRILINEAR,
		AE_FILTER_ANISOTROPIC_2X,
		AE_FILTER_ANISOTROPIC_4X,
		AE_FILTER_ANISOTROPIC_8X,
		AE_FILTER_ANISOTROPIC_16X,

		// relative texture filters, "default" usually equals anisotropic_2x
		AE_FILTER_DEFAULT_MINUS_TWO,
		AE_FILTER_DEFAULT_MINUS_ONE,
		AE_FILTER_DEFAULT,
		AE_FILTER_DEFAULT_PLUS_ONE,
		AE_FILTER_DEFAULT_PLUS_TWO,
	};	

	enum AE_TEXTURE_WRAP
	{
		AE_WRAP_REPEAT,
		AE_WRAP_CLAMP_X,
		AE_WRAP_CLAMP_Y,
		AE_WRAP_CLAMP,
	};

	enum AE_TEXTURE_COMPARE
	{
		AE_COMPARISON_DISABLED,
		AE_COMPARISON_LESS,
		AE_COMPARISON_EQUAL,
		AE_COMPARISON_LEQUAL,
		AE_COMPARISON_GREATER,
		AE_COMPARISON_NOTEQUAL,
		AE_COMPARISON_GEQUAL,
		AE_COMPARISON_ALWAYS,
	};

	enum AE_CUBEMAP_FACE
	{
		AE_CUBEMAP_NONE,
		AE_CUBEMAP_POS_X,
		AE_CUBEMAP_NEG_X,
		AE_CUBEMAP_POS_Y,
		AE_CUBEMAP_NEG_Y,
		AE_CUBEMAP_POS_Z,
		AE_CUBEMAP_NEG_Z,
	};

	enum AE_TEXTURE_TYPE
	{
		AE_TYPE_UNDEFINED,
		//AE_TYPE_TEXTURE_1D,
		//AE_TYPE_TEXTURE_1D_ARRAY,
		AE_TYPE_TEXTURE_2D,
		AE_TYPE_TEXTURE_2D_ARRAY,
		AE_TYPE_TEXTURE_3D,
		AE_TYPE_TEXTURE_CUBE,
		AE_TYPE_TEXTURE_CUBE_ARRAY,
		AE_TYPE_TEXTURE_RECTANGLE,
	};

	enum AE_TEXTURE_FORMAT
	{
		AE_FORMAT_UNDEFINED,
		AE_FORMAT_R32G32B32A32_FLOAT,
		AE_FORMAT_R32G32B32_FLOAT,
		AE_FORMAT_R32G32_FLOAT,
		AE_FORMAT_R32_FLOAT,

		AE_FORMAT_R32G32B32A32_UINT,
		AE_FORMAT_R32G32B32_UINT,
		AE_FORMAT_R32G32_UINT,
		AE_FORMAT_R32_UINT,

		AE_FORMAT_R32G32B32A32_SINT,
		AE_FORMAT_R32G32B32_SINT,
		AE_FORMAT_R32G32_SINT,
		AE_FORMAT_R32_SINT,

		AE_FORMAT_R16G16B16A16_FLOAT,
		AE_FORMAT_R16G16_FLOAT,
		AE_FORMAT_R16_FLOAT,

		AE_FORMAT_R16G16B16A16_UINT,
		AE_FORMAT_R16G16_UINT,
		AE_FORMAT_R16_UINT,

		AE_FORMAT_R16G16B16A16_SINT,
		AE_FORMAT_R16G16_SINT,
		AE_FORMAT_R16_SINT,

		AE_FORMAT_R16G16B16A16_UNORM,
		AE_FORMAT_R16G16_UNORM,
		AE_FORMAT_R16_UNORM,

		AE_FORMAT_R8G8B8A8_UNORM,
		AE_FORMAT_R8G8B8A8_UINT,
		AE_FORMAT_R8G8B8A8_SINT,

		AE_FORMAT_R8G8_UNORM,
		AE_FORMAT_R8G8_UINT,
		AE_FORMAT_R8G8_SINT,

		AE_FORMAT_R8_UNORM,
		AE_FORMAT_R8_UINT,
		AE_FORMAT_R8_SINT,

		AE_FORMAT_R11G11B10_FLOAT,

		AE_FORMAT_DEPTH24_STENCIL8,
		AE_FORMAT_DEPTH32,

		AE_FORMAT_SHADOW24,
		AE_FORMAT_SHADOW32,

		AE_FORMAT_RGBE,

		AE_FORMAT_COMPRESSED_RGB,
		AE_FORMAT_COMPRESSED_RGBA,
	};

	// typedef for a Handle to a TextureResource
	typedef aeTypedResourceHandle<aeTextureResource> aeTextureResourceHandle;

	inline bool isCompressed (AE_TEXTURE_FORMAT f)
	{
		switch (f)
		{
		case AE_FORMAT_COMPRESSED_RGB:
			return (true);
		case AE_FORMAT_COMPRESSED_RGBA:
			return (true);
		}

		return (false);
	}

	// Writes an image as a TGA to a file. pData needs to be of size Width*Height*BPP and be in RGB(A) order. BPP can be 3 or 4.
	AE_GRAPHICS_DLL bool ExportTGA (const char* szFile, int iWidth, int iHeight, int iBytesPerPixel, aeUInt8* pData, bool bCompress);
	//void ExportRGBE (const char* szFile, int iWidth, int iHeight, float* pData, bool bCompress);
	void TestRGBE (float fRed, float fGreen, float fBlue);
}

#include "DeclarationsOGL.h"
#include "DeclarationsD3D11.h"

#endif


