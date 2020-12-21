// <<< Information >>>
// Author:	Jan Krassnigg (Jan@Krassnigg.de)
// >>> Information <<<

#include "Main.h"



namespace AE_NS_GRAPHICS
{
	//void ExportRGBE (const char* szFile, int iWidth, int iHeight, float* pData, bool bCompress)
	//{
	//	vector<aeUInt8> RGBE (iWidth * iHeight * 4);

	//	for (int y = 0; y < iHeight; ++y)
	//	{
	//		for (int x = 0; x < iWidth; ++x)
	//		{
	//			float fColor[3];
	//			fColor[0] = pData[(y * iWidth + x) * 3 + 0];
	//			fColor[1] = pData[(y * iWidth + x) * 3 + 1];
	//			fColor[2] = pData[(y * iWidth + x) * 3 + 2];

	//			const aeUInt32 uiRaw = aeUTILITY::ConvertFloat3ToSRGB (fColor[0], fColor[1], fColor[2]);

	//			aeUInt8 a = (uiRaw & 0xFF000000) >> 24;
	//			aeUInt8 b = (uiRaw & 0x00FF0000) >> 16;
	//			aeUInt8 g = (uiRaw & 0x0000FF00) >> 8;
	//			aeUInt8 r = (uiRaw & 0x000000FF) >> 0;

	//			RGBE[(y * iWidth + x) * 4 + 0] = b;
	//			RGBE[(y * iWidth + x) * 4 + 1] = g;
	//			RGBE[(y * iWidth + x) * 4 + 2] = r;
	//			RGBE[(y * iWidth + x) * 4 + 3] = a;
	//		}
	//	}

	//	ExportTGA (szFile, iWidth, iHeight, 4, &RGBE[0], true);
	//}







}