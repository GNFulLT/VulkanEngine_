#include "base_image_loader.h"
#include "../stb_image.h"
#include "../../manager/memory_manager.h"
GNF_Image* BaseImageLoader::load(const String& path)
{
	int texWidth, texHeight, texChannels;
	auto* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	GNF_Image* img = MemoryManager::get_singleton()->fnew_c<GNF_Image>();
	img->width = texWidth;
	img->height = texHeight;
	img->format = IMAGE_FORMAT_R8G8B8A8_UNSIGNED_BYTE;
	img->pixels = (uint8_t*)pixels;
	return img;
}
void BaseImageLoader::unload(GNF_Image* img)
{
	stbi_image_free(img->pixels);
	MemoryManager::get_singleton()->fdel_c(img);
}
