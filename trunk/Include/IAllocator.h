#ifndef __GRP_I_ALLOCATOR_H__
#define __GRP_I_ALLOCATOR_H__

namespace grp
{

class IAllocator
{
public:
	virtual ~IAllocator(){}

	virtual void* allocateChunk(size_t size) = 0;

	virtual void deallocateChunk(const void* chunk) = 0;
};

}

#endif
