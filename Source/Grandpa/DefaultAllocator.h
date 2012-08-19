#ifndef __GRP_DEFAULT_ALLOCATOR_H__
#define __GRP_DEFAULT_ALLOCATOR_H__

#include "IAllocator.h"

namespace grp
{

class DefaultAllocator : public IAllocator
{
public:
	DefaultAllocator();
	virtual ~DefaultAllocator();

	virtual void* allocateChunk(size_t size);

	virtual void deallocateChunk(const void* chunk);

	size_t getAllocatedCount() const;

	size_t getAllocatedSize() const;

	size_t getTotalAllocatedCount() const;

	size_t getTotalAllocatedSize() const;

private:
	size_t	m_totalAllocatedCount;
	size_t	m_totalAllocatedSize;
	size_t	m_allocatedCount;
	size_t	m_allocatedSize;
};

}

#endif
