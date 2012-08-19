#ifndef __GRP_REFERENCE_COUNTED_H__
#define __GRP_REFERENCE_COUNTED_H__

namespace grp
{

class ReferenceCounted
{
public:
	ReferenceCounted();
	virtual ~ReferenceCounted(){}

	void grab() const;

	void drop() const;

	virtual void free() const;

private:
	mutable int		m_referenceCount;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
inline ReferenceCounted::ReferenceCounted()
	: m_referenceCount(0)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void ReferenceCounted::grab() const
{
	++m_referenceCount;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void ReferenceCounted::drop() const
{
	assert(m_referenceCount > 0);

	--m_referenceCount;
	if (m_referenceCount <= 0)
	{
		free();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void ReferenceCounted::free() const
{
	GRP_DELETE(this);
}

#define SAFE_DROP(p)      {\
								if ((p) != NULL)\
								{\
									(p)->drop();\
									(p) = NULL;\
								}\
							}

}

#endif
