#ifndef __GRP_I_PART_H__
#define __GRP_I_PART_H__

namespace grp
{
class IMesh;
class IProperty;
class IMaterial;

class IPart
{
public:
	virtual IResource* getResource() const = 0;

	virtual void setVisible(bool visible) = 0;
	virtual bool isVisible() const = 0;

	virtual bool isSkinnedPart() const = 0;

	virtual bool isMeshBuilt() const = 0;

	virtual IMesh* getMesh() const = 0;

	virtual size_t getMaterialCount() const = 0;

	virtual IMaterial* getMaterial(size_t index) const = 0;

	virtual IProperty* getProperty() const = 0;

	virtual void setUserData(void* data) = 0;
	virtual void* getUserData() const = 0;


protected:
	virtual ~IPart(){}
};

}

#endif
