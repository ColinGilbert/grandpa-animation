#include "Precompiled.h"
#include "GredModel.h"
#include "GredCommon.h"

namespace gred
{

void WINAPI findGmdFile( const std::wstring& filename, void* param )
{
	std::vector<std::wstring>* list = reinterpret_cast<std::vector<std::wstring>* >( param );

	if ( filename.rfind( L".gmd" ) + 4 == filename.length() )
	{
		list->push_back( filename );
	}
}

void WINAPI findGptFile( const std::wstring& filename, void* param )
{
	std::vector<std::wstring>* list = reinterpret_cast<std::vector<std::wstring>* >( param );

	if ( filename.rfind( L".gpt" ) + 4 == filename.length() )
	{
		list->push_back( filename );
	}
}

void WINAPI findGskFile( const std::wstring& filename, void* param )
{
	std::vector<std::wstring>* list = reinterpret_cast<std::vector<std::wstring>* >( param );

	if ( filename.rfind( L".gsk" ) + 4 == filename.length() )
	{
		list->push_back( filename );
	}
}

void getModelFileList( std::vector<std::wstring>& fileList )
{
	enumFile( L"./", findGmdFile, &fileList );
}

void getPartFileList( std::vector<std::wstring>& fileList )
{
	enumFile( L"./", findGptFile, &fileList );
}

void getSkeletonFileList( std::vector<std::wstring>& fileList )
{
	enumFile( L"./", findGskFile, &fileList );
}

}