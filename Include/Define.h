#ifndef __GRP_DEFINE_H__
#define __GRP_DEFINE_H__

#ifndef GRANDPA_STATICLIB
	#ifdef GRANDPA_EXPORTS
		#define GRANDPA_API	__declspec(dllexport)
	#else
		#define GRANDPA_API	__declspec(dllimport)
	#endif
#else
	#define GRANDPA_API
#endif


#if defined (_MSC_VER) && defined (UNICODE)
	#define GRP_USE_WCHAR
#endif

namespace grp
{

typedef unsigned long Color32;

typedef unsigned long Index32;

typedef unsigned short Index16;

#if defined (GRP_USE_WCHAR)
	typedef wchar_t Char;
	#define GT(str) L##str
#else
	typedef char Char;
	#define GT(str) str
#endif

}

#define GRANDPA_SQRT_TABLE

#define GRANDPA_EXCEPTION

#if defined(GRANDPA_EXCEPTION)
#else
	#if defined (_MSC_VER)
		#define _HAS_EXCEPTIONS 0
		#define _STATIC_CPPLIB
	#endif
#endif

#endif
