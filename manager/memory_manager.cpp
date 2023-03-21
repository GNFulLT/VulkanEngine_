#include "memory_manager.h"

bool operator==(const MemoryObjectMetaInfo& lhs, const MemoryObjectMetaInfo& rhs)
{
	return lhs.id == rhs.id;
}

bool operator==(const MemorySingletonMetaInfo& lhs, const MemorySingletonMetaInfo& rhs)
{
	return (strcmp(lhs.name.c_str(), rhs.name.c_str()) == 0) && lhs.mem_size == rhs.mem_size;
}

namespace std {

	template <>
	struct hash<MemoryObjectMetaInfo>
	{
		std::size_t operator()(const MemoryObjectMetaInfo& k) const
		{
			return k.id;
		}
	};

	template <>
	struct hash<MemorySingletonMetaInfo>
	{
		std::size_t operator()(const MemorySingletonMetaInfo& k) const
		{
			return hash_string(k.name.c_str());
		}
	};
}