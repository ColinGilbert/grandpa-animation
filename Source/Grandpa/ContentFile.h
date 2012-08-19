#ifndef __GRP_CONTENT_FILE_H__
#define __GRP_CONTENT_FILE_H__

#include <istream>
#include <ostream>

namespace grp
{

class ContentFile
{
public:
	virtual ~ContentFile(){}

	const STRING& getName() const;

protected:
	STRING	m_name;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
inline const STRING& ContentFile::getName() const
{
	return m_name;
}

}

#endif
