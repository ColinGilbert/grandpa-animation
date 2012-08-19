#include "Precompiled.h"
#include "DefaultAllocator.h"

namespace grp
{

///////////////////////////////////////////////////////////////////////////////////////////////////
DefaultAllocator::DefaultAllocator()
	: m_totalAllocatedCount(0)
	, m_totalAllocatedSize(0)
	, m_allocatedCount(0)
	, m_allocatedSize(0)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
DefaultAllocator::~DefaultAllocator()
{
	if (m_allocatedCount != 0 || m_allocatedSize != 0)
	{
		WRITE_LOG(WARNING, GT("Memory leak detected!"));
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void* DefaultAllocator::allocateChunk(size_t size)
{
	m_totalAllocatedCount++;
	m_allocatedCount++;
	m_totalAllocatedSize += size;
	m_allocatedSize += size;
	size_t* p = (size_t*)(malloc(size + sizeof(size_t)));
	*p = size;
	return p + 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DefaultAllocator::deallocateChunk(const void* chunk)
{
	const size_t* p = (size_t*)chunk - 1;
	m_allocatedSize -= *p;
	m_allocatedCount--;
	free((void*)p);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
size_t DefaultAllocator::getAllocatedCount() const
{
	return m_allocatedCount;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
size_t DefaultAllocator::getAllocatedSize() const
{
	return m_allocatedSize;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
size_t DefaultAllocator::getTotalAllocatedCount() const
{
	return m_totalAllocatedCount;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
size_t DefaultAllocator::getTotalAllocatedSize() const
{
	return m_totalAllocatedSize;
}

}
