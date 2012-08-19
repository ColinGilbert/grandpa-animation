#include "Precompiled.h"
#include "PathUtil.h"

namespace grp
{

///////////////////////////////////////////////////////////////////////////////////////////////////
bool containFolder(const STRING& url)
{
	return (url.find_last_of(GT('/')) != STRING::npos || url.find_last_of(GT('\\')) != STRING::npos);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void getUrlBase(const STRING& url, STRING& baseUrl)
{
	size_t slashPos = url.find_last_of(GT('/'));
	size_t backSlashPos = url.find_last_of(GT('\\'));
	if (slashPos == STRING::npos)
	{
		if (backSlashPos == STRING::npos)
		{
			baseUrl = GT("");
		}
		else
		{
			baseUrl = url.substr(0, backSlashPos + 1);
		}
	}
	else
	{
		if (backSlashPos == STRING::npos)
		{
			baseUrl = url.substr(0, slashPos + 1);
		}
		else
		{
			baseUrl = url.substr(0, std::max(slashPos, backSlashPos) + 1);
		}
	}
}

}
