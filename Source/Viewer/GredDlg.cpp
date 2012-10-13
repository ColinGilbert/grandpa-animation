// GredDlg.cpp : implementation file
//

#include "Precompiled.h"
#include "Gred.h"
#include "GredDlg.h"
#include "GredModel.h"
#include "Grandpa.h"
#include "MultithreadFileLoader.h"
#include "MultithreadResManager.h"
#include "PathUtil.h"
#include "DemoCharacter.h"
#include "Performance.h"
#include <string>
#include <cassert>
//#include "Mmsystem.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(x) if (x != NULL){ delete (x); (x) = NULL; }
#endif

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x) if (x != NULL){ (x)->Release(); (x) = NULL; }
#endif

MultithreadFileLoader* g_fileLoader = NULL;
MultithreadResManager* g_resourceManager = NULL;

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CGredDlg dialog




CGredDlg::CGredDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGredDlg::IDD, pParent)
	, m_model(NULL)
	, m_modelNode(NULL)
	, m_propertyNode(NULL)
	, m_partNode(NULL)
	, m_animationNode(NULL)
	, m_eventNode(NULL)
	, m_eventParamNode(NULL)
	, m_lastEditList(LIST_NONE)
	, m_lastEditRow(-1)
	, m_lastEditColumn(-1)
	, m_d3d(NULL)
	, m_device(NULL)
	, m_vertexDeclaration(NULL)
	, m_timeScale(50)
	, m_timeScaleString(_T("Time Scale:1.00"))
	, m_paused(false)
	, m_rbuttonPushed(false)
	, m_skeletonName(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
#ifdef _PERFORMANCE
	::SetThreadAffinityMask(::GetCurrentThread(), 1);
	PerfManager::createTheOne();
	grp::setProfiler(PerfManager::getTheOne());
#endif
}

CGredDlg::~CGredDlg()
{
	SAFE_DELETE(m_model);

	SAFE_DELETE(g_fileLoader);
	SAFE_DELETE(g_resourceManager);

	grp::destroy();

	SAFE_RELEASE(m_vertexDeclaration);
	SAFE_RELEASE(m_effect);
	SAFE_RELEASE(m_device);
	SAFE_RELEASE(m_d3d);
}

bool CGredDlg::initDevice()
{
	m_d3d = ::Direct3DCreate9(D3D_SDK_VERSION);
	if (m_d3d == NULL)
	{
		return false;
	}
	D3DPRESENT_PARAMETERS pp;
	::ZeroMemory(&pp, sizeof(pp));
	pp.BackBufferFormat = D3DFMT_X8R8G8B8;
	pp.MultiSampleType = D3DMULTISAMPLE_NONE;
	pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	pp.hDeviceWindow = m_preview.m_hWnd;
	pp.Windowed = TRUE;
	pp.EnableAutoDepthStencil = TRUE;
	pp.AutoDepthStencilFormat = D3DFMT_D24X8;
	pp.BackBufferCount = 1;
	m_d3d->CreateDevice(0,
						D3DDEVTYPE_HAL,
						m_preview.m_hWnd,
						D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED,
						&pp,
						&m_device);
	if (m_device == NULL)
	{
		return false;
	}
	LPD3DXBUFFER error = NULL;
	::D3DXCreateEffectFromFile(m_device, L"./Test/Demo.fx", NULL, NULL, D3DXFX_NOT_CLONEABLE, NULL, &m_effect, &error);
	if (error != NULL)
	{
		return false;
	}

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

	m_device->CreateVertexDeclaration(elements, &m_vertexDeclaration);

	return true;
}

void CGredDlg::initCamera()
{
	m_camera.setControlMode(DemoCamera::THIRD_PERSON);
	m_camera.setProjectionMode(DemoCamera::ORTHO);
	m_camera.setYaw(3.1416f);
	m_camera.setViewParams(grp::Vector3(0, 1, -2.5), grp::Vector3(0, 1, 0), grp::Vector3(0, 1, 0));
	m_camera.setViewDistance(20.0f);
}

void CGredDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MODELS, m_modelComboBox);

	DDX_Control(pDX, IDC_PROPERTIES, m_propertyList);
	DDX_Control(pDX, IDC_PARTS, m_partList);
	DDX_Control(pDX, IDC_PART_PROPERTIES, m_partPropertyList);
	DDX_Control(pDX, IDC_ANIMATIONS, m_animationList);
	DDX_Control(pDX, IDC_EVENTS, m_eventList);
	DDX_Control(pDX, IDC_EVENT_PARAMS, m_eventParamList);

	DDX_Control(pDX, IDC_VIEW, m_preview);
	DDX_Slider(pDX, IDC_TIME_SCALE_SLIDER, m_timeScale);
	DDV_MinMaxInt(pDX, m_timeScale, 0, 100);
	DDX_Text(pDX, IDC_SCALE, m_timeScaleString);
	DDX_Control(pDX, IDC_CUR_TIME, m_timeCtrl);
	DDX_Control(pDX, IDC_PAUSE, m_pauseButton);
	DDX_Control(pDX, IDC_TIME_SLIDER, m_timeSlider);
	DDX_Text(pDX, IDC_EDIT_SKELETON, m_skeletonName);
}

BEGIN_MESSAGE_MAP(CGredDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_CBN_SELCHANGE(IDC_MODELS, &CGredDlg::OnCbnSelchangeModels)
	ON_BN_CLICKED(IDC_BROWSE_MODEL, &CGredDlg::OnBnClickedBrowseModel)
	ON_CBN_EDITCHANGE(IDC_MODELS, &CGredDlg::OnCbnEditchangeModels)
	ON_CBN_SELCHANGE(IDC_SKELETONS, &CGredDlg::OnCbnSelchangeSkeletons)
	ON_CBN_EDITCHANGE(IDC_SKELETONS, &CGredDlg::OnCbnEditchangeSkeletons)
	ON_BN_CLICKED(IDC_BROWSE_SKELETON, &CGredDlg::OnBnClickedBrowseSkeleton)
	ON_NOTIFY(NM_CLICK, IDC_PARTS, &CGredDlg::OnNMClickParts)
	ON_NOTIFY(NM_CLICK, IDC_ANIMATIONS, &CGredDlg::OnNMClickAnimations)
	ON_NOTIFY(NM_CLICK, IDC_EVENTS, &CGredDlg::OnNMClickEvents)
	ON_NOTIFY(NM_DBLCLK, IDC_PARTS, &CGredDlg::OnNMDblclkParts)
	ON_BN_CLICKED(IDC_ADD_PART, &CGredDlg::OnBnClickedAddPart)
	ON_BN_CLICKED(IDC_NEW_PART, &CGredDlg::OnBnClickedNewPart)
	ON_BN_CLICKED(IDC_EDIT_PART, &CGredDlg::OnBnClickedEditPart)
	ON_BN_CLICKED(IDC_REMOVE_PART, &CGredDlg::OnBnClickedRemovePart)
	ON_BN_CLICKED(IDC_ADD_ANIMATION, &CGredDlg::OnBnClickedAddAnimation)
	ON_BN_CLICKED(IDC_REMOVE_ANIMATION, &CGredDlg::OnBnClickedRemoveAnimation)
	ON_BN_CLICKED(IDC_ADD_EVENT, &CGredDlg::OnBnClickedAddEvent)
	ON_BN_CLICKED(IDC_REMOVE_EVENT, &CGredDlg::OnBnClickedRemoveEvent)
	ON_BN_CLICKED(IDC_NEW_MODEL, &CGredDlg::OnBnClickedNewModel)
	ON_BN_CLICKED(IDC_ADD_PROPERTY, &CGredDlg::OnBnClickedAddProperty)
	ON_BN_CLICKED(IDC_REMOVE_PROPERTY, &CGredDlg::OnBnClickedRemoveProperty)
	ON_BN_CLICKED(IDC_ADD_EVENT_PARAM, &CGredDlg::OnBnClickedAddEventParam)
	ON_BN_CLICKED(IDC_REMOVE_EVENT_PARAM, &CGredDlg::OnBnClickedRemoveEventParam)
	ON_NOTIFY(NM_CLICK, IDC_EVENT_PARAMS, &CGredDlg::OnNMClickEventParams)
	ON_WM_TIMER()
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_TIME_SCALE_SLIDER, &CGredDlg::OnNMReleasedcaptureTimeScaleSlider)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_TIME_SLIDER, &CGredDlg::OnNMReleasedcaptureTimeSlider)
	ON_BN_CLICKED(IDC_PAUSE, &CGredDlg::OnBnClickedPause)
	ON_WM_RBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEWHEEL()
END_MESSAGE_MAP()


// CGredDlg message handlers

BOOL CGredDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	initDevice();

	initCamera();

	g_fileLoader = new MultithreadFileLoader(m_device);
	g_resourceManager = new MultithreadResManager(g_fileLoader);
	grp::initialize(NULL, g_fileLoader, NULL, g_resourceManager);

	// TODO: Add extra initialization here
	initPropertyList();
	initPartList();
	initPartPropertyList();
	initAnimationList();
	initEventList();
	initEventParamList();

	updateModelList();

	SetTimer(0, 10, NULL);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CGredDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CGredDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CGredDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CGredDlg::initPropertyList()
{
	m_propertyList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	LVCOLUMN lc;
	lc.mask = LVCF_WIDTH | LVCF_FMT | LVCF_TEXT | LVCF_SUBITEM;
	lc.cx = 68;
	lc.fmt = LVCFMT_LEFT;
	lc.pszText = L"Name";
	lc.cchTextMax = 100;
	lc.iSubItem = 0;
	m_propertyList.InsertColumn(0, &lc);

	lc.cx = 80;
	lc.pszText = L"Value";
	lc.iSubItem = 1;
	m_propertyList.InsertColumn(1, &lc);
}

void CGredDlg::initPartList()
{
	m_partList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	LVCOLUMN lc;
	lc.mask = LVCF_WIDTH | LVCF_FMT | LVCF_TEXT | LVCF_SUBITEM;
	lc.cx = 68;
	lc.fmt = LVCFMT_LEFT;
	lc.pszText = L"Slot";
	lc.cchTextMax = 100;
	lc.iSubItem = 0;
	m_partList.InsertColumn(0, &lc);

	//lc.cx = 25;
	//lc.pszText = L"Visible";
	//lc.iSubItem = 1;
	//m_partList.InsertColumn(1, &lc);

	lc.cx = 220;
	lc.pszText = L"Filename";
	lc.iSubItem = 2;
	m_partList.InsertColumn(2, &lc);

	lc.cx = 48;
	lc.pszText = L"Type";
	lc.iSubItem = 3;
	m_partList.InsertColumn(3, &lc);
}

void CGredDlg::initPartPropertyList()
{
	m_partPropertyList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	LVCOLUMN lc;
	lc.mask = LVCF_WIDTH | LVCF_FMT | LVCF_TEXT | LVCF_SUBITEM;
	lc.cx = 68;
	lc.fmt = LVCFMT_LEFT;
	lc.pszText = L"Name";
	lc.cchTextMax = 100;
	lc.iSubItem = 0;
	m_partPropertyList.InsertColumn(0, &lc);

	lc.cx = 80;
	lc.pszText = L"Value";
	lc.iSubItem = 1;
	m_partPropertyList.InsertColumn(1, &lc);
}

void CGredDlg::initAnimationList()
{
	m_animationList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	LVCOLUMN lc;
	lc.mask = LVCF_WIDTH | LVCF_FMT | LVCF_TEXT | LVCF_SUBITEM;
	lc.cx = 68;
	lc.fmt = LVCFMT_LEFT;
	lc.pszText = L"Slot";
	lc.cchTextMax = 100;
	lc.iSubItem = 0;
	m_animationList.InsertColumn(0, &lc);

	lc.cx = 165;
	lc.pszText = L"Filename";
	lc.iSubItem = 1;
	m_animationList.InsertColumn(1, &lc);

	lc.cx = 36;
	lc.pszText = L"Start";
	lc.iSubItem = 2;
	m_animationList.InsertColumn(2, &lc);

	lc.cx = 36;
	lc.pszText = L"End";
	lc.iSubItem = 3;
	m_animationList.InsertColumn(3, &lc);
}

void CGredDlg::initEventList()
{
	m_eventList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	LVCOLUMN lc;
	lc.mask = LVCF_WIDTH | LVCF_FMT | LVCF_TEXT | LVCF_SUBITEM;
	lc.cx = 68;
	lc.fmt = LVCFMT_LEFT;
	lc.pszText = L"Name";
	lc.cchTextMax = 100;
	lc.iSubItem = 0;
	m_eventList.InsertColumn(0, &lc);

	lc.cx = 80;
	lc.pszText = L"Time";
	lc.iSubItem = 1;
	m_eventList.InsertColumn(1, &lc);
}

void CGredDlg::initEventParamList()
{
	m_eventParamList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	LVCOLUMN lc;
	lc.mask = LVCF_WIDTH | LVCF_FMT | LVCF_TEXT | LVCF_SUBITEM;
	lc.cx = 68;
	lc.fmt = LVCFMT_LEFT;
	lc.pszText = L"Name";
	lc.cchTextMax = 100;
	lc.iSubItem = 0;
	m_eventParamList.InsertColumn(0, &lc);

	lc.cx = 80;
	lc.pszText = L"Value";
	lc.iSubItem = 1;
	m_eventParamList.InsertColumn(1, &lc);
}

void CGredDlg::updateModelList()
{
	m_modelComboBox.ResetContent();

	gred::getModelFileList(m_models);

	int index = 0;
	for (std::vector<std::wstring>::iterator iter = m_models.begin();
		iter != m_models.end();
		++iter)
	{
		m_modelComboBox.InsertString(index++, (*iter).c_str());
	}
	if (m_models.size() > 0)
	{
		m_modelComboBox.SetCurSel(0);
		m_modelFilename = m_models[0];
		refreshModel();
	}
}

void CGredDlg::readModel()
{
	m_propertyNodes.clear();
	m_partNodes.clear();
	m_animationNodes.clear();
	m_eventNodes.clear();
	m_eventParamNodes.clear();

	m_partNode = NULL;
	m_animationNode = NULL;
	m_eventNode = NULL;

	if (!m_modelFile.loadFromFile(m_modelFilename.c_str()))
	{
		return;
	}
	m_modelNode = m_modelFile.findChild(L"model");
	if (m_modelNode == NULL)
	{
		return;
	}
	slim::XmlAttribute* skeletonAttribute = m_modelNode->findAttribute(L"skeleton");
	if (skeletonAttribute == NULL)
	{
		m_skeletonName = L"";
	}
	else
	{
		m_skeletonName = skeletonAttribute->getValue<const wchar_t*>();
	}
	
	m_partNodes.reserve(m_modelNode->getChildCount(L"part"));
	m_animationNodes.reserve(m_modelNode->getChildCount(L"animation"));
	m_propertyNodes.reserve(m_modelNode->getChildCount(L"property"));

	slim::NodeIterator nodeIter;
	for (slim::XmlNode* child = m_modelNode->getFirstChild(nodeIter);
		child != NULL;
		child = m_modelNode->getNextChild(nodeIter))
	{
		if (Strcmp(child->getName(), L"part") == 0)
		{
			m_partNodes.push_back(child);
		}
		else if (Strcmp(child->getName(), L"animation") == 0)
		{
			m_animationNodes.push_back(child);
		}
		else if (Strcmp(child->getName(), L"property") == 0)
		{
			m_propertyNodes.push_back(child);
		}
	}
	UpdateData(FALSE);
}

void CGredDlg::createGrpModel()
{
	if (m_device == NULL)
	{
		return;
	}
	SAFE_DELETE(m_model);
	m_model = new DemoCharacter(m_modelFilename.c_str(), m_device);
	grp::IModel* grpModel = m_model->getModel();
	grp::Matrix coordMatrix(grp::Matrix::ZERO);
	coordMatrix._11 = 1.0f;
	coordMatrix._23 = 1.0f;
	coordMatrix._32 = 1.0f;
	coordMatrix._44 = 1.0f;
	grpModel->setTransform(coordMatrix);
	grpModel->playAnimation(L"stand", grp::ANIMATION_LOOP);
}

void CGredDlg::refreshModel()
{
	createGrpModel();

	readModel();

	refreshPropertyList();
	refreshPartList();
	refreshPartPropertyList();
	refreshAnimationList();
	refreshEventList();
	refreshEventParamList();

	m_camera.setOrthoParams(0.1f, 0.1f, 0.1f, 40.0f);
}

bool CGredDlg::isSkinnedPart(slim::XmlNode* partNode)
{
	slim::XmlDocument doc;

	assert(partNode != NULL);
	slim::XmlAttribute* filenameAttr = partNode->findAttribute(L"filename");
	if (filenameAttr != NULL)
	{
		std::wstring filePath = filenameAttr->getValue<const wchar_t*>();
		if (grp::containFolder(m_modelFilename))
		{
			std::wstring folder;
			grp::getUrlBase(m_modelFilename, folder);
			filePath = folder + filePath;
		}
		if (!doc.loadFromFile(filePath.c_str()))
		{
			return false;
		}
		partNode = doc.findChild(L"part");
	}
	if (partNode == NULL)
	{
		return false;
	}
	return (wcscmp(partNode->readAttribute<const wchar_t*>(L"type", L""), L"skinned") == 0);
}

void CGredDlg::refreshPropertyList()
{
	m_propertyList.DeleteAllItems();
	int index = 0;
	for (int i = 0; i < (int)m_propertyNodes.size(); ++i)
	{
		slim::XmlNode* node = m_propertyNodes[i];
		slim::AttributeIterator iter;
		slim::XmlAttribute* attribute = node->getFirstAttribute(iter);
		if (attribute == NULL)
		{
			continue;
		}
		m_propertyList.InsertItem(index, attribute->getName());
		m_propertyList.SetItemText(index, 1, attribute->getValue<const wchar_t*>());
		++index;
	}
}

void CGredDlg::refreshPartList()
{
	m_partList.DeleteAllItems();
	for (int i = 0; i < (int)m_partNodes.size(); ++i)
	{
		slim::XmlNode* node = m_partNodes[i];
		m_partList.InsertItem(i, node->readAttribute<const wchar_t*>(L"slot", L""));
		const wchar_t* filename = node->readAttribute<const wchar_t*>(L"filename", L"");
		//temp, we don't have visible property right now
		//m_partList.SetItemText(i, 1, L"y");
		if (filename[0] == 0)
		{
			m_partList.SetItemText(i, 1, L"-");
		}
		else
		{
			m_partList.SetItemText(i, 1, filename);
		}
		if (isSkinnedPart(node))
		{
			m_partList.SetItemText(i, 2, L"skinned");
		}
		else
		{
			m_partList.SetItemText(i, 2, L"rigid");
		}
	}
}

void CGredDlg::refreshPartPropertyList()
{
	m_partPropertyList.DeleteAllItems();
	if (m_partNode == NULL)
	{
		return;
	}
	slim::XmlDocument doc;
	slim::XmlNode* partNode = m_partNode;

	slim::XmlAttribute* filenameAttr = partNode->findAttribute(L"filename");
	if (filenameAttr != NULL)
	{
		std::wstring filePath = filenameAttr->getValue<const wchar_t*>();
		if (grp::containFolder(m_modelFilename))
		{
			std::wstring folder;
			grp::getUrlBase(m_modelFilename, folder);
			filePath = folder + filePath;
		}
		if (!doc.loadFromFile(filePath.c_str()))
		{
			return;
		}
		partNode = doc.findChild(L"part");
	}
	if (partNode == NULL)
	{
		return;
	}
	slim::NodeIterator nodeIter;
	int index = 0;
	for (slim::XmlNode* child = partNode->findFirstChild(L"property", nodeIter);
		child != NULL;
		child = partNode->findNextChild(L"property", nodeIter))
	{
		slim::AttributeIterator iter;
		slim::XmlAttribute* attribute = child->getFirstAttribute(iter);
		if (attribute == NULL)
		{
			continue;
		}
		m_partPropertyList.InsertItem(index, attribute->getName());
		m_partPropertyList.SetItemText(index, 1, attribute->getValue<const wchar_t*>());
		++index;
	}
}

void CGredDlg::refreshAnimationList()
{
	m_animationList.DeleteAllItems();
	for (int i = 0; i < (int)m_animationNodes.size(); ++i)
	{
		slim::XmlNode* node = m_animationNodes[i];
		
		m_animationList.InsertItem(i, node->readAttribute<const wchar_t*>(L"slot", L""));
		
		const wchar_t* filename = node->readAttribute<const wchar_t*>(L"filename", L"");
		m_animationList.SetItemText(i, 1, filename);

		slim::XmlAttribute* startAttribute = node->findAttribute(L"start");
		slim::XmlAttribute* endAttribute = node->findAttribute(L"end");
		if (startAttribute == NULL)
		{
			m_animationList.SetItemText(i, 2, L"-");
		}
		else
		{
			m_animationList.SetItemText(i, 2, startAttribute->getValue<const wchar_t*>());
		}
		if (endAttribute == NULL)
		{
			m_animationList.SetItemText(i, 3, L"-");
		}
		else
		{
			m_animationList.SetItemText(i, 3, endAttribute->getValue<const wchar_t*>());
		}
	}
}

void CGredDlg::refreshEventList()
{
	m_eventList.DeleteAllItems();

	if (m_animationNode == NULL)
	{
		return;
	}
	m_eventNodes.clear();
	m_eventNodes.reserve(m_animationNode->getChildCount(L"event"));
	int eventIndex = 0;
	slim::NodeIterator nodeIter;
	for (slim::XmlNode* child = m_animationNode->getFirstChild(nodeIter);
		child != NULL;
		child = m_animationNode->getNextChild(nodeIter))
	{
		if (wcscmp(child->getName(), L"event") != 0)
		{
			continue;
		}
		m_eventNodes.push_back(child);
		m_eventList.InsertItem(eventIndex, child->readAttribute<const wchar_t*>(L"name", L""));
		slim::XmlAttribute* typeAttr = child->findAttribute(L"type");
		if (typeAttr == NULL || Strcmp(typeAttr->getValue<const wchar_t*>(), L"time") == 0)
		{
			typeAttr = child->findAttribute(L"time");
		}
		if (typeAttr == NULL)
		{
			m_eventList.SetItemText(eventIndex, 1, L"-");
		}
		else
		{
			m_eventList.SetItemText(eventIndex, 1, typeAttr->getValue<const wchar_t*>());
		}
		++eventIndex;
	}
}

void CGredDlg::refreshEventParamList()
{
	m_eventParamList.DeleteAllItems();
	if (m_eventNode == NULL)
	{
		return;
	}
	m_eventParamNodes.clear();
	m_eventParamNodes.reserve(m_eventNode->getChildCount(L"param"));
	int paramIndex = 0;
	slim::NodeIterator nodeIter;
	for (slim::XmlNode* child = m_eventNode->findFirstChild(L"param", nodeIter);
		child != NULL;
		child = m_eventNode->findNextChild(L"param", nodeIter))
	{
		slim::AttributeIterator iter;
		slim::XmlAttribute* attribute = child->getFirstAttribute(iter);
		if (attribute == NULL)
		{
			continue;
		}
		m_eventParamNodes.push_back(child);
		m_eventParamList.InsertItem(paramIndex, attribute->getName());
		m_eventParamList.SetItemText(paramIndex, 1, attribute->getValue<const wchar_t*>());
		++paramIndex;
	}
}

void CGredDlg::OnBnClickedNewModel()
{
	// TODO: Add your control notification handler code here
}

void CGredDlg::OnCbnSelchangeModels()
{
	int selected = m_modelComboBox.GetCurSel();
	if (selected >= 0 && selected < static_cast<int>(m_models.size()))
	{
		m_modelFilename = m_models[selected];
		refreshModel();
	}
}

void CGredDlg::OnCbnEditchangeModels()
{
	wchar_t filePath[MAX_PATH];
	m_modelComboBox.GetWindowText(filePath, sizeof(filePath));
	m_modelFilename = filePath;
	refreshModel();
}

void CGredDlg::OnBnClickedBrowseModel()
{
	CFileDialog fileDlg(TRUE);
	if (IDOK == fileDlg.DoModal())
	{
		m_modelFilename = fileDlg.GetPathName();
		m_modelComboBox.SetWindowText(m_modelFilename.c_str());
		refreshModel();
	}
}

void CGredDlg::OnCbnSelchangeSkeletons()
{
}

void CGredDlg::OnCbnEditchangeSkeletons()
{
}

void CGredDlg::OnBnClickedBrowseSkeleton()
{
	CFileDialog fileDlg(TRUE);
	if (IDOK == fileDlg.DoModal())
	{
		CString skeletonFilename = fileDlg.GetPathName();
		m_modelComboBox.SetWindowText(skeletonFilename);
		//...
	}
}

void CGredDlg::OnNMClickParts(NMHDR *pNMHDR, LRESULT *pResult)
{
	Invalidate();

	LPNMITEMACTIVATE item = (LPNMITEMACTIVATE) pNMHDR;
	int selected = item->iItem;
	int column = item->iSubItem;

	if (selected >= 0 && selected < m_partList.GetItemCount())
	{
		m_partNode = m_partNodes[selected];
	}
	else
	{
		m_partNode = NULL;
	}
	refreshPartPropertyList();
}

void CGredDlg::OnNMDblclkParts(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 0;
}

void CGredDlg::OnNMClickAnimations(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE item = (LPNMITEMACTIVATE) pNMHDR;
	int selected = item->iItem;
	int column = item->iSubItem;

	if (selected >= 0 && selected < m_animationList.GetItemCount())
	{
		m_animationNode = m_animationNodes[selected];
		grp::IModel* model = m_model->getModel();
		model->stopAllAnimations();
		const wchar_t* slot = m_animationNode->readAttribute<const wchar_t*>(L"slot", L"");
		model->playAnimation(slot, grp::ANIMATION_LOOP);
	}
	else
	{
		m_animationNode = NULL;
	}
	refreshEventList();
	m_eventNode = NULL;
	refreshEventParamList();
}

void CGredDlg::OnNMClickEvents(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE item = (LPNMITEMACTIVATE) pNMHDR;
	int selected = item->iItem;
	int column = item->iSubItem;

	if (selected >= 0 && selected < m_eventList.GetItemCount())
	{
		m_eventNode =  m_eventNodes[selected];
	}
	else
	{
		m_eventNode = NULL;
	}
	refreshEventParamList();
}

void CGredDlg::OnNMClickEventParams(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE item = (LPNMITEMACTIVATE) pNMHDR;
	int selected = item->iItem;
	int column = item->iSubItem;

	if (selected >= 0 && selected < m_eventParamList.GetItemCount())
	{
		m_eventParamNode = m_eventParamNodes[selected];
	}
	else
	{
		m_eventParamNode = NULL;
	}
}

void CGredDlg::OnBnClickedAddProperty()
{
	if (m_modelNode == NULL)
	{
		MessageBox(L"Select a model.", L"Note", MB_OK | MB_ICONINFORMATION);
	}
	m_propertyNode = m_modelNode->addChild(L"property");
	m_propertyNode->addAttribute(L"new property", L"new value");
	m_propertyNodes.push_back(m_propertyNode);

	m_modelFile.save(m_modelFilename.c_str());
	createGrpModel();

	refreshPropertyList(); 
	m_propertyList.SetItemState((int)m_propertyNodes.size() - 1, LVIS_SELECTED, LVIS_SELECTED); 
}

void CGredDlg::OnBnClickedRemoveProperty()
{
	if (m_modelNode == NULL || m_propertyNode == NULL)
	{
		MessageBox(L"Select a property.", L"Note", MB_OK | MB_ICONINFORMATION);
		return;
	}
	m_modelNode->removeChild(m_propertyNode);
	m_modelFile.save(m_modelFilename.c_str());

	createGrpModel();

	for (std::vector<slim::XmlNode*>::iterator iter = m_propertyNodes.begin();
		iter != m_propertyNodes.end();
		++iter)
	{
		if (*iter == m_propertyNode)
		{
			m_propertyNodes.erase(iter);
			break;
		}
	}
	m_propertyNode = NULL;
	refreshPropertyList();
}

void CGredDlg::OnBnClickedAddPart()
{
	// TODO: Add your control notification handler code here
}

void CGredDlg::OnBnClickedNewPart()
{
	// TODO: Add your control notification handler code here
}

void CGredDlg::OnBnClickedEditPart()
{
	// TODO: Add your control notification handler code here
}

void CGredDlg::OnBnClickedRemovePart()
{
	if (m_modelNode == NULL || m_partNode == NULL)
	{
		MessageBox(L"Select a part.", L"Note", MB_OK | MB_ICONINFORMATION);
		return;
	}
	m_modelNode->removeChild(m_partNode);
	m_modelFile.save(m_modelFilename.c_str());
	
	createGrpModel();

	for (std::vector<slim::XmlNode*>::iterator iter = m_partNodes.begin();
		iter != m_partNodes.end();
		++iter)
	{
		if (*iter == m_partNode)
		{
			m_partNodes.erase(iter);
			break;
		}
	}
	refreshPartList();
	m_partNode = NULL;
	refreshPartPropertyList();
}

void CGredDlg::OnBnClickedAddAnimation()
{
	if (m_modelNode == NULL)
	{
		return;
	}
	slim::XmlNode* animationNode = m_modelNode->addChild(L"animation");
	animationNode->addAttribute(L"name", L"new animation");
	animationNode->addAttribute(L"slot", L"new slot");
	animationNode->addAttribute(L"filename", L"filename.gan");
	m_animationNodes.push_back(animationNode);

	m_modelFile.save(m_modelFilename.c_str());
	createGrpModel();

	refreshAnimationList();
	m_animationNode = animationNode;
	m_animationList.SetItemState((int)m_animationNodes.size() - 1, LVIS_SELECTED, LVIS_SELECTED);
	refreshEventList();
	m_eventNode = NULL;
	refreshEventParamList();
}

void CGredDlg::OnBnClickedRemoveAnimation()
{
	if (m_modelNode == NULL || m_animationNode == NULL)
	{
		MessageBox(L"Select an animation.", L"Note", MB_OK | MB_ICONINFORMATION);
		return;
	}
	m_modelNode->removeChild(m_animationNode);
	m_modelFile.save(m_modelFilename.c_str());

	createGrpModel();

	for (std::vector<slim::XmlNode*>::iterator iter = m_animationNodes.begin();
		iter != m_animationNodes.end();
		++iter)
	{
		if (*iter == m_animationNode)
		{
			m_animationNodes.erase(iter);
			break;
		}
	}
	refreshAnimationList();
	m_animationNode = NULL;
	refreshEventList();
	m_eventNode = NULL;
	refreshEventParamList();
}

void CGredDlg::OnBnClickedAddEvent()
{
	if (m_animationNode == NULL)
	{
		MessageBox(L"Select an animation.", L"Note", MB_OK | MB_ICONINFORMATION);
		return;
	}
	slim::XmlNode* eventNode = m_animationNode->addChild(L"event");
	eventNode->addAttribute(L"name", L"new event");
	eventNode->addAttribute(L"time", L"start");
	m_eventNodes.push_back(eventNode);

	m_modelFile.save(m_modelFilename.c_str());
	createGrpModel();

	refreshEventList();
	m_eventNode = eventNode;
	refreshEventParamList();
	m_eventList.SetItemState((int)m_eventNodes.size() - 1, LVIS_SELECTED, LVIS_SELECTED); 
}

void CGredDlg::OnBnClickedRemoveEvent()
{
	if (m_animationNode == NULL || m_eventNode == NULL)
	{
		MessageBox(L"Select an event.", L"Note", MB_OK | MB_ICONINFORMATION);
		return;
	}
	m_animationNode->removeChild(m_eventNode);
	m_modelFile.save(m_modelFilename.c_str());
	createGrpModel();

	for (std::vector<slim::XmlNode*>::iterator iter = m_eventNodes.begin();
		iter != m_eventNodes.end();
		++iter)
	{
		if (*iter == m_eventNode)
		{
			m_eventNodes.erase(iter);
			break;
		}
	}
	refreshEventList();
	m_eventNode = NULL;
	refreshEventParamList();
}

void CGredDlg::OnBnClickedAddEventParam()
{
	if (m_eventNode == NULL)
	{
		MessageBox(L"Select an event.", L"Note", MB_OK | MB_ICONINFORMATION);
		return;
	}
	slim::XmlNode* paramNode = m_eventNode->addChild(L"param");
	paramNode->addAttribute(L"new param", L"new value");

	m_eventParamNodes.push_back(paramNode);
	m_modelFile.save(m_modelFilename.c_str());
	createGrpModel();

	m_eventParamNode = paramNode;
	refreshEventParamList();
	m_eventParamList.SetItemState((int)m_eventParamNodes.size() - 1, LVIS_SELECTED, LVIS_SELECTED); 
}

void CGredDlg::OnBnClickedRemoveEventParam()
{
	if (m_animationNode == NULL || m_eventNode == NULL || m_eventParamNode == NULL)
	{
		MessageBox(L"Select a param.", L"Note", MB_OK | MB_ICONINFORMATION);
		return;
	}
	m_eventNode->removeChild(m_eventParamNode);
	m_modelFile.save(m_modelFilename.c_str());
	createGrpModel();
	for (std::vector<slim::XmlNode*>::iterator iter = m_eventParamNodes.begin();
		iter != m_eventParamNodes.end();
		++iter)
	{
		if (*iter == m_eventParamNode)
		{
			m_eventParamNodes.erase(iter);
			break;
		}
	}
	m_eventParamNode = NULL;
	refreshEventParamList();
}

void CGredDlg::OnTimer(UINT_PTR nIDEvent)
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

	if (m_model == NULL)
	{
		return;
	}
	if (m_animationNode != NULL && !m_paused)
	{
		const wchar_t* slot = m_animationNode->readAttribute<const wchar_t*>(L"slot", L"");
		grp::IAnimation* animation = m_model->getModel()->findAnimation(slot);
		if (animation != NULL)
		{
			float time = animation->getTime();
			CString timeString;
			timeString.Format(L"Current Time:%.2f", time);
			m_timeCtrl.SetWindowTextW(timeString);
			
			int sliderPos = static_cast<int>(time * 100 / animation->getDuration());
			m_timeSlider.SetPos(sliderPos);
		}
	}

	__int64 counter, frequency;
	::QueryPerformanceCounter((LARGE_INTEGER*)&counter);
	::QueryPerformanceFrequency((LARGE_INTEGER*)&frequency);
	float time = float((double)counter / frequency);
	float elapsedTime = time - m_time;
	if (!m_paused)
	{
		float scale = (m_timeScale - 25) / 25.0f;
		m_model->update(time, elapsedTime * scale);
	}
	else
	{
		m_model->update(time, 0.0f);
	}

	const grp::AaBox& bb = m_model->getModel()->getBoundingBox();
	float projectionHeight = bb.getSize().Z * 1.1f;
	if (projectionHeight > m_camera.getHeight())
	{
		m_camera.setOrthoParams(projectionHeight * 1.202f, projectionHeight, 0.1f, 40.0f);
		m_camera.setLookAtPos(grp::Vector3(0, bb.getCenter().Z, 0));
	}
	m_camera.update(0, 0.03f);

	if (g_resourceManager != NULL)
	{
		g_resourceManager->update(elapsedTime);
	}

	m_time = time;

	m_device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DXCOLOR(0.3f,0.3f,0.6f,0.5f), 1.0f, 0);

	m_device->BeginScene();

	m_device->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	m_device->SetRenderState(D3DRS_ALPHAREF, (DWORD)0x00000080);
	m_device->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
	m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
	m_device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

	m_device->SetVertexDeclaration(m_vertexDeclaration);

	m_effect->SetValue("g_vCameraPos", (D3DXVECTOR3*)&m_camera.getEyePos(), sizeof(grp::Vector3));
	const grp::Vector3 viewDir = m_camera.getViewDirection();
	m_effect->SetValue("g_vLightDir", &D3DXVECTOR3(-viewDir.X, -viewDir.Y, -viewDir.Z), sizeof(D3DXVECTOR3));

	float ambient = 0.4f;
	float diffuse = 0.7f;
	m_effect->SetFloat("g_fAmbient", ambient);
	m_effect->SetFloat("g_fDiffuse", diffuse);

	m_effect->SetBool("g_bDiffuse", TRUE);
	m_effect->SetBool("g_bGloss", TRUE);

	//temp
	if (wcscmp(m_model->getModel()->getFirstPart()->getMaterial(0)->type(), L"normalmap") == 0)
	{
		m_effect->SetTechnique("Normalmap");
	}
	else
	{
		m_effect->SetTechnique("VertexLight");
	}
	UINT cPasses;
	m_effect->Begin(&cPasses, 0);
	m_model->render(m_effect,
					(const D3DXMATRIXA16&)m_camera.getViewMatrix(),
					(const D3DXMATRIXA16&)m_camera.getProjectionMatrix());
	m_effect->End();

	m_device->EndScene();
	m_device->Present(NULL, NULL, NULL, NULL);

	CDialog::OnTimer(nIDEvent);
}

void CGredDlg::OnNMReleasedcaptureTimeScaleSlider(NMHDR *pNMHDR, LRESULT *pResult)
{
	UpdateData();
	if (m_animationNode != NULL)
	{
		m_timeScaleString.Format(L"Time Scale:%.2f", (m_timeScale - 25) / 25.0f);
		UpdateData(FALSE);
	}
}

void CGredDlg::OnNMReleasedcaptureTimeSlider(NMHDR *pNMHDR, LRESULT *pResult)
{
	if (m_animationNode == NULL || m_model == NULL || !m_paused)
	{
		return;
	}
	grp::IModel* model = m_model->getModel();
	if (model == NULL)
	{
		return;
	}
	const wchar_t* slot = m_animationNode->readAttribute<const wchar_t*>(L"slot", L"");
	grp::IAnimation* animation = model->findAnimation(slot);
	if (animation == NULL)
	{
		return;
	}
	float time = m_timeSlider.GetPos() * animation->getDuration() / 100.0f;
	CString timeString;
	timeString.Format(L"Current Time:%.2f", time);
	m_timeCtrl.SetWindowTextW(timeString);
	animation->setTime(time);
}

void CGredDlg::OnBnClickedPause()
{
	if (m_paused)
	{
		m_paused = false;
		m_pauseButton.SetWindowTextW(L"||");
	}
	else
	{
		m_paused = true;
		m_pauseButton.SetWindowTextW(L">");
	}
}

void CGredDlg::OnRButtonDown(UINT nFlags, CPoint point)
{
	m_rbuttonPushed = true;
	m_lastPoint = point;
	::SetCapture(m_hWnd);
	CDialog::OnRButtonDown(nFlags, point);
}

void CGredDlg::OnRButtonUp(UINT nFlags, CPoint point)
{
	m_rbuttonPushed = false;
	::ReleaseCapture();
	CDialog::OnRButtonUp(nFlags, point);
}

void CGredDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	if (!m_rbuttonPushed || point == m_lastPoint)
	{
		return;
	}
	m_camera.setPitch(m_camera.getPitch() + (m_lastPoint.y - point.y) / 100.0f);
	m_camera.setYaw(m_camera.getYaw() + (point.x - m_lastPoint.x) / 100.0f);
	m_lastPoint = point;
	CDialog::OnMouseMove(nFlags, point);
}

BOOL CGredDlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	return CDialog::OnMouseWheel(nFlags, zDelta, pt);
}
