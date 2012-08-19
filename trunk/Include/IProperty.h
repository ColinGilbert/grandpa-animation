#ifndef __GRP_I_PROPERTY_H__
#define __GRP_I_PROPERTY_H__

namespace grp
{

class IProperty
{
public:
	virtual size_t getCount() const = 0;

    virtual bool hasProperty(const Char* name) const = 0;
	virtual const Char* getName(size_t index) const = 0;
	virtual const Char* getValue(size_t index) const = 0;

	virtual const Char* getString(const Char* name) const = 0;
	virtual int getInt(const Char* name) const = 0;
	virtual bool getBool(const Char* name) const = 0;
	virtual float getFloat(const Char* name) const = 0;
	virtual Vector2 getVector2(const Char* name) const = 0;
	virtual Vector3 getVector3(const Char* name) const = 0;
	virtual Vector4 getVector4(const Char* name) const = 0;
	virtual Color32 getColor(const Char* name) const = 0;

protected:
	virtual ~IProperty(){}
};

}

#endif
