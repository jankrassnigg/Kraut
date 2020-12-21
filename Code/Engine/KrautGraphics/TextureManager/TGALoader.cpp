// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#include "TGALoader.h"
#include <stdio.h>

#include <KrautFoundation/FileSystem/FileIn.h>


namespace AE_NS_GRAPHICS
{
	aeTGALoader::aeTGALoader ()
	{
		m_iImageWidth = 0;
		m_iImageHeight = 0;
		m_iBytesPerPixel = 0;
	}

	bool aeTGALoader::LoadTGA (const char* szFile)
	{
		aeFileIn File;
    
    if (!File.Open (szFile, 1024 * 4))
			return (false);

		// Read the header of the TGA
		unsigned char ucHeader[18];
		File.Read (ucHeader, sizeof (unsigned char) * 18);

		// read the number of extra-bytes after the header (application specific data, not of interest for us)
		while (ucHeader[0] > 0)
		{
			--ucHeader[0];

			unsigned char temp;
			File.Read (&temp, sizeof (unsigned char));
		}

		// compute the image dimensions
		m_iImageWidth = ucHeader[13] * 256 + ucHeader[12];
		m_iImageHeight = ucHeader[15] * 256 + ucHeader[14];
		m_iBytesPerPixel = ucHeader[16] / 8;


		// check whether width, height an BitsPerPixel are valid
		if ((m_iImageWidth <= 0) || (m_iImageHeight <= 0) || ((m_iBytesPerPixel != 1) && (m_iBytesPerPixel != 3) && (m_iBytesPerPixel != 4)))
			return (false);

		// allocate the image-buffer
		m_Pixels.resize (m_iImageWidth * m_iImageHeight * 4);


		// call the appropriate loader-routine
		if ((ucHeader[2] == 2) || (ucHeader[2] == 3))
			LoadUncompressedTGA (File);
		else 
		if ((ucHeader[2] == 10) || (ucHeader[2] == 11))
			LoadCompressedTGA (File);
		else
			return (false);

		return (true);
	}

	void aeTGALoader::LoadUncompressedTGA (aeStreamIn& File)
	{
		unsigned char ucBuffer[4] = {255, 255, 255, 255};

		unsigned int* pIntPointer = (unsigned int*) &m_Pixels[0];
		unsigned int* pIntBuffer = (unsigned int*) &ucBuffer[0];

		const int iPixelCount	= m_iImageWidth * m_iImageHeight;

		for (int i = 0; i < iPixelCount; ++i)
		{
			File.Read (ucBuffer, sizeof (unsigned char) * m_iBytesPerPixel);

			// if this is an 8-Bit TGA only, store the one channel in all four channels
			// if it is a 24-Bit TGA (3 channels), the fourth channel stays at 255 all the time, since the 4th value in ucBuffer is never overwritten
			if (m_iBytesPerPixel == 1)
			{
				ucBuffer[1] = ucBuffer[0];
				ucBuffer[2] = ucBuffer[0];
				ucBuffer[3] = ucBuffer[0];
			}

			// copy all four values in one operation
			(*pIntPointer) = (*pIntBuffer);
			++pIntPointer;
		}
	}

	void aeTGALoader::LoadCompressedTGA (aeStreamIn& File)
	{
		int iCurrentPixel	= 0;
		int iCurrentByte	= 0;
		unsigned char ucBuffer[4] = {255, 255, 255, 255};
		const int iPixelCount	= m_iImageWidth * m_iImageHeight;

		unsigned int* pIntPointer = (unsigned int*) &m_Pixels[0];
		unsigned int* pIntBuffer = (unsigned int*) &ucBuffer[0];

		do
		{
			unsigned char ucChunkHeader = 0;

			File.Read (&ucChunkHeader, sizeof (unsigned char));

			if (ucChunkHeader < 128)
			{
				// If the header is < 128, it means it is the number of RAW color packets minus 1
				// that follow the header
				// add 1 to get number of following color values

				ucChunkHeader++;	

				// Read RAW color values
				for (int i = 0; i < (int) ucChunkHeader; ++i)	
				{
					File.Read (&ucBuffer[0], m_iBytesPerPixel);

					// if this is an 8-Bit TGA only, store the one channel in all four channels
					// if it is a 24-Bit TGA (3 channels), the fourth channel stays at 255 all the time, since the 4th value in ucBuffer is never overwritten
					if (m_iBytesPerPixel == 1)
					{
						ucBuffer[1] = ucBuffer[0];
						ucBuffer[2] = ucBuffer[0];
						ucBuffer[3] = ucBuffer[0];
					}

					// copy all four values in one operation
					(*pIntPointer) = (*pIntBuffer);

					++pIntPointer;
					++iCurrentPixel;
				}
			}
			else // chunkheader > 128 RLE data, next color reapeated (chunkheader - 127) times
			{
				ucChunkHeader -= 127;	// Subteact 127 to get rid of the ID bit

				// read the current color
				File.Read (&ucBuffer[0], m_iBytesPerPixel);

				// if this is an 8-Bit TGA only, store the one channel in all four channels
				// if it is a 24-Bit TGA (3 channels), the fourth channel stays at 255 all the time, since the 4th value in ucBuffer is never overwritten
				if (m_iBytesPerPixel == 1)
				{
					ucBuffer[1] = ucBuffer[0];
					ucBuffer[2] = ucBuffer[0];
					ucBuffer[3] = ucBuffer[0];
				}

				// copy the color into the image data as many times as dictated 
				for (int i = 0; i < (int) ucChunkHeader; ++i)
				{
					(*pIntPointer) = (*pIntBuffer);
					++pIntPointer;

					++iCurrentPixel;
				}
			}
		}
		while (iCurrentPixel < iPixelCount);
	}
}



