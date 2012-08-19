// PerfSpyDlg.h : header file
//

#if !defined(AFX_PERFSPYDLG_H__A5B6C7AB_FA24_4A02_8819_A75E42BA9F83__INCLUDED_)
#define AFX_PERFSPYDLG_H__A5B6C7AB_FA24_4A02_8819_A75E42BA9F83__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Performance.h"
#include "afxwin.h"
//#include "afxwin.h"

#define MAX_PAINT_COLOR		6
/////////////////////////////////////////////////////////////////////////////
// CPerfSpyDlg dialog

class CPerfSpyDlg : public CDialog
{
// Construction
public:
	CPerfSpyDlg(CWnd* pParent = NULL);	// standard constructor
	~CPerfSpyDlg();
// Dialog Data
	//{{AFX_DATA(CPerfSpyDlg)
	enum { IDD = IDD_PERFSPY_DIALOG };
	CStatic	m_ctlTo;
	CStatic	m_ctlFrom;
	CEdit	m_ctlEnd;
	CEdit	m_ctlBegin;
	CButton	m_ctlRefresh;
	CStatic	m_ctlColorHint;
	CStatic	m_ctlGraph;
	CListCtrl	m_ctlListUser;
	CSliderCtrl	m_ctlSlider;
	CListCtrl	m_ctlListProfile;
	int		m_iCurFrmIdx;
	int		m_iTotalDataFrame;
	BOOL	m_bShowTotal;
	int		m_iPaintScale;
	int		m_iFrameEnd;
	int		m_iFrameBegin;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPerfSpyDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
private:
	void InitListCtrl();
	void InitUserDataList();

	void OpenDataFile( LPCSTR pPath );
	BOOL DoLoad();

	void ReadDataFrame();
	void InsertProfileLineToList( int iIndex, BOOL bParent, LPCSTR pName,
							int iTotalCalls, float fTotalTime, float fRateParent,
							int iFrameCount, float fRateRoot );
	void RefreshProfileData();
	void RefreshTotalProfileData();

	void GetTimeDataForGraph();
	void GetUserDataForGraph();

	void GetTotalNodeDataArr();
	void GetTotalUserDataArr();

//	void CountUserDataCouple();

	void DrawProfileData();
	void DrawColorHint();
	void DrawOneFrame( int iIndex, int iPosX, int iMaxY, CPaintDC &dc );

	void GetTotalRootTime();

	virtual void OnOK();

protected:
	HICON m_hIcon;

	PerfManager*		m_pPerfManager;

	CString		m_strFile;			//采样数据文件名
	PerfIterator*	m_pIterator;	//采样评估树浏览器
	
	int			m_iUserDataSize;	//用户统计数据个数
	char*		m_szUserDataName;	//用户数据名
	int*		m_pUserData;		//用户数据(单帧)
	__int64*	m_pTotalUserData;	//指向用户数据总和的数组
									//防止总和过大, 用__int64保存
	//"显示总和"模式下需要的数据
	int			m_iTotalFrame;			//总帧数(游戏逻辑帧, 不是数据帧),这个对于任何节点都是一样的
	__int64		m_i64TotalRootTime;		//根节点的耗时总和, 用于计算%total
										//这里所指根节点其实是CPerfManager::Root的儿子
	__int64		m_i64TotalParentTime;	//当前父节点耗时总和
	int			m_iTotalParentCall;		//当前父节点调用总数
	__int64*	m_pTotalChildTime;		//子节点耗时总和数组
	int*		m_pTotalChildCall;		//子节点调用次数总和数组
	
	//绘制图表需要的数据
	int			m_iTotalChild;		//当前父节点的子节点个数
	__int64*	m_pParentTime;		//数组指针, 该数组记录了当前父节点在所有数据帧的耗时
	__int64*	m_pParentTimePerFr;	//数组指针，该数组记录了当前父节点在所有数据帧的每帧耗时
	__int64**	m_ppChildTime;		//指向一个指针数组, 该数组每个元素指向另一个数组
	__int64**	m_ppChildTimePerFr;	//指向一个指针数组, 该数组每个元素指向另一个数组
									//记录了当前父节点的每个子节点在所有数据帧的每帧耗时
	__int64		m_i64MaxTime;		//当前父节点的最大耗时
	__int64		m_i64MaxTimePerFr;	//当前父节点的最大每帧耗时
	__int64		m_i64MinTime;		//当前父节点的最小耗时
	int			m_iMaxTimeIndex;	//当前父节点的最大耗时对应的数据帧序号
	int			m_iMinTimeIndex;	//当前父节点的最小耗时对应的数据帧序号
	int*		m_pUserDataGraph;	//某个用户数据数组(所有帧)
									//用户数据List中被选中行对应的用户数据也将被绘制成曲线

	//绘图时只画选中的用户数据, 所以最大最小值只是一个数而不是一个数组
	int			m_iMaxUserData;		//用户数据最大值
	int			m_iMinUserData;		//用户数据最小值
	int			m_iMaxUserIndex;	//最大用户数据对应的数据帧序号
	int			m_iMinUserIndex;	//最小用户数据对应的数据帧序号

	__int64		m_i64UserDataCouple;//当前选中的用户数据和当前父节点数据的耦合度

	CPen		m_pen[MAX_PAINT_COLOR];		//绘图用画笔
	CBrush		m_brush[MAX_PAINT_COLOR];	//绘图用画刷
	COLORREF	m_color[MAX_PAINT_COLOR];	//绘图用颜色
	CBrush		m_br;						//绘制用户数据用的画刷	
	CPen		m_penBlack;
	CPen		m_penWhite;

	BOOL		m_bLBtnDown;		//左键是否按下
	int			m_iMouseLastPosX;	//鼠标上一次位置

	BOOL		m_bInitialized;

	// Generated message map functions
	//{{AFX_MSG(CPerfSpyDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnButtonOpen();
	afx_msg void OnClickListProfile(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnButtonPrev();
	afx_msg void OnButtonNext();
	afx_msg void OnReleasedcaptureSliderProgress(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeEditFrameindex();
	afx_msg void OnKillfocusEditFrameindex();
	afx_msg void OnClickListUserdata(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnCheckShowtotal();
	afx_msg void OnButtonPrevpage();
	afx_msg void OnButtonNextpage();
	afx_msg void OnButtonMaxtime();
	afx_msg void OnButtonMintime();
	afx_msg void OnButtonUsermax();
	afx_msg void OnButtonUsermin();
	afx_msg void OnChangeEditScale();
	afx_msg void OnButtonRefresh();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	// 采样时间
	CString m_strFrameTime;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	CButton m_ctlOpen;
	CButton m_ctlQuit;
	afx_msg void OnDropFiles(HDROP hDropInfo);
	BOOL m_bDrawTimePerFr;
	afx_msg void OnBnClickedRadioTime();
	afx_msg void OnBnClickedRadioTimePerFr();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PERFSPYDLG_H__A5B6C7AB_FA24_4A02_8819_A75E42BA9F83__INCLUDED_)
