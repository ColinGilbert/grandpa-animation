/**********************************************************************
 *<
	FILE: DllEntry.cpp

	DESCRIPTION: Contains the Dll Entry stuff

	CREATED BY: 

	HISTORY: 

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/
#include "GrandpaExport.h"

extern ClassDesc2* GetGrandpsExportDesc();

HINSTANCE g_hInstance;
int controlsInit = FALSE;
 
 
BOOL WINAPI DllMain(HINSTANCE hinstDLL,ULONG fdwReason,LPVOID lpvReserved)
{
	g_hInstance = hinstDLL;				// Hang on to this DLL's instance handle.

	if (!controlsInit) {
		controlsInit = TRUE;
		InitCustomControls(g_hInstance);	// Initialize MAX's custom controls
		InitCommonControls();			// Initialize Win95 controls
	}
			
	return (TRUE);
}

__declspec(dllexport) const TCHAR* LibDescription()
{
	return GetString(IDS_LIBDESCRIPTION);
}

//TODO: Must change this number when adding a new class
__declspec(dllexport) int LibNumberClasses()
{
	return 1;
}

__declspec(dllexport) ClassDesc* LibClassDesc(int i)
{
	switch(i) {
		case 0: return GetGrandpsExportDesc();
		default: return 0;
	}
}

__declspec(dllexport) ULONG LibVersion()
{
	return VERSION_3DSMAX;
}

TCHAR *GetString(int id)
{
	static TCHAR buf[256];

	if (g_hInstance)
		return LoadString(g_hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
}

