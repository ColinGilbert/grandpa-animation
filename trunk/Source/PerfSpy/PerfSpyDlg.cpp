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
		//�й�����psf�ļ���ȥ��""
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

	//��һ������ʱ��¼�����ڴ�С��ԭʼ����
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

	//�����ļ���ק
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
	//�ܺ�ʱ
	lc.cx = 96;
	lc.pszText = "total time(ms)";
	lc.iSubItem = COLUMN_TOTAL_TIME;
	m_ctlListProfile.InsertColumn( 1, &lc );
	//�ܵ��ô���
	lc.cx = 78;
	lc.pszText = "total calls";
	lc.iSubItem = COLUMN_TOTAL_CALLS;
	m_ctlListProfile.InsertColumn( 2, &lc );
	//ÿ֡��ʱ
	lc.cx = 60;
	lc.pszText = "ms/fr";
	lc.iSubItem = COLUMN_TIME_PER_FRAME;
	m_ctlListProfile.InsertColumn( 3, &lc );
	//ÿ�ε��ú�ʱ
	lc.cx = 60;
	lc.pszText = "ms/call";
	lc.iSubItem = COLUMN_TIME_PER_CALL;
	m_ctlListProfile.InsertColumn( 4, &lc );
	//ÿ֡���ô���
	lc.cx = 60;
	lc.pszText = "call/fr";
	lc.iSubItem = COLUMN_CALLS_PER_FRAME;
	m_ctlListProfile.InsertColumn( 5, &lc );
	//��ʱռ���ڵ����
	lc.cx = 56;
	lc.fmt = LVCFMT_RIGHT;
	lc.pszText = "%parent";
	lc.iSubItem = COLUMN_RATE_PARENT;
	m_ctlListProfile.InsertColumn( 6, &lc );
	//��ʱռ���ڵ����
	lc.cx = 56;
	lc.pszText = "%total";
	lc.iSubItem = COLUMN_RATE_TOTAL;
	m_ctlListProfile.InsertColumn( 7, &lc );
}

void CPerfSpyDlg::InitUserDataList()
{
	m_ctlListUser.SetExtendedStyle( LVS_EX_FULLROWSELECT );//| LVS_EX_TWOCLICKACTIVATE );
	//�û�������
	LVCOLUMN lc;
	lc.mask = LVCF_WIDTH | LVCF_FMT | LVCF_TEXT | LVCF_SUBITEM;
	lc.cx = 100;
	lc.fmt = LVCFMT_RIGHT;
	lc.pszText = "��Ŀ";
	lc.iSubItem = 0;
	m_ctlListUser.InsertColumn( 0, &lc );
	//�û�����ֵ
	lc.cx = 100;
	lc.fmt = LVCFMT_LEFT;
	lc.pszText = "ֵ";
	lc.iSubItem = 1;
	m_ctlListUser.InsertColumn( 1, &lc );
	//�û����ݺͺ�ʱ����϶�
	lc.cx = 100;
	lc.pszText = "��ֵ";
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
		MessageBox( "��Ч��psf�ļ�����psf���ݰ汾���ԣ�", "����", MB_OK | MB_ICONERROR );
		return;
	}

	//�����ͼ��������
	GetTimeDataForGraph();
	GetUserDataForGraph();

	m_iFrameBegin = 0;
	m_iFrameEnd = m_iTotalDataFrame;
	UpdateData( FALSE );

	//ˢ�²����ڵ��б�
	if ( m_bShowTotal )
	{
		//��ȡ�ܺ�����
		GetTotalRootTime();
		GetTotalNodeDataArr();
		GetTotalUserDataArr();
		RefreshTotalProfileData();
	}
	else
	{
		RefreshProfileData();
	}
	//�ػ�ͼ��ؼ�
	Invalidate( FALSE );

	SetWindowText( m_strFile );
}

BOOL CPerfSpyDlg::DoLoad()
{
	//��psf�����ļ�
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

	//�û������������б�
	m_ctlListUser.DeleteAllItems();
	m_pPerfManager->GetUserDataName( m_strFile, m_szUserDataName );
	for ( int i = 0; i < m_iUserDataSize; i++ )
	{
		m_ctlListUser.InsertItem( i, m_szUserDataName + MAX_NODE_NAME_LEN * i );
	}

	//����һ֡����
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
//ˢ��������������������б��Լ��û�����
///////////////////////////////////////////////////////////////////////////////
void CPerfSpyDlg::RefreshProfileData()
{
	//����û�ָ��Ϊ��ʾ�ܺ���ʲôҲ����
	if ( NULL == m_pIterator || m_bShowTotal )
	{
		return;
	}
//	int iSelOld = m_ctlListProfile.GetCurSel();
	int iIndex = 0;

	//���list����
	//m_ctlListProfile.ResetContent();
	m_ctlListProfile.DeleteAllItems();

	LPCSTR	pName;	//�ڵ���
	int		iTotalCalls;		//���ô���
	float	fTotalTimeRoot,		//���ڵ��ʱ
			fTotalTimeParent,	//���ڵ��ʱ
			fTotalTime,			//�ӽڵ��ʱ
			fLeftTime;			//ʣ��û��ͳ�ƺ�ʱ
	float	fRateParent,		//��ʱռ���ڵ����
			fRateRoot,			//��ʱռ���ڵ����
			fRateParentLeft,	//ʣ���ʱ����
			fRateRootLeft;

	//�ܺ�ʱ, �����ڵ��ʱ
	fTotalTimeRoot = (float)m_pPerfManager->GetRootTotalTime() * 1000 / PerfGetTickRate();

	if ( 0.0 == fTotalTimeRoot )
	{
		m_ctlListProfile.InsertItem( 0, "No data." );
		return;
	}
	
	//��ǰ�ڵ�
	pName		 = m_pIterator->GetCurrentParentName();
	iTotalCalls	 = m_pIterator->GetCurrentParentTotalCalls();
	if ( 0 == strcmp( "Root", m_pIterator->GetCurrentParentName() ) )
	{
		//���ڵ�
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

	//�ӽڵ�
	m_pIterator->First();
	while ( !m_pIterator->IsDone() )
	{
		pName		= m_pIterator->GetCurrentName();
		iTotalCalls = m_pIterator->GetCurrentTotalCalls();
		fTotalTime	= (float)m_pIterator->GetCurrentTotalTime() * 1000 / PerfGetTickRate();
		fLeftTime  -= fTotalTime;
		//��ʱռ���ڵ�ı���
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
	//ʣ��
	if ( m_pIterator->Parent()->GetIndex() != 0 )
	{
		//���ڵ�û��Other����
		InsertProfileLineToList( iIndex, FALSE, "Other", 0, fLeftTime, fRateParentLeft,
						iFrameCount, fRateRootLeft );
	}

	//�û�����
	CString strData;
	for ( int i = 0; i < m_iUserDataSize; i++ )
	{
		strData.Format( "%d", m_pUserData[i] );
		m_ctlListUser.SetItemText( i, 1, strData );
	}
}

///////////////////////////////////////////////////////////////////////////////
//ˢ��������������������б��Լ��û������ܺ�
///////////////////////////////////////////////////////////////////////////////
void CPerfSpyDlg::RefreshTotalProfileData()
{
	if ( NULL == m_pIterator )
	{
		return;
	}
	int iIndex = 0;

	//���list����
	m_ctlListProfile.DeleteAllItems();

	LPCSTR	pName;	//�ڵ���
	int		iTotalCalls;		//���ô���
	float	fTotalTimeRoot,		//���ڵ��ʱ
			fTotalTimeParent,	//���ڵ��ʱ
			fTotalTime,			//�ӽڵ��ʱ
			fLeftTime;			//ʣ��û��ͳ�ƺ�ʱ
	float	fRateParent,	//��ʱռ���ڵ����
			fRateRoot,		//��ʱռ���ڵ����
			fRateParentLeft,	//ʣ���ʱ����
			fRateRootLeft;

	//�ܺ�ʱ, �����ڵ��ʱ
	fTotalTimeRoot = (float)m_i64TotalRootTime * 1000 / PerfGetTickRate();

	if ( 0.0 == fTotalTimeRoot )
	{
		m_ctlListProfile.InsertItem( 0, "No data." );
		return;
	}
	
	//��ǰ�ڵ�
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

	//�ӽڵ�
	m_pIterator->First();
	while ( !m_pIterator->IsDone() )
	{
		pName		= m_pIterator->GetCurrentName();
		iTotalCalls = m_pTotalChildCall[iIndex - 1];//��ǰ����һ��parent����Ҫ-1
		fTotalTime	= (float)m_pTotalChildTime[iIndex - 1] * 1000 / PerfGetTickRate();// * 1000.0;
		fLeftTime  -= fTotalTime;
		//��ʱռ���ڵ�ı���
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
	//ʣ��
	if ( m_pIterator->Parent()->GetIndex() != 0 )
	{
		//���ڵ�û��Other����
		InsertProfileLineToList( iIndex, FALSE, "Other", 0, fLeftTime, fRateParentLeft,
							m_iTotalFrame, fRateRootLeft );
	}

	//�û�����
	CString strData;
	for ( int i = 0; i < m_iUserDataSize; i++ )
	{
		strData.Format( "%I64d", m_pTotalUserData[i] );
		m_ctlListUser.SetItemText( i, 1, strData );
		//�û����ݾ�ֵ
		if ( m_iFrameEnd - m_iFrameBegin > 0 )
		{
			float fAverage = (float)m_pTotalUserData[i] / ( m_iFrameEnd - m_iFrameBegin );
			strData.Format( "%f", fAverage );
			m_ctlListUser.SetItemText( i, 2, strData );
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
//���������б������һ��
//iIndex		�����
//bParent		�Ƿ񸸽ڵ�(�����,ǰ���..)
//pName			�ڵ�����
//iTotalCalls	���ô���(��)
//fTotalTime	�ܺ�ʱ(΢��)
//fRateParent	ռ���ڵ��ʱ�ı���(%)
//fRateRoot		ռ���ڵ��ʱ�ı���(%)
///////////////////////////////////////////////////////////////////////////////
void CPerfSpyDlg::InsertProfileLineToList( int iIndex, BOOL bParent, LPCSTR pName,
							int iTotalCalls, float fTotalTime, float fRateParent,
							int iFrameCount, float fRateRoot )
{
	CString	strOut;
	float	fTimePerCall;
	float	fTimePerFrame;

	//����
	if ( bParent )
	{
		strOut.Format( "%s", pName );
	}
	else
	{
		strOut.Format( "��%s", pName );
	}
	//m_ctlListProfile.SetItemText( iIndex, COLUMN_NAME1, strOut );
	m_ctlListProfile.InsertItem( iIndex, strOut );
	//�ܺ�ʱ
	strOut.Format( "%f", fTotalTime );
	m_ctlListProfile.SetItemText( iIndex, COLUMN_TOTAL_TIME, strOut );
	//ƽ��ÿ֡��ʱ
	if ( 0 == iFrameCount )
	{
		iFrameCount = 1;
	}
	fTimePerFrame = fTotalTime / iFrameCount;

	strOut.Format( "%f", fTimePerFrame );
	m_ctlListProfile.SetItemText( iIndex, COLUMN_TIME_PER_FRAME, strOut );
	
	if ( 0 == iTotalCalls )
	{
		//�ܵ��ô���
		m_ctlListProfile.SetItemText( iIndex, COLUMN_TOTAL_CALLS, "-" );
		//ƽ��ÿ�ε��ú�ʱ
		m_ctlListProfile.SetItemText( iIndex, COLUMN_TIME_PER_CALL, "-" );
		//ÿ֡���ô���
		m_ctlListProfile.SetItemText( iIndex, COLUMN_CALLS_PER_FRAME, "-" );
	}
	else
	{
		//�ܵ��ô���
		strOut.Format( "%d", iTotalCalls );
		m_ctlListProfile.SetItemText( iIndex, COLUMN_TOTAL_CALLS, strOut );
		//ƽ��ÿ�ε��ú�ʱ
		fTimePerCall = fTotalTime / iTotalCalls;
		strOut.Format( "%f", fTimePerCall );
		m_ctlListProfile.SetItemText( iIndex, COLUMN_TIME_PER_CALL, strOut );
		//ÿ֡���ô���
		strOut.Format( "%f", (float)iTotalCalls / iFrameCount );
		m_ctlListProfile.SetItemText( iIndex, COLUMN_CALLS_PER_FRAME, strOut );
	}
	
	//��ʱռ���ڵ����
	strOut.Format( "%.2f", fRateParent );
	m_ctlListProfile.SetItemText( iIndex, COLUMN_RATE_PARENT, strOut );
	//��ʱռ���ڵ�(�ܺ�ʱ)����
	strOut.Format( "%.2f", fRateRoot );
	m_ctlListProfile.SetItemText( iIndex, COLUMN_RATE_TOTAL, strOut );
}

//��ȡ��ͼ����ĺ�ʱ����
void CPerfSpyDlg::GetTimeDataForGraph()
{
	if ( NULL == m_pIterator || 0 == m_iTotalDataFrame )
	{
		return;
	}

	//���ͷžɵ�����
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
	//���ڵ�����
	m_pParentTime = new __int64[m_iTotalDataFrame];
	m_pParentTimePerFr = new __int64[m_iTotalDataFrame];
	m_pPerfManager->GetNodeTimeArr( m_strFile, m_pIterator->Parent()->GetIndex(),
								m_pParentTime, m_pParentTimePerFr, m_i64MaxTime, m_i64MinTime,
								m_iMaxTimeIndex, m_iMinTimeIndex, m_i64MaxTimePerFr );
	
	//�ӽڵ�����
	m_iTotalChild = m_pIterator->GetTotalChild();
	if ( 0 == m_iTotalChild )
	{
		return;
	}
	m_ppChildTime = new __int64*[m_iTotalChild];
	m_ppChildTimePerFr = new __int64*[m_iTotalChild];
	memset( m_ppChildTime, 0, sizeof(__int64*) * m_iTotalChild );
	memset( m_ppChildTimePerFr, 0, sizeof(__int64*) * m_iTotalChild );
	//�ӽڵ����ֵ����
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

//��ȡ��ͼ������û�����
void CPerfSpyDlg::GetUserDataForGraph()
{
	if ( NULL == m_pIterator || 0 == m_iTotalDataFrame )
	{
		return;
	}

	//���ͷžɵ�
	SAFE_DELETE_ARR( m_pUserDataGraph );
	
	//��ȡ��ѡ�е������
	POSITION pos = m_ctlListUser.GetFirstSelectedItemPosition();
	if ( NULL == pos )
	{
		return;
	}
	//ֻ��Ҫ��ѡ�е��ж�Ӧ���û�����
	int iIndex = m_ctlListUser.GetNextSelectedItem( pos );
	m_pUserDataGraph = new int[m_iTotalDataFrame];
	m_pPerfManager->GetUserDataArr( m_strFile, iIndex, m_pUserDataGraph, m_iMaxUserData,
						m_iMinUserData, m_iMaxUserIndex, m_iMinUserIndex );
}

//��ȡ��ǰ���ڵ㼰�ӽڵ�����������֡�������ܺ�, ��ShowTotal״̬��ʹ��
void CPerfSpyDlg::GetTotalNodeDataArr()
{
	//��Ҫ��Щ������
	//��ʱ, ���ô���, ֡��, ���ڵ��ʱ
	if ( NULL == m_pIterator || 0 == m_iTotalDataFrame )
	{
		return;
	}
	//��֡��
	m_pPerfManager->GetTotalFrame( m_strFile, m_iTotalFrame, m_iFrameBegin, m_iFrameEnd );
	//���ڵ��ܺ�����
	if ( 0 == strcmp( "Root", m_pIterator->GetCurrentParentName() ) )
	{
		//���ڵ�
		m_i64TotalParentTime = m_i64TotalRootTime;
		m_iTotalParentCall = 0;
	}
	else
	{
		m_pPerfManager->GetNodeTotalData( m_strFile, m_pIterator->Parent()->GetIndex(), 
									m_i64TotalParentTime, m_iTotalParentCall,
									m_iFrameBegin, m_iFrameEnd );
	}
	//�ӽڵ��ܺ�����
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

//��ȡ�����û����ݵ���������֡���ܺ�
void CPerfSpyDlg::GetTotalUserDataArr()
{
	if ( NULL == m_pIterator
		|| 0 == m_iTotalDataFrame
		|| NULL == m_pTotalUserData ) //m_pTotalUserData�ռ�Ӧ���Ѿ�����
	{
		return;
	}
	m_pPerfManager->GetTotalUserDataArr( m_strFile, m_pTotalUserData,
										m_iFrameBegin, m_iFrameEnd );
}

//����
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
	//X������ÿ�����ر�ʾһ������֡
	//���ֵ�ǰͼ�������λ�ö�Ӧ��ǰ����֡
	int iMidX = rcCanvas.Width() / 2;
	if ( m_iCurFrmIdx - iMidX < 0 )
	{
		//ǰ���пհ�
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
		//�����пհ�
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
	//����
	int iCurX, iCurIndex;
	for ( iCurX = iStartX, iCurIndex = iStartIndex;
		  iCurX < iEndX && iCurIndex < iEndIndex;
		  iCurX++, iCurIndex++ )
	{
		//��ǰ����֡����(���������ߵ�ס��, ��ֹ�϶���ʱ����˸)
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

//��ProfileList�Ա߻�����ɫ��ʾ, һ���û���ȷ������ɫ�����ĸ��ڵ�
void CPerfSpyDlg::DrawColorHint()
{
	CPaintDC dc( &m_ctlColorHint );

	CRect rc;
	m_ctlColorHint.GetClientRect( &rc );
	//��ɫ
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
		//�����һ��Other��
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
//���߶η�ʽ���Ƶ�������֡������
//iIndex		����֡���
//iPosX			��ǰ���Ƶ�X����
//iMaxY			���ֵ��Y�߶�
//dc			�豸����
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

	//����ÿ֡��ʱ or �����ܺ�ʱ
	__int64 i64MaxValue = ( m_bDrawTimePerFr ? m_i64MaxTimePerFr : m_i64MaxTime )
						/ m_iPaintScale;//����Ŵ�

	if ( i64MaxValue > 0 )
	{
		__int64 iTimeOther = m_bDrawTimePerFr ? m_pParentTimePerFr[iIndex] : m_pParentTime[iIndex];
		//�����µ��ϵ�˳����Ƶ�ǰ�ڵ�������ӽڵ�����
		for ( int i = 0; i < m_iTotalChild; i++ )
		{
			dc.SelectObject( &m_pen[iCurColor] );
			//����ÿ֡��ʱ or �����ܺ�ʱ
			int iHeight = int( ( m_bDrawTimePerFr ? m_ppChildTimePerFr[i][iIndex]
											 : m_ppChildTime[i][iIndex] )
								* iMaxY / i64MaxValue );

			iCurY -= iHeight;

			if ( iCurY < -1 )
			{
				//���, ʣ�²���
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
		//����Other��, ���ǵ�ǰ���ڵ��Ǹ��ڵ�(��Ϊ���ڵ��Time��0, Other���Ǹ���)
		if ( m_pIterator->Parent()->GetIndex() != 0 )
		{
			dc.SelectObject( &m_pen[iCurColor] );
			int iHeight = int( iMaxY * iTimeOther / i64MaxValue );
			iCurY -= iHeight;
			if ( iCurY < -1 )
			{
				//���, ʣ�²���
				dc.LineTo( iPosX, -1 );
				iTimeOther = 0;
			}
			else if ( iCurY < iMaxY )
			{
				dc.LineTo( iPosX, iCurY );
			}
			
		}
	}
	//û���������, ���Ա����ʣ�µĻ��ɵ�ɫ
	if ( iCurY > -1 )
	{
		dc.SelectObject( &m_penWhite );
		dc.LineTo( iPosX, -1 );
	}

	//�����û�����, ����еĻ�
	int iUserDataOffset = m_iMaxUserData - m_iMinUserData;
	if ( m_pUserDataGraph != NULL )
	{
		if ( iUserDataOffset > 0 )
		{
			//��Ϊ���Ƶ�Ϊ��������, Ϊ�˷�ֹ����������ȥ, ��iMaxY��1
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
	//ListBox�����
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
		//�ص���һ��
		m_pIterator->EnterParent();
	}
	else
	{
		m_pIterator->EnterChild( iIndex - 1 );
	}
	
	//�����ͼ��������
	GetTimeDataForGraph();
	GetUserDataForGraph();

	if ( m_bShowTotal )
	{
		//��ȡ�ܺ�����
		GetTotalNodeDataArr();
		RefreshTotalProfileData();
	}
	else
	{
		RefreshProfileData();
	}
	
	//����
	Invalidate( FALSE );
}

void CPerfSpyDlg::OnButtonPrev() 
{
	//ǰһ����֡
	if ( m_iCurFrmIdx <= 0 )
	{
		return;
	}
	m_iCurFrmIdx--;
	ReadDataFrame();
	m_ctlSlider.SetPos( m_iCurFrmIdx );
	UpdateData( FALSE );

	RefreshProfileData();
	//����
	//DrawProfileData();
	Invalidate( FALSE );
}

void CPerfSpyDlg::OnButtonNext() 
{
	//��һ����֡
	if ( m_iCurFrmIdx >= m_iTotalDataFrame - 1 )
	{
		return;
	}
	m_iCurFrmIdx++;
	ReadDataFrame();
	m_ctlSlider.SetPos( m_iCurFrmIdx );
	UpdateData( FALSE );

	RefreshProfileData();
	//����
	//DrawProfileData();
	Invalidate( FALSE );
}

void CPerfSpyDlg::OnReleasedcaptureSliderProgress(NMHDR* pNMHDR, LRESULT* pResult) 
{
	//������λ�øı�
	*pResult = 0;
	m_iCurFrmIdx = m_ctlSlider.GetPos();
	ReadDataFrame();
	m_ctlSlider.SetPos( m_iCurFrmIdx );
	UpdateData( FALSE );

	RefreshProfileData();
	//����
	//DrawProfileData();
	Invalidate( FALSE );
}

void CPerfSpyDlg::OnChangeEditFrameindex() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_ColorHintANGE flag ORed into the mask.
	//��ǰ֡��Ÿı�
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
		//֡��ų���, ǿ����Ϊ0
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

	//���Ʊ�ѡ�����û�����
	GetUserDataForGraph();
	
	//��ȡ��ѡ�е������
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

//ʵ��ͼ����ק����
void CPerfSpyDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
	//ָ���Ƿ�����ͼ��Χ��
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
		//ͼ����ק��
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
		//��ȡ�ܺ�����
		GetTotalRootTime();
		GetTotalNodeDataArr();
		GetTotalUserDataArr();
		//ˢ�½ڵ���û�����list
		RefreshTotalProfileData();
		//�޸Ŀؼ�״̬
		m_ctlBegin.EnableWindow( TRUE );
		m_ctlEnd.EnableWindow( TRUE );
		m_ctlFrom.EnableWindow( TRUE );
		m_ctlTo.EnableWindow( TRUE );
		m_ctlRefresh.EnableWindow( TRUE );
	}
	else
	{
		RefreshProfileData();
		//�޸Ŀؼ�״̬
		m_ctlBegin.EnableWindow( FALSE );
		m_ctlEnd.EnableWindow( FALSE );
		m_ctlFrom.EnableWindow( FALSE );
		m_ctlTo.EnableWindow( FALSE );
		m_ctlRefresh.EnableWindow( FALSE );
	}
}

void CPerfSpyDlg::OnButtonPrevpage() 
{
	//ǰһҳ����
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
	//����
	Invalidate( FALSE );
}

void CPerfSpyDlg::OnButtonNextpage() 
{
	//��һҳ����
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
	//����
	Invalidate( FALSE );
}

void CPerfSpyDlg::OnButtonMaxtime() 
{
	//��λ�����ڵ��ʱ���ֵ���ڵ�����֡
	if ( m_iMaxTimeIndex < 0 || m_iMaxTimeIndex >= m_iTotalDataFrame )
	{
		return;
	}
	m_iCurFrmIdx = m_iMaxTimeIndex;
	ReadDataFrame();
	m_ctlSlider.SetPos( m_iCurFrmIdx );
	UpdateData( FALSE );

	RefreshProfileData();
	//����
	Invalidate( FALSE );
}

void CPerfSpyDlg::OnButtonMintime() 
{
	//��λ�����ڵ��ʱ��Сֵ���ڵ�����֡
	if ( m_iMaxTimeIndex < 0 || m_iMaxTimeIndex >= m_iTotalDataFrame )
	{
		return;
	}
	m_iCurFrmIdx = m_iMinTimeIndex;
	ReadDataFrame();
	m_ctlSlider.SetPos( m_iCurFrmIdx );
	UpdateData( FALSE );

	RefreshProfileData();
	//����
	Invalidate( FALSE );
}

void CPerfSpyDlg::OnButtonUsermax() 
{
	//��λ���û��������ֵ���ڵ�����֡
	if ( m_iMaxUserIndex < 0 || m_iMaxUserIndex >= m_iTotalDataFrame )
	{
		return;
	}
	m_iCurFrmIdx = m_iMaxUserIndex;
	ReadDataFrame();
	m_ctlSlider.SetPos( m_iCurFrmIdx );
	UpdateData( FALSE );

	RefreshProfileData();
	//����
	Invalidate( FALSE );
}

void CPerfSpyDlg::OnButtonUsermin() 
{
	//��λ���û�������Сֵ���ڵ�����֡
	if ( m_iMinUserIndex < 0 || m_iMinUserIndex >= m_iTotalDataFrame )
	{
		return;
	}
	m_iCurFrmIdx = m_iMinUserIndex;
	ReadDataFrame();
	m_ctlSlider.SetPos( m_iCurFrmIdx );
	UpdateData( FALSE );

	RefreshProfileData();
	//����
	Invalidate( FALSE );
}

void CPerfSpyDlg::OnChangeEditScale() 
{
	//��ʾ�����ı�
	UpdateData();
	if ( m_iPaintScale < 1 )
	{
		return;
	}
	//���µı������»���
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

	//��ȡ�ܺ�����
	GetTotalRootTime();
	GetTotalNodeDataArr();
	GetTotalUserDataArr();
	//ˢ�½ڵ���û�����list
	RefreshTotalProfileData();
}

//��ȡ��������֡���ڵ�ĺ�ʱ�ܺͣ�ShowTotal״̬��ʹ�ã�
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

	//�û�������
	iNewCX = iOriginCX_User + ( ( cx > iOriginCX_Wnd ) ? ( cx - iOriginCX_Wnd ) : 0 );
	iNewCY = iOriginCY_User + ( ( cy > iOriginCY_Wnd ) ? ( cy - iOriginCY_Wnd ) / 3 : 0 );

	m_ctlListUser.SetWindowPos( NULL, 0, 0, iNewCX, iNewCY, SWP_NOMOVE | SWP_NOZORDER );

	//Profile������
	iNewX = iOriginX_Pfl;
	iNewY = iOriginY_Pfl + ( ( cy > iOriginCY_Wnd ) ? ( cy - iOriginCY_Wnd ) / 3 : 0 );
	iNewCX = iOriginCX_Pfl + ( ( cx > iOriginCX_Wnd ) ? ( cx - iOriginCX_Wnd ) : 0 );
	iNewCY = iOriginCY_Pfl + ( ( cy > iOriginCY_Wnd ) ? ( cy - iOriginCY_Wnd ) * 2 / 3 : 0 );
	
	m_ctlListProfile.SetWindowPos( NULL, iNewX, iNewY, iNewCX, iNewCY, SWP_NOZORDER );

	//��ɫ��ʾ
	iNewX = iOriginX_ColorHint;
	iNewY = iOriginY_ColorHint + ( ( cy > iOriginCY_Wnd ) ? ( cy - iOriginCY_Wnd ) / 3 : 0 );
	iNewCX = iOriginCX_ColorHint;
	iNewCY = iOriginCY_ColorHint + ( ( cy > iOriginCY_Wnd ) ? ( cy - iOriginCY_Wnd ) * 2 / 3 : 0 );

	m_ctlColorHint.SetWindowPos( NULL, iNewX, iNewY, iNewCX, iNewCY, SWP_NOZORDER );

	//��״ͼ
	iNewCX = iOriginCX_Graph + ( ( cx > iOriginCX_Wnd ) ? ( cx - iOriginCX_Wnd ) : 0 );
	iNewCY = iOriginCY_Graph;

	m_ctlGraph.SetWindowPos( NULL, iNewX, iNewY, iNewCX, iNewCY, SWP_NOZORDER | SWP_NOMOVE );

	//��ť
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
