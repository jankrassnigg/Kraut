// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#ifndef AE_GRAPHICS_TEXTUREMANAGER_TGALOADER_H
#define AE_GRAPHICS_TEXTUREMANAGER_TGALOADER_H

#include "Declarations.h"
#include <vector>

namespace AE_NS_GRAPHICS
{
	//! Small class to load a TGA-file into a memory buffer.
	class aeTGALoader
	{
	public:
		aeTGALoader ();

		//! Loads a TGA. Can be 8, 24 or 32 Bits per pixel, uncompressed or (RLE) compressed. Returns false, if the TGA could not be loaded.
		bool LoadTGA (const char* szFile);

		//! Returns the width of the image in pixels
		int getWidth (void) const {return (m_iImageWidth);}
		//! Returns the height of the image in pixels
		int getHeight (void) const {return (m_iImageHeight);}

		//! Returns the pixel at location (x,y). 
		/*! ATTENTION:
			"getPixel (x,y)[0]" is BLUE
			"getPixel (x,y)[1]" is GREEN
			"getPixel (x,y)[2]" is RED
			"getPixel (x,y)[3]" is ALPHA
			This allows faster load-times (TGAs are stored this way natively) and faster upload-times to OpenGL.
			Use GL_BGRA as "format", when uploading the data to OpenGL.
		*/
		const unsigned char* getPixel (int x, int y) const {return (&m_Pixels[(y * m_iImageWidth + x) *4]);}

		//! Returns the raw array, as needed when uploading the image to OpenGL.
		/*! Upload the image to OpenGL like this:
			aeTGALoader Image;
			gluBuild2DMipmaps (GL_TEXTURE_2D, 4, Image.getWidth (), Image.getHeight (), GL_BGRA, GL_UNSIGNED_BYTE, Image.getDataForOpenGL  ());
		*/
		const unsigned char* getDataForOpenGL (void) const {return (&m_Pixels[0]);}

	private:
		void LoadCompressedTGA (aeStreamIn& File);
		void LoadUncompressedTGA (aeStreamIn& File);

    std::vector<unsigned char> m_Pixels;

		int m_iImageWidth;
		int m_iImageHeight;
		int m_iBytesPerPixel;
	};



}

#endif