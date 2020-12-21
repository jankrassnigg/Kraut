// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#include "Main.h"
#include <KrautFoundation/FileSystem/FileOut.h>


namespace AE_NS_GRAPHICS
{
	struct TGAColor
	{
		TGAColor () : r (0), g (0), b (0), a (0) {}

		unsigned char r;
		unsigned char g;
		unsigned char b;
		unsigned char a;

		bool operator== (const TGAColor c) const
		{
			return ((r == c.r) && (g == c.g) && (b == c.b) && (a == c.a));
		}

		bool operator!= (const TGAColor c) const
		{
			return ((r != c.r) || (g != c.g) && (b == c.b) && (a == c.a));
		}
	};

	bool ExportTGA (const char* szFile, int iWidth, int iHeight, int iBytesPerPixel, aeUInt8* pData, bool bCompress)
	{
    aeFilePath sPath = aePathFunctions::ChangeExistingFileExtension (szFile, "tga");

		aeFileOut File;
    if (!File.Open (sPath.c_str (), 128))
      return false;

		unsigned char ucHeader[18];
		for (int i = 0; i < 18; ++i)
			ucHeader[i] = 0;

		if (!bCompress)
		{
			// uncompressed TGA
			ucHeader[2] = 2;
		}
		else
		{
			// compressed TGA
			ucHeader[2] = 10;
		}

		ucHeader[13] = iWidth / 256;
		ucHeader[15] = iHeight/ 256;
		ucHeader[12] = iWidth % 256;
		ucHeader[14] = iHeight% 256;
		ucHeader[16] = 8 * iBytesPerPixel;

		File.Write (ucHeader, sizeof (char) * 18);

		if (!bCompress)
		{
			for (int y = 0; y < iHeight; ++y)
			{
				for (int x = 0; x < iWidth; ++x)
				{
					TGAColor c;
					c.r = pData[(y * iWidth + x) * iBytesPerPixel + 0];
					c.g = pData[(y * iWidth + x) * iBytesPerPixel + 1];
					c.b = pData[(y * iWidth + x) * iBytesPerPixel + 2];

					File << c.b;
					File << c.g;
					File << c.r;

					if (iBytesPerPixel == 4)
					{
						c.a = pData[(y * iWidth + x) * iBytesPerPixel + 3];
						File << c.a;
					}
				}
			}
		}
		else
		{
			int iRLE = 0;
			TGAColor pc;
      std::vector<TGAColor> unequal;
			int iEqual = 0;

			for (int y = 0; y < iHeight; ++y)
			{
				for (int x = 0; x < iWidth; ++x)
				{
					TGAColor c;
					c.r = pData[(y * iWidth + x) * iBytesPerPixel + 0];
					c.g = pData[(y * iWidth + x) * iBytesPerPixel + 1];
					c.b = pData[(y * iWidth + x) * iBytesPerPixel + 2];

					if (iBytesPerPixel == 4)
						c.a = pData[(y * iWidth + x) * iBytesPerPixel + 3];

					if (iRLE == 0) // no comparison possible yet
					{
						pc = c;
						iRLE = 1;
						unequal.push_back (c);
					}
					else
					if (iRLE == 1) // has one value gathered for comparison
					{
						if (c == pc)
						{
							iRLE = 2; // two values were equal
							iEqual = 2; // go into equal-mode
						}
						else
						{
							iRLE = 3; // two values were unequal
							pc = c; // go into unequal-mode
							unequal.push_back (c);
						}
					}
					else
					if (iRLE == 2) // equal values
					{
						if ((c == pc) && (iEqual < 128))
							++iEqual;
						else
						{
							unsigned char ucRepeat = iEqual + 127;

							File << ucRepeat;
							File << pc.b;
							File << pc.g;
							File << pc.r;

							if (iBytesPerPixel == 4)
								File << pc.a;

							pc = c;
							iRLE = 1;
							unequal.clear ();
							unequal.push_back (c);
						}
					}
					else
					if (iRLE == 3)
					{
						if ((c != pc) && (unequal.size () < 128))
						{
							unequal.push_back (c);
							pc = c;
						}
						else
						{
							unsigned char ucRepeat = (unsigned char) (unequal.size ()) - 1;
							File << ucRepeat;

							for (aeUInt32 i = 0; i < unequal.size (); ++i)
							{
								File << unequal[i].b;
								File << unequal[i].g;
								File << unequal[i].r;

								if (iBytesPerPixel == 4)
									File << unequal[i].a;
							}

							pc = c;
							iRLE = 1;
							unequal.clear ();
							unequal.push_back (c);
						}
					}
				}
			}


			if (iRLE == 1) // has one value gathered for comparison
			{
				unsigned char ucRepeat = 0;

				File << ucRepeat;
				File << pc.b;
				File << pc.g;
				File << pc.r;

				if (iBytesPerPixel == 4)
					File << pc.a;
			}
			else
			if (iRLE == 2) // equal values
			{
				unsigned char ucRepeat = iEqual + 127;

				File << ucRepeat;
				File << pc.b;
				File << pc.g;
				File << pc.r;

				if (iBytesPerPixel == 4)
					File << pc.a;
			}
			else
			if (iRLE == 3)
			{
				unsigned char ucRepeat = (unsigned char) (unequal.size ()) - 1;
				File << ucRepeat;

				for (aeUInt32 i = 0; i < unequal.size (); ++i)
				{
					File << unequal[i].b;
					File << unequal[i].g;
					File << unequal[i].r;

					if (iBytesPerPixel == 4)
						File << unequal[i].a;
				}
			}
		}

    return true;
	}
}