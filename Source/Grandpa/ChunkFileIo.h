#ifndef __GRP_CHUNK_FILE_IO_H__
#define __GRP_CHUNK_FILE_IO_H__

#include <istream>
#include <ostream>

namespace grp
{
///////////////////////////////////////////////////////////////////////////////////////////////////
//reading

//search for the chunk with specific name, from current position of a stream
//if chunk is found, pointer of stream will be pointed to the start of chunk data
//if chunk is not found, pointer remain the old position when this function is called
//boundary:		limitation for search range, 0 for no limit
bool findChunk(std::istream& input, int chunkName, size_t& chunkSize, size_t boundary = 0xffffffff);

//read a chunk with specific name and size
bool readChunk(std::istream& input, int chunkName,	char* buffer, size_t chunkSize, size_t boundary = 0xffffffff);

bool readStringChunk(std::istream& input, STRING& str, size_t& chunkSizeLeft, int chunkName);


///////////////////////////////////////////////////////////////////////////////////////////////////
//writing

bool createChunk(std::ostream& output, int chunkName, size_t chunkSize = 0, const char* buffer = 0);

bool updateChunkSize(std::ostream& output, size_t newSize);

bool writeStringChunk(std::ostream& output, const STRING& str, size_t& outLength, int chunkName);

//name and size
const size_t CHUNK_HEADER_SIZE = sizeof(int) + sizeof(size_t);

}

#endif
