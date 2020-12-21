// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#ifndef TEXTUREMANAGER3_DDSLOADER_H
#define TEXTUREMANAGER3_DDSLOADER_H

#include "Declarations.h"

#include <vector>
#include <deque>

namespace AE_NS_GRAPHICS
{
	enum aeDDSType
	{
		AE_DDS_NONE,
		AE_DDS_TEXTURE2D,
		AE_DDS_TEXTURE3D,
		AE_DDS_CUBEMAP,
	};


	enum aeDSSCompression
	{
		AE_COMPR_NONE,
		AE_COMPR_RGB8,
		AE_COMPR_ARGB8,
		AE_COMPR_L8A8,
		AE_COMPR_L8,
		AE_COMPR_A8,
		AE_COMPR_DXT1,
		AE_COMPR_DXT1_A,
		AE_COMPR_DXT3,
		AE_COMPR_DXT5,

		AE_COMPR_RGBA16F,
		AE_COMPR_RGBA32F,
	};

	class aeDDSLoader
	{
	public:
		aeDDSLoader ();

		bool LoadDDS (const char* szFile, bool bLoadForD3D);

		void PrintDDSInformation (const char* szFile);

#ifdef AE_RENDERAPI_OPENGL
		void UploadIntoOpenGLTexture (aeUInt32 iTextureID, aeUInt32& uiWidth, aeUInt32& uiHeight, aeUInt32& uiDepth, aeUInt8 iDropMipmaps = 0, aeUInt32 uiMaxResolution = 8192);
#endif

		aeUInt32 getWidth (void) const {return (m_uiWidth);}
		aeUInt32 getHeight (void) const {return (m_uiHeight);}

		aeDDSType getTextureType (void) const {return (m_TextureType);}

	private:

		void LoadSlice (aeStreamIn& File, aeUInt32 uiWidth, aeUInt32 uiHeight, aeDSSCompression Compression, std::vector<aeUInt8>& out_Data);
		void Load2DMipmapChain (aeStreamIn& File, std::deque<std::vector<aeUInt8> >& out_Data);

		void FlipSliceVertical (aeUInt32 uiWidth, aeUInt32 uiHeight, aeUInt32 iBPP, std::vector<aeUInt8>& inout_Data);

		void FlipDXTBlockInterpolated (aeUInt8* pData, aeUInt32 uiBlockHeight);
		void FlipDXTBlockInterpolatedAlpha (aeUInt8* pData, aeUInt32 uiBlockHeight);
		void FlipDXTBlockExplicit (aeUInt8* pData, aeUInt32 uiBlockHeight);

		void FlipDXT1 (aeUInt32 uiWidth, aeUInt32 uiHeight, std::vector<aeUInt8>& inout_Data);
		void FlipDXT3 (aeUInt32 uiWidth, aeUInt32 uiHeight, std::vector<aeUInt8>& inout_Data);
		void FlipDXT5 (aeUInt32 uiWidth, aeUInt32 uiHeight, std::vector<aeUInt8>& inout_Data);

		void LoadTexture2D (aeStreamIn& File);
		void LoadCubemap (aeStreamIn& File);

		void Upload2DSlice (aeInt32 iTexture, unsigned int glTexType, aeUInt8 iDropMipmaps, aeUInt32 uiMaxResolution, aeUInt32& uiWidth, aeUInt32& uiHeight);

		void UploadTexture2D (aeUInt8 iDropMipmaps, aeUInt32 uiMaxResolution, aeUInt32& uiWidth, aeUInt32& uiHeight);
		void UploadCubemap (aeUInt8 iDropMipmaps, aeUInt32 uiMaxResolution, aeUInt32& uiWidth, aeUInt32& uiHeight);

		bool m_bLoadForD3D;
		aeDDSType m_TextureType;
		aeDSSCompression m_Compression;
		aeUInt32 m_uiWidth;
		aeUInt32 m_uiHeight;
		aeUInt32 m_uiDepth;
		aeInt32 m_iMipmaps;

    std::deque< std::deque< std::vector <aeUInt8> > > m_TexturesMipmaps;
	};
}

#endif

