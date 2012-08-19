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

	CString		m_strFile;			//���������ļ���
	PerfIterator*	m_pIterator;	//���������������
	
	int			m_iUserDataSize;	//�û�ͳ�����ݸ���
	char*		m_szUserDataName;	//�û�������
	int*		m_pUserData;		//�û�����(��֡)
	__int64*	m_pTotalUserData;	//ָ���û������ܺ͵�����
									//��ֹ�ܺ͹���, ��__int64����
	//"��ʾ�ܺ�"ģʽ����Ҫ������
	int			m_iTotalFrame;			//��֡��(��Ϸ�߼�֡, ��������֡),��������κνڵ㶼��һ����
	__int64		m_i64TotalRootTime;		//���ڵ�ĺ�ʱ�ܺ�, ���ڼ���%total
										//������ָ���ڵ���ʵ��CPerfManager::Root�Ķ���
	__int64		m_i64TotalParentTime;	//��ǰ���ڵ��ʱ�ܺ�
	int			m_iTotalParentCall;		//��ǰ���ڵ��������
	__int64*	m_pTotalChildTime;		//�ӽڵ��ʱ�ܺ�����
	int*		m_pTotalChildCall;		//�ӽڵ���ô����ܺ�����
	
	//����ͼ����Ҫ������
	int			m_iTotalChild;		//��ǰ���ڵ���ӽڵ����
	__int64*	m_pParentTime;		//����ָ��, �������¼�˵�ǰ���ڵ�����������֡�ĺ�ʱ
	__int64*	m_pParentTimePerFr;	//����ָ�룬�������¼�˵�ǰ���ڵ�����������֡��ÿ֡��ʱ
	__int64**	m_ppChildTime;		//ָ��һ��ָ������, ������ÿ��Ԫ��ָ����һ������
	__int64**	m_ppChildTimePerFr;	//ָ��һ��ָ������, ������ÿ��Ԫ��ָ����һ������
									//��¼�˵�ǰ���ڵ��ÿ���ӽڵ�����������֡��ÿ֡��ʱ
	__int64		m_i64MaxTime;		//��ǰ���ڵ������ʱ
	__int64		m_i64MaxTimePerFr;	//��ǰ���ڵ�����ÿ֡��ʱ
	__int64		m_i64MinTime;		//��ǰ���ڵ����С��ʱ
	int			m_iMaxTimeIndex;	//��ǰ���ڵ������ʱ��Ӧ������֡���
	int			m_iMinTimeIndex;	//��ǰ���ڵ����С��ʱ��Ӧ������֡���
	int*		m_pUserDataGraph;	//ĳ���û���������(����֡)
									//�û�����List�б�ѡ���ж�Ӧ���û�����Ҳ�������Ƴ�����

	//��ͼʱֻ��ѡ�е��û�����, ���������Сֵֻ��һ����������һ������
	int			m_iMaxUserData;		//�û��������ֵ
	int			m_iMinUserData;		//�û�������Сֵ
	int			m_iMaxUserIndex;	//����û����ݶ�Ӧ������֡���
	int			m_iMinUserIndex;	//��С�û����ݶ�Ӧ������֡���

	__int64		m_i64UserDataCouple;//��ǰѡ�е��û����ݺ͵�ǰ���ڵ����ݵ���϶�

	CPen		m_pen[MAX_PAINT_COLOR];		//��ͼ�û���
	CBrush		m_brush[MAX_PAINT_COLOR];	//��ͼ�û�ˢ
	COLORREF	m_color[MAX_PAINT_COLOR];	//��ͼ����ɫ
	CBrush		m_br;						//�����û������õĻ�ˢ	
	CPen		m_penBlack;
	CPen		m_penWhite;

	BOOL		m_bLBtnDown;		//����Ƿ���
	int			m_iMouseLastPosX;	//�����һ��λ��

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
	// ����ʱ��
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
