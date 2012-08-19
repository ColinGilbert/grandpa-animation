#ifndef __GRP_PROPERTY_H__
#define __GRP_PROPERTY_H__

#include "IProperty.h"
#include "SlimXml.h"

namespace grp
{

struct PropertyPair
{
	STRING	name;
	STRING	value;
};

class Property : public IProperty
{
public:
	virtual size_t getCount() const;

    virtual bool hasProperty(const Char* name) const;
	virtual const Char* getName(size_t index) const;
	virtual const Char* getValue(size_t index) const;

	virtual const Char* getString(const Char* name) const;
	virtual int getInt(const Char* name) const;
	virtual bool getBool(const Char* name) const;
	virtual float getFloat(const Char* name) const;
	virtual Vector2 getVector2(const Char* name) const;
	virtual Vector3 getVector3(const Char* name) const;
	virtual Vector4 getVector4(const Char* name) const;
	virtual Color32 getColor(const Char* name) const;

private:
	const PropertyPair* findPair(const Char* name) const;

public:
	VECTOR(PropertyPair)	pairs;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
inline size_t Property::getCount() const
{
	return pairs.size();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline bool Property::hasProperty(const Char* name) const
{
    const PropertyPair* pair = findPair(name);
    return pair != NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline const Char* Property::getName(size_t index) const
{
	return pairs[index].name.c_str();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline const Char* Property::getValue(size_t index) const
{
	return pairs[index].value.c_str();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline const Char* Property::getString(const Char* name) const
{
	const PropertyPair* pair = findPair(name);
	if (pair != NULL)
	{
		return pair->value.c_str();
	}
	return GT("");
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline int Property::getInt(const Char* name) const
{
	const PropertyPair* pair = findPair(name);
	if (pair != NULL)
	{
		int value;
		Sscanf(pair->value.c_str(), GT("%d"), &value);
		return value;
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline bool Property::getBool(const Char* name) const
{
	const PropertyPair* pair = findPair(name);
	if (pair != NULL)
	{
		return (pair->value == GT("true") || pair->value == GT("TRUE"));
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline float Property::getFloat(const Char* name) const
{
	const PropertyPair* pair = findPair(name);
	if (pair != NULL)
	{
		float value;
		Sscanf(pair->value.c_str(), GT("%f"), &value);
		return value;
	}
	return 0.0f;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline Vector2 Property::getVector2(const Char* name) const
{
	const PropertyPair* pair = findPair(name);
	if (pair != NULL)
	{
		Vector2 v2;
		Sscanf(pair->value.c_str(), GT("%f,%f"), &v2.X, &v2.Y);
		return v2;
	}
	return Vector2::ZERO;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline Vector3 Property::getVector3(const Char* name) const
{
	const PropertyPair* pair = findPair(name);
	if (pair != NULL)
	{
		Vector3 v3;
		Sscanf(pair->value.c_str(), GT("%f,%f,%f"), &v3.X, &v3.Y, &v3.Z);
		return v3;
	}
	return Vector3::ZERO;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline Vector4 Property::getVector4(const Char* name) const
{
	const PropertyPair* pair = findPair(name);
	if (pair != NULL)
	{
		Vector4 v4;
		Sscanf(pair->value.c_str(), GT("%f,%f,%f,%f"), &v4.X, &v4.Y, &v4.Z, &v4.W);
		return v4;
	}
	return Vector4::ZERO;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline Color32 Property::getColor(const Char* name) const
{
	const PropertyPair* pair = findPair(name);
	if (pair != NULL)
	{
		unsigned int r, g, b, a;
		Sscanf(pair->value.c_str(), GT("%u,%u,%u,%u"), &r, &g, &b, &a);
		Color32 color = (((a&0xff) << 24) | ((r&0xff)<<16) | ((g&0xff)<<8) | (b&0xff));
		return color;
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline const PropertyPair* Property::findPair(const Char* name) const
{
	for (VECTOR(PropertyPair)::const_iterator iter = pairs.begin();
		iter != pairs.end();
		++iter)
	{
		if (Strcmp(iter->name.c_str(), name) == 0)
		{
			return &(*iter);
		}
	}
	return NULL;
}

}

#endif
