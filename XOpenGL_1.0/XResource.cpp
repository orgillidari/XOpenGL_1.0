#include "XResource.h"

#include "FreeImage.h"

namespace Smile
{
	int XResource::LoadTextureFile(const char* pFile, char** pBuffer, int* w, int* h)
	{
		FREE_IMAGE_FORMAT fif = FreeImage_GetFileType(pFile, 0);
		if (fif == FIF_UNKNOWN)
			return 0;

		FIBITMAP* dib = FreeImage_Load(fif, pFile);
		
		FIBITMAP* temp = dib;
		dib = FreeImage_ConvertTo32Bits(dib);
		FreeImage_Unload(temp);

		char* tempBuffer = (char*)FreeImage_GetBits(dib);
		*w = FreeImage_GetWidth(dib);
		*h = FreeImage_GetHeight(dib);

		//tempBuffer指向的内存在调用FreeImage_Unload()后会释放，所以这里分配内存空间，需要外部释放。但这种方法并不高效和安全。最好需要在这里直接创建纹理。
		size_t size = *w * *h * 4;
		*pBuffer = new char[size];
		memcpy(*pBuffer, tempBuffer, size);

		FreeImage_Unload(dib);

		return 1;
	}
}