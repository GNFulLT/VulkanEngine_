#ifndef BITMAP_H
#define BITMAP_H


#include "../typedefs.h"
#include "../../plugin/pinclude/basic_shared_types.h"

class Bitmap
{
public:
	Bitmap(GNF_Image* img,size_t imageCount = 1);
private:
	GNF_Image* m_image = nullptr;
	size_t m_imageCount;
};


#endif // BITMAP_H