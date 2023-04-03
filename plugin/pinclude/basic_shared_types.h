#ifndef BASIC_SHARED_TYPES_H
#define BASIC_SHARED_TYPES_H

extern "C"
{
	struct GNF_UVec2
	{
		unsigned int x;
		unsigned int y;
	};

	typedef enum IMAGE_FORMAT
	{
		IMAGE_FORMAT_R8G8B8A8_UNSIGNED_BYTE,
		IMAGE_FORMAT_R8G8B8A8_FLOAT,
		IMAGE_FORMAT_R8G8B8_UNSIGNED_BYTE,
		IMAGE_FORMAT_R8G8B8_FLOAT,
	} IMAGE_FORMAT;

	struct GNF_Image
	{
		size_t      width;
		size_t      height;
		IMAGE_FORMAT format;
		uint8_t* pixels;
	};

}
#endif
