///////////////////////////////////////////////////////////////////////////////
//GrandpaExport.h
//
//描述：
//
//历史：
//		07-12-10	创建
///////////////////////////////////////////////////////////////////////////////
#ifndef ___GRP_EXPORT_H__
#define ___GRP_EXPORT_H__

#include "Max.h"
#include "resource.h"
#include "istdplug.h"
#include "iparamb2.h"
#include "iparamm2.h"
#include <vector>

extern TCHAR *GetString(int id);
extern HINSTANCE g_hInstance;

#define SKANIEXPORT_CLASS_ID	Class_ID(0x00dacfae, 0x00a8c565)

///////////////////////////////////////////////////////////////////////////////
//实现SceneExport接口类
///////////////////////////////////////////////////////////////////////////////
class CGrandpaExport : public SceneExport
{
public:
	CGrandpaExport();
	~CGrandpaExport();		

public:
	static HWND hParams;

	virtual int				ExtCount();					// Number of extensions supported
	virtual const TCHAR *	Ext(int n);					// Extension #n (i.e. "3DS")
	virtual const TCHAR *	LongDesc();					// Long ASCII description (i.e. "Autodesk 3D Studio File")
	virtual const TCHAR *	ShortDesc();				// Short ASCII description (i.e. "3D Studio")
	virtual const TCHAR *	AuthorName();				// ASCII Author name
	virtual const TCHAR *	CopyrightMessage();			// ASCII Copyright message
	virtual const TCHAR *	OtherMessage1();			// Other message #1
	virtual const TCHAR *	OtherMessage2();			// Other message #2
	virtual unsigned int	Version();					// Version number * 100 (i.e. v3.01 = 301)
	virtual void			ShowAbout(HWND hWnd);		// Show DLL's "About..." box

	virtual BOOL SupportsOptions(int ext, DWORD options);
	virtual int	 DoExport(const TCHAR *name,
						   ExpInterface *pExpInterface,
						   Interface *pInterface,
						   BOOL suppressPrompts = FALSE,
						   DWORD options = 0);
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
class CGrandpaClassDesc : public ClassDesc2
{
public:
	int 			IsPublic() { return TRUE; }
	void *			Create(BOOL loading = FALSE) { return new CGrandpaExport(); }
	const TCHAR *	ClassName() { return GetString(IDS_DESC_SHORT); }
	SClass_ID		SuperClassID() { return SCENE_EXPORT_CLASS_ID; }
	Class_ID		ClassID() { return SKANIEXPORT_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_CATEGORY); }

	const TCHAR*	InternalName() { return "GrandpaExporter"; }	// returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return g_hInstance; }				// returns owning module handle

};

#endif
