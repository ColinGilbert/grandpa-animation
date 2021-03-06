﻿///////////////////////////////////////////////////////////////////////////////
//GrandpaExport.cpp
//
//描述：
//
//历史
//		07-12-10	创建
///////////////////////////////////////////////////////////////////////////////
#include "Precompiled.h"
#include "GrandpaExport.h"
#include "Exporter.h"
#include "StrSafe.h"

static CGrandpaClassDesc GrandpaExportDesc;

/////////////////////////////////////////////////////////////////////////////////////////////
ClassDesc2* GetGrandpsExportDesc()
{
	return &GrandpaExportDesc;
}

/////////////////////////////////////////////////////////////////////////////////////////////
CGrandpaExport::CGrandpaExport()
{
	std::locale::global( std::locale("") );
}

/////////////////////////////////////////////////////////////////////////////////////////////
CGrandpaExport::~CGrandpaExport() 
{
}

/////////////////////////////////////////////////////////////////////////////////////////////
int CGrandpaExport::ExtCount()
{
	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////
const TCHAR *CGrandpaExport::Ext(int n)
{
	return "";//GetString(IDS_FILE_EXT);
}

/////////////////////////////////////////////////////////////////////////////////////////////
const TCHAR *CGrandpaExport::LongDesc()
{
	return "Grandpa";
}

/////////////////////////////////////////////////////////////////////////////////////////////
const TCHAR *CGrandpaExport::ShortDesc() 
{
	return "Grandpa";
}

/////////////////////////////////////////////////////////////////////////////////////////////
const TCHAR *CGrandpaExport::AuthorName()
{
	return GetString(IDS_AUTHOR_NAME);
}

/////////////////////////////////////////////////////////////////////////////////////////////
const TCHAR *CGrandpaExport::CopyrightMessage() 
{
	return GetString(IDS_COPYRIGHT);
}

/////////////////////////////////////////////////////////////////////////////////////////////
const TCHAR *CGrandpaExport::OtherMessage1() 
{
	return GetString(IDS_MESSAGE1);
}

/////////////////////////////////////////////////////////////////////////////////////////////
const TCHAR *CGrandpaExport::OtherMessage2() 
{
	return GetString(IDS_MESSAGE2);
}

/////////////////////////////////////////////////////////////////////////////////////////////
unsigned int CGrandpaExport::Version()
{
	return 100;
}

/////////////////////////////////////////////////////////////////////////////////////////////
void CGrandpaExport::ShowAbout( HWND hWnd )
{			
}

/////////////////////////////////////////////////////////////////////////////////////////////
BOOL CGrandpaExport::SupportsOptions(int ext, DWORD options)
{
	// TODO Decide which options to support.  Simply return
	// true for each option supported by each Extension 
	// the exporter supports.
	return ( options == SCENE_EXPORT_SELECTED );
}

/////////////////////////////////////////////////////////////////////////////////////////////
int	CGrandpaExport::DoExport( const TCHAR *szFilename,
							  ExpInterface *pExpInterface,
							  Interface *pInterface,
							  BOOL suppressPrompts,
							  DWORD options ) 
{
	if ( NULL == szFilename )
	{
		return FALSE;
	}

	//unicode版max sdk编不过，只好这样了
	wchar_t unicodeFilename[MAX_PATH];
	mbstowcs( unicodeFilename, szFilename, MAX_PATH - 1 );

	CExporter exporter( pInterface );
	if ( !exporter.doExport( unicodeFilename ) )
	{
		::MessageBox( NULL, exporter.getLastError().c_str(), "Error", MB_OK | MB_ICONERROR );
		return TRUE;	//自己报错了就别让max再报错
	}

	return TRUE;
}