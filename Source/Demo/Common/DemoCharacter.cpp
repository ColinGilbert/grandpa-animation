#include "Grandpa.h"
#include "DemoCharacter.h"
#include "MultithreadResManager.h"
#include "DemoTexture.h"
#include "Ragdoll.h"
#include "d3d9.h"
#include "d3dx9.h"
#include <cassert>
#include "Performance.h"

extern MultithreadResManager* g_resourceManager;

///////////////////////////////////////////////////////////////////////////////////////////////////
DemoCharacter::DemoCharacter(const wchar_t* modelPath, IDirect3DDevice9* device)
	: m_device(device)
	, m_model(NULL)
	, m_ragdoll(NULL)
	, m_gpuSkinning(false)
{
	getUrlBase(modelPath, m_baseUrl);
	m_model = grp::createModel(modelPath, this);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
DemoCharacter::~DemoCharacter()
{
	if (m_ragdoll != NULL)
	{
		delete m_ragdoll;
	}
	if (m_model != NULL)
	{
		//内部会调用onPartDestroy
		grp::destroyModel(m_model);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DemoCharacter::update(double time, float elapsedTime, unsigned long flag)
{
	if (m_model == NULL)
	{
		return;
	}
	if (m_ragdoll != NULL)
	{
		::Sleep(15);
		m_ragdoll->update(time * 0.1f, (float)elapsedTime * 0.1f);
		//m_model->stopAllAnimations();
	}
	if (m_model != NULL)
	{
		m_model->update(time, elapsedTime, flag);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DemoCharacter::render(ID3DXEffect* effect, const D3DXMATRIXA16& mView, const D3DXMATRIXA16& mProj, bool temp)
{
	if (m_model == NULL)
	{
		return;
	}
	for (grp::IPart* part = m_model->getFirstPart();
		part != NULL;
		part = m_model->getNextPart())
	{
		renderPart(effect, part, mView, mProj, temp);
	}
	//if (m_ragdoll != NULL)
	//{
	//	m_ragdoll->render(m_device, mView, mProj);
	//}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DemoCharacter::renderNormal(const D3DXMATRIXA16& mView, const D3DXMATRIXA16& mProj)
{
	assert(m_model != NULL);

	for (grp::IPart* part = m_model->getFirstPart();
		part != NULL;
		part = m_model->getNextPart())
	{
		grp::IMesh* mesh = part->getMesh();
		if (mesh == NULL)
		{
			continue;
		}
		grp::Matrix meshTransform = mesh->getTransform() * m_model->getTransform();
		D3DXMATRIXA16 mWorld;
		memcpy(&mWorld, &meshTransform, sizeof(D3DXMATRIX));
		
		m_device->SetTransform(D3DTS_WORLD, &mWorld);
		m_device->SetTransform(D3DTS_VIEW, &mView);
		m_device->SetTransform(D3DTS_PROJECTION, &mProj);

		renderMeshNormal(mesh);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DemoCharacter::renderBoundingBox(const D3DXMATRIXA16& mView, const D3DXMATRIXA16& mProj)
{
	assert(m_model != NULL);

	const grp::Matrix& transform = m_model->getTransform();
	D3DXMATRIXA16 mWorld;
	memcpy(&mWorld, &transform, sizeof(D3DXMATRIX));

	m_device->SetTransform(D3DTS_WORLD, &mWorld);
	m_device->SetTransform(D3DTS_VIEW, &mView);
	m_device->SetTransform(D3DTS_PROJECTION, &mProj);

	drawBox(m_model->getBoundingBox());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DemoCharacter::renderSkeleton(const D3DXMATRIXA16& mView, const D3DXMATRIXA16& mProj)
{
	assert(m_model != NULL);
	grp::ISkeleton* skeleton = m_model->getSkeleton();
	if (skeleton == NULL)
	{
		return;
	}
	const grp::Matrix& transform = m_model->getTransform();
	D3DXMATRIXA16 mWorld;
	memcpy(&mWorld, &transform, sizeof(D3DXMATRIX));

	m_device->SetTransform(D3DTS_WORLD, &mWorld);
	m_device->SetTransform(D3DTS_VIEW, &mView);
	m_device->SetTransform(D3DTS_PROJECTION, &mProj);

	size_t boneCount = skeleton->getBoneCount();
	std::vector<grp::Vector3> vertices(2 * boneCount);
	for (size_t i = 1; i < boneCount; ++i)
	{
		grp::IBone* bone = skeleton->getBoneById(i);
		if (bone == NULL)
		{
			continue;
		}
		grp::IBone* parent = bone->getParent();
		if (parent == NULL)
		{
			continue;
		}
		vertices[2 * i] = bone->getAbsoluteTransform().getTranslation();
		vertices[2 * i + 1] = parent->getAbsoluteTransform().getTranslation();
	}
	m_device->SetFVF(D3DFVF_XYZ);
	m_device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	m_device->DrawPrimitiveUP(D3DPT_LINELIST, boneCount - 1, &vertices[2], sizeof(grp::Vector3)); 
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DemoCharacter::setGpuSkinning(bool enable)
{
	m_gpuSkinning = enable;
	if (m_model != NULL)
	{
		for (grp::IPart* part = m_model->getFirstPart();
			part != NULL;
			part = m_model->getNextPart())
		{
			grp::IMesh* mesh = part->getMesh();
			if (mesh == NULL)
			{
				continue;
			}
			grp::ISkin* skin = part->getMesh()->getSkin();
			if (skin != NULL)
			{
				skin->setGpuSkinning(enable);
				releasePartD3dBuffers(part);
				createPartD3dBuffers(part);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DemoCharacter::onPartBuilt(grp::IModel* model, const wchar_t* slot, grp::IPart* part)
{
	grp::ISkin* skin = part->getMesh()->getSkin();
	if (skin != NULL)
	{
		skin->setGpuSkinning(m_gpuSkinning);
	}
	createPartD3dBuffers(part);
	setPartBuffersBuilt(part);
}

void DemoCharacter::onMaterialBuilt(grp::IModel* model, grp::IPart* part, size_t materialIndex)
{
	assert(part != NULL);
	grp::IMaterial* material = part->getMaterial(materialIndex);
	assert(material != NULL);

	DemoTexture* textureResource = NULL;
	for (size_t i = 0; i < material->getTextureCount(); ++i)
	{
		grp::ITexture* texture = material->getTexture(i);
		std::wstring path = texture->filename();
		if (path.find_last_of(L'/') == std::wstring::npos
			&& path.find_last_of(L'\\') == std::wstring::npos)
		{
			std::wstring fullUrl = m_baseUrl + texture->filename();
			textureResource = g_resourceManager->grabTexture(fullUrl.c_str());
		}
		else
		{
			//自带路径
			textureResource = g_resourceManager->grabTexture(texture->filename());
		}
		if (textureResource == NULL)
		{
			continue;
		}
		texture->setUserData(textureResource);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DemoCharacter::onPartDestroy(grp::IModel* model, const wchar_t* slot, grp::IPart* part)
{
	releasePartD3dBuffers(part);
	releasePartD3dTextures(part);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DemoCharacter::onSkeletonBuilt(grp::IModel* model, grp::ISkeleton* skeleton)
{
	//add ik or ragdoll control here
	if (skeleton != NULL)
	{
		skeleton->setCallback(this);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DemoCharacter::onAnimationStart(grp::IModel* model, const wchar_t* slot, const wchar_t* eventName, grp::IProperty* params)
{
	//handle event here
	if (wcscmp(eventName, L"sound") == 0)
	{
		//play sound here
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DemoCharacter::onAnimationEnd(grp::IModel* model, const wchar_t* slot, const wchar_t* eventName, grp::IProperty* params)
{
	//handle event here
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DemoCharacter::onAnimationTime(grp::IModel* model, const wchar_t* slot, const wchar_t* eventName, grp::IProperty* params)
{
	//handle event here
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DemoCharacter::drawBox(const grp::AaBox& box)
{
	std::vector<grp::Vector3> vertices(24);
	vertices[0] = vertices[2] = vertices[4] = box.MinEdge;
	vertices[1] = vertices[13] = vertices[17] = grp::Vector3(box.MaxEdge.X, box.MinEdge.Y, box.MinEdge.Z);
	vertices[3] = vertices[7] = vertices[9] = grp::Vector3(box.MinEdge.X, box.MaxEdge.Y, box.MinEdge.Z);
	vertices[5] = vertices[11] = vertices[15] = grp::Vector3(box.MinEdge.X, box.MinEdge.Y, box.MaxEdge.Z);
	vertices[18] = vertices[6] = vertices[10] = grp::Vector3(box.MinEdge.X, box.MaxEdge.Y, box.MaxEdge.Z);
	vertices[20] = vertices[14] = vertices[16] = grp::Vector3(box.MaxEdge.X, box.MinEdge.Y, box.MaxEdge.Z);
	vertices[22] = vertices[8] = vertices[12] = grp::Vector3(box.MaxEdge.X, box.MaxEdge.Y, box.MinEdge.Z);
	vertices[19] = vertices[21] = vertices[23] = box.MaxEdge;

	m_device->SetFVF(D3DFVF_XYZ);
	m_device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	m_device->DrawPrimitiveUP(D3DPT_LINELIST, 12, &vertices[0], sizeof(grp::Vector3)); 
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DemoCharacter::renderPart(ID3DXEffect* effect, grp::IPart* part, const D3DXMATRIXA16& mView, const D3DXMATRIXA16& mProj, bool temp)
{
	assert(part != NULL);
	if (!part->isVisible())
	{
		return;
	}
	if (!isPartBuffersBuilt(part))
	{
		return;
	}
	grp::IMesh* mesh = part->getMesh();
	if (part->isSkinnedPart() && !m_gpuSkinning)
	{
		updateVertexBuffer(mesh);
	}

	D3DXMATRIXA16 mWorldViewProjection;
	D3DXMATRIXA16 mWorld;

	grp::Matrix meshTransform = mesh->getTransform() * m_model->getTransform();
	memcpy(&mWorld, &meshTransform, sizeof(D3DXMATRIX));
	mWorldViewProjection = mWorld * mView * mProj;
	effect->SetMatrix("g_mWorldViewProjection", &mWorldViewProjection);
	effect->SetMatrix("g_mWorld", &mWorld);

	grp::IVertexStream* staticStream = mesh->getStaticVertexStream();
	grp::IVertexStream* dynamicStream = mesh->getDynamicVertexStream();
	assert(staticStream != NULL && dynamicStream != NULL);
	IDirect3DVertexBuffer9* staticVertexBuffer = (IDirect3DVertexBuffer9*)staticStream->getUserData();
	IDirect3DVertexBuffer9* dynamicVertexBuffer = (IDirect3DVertexBuffer9*)dynamicStream->getUserData();
	//assert(staticVertexBuffer != NULL && dynamicVertexBuffer != NULL);
	assert(m_device != NULL);
	if (dynamicVertexBuffer != NULL && temp)
	{
		m_device->SetStreamSource(0, dynamicVertexBuffer, 0, dynamicStream->getStride());
	}
	size_t source = dynamicVertexBuffer == NULL ? 0 : 1;
	size_t stride = staticStream->getStride();
	if (temp)
	{
		m_device->SetStreamSource(source, staticVertexBuffer, 0, stride);
	}
	if (part->isSkinnedPart() && m_gpuSkinning)
	{
		PERF_NODE("SetMatrixArray");

		IDirect3DVertexBuffer9* skinVertexBuffer = (IDirect3DVertexBuffer9*)mesh->getUserData();
		assert(skinVertexBuffer != NULL);
		if (temp)
		{
			m_device->SetStreamSource(2, skinVertexBuffer, 0, 20);
		}
		effect->SetMatrixArray("g_boneMatrices", 
								(D3DXMATRIX*)mesh->getSkin()->getBoneMatrices(),
								mesh->getSkin()->getBoneCount());
	}

	for (size_t i = 0; i < mesh->getMeshBufferCount(); ++i)
	{
		PERF_NODE("Crap");

		grp::IMeshBuffer* buffer = mesh->getMeshBuffer(i);
		IDirect3DIndexBuffer9* indexBuffer = (IDirect3DIndexBuffer9*)buffer->getUserData();
		assert(indexBuffer != NULL);
		if (temp)
		{
			PERF_NODE("SetIndices");
			m_device->SetIndices(indexBuffer);
		}
		if (i < part->getMaterialCount() && temp)
		{
			setMaterial(effect, part->getMaterial(i));
		}
		{
			PERF_NODE("Draw");

			assert(effect != NULL);
			effect->BeginPass(0);
			m_device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, mesh->getVertexCount(),
										0, static_cast<UINT>(buffer->getIndexCount() / 3));
			effect->EndPass();
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
IDirect3DVertexBuffer9* DemoCharacter::createVertexBuffer(grp::IMesh* mesh, grp::IVertexStream* vertexStream) const
{
	assert(m_device != NULL);
	assert(mesh != NULL);
	assert(vertexStream != NULL);

	DWORD vertexCount = mesh->getMaxVertexCount();
	assert(vertexCount > 0);

	IDirect3DVertexBuffer9* vertexBuffer = NULL;
	m_device->CreateVertexBuffer(vertexStream->getStride() * vertexCount,
								 D3DUSAGE_DYNAMIC,
								 0,
								 D3DPOOL_DEFAULT,
								 &vertexBuffer,
								 NULL);
	vertexStream->setUserData(vertexBuffer);
	return vertexBuffer;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DemoCharacter::createIndexBuffer(grp::IMeshBuffer* buffer) const
{
	assert(m_device != NULL);
	assert(buffer != NULL);

	DWORD indexCount = buffer->getMaxIndexCount();
	assert(indexCount > 0);
	IDirect3DIndexBuffer9* indexBuffer = NULL;
	m_device->CreateIndexBuffer(sizeof(DWORD) * indexCount,
								D3DUSAGE_DYNAMIC,
								D3DFMT_INDEX32,
								D3DPOOL_DEFAULT,
								&indexBuffer,
								NULL);

	//no lod, copy only once
	const DWORD* src = buffer->getIndices();
	DWORD* dst = NULL;
	indexBuffer->Lock(0, 0, (void**)&dst, 0);
	memcpy(dst, src, indexCount * sizeof(DWORD));
	indexBuffer->Unlock();

	buffer->setUserData(indexBuffer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DemoCharacter::generateSkinVertexBuffer(grp::IMesh* mesh)
{
	grp::ISkin* skin = mesh->getSkin();
	assert(skin != NULL);
	IDirect3DVertexBuffer9* vertexBuffer = NULL;
	m_device->CreateVertexBuffer(20 * skin->getSkinVertexCount(),
								D3DUSAGE_DYNAMIC,
								0,
								D3DPOOL_DEFAULT,
								&vertexBuffer,
								NULL);
	unsigned long* data = NULL;
	vertexBuffer->Lock(0, 0, (void**)&data, 0);
	for (size_t vertex = 0; vertex < skin->getSkinVertexCount(); ++vertex, data += 5)
	{
		*data = 0;
		const grp::VertexInfluence* influences = skin->getVertexInfluences(vertex);
		assert(grp::MAX_VERTEX_INFLUENCE == 4);
		for (size_t i = 0; i < 4; ++i)
		{
			//只适用于依赖骨骼不超过256个的mesh
			*data |= ((influences[i].boneIndex & 0xff) << (i * 8));
			*((float*)(data + i + 1)) = influences[i].weight;
		}
	}
	vertexBuffer->Unlock();

	mesh->setUserData(vertexBuffer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DemoCharacter::updateVertexBuffer(grp::IMesh* mesh)
{
	PERF_NODE_FUNC();

	assert(mesh != NULL);
	if (mesh->getVertexCount() == 0)
	{
		return;
	}
	grp::IVertexStream* dynamicStream = mesh->getDynamicVertexStream();
	assert(dynamicStream != NULL);
	IDirect3DVertexBuffer9* dynamicVertexBuffer = (IDirect3DVertexBuffer9*)dynamicStream->getUserData();
	assert(dynamicVertexBuffer != NULL);

	//copy data
	const void* src = dynamicStream->getBuffer();
	void* dst = NULL;
	size_t size = dynamicStream->getStride() * mesh->getVertexCount();

	dynamicVertexBuffer->Lock(0, 0, &dst, 0);
	memcpy(dst, src, size);
	dynamicVertexBuffer->Unlock();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DemoCharacter::setMaterial(ID3DXEffect* effect, grp::IMaterial* material)
{
	PERF_NODE_FUNC();

	assert(effect != NULL);

	setTexture(effect, "g_texDiffuse", material->getTexture(0));
	setTexture(effect, "g_texNormal", material->getTexture(1));
	setTexture(effect, "g_texGloss", material->getTexture(2));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DemoCharacter::setTexture(ID3DXEffect* effect, const char* name, grp::ITexture* texture)
{
	if (texture == NULL || texture->filename() == NULL || texture->filename()[0] == 0)
	{
		return;
	}
	DemoTexture* textureResource = (DemoTexture*)texture->getUserData();
	if (textureResource == NULL)
	{
		return;
	}
	assert(effect != NULL);
	effect->SetTexture(name, textureResource->getD3dTexture());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DemoCharacter::createPartD3dBuffers(grp::IPart* part)
{
	grp::IMesh* mesh = part->getMesh();
	assert(mesh != NULL);

	//vertex buffers
	grp::IVertexStream* staticStream = mesh->getStaticVertexStream();
	assert(staticStream != NULL);
	IDirect3DVertexBuffer9* staticVertexBuffer = createVertexBuffer(mesh, staticStream);
	assert(staticVertexBuffer != NULL);
	void* dstBuffer = NULL;
	staticVertexBuffer->Lock(0, 0, &dstBuffer, 0);
	memcpy(dstBuffer, staticStream->getBuffer(), staticStream->getStride() * mesh->getMaxVertexCount());
	staticVertexBuffer->Unlock();

	if (part->isSkinnedPart())
	{
		grp::IVertexStream* dynamicStream = mesh->getDynamicVertexStream();
		assert(dynamicStream != NULL);
		IDirect3DVertexBuffer9* dynamicVertexBuffer = createVertexBuffer(mesh, dynamicStream);

		if (m_gpuSkinning)
		{
			assert(dynamicVertexBuffer != NULL);
			dynamicVertexBuffer->Lock(0, 0, &dstBuffer, 0);
			memcpy(dstBuffer, dynamicStream->getBuffer(), dynamicStream->getStride() * mesh->getMaxVertexCount());
			dynamicVertexBuffer->Unlock();

			generateSkinVertexBuffer(mesh);
		}
	}
	//index buffers
	for (size_t i = 0; i < mesh->getMeshBufferCount(); ++i)
	{
		grp::IMeshBuffer* buffer = mesh->getMeshBuffer(i);
		createIndexBuffer(buffer);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DemoCharacter::releasePartD3dBuffers(grp::IPart* part)
{
	assert(part != NULL);
	grp::IMesh* mesh = part->getMesh();
	if (mesh == NULL)
	{
		return;
	}
	IDirect3DVertexBuffer9* skinVertexBuffer = (IDirect3DVertexBuffer9*)mesh->getUserData();
	if (skinVertexBuffer != NULL)
	{
		skinVertexBuffer->Release();
		mesh->setUserData(NULL);
	}
	grp::IVertexStream* staticStream = mesh->getStaticVertexStream();
	grp::IVertexStream* dynamicStream = mesh->getDynamicVertexStream();
	if (staticStream != NULL)
	{
		IDirect3DVertexBuffer9* vertexBuffer = (IDirect3DVertexBuffer9*)staticStream->getUserData();
		if (vertexBuffer != NULL)
		{
			vertexBuffer->Release();
			staticStream->setUserData(NULL);
		}
	}
	if (dynamicStream != NULL)
	{
		IDirect3DVertexBuffer9* vertexBuffer = (IDirect3DVertexBuffer9*)dynamicStream->getUserData();
		if (vertexBuffer != NULL)
		{
			vertexBuffer->Release();
			dynamicStream->setUserData(NULL);
		}
	}
	for (size_t i = 0; i < mesh->getMeshBufferCount(); ++i)
	{
		grp::IMeshBuffer* buffer = mesh->getMeshBuffer(i);
		IDirect3DIndexBuffer9* indexBuffer = (IDirect3DIndexBuffer9*)buffer->getUserData();
		if (indexBuffer != NULL)
		{
			indexBuffer->Release();
			buffer->setUserData(NULL);
		}
	}
}

void DemoCharacter::releasePartD3dTextures(grp::IPart* part)
{
	assert(part != NULL);
	for (size_t i = 0; i < part->getMaterialCount(); ++i)
	{
		grp::IMaterial* material = part->getMaterial(i);
		for (size_t j = 0; j < material->getTextureCount(); ++j)
		{
			DemoTexture* texture = (DemoTexture*)material->getTexture(j)->getUserData();
			if (texture != NULL)
			{
				g_resourceManager->dropTexture(texture);
			}
		}
	}	
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DemoCharacter::getUrlBase(const std::wstring& url, std::wstring& baseUrl)
{
	size_t slashPos = url.find_last_of(L'/');
	size_t backSlashPos = url.find_last_of(L'\\');
	if (slashPos == std::wstring::npos)
	{
		if (backSlashPos == std::wstring::npos)
		{
			baseUrl = L"";
		}
		else
		{
			baseUrl = url.substr(0, backSlashPos + 1);
		}
	}
	else
	{
		if (backSlashPos == std::wstring::npos)
		{
			baseUrl = url.substr(0, slashPos + 1);
		}
		else
		{
			baseUrl = url.substr(0, max(slashPos, backSlashPos) + 1);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DemoCharacter::setPartBuffersBuilt(grp::IPart* part)
{
	assert(part != NULL);
	part->setUserData((void*)true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool DemoCharacter::isPartBuffersBuilt(grp::IPart* part) const
{
	assert(part != NULL);
	return (part->getUserData() != NULL);
}

void DemoCharacter::renderMeshNormal(grp::IMesh* mesh)
{
	assert(m_device != NULL);
	assert(mesh != NULL);

	grp::IVertexStream* stream = mesh->getDynamicVertexStream();
	assert(stream != NULL);
	const unsigned char* buffer = stream->getBuffer();
	size_t stride = stream->getStride();

	size_t vertexCount = mesh->getVertexCount();
	//每条法线两个端点
	std::vector<grp::Vector3> normals(vertexCount * 2);
	for (size_t i = 0; i < vertexCount; ++i, buffer += stride)
	{
		const grp::Vector3* position = reinterpret_cast<const grp::Vector3*>(buffer);
		const grp::Vector3* normal = position + 1;
		normals[i * 2] = *position;
		normals[i * 2 + 1] = (*position) + (*normal) * 0.1f;
	}
	m_device->SetFVF(D3DFVF_XYZ);
	m_device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
	m_device->DrawPrimitiveUP(D3DPT_LINELIST, vertexCount, &normals[0], sizeof(grp::Vector3)); 
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DemoCharacter::onPostUpdate(grp::ISkeleton* skeleton)
{
	if (m_ragdoll != NULL)
	{
		m_ragdoll->updateSkeleton();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DemoCharacter::createRagdoll(const grp::Matrix& transform)
{
	assert(m_model != NULL);

	grp::ISkeleton* skeleton = m_model->getSkeleton();
	if (skeleton == NULL)
	{
		return;
	}
	if (m_ragdoll != NULL)
	{
		delete m_ragdoll;
	}
	m_ragdoll = new Ragdoll(skeleton);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DemoCharacter::destroyRagdoll()
{
	if (m_ragdoll != NULL)
	{
		delete m_ragdoll;
		m_ragdoll = NULL;
	}
}
