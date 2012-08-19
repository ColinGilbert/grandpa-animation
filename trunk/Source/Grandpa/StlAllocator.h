#ifndef __GRP_STL_ALLOCATOR_H__
#define __GRP_STL_ALLOCATOR_H__

#include <limits>
#include <string>
#include <vector>
#include <list>
#include <map>
#if defined (_MSC_VER)
	#include <hash_map>
#elif defined (__GNUC__)
	#include <ext/hash_map>
#endif

namespace grp
{

template<typename Type>
class StlAllocator
{
public:
	typedef Type value_type;
	typedef value_type* pointer;
	typedef const value_type* const_pointer;
	typedef value_type& reference;
	typedef const value_type& const_reference;
	typedef std::size_t size_type;
	typedef std::ptrdiff_t difference_type;

public:
	template<typename U>
	struct rebind
	{
		typedef StlAllocator<U> other;
	};

public:
	inline StlAllocator() {}
	inline ~StlAllocator() {}
	inline StlAllocator(StlAllocator const&) {}
	template<typename U>
	inline StlAllocator(StlAllocator<U> const&) {}

	inline pointer address(reference r) { return &r; }
	inline const_pointer address(const_reference r) { return &r; }

	inline pointer allocate(size_type cnt, typename std::allocator<void>::const_pointer = 0)
	{ 
		return reinterpret_cast<pointer>(grp::g_allocator->allocateChunk(cnt * sizeof(Type))); 
	}
	inline void deallocate(pointer p, size_type size)
	{ 
		grp::g_allocator->deallocateChunk(p);
	}

	inline size_type max_size() const
	{ 
		return std::numeric_limits<size_type>::max() / sizeof(Type);
	}

	inline void construct(pointer p, const Type& t) { new(p) Type(t); }
	inline void destroy(pointer p) { p->~Type(); }

	inline bool operator==(StlAllocator const&) const { return true; }
	inline bool operator!=(StlAllocator const& a) const { return !operator==(a); }
};

//containers using our own allocator
#define VECTOR(t)			std::vector<t, StlAllocator<t> >
#define LIST(t)				std::list<t, StlAllocator<t> >
#define MAP(t1, t2)			std::map<t1, t2, std::less<t1>, StlAllocator<std::pair<const t1, t2> > >

#if defined (_MSC_VER)
	#define STRING				std::basic_string<Char, std::char_traits<Char>, StlAllocator<Char> >
	#define HASH_MAP(t1, t2)	stdext::hash_map<t1, t2, stdext::hash_compare<t1, std::less<t1> >, StlAllocator<std::pair<const t1, t2> > >
#elif defined (__GNUC__)
	#if defined (GRP_USE_WCHAR)
		#define STRING			std::wstring
	#else
		#define STRING			std::string
	#endif
	struct str_hash
	{
		size_t operator()(const STRING& str) const
		{
			return __gnu_cxx::__stl_hash_string(str.c_str());
		}
	};
	#define HASH_MAP(t1, t2)	__gnu_cxx::hash_map<t1, t2, str_hash, __gnu_cxx::equal_to<t1>, StlAllocator<std::pair<const t1, t2> > >
#endif
}

#endif
