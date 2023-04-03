#ifndef BASIC_SHARED_TYPES_H
#define BASIC_SHARED_TYPES_H

extern "C"
{
	struct GNF_UVec2
	{
		unsigned int x;
		unsigned int y;
	};

	enum IMAGE_FORMAT
	{
		IMAGE_FORMAT_UNSIGNED_BYTE,
		IMAGE_FORMAT_FLOAT,
	};

	struct GNF_Image
	{
		size_t      width;
		size_t      height;
		IMAGE_FORMAT format;
		uint8_t* pixels;
	};

}
#endif
