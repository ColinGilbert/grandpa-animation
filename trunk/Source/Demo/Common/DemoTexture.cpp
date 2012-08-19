#include "Grandpa.h"
#include "DemoTexture.h"
#include "d3d9.h"
#include "d3dx9.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
DemoTexture::DemoTexture()
	:	m_d3dTexture(NULL)
	,	m_state(grp::RES_STATE_LOADING)
	,	m_referenceCount(0)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
DemoTexture::~DemoTexture()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DemoTexture::build(IDirect3DDevice9* device)
{
	//注意！如果需要多线程加载纹理，创建d3d设备时必须加上D3DCREATE_MULTITHREADED属性
	if (SUCCEEDED(::D3DXCreateTextureFromFile(device, m_url.c_str(), &m_d3dTexture)))
	{
		setResourceState(grp::RES_STATE_COMPLETE);
	}
	else
	{
		setResourceState(grp::RES_STATE_BROKEN);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void DemoTexture::destroy()
{
	if (m_d3dTexture != NULL)
	{
		m_d3dTexture->Release();
	}
	delete this;
}
