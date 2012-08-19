// PerfSpyDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PerfSpy.h"
#include "PerfSpyDlg.h"
#include <time.h>
#include <vector>
using std::vector;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static int	iOriginCX_Wnd,
			iOriginCY_Wnd,

			iOriginCX_User,
			iOriginCY_User,

			iOriginX_Pfl,
			iOriginY_Pfl,
			iOriginCX_Pfl,
			iOriginCY_Pfl,

			iOriginCX_Graph,
			iOriginCY_Graph,

			iOriginX_ColorHint,
			iOriginY_ColorHint,
			iOriginCX_ColorHint,
			iOriginCY_ColorHint,
			
			iOriginX_EditSearch,
			iOriginX_BtnSearch,
			iOriginY_Search,

			iOriginX_Open,
			iOriginY_Open,

			iOriginX_Quit,
			iOriginY_Quit;


#define SAFE_DELETE(p)	if ( (p) != NULL )	\
						{	\
							delete (p);	\
							(p) = NULL;	\
						}

#define SAFE_DELETE_ARR(p)	if ( (p) != NULL )	\
							{	\
								delete[] (p);	\
								(p) = NULL;	\
							}

enum
{
	COLUMN_NAME1			= 0,
	COLUMN_TOTAL_TIME		= 1,
	COLUMN_TOTAL_CALLS		= 2,
	COLUMN_TIME_PER_FRAME	= 3,
	COLUMN_TIME_PER_CALL	= 4,
	COLUMN_CALLS_PER_FRAME	= 5,
	COLUMN_RATE_PARENT		= 6,
	COLUMN_RATE_TOTAL		= 7
};

#define PAGE_SIZE	500

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPerfSpyDlg dialog

CPerfSpyDlg::CPerfSpyDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPerfSpyDlg::IDD, pParent)
	, m_strFrameTime(L"")
	, m_pIterator( NULL )
	, m_szUserDataName( NULL )
	, m_pUserData( NULL )
	, m_pTotalUserData( NULL )
	, m_pTotalChildTime( NULL )
	, m_pTotalChildCall( NULL )
	, m_pParentTime( NULL )
	, m_pParentTimePerFr( NULL )
	, m_ppChildTime( NULL )
	, m_ppChildTimePerFr( NULL )
	, m_pUserDataGraph( NULL )
	, m_iTotalChild( 0 )
	, m_bLBtnDown( FALSE )
	, m_bInitialized( FALSE )
	, m_bDrawTimePerFr(FALSE)
{
	m_pPerfManager = PerfManager::createTheOne();
	//{{AFX_DATA_INIT(CPerfSpyDlg)
	m_iCurFrmIdx = 0;
	m_iTotalDataFrame = 0;
	m_bShowTotal = FALSE;
	m_iPaintScale = 1;
	m_iFrameEnd = 0;
	m_iFrameBegin = 0;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	
	m_color[0] = RGB( 255,   0,   0 );
	m_color[1] = RGB(   0, 128,   0 );
	m_color[2] = RGB(   0,   0, 255 );
	m_color[3] = RGB( 255,   0, 128 );
	m_color[4] = RGB(   0,   0, 128 );
	m_color[5] = RGB( 128, 128,   0 );

	for ( int i = 0; i < MAX_PAINT_COLOR; i++ )
	{
		m_pen[i].CreatePen( PS_SOLID, 1, m_color[i] );
		m_brush[i].CreateSolidBrush( m_color[i] );
	}
	
	m_penBlack.CreatePen( PS_SOLID, 1, RGB( 0, 0, 0 ) );
	m_penWhite.CreatePen( PS_SOLID, 1, RGB( 255, 255, 255 ) );
	
	m_br.CreateSolidBrush( RGB( 200, 200, 0 ) );
}

CPerfSpyDlg::~CPerfSpyDlg()
{
	//g_DefPerfManager.SaveDataFrame( "PerfSpy.psf" );

	SAFE_DELETE( m_pIterator );
	SAFE_DELETE_ARR( m_szUserDataName );
	SAFE_DELETE_ARR( m_pUserData );
	SAFE_DELETE_ARR( m_pTotalUserData );
	SAFE_DELETE_ARR( m_pTotalChildTime );
	SAFE_DELETE_ARR( m_pTotalChildCall );
	SAFE_DELETE_ARR( m_pUserDataGraph );
	SAFE_DELETE_ARR( m_pParentTime );
	SAFE_DELETE_ARR( m_pParentTimePerFr );
	if ( m_ppChildTime != NULL )
	{
		for ( int i = 0; i < m_iTotalChild; i++ )
		{
			SAFE_DELETE_ARR( m_ppChildTime[i] );
		}
		SAFE_DELETE_ARR( m_ppChildTime );
	}
	if ( m_ppChildTimePerFr != NULL )
	{
		for ( int i = 0; i < m_iTotalChild; i++ )
		{
			SAFE_DELETE_ARR( m_ppChildTimePerFr[i] );
		}
		SAFE_DELETE_ARR( m_ppChildTimePerFr );
	}
	PerfManager::destroyTheOne();
	m_pPerfManager = NULL;
}

void CPerfSpyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPerfSpyDlg)
	DDX_Control(pDX, IDC_STATIC_TO, m_ctlTo);
	DDX_Control(pDX, IDC_STATIC_FROM, m_ctlFrom);
	DDX_Control(pDX, IDC_EDIT_END, m_ctlEnd);
	DDX_Control(pDX, IDC_EDIT_BEGIN, m_ctlBegin);
	DDX_Control(pDX, IDC_BUTTON_REFRESH, m_ctlRefresh);
	DDX_Control(pDX, IDC_STATIC_COLORHINT, m_ctlColorHint);
	DDX_Control(pDX, IDC_STATIC_GRAPH, m_ctlGraph);
	DDX_Control(pDX, IDC_LIST_USERDATA, m_ctlListUser);
	DDX_Control(pDX, IDC_SLIDER_PROGRESS, m_ctlSlider);
	DDX_Control(pDX, IDC_LIST_PROFILE, m_ctlListProfile);
	DDX_Text(pDX, IDC_EDIT_FRAMEINDEX, m_iCurFrmIdx);
	DDX_Text(pDX, IDC_EDIT_TOTALDATAFRAME, m_iTotalDataFrame);
	DDX_Check(pDX, IDC_CHECK_SHOWTOTAL, m_bShowTotal);
	DDX_Text(pDX, IDC_EDIT_SCALE, m_iPaintScale);
	DDV_MinMaxInt(pDX, m_iPaintScale, 1, 255);
	DDX_Text(pDX, IDC_EDIT_END, m_iFrameEnd);
	DDX_Text(pDX, IDC_EDIT_BEGIN, m_iFrameBegin);
	DDX_Text(pDX, IDC_EDIT_TIME, m_strFrameTime);
	DDX_Control(pDX, IDC_BUTTON_OPEN, m_ctlOpen);
	DDX_Control(pDX, IDCANCEL, m_ctlQuit);
	DDX_Radio(pDX, IDC_RADIO_TIME, m_bDrawTimePerFr);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CPerfSpyDlg, CDialog)
	//{{AFX_MSG_MAP(CPerfSpyDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_OPEN, OnButtonOpen)
	ON_NOTIFY(NM_CLICK, IDC_LIST_PROFILE, OnClickListProfile)
	ON_BN_CLICKED(IDC_BUTTON_PREV, OnButtonPrev)
	ON_BN_CLICKED(IDC_BUTTON_NEXT, OnButtonNext)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER_PROGRESS, OnReleasedcaptureSliderProgress)
	ON_EN_CHANGE(IDC_EDIT_FRAMEINDEX, OnChangeEditFrameindex)
	ON_EN_KILLFOCUS(IDC_EDIT_FRAMEINDEX, OnKillfocusEditFrameindex)
	ON_NOTIFY(NM_CLICK, IDC_LIST_USERDATA, OnClickListUserdata)
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_BN_CLICKED(IDC_CHECK_SHOWTOTAL, OnCheckShowtotal)
	ON_BN_CLICKED(IDC_BUTTON_PREVPAGE, OnButtonPrevpage)
	ON_BN_CLICKED(IDC_BUTTON_NEXTPAGE, OnButtonNextpage)
	ON_BN_CLICKED(IDC_BUTTON_MAXTIME, OnButtonMaxtime)
	ON_BN_CLICKED(IDC_BUTTON_MINTIME, OnButtonMintime)
	ON_BN_CLICKED(IDC_BUTTON_USERMAX, OnButtonUsermax)
	ON_BN_CLICKED(IDC_BUTTON_USERMIN, OnButtonUsermin)
	ON_EN_CHANGE(IDC_EDIT_SCALE, OnChangeEditScale)
	ON_BN_CLICKED(IDC_BUTTON_REFRESH, OnButtonRefresh)
	ON_WM_SIZE()
	ON_WM_DROPFILES()
	ON_BN_CLICKED(IDC_RADIO_TIME, OnBnClickedRadioTime)
	ON_BN_CLICKED(IDC_RADIO_TIME_PER_FR, OnBnClickedRadioTimePerFr)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPerfSpyDlg message handlers

BOOL CPerfSpyDlg::OnInitDialog()
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
	
	// TODO: Add extra initialization here
	InitListCtrl();
	InitUserDataList();

	//temp
	//ASSERT( FALSE );

	if ( strlen( AfxGetApp()->m_lpCmdLine ) > 0 )
	{
		//有关联的psf文件，去掉""
		CString strParam = AfxGetApp()->m_lpCmdLine;
		CString strFilePath = strParam;
		if ( strParam.Left( 1 ) == "\"" )
		{
			strFilePath = strParam.Right( strParam.GetLength() - 1 );
		}
		if ( strParam.Right( 1 ) == "\"" )
		{
			strParam = strFilePath;
			strFilePath = strParam.Left( strParam.GetLength() - 1 );
		}

		OpenDataFile( strFilePath );
	}

	m_bInitialized = TRUE;

	//第一次运行时记录各窗口大小的原始数据
	RECT rcWnd;
	this->GetClientRect( &rcWnd );
	iOriginCX_Wnd = rcWnd.right - rcWnd.left;
	iOriginCY_Wnd = rcWnd.bottom - rcWnd.top;

	RECT rcUser;
	m_ctlListUser.GetWindowRect( &rcUser );
	ScreenToClient( &rcUser );
	iOriginCX_User = rcUser.right - rcUser.left;
	iOriginCY_User = rcUser.bottom - rcUser.top;

	RECT rcPfl;
	m_ctlListProfile.GetWindowRect( &rcPfl );
	ScreenToClient( &rcPfl );
	iOriginX_Pfl = rcPfl.left;
	iOriginY_Pfl = rcPfl.top;
	iOriginCX_Pfl = rcPfl.right - rcPfl.left;
	iOriginCY_Pfl = rcPfl.bottom - rcPfl.top;

	RECT rcCH;
	m_ctlColorHint.GetWindowRect( &rcCH );
	ScreenToClient( &rcCH );
	iOriginX_ColorHint = rcCH.left;
	iOriginY_ColorHint = rcCH.top;
	iOriginCX_ColorHint = rcCH.right - rcCH.left;
	iOriginCY_ColorHint = rcCH.bottom - rcCH.top;

	RECT rcGraph;
	m_ctlGraph.GetWindowRect( &rcGraph );
	ScreenToClient( &rcGraph );
	iOriginCX_Graph = rcGraph.right - rcGraph.left;
	iOriginCY_Graph = rcGraph.bottom - rcGraph.top;

	RECT rcOpen;
	m_ctlOpen.GetWindowRect( &rcOpen );
	ScreenToClient( &rcOpen );
	iOriginX_Open = rcOpen.left;
	iOriginY_Open = rcOpen.top;

	RECT rcQuit;
	m_ctlQuit.GetWindowRect( &rcQuit );
	ScreenToClient( &rcQuit );
	iOriginX_Quit = rcQuit.left;
	iOriginY_Quit = rcQuit.top;

	//接受文件托拽
	DragAcceptFiles();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CPerfSpyDlg::InitListCtrl()
{
	m_ctlListProfile.SetExtendedStyle( LVS_EX_FULLROWSELECT );//| LVS_EX_TWOCLICKACTIVATE );

	LVCOLUMN lc;
	lc.mask = LVCF_WIDTH | LVCF_FMT | LVCF_TEXT | LVCF_SUBITEM;
	lc.cx = 218;
	lc.fmt = LVCFMT_LEFT;
	lc.pszText = "Name";
	lc.cchTextMax = 100;
	lc.iSubItem = COLUMN_NAME1;
	m_ctlListProfile.InsertColumn( 0, &lc );
	//总耗时
	lc.cx = 96;
	lc.pszText = "total time(ms)";
	lc.iSubItem = COLUMN_TOTAL_TIME;
	m_ctlListProfile.InsertColumn( 1, &lc );
	//总调用次数
	lc.cx = 78;
	lc.pszText = "total calls";
	lc.iSubItem = COLUMN_TOTAL_CALLS;
	m_ctlListProfile.InsertColumn( 2, &lc );
	//每帧耗时
	lc.cx = 60;
	lc.pszText = "ms/fr";
	lc.iSubItem = COLUMN_TIME_PER_FRAME;
	m_ctlListProfile.InsertColumn( 3, &lc );
	//每次调用耗时
	lc.cx = 60;
	lc.pszText = "ms/call";
	lc.iSubItem = COLUMN_TIME_PER_CALL;
	m_ctlListProfile.InsertColumn( 4, &lc );
	//每帧调用次数
	lc.cx = 60;
	lc.pszText = "call/fr";
	lc.iSubItem = COLUMN_CALLS_PER_FRAME;
	m_ctlListProfile.InsertColumn( 5, &lc );
	//耗时占父节点比例
	lc.cx = 56;
	lc.fmt = LVCFMT_RIGHT;
	lc.pszText = "%parent";
	lc.iSubItem = COLUMN_RATE_PARENT;
	m_ctlListProfile.InsertColumn( 6, &lc );
	//耗时占根节点比例
	lc.cx = 56;
	lc.pszText = "%total";
	lc.iSubItem = COLUMN_RATE_TOTAL;
	m_ctlListProfile.InsertColumn( 7, &lc );
}

void CPerfSpyDlg::InitUserDataList()
{
	m_ctlListUser.SetExtendedStyle( LVS_EX_FULLROWSELECT );//| LVS_EX_TWOCLICKACTIVATE );
	//用户数据名
	LVCOLUMN lc;
	lc.mask = LVCF_WIDTH | LVCF_FMT | LVCF_TEXT | LVCF_SUBITEM;
	lc.cx = 100;
	lc.fmt = LVCFMT_RIGHT;
	lc.pszText = "项目";
	lc.iSubItem = 0;
	m_ctlListUser.InsertColumn( 0, &lc );
	//用户数据值
	lc.cx = 100;
	lc.fmt = LVCFMT_LEFT;
	lc.pszText = "值";
	lc.iSubItem = 1;
	m_ctlListUser.InsertColumn( 1, &lc );
	//用户数据和耗时的耦合度
	lc.cx = 100;
	lc.pszText = "均值";
	lc.iSubItem = 2;
	m_ctlListUser.InsertColumn( 2, &lc );
}

void CPerfSpyDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CPerfSpyDlg::OnPaint() 
{
	//PERF_DEFAULT_FUNC();

	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

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
		DrawProfileData();
		DrawColorHint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CPerfSpyDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CPerfSpyDlg::OnButtonOpen() 
{
	// TODO: Add your control notification handler code here
	static char BASED_CODE szFilter[] = "User Files (*.perf)|*.perf|Profile Files (*.perf)|*.gam|All Files (*.*)|*.*||";
	CFileDialog FileDlg( TRUE, "psf", m_strFile, OFN_FILEMUSTEXIST,	szFilter, this );
	if( FileDlg.DoModal() == IDOK )
	{
		OpenDataFile( FileDlg.GetPathName() );
	}
}

void CPerfSpyDlg::OpenDataFile( LPCSTR pPath )
{
	m_strFile = pPath;

	if ( !DoLoad() )
	{
		MessageBox( "无效的psf文件，或psf数据版本不对！", "错误", MB_OK | MB_ICONERROR );
		return;
	}

	//读入绘图所需数据
	GetTimeDataForGraph();
	GetUserDataForGraph();

	m_iFrameBegin = 0;
	m_iFrameEnd = m_iTotalDataFrame;
	UpdateData( FALSE );

	//刷新采样节点列表
	if ( m_bShowTotal )
	{
		//获取总和数据
		GetTotalRootTime();
		GetTotalNodeDataArr();
		GetTotalUserDataArr();
		RefreshTotalProfileData();
	}
	else
	{
		RefreshProfileData();
	}
	//重绘图表控件
	Invalidate( FALSE );

	SetWindowText( m_strFile );
}

BOOL CPerfSpyDlg::DoLoad()
{
	//打开psf数据文件
	if ( !m_pPerfManager->OpenDataFile( m_strFile, &m_iTotalDataFrame, &m_iUserDataSize ) )
	{
		return FALSE;
	}

	m_iCurFrmIdx = 0;

	SAFE_DELETE_ARR( m_szUserDataName );
	if ( m_iUserDataSize > 0 )
	{
		m_szUserDataName = new char[m_iUserDataSize * MAX_NODE_NAME_LEN];
	}

	SAFE_DELETE_ARR( m_pUserData );
	SAFE_DELETE_ARR( m_pTotalUserData );

	if ( m_iUserDataSize > 0 )
	{
		m_pUserData = new int[m_iUserDataSize];
		m_pTotalUserData = new __int64[m_iUserDataSize];
	}

	//用户数据名加入列表
	m_ctlListUser.DeleteAllItems();
	m_pPerfManager->GetUserDataName( m_strFile, m_szUserDataName );
	for ( int i = 0; i < m_iUserDataSize; i++ )
	{
		m_ctlListUser.InsertItem( i, m_szUserDataName + MAX_NODE_NAME_LEN * i );
	}

	//读入一帧数据
	ReadDataFrame();

	if ( m_pIterator != NULL )
	{
		m_pPerfManager->ReleaseIterator( m_pIterator );
	}
	m_pIterator = m_pPerfManager->GetIterator();

	m_ctlSlider.SetRange( 0, m_iTotalDataFrame - 1 );
	m_ctlSlider.SetPos( m_iCurFrmIdx );

	UpdateData( FALSE );

	return TRUE;
}

void CPerfSpyDlg::ReadDataFrame()
{
	if ( m_iCurFrmIdx < 0 || m_iCurFrmIdx >= m_iTotalDataFrame )
	{
		return;
	}
	__time64_t time;
	m_pPerfManager->LoadDataFrame( m_strFile, m_iCurFrmIdx, m_pUserData, &time );
	CTime timeFrame( time );
	m_strFrameTime = timeFrame.Format( "%y-%m-%d %H:%M:%S" );
}

///////////////////////////////////////////////////////////////////////////////
//刷新性能评估采样树输出列表以及用户数据
///////////////////////////////////////////////////////////////////////////////
void CPerfSpyDlg::RefreshProfileData()
{
	//如果用户指定为显示总和则什么也不干
	if ( NULL == m_pIterator || m_bShowTotal )
	{
		return;
	}
//	int iSelOld = m_ctlListProfile.GetCurSel();
	int iIndex = 0;

	//清空list内容
	//m_ctlListProfile.ResetContent();
	m_ctlListProfile.DeleteAllItems();

	LPCSTR	pName;	//节点名
	int		iTotalCalls;		//调用次数
	float	fTotalTimeRoot,		//根节点耗时
			fTotalTimeParent,	//父节点耗时
			fTotalTime,			//子节点耗时
			fLeftTime;			//剩余没有统计耗时
	float	fRateParent,		//耗时占父节点比例
			fRateRoot,			//耗时占根节点比例
			fRateParentLeft,	//剩余耗时比例
			fRateRootLeft;

	//总耗时, 即根节点耗时
	fTotalTimeRoot = (float)m_pPerfManager->GetRootTotalTime() * 1000 / PerfGetTickRate();

	if ( 0.0 == fTotalTimeRoot )
	{
		m_ctlListProfile.InsertItem( 0, "No data." );
		return;
	}
	
	//当前节点
	pName		 = m_pIterator->GetCurrentParentName();
	iTotalCalls	 = m_pIterator->GetCurrentParentTotalCalls();
	if ( 0 == strcmp( "Root", m_pIterator->GetCurrentParentName() ) )
	{
		//根节点
		fTotalTimeParent = fTotalTimeRoot;
	}
	else
	{
		fTotalTimeParent = (float)m_pIterator->GetCurrentParentTotalTime() * 1000 / PerfGetTickRate();
	}
	fRateRoot = float( fTotalTimeParent / fTotalTimeRoot * 100.0 );

	int iFrameCount = m_pPerfManager->GetFrameCountSinceReset();

	InsertProfileLineToList( iIndex, TRUE, pName, iTotalCalls, fTotalTimeParent,
								100.0, iFrameCount, fRateRoot );

	fLeftTime = fTotalTimeParent;
	fRateParentLeft = 100.0;
	fRateRootLeft = fRateRoot;

	iIndex++;

	//子节点
	m_pIterator->First();
	while ( !m_pIterator->IsDone() )
	{
		pName		= m_pIterator->GetCurrentName();
		iTotalCalls = m_pIterator->GetCurrentTotalCalls();
		fTotalTime	= (float)m_pIterator->GetCurrentTotalTime() * 1000 / PerfGetTickRate();
		fLeftTime  -= fTotalTime;
		//耗时占父节点的比例
		if ( fTotalTimeParent != 0.0 )
		{
			fRateParent = float( fTotalTime / fTotalTimeParent * 100.0 );
		}
		else
		{
			fRateParent = 0.0;
		}
		fRateRoot = float( fTotalTime / fTotalTimeRoot * 100.0 );

		InsertProfileLineToList( iIndex, FALSE, pName, iTotalCalls, fTotalTime,
							fRateParent, iFrameCount, fRateRoot );

		fRateParentLeft -= fRateParent;
		fRateRootLeft -= fRateRoot;

		m_pIterator->Next();
		iIndex++;
	}
	//剩余
	if ( m_pIterator->Parent()->GetIndex() != 0 )
	{
		//根节点没有Other部分
		InsertProfileLineToList( iIndex, FALSE, "Other", 0, fLeftTime, fRateParentLeft,
						iFrameCount, fRateRootLeft );
	}

	//用户数据
	CString strData;
	for ( int i = 0; i < m_iUserDataSize; i++ )
	{
		strData.Format( "%d", m_pUserData[i] );
		m_ctlListUser.SetItemText( i, 1, strData );
	}
}

///////////////////////////////////////////////////////////////////////////////
//刷新性能评估采样树输出列表以及用户数据总和
///////////////////////////////////////////////////////////////////////////////
void CPerfSpyDlg::RefreshTotalProfileData()
{
	if ( NULL == m_pIterator )
	{
		return;
	}
	int iIndex = 0;

	//清空list内容
	m_ctlListProfile.DeleteAllItems();

	LPCSTR	pName;	//节点名
	int		iTotalCalls;		//调用次数
	float	fTotalTimeRoot,		//根节点耗时
			fTotalTimeParent,	//父节点耗时
			fTotalTime,			//子节点耗时
			fLeftTime;			//剩余没有统计耗时
	float	fRateParent,	//耗时占父节点比例
			fRateRoot,		//耗时占根节点比例
			fRateParentLeft,	//剩余耗时比例
			fRateRootLeft;

	//总耗时, 即根节点耗时
	fTotalTimeRoot = (float)m_i64TotalRootTime * 1000 / PerfGetTickRate();

	if ( 0.0 == fTotalTimeRoot )
	{
		m_ctlListProfile.InsertItem( 0, "No data." );
		return;
	}
	
	//当前节点
	pName			 = m_pIterator->GetCurrentParentName();
	iTotalCalls		 = m_iTotalParentCall;
	fTotalTimeParent = (float)m_i64TotalParentTime * 1000 / PerfGetTickRate();// * 1000.0;
	fRateRoot = float( fTotalTimeParent / fTotalTimeRoot * 100.0 );

	InsertProfileLineToList( iIndex, TRUE, pName, iTotalCalls, fTotalTimeParent,
							100.0, m_iTotalFrame, fRateRoot );

	fLeftTime = fTotalTimeParent;
	fRateParentLeft = 100.0;
	fRateRootLeft = fRateRoot;

	iIndex++;

	//子节点
	m_pIterator->First();
	while ( !m_pIterator->IsDone() )
	{
		pName		= m_pIterator->GetCurrentName();
		iTotalCalls = m_pTotalChildCall[iIndex - 1];//最前面有一个parent所以要-1
		fTotalTime	= (float)m_pTotalChildTime[iIndex - 1] * 1000 / PerfGetTickRate();// * 1000.0;
		fLeftTime  -= fTotalTime;
		//耗时占父节点的比例
		if ( fTotalTimeParent != 0.0 )
		{
			fRateParent = float( fTotalTime / fTotalTimeParent * 100.0 );
		}
		else
		{
			fRateParent = 0.0;
		}
		fRateRoot = float( fTotalTime / fTotalTimeRoot * 100.0 );

		InsertProfileLineToList( iIndex, FALSE, pName, iTotalCalls, fTotalTime,
						fRateParent, m_iTotalFrame, fRateRoot );

		fRateParentLeft -= fRateParent;
		fRateRootLeft -= fRateRoot;

		m_pIterator->Next();
		iIndex++;
	}
	//剩余
	if ( m_pIterator->Parent()->GetIndex() != 0 )
	{
		//根节点没有Other部分
		InsertProfileLineToList( iIndex, FALSE, "Other", 0, fLeftTime, fRateParentLeft,
							m_iTotalFrame, fRateRootLeft );
	}

	//用户数据
	CString strData;
	for ( int i = 0; i < m_iUserDataSize; i++ )
	{
		strData.Format( "%I64d", m_pTotalUserData[i] );
		m_ctlListUser.SetItemText( i, 1, strData );
		//用户数据均值
		if ( m_iFrameEnd - m_iFrameBegin > 0 )
		{
			float fAverage = (float)m_pTotalUserData[i] / ( m_iFrameEnd - m_iFrameBegin );
			strData.Format( "%f", fAverage );
			m_ctlListUser.SetItemText( i, 2, strData );
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
//向采样结果列表中添加一行
//iIndex		行序号
//bParent		是否父节点(如果是,前面加..)
//pName			节点名称
//iTotalCalls	调用次数(次)
//fTotalTime	总耗时(微秒)
//fRateParent	占父节点耗时的比例(%)
//fRateRoot		占根节点耗时的比例(%)
///////////////////////////////////////////////////////////////////////////////
void CPerfSpyDlg::InsertProfileLineToList( int iIndex, BOOL bParent, LPCSTR pName,
							int iTotalCalls, float fTotalTime, float fRateParent,
							int iFrameCount, float fRateRoot )
{
	CString	strOut;
	float	fTimePerCall;
	float	fTimePerFrame;

	//名字
	if ( bParent )
	{
		strOut.Format( "%s", pName );
	}
	else
	{
		strOut.Format( "├%s", pName );
	}
	//m_ctlListProfile.SetItemText( iIndex, COLUMN_NAME1, strOut );
	m_ctlListProfile.InsertItem( iIndex, strOut );
	//总耗时
	strOut.Format( "%f", fTotalTime );
	m_ctlListProfile.SetItemText( iIndex, COLUMN_TOTAL_TIME, strOut );
	//平均每帧耗时
	if ( 0 == iFrameCount )
	{
		iFrameCount = 1;
	}
	fTimePerFrame = fTotalTime / iFrameCount;

	strOut.Format( "%f", fTimePerFrame );
	m_ctlListProfile.SetItemText( iIndex, COLUMN_TIME_PER_FRAME, strOut );
	
	if ( 0 == iTotalCalls )
	{
		//总调用次数
		m_ctlListProfile.SetItemText( iIndex, COLUMN_TOTAL_CALLS, "-" );
		//平均每次调用耗时
		m_ctlListProfile.SetItemText( iIndex, COLUMN_TIME_PER_CALL, "-" );
		//每帧调用次数
		m_ctlListProfile.SetItemText( iIndex, COLUMN_CALLS_PER_FRAME, "-" );
	}
	else
	{
		//总调用次数
		strOut.Format( "%d", iTotalCalls );
		m_ctlListProfile.SetItemText( iIndex, COLUMN_TOTAL_CALLS, strOut );
		//平均每次调用耗时
		fTimePerCall = fTotalTime / iTotalCalls;
		strOut.Format( "%f", fTimePerCall );
		m_ctlListProfile.SetItemText( iIndex, COLUMN_TIME_PER_CALL, strOut );
		//每帧调用次数
		strOut.Format( "%f", (float)iTotalCalls / iFrameCount );
		m_ctlListProfile.SetItemText( iIndex, COLUMN_CALLS_PER_FRAME, strOut );
	}
	
	//耗时占父节点比例
	strOut.Format( "%.2f", fRateParent );
	m_ctlListProfile.SetItemText( iIndex, COLUMN_RATE_PARENT, strOut );
	//耗时占根节点(总耗时)比例
	strOut.Format( "%.2f", fRateRoot );
	m_ctlListProfile.SetItemText( iIndex, COLUMN_RATE_TOTAL, strOut );
}

//获取绘图所需的耗时数据
void CPerfSpyDlg::GetTimeDataForGraph()
{
	if ( NULL == m_pIterator || 0 == m_iTotalDataFrame )
	{
		return;
	}

	//现释放旧的数据
	SAFE_DELETE_ARR( m_pParentTime );
	SAFE_DELETE_ARR( m_pParentTimePerFr );
	if ( m_ppChildTime != NULL )
	{
		for ( int i = 0; i < m_iTotalChild; i++ )
		{
			SAFE_DELETE_ARR( m_ppChildTime[i] );
		}
		SAFE_DELETE_ARR( m_ppChildTime );
	}
	if ( m_ppChildTimePerFr != NULL )
	{
		for ( int i = 0; i < m_iTotalChild; i++ )
		{
			SAFE_DELETE_ARR( m_ppChildTimePerFr[i] );
		}
		SAFE_DELETE_ARR( m_ppChildTimePerFr );
	}
	//父节点数据
	m_pParentTime = new __int64[m_iTotalDataFrame];
	m_pParentTimePerFr = new __int64[m_iTotalDataFrame];
	m_pPerfManager->GetNodeTimeArr( m_strFile, m_pIterator->Parent()->GetIndex(),
								m_pParentTime, m_pParentTimePerFr, m_i64MaxTime, m_i64MinTime,
								m_iMaxTimeIndex, m_iMinTimeIndex, m_i64MaxTimePerFr );
	
	//子节点数据
	m_iTotalChild = m_pIterator->GetTotalChild();
	if ( 0 == m_iTotalChild )
	{
		return;
	}
	m_ppChildTime = new __int64*[m_iTotalChild];
	m_ppChildTimePerFr = new __int64*[m_iTotalChild];
	memset( m_ppChildTime, 0, sizeof(__int64*) * m_iTotalChild );
	memset( m_ppChildTimePerFr, 0, sizeof(__int64*) * m_iTotalChild );
	//子节点最大值数组
	int iCurChild = 0;
	__int64 i64MaxTimeChild, i64MinTimeChild, i64MaxTimeChildPerFr;
	int iMaxIndexChild, iMinIndexChild;
	PerfNode *pChild = m_pIterator->First();
	while ( pChild != NULL )
	{
		m_ppChildTime[iCurChild] = new __int64[m_iTotalDataFrame];
		m_ppChildTimePerFr[iCurChild] = new __int64[m_iTotalDataFrame];
		m_pPerfManager->GetNodeTimeArr( m_strFile, pChild->GetIndex(),
								m_ppChildTime[iCurChild], m_ppChildTimePerFr[iCurChild],
								i64MaxTimeChild, i64MinTimeChild,
								iMaxIndexChild, iMinIndexChild, i64MaxTimeChildPerFr );
		if ( i64MaxTimeChild > m_i64MaxTime )
		{
			m_i64MaxTime = i64MaxTimeChild;
			m_i64MinTime = i64MinTimeChild;
			m_iMaxTimeIndex = iMaxIndexChild;
			m_iMinTimeIndex = iMinIndexChild;
		}
		iCurChild++;
		if ( iCurChild >= m_iTotalChild )
		{
			break;
		}
		pChild = m_pIterator->Next();
	}
}

//获取绘图所需的用户数据
void CPerfSpyDlg::GetUserDataForGraph()
{
	if ( NULL == m_pIterator || 0 == m_iTotalDataFrame )
	{
		return;
	}

	//先释放旧的
	SAFE_DELETE_ARR( m_pUserDataGraph );
	
	//获取被选中的行序号
	POSITION pos = m_ctlListUser.GetFirstSelectedItemPosition();
	if ( NULL == pos )
	{
		return;
	}
	//只需要被选中的行对应的用户数据
	int iIndex = m_ctlListUser.GetNextSelectedItem( pos );
	m_pUserDataGraph = new int[m_iTotalDataFrame];
	m_pPerfManager->GetUserDataArr( m_strFile, iIndex, m_pUserDataGraph, m_iMaxUserData,
						m_iMinUserData, m_iMaxUserIndex, m_iMinUserIndex );
}

//获取当前父节点及子节点在所有数据帧的数据总和, 在ShowTotal状态下使用
void CPerfSpyDlg::GetTotalNodeDataArr()
{
	//需要哪些数据呢
	//耗时, 调用次数, 帧数, 根节点耗时
	if ( NULL == m_pIterator || 0 == m_iTotalDataFrame )
	{
		return;
	}
	//总帧数
	m_pPerfManager->GetTotalFrame( m_strFile, m_iTotalFrame, m_iFrameBegin, m_iFrameEnd );
	//父节点总和数据
	if ( 0 == strcmp( "Root", m_pIterator->GetCurrentParentName() ) )
	{
		//根节点
		m_i64TotalParentTime = m_i64TotalRootTime;
		m_iTotalParentCall = 0;
	}
	else
	{
		m_pPerfManager->GetNodeTotalData( m_strFile, m_pIterator->Parent()->GetIndex(), 
									m_i64TotalParentTime, m_iTotalParentCall,
									m_iFrameBegin, m_iFrameEnd );
	}
	//子节点总和数据
	SAFE_DELETE_ARR( m_pTotalChildTime );
	SAFE_DELETE_ARR( m_pTotalChildCall );

	if ( m_iTotalChild <= 0 )
	{
		return;
	}
	m_pTotalChildTime = new __int64[m_iTotalChild];
	m_pTotalChildCall = new int[m_iTotalChild];

	int iCurChild = 0;
	PerfNode *pChild = m_pIterator->First();
	while ( pChild != NULL )
	{
		m_pPerfManager->GetNodeTotalData( m_strFile, pChild->GetIndex(),
								m_pTotalChildTime[iCurChild], m_pTotalChildCall[iCurChild],
								m_iFrameBegin, m_iFrameEnd );
		iCurChild++;
		if ( iCurChild >= m_iTotalChild )
		{
			break;
		}
		pChild = m_pIterator->Next();
	}
}

//获取所有用户数据的所有数据帧的总和
void CPerfSpyDlg::GetTotalUserDataArr()
{
	if ( NULL == m_pIterator
		|| 0 == m_iTotalDataFrame
		|| NULL == m_pTotalUserData ) //m_pTotalUserData空间应该已经分配
	{
		return;
	}
	m_pPerfManager->GetTotalUserDataArr( m_strFile, m_pTotalUserData,
										m_iFrameBegin, m_iFrameEnd );
}

//绘制
void CPerfSpyDlg::DrawProfileData()
{
	CRect rcCanvas;
	m_ctlGraph.GetClientRect( &rcCanvas );

	CPaintDC dc( &m_ctlGraph );
	CBrush br;
	br.CreateSolidBrush( RGB( 255,255,255 ) );

	if ( NULL == m_pIterator )
	{
		dc.FillRect( &rcCanvas, &br );
		return;
	}

	int iStartX, iEndX, iStartIndex, iEndIndex;
	//X方向上每个象素表示一个数据帧
	//保持当前图像的中心位置对应当前数据帧
	int iMidX = rcCanvas.Width() / 2;
	if ( m_iCurFrmIdx - iMidX < 0 )
	{
		//前面有空白
		iStartX = iMidX - m_iCurFrmIdx;
		iStartIndex = 0;
		RECT rect = rcCanvas;
		rect.right = iStartX;
		dc.FillRect( &rect, &br );
	}
	else
	{
		iStartX = 0;
		iStartIndex = m_iCurFrmIdx - iMidX;
	}
	if ( m_iCurFrmIdx + ( rcCanvas.Width() - iMidX ) > m_iTotalDataFrame )
	{
		//后面有空白
		iEndX = iMidX + ( m_iTotalDataFrame - m_iCurFrmIdx );
		iEndIndex = m_iTotalDataFrame;

		RECT rect = rcCanvas;
		rect.left = iEndX;
		dc.FillRect( &rect, &br );
	}
	else
	{
		iEndX = rcCanvas.Width();
		iEndIndex = m_iCurFrmIdx + ( rcCanvas.Width() - iMidX );
	}
	//绘制
	int iCurX, iCurIndex;
	for ( iCurX = iStartX, iCurIndex = iStartIndex;
		  iCurX < iEndX && iCurIndex < iEndIndex;
		  iCurX++, iCurIndex++ )
	{
		//当前数据帧不画(反正被黑线挡住了, 防止拖动的时候闪烁)
		if ( iCurX == iMidX )
		{
			continue;
		}
		DrawOneFrame( iCurIndex, iCurX, rcCanvas.Height() - 1, dc );
	}
	dc.SelectObject( &m_penBlack );
	dc.MoveTo( iMidX, 0 );
	dc.LineTo( iMidX, rcCanvas.bottom );
}

//在ProfileList旁边绘制颜色提示, 一遍用户明确哪种颜色代表哪个节点
void CPerfSpyDlg::DrawColorHint()
{
	CPaintDC dc( &m_ctlColorHint );

	CRect rc;
	m_ctlColorHint.GetClientRect( &rc );
	//底色
	CBrush brBk;
	brBk.CreateSolidBrush( RGB( 255, 255, 255 ) );
	dc.FillRect( &rc, &brBk );

	if ( NULL == m_pIterator )
	{
		return;
	}

	rc.left   = 0;
	rc.top    = 20;
	rc.right  = 14;
	rc.bottom = rc.top + 13;

	int iCurColor = 0;
	int iEnd;
	if ( m_pIterator->Parent()->GetIndex() != 0 )
	{
		//最后有一个Other块
		iEnd = m_iTotalChild + 1;
	}
	else
	{
		iEnd = m_iTotalChild;
	}
	for ( int i = 0; i < iEnd; i++ )
	{
		rc.top += 13;
		rc.bottom += 13;
		
		dc.FillRect( &rc, &m_brush[iCurColor] );
		if ( ++iCurColor >= MAX_PAINT_COLOR )
		{
			iCurColor = 0;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
//以线段方式绘制单个数据帧的数据
//iIndex		数据帧序号
//iPosX			当前绘制的X坐标
//iMaxY			最大值的Y高度
//dc			设备环境
///////////////////////////////////////////////////////////////////////////////
void CPerfSpyDlg::DrawOneFrame( int iIndex, int iPosX, int iMaxY, CPaintDC &dc )
{
	if ( NULL == m_pParentTime )
	{
		return;
	}

	int iCurY = iMaxY;
	int iCurColor = 0;

	dc.MoveTo( iPosX, iCurY );

	//绘制每帧耗时 or 绘制总耗时
	__int64 i64MaxValue = ( m_bDrawTimePerFr ? m_i64MaxTimePerFr : m_i64MaxTime )
						/ m_iPaintScale;//处理放大

	if ( i64MaxValue > 0 )
	{
		__int64 iTimeOther = m_bDrawTimePerFr ? m_pParentTimePerFr[iIndex] : m_pParentTime[iIndex];
		//按从下到上的顺序绘制当前节点的所有子节点数据
		for ( int i = 0; i < m_iTotalChild; i++ )
		{
			dc.SelectObject( &m_pen[iCurColor] );
			//绘制每帧耗时 or 绘制总耗时
			int iHeight = int( ( m_bDrawTimePerFr ? m_ppChildTimePerFr[i][iIndex]
											 : m_ppChildTime[i][iIndex] )
								* iMaxY / i64MaxValue );

			iCurY -= iHeight;

			if ( iCurY < -1 )
			{
				//溢出, 剩下不画
				dc.LineTo( iPosX, -1 );
				iTimeOther = 0;
				break;
			}
			else if ( iCurY < iMaxY )
			{
				dc.LineTo( iPosX, iCurY );
			}
			iCurColor++;
			if ( iCurColor >= MAX_PAINT_COLOR )
			{
				iCurColor = 0;
			}
			iTimeOther -= ( m_bDrawTimePerFr ? m_ppChildTimePerFr[i][iIndex]
											 : m_ppChildTime[i][iIndex] );
		}
		//绘制Other段, 除非当前父节点是根节点(因为根节点的Time是0, Other会是负数)
		if ( m_pIterator->Parent()->GetIndex() != 0 )
		{
			dc.SelectObject( &m_pen[iCurColor] );
			int iHeight = int( iMaxY * iTimeOther / i64MaxValue );
			iCurY -= iHeight;
			if ( iCurY < -1 )
			{
				//溢出, 剩下不画
				dc.LineTo( iPosX, -1 );
				iTimeOther = 0;
			}
			else if ( iCurY < iMaxY )
			{
				dc.LineTo( iPosX, iCurY );
			}
			
		}
	}
	//没有清除背景, 所以必须把剩下的画成底色
	if ( iCurY > -1 )
	{
		dc.SelectObject( &m_penWhite );
		dc.LineTo( iPosX, -1 );
	}

	//绘制用户数据, 如果有的话
	int iUserDataOffset = m_iMaxUserData - m_iMinUserData;
	if ( m_pUserDataGraph != NULL )
	{
		if ( iUserDataOffset > 0 )
		{
			//因为绘制点为两个象素, 为了防止画到框外面去, 把iMaxY减1
			iCurY = int( iMaxY - (__int64)( iMaxY - 1 ) * ( m_pUserDataGraph[iIndex] - m_iMinUserData) / iUserDataOffset );
		}
		else
		{
			iCurY = iMaxY;
		}
		CRect rc( iPosX - 1, iCurY - 1, iPosX + 1, iCurY + 1 );
		dc.FillRect( &rc, &m_br );
	}
}

void CPerfSpyDlg::OnClickListProfile(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	
	*pResult = 0;
	//ListBox被点击
	if ( NULL == m_pIterator )
	{
		return;
	}
	POSITION pos = m_ctlListProfile.GetFirstSelectedItemPosition();
	if ( NULL == pos )
	{
		return;
	}
	int iIndex = m_ctlListProfile.GetNextSelectedItem( pos );
	if ( iIndex <= 0 )
	{
		//回到上一层
		m_pIterator->EnterParent();
	}
	else
	{
		m_pIterator->EnterChild( iIndex - 1 );
	}
	
	//读入绘图所需数据
	GetTimeDataForGraph();
	GetUserDataForGraph();

	if ( m_bShowTotal )
	{
		//获取总和数据
		GetTotalNodeDataArr();
		RefreshTotalProfileData();
	}
	else
	{
		RefreshProfileData();
	}
	
	//绘制
	Invalidate( FALSE );
}

void CPerfSpyDlg::OnButtonPrev() 
{
	//前一数据帧
	if ( m_iCurFrmIdx <= 0 )
	{
		return;
	}
	m_iCurFrmIdx--;
	ReadDataFrame();
	m_ctlSlider.SetPos( m_iCurFrmIdx );
	UpdateData( FALSE );

	RefreshProfileData();
	//绘制
	//DrawProfileData();
	Invalidate( FALSE );
}

void CPerfSpyDlg::OnButtonNext() 
{
	//后一数据帧
	if ( m_iCurFrmIdx >= m_iTotalDataFrame - 1 )
	{
		return;
	}
	m_iCurFrmIdx++;
	ReadDataFrame();
	m_ctlSlider.SetPos( m_iCurFrmIdx );
	UpdateData( FALSE );

	RefreshProfileData();
	//绘制
	//DrawProfileData();
	Invalidate( FALSE );
}

void CPerfSpyDlg::OnReleasedcaptureSliderProgress(NMHDR* pNMHDR, LRESULT* pResult) 
{
	//进度条位置改变
	*pResult = 0;
	m_iCurFrmIdx = m_ctlSlider.GetPos();
	ReadDataFrame();
	m_ctlSlider.SetPos( m_iCurFrmIdx );
	UpdateData( FALSE );

	RefreshProfileData();
	//绘制
	//DrawProfileData();
	Invalidate( FALSE );
}

void CPerfSpyDlg::OnChangeEditFrameindex() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_ColorHintANGE flag ORed into the mask.
	//当前帧序号改变
	UpdateData( TRUE );

	ReadDataFrame();
	m_ctlSlider.SetPos( m_iCurFrmIdx );
	Invalidate( FALSE );
}

void CPerfSpyDlg::OnKillfocusEditFrameindex() 
{
	// TODO: Add your control notification handler code here
	UpdateData( TRUE );
	if ( m_iCurFrmIdx < 0 || m_iCurFrmIdx >= m_iTotalDataFrame )
	{
		//帧序号超界, 强制设为0
		m_iCurFrmIdx = 0;
		ReadDataFrame();
		m_ctlSlider.SetPos( m_iCurFrmIdx );
		UpdateData( FALSE );
		RefreshProfileData();
		Invalidate( FALSE );
	}
}

void CPerfSpyDlg::OnClickListUserdata(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	*pResult = 0;

	//绘制被选定的用户数据
	GetUserDataForGraph();
	
	//获取被选中的行序号
/*	POSITION pos = m_ctlListUser.GetFirstSelectedItemPosition();
	if ( NULL == pos )
	{
		return;
	}
	CString strData;
	int iIndex = m_ctlListUser.GetNextSelectedItem( pos );
	strData.Format( "%I64d", m_i64UserDataCouple );
	m_ctlListUser.SetItemText( iIndex, 2, strData );
*/	Invalidate( FALSE );
}

//实现图表拖拽功能
void CPerfSpyDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	//指针是否落在图表范围内
	CRect rcGraph;
	m_ctlGraph.GetWindowRect( &rcGraph );
	ScreenToClient( &rcGraph );
	if ( rcGraph.PtInRect( point ) )
	{
		m_bLBtnDown = TRUE;
		m_iMouseLastPosX = point.x;
		SetCapture();
	}
	CDialog::OnLButtonDown(nFlags, point);
}

void CPerfSpyDlg::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	if ( m_bLBtnDown && m_iTotalDataFrame > 0 
		&& point.x != m_iMouseLastPosX )
	{
		//图表被拖拽了
		int iIndexOffset = point.x - m_iMouseLastPosX;
		int iIndexFrameNew;
		if ( m_iCurFrmIdx - iIndexOffset < 0 )
		{
			iIndexFrameNew = 0;
		}
		else if ( m_iCurFrmIdx - iIndexOffset >= m_iTotalDataFrame )
		{
			iIndexFrameNew = m_iTotalDataFrame - 1;
		}
		else
		{
			iIndexFrameNew = m_iCurFrmIdx - iIndexOffset;
		}
		m_iMouseLastPosX = point.x;

		if ( iIndexFrameNew != m_iCurFrmIdx )
		{
			m_iCurFrmIdx = iIndexFrameNew;
			ReadDataFrame();
			m_ctlSlider.SetPos( m_iCurFrmIdx );
			UpdateData( FALSE );
			RefreshProfileData();
			Invalidate( FALSE );
		}
	}
	CDialog::OnMouseMove(nFlags, point);
}

void CPerfSpyDlg::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	ReleaseCapture();
	if ( m_bLBtnDown )
	{
		m_bLBtnDown = FALSE;
	}
	CDialog::OnLButtonUp(nFlags, point);
}

void CPerfSpyDlg::OnCheckShowtotal() 
{
	// TODO: Add your control notification handler code here
	UpdateData();
	if ( m_bShowTotal )
	{
		//获取总和数据
		GetTotalRootTime();
		GetTotalNodeDataArr();
		GetTotalUserDataArr();
		//刷新节点和用户数据list
		RefreshTotalProfileData();
		//修改控件状态
		m_ctlBegin.EnableWindow( TRUE );
		m_ctlEnd.EnableWindow( TRUE );
		m_ctlFrom.EnableWindow( TRUE );
		m_ctlTo.EnableWindow( TRUE );
		m_ctlRefresh.EnableWindow( TRUE );
	}
	else
	{
		RefreshProfileData();
		//修改控件状态
		m_ctlBegin.EnableWindow( FALSE );
		m_ctlEnd.EnableWindow( FALSE );
		m_ctlFrom.EnableWindow( FALSE );
		m_ctlTo.EnableWindow( FALSE );
		m_ctlRefresh.EnableWindow( FALSE );
	}
}

void CPerfSpyDlg::OnButtonPrevpage() 
{
	//前一页数据
	if ( m_iCurFrmIdx <= 0 )
	{
		return;
	}
	else if ( m_iCurFrmIdx > PAGE_SIZE )
	{
		m_iCurFrmIdx -= PAGE_SIZE;
	}
	else
	{
		m_iCurFrmIdx = 0;
	}
	ReadDataFrame();
	m_ctlSlider.SetPos( m_iCurFrmIdx );
	UpdateData( FALSE );

	RefreshProfileData();
	//绘制
	Invalidate( FALSE );
}

void CPerfSpyDlg::OnButtonNextpage() 
{
	//后一页数据
	if ( m_iCurFrmIdx >= m_iTotalDataFrame )
	{
		return;
	}
	else if ( m_iCurFrmIdx < m_iTotalDataFrame - PAGE_SIZE )
	{
		m_iCurFrmIdx += PAGE_SIZE;
	}
	else
	{
		m_iCurFrmIdx = m_iTotalDataFrame - 1;
	}
	ReadDataFrame();
	m_ctlSlider.SetPos( m_iCurFrmIdx );
	UpdateData( FALSE );

	RefreshProfileData();
	//绘制
	Invalidate( FALSE );
}

void CPerfSpyDlg::OnButtonMaxtime() 
{
	//定位到父节点耗时最大值所在的数据帧
	if ( m_iMaxTimeIndex < 0 || m_iMaxTimeIndex >= m_iTotalDataFrame )
	{
		return;
	}
	m_iCurFrmIdx = m_iMaxTimeIndex;
	ReadDataFrame();
	m_ctlSlider.SetPos( m_iCurFrmIdx );
	UpdateData( FALSE );

	RefreshProfileData();
	//绘制
	Invalidate( FALSE );
}

void CPerfSpyDlg::OnButtonMintime() 
{
	//定位到父节点耗时最小值所在的数据帧
	if ( m_iMaxTimeIndex < 0 || m_iMaxTimeIndex >= m_iTotalDataFrame )
	{
		return;
	}
	m_iCurFrmIdx = m_iMinTimeIndex;
	ReadDataFrame();
	m_ctlSlider.SetPos( m_iCurFrmIdx );
	UpdateData( FALSE );

	RefreshProfileData();
	//绘制
	Invalidate( FALSE );
}

void CPerfSpyDlg::OnButtonUsermax() 
{
	//定位到用户数据最大值所在的数据帧
	if ( m_iMaxUserIndex < 0 || m_iMaxUserIndex >= m_iTotalDataFrame )
	{
		return;
	}
	m_iCurFrmIdx = m_iMaxUserIndex;
	ReadDataFrame();
	m_ctlSlider.SetPos( m_iCurFrmIdx );
	UpdateData( FALSE );

	RefreshProfileData();
	//绘制
	Invalidate( FALSE );
}

void CPerfSpyDlg::OnButtonUsermin() 
{
	//定位到用户数据最小值所在的数据帧
	if ( m_iMinUserIndex < 0 || m_iMinUserIndex >= m_iTotalDataFrame )
	{
		return;
	}
	m_iCurFrmIdx = m_iMinUserIndex;
	ReadDataFrame();
	m_ctlSlider.SetPos( m_iCurFrmIdx );
	UpdateData( FALSE );

	RefreshProfileData();
	//绘制
	Invalidate( FALSE );
}

void CPerfSpyDlg::OnChangeEditScale() 
{
	//显示比例改变
	UpdateData();
	if ( m_iPaintScale < 1 )
	{
		return;
	}
	//按新的比例重新绘制
	Invalidate( FALSE );
}

void CPerfSpyDlg::OnButtonRefresh() 
{
	if ( !m_bShowTotal )
	{
		return;
	}
	UpdateData();
	if ( m_iFrameBegin < 0 )
	{
		m_iFrameBegin = 0;
	}
	if ( m_iFrameEnd >= m_iTotalDataFrame )
	{
		m_iFrameEnd = m_iTotalDataFrame;
	}
	if ( m_iFrameBegin >= m_iFrameEnd )
	{
		m_iFrameEnd = m_iFrameBegin + 1;
	}
	UpdateData( FALSE );

	//获取总和数据
	GetTotalRootTime();
	GetTotalNodeDataArr();
	GetTotalUserDataArr();
	//刷新节点和用户数据list
	RefreshTotalProfileData();
}

//获取所有数据帧根节点的耗时总和（ShowTotal状态下使用）
void CPerfSpyDlg::GetTotalRootTime()
{
	m_i64TotalRootTime = 0;

	PerfIterator *pIterator = m_pPerfManager->GetIterator();
	ASSERT( pIterator != NULL );
	
	PerfNode *pChild = pIterator->First();
	while ( pChild != NULL )
	{
		__int64 i64Time = 0;
		int iNoUse = 0;

		m_pPerfManager->GetNodeTotalData( m_strFile, pChild->GetIndex(),
								i64Time, iNoUse, m_iFrameBegin, m_iFrameEnd );
		m_i64TotalRootTime += i64Time;

		pChild = pIterator->Next();
	}
}

void CPerfSpyDlg::OnOK()
{
}

void CPerfSpyDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	if ( !m_bInitialized )
	{
		return;
	}

	int iNewX, iNewY, iNewCX, iNewCY;

	//用户数据区
	iNewCX = iOriginCX_User + ( ( cx > iOriginCX_Wnd ) ? ( cx - iOriginCX_Wnd ) : 0 );
	iNewCY = iOriginCY_User + ( ( cy > iOriginCY_Wnd ) ? ( cy - iOriginCY_Wnd ) / 3 : 0 );

	m_ctlListUser.SetWindowPos( NULL, 0, 0, iNewCX, iNewCY, SWP_NOMOVE | SWP_NOZORDER );

	//Profile数据区
	iNewX = iOriginX_Pfl;
	iNewY = iOriginY_Pfl + ( ( cy > iOriginCY_Wnd ) ? ( cy - iOriginCY_Wnd ) / 3 : 0 );
	iNewCX = iOriginCX_Pfl + ( ( cx > iOriginCX_Wnd ) ? ( cx - iOriginCX_Wnd ) : 0 );
	iNewCY = iOriginCY_Pfl + ( ( cy > iOriginCY_Wnd ) ? ( cy - iOriginCY_Wnd ) * 2 / 3 : 0 );
	
	m_ctlListProfile.SetWindowPos( NULL, iNewX, iNewY, iNewCX, iNewCY, SWP_NOZORDER );

	//颜色提示
	iNewX = iOriginX_ColorHint;
	iNewY = iOriginY_ColorHint + ( ( cy > iOriginCY_Wnd ) ? ( cy - iOriginCY_Wnd ) / 3 : 0 );
	iNewCX = iOriginCX_ColorHint;
	iNewCY = iOriginCY_ColorHint + ( ( cy > iOriginCY_Wnd ) ? ( cy - iOriginCY_Wnd ) * 2 / 3 : 0 );

	m_ctlColorHint.SetWindowPos( NULL, iNewX, iNewY, iNewCX, iNewCY, SWP_NOZORDER );

	//柱状图
	iNewCX = iOriginCX_Graph + ( ( cx > iOriginCX_Wnd ) ? ( cx - iOriginCX_Wnd ) : 0 );
	iNewCY = iOriginCY_Graph;

	m_ctlGraph.SetWindowPos( NULL, iNewX, iNewY, iNewCX, iNewCY, SWP_NOZORDER | SWP_NOMOVE );

	//按钮
	iNewX = iOriginX_Open + cx - iOriginCX_Wnd;
	iNewY = iOriginY_Open + cy - iOriginCY_Wnd;

	m_ctlOpen.SetWindowPos( NULL, iNewX, iNewY, 0, 0, SWP_NOZORDER | SWP_NOSIZE );

	iNewX = iOriginX_Quit + cx - iOriginCX_Wnd;
	iNewY = iOriginY_Quit + cy - iOriginCY_Wnd;

	m_ctlQuit.SetWindowPos( NULL, iNewX, iNewY, 0, 0, SWP_NOZORDER | SWP_NOSIZE );

	Invalidate();
}

void CPerfSpyDlg::OnDropFiles(HDROP hDropInfo)
{
	UINT nFiles = ::DragQueryFile( hDropInfo, (UINT)-1, NULL, 0 );
	if ( nFiles <= 0 )
	{
		return;
	}
	TCHAR szFileName[_MAX_PATH];
	::DragQueryFile( hDropInfo, 0, szFileName, _MAX_PATH );
	OpenDataFile( szFileName );

	CDialog::OnDropFiles(hDropInfo);
}

void CPerfSpyDlg::OnBnClickedRadioTime()
{
	UpdateData();
	Invalidate( FALSE );
}

void CPerfSpyDlg::OnBnClickedRadioTimePerFr()
{
	UpdateData();
	Invalidate( FALSE );
}
