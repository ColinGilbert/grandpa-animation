#ifndef __GRP_PATH_UTIL_H__
#define __GRP_PATH_UTIL_H__

#include <string>

namespace grp
{

bool containFolder(const STRING& url);

void getUrlBase(const STRING& url, STRING& baseUrl);

}

#endif
