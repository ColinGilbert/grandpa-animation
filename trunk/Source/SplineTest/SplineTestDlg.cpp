
// SplineTestDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SplineTest.h"
#include "SplineTestDlg.h"
#include "Grandpa.h"
#include "float.h"
#include "SplineFunctions.h"
#include "Bezier.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


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


// CSplineTestDlg dialog




CSplineTestDlg::CSplineTestDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSplineTestDlg::IDD, pParent)
	, m_pickedKnot( -1 )
	, m_currentPoint( 0 )
	, m_currentT( 0.0f )
	, m_lastInsert( -1 )

	, m_maxError(0)
	, m_drawPolygon(FALSE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSplineTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_ERROR, m_maxError);
	DDX_Check(pDX, IDC_CHECK_POLYGON, m_drawPolygon);
}

BEGIN_MESSAGE_MAP(CSplineTestDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_FIT, &CSplineTestDlg::OnBnClickedButtonFit)
	ON_BN_CLICKED(IDC_BUTTON_UNDO, &CSplineTestDlg::OnBnClickedButtonUndo)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR, &CSplineTestDlg::OnBnClickedButtonClear)
	ON_BN_CLICKED(IDC_CHECK_POLYGON, &CSplineTestDlg::OnBnClickedCheckPolygon)
END_MESSAGE_MAP()


// CSplineTestDlg message handlers

BOOL CSplineTestDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
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

	//SetTimer( 1, 20, NULL );

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSplineTestDlg::OnSysCommand(UINT nID, LPARAM lParam)
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
void drawPoints( CPaintDC& dc, const std::vector<grp::Vector2>& points )
{
	for ( size_t i = 0; i < points.size(); ++i )
	{
		RECT rect;
		rect.left = (long)points[i].X - 3;
		rect.top = (long)points[i].Y - 3;
		rect.right = (long)points[i].X + 3;
		rect.bottom = (long)points[i].Y + 3;
		dc.Rectangle( &rect );
	}
}

void drawSpline( CPaintDC& dc, const std::vector<grp::Vector2>& points, const std::vector<grp::Vector2>& knots )
{
	if ( points.size() < 2 )
	{
		return;
	}
	std::vector<POINT> ptArr( points.size() * 3 - 2 );
	for ( size_t i = 0; i + 1 < points.size(); ++i )
	{
		ptArr[3*i].x = (long)points[i].X;
		ptArr[3*i].y = (long)points[i].Y;
		ptArr[3*i+1].x = (long)knots[i*2].X;
		ptArr[3*i+1].y = (long)knots[i*2].Y;
		ptArr[3*i+2].x = (long)knots[i*2+1].X;
		ptArr[3*i+2].y = (long)knots[i*2+1].Y;
	}
	ptArr.back().x = (long)points.back().X;
	ptArr.back().y = (long)points.back().Y;
	dc.PolyBezier( &ptArr[0], ptArr.size() );
}

void drawLine( CPaintDC& dc, const std::vector<grp::Vector2>& points )
{
	if ( points.size() < 2 )
	{
		return;
	}
	std::vector<POINT> ptArr( points.size() );
	for ( size_t i = 0; i < points.size(); ++i )
	{
		ptArr[i].x = (long)points[i].X;
		ptArr[i].y = (long)points[i].Y;
	}
	dc.Polyline( &ptArr[0], ptArr.size() );
}

void CSplineTestDlg::OnPaint()
{
	static HPEN redPen = CreatePen( 0, 1, RGB( 255, 126, 0 ) );
	static HPEN bluePen = CreatePen( 0, 1, RGB( 0, 0, 255 ) );
	static HPEN greyPen = CreatePen( 0, 1, RGB( 180, 180, 180 ) );

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
		CPaintDC dc(this);
		
		//if ( m_points.size() >= 2 )
		//{
		//	grp::Vector2 current = grp::sampleCubicBezier( m_points[m_currentPoint],
		//													m_knots[m_currentPoint*2],
		//													m_knots[m_currentPoint*2+1],
		//													m_points[m_currentPoint+1],
		//													m_currentT );
		//	POINT currentPos;
		//	currentPos.x = (int)current.X;
		//	currentPos.y = (int)current.Y;
		//	dc.Ellipse( currentPos.x - 4, currentPos.y - 4, currentPos.x + 4, currentPos.y + 4 );
		//}

		if ( m_drawPolygon )
		{
			dc.SelectObject( greyPen );
			drawLine( dc, m_knots );
		}

		dc.SelectObject( bluePen );
		drawPoints( dc, m_points );
		drawSpline( dc, m_points, m_knots );

		dc.SelectObject( redPen );
		drawPoints( dc, m_fitPoints );
		drawSpline( dc, m_fitPoints, m_fitKnots );

		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CSplineTestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CSplineTestDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	resetFitting();

	for ( size_t i = 0; i < m_points.size(); ++i )
	{
		if ( fabs( m_points[i].X - point.x ) <= 3 && fabs( m_points[i].Y - point.y ) <= 3 )
		{
			m_pickedKnot = i;
			break;
		}
	}
	
	if ( m_pickedKnot < 0 )
	{
		m_pickedKnot = m_points.size();
		m_points.push_back( grp::Vector2( (float)point.x, (float)point.y ) );
		m_timeArray.push_back( (float)m_timeArray.size() );
	}

	if ( m_points.size() > 1 )
	{
		m_knots.resize( ( m_points.size() - 1 ) * 2 );
		grp::getSplineKnots( &m_points[0], m_points.size(), &m_knots[0], &m_timeArray[0] );
	}

	InvalidateRect( NULL );

	::SetCapture( m_hWnd );
	CDialog::OnLButtonDown(nFlags, point);
}

void CSplineTestDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	if ( m_pickedKnot >= 0 )
	{
		m_points[m_pickedKnot].X = (float)point.x;
		m_points[m_pickedKnot].Y = (float)point.y;

		if ( m_points.size() > 1 )
		{
			grp::getSplineKnots( &m_points[0], m_points.size(), &m_knots[0], &m_timeArray[0] );
		}
		InvalidateRect( NULL );
	}
	CDialog::OnMouseMove(nFlags, point);
}

void CSplineTestDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	::ReleaseCapture();
	m_pickedKnot = -1;
	CDialog::OnLButtonUp(nFlags, point);
}

void CSplineTestDlg::OnTimer(UINT_PTR nIDEvent)
{
	m_currentT += 0.05f;
	if ( m_currentT > 1.0f )
	{
		m_currentT -= 1.0f;
		m_currentPoint++;
		if ( m_currentPoint + 1 >= (int)m_points.size() )
		{
			m_currentPoint = 0;
		}
	}
	InvalidateRect( NULL );

	CDialog::OnTimer(nIDEvent);
}

void CSplineTestDlg::fit( float threshold )
{
	size_t maxErrorIndex = 0;
	size_t insertIndex = 0;
	float maxError;

	if ( m_fitPoints.empty() )
	{
		m_fitPoints.push_back( m_points[0] );
		m_fitTimeArray.push_back( 0.0f );
		m_fitPointIndices.push_back( 0 );
		return;
	}

	m_fitKnots.resize( m_fitPoints.size() * 2 );

	grp::getSplineKnots( &m_fitPoints[0], m_fitPoints.size(), &m_fitKnots[0], &m_fitTimeArray[0] );

	grp::getMaxErrorPoint( &m_fitKnots[0], &m_fitPointIndices[0], m_fitPoints.size(),
							&m_points[0], &m_timeArray[0], m_points.size(), threshold, maxErrorIndex, insertIndex, maxError );
	
	m_fitPointIndices.insert( m_fitPointIndices.begin() + insertIndex, maxErrorIndex );
	m_fitPoints.insert( m_fitPoints.begin() + insertIndex, m_points[maxErrorIndex] );
	m_fitTimeArray.insert( m_fitTimeArray.begin() + insertIndex, m_timeArray[maxErrorIndex] );
	m_fitKnots.resize( m_fitPoints.size() * 2 );

	m_lastInsert = insertIndex;

	grp::getSplineKnots( &m_fitPoints[0], m_fitPoints.size(), &m_fitKnots[0], &m_fitTimeArray[0] );

	grp::getMaxErrorPoint( &m_fitKnots[0], &m_fitPointIndices[0], m_fitPoints.size(),
							&m_points[0], NULL, m_points.size(), threshold, maxErrorIndex, insertIndex, maxError );
	m_maxError = maxError;
	UpdateData( FALSE );

}

void CSplineTestDlg::OnBnClickedButtonFit()
{
	fit( 1.0f );
	InvalidateRect( NULL );
}

void CSplineTestDlg::OnBnClickedButtonUndo()
{
	if ( m_lastInsert >= 0 )
	{
		m_fitPoints.erase( m_fitPoints.begin() + m_lastInsert );
		m_fitPointIndices.erase( m_fitPointIndices.begin() + m_lastInsert );
		m_fitTimeArray.erase( m_fitTimeArray.begin() + m_lastInsert );
		m_fitKnots.resize( m_fitPoints.size() * 2 );
		grp::getSplineKnots( &m_fitPoints[0], m_fitPoints.size(), &m_fitKnots[0], &m_fitTimeArray[0] );

		m_lastInsert = -1;

		size_t maxErrorIndex = 0;
		size_t insertIndex = 0;
		grp::getMaxErrorPoint( &m_fitKnots[0], &m_fitPointIndices[0], m_fitPoints.size(),
								&m_points[0], NULL, m_points.size(), 1.0f, maxErrorIndex, insertIndex, m_maxError );

		UpdateData( FALSE );

		InvalidateRect( NULL );
	}
}

void CSplineTestDlg::resetFitting()
{
	m_fitPoints.clear();
	m_fitPointIndices.clear();
	m_fitKnots.clear();
	m_lastInsert = -1;
}

void CSplineTestDlg::OnBnClickedButtonClear()
{
	m_points.clear();
	m_knots.clear();
	m_pickedKnot = -1;
	m_currentPoint = 0;
	m_currentT = 0.0f;
	m_maxError = 0.0f;
	resetFitting();
	UpdateData( FALSE );
	InvalidateRect( NULL );
}

void CSplineTestDlg::OnBnClickedCheckPolygon()
{
	UpdateData();
	InvalidateRect( NULL );
}
