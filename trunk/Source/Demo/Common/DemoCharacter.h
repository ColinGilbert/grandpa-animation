#ifndef __DEMO_CHARACTER_H__
#define __DEMO_CHARACTER_H__

#include "IEventHandler.h"
#include "Ragdoll.h"
#include "d3dx9.h"
#include <string>

namespace grp
{
class IModel;
class IPart;
class IMesh;
class IMeshBuffer;
class IVertexStream;
class IMaterial;
class ITexture;
}
class DemoTexture;
struct IDirect3DDevice9;
struct IDirect3DVertexBuffer9;
struct IDirect3DIndexBuffer9;
struct ID3DXEffect;
class Ragdoll;

///////////////////////////////////////////////////////////////////////////////////////////////////
class DemoCharacter : public grp::IEventHandler, public grp::ISkeletonCallback
{
public:
	DemoCharacter(const wchar_t* modelPath, IDirect3DDevice9* device);
	~DemoCharacter();

	grp::IModel* getModel();

	virtual void update(double time, float elapsedTime, unsigned long flag = 0);

	virtual void render(ID3DXEffect* effect, const D3DXMATRIXA16& mView, const D3DXMATRIXA16& mProj, bool temp = true);

	void renderNormal(const D3DXMATRIXA16& mView, const D3DXMATRIXA16& mProj);
	void renderBoundingBox(const D3DXMATRIXA16& mView, const D3DXMATRIXA16& mProj);
	void renderSkeleton(const D3DXMATRIXA16& mView, const D3DXMATRIXA16& mProj);

	void setGpuSkinning(bool enable);
	bool isGpuSkinning() const;

	//from IEventHandler below
	virtual void onPartBuilt(grp::IModel* model, const wchar_t* slot, grp::IPart* part);
	virtual void onMaterialBuilt(grp::IModel* model, grp::IPart* part, size_t materialIndex);
	virtual void onPartDestroy(grp::IModel* model, const wchar_t* slot, grp::IPart* part);

	virtual void onSkeletonBuilt(grp::IModel* model, grp::ISkeleton* skeleton);

	virtual void onAnimationStart(grp::IModel* model, const wchar_t* slot, const wchar_t* eventName, grp::IProperty* params);
	virtual void onAnimationEnd(grp::IModel* model, const wchar_t* slot, const wchar_t* eventName, grp::IProperty* params);
	virtual void onAnimationTime(grp::IModel* model, const wchar_t* slot, const wchar_t* eventName, grp::IProperty* params);

	//for ragdoll
	virtual void onPostUpdate(grp::ISkeleton* skeleton);

	void createRagdoll(const grp::Matrix& transform);
	void destroyRagdoll();

protected:
	void drawBox(const grp::AaBox& box);

private:
	void renderPart(ID3DXEffect* effect, grp::IPart* part, const D3DXMATRIXA16& mView, const D3DXMATRIXA16& mProj, bool temp);

	IDirect3DVertexBuffer9* createVertexBuffer(grp::IMesh* mesh, grp::IVertexStream* vertexStream) const;
	void createIndexBuffer(grp::IMeshBuffer* buffer) const;

	void generateSkinVertexBuffer(grp::IMesh* mesh);

	void updateVertexBuffer(grp::IMesh* mesh);

	void setMaterial(ID3DXEffect* effect, grp::IMaterial* material);

	void setTexture(ID3DXEffect* effect, const char* name, grp::ITexture* texture);

	void createPartD3dBuffers(grp::IPart* part);
	void releasePartD3dBuffers(grp::IPart* part);
	void releasePartD3dTextures(grp::IPart* part);

	void getUrlBase(const std::wstring& url, std::wstring& baseUrl);

	void setPartBuffersBuilt(grp::IPart* part);
	bool isPartBuffersBuilt(grp::IPart* part) const;

	void renderMeshNormal(grp::IMesh* mesh);

	//for ragdoll
	void setBoneTransform(const wchar_t* boneName, const grp::Vector3& position, const grp::Quaternion& rotation);

protected:
	IDirect3DDevice9*	m_device;
	grp::IModel*		m_model;
	std::wstring		m_baseUrl;
	Ragdoll*			m_ragdoll;
	bool				m_gpuSkinning;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
inline grp::IModel* DemoCharacter::getModel()
{
	return m_model;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline bool DemoCharacter::isGpuSkinning() const
{
	return m_gpuSkinning;
}

#endif
