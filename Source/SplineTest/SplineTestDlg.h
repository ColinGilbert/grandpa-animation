
// SplineTestDlg.h : header file
//

#pragma once

#include <vector>
// CSplineTestDlg dialog
class CSplineTestDlg : public CDialog
{
// Construction
public:
	CSplineTestDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_SPLINETEST_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

	void fit( float threshold );
	void resetFitting();

	std::vector<grp::Vector2>	m_points;
	std::vector<float>			m_timeArray;
	std::vector<grp::Vector2>	m_knots;

	int		m_pickedKnot;

	int		m_currentPoint;
	float	m_currentT;

	std::vector<size_t>			m_fitPointIndices;
	std::vector<grp::Vector2>	m_fitPoints;
	std::vector<float>			m_fitTimeArray;
	std::vector<grp::Vector2>	m_fitKnots;
	int	m_lastInsert;

public:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedButtonFit();
	afx_msg void OnBnClickedButtonUndo();
	float m_maxError;
	afx_msg void OnBnClickedButtonClear();
	afx_msg void OnBnClickedCheckPolygon();
	BOOL m_drawPolygon;
};
