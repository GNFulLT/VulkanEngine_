#ifndef BASE_IMAGE_LOADER_H
#define BASE_IMAGE_LOADER_H

#include "image_loader.h"

class BaseImageLoader : public ImageLoader
{
public:
	virtual GNF_Image* load(const String& path) final override;

	virtual void unload(GNF_Image* img) final override;
private:
};

#endif // BASE_IMAGE_LOADER_H