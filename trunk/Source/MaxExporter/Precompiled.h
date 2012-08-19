//-----------------------------------------------------------------------------
// File: Pch.h
//
// Desc: Header file to precompile
//
// Copyright (C) 1998-2000 Microsoft Corporation. All Rights Reserved.
//-----------------------------------------------------------------------------
#ifndef __PCH__H
#define __PCH__H

#include "Define.h"

#include "windows.h"
#include "Max.h"
#include "resource.h"
#include "istdplug.h"
#include "iparamb2.h"
#include "iparamm2.h"
#include "modstack.h"
#include "bipexp.h"
#include <assert.h>
#include <io.h>
#include <fcntl.h>
#include <direct.h>
#include <commdlg.h>
#include "stdmat.h"

#include <cassert>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <hash_map>
#include <algorithm>
#include <istream>
#include <ostream>
#include <fstream>
#include <stdio.h>

#define STRING				std::wstring
#define VECTOR(t)			std::vector<t>
#define LIST(t)				std::list<t>
#define MAP(t1, t2)			std::map<t1, t2>
#define HASH_MAP(t1, t2)	stdext::hash_map<t1, t2>

#define GRP_NEW new
#define GRP_DELETE(x) delete x;

#define GRP_USE_WCHAR
#define SLIM_USE_WCHAR

#include "Core.h"

#undef max
#undef min

#endif