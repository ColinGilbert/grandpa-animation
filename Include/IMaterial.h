#ifndef __GRP_I_MATERIAL_H__
#define __GRP_I_MATERIAL_H__

namespace grp
{

////////////////////////////////////////////////////////////////////////////////////////////////////
enum TexcoordMode
{
	TEXCOORD_WRAP = 1,
	TEXCOORD_MIRROR,
	TEXCOORD_CLAMP,
	TEXCOORD_BORDER,
	TEXCOORD_COUNT
};

////////////////////////////////////////////////////////////////////////////////////////////////////
enum TriangleCullMode
{
	CULL_NONE = 1,
	CULL_CW,
	CULL_CCW,
	CULL_COUNT
};

////////////////////////////////////////////////////////////////////////////////////////////////////
enum ZBufferMode
{
	Z_NEVER = 1,
	Z_LESS,
	Z_EQUAL,
	Z_LESSEQUAL,
	Z_GREATER,
	Z_NOTEQUAL,
	Z_GREATEREQUAL,
	Z_ALWAYS,
	Z_COUNT
};

////////////////////////////////////////////////////////////////////////////////////////////////////
class ITexture
{
public:
	virtual const Char* filename() const = 0;
	virtual TexcoordMode texcoordMode() const = 0;
	virtual bool linearFilter() const = 0;
	virtual bool mipmap() const = 0;

	virtual void setUserData(void* data) = 0;
	virtual void* getUserData() const = 0;

protected:
	virtual ~ITexture(){}
};

////////////////////////////////////////////////////////////////////////////////////////////////////
class IMaterial
{
public:
    virtual void build() = 0;
    virtual bool isBuilt() const = 0;
    virtual const IResource* getResource() const = 0;
	virtual const Char* type() const = 0;
	virtual TriangleCullMode cullMode() const = 0;
	virtual ZBufferMode	zMode() const = 0;
	virtual Color32 color() const = 0;
	virtual bool fog() const = 0;
	virtual bool zEnable() const = 0;
	virtual bool zWrite() const = 0;
	virtual bool wireframe() const = 0;

	virtual size_t getTextureCount() const = 0;
	virtual ITexture* getTexture(size_t index) = 0;

	virtual IProperty* getProperty() const = 0;

	virtual void setUserData(void* data) = 0;
	virtual void* getUserData() const = 0;

protected:
	virtual ~IMaterial(){}
};

}

#endif
