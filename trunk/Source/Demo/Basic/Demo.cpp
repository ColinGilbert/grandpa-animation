//--------------------------------------------------------------------------------------
// File: Demo.cpp
//
// This sample shows a simple example of the Microsoft Direct3D's High-Level 
// Shader Language (HLSL) using the Effect interface. 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#include "DXUT.h"
#include "DXUTgui.h"
#include "SDKmisc.h"
#include "resource.h"

#include "Grandpa.h"
#include "Performance.h"

#include "MultithreadFileLoader.h"
#include "MultithreadResManager.h"
#include "DemoCharacter.h"
#include "DemoCamera.h"

#define DEMO_RIGHT_HAND_COORD  1

MultithreadFileLoader*	g_fileLoader = NULL;
MultithreadResManager*	g_resourceManager = NULL;

//#define DEBUG_VS   // Uncomment this line to debug vertex shaders 
//#define DEBUG_PS   // Uncomment this line to debug pixel shaders 

//--------------------------------------------------------------------------------------
// Global variables
//--------------------------------------------------------------------------------------
ID3DXFont*              g_pFont = NULL;         // Font for drawing text
ID3DXSprite*            g_pSprite = NULL;       // Sprite for batching draw text calls
bool                    g_bShowHelp = true;     // If true, it renders the UI control text

DemoCamera				g_camera;
ID3DXEffect*            g_pEffect = NULL;       // D3DX effect interface
CDXUTDialogResourceManager g_DialogResourceManager; // manager for shared resources of dialogs
CDXUTDialog             g_SampleUI;             // dialog for sample specific controls

IDirect3DVertexDeclaration9* g_pVdMesh = NULL;
IDirect3DVertexDeclaration9* g_pVdMeshGpuSkinning = NULL;
IDirect3DVertexDeclaration9* g_pVdFloor = NULL;

DemoCharacter* g_character = NULL;
grp::IModel* g_model = NULL;

const float	WALK_SPEED = 0.5f;
const float RUN_SPEED = 0.95f;
float g_fCurSpeed = 0;

float g_fAmbient = 0.3f;
float g_fDiffuse = 0.8f;

#if (DEMO_RIGHT_HAND_COORD)
D3DXVECTOR3	g_vLightDir = D3DXVECTOR3(0.0f, -0.707f, 0.707f);
#else
D3DXVECTOR3	g_vLightDir = D3DXVECTOR3(0.0f, 0.707f, -0.707f);
#endif

IDirect3DVertexBuffer9* g_pVbFloor = NULL;

const float FLOOR_SIZE = 5.0f;

#if (DEMO_RIGHT_HAND_COORD)
float g_aVertexFloor[] = 
{
	-FLOOR_SIZE, 0.0f, -FLOOR_SIZE, 0.0f, 1.0f,
	 FLOOR_SIZE, 0.0f, -FLOOR_SIZE, 1.0f, 1.0f,
	-FLOOR_SIZE, 0.0f,  FLOOR_SIZE, 0.0f, 0.0f,
	 FLOOR_SIZE, 0.0f, -FLOOR_SIZE, 1.0f, 1.0f,
	 FLOOR_SIZE, 0.0f,  FLOOR_SIZE, 1.0f, 0.0f,
	-FLOOR_SIZE, 0.0f,  FLOOR_SIZE, 0.0f, 0.0f
};
#else
float g_aVertexFloor[] = 
{
	-FLOOR_SIZE, -FLOOR_SIZE, 0.0f, 0.0f, 1.0f,
	 FLOOR_SIZE, -FLOOR_SIZE, 0.0f, 1.0f, 1.0f,
	-FLOOR_SIZE,  FLOOR_SIZE, 0.0f, 0.0f, 0.0f,
	 FLOOR_SIZE, -FLOOR_SIZE, 0.0f, 1.0f, 1.0f,
	 FLOOR_SIZE,  FLOOR_SIZE, 0.0f, 1.0f, 0.0f,
	-FLOOR_SIZE,  FLOOR_SIZE, 0.0f, 0.0f, 0.0f
};
#endif

IDirect3DTexture9* g_pTexFloor = NULL;

//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
#define IDC_STATIC_WEIGHT		10
#define IDC_SLIDER_WEIGHT		11
#define IDC_SLIDER_TIMESCALE	13
#define IDC_CHECK_WIREFRAME		14
#define IDC_CHECK_NORMALMAP		15
#define IDC_CHECK_DIFFUSEMAP	22
#define IDC_CHECK_WEIGHTLOD		16
#define IDC_CHECK_ANIMATIONLOD	17
#define IDC_CHECK_SKELETONLOD	18

#define IDC_STATIC_TIMESCALE	19

#define IDC_CHECK_GLOSS			24

#define IDC_BUTTON_ATTACK		36

#define IDC_CHECK_GPU_SKINNING	37

#define IDC_CHECK_DRAW_NORMAL	38

#define IDC_CHECK_DRAW_BOUNDINGBOX 39

#define IDC_CHECK_RAGDOLL		40

#define IDC_CHECK_DRAW_SKELETON	41

float g_fTimeScale = 1.0f;

//--------------------------------------------------------------------------------------
// Forward declarations 
//--------------------------------------------------------------------------------------
bool    CALLBACK IsDeviceAcceptable(D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, bool bWindowed, void* pUserContext);
bool    CALLBACK ModifyDeviceSettings(DXUTDeviceSettings* pDeviceSettings, void* pUserContext);
HRESULT CALLBACK OnCreateDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext);
HRESULT CALLBACK OnResetDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext);
void    CALLBACK OnFrameMove(double fTime, float fElapsedTime, void* pUserContext);
void    CALLBACK OnFrameRender(IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext);
LRESULT CALLBACK MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing, void* pUserContext);
void    CALLBACK KeyboardProc(UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext);
void    CALLBACK OnGUIEvent(UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext);
void    CALLBACK OnLostDevice(void* pUserContext);
void    CALLBACK OnDestroyDevice(void* pUserContext);

void    InitApp();
void    RenderText(double fTime);

void setAnimationWeight()
{
	if (g_model == NULL)
	{
		return;
	}
#define SET_ANIMATION_WEIGHT(a, w)	\
	{\
		grp::IAnimation* animation = g_model->findAnimation(a);\
		if (animation != NULL)\
		{\
			animation->setWeight(w);\
		}\
	}

	float weight = g_SampleUI.GetSlider(IDC_SLIDER_WEIGHT)->GetValue() / 100.0f;
	if (weight < 0.5f)
	{
		SET_ANIMATION_WEIGHT(L"fight", 2 * (0.5f - weight));
		SET_ANIMATION_WEIGHT(L"walk", 2 * weight);
		SET_ANIMATION_WEIGHT(L"run", 0.0f);
		g_fCurSpeed = weight * WALK_SPEED * 2;
	}
	else
	{
		SET_ANIMATION_WEIGHT(L"fight", 0.0f);
		SET_ANIMATION_WEIGHT(L"walk", 2 * (1.0f - weight));
		SET_ANIMATION_WEIGHT(L"run", 2 * (weight - 0.5f));
		g_fCurSpeed = 2 * (WALK_SPEED * (1.0f - weight) + RUN_SPEED * (weight - 0.5f));
	}
}

//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
INT WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int)
{
	// enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	// Set the callback functions. These functions allow DXUT to notify
	// the application about device changes, user input, and windows messages.  The 
	// callbacks are optional so you need only set callbacks for events you're interested 
	// in. However, if you don't handle the device reset/lost callbacks then the sample 
	// framework won't be able to reset your device since the application must first 
	// release all device resources before resetting.  Likewise, if you don't handle the 
	// device created/destroyed callbacks then DXUT won't be able to 
	// recreate your device resources.
	DXUTSetCallbackD3D9DeviceAcceptable(IsDeviceAcceptable);
	DXUTSetCallbackD3D9DeviceCreated(OnCreateDevice);
	DXUTSetCallbackD3D9DeviceReset(OnResetDevice);
	DXUTSetCallbackD3D9FrameRender(OnFrameRender);
	DXUTSetCallbackD3D9DeviceLost(OnLostDevice);
	DXUTSetCallbackD3D9DeviceDestroyed(OnDestroyDevice);
	DXUTSetCallbackMsgProc(MsgProc);
	DXUTSetCallbackKeyboard(KeyboardProc);
	DXUTSetCallbackFrameMove(OnFrameMove);
	DXUTSetCallbackDeviceChanging(ModifyDeviceSettings);

	// Show the cursor and clip it when in full screen
	DXUTSetCursorSettings(true, true);

	InitApp();

	// Initialize DXUT and create the desired Win32 window and Direct3D 
	// device for the application. Calling each of these functions is optional, but they
	// allow you to set several options which control the behavior of the framework.
	DXUTInit(true, true); // Parse the command line and show msgboxes
	DXUTSetHotkeyHandling(true, true, true);
	DXUTCreateWindow(L"Demo");
	DXUTCreateDevice(true, 640, 480);

	// Pass control to DXUT for handling the message pump and 
	// dispatching render calls. DXUT will call your FrameMove 
	// and FrameRender callback when there is idle time between handling window messages.
	DXUTMainLoop();

	// Perform any application-level cleanup here. Direct3D device resources are released within the
	// appropriate callback functions and therefore don't require any cleanup code here.

	return DXUTGetExitCode();
}


//--------------------------------------------------------------------------------------
// Initialize the app 
//--------------------------------------------------------------------------------------
void InitApp()
{
	// Initialize dialogs
	g_SampleUI.Init(&g_DialogResourceManager);

	int iY = 10;
	g_SampleUI.SetCallback(OnGUIEvent); iY = 10; 

	g_SampleUI.AddStatic(IDC_STATIC_TIMESCALE, L"Time scale", 50, iY, 100, 24);
	g_SampleUI.AddSlider(IDC_SLIDER_TIMESCALE, 50, iY += 16, 100, 22);
	g_SampleUI.AddStatic(IDC_STATIC_WEIGHT, L"idle / Walk / run", 50, iY += 20, 100, 24);
	g_SampleUI.AddSlider(IDC_SLIDER_WEIGHT, 50, iY += 16, 100, 22, 0, 100, 0);

	g_SampleUI.AddCheckBox(IDC_CHECK_RAGDOLL, L"Ragdoll", 50, iY += 20, 100, 22, false);

	g_SampleUI.AddCheckBox(IDC_CHECK_GPU_SKINNING, L"GPU Skinning", 50, iY += 30, 100, 22, false);

	g_SampleUI.AddCheckBox(IDC_CHECK_DIFFUSEMAP, L"Diffusemap", 50, iY += 30, 100, 22, true);
	g_SampleUI.AddCheckBox(IDC_CHECK_NORMALMAP, L"Normalmap", 50, iY += 20, 100, 22, true);
	g_SampleUI.AddCheckBox(IDC_CHECK_GLOSS, L"Glossmap", 50, iY += 20, 100, 22, true);
	g_SampleUI.AddCheckBox(IDC_CHECK_WIREFRAME, L"Wireframe", 50, iY += 20, 100, 22);
	g_SampleUI.AddCheckBox(IDC_CHECK_DRAW_NORMAL, L"Draw Normal", 50, iY += 20, 100, 22, false);
	g_SampleUI.AddCheckBox(IDC_CHECK_DRAW_SKELETON, L"Draw Skeleton", 50, iY += 20, 100, 22, false);
	
	g_SampleUI.AddCheckBox(IDC_CHECK_DRAW_BOUNDINGBOX, L"Draw Bounding", 50, iY += 20, 100, 22, false);

	g_SampleUI.AddButton(IDC_BUTTON_ATTACK, L"Attack", 50, iY += 25, 100, 22);
}


//--------------------------------------------------------------------------------------
// Called during device initialization, this code checks the device for some 
// minimum set of capabilities, and rejects those that don't pass by returning E_FAIL.
//--------------------------------------------------------------------------------------
bool CALLBACK IsDeviceAcceptable(D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, 
								 D3DFORMAT BackBufferFormat, bool bWindowed, void* pUserContext)
{
	// No fallback defined by this app, so reject any device that 
	// doesn't support at least ps2.0
	if(pCaps->PixelShaderVersion < D3DPS_VERSION(2,0))
		return false;

	// Skip backbuffer formats that don't support alpha blending
	IDirect3D9* pD3D = DXUTGetD3D9Object(); 
	if(FAILED(pD3D->CheckDeviceFormat(pCaps->AdapterOrdinal, pCaps->DeviceType,
		AdapterFormat, D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING, 
		D3DRTYPE_TEXTURE, BackBufferFormat)))
		return false;

	return true;
}


//--------------------------------------------------------------------------------------
// This callback function is called immediately before a device is created to allow the 
// application to modify the device settings. The supplied pDeviceSettings parameter 
// contains the settings that the framework has selected for the new device, and the 
// application can make any desired changes directly to this structure.  Note however that 
// DXUT will not correct invalid device settings so care must be taken 
// to return valid device settings, otherwise IDirect3D9::CreateDevice() will fail.  
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings(DXUTDeviceSettings* pDeviceSettings, void* pUserContext)
{
	assert(DXUT_D3D9_DEVICE == pDeviceSettings->ver);

	HRESULT hr;
	IDirect3D9* pD3D = DXUTGetD3D9Object();
	D3DCAPS9 caps;

	V(pD3D->GetDeviceCaps(pDeviceSettings->d3d9.AdapterOrdinal,
		pDeviceSettings->d3d9.DeviceType,
		&caps));

	// If device doesn't support HW T&L or doesn't support 1.1 vertex shaders in HW 
	// then switch to SWVP.
	if((caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) == 0 ||
		caps.VertexShaderVersion < D3DVS_VERSION(1,1))
	{
		pDeviceSettings->d3d9.BehaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	}

	// Debugging vertex shaders requires either REF or software vertex processing 
	// and debugging pixel shaders requires REF.  
#ifdef DEBUG_VS
	if(pDeviceSettings->d3d9.DeviceType != D3DDEVTYPE_REF)
	{
		pDeviceSettings->d3d9.BehaviorFlags &= ~D3DCREATE_HARDWARE_VERTEXPROCESSING;
		pDeviceSettings->d3d9.BehaviorFlags &= ~D3DCREATE_PUREDEVICE;                            
		pDeviceSettings->d3d9.BehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	}
#endif
#ifdef DEBUG_PS
	pDeviceSettings->d3d9.DeviceType = D3DDEVTYPE_REF;
#endif
	// For the first device created if its a REF device, optionally display a warning dialog box
	static bool s_bFirstTime = true;
	if(s_bFirstTime)
	{
		s_bFirstTime = false;
		if(pDeviceSettings->d3d9.DeviceType == D3DDEVTYPE_REF)
			DXUTDisplaySwitchingToREFWarning(pDeviceSettings->ver);
	}

	return true;
}

//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has been 
// created, which will happen during application initialization and windowed/full screen 
// toggles. This is the best location to create D3DPOOL_MANAGED resources since these 
// resources need to be reloaded whenever the device is destroyed. Resources created  
// here should be released in the OnDestroyDevice callback. 
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnCreateDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext)
{
#ifdef _PERFORMANCE
	::SetThreadAffinityMask(::GetCurrentThread(), 1);
	PerfManager::createTheOne();
	grp::setProfiler(PerfManager::getTheOne());
#endif

	HRESULT hr;

	V_RETURN(g_DialogResourceManager.OnD3D9CreateDevice(pd3dDevice));
	// Initialize the font
	V_RETURN(D3DXCreateFont(pd3dDevice, 15, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET, 
		OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, 
		L"Arial", &g_pFont));

	// Define DEBUG_VS and/or DEBUG_PS to debug vertex and/or pixel shaders with the 
	// shader debugger. Debugging vertex shaders requires either REF or software vertex 
	// processing, and debugging pixel shaders requires REF.  The 
	// D3DXSHADER_FORCE_*_SOFTWARE_NOOPT flag improves the debug experience in the 
	// shader debugger.  It enables source level debugging, prevents instruction 
	// reordering, prevents dead code elimination, and forces the compiler to compile 
	// against the next higher available software target, which ensures that the 
	// unoptimized shaders do not exceed the shader model limitations.  Setting these 
	// flags will cause slower rendering since the shaders will be unoptimized and 
	// forced into software.  See the DirectX documentation for more information about 
	// using the shader debugger.
	DWORD dwShaderFlags = D3DXFX_NOT_CLONEABLE;

#if defined(DEBUG) || defined(_DEBUG)
	// Set the D3DXSHADER_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	dwShaderFlags |= D3DXSHADER_DEBUG;
#endif

#ifdef DEBUG_VS
	dwShaderFlags |= D3DXSHADER_FORCE_VS_SOFTWARE_NOOPT;
#endif
#ifdef DEBUG_PS
	dwShaderFlags |= D3DXSHADER_FORCE_PS_SOFTWARE_NOOPT;
#endif

	// Read the D3DX effect file
	WCHAR str[MAX_PATH];
	V_RETURN(DXUTFindDXSDKMediaFileCch(str, MAX_PATH, L"Test/Demo.fx"));

	// If this fails, there should be debug output as to 
	// why the .fx file failed to compile
	LPD3DXBUFFER pError = NULL;
	D3DXCreateEffectFromFile(pd3dDevice, str, NULL, NULL, dwShaderFlags, NULL, &g_pEffect, &pError);
	if (NULL != pError)
	{
		char* szErr = (char*)(pError->GetBufferPointer());
		assert(false);
	}

	V(g_pEffect->SetFloat("g_fAmbient", g_fAmbient));
	V(g_pEffect->SetFloat("g_fDiffuse", g_fDiffuse));

	// Setup the camera's view parameters
	g_camera.setControlMode(DemoCamera::THIRD_PERSON);
	g_camera.setViewDistance(3.0f);
	g_camera.setYaw(3.1415926f);
	//g_camera.setRoll(3.14159f/4);
#if (DEMO_RIGHT_HAND_COORD)
	g_camera.setViewParams(grp::Vector3(-2.0f, -2.0f, 1.0f),
							grp::Vector3(0.0f, 0.0f, 1.0f),
							grp::Vector3(0.0f, 0.0f, 1.0f));
	g_camera.setCoordinateSystem(DemoCamera::RIGHT_HAND);
	g_camera.setUpAxis(DemoCamera::Z_UP);
#else
	g_camera.setViewParams(grp::Vector3(-2.0f, 1.0f, -2.0f),
							grp::Vector3(0.0f, 1.0f, 0.0f),
							grp::Vector3(0.0f, 1.0f, 0.0f));
#endif
	// Setup the camera's projection parameters
	float fAspectRatio = pBackBufferSurfaceDesc->Width / (FLOAT)pBackBufferSurfaceDesc->Height;
	g_camera.setPerspectiveParams(D3DX_PI/4, fAspectRatio, 0.001f, 200.0f);

	g_fileLoader = new MultithreadFileLoader(pd3dDevice);
	g_fileLoader->enableMultithread(false);
	g_resourceManager = new MultithreadResManager(g_fileLoader);

	grp::initialize(NULL, g_fileLoader, NULL, g_resourceManager);

	g_character = new DemoCharacter(L"Test/warrior.gmd", pd3dDevice);
	//g_character->setGpuSkinning(true);
	g_model = g_character->getModel();
	if (g_model == NULL)
	{
		return E_FAIL;
	}
	//g_model->enableMeshLod(true);
	//g_model->enableWeightLod(true);
	//g_model->enableSkeletonLod(true);
	//g_model->setLodTolerance(0.1f);

#if (DEMO_RIGHT_HAND_COORD)
	grp::Matrix coordTransform(grp::Matrix::IDENTITY);
#else
	grp::Matrix coordTransform(grp::Matrix::ZERO);
	coordTransform._11 = 1.0f;
	coordTransform._23 = 1.0f;
	coordTransform._32 = 1.0f;
	coordTransform._44 = 1.0f;
#endif

	g_model->setTransform(coordTransform);// * translateMatrix);

	g_model->playAnimation(L"fight", grp::ANIMATION_LOOP, 0, 0);
	g_model->playAnimation(L"walk", grp::ANIMATION_LOOP, 0, 0);
	g_model->playAnimation(L"run", grp::ANIMATION_LOOP, 0, 0);

	setAnimationWeight();

	return S_OK;
}

//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has been 
// reset, which will happen after a lost device scenario. This is the best location to 
// create D3DPOOL_DEFAULT resources since these resources need to be reloaded whenever 
// the device is lost. Resources created here should be released in the OnLostDevice 
// callback. 
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnResetDevice(IDirect3DDevice9* pd3dDevice, 
							   const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext)
{
	HRESULT hr;

	D3DVERTEXELEMENT9 elements[8];

	elements[0].Stream		= 0;
	elements[0].Offset		= 0;
	elements[0].Type		= D3DDECLTYPE_FLOAT3;
	elements[0].Method		= D3DDECLMETHOD_DEFAULT;
	elements[0].Usage		= D3DDECLUSAGE_POSITION;
	elements[0].UsageIndex	= 0;

	elements[1].Stream		= 0;
	elements[1].Offset		= 12;
	elements[1].Type		= D3DDECLTYPE_FLOAT3;
	elements[1].Method		= D3DDECLMETHOD_DEFAULT;
	elements[1].Usage		= D3DDECLUSAGE_NORMAL;
	elements[1].UsageIndex	= 0;

	elements[2].Stream		= 0;
	elements[2].Offset		= 24;
	elements[2].Type		= D3DDECLTYPE_FLOAT3;
	elements[2].Method		= D3DDECLMETHOD_DEFAULT;
	elements[2].Usage		= D3DDECLUSAGE_TANGENT;
	elements[2].UsageIndex	= 0;

	elements[3].Stream		= 0;
	elements[3].Offset		= 36;
	elements[3].Type		= D3DDECLTYPE_FLOAT3;
	elements[3].Method		= D3DDECLMETHOD_DEFAULT;
	elements[3].Usage		= D3DDECLUSAGE_BINORMAL;
	elements[3].UsageIndex	= 0;

	elements[4].Stream		= 1;
	elements[4].Offset		= 0;
	elements[4].Type		= D3DDECLTYPE_FLOAT2;
	elements[4].Method		= D3DDECLMETHOD_DEFAULT;
	elements[4].Usage		= D3DDECLUSAGE_TEXCOORD;
	elements[4].UsageIndex	= 0;

	elements[5].Stream		= 0xff;
	elements[5].Offset		= 0;
	elements[5].Type		= D3DDECLTYPE_UNUSED;
	elements[5].Method		= 0;
	elements[5].Usage		= 0;
	elements[5].UsageIndex	= 0;

	V(pd3dDevice->CreateVertexDeclaration(elements, &g_pVdMesh));

	elements[5].Stream		= 2;
	elements[5].Offset		= 0;
	elements[5].Type		= D3DDECLTYPE_UBYTE4;
	elements[5].Method		= D3DDECLMETHOD_DEFAULT;
	elements[5].Usage		= D3DDECLUSAGE_BLENDINDICES;
	elements[5].UsageIndex	= 0;

	elements[6].Stream		= 2;
	elements[6].Offset		= 4;
	elements[6].Type		= D3DDECLTYPE_FLOAT4;
	elements[6].Method		= D3DDECLMETHOD_DEFAULT;
	elements[6].Usage		= D3DDECLUSAGE_BLENDWEIGHT;
	elements[6].UsageIndex	= 0;

	elements[7].Stream		= 0xff;
	elements[7].Offset		= 0;
	elements[7].Type		= D3DDECLTYPE_UNUSED;
	elements[7].Method		= 0;
	elements[7].Usage		= 0;
	elements[7].UsageIndex	= 0;

	V(pd3dDevice->CreateVertexDeclaration(elements, &g_pVdMeshGpuSkinning));

	elements[0].Stream		= 0;
	elements[0].Offset		= 0;
	elements[0].Type		= D3DDECLTYPE_FLOAT3;
	elements[0].Method		= D3DDECLMETHOD_DEFAULT;
	elements[0].Usage		= D3DDECLUSAGE_POSITION;
	elements[0].UsageIndex	= 0;

	elements[1].Stream		= 0;
	elements[1].Offset		= 12;
	elements[1].Type		= D3DDECLTYPE_FLOAT2;
	elements[1].Method		= D3DDECLMETHOD_DEFAULT;
	elements[1].Usage		= D3DDECLUSAGE_TEXCOORD;
	elements[1].UsageIndex	= 0;

	elements[2].Stream		= 0xff;
	elements[2].Offset		= 0;
	elements[2].Type		= D3DDECLTYPE_UNUSED;
	elements[2].Method		= 0;
	elements[2].Usage		= 0;
	elements[2].UsageIndex	= 0;

	V(pd3dDevice->CreateVertexDeclaration(elements, &g_pVdFloor));

	//�ذ�
	V_RETURN(pd3dDevice->CreateVertexBuffer(6 * (sizeof(D3DXVECTOR3) + sizeof(D3DXVECTOR2)),
											D3DUSAGE_DYNAMIC,
											0,
											D3DPOOL_DEFAULT,
											&g_pVbFloor,
											NULL));

	V_RETURN(g_DialogResourceManager.OnD3D9ResetDevice());
	
	if(g_pFont)
		V_RETURN(g_pFont->OnResetDevice());
	if(g_pEffect)
		V_RETURN(g_pEffect->OnResetDevice());

	// Create a sprite to help batch calls when drawing many lines of text
	V_RETURN(D3DXCreateSprite(pd3dDevice, &g_pSprite));

	V_RETURN(::D3DXCreateTextureFromFile(pd3dDevice, L"Test/floor.dds", &g_pTexFloor));

	g_SampleUI.SetLocation(pBackBufferSurfaceDesc->Width-170, 0);
	g_SampleUI.SetSize(170, 300);

	float fAspectRatio = pBackBufferSurfaceDesc->Width / (FLOAT)pBackBufferSurfaceDesc->Height;
	g_camera.setPerspectiveParams(D3DX_PI/3, fAspectRatio, 0.1f, 200.0f);

	return S_OK;
}

//--------------------------------------------------------------------------------------
// This callback function will be called once at the beginning of every frame. This is the
// best location for your application to handle updates to the scene, but is not 
// intended to contain actual rendering calls, which should instead be placed in the 
// OnFrameRender callback.  
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove(double fTime, float fElapsedTime, void* pUserContext)
{
#ifdef _PERFORMANCE
	PerfManager::getTheOne()->IncrementFrameCounter();

	const DWORD PERF_SAVE_INTERVAL = 2000;

	static DWORD dwLastSaveTime = ::GetTickCount();
	DWORD dwCurTime = ::GetTickCount();
	if (dwCurTime - dwLastSaveTime >= PERF_SAVE_INTERVAL)
	{
		PerfManager::getTheOne()->SaveDataFrame("Demo.perf");
		PerfManager::getTheOne()->Reset();
		dwLastSaveTime = dwCurTime;
	}
#endif

	PERF_NODE_FUNC();

	g_camera.update(fTime, fElapsedTime);

	if (g_character != NULL)
	{
		g_character->update(fTime, g_fTimeScale * fElapsedTime);
	}
	if (g_resourceManager != NULL)
	{
		g_resourceManager->update(fElapsedTime);
	}
}


//--------------------------------------------------------------------------------------
// This callback function will be called at the end of every frame to perform all the 
// rendering calls for the scene, and it will also be called if the window needs to be 
// repainted. After this function has returned, DXUT will call 
// IDirect3DDevice9::Present to display the contents of the next buffer in the swap chain
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameRender(IDirect3DDevice9* pd3dDevice, double fTime, float fElapsedTime, void* pUserContext)
{
	PERF_NODE_FUNC();

	HRESULT hr;

	float *pVertex = NULL;
	V(g_pVbFloor->Lock(0, 0, (void**)&pVertex, 0));
	//�ƶ��ذ�����
	for (int i = 0; i < 6; ++i)
	{
		g_aVertexFloor[i*5+4] += (g_fCurSpeed * fElapsedTime * g_fTimeScale / FLOOR_SIZE * 1.8f);
	}
	memcpy(pVertex, g_aVertexFloor, sizeof(g_aVertexFloor));	
	V(g_pVbFloor->Unlock());

	D3DXMATRIXA16 mWorldViewProjection;
	D3DXMATRIXA16 mWorld;
	D3DXMATRIXA16 mView;
	D3DXMATRIXA16 mProj;
	UINT iPass, cPasses;

	// Clear the render target and the zbuffer 
	V(pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DXCOLOR(0.3f,0.3f,0.6f,0.5f), 1.0f, 0));

	// Render the scene
	if(SUCCEEDED(pd3dDevice->BeginScene()))
	{
		// Get the projection & view matrix from the camera class
		mProj = reinterpret_cast<const D3DXMATRIXA16&>(g_camera.getProjectionMatrix());
		mView = reinterpret_cast<const D3DXMATRIXA16&>(g_camera.getViewMatrix());

		pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
		pd3dDevice->SetRenderState(D3DRS_ALPHAREF, (DWORD)0x00000080);
		pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
		pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);

		if (g_SampleUI.GetCheckBox(IDC_CHECK_WIREFRAME)->GetChecked())
		{
			pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
		}
		else
		{
			pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
		}

		//�ذ壬���ò�ͬ�任
		D3DXMATRIXA16 coordMatrix;
		memset(&coordMatrix, 0, sizeof(D3DXMATRIXA16));
		coordMatrix._11 = 1.0f;
		coordMatrix._23 = 1.0f;
		coordMatrix._32 = 1.0f;
		coordMatrix._44 = 1.0f;
		V(g_pEffect->SetMatrix("g_mWorldViewProjection", &(coordMatrix*mView*mProj)));
		V(g_pEffect->SetMatrix("g_mWorld", &coordMatrix));

		V(pd3dDevice->SetVertexDeclaration(g_pVdFloor));
		V(pd3dDevice->SetStreamSource(0, g_pVbFloor, 0, 20));
		V(g_pEffect->SetTexture("g_texDiffuse", g_pTexFloor));
		V(g_pEffect->SetTechnique("NoLight"));

		V(g_pEffect->Begin(&cPasses, 0));
		for (iPass = 0; iPass < cPasses; iPass++)
		{
			V(g_pEffect->BeginPass(iPass));
			V(pd3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2));
			V(g_pEffect->EndPass());
		}
		V(g_pEffect->End());

		//��ɫ
		if (g_character->isGpuSkinning())
		{
			V(pd3dDevice->SetVertexDeclaration(g_pVdMeshGpuSkinning));
		}
		else
		{
			V(pd3dDevice->SetVertexDeclaration(g_pVdMesh));
		}
		V(g_pEffect->SetValue("g_vCameraPos", &g_camera.getEyePos(), sizeof(grp::Vector3)));
		//g_vLightDir = D3DXVECTOR3(cosf(fTime), sinf(fTime), 0.0f);
		V(g_pEffect->SetValue("g_vLightDir", &g_vLightDir, sizeof(D3DXVECTOR3)));

		V(g_pEffect->SetBool("g_bDiffuse",
			g_SampleUI.GetCheckBox(IDC_CHECK_DIFFUSEMAP)->GetChecked()));
		V(g_pEffect->SetBool("g_bGloss",
			g_SampleUI.GetCheckBox(IDC_CHECK_GLOSS)->GetChecked()));

		if (g_SampleUI.GetCheckBox(IDC_CHECK_NORMALMAP)->GetChecked())
		{
			if (g_character->isGpuSkinning())
			{
				V(g_pEffect->SetTechnique("Normalmap_Gpu_Skinning"));
			}
			else
			{
				V(g_pEffect->SetTechnique("Normalmap"));
			}
		}
		else
		{
			if (g_character->isGpuSkinning())
			{
				V(g_pEffect->SetTechnique("VertexLight_Gpu_Skinning"));
			}
			else
			{
				V(g_pEffect->SetTechnique("VertexLight"));
			}
		}

		if (g_character != NULL)
		{
			bool drawNormal = g_SampleUI.GetCheckBox(IDC_CHECK_DRAW_NORMAL)->GetChecked();
			bool drawSkeleton = g_SampleUI.GetCheckBox(IDC_CHECK_DRAW_SKELETON)->GetChecked();
			bool drawBoundingBox = g_SampleUI.GetCheckBox(IDC_CHECK_DRAW_BOUNDINGBOX)->GetChecked();
			if (!drawSkeleton || g_SampleUI.GetCheckBox(IDC_CHECK_WIREFRAME)->GetChecked())
			{
				UINT cPasses;
				g_pEffect->Begin(&cPasses, 0);
				g_character->render(g_pEffect, mView, mProj);
				g_pEffect->End();
			}
			if (drawNormal)
			{
				g_character->renderNormal(mView, mProj);
			}
			if (drawBoundingBox)
			{
				g_character->renderBoundingBox(mView, mProj);
			}
			if (drawSkeleton)
			{
				g_character->renderSkeleton(mView, mProj);
			}
		}

		if (g_bShowHelp)
		{
			g_SampleUI.OnRender(fElapsedTime);
		}
		RenderText(fTime);

		V(pd3dDevice->EndScene());
	}
}


//--------------------------------------------------------------------------------------
// Render the help and statistics text. This function uses the ID3DXFont interface for 
// efficient text rendering.
//--------------------------------------------------------------------------------------
void RenderText(double fTime)
{
	// The helper object simply helps keep track of text position, and color
	// and then it calls pFont->DrawText(m_pSprite, strMsg, -1, &rc, DT_NOCLIP, m_clr);
	// If NULL is passed in as the sprite object, then it will work fine however the 
	// pFont->DrawText() will not be batched together.  Batching calls will improves perf.
	CDXUTTextHelper txtHelper(g_pFont, g_pSprite, 15);

	// Output statistics
	txtHelper.Begin();
	txtHelper.SetInsertionPos(2, 0);
	txtHelper.SetForegroundColor(D3DXCOLOR(1.0f, 1.0f, 0.0f, 1.0f));
	txtHelper.DrawTextLine(DXUTGetFrameStats(DXUTIsVsyncEnabled()));
	txtHelper.End();
}


//--------------------------------------------------------------------------------------
// Before handling window messages, DXUT passes incoming windows 
// messages to the application through this callback function. If the application sets 
// *pbNoFurtherProcessing to TRUE, then DXUT will not process this message.
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing, void* pUserContext)
{
	// Always allow dialog resource manager calls to handle global messages
	// so GUI state is updated correctly
	*pbNoFurtherProcessing = g_DialogResourceManager.MsgProc(hWnd, uMsg, wParam, lParam);
	if(*pbNoFurtherProcessing)
		return 0;

	*pbNoFurtherProcessing = g_SampleUI.MsgProc(hWnd, uMsg, wParam, lParam);
	if(*pbNoFurtherProcessing)
		return 0;

	// Pass all remaining windows messages to camera so it can respond to user input
	//g_camera.HandleMessages(hWnd, uMsg, wParam, lParam);
	int mouseX = (short)LOWORD(lParam);
	int mouseY = (short)HIWORD(lParam);
	if (uMsg == WM_RBUTTONDOWN)
	{
		::SetCapture(hWnd);
		g_camera.onMouseDown(mouseX, mouseY);
	}
	else if (uMsg == WM_RBUTTONUP)
	{
		::ReleaseCapture();
		g_camera.onMouseUp();
	}
	else if (uMsg == WM_MOUSEMOVE)
	{
		g_camera.onMouseMove(mouseX, mouseY);
	}
	else if (uMsg == WM_MOUSEWHEEL)
	{
		g_camera.onMouseWheel((short)HIWORD(wParam));
	}
	return 0;
}

//--------------------------------------------------------------------------------------
// As a convenience, DXUT inspects the incoming windows messages for
// keystroke messages and decodes the message parameters to pass relevant keyboard
// messages to the application.  The framework does not remove the underlying keystroke 
// messages, which are still passed to the application's MsgProc callback.
//--------------------------------------------------------------------------------------
void CALLBACK KeyboardProc(UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext)
{
	if(bKeyDown)
	{
		switch(nChar)
		{
		case VK_F1: g_bShowHelp = !g_bShowHelp; break;
		}
	}
}

//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
void CALLBACK OnGUIEvent(UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext)
{

	switch(nControlID)
	{
	case IDC_SLIDER_TIMESCALE:
		{
			int iValue = g_SampleUI.GetSlider(IDC_SLIDER_TIMESCALE)->GetValue();
			if (iValue < 50)
			{
				g_fTimeScale = iValue / 50.0f;
			}
			else
			{
				g_fTimeScale = iValue / 10.0f - 4.0f;
			}
		}
		break;

	case IDC_SLIDER_WEIGHT:
		{
			if (g_model == NULL)
			{
				break;
			}
			setAnimationWeight();
			break;
		}

	case IDC_BUTTON_ATTACK:
		if (g_model != NULL)
		{
			grp::IAnimation* fight = g_model->findAnimation(L"fight");
			if (fight != NULL && fight->getWeight() == 1.0f)
			{
				g_model->playAnimation(L"attack", grp::ANIMATION_SINGLE);
			}
			else
			{
				g_model->playAnimation(L"attack_up", grp::ANIMATION_SINGLE);
			}
		}
		break;
	case IDC_CHECK_RAGDOLL:
		if (g_SampleUI.GetCheckBox(IDC_CHECK_RAGDOLL)->GetChecked())
		{
			g_character->createRagdoll(g_model->getTransform());
		}
		else
		{
			g_character->destroyRagdoll();
		}
		break;
	case IDC_CHECK_GPU_SKINNING:
		{
			bool enable = g_SampleUI.GetCheckBox(IDC_CHECK_GPU_SKINNING)->GetChecked();
			g_character->setGpuSkinning(enable);
		}
		break;
	}
}

//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has 
// entered a lost state and before IDirect3DDevice9::Reset is called. Resources created
// in the OnResetDevice callback should be released here, which generally includes all 
// D3DPOOL_DEFAULT resources. See the "Lost Devices" section of the documentation for 
// information about lost devices.
//--------------------------------------------------------------------------------------
void CALLBACK OnLostDevice(void* pUserContext)
{
	g_DialogResourceManager.OnD3D9LostDevice();
	if(g_pFont)
		g_pFont->OnLostDevice();
	if(g_pEffect)
		g_pEffect->OnLostDevice();
	SAFE_RELEASE(g_pSprite);

	SAFE_RELEASE(g_pTexFloor);

	SAFE_RELEASE(g_pVbFloor);

	SAFE_RELEASE(g_pVdFloor);
	SAFE_RELEASE(g_pVdMesh);
	SAFE_RELEASE(g_pVdMeshGpuSkinning);
}

//--------------------------------------------------------------------------------------
// This callback function will be called immediately after the Direct3D device has 
// been destroyed, which generally happens as a result of application termination or 
// windowed/full screen toggles. Resources created in the OnCreateDevice callback 
// should be released here, which generally includes all D3DPOOL_MANAGED resources. 
//--------------------------------------------------------------------------------------
void CALLBACK OnDestroyDevice(void* pUserContext)
{
	g_DialogResourceManager.OnD3D9DestroyDevice();
	//CDXUTDirectionWidget::StaticOnD3D9DestroyDevice();

	SAFE_RELEASE(g_pEffect);
	SAFE_RELEASE(g_pFont);

	SAFE_RELEASE(g_pTexFloor);

	SAFE_DELETE(g_character);
	g_model = NULL;

	SAFE_DELETE(g_fileLoader);
	SAFE_DELETE(g_resourceManager);

	grp::destroy();

	SAFE_RELEASE(g_pVbFloor);

	SAFE_RELEASE(g_pVdFloor);
	SAFE_RELEASE(g_pVdMesh);
	SAFE_RELEASE(g_pVdMeshGpuSkinning);
}
