// GredDlg.h : header file
//

#pragma once

#include <vector>
#include <string>
#include "afxwin.h"
#include "afxcmn.h"
#include "SlimXml.h"
#include "DemoCamera.h"

namespace grp
{
class IModel;
}
namespace slim
{
class XmlDocument;
class XmlNode;
}
class DemoCharacter;
struct IDirect3D9;
struct IDirect3DDevice9;
struct ID3DXEffect;
struct IDirect3DVertexDeclaration9;

// CGredDlg dialog
class CGredDlg : public CDialog
{
// Construction
public:
	CGredDlg(CWnd* pParent = NULL);	// standard constructor
	~CGredDlg();

// Dialog Data
	enum { IDD = IDD_GRED_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	virtual void OnOK(){}

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

	bool initDevice();

	void initCamera();

	void initPropertyList();
	void initPartList();
	void initPartPropertyList();
	void initAnimationList();
	void initEventList();
	void initEventParamList();

	void updateModelList();
	void updateSkeletonList();

	void createGrpModel();

	void refreshModel();

	void readModel();

	bool isSkinnedPart( slim::XmlNode* partNode );

	void refreshPropertyList();
	void refreshAnimationList();
	void refreshEventList();
	void refreshEventParamList();
	void refreshPartList();
	void refreshPartPropertyList();

	std::vector<std::wstring>	m_models;
	//std::vector<std::wstring>	m_parts;
	//std::vector<std::wstring>	m_skeletons;

	std::wstring	m_modelFilename;

	DemoCharacter*	m_model;

	slim::XmlDocument			m_modelFile;
	slim::XmlNode*				m_modelNode;

	//part property不能编辑，所以没有m_partPropertyNode和m_partPropertyNodes;
	slim::XmlNode*				m_propertyNode;
	slim::XmlNode*				m_partNode;
	slim::XmlNode*				m_animationNode;
	slim::XmlNode*				m_eventNode;
	slim::XmlNode*				m_eventParamNode;

	std::vector<slim::XmlNode*>	m_propertyNodes;
	std::vector<slim::XmlNode*>	m_partNodes;
	std::vector<slim::XmlNode*>	m_animationNodes;
	std::vector<slim::XmlNode*>	m_eventNodes;
	std::vector<slim::XmlNode*>	m_eventParamNodes;
	
	DemoCamera					m_camera;

	enum LIST_ENUM
	{
		LIST_NONE = 0,
		LIST_PROPERTY,
		LIST_PART,
		LIST_ANIMATION,
		LIST_EVENT,
		LIST_EVENT_PARAM
	};
	LIST_ENUM	m_lastEditList;
	int			m_lastEditRow;
	int			m_lastEditColumn;

	IDirect3D9*						m_d3d;
	IDirect3DDevice9*				m_device;
	ID3DXEffect*					m_effect;
	IDirect3DVertexDeclaration9*	m_vertexDeclaration;
	float							m_time;

private:
	CComboBox m_modelComboBox;

	CListCtrl m_partList;
	CListCtrl m_animationList;
	CListCtrl m_eventList;
	CListCtrl m_propertyList;
	CListCtrl m_partPropertyList;
	CListCtrl m_eventParamList;

	CStatic m_preview;

	int m_timeScale;
	CString m_timeScaleString;
	CStatic m_timeCtrl;

	CPoint	m_lastPoint;
	bool	m_paused;
	bool	m_rbuttonPushed;

private:
	afx_msg void OnCbnSelchangeModels();
	afx_msg void OnBnClickedBrowseModel();
	afx_msg void OnCbnEditchangeModels();
	afx_msg void OnCbnSelchangeSkeletons();
	afx_msg void OnCbnEditchangeSkeletons();
	afx_msg void OnBnClickedBrowseSkeleton();
	afx_msg void OnNMClickParts(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkParts(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickAnimations(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickEvents(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedAddPart();
	afx_msg void OnBnClickedNewPart();
	afx_msg void OnBnClickedEditPart();
	afx_msg void OnBnClickedRemovePart();
	afx_msg void OnBnClickedAddAnimation();
	afx_msg void OnBnClickedRemoveAnimation();
	afx_msg void OnBnClickedAddEvent();
	afx_msg void OnBnClickedRemoveEvent();
	afx_msg void OnBnClickedNewModel();
	afx_msg void OnBnClickedAddProperty();
	afx_msg void OnBnClickedRemoveProperty();
	afx_msg void OnBnClickedAddEventParam();
	afx_msg void OnBnClickedRemoveEventParam();
	afx_msg void OnNMClickEventParams(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickProperties(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnNMReleasedcaptureTimeScaleSlider(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMReleasedcaptureTimeSlider(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedPause();
private:
	CButton m_pauseButton;
	CSliderCtrl m_timeSlider;
public:
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	CString m_skeletonName;
};
