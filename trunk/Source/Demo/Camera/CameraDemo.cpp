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
#include "StrSafe.h"

#include "Grandpa.h"
#include "MultithreadFileLoader.h"
#include "MultithreadResManager.h"
#include "DemoCharacter.h"
#include "DemoCamera.h"
#include "Performance.h"

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
DemoCamera*				g_camera;
ID3DXEffect*            g_pEffect = NULL;       // D3DX effect interface
CDXUTDialogResourceManager g_DialogResourceManager; // manager for shared resources of dialogs
CDXUTDialog             g_SampleUI;             // dialog for sample specific controls

IDirect3DVertexDeclaration9* g_pVdMesh = NULL;
IDirect3DVertexDeclaration9* g_pVdFloor = NULL;

DemoCharacter* g_character = NULL;
grp::IModel* g_model = NULL;

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
#define IDC_SLIDER_TIMESCALE	13
#define IDC_CHECK_WIREFRAME		14
#define IDC_CHECK_NORMALMAP		15
#define IDC_CHECK_DIFFUSEMAP	22

#define IDC_STATIC_TIMESCALE	19

#define IDC_CHECK_GLOSS			24

#define IDC_RADIO_FIRSTPERSON	25
#define IDC_RADIO_THIRDPERSON	26

#define IDC_BUTTON_RESET		27

#define IDC_BUTTON_ADD_KEY		28
#define IDC_BUTTON_PLAY			29
#define IDC_STATIC_KEY_NUM		30

#define IDC_CHECK_SHAKING		31
#define IDC_SLIDER_POSITION		32
#define IDC_SLIDER_ROTATION		33
#define IDC_SLIDER_FREQUENCY	34

#define IDC_STATIC_POSITION		32
#define IDC_STATIC_ROTATION		33
#define IDC_STATIC_FREQUENCY	34

float g_fTimeScale = 1.0f;

grp::ISpline<grp::Vector3>* g_cameraPositionSpline;
grp::ISpline<grp::Quaternion>* g_cameraRotationSpline;
grp::ISpline<float>* g_cameraViewDistanceSpline;
bool g_playingCameraAnimation = false;
float g_cameraAnimationTime = 0.0f;

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
	g_SampleUI.AddCheckBox(IDC_CHECK_DIFFUSEMAP, L"Diffusemap", 50, iY += 30, 100, 22, true);
	g_SampleUI.AddCheckBox(IDC_CHECK_NORMALMAP, L"Normalmap", 50, iY += 20, 100, 22, true);
	g_SampleUI.AddCheckBox(IDC_CHECK_GLOSS, L"Glossmap", 50, iY += 20, 100, 22, true);
	g_SampleUI.AddCheckBox(IDC_CHECK_WIREFRAME, L"Wireframe", 50, iY += 20, 100, 22);
	g_SampleUI.AddRadioButton(IDC_RADIO_FIRSTPERSON, 0, L"First Person", 50, iY += 30, 100, 22, false);
	g_SampleUI.AddRadioButton(IDC_RADIO_THIRDPERSON, 0, L"Third Person", 50, iY += 20, 100, 22, true);
	
	g_SampleUI.AddStatic(IDC_STATIC_KEY_NUM, L"Total Key: 0", 50, iY += 30, 100, 22);
	g_SampleUI.AddButton(IDC_BUTTON_ADD_KEY, L"Add Key", 50, iY += 25, 100, 22);
	g_SampleUI.AddButton(IDC_BUTTON_PLAY, L"Play", 50, iY += 25, 100, 22);
	g_SampleUI.AddButton(IDC_BUTTON_RESET, L"Reset", 50, iY += 25, 100, 22);

	g_SampleUI.AddCheckBox(IDC_CHECK_SHAKING, L"Enable Shaking", 50, iY += 30, 100, 22);
	g_SampleUI.AddStatic(IDC_STATIC_POSITION, L"Position:", 50, iY += 20, 100, 22);
	g_SampleUI.AddSlider(IDC_SLIDER_POSITION, 50, iY +=20, 100, 22, 0, 50, 5);
	g_SampleUI.AddStatic(IDC_STATIC_ROTATION, L"Rotation:", 50, iY += 20, 100, 22);
	g_SampleUI.AddSlider(IDC_SLIDER_ROTATION, 50, iY +=20, 100, 22, 0, 50, 5);
	g_SampleUI.AddStatic(IDC_STATIC_FREQUENCY, L"Frequency:", 50, iY += 20, 100, 22);
	g_SampleUI.AddSlider(IDC_SLIDER_FREQUENCY, 50, iY +=20, 100, 22, 1, 100, 50);
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

void setCameraHotKeys()
{
	g_camera->setHotKey(DemoCamera::KEY_MOVE_LEFT, 'A');
	g_camera->setHotKey(DemoCamera::KEY_MOVE_RIGHT, 'D');
	g_camera->setHotKey(DemoCamera::KEY_MOVE_FORWARD, 'W');
	g_camera->setHotKey(DemoCamera::KEY_MOVE_BACKWARD, 'S');
	g_camera->setHotKey(DemoCamera::KEY_MOVE_UP, 'E');
	g_camera->setHotKey(DemoCamera::KEY_MOVE_DOWN, 'Q');
	g_camera->setHotKey(DemoCamera::KEY_ROTATE_INC_YAW, VK_LEFT);
	g_camera->setHotKey(DemoCamera::KEY_ROTATE_DEC_YAW, VK_RIGHT);
	g_camera->setHotKey(DemoCamera::KEY_ROTATE_INC_PITCH, VK_DOWN);
	g_camera->setHotKey(DemoCamera::KEY_ROTATE_DEC_PITCH, VK_UP);
	g_camera->setHotKey(DemoCamera::KEY_ROTATE_INC_ROLL, 'Z');
	g_camera->setHotKey(DemoCamera::KEY_ROTATE_DEC_ROLL, 'C');
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

	g_fileLoader = new MultithreadFileLoader(pd3dDevice);
	g_fileLoader->enableMultithread(false);
	g_resourceManager = new MultithreadResManager(g_fileLoader);

	grp::initialize(NULL, g_fileLoader, NULL, g_resourceManager);

	g_camera = new DemoCamera;

	setCameraHotKeys();
	g_camera->setControlMode(DemoCamera::THIRD_PERSON);
	g_camera->setViewDistance(3.0f);
	g_camera->setYaw(3.1415926f);
#if (DEMO_RIGHT_HAND_COORD)
	g_camera->setViewParams(grp::Vector3(-2.0f, -2.0f, 1.0f),
							grp::Vector3(0.0f, 0.0f, 1.0f),
							grp::Vector3(0.0f, 0.0f, 1.0f));
	g_camera->setCoordinateSystem(DemoCamera::RIGHT_HAND);
	g_camera->setUpAxis(DemoCamera::Z_UP);
#else
	g_camera->setViewParams(grp::Vector3(-2.0f, 1.0f, -2.0f),
							grp::Vector3(0.0f, 1.0f, 0.0f),
							grp::Vector3(0.0f, 1.0f, 0.0f));
#endif
	// Setup the camera's projection parameters
	float fAspectRatio = pBackBufferSurfaceDesc->Width / (FLOAT)pBackBufferSurfaceDesc->Height;
	g_camera->setPerspectiveParams(D3DX_PI/4, fAspectRatio, 0.001f, 200.0f);


	g_character = new DemoCharacter(L"Test/warrior.gmd", pd3dDevice);
	g_model = g_character->getModel();
	if (g_model == NULL)
	{
		return E_FAIL;
	}
#if (DEMO_RIGHT_HAND_COORD)
	grp::Matrix coordTransform(grp::Matrix::IDENTITY);
#else
	grp::Matrix coordTransform(grp::Matrix::ZERO);
	coordTransform._11 = 1.0f;
	coordTransform._23 = 1.0f;
	coordTransform._32 = 1.0f;
	coordTransform._44 = 1.0f;
#endif

	g_model->setTransform(coordTransform);

	g_model->playAnimation(L"fight", grp::ANIMATION_LOOP, 0, 0);

	g_cameraPositionSpline = grp::createSpline<grp::Vector3>();
	g_cameraRotationSpline = grp::createSpline<grp::Quaternion>();
	g_cameraViewDistanceSpline = grp::createSpline<float>();

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

	//地板
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
	g_camera->setPerspectiveParams(D3DX_PI/3, fAspectRatio, 0.1f, 200.0f);

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

	if (g_playingCameraAnimation)
	{
		assert(g_cameraPositionSpline->keyCount() == g_cameraRotationSpline->keyCount());
		g_cameraAnimationTime += fElapsedTime * g_fTimeScale;
		if (g_cameraAnimationTime > g_cameraPositionSpline->maxTime())
		{
			g_playingCameraAnimation = false;
		}
		grp::Vector3 position = g_cameraPositionSpline->sample(g_cameraAnimationTime);
		grp::Quaternion rotation = g_cameraRotationSpline->sample(g_cameraAnimationTime);

		if (g_camera->getControlMode() == DemoCamera::THIRD_PERSON)
		{
			float viewDistance = g_cameraViewDistanceSpline->sample(g_cameraAnimationTime);
			g_camera->setViewDistance(viewDistance, false);
		}
		g_camera->setEyePos(position);
		grp::Euler euler = rotation.getEuler_zxy();
		g_camera->setEulerAngle(euler);
	}

	g_camera->update(fTime, fElapsedTime);

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
		mProj = reinterpret_cast<const D3DXMATRIXA16&>(g_camera->getProjectionMatrix());
		mView = reinterpret_cast<const D3DXMATRIXA16&>(g_camera->getViewMatrix());

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

		//地板，采用不同变换
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

		//角色
		V(pd3dDevice->SetVertexDeclaration(g_pVdMesh));
		V(g_pEffect->SetValue("g_vCameraPos", &g_camera->getEyePos(), sizeof(grp::Vector3)));
		//g_vLightDir = D3DXVECTOR3(cosf(fTime), sinf(fTime), 0.0f);
		V(g_pEffect->SetValue("g_vLightDir", &g_vLightDir, sizeof(D3DXVECTOR3)));

		V(g_pEffect->SetBool("g_bDiffuse",
			g_SampleUI.GetCheckBox(IDC_CHECK_DIFFUSEMAP)->GetChecked()));
		V(g_pEffect->SetBool("g_bGloss",
			g_SampleUI.GetCheckBox(IDC_CHECK_GLOSS)->GetChecked()));

		if (g_SampleUI.GetCheckBox(IDC_CHECK_NORMALMAP)->GetChecked())
		{
			V(g_pEffect->SetTechnique("Normalmap"));
		}
		else
		{
			V(g_pEffect->SetTechnique("VertexLight"));
		}

		if (g_character != NULL)
		{
			UINT cPasses;
			g_pEffect->Begin(&cPasses, 0);
			g_character->render(g_pEffect, mView, mProj);
			g_pEffect->End();
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

	if (g_playingCameraAnimation)
	{
		return 0;
	}
	// Pass all remaining windows messages to camera so it can respond to user input
	int mouseX = (short)LOWORD(lParam);
	int mouseY = (short)HIWORD(lParam);
	if (uMsg == WM_RBUTTONDOWN)
	{
		::SetCapture(hWnd);
		g_camera->onMouseDown(mouseX, mouseY);
	}
	else if (uMsg == WM_RBUTTONUP)
	{
		::ReleaseCapture();
		g_camera->onMouseUp();
	}
	else if (uMsg == WM_MOUSEMOVE)
	{
		g_camera->onMouseMove(mouseX, mouseY);
	}
	else if (uMsg == WM_MOUSEWHEEL)
	{
		g_camera->onMouseWheel((short)HIWORD(wParam));
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
	if (g_playingCameraAnimation)
	{
		return;
	}
	if (bKeyDown)
	{
		switch(nChar)
		{
		case VK_F1:
			g_bShowHelp = !g_bShowHelp;
			break;
		}
		g_camera->onKeyDown(nChar);
	}
	else
	{
		g_camera->onKeyUp(nChar);
	}
}

void shakeCamera()
{
	float posAmplitude = g_SampleUI.GetSlider(IDC_SLIDER_POSITION)->GetValue() * 0.01f;
	float dirAmplitude = g_SampleUI.GetSlider(IDC_SLIDER_ROTATION)->GetValue() * 0.005f;
	float frequency = (float)g_SampleUI.GetSlider(IDC_SLIDER_FREQUENCY)->GetValue();
	g_camera->startShaking(grp::Vector3(posAmplitude, posAmplitude, posAmplitude),
						  grp::Euler(dirAmplitude, dirAmplitude, dirAmplitude),
						  frequency);
	wchar_t str[64];
	swprintf(str, sizeof(str)/sizeof(str[0]), L"Position: %.2f", posAmplitude);
	g_SampleUI.GetStatic(IDC_STATIC_POSITION)->SetText(str);
	swprintf(str, sizeof(str)/sizeof(str[0]), L"Rotation: %.2f", dirAmplitude);
	g_SampleUI.GetStatic(IDC_STATIC_ROTATION)->SetText(str);
	swprintf(str, sizeof(str)/sizeof(str[0]), L"Frequency: %.2f", frequency);
	g_SampleUI.GetStatic(IDC_STATIC_FREQUENCY)->SetText(str);
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

	case IDC_RADIO_FIRSTPERSON:
		if (g_camera->getControlMode() != DemoCamera::FIRST_PERSON)
		{
			g_cameraPositionSpline->clear();
			g_cameraRotationSpline->clear();
			g_cameraViewDistanceSpline->clear();
			g_playingCameraAnimation = false;
			g_SampleUI.GetStatic(IDC_STATIC_KEY_NUM)->SetText(L"Total Key: 0");
			g_camera->setControlMode(DemoCamera::FIRST_PERSON);
		}
		break;

	case IDC_RADIO_THIRDPERSON:
		if (g_camera->getControlMode() != DemoCamera::THIRD_PERSON)
		{
			g_cameraPositionSpline->clear();
			g_cameraRotationSpline->clear();
			g_cameraViewDistanceSpline->clear();
			g_playingCameraAnimation = false;
			g_SampleUI.GetStatic(IDC_STATIC_KEY_NUM)->SetText(L"Total Key: 0");
			g_camera->setControlMode(DemoCamera::THIRD_PERSON);
			#if (DEMO_RIGHT_HAND_COORD)
				g_camera->setLookAtPos(grp::Vector3(0,0,1));
			#else
				g_camera->setLookAtPos(grp::Vector3(0,1,0));
			#endif
		}
		break;

	case IDC_BUTTON_RESET:
		g_cameraPositionSpline->clear();
		g_cameraRotationSpline->clear();
		g_cameraViewDistanceSpline->clear();
		g_playingCameraAnimation = false;
		g_SampleUI.GetStatic(IDC_STATIC_KEY_NUM)->SetText(L"Total Key: 0");
		break;

	case IDC_BUTTON_ADD_KEY:
		{
			size_t keyCount = g_cameraPositionSpline->keyCount();
			g_cameraPositionSpline->addKey(g_camera->getEyePos(), (float)keyCount / 2);
			grp::Quaternion r;
			r.setEuler_zxy(g_camera->getEulerAngle());
			g_cameraRotationSpline->addKey(r, (float)keyCount / 2);
			if (g_camera->getControlMode() == DemoCamera::THIRD_PERSON)
			{
				g_cameraViewDistanceSpline->addKey(g_camera->getViewDistance(), (float)keyCount / 2);
			}
			wchar_t str[64];
			swprintf(str, sizeof(str)/sizeof(str[0]), L"Total Key: %d", keyCount + 1);
			g_SampleUI.GetStatic(IDC_STATIC_KEY_NUM)->SetText(str);
		}
		break;

	case IDC_BUTTON_PLAY:
		{
			if (g_cameraPositionSpline->keyCount() < 2)
			{
				::MessageBox(NULL, L"Require at least 2 keys.", L"Info", MB_OK | MB_ICONINFORMATION);
				return;
			}
			g_cameraPositionSpline->buildKnots();
			g_cameraRotationSpline->buildKnots();
			if (g_cameraViewDistanceSpline->keyCount() >= 2)
			{
				g_cameraViewDistanceSpline->buildKnots();
			}
			g_playingCameraAnimation = true;
			g_cameraAnimationTime = 0.0f;
		}
		break;

	case IDC_CHECK_SHAKING:
		if (g_SampleUI.GetCheckBox(IDC_CHECK_SHAKING)->GetChecked())
		{
			shakeCamera();
		}
		else
		{
			g_camera->endShaking();
		}
		break;
		
	case IDC_SLIDER_POSITION:
	case IDC_SLIDER_ROTATION:
	case IDC_SLIDER_FREQUENCY:
		if (g_SampleUI.GetCheckBox(IDC_CHECK_SHAKING)->GetChecked())
		{
			shakeCamera();
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
	//CDXUTDirectionWidget::StaticOnD3D9LostDevice();
	if(g_pFont)
		g_pFont->OnLostDevice();
	if(g_pEffect)
		g_pEffect->OnLostDevice();
	SAFE_RELEASE(g_pSprite);

	SAFE_RELEASE(g_pTexFloor);

	SAFE_RELEASE(g_pVbFloor);

	SAFE_RELEASE(g_pVdFloor);
	SAFE_RELEASE(g_pVdMesh);
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

	grp::destroySpline(g_cameraPositionSpline);
	grp::destroySpline(g_cameraRotationSpline);
	grp::destroySpline(g_cameraViewDistanceSpline);

	SAFE_RELEASE(g_pEffect);
	SAFE_RELEASE(g_pFont);

	SAFE_RELEASE(g_pTexFloor);

	SAFE_DELETE(g_character);
	g_model = NULL;

	SAFE_DELETE(g_fileLoader);
	SAFE_DELETE(g_resourceManager);

	delete g_camera;
	g_camera = NULL;

	grp::destroy();

	SAFE_RELEASE(g_pVbFloor);

	SAFE_RELEASE(g_pVdFloor);
	SAFE_RELEASE(g_pVdMesh);
}
