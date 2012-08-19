#ifndef __PERFORMANCE_H__
#define __PERFORMANCE_H__

#ifdef _WIN32

#include "stdio.h"
#include "time.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
//获取本机CPU频率
///////////////////////////////////////////////////////////////////////////////////////////////////
long long PerfGetTickRate();

///////////////////////////////////////////////////////////////////////////////////////////////////
//读取CPU指令计数器值
///////////////////////////////////////////////////////////////////////////////////////////////////
inline void PerfGetTicks( long long * ticks )
{
	unsigned int lowHalf, highHalf;
	__asm
	{
		rdtsc
		mov lowHalf, eax;
		mov highHalf, edx;
	}
	*ticks = ( (unsigned long long)highHalf << 32 ) | (unsigned long long)lowHalf;
}

#define	MAX_NODE_NAME_LEN	64

//pfl文件头
struct PERF_FILE_HEADER;
struct PERF_NODE_HEADER;
struct PERF_NODE_DATA;

///////////////////////////////////////////////////////////////////////////////////////////////////
// 性能评估树节点
///////////////////////////////////////////////////////////////////////////////////////////////////
class PerfNode
{
public:
	PerfNode( const char* name, PerfNode *parent );
	PerfNode( const char* name );
	~PerfNode();

	void Reset();
	void Call();
	bool Return();

	PerfNode* GetParent()
	{
		return Parent;
	}
	PerfNode* GetSibling()
	{
		return Sibling;
	}
	PerfNode* GetChild()
	{
		return Child;
	}
	const char* GetName()
	{
		return Name;
	}
	int GetIndex()
	{
		return iIndex;
	}
	int	GetTotalCalls()
	{
		return TotalCalls;
	}
	long long GetTotalTime()
	{
		return TotalTime;
	}

protected:
	const char*		Name;				//节点名
	int				TotalCalls;			//总调用次数
	long long			TotalTime;			//总开销时间
	long long			StartTime;			//最后调用时间
	int				RecursionCounter;	//递归计数

	PerfNode*	Parent;
	PerfNode*	Child;
	PerfNode*	Sibling;

	//节点序号，为了存盘时候的方便
	int				iIndex;
	
	friend	class	PerfManager;
};

inline void PerfNode::Call()
{
	TotalCalls++;
	if ( RecursionCounter++ == 0 )
	{
		PerfGetTicks( &StartTime );
	}
}

inline bool PerfNode::Return()
{
	if ( --RecursionCounter == 0 && TotalCalls != 0 )
	{ 
		long long time;
		PerfGetTicks( &time );
		time -= StartTime;
		TotalTime += time;
	}
	return ( RecursionCounter == 0 );
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// 评估树浏览器
///////////////////////////////////////////////////////////////////////////////////////////////////
class PerfIterator
{
public:
	// Access all the children of the current parent
	PerfNode* First();
	PerfNode* Next();

	bool IsDone();
	void EnterChild( int index );	// Make the given child the new parent
	void EnterParent();				// Make the current parent's parent the new parent
	int GetTotalChild();			//获取当前parent的child个数

	// Access the current child
	const char* GetCurrentName()
	{
		return CurrentChild->GetName();
	}
	int	GetCurrentTotalCalls()
	{
		return CurrentChild->GetTotalCalls();
	}
	long long GetCurrentTotalTime()
	{
		return CurrentChild->GetTotalTime();
	}

	const char* GetCurrentParentName()
	{
		return CurrentParent->GetName();
	}
	int GetCurrentParentTotalCalls()
	{
		return CurrentParent->GetTotalCalls();
	}
	long long GetCurrentParentTotalTime()
	{
		return CurrentParent->GetTotalTime();
	}

	PerfNode* Parent()
	{
		return CurrentParent;
	}

protected:
	PerfNode*	CurrentParent;
	PerfNode*	CurrentChild;

	PerfIterator( PerfNode* start );
	friend	class	PerfManager;
};


///////////////////////////////////////////////////////////////////////////////////////////////////
//评估系统管理器
///////////////////////////////////////////////////////////////////////////////////////////////////
class PerfManager
{
private:
	PerfManager();
	~PerfManager();

public:
	static PerfManager* createTheOne();
	static void destroyTheOne();

	static void setTheOne( PerfManager* perfManager );
	static PerfManager* getTheOne();

public:
	inline void StartProfile( const char* name );
	inline void StopProfile();
	inline PerfNode* FindSubNode( PerfNode* pParent, const char* name );
	
	PerfNode* CreateNewNode( const char* name, PerfNode *pParent );

	void	Reset();
	void	IncrementFrameCounter();
	int		GetFrameCountSinceReset()
	{
		return FrameCounter;
	}
	long long	GetTimeSinceReset();

	PerfIterator* GetIterator()
	{
		return new PerfIterator( m_pRoot );
	}
	void ReleaseIterator( PerfIterator * iterator )
	{
		delete iterator;
	}
	
	long long GetRootTotalTime();


	//////////////////////////////////////////////////////////////////////////////
	//以下为存盘/读盘相关接口
	//////////////////////////////////////////////////////////////////////////////
	bool OpenDataFile( const char* pPath, int* pTotalDataFrame, int* pUserDataSize );
	bool CreateFileHeader( const char* pPath, int iUserDataSize, const char** pUserDataName );
	bool RecreateDataFile( const char* pPath, int iTotalNodeOld,
							int iUserDataSize,	const char** pUserDataName );
	void ExportNodeHeader( PerfNode *pNode, char* pHeader );
	void ExportNodeData( PerfNode *pNode, char* pData );

	bool SaveDataFrame( const char* pPath, int* pUserData = 0, const char** pUserDataName = 0, int iUserDataSize = 0 );
	bool LoadDataFrame( const char* pPath, int iFrameIndex, int* pUserData, __time64_t *pTime );
	
	bool GetUserDataName( const char* pPath, char* pName );

	bool GetNodeTimeArr( const char* pPath, int iNodeIndex,
						long long* pTime, long long* pTimePerFr,
						long long& iMaxTime, long long& iMinTime,
						int& iMaxIndex, int& iMinIndex, long long& iMaxTimePerFr );

	bool GetUserDataArr( const char* pPath, int iIndex, int* pData,
								int& iMaxValue, int& iMinValue,
								int& iMaxIndex, int& iMinIndex );

	bool GetTotalFrame( const char* pPath, int& iTotalFrame, 
							int iFrameBegin, int iFrameEnd );
	bool GetNodeTotalData( const char* pPath, int iNodeIndex, long long& i64TotalTime, int& iTotalCall,
							int iFrameBegin, int iFrameEnd );
	bool GetTotalUserDataArr( const char* pPath, long long* pTotalUserData,
							int iFrameBegin, int iFrameEnd );

	PerfNode* GetNode( int iIndex );

private:
	void ReleaseMemory();
	bool VerifyFileHeader( PERF_FILE_HEADER* pHeader );

	bool HandleOldFile( const char* pPath, int iUserDataSize, const char** pUserDataName );
	
	int sizeOfHeader() const;
	bool readHeader(FILE* fp, PERF_FILE_HEADER& h);

	int sizeOfNodeHeader() const;
	bool readNodeHeader(FILE* fp, PERF_NODE_HEADER& h);

	int sizeOfNodeData() const;
	bool readNodeData(FILE* fp, PERF_NODE_DATA& d);

private:
	int			m_iTotalNode;	//总节点数
	PerfNode*	m_pRoot;
	PerfNode*	m_pCurrentNode;
	int			FrameCounter;
	long long		ResetTime;

	static PerfManager* theOne;

	//以下只有PerfSpy才用
	PerfNode**			m_pNodeArr;
	PERF_NODE_HEADER*	m_pNodeData;
};

inline PerfManager* PerfManager::createTheOne()
{
	if ( theOne == NULL )
	{
		theOne = new PerfManager;
	}
	return theOne;
}

inline void PerfManager::destroyTheOne()
{
	if ( theOne != NULL )
	{
		delete theOne;
		theOne = NULL;
	}
}

inline void PerfManager::setTheOne( PerfManager* perfManager )
{
	theOne = perfManager;
}

inline PerfManager* PerfManager::getTheOne()
{
	return theOne;
}

inline void PerfManager::StartProfile( const char* name )
{
	//使用指针比较取代字符串比较
	if ( name != m_pCurrentNode->GetName() )	//如果相等就是递归
	{	//非递归
		m_pCurrentNode = FindSubNode( m_pCurrentNode, name );
	}
	m_pCurrentNode->Call();
}

inline void PerfManager::StopProfile()
{
	//如果没有返回true, 则说明采样的函数块被重入(递归调用)了
	if ( m_pCurrentNode->Return() )
	{
		m_pCurrentNode = m_pCurrentNode->GetParent();
	}
}

inline PerfNode* PerfManager::FindSubNode( PerfNode* pParent, const char* name )
{
	// Try to find this sub node
	PerfNode * child = pParent->GetChild();
	while ( child )
	{
		if ( child->GetName() == name )
		{
			return child;
		}
		child = child->GetSibling();
	}
	// We didn't find it, so add it
	return CreateNewNode( name, pParent );
}

/*
** ProfileSampleClass is a simple way to profile a function's scope
** Use the PROFILE macro at the start of scope to time
*/
class CPerfSample
{
public:
	inline CPerfSample( const char* name )
	{ 
		PerfManager::getTheOne()->StartProfile( name ); 
	}
	
	inline ~CPerfSample()					
	{ 
		PerfManager::getTheOne()->StopProfile(); 
	}
};

#else	//#ifndef _WIN32
	#ifdef _PERFORMANCE
		#undef _PERFORMANCE
	#endif
#endif	//#ifndef _WIN32

#ifdef _PERFORMANCE
	#define	PERF_NODE( name )		CPerfSample __perfSample( name );
	#define PERF_NODE_FUNC()		CPerfSample __perfSample( __FUNCTION__ );
#else
	#define	PERF_NODE( name )	
	#define PERF_NODE_FUNC()	
#endif

#endif
