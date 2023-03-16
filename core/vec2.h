#ifndef VEC2_H
#define VEC2_H
#include "../plugin/pinclude/basic_shared_types.h"

struct UVec2 : public GNF_UVec2
{
	union
	{
		struct {
			unsigned int x;
			unsigned int y;
		};
		unsigned int v[2];
	};
	UVec2() : UVec2(0, 0)
	{

	}
	UVec2(unsigned int x, unsigned int y) : x(x), y(y)
	{

	}
};

#endif // VEC2_H