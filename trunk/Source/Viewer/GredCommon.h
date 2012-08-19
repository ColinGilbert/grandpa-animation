#ifndef __GRED_COMMON_H__
#define __GRED_COMMON_H__

#include <string>

namespace gred
{

typedef void ( WINAPI *EnumCallback )( const std::wstring& path, void* param );

void enumFile( const std::wstring& searchPath, EnumCallback calllback, void* param );

}

#endif
