// PerfSpy.h : main header file for the PERFSPY application
//

#if !defined(AFX_PERFSPY_H__9B4546BB_852D_4D1A_B7DF_6FE6D2D019E2__INCLUDED_)
#define AFX_PERFSPY_H__9B4546BB_852D_4D1A_B7DF_6FE6D2D019E2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CPerfSpyApp:
// See PerfSpy.cpp for the implementation of this class
//

class CPerfSpyApp : public CWinApp
{
public:
	CPerfSpyApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPerfSpyApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CPerfSpyApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PERFSPY_H__9B4546BB_852D_4D1A_B7DF_6FE6D2D019E2__INCLUDED_)
