#ifndef __GRED_MODEL_H__
#define __GRED_MODEL_H__

#include <vector>
#include <string>

namespace gred
{

void getModelFileList( std::vector<std::wstring>& fileList );

void getPartFileList( std::vector<std::wstring>& fileList );

void getSkeletonFileList( std::vector<std::wstring>& fileList );

}

#endif
