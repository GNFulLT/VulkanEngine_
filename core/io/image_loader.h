#ifndef IMAGE_LOADER_H
#define IMAGE_LOADER_H

#include "bitmap.h"
#include "../string/string_types.h"

class ImageLoader
{
public:
	virtual ~ImageLoader() = default;

	virtual GNF_Image* load(const String& path) = 0;

	virtual void unload(GNF_Image* img) = 0;
private:
};

#endif // IMAGE_LOADER_H