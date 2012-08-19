#include "Precompiled.h"
#include "ChunkFileIo.h"
#include "SlimXml.h"
#include <cassert>
#include "Performance.h"

namespace grp
{

///////////////////////////////////////////////////////////////////////////////////////////////////
bool findChunk(std::istream& input, int chunkName, size_t& chunkSize, size_t boundary)
{
	unsigned long originPos = (unsigned long)input.tellg();

	size_t offset = 0;
	int name = 0;

	while (!input.eof())
	{
		input.read((char*)(&name), sizeof(name));
		if (input.eof())
		{
			break;
		}
		input.read((char*)&chunkSize, sizeof(chunkSize));
		if (input.eof())
		{
			break;
		}
		offset += (8 + chunkSize);
		if (boundary != 0xffffffff && offset > boundary)
		{
			break;
		}
		if (name == chunkName)
		{
			return true;
		}
		input.seekg(chunkSize, std::ios::cur);
		if (input.eof())
		{
			break;
		}
	}
	//failed
	input.clear();
	input.seekg(originPos);
	chunkSize = 0;
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool readChunk(std::istream& input, int chunkName, char* buffer, size_t chunkSize,	size_t boundary)
{
	assert(buffer != NULL);

	unsigned long originPos = (unsigned long)input.tellg();

	size_t size;
	if (!findChunk(input, chunkName, size, boundary))
	{
		return false;
	}
	if (size != chunkSize)
	{
		input.seekg(originPos);
		return false;
	}
	input.read(buffer, size);
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool createChunk(std::ostream& output, int chunkName, size_t chunkSize, const char* buffer)
{
	output.write((char*)&chunkName, sizeof(chunkName));
	output.write((char*)&chunkSize, sizeof(chunkSize));
	if (chunkSize > 0 && buffer != NULL)
	{
		output.write(buffer, chunkSize);
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool updateChunkSize(std::ostream& output, size_t newSize)
{
	int backSize = newSize + sizeof(newSize);
	output.seekp(-backSize, std::ios::cur);
	output.write((char*)&newSize, sizeof(newSize));
	output.seekp(newSize, std::ios::cur);
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool readStringChunk(std::istream& input, STRING& str, size_t& chunkSizeLeft, int chunkName)
{
	//PERF_NODE_FUNC();

	size_t strLength;
	if (!findChunk(input, chunkName, strLength, chunkSizeLeft))
	{
		return false;
	}
	chunkSizeLeft -= (strLength + CHUNK_HEADER_SIZE);

#if defined (GRP_USE_WCHAR)
	std::string utf8Str;
	utf8Str.resize(strLength);
	input.read(&utf8Str[0], strLength);
	str.resize(strLength);
	size_t actualLength = slim::utf8toutf16(&utf8Str[0], strLength, &str[0], strLength);
	str.resize(actualLength);
#else
	str.resize(strLength);
	input.read(&str[0], strLength);
#endif
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool writeStringChunk(std::ostream& output, const STRING& str, size_t& outLength, int chunkName)
{
#if defined (GRP_USE_WCHAR)
	size_t bufferLength = str.length() * 3;
	std::string utf8Str;
	utf8Str.resize(bufferLength);	
	outLength = slim::utf16toutf8(&str[0], str.length(), &utf8Str[0], bufferLength);
	if (!createChunk(output, chunkName, outLength, &utf8Str[0]))
	{
		return false;
	}
#else
	if (!createChunk(output, chunkName, outLength, &str[0]))
	{
		return false;
	}
#endif
	return true;
}

}
