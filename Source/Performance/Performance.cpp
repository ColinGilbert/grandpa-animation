#ifdef _WIN32

#include "windows.h"
#include "Performance.h"
#include "assert.h"
#include "limits.h"

static long long g_i64CpuFreq = 2000000000;	//Ĭ��CPUָ��Ƶ��
static const int CALC_FREQ_TIME = 500;		//����cpuƵ��ʱ��

unsigned int g_dwFileFlag = 'PERF';
unsigned int g_dwFileVersion = '1100';

PerfManager* PerfManager::theOne = NULL;

///////////////////////////////////////////////////////////////////////////////////////////////////
//pfl�ļ�ͷ
struct PERF_FILE_HEADER
{
	unsigned int	dwFlag;			//�ļ���ʶ
	unsigned int	dwVersion;		//�汾
	long long		i64CPUFreq;		//cpu��Ƶ
	int				iNumNode;		//�����ڵ�����
	int				iUserDataSize;	//�û���������
	unsigned int	dwReserve1;		//����
	unsigned int	dwReserve2;
	unsigned int	dwReserve3;
	unsigned int	dwReserve4;
};
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
//pfl�ļ������ڵ�ṹ
struct PERF_NODE_HEADER
{
	char	szName[MAX_NODE_NAME_LEN];	//�ڵ�����
	int		iParent;				//���ڵ����
	int		iSibling;				//��һ���ֵܽڵ����
	int		iChild;					//�ӽڵ����
};
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
//pfl�ļ������ڵ�����
struct PERF_NODE_DATA
{
	int			iTotalCalls;	//�ܵ��ô���
	long long	iTotalTime;		//�ܿ���ʱ��(cpu������)
};
///////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////
//���㱾��CPUָ��Ƶ��
///////////////////////////////////////////////////////////////////////////////////////////////////
void PerfCountTickRate()
{
	HKEY Key;
	DWORD Speed = 0;
	DWORD Size = sizeof(Speed);

	long Error = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
							   "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
							   0, KEY_READ, &Key );
	if ( Error != ERROR_SUCCESS )
	{
		return;
	}
	Error = RegQueryValueEx(Key, "~MHz", NULL, NULL, (LPBYTE)&Speed, &Size);

	RegCloseKey(Key);

	if ( Error != ERROR_SUCCESS )
	{
		return;
	}

	g_i64CpuFreq = Speed * 1000000;

	//int		iTimeBefore, iTimeAfter, iTimeUsed;
	//long long	iTickBefore, iTickAfter, iTickUsed;

	//iTimeBefore = GetTickCount();
	//PerfGetTicks( &iTickBefore );

	//Sleep( CALC_FREQ_TIME );

	//iTimeAfter = GetTickCount();
	//PerfGetTicks( &iTickAfter );

	//iTimeUsed = iTimeAfter - iTimeBefore;
	//iTickUsed = iTickAfter - iTickBefore;

	//g_i64CpuFreq = iTickUsed * 1000 / iTimeUsed;

	//return g_i64CpuFreq;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//��ȡCPUָ��Ƶ��
///////////////////////////////////////////////////////////////////////////////////////////////////
long long PerfGetTickRate()
{
	return g_i64CpuFreq;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// CPerfNode
///////////////////////////////////////////////////////////////////////////////////////////////////

/***********************************************************************************************
 * INPUT:                                                                                      *
 * name - pointer to a static string which is the name of this profile node                    *
 * parent - parent pointer                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 * The name is assumed to be a static pointer, only the pointer is stored and compared for     *
 * efficiency reasons.                                                                         *
 *=============================================================================================*/
//iParent	���ڵ����
PerfNode::PerfNode( const char* name, PerfNode* parent ) :
	Name( name ),
	TotalCalls( 0 ),
	TotalTime( 0 ),
	StartTime( 0 ),
	RecursionCounter( 0 ),
	Parent( parent ),
	Child( NULL ),
	Sibling( NULL )
{
	Reset();
}

PerfNode::PerfNode( const char* name ) :
	Name( name ),
	TotalCalls( 0 ),
	TotalTime( 0 ),
	StartTime( 0 ),
	RecursionCounter( 0 ),
	Parent( NULL ),
	Child( NULL ),
	Sibling( NULL )
{
	Reset();
}

PerfNode::~PerfNode()
{
	delete Child;
	delete Sibling;
}

void PerfNode::Reset()
{
	TotalCalls = 0;
	TotalTime = 0;

	if ( Child )
	{
		Child->Reset();
	}
	if ( Sibling )
	{
		Sibling->Reset();
	}
}

/***************************************************************************************************
**
** PerfIterator
**
***************************************************************************************************/
PerfIterator::PerfIterator( PerfNode* start )
{
	CurrentParent = start;
	CurrentChild = CurrentParent->GetChild();
}

PerfNode* PerfIterator::First()
{
	return ( CurrentChild = CurrentParent->GetChild() );
}

PerfNode* PerfIterator::Next()
{
	return ( CurrentChild = CurrentChild->GetSibling() );
}

bool PerfIterator::IsDone()
{
	return CurrentChild == NULL;
}

void PerfIterator::EnterChild( int index )
{
	CurrentChild = CurrentParent->GetChild();
	while ( CurrentChild != NULL && index-- != 0 )
	{
		CurrentChild = CurrentChild->GetSibling();
	}
	if ( CurrentChild != NULL )
	{
		CurrentParent = CurrentChild;
		CurrentChild = CurrentParent->GetChild();
	}
}

void PerfIterator::EnterParent()
{
	if ( CurrentParent->GetParent() != NULL )
	{
		CurrentParent = CurrentParent->GetParent();
	}
	CurrentChild = CurrentParent->GetChild();
}

int PerfIterator::GetTotalChild()
{
	int iNum = 0;
	PerfNode *pChild = CurrentParent->GetChild();
	while ( pChild != NULL )
	{
		iNum++;
		pChild = pChild->GetSibling();
	}
	return iNum;
}

/***************************************************************************************************
**
** CPerfManager
**
***************************************************************************************************/

PerfManager::PerfManager()
{
	PerfCountTickRate();

	m_pRoot = new PerfNode( "Root", NULL );
	m_pRoot->iIndex = 0;
	m_iTotalNode = 1;
	m_pCurrentNode = m_pRoot;
	m_pNodeArr = NULL;	//�ڵ�ָ������, ���ļ������������ڵ�ʱʹ��
	m_pNodeData = NULL;	//��Žڵ����ֵĻ�����ָ��
	FrameCounter = 0;
	ResetTime = 0;
	m_pRoot->Reset();
}

PerfManager::~PerfManager()
{
	ReleaseMemory();
}

PerfNode* PerfManager::CreateNewNode( const char* name, PerfNode *pParent )
{
	PerfNode *pNode = new PerfNode( name, pParent );
	pNode->iIndex = m_iTotalNode++;
	pNode->Sibling = pParent->GetChild();
	pParent->Child = pNode;
	return pNode;
}

/***********************************************************************************************
 * PerfManager::Reset -- Reset the contents of the profiling system						   *
 *                                                                                             *
 *    This resets everything except for the tree structure.  All of the timing data is reset.  *
 *=============================================================================================*/
void PerfManager::Reset()
{ 
	m_pRoot->Reset(); 
	FrameCounter = 0;
	PerfGetTicks( &ResetTime );
}

/***********************************************************************************************
 * PerfManager::Increment_Frame_Counter -- Increment the frame counter						   *
 *=============================================================================================*/
void PerfManager::IncrementFrameCounter()
{
	FrameCounter++;
}

/***********************************************************************************************
 * PerfManager::GetTime_Since_Reset -- returns the elapsed time since last reset			   *
 *=============================================================================================*/
long long PerfManager::GetTimeSinceReset()
{
	long long time;
	PerfGetTicks( &time );
	time -= ResetTime;
	return time;
}

///////////////////////////////////////////////////////////////////////////////
//���������ļ�ͷ
//pPath			�����ļ�·��
//iUserDataSize	�û�ͳ�����ݵĴ�С(int����)
//pUserDataName	�û�ͳ�����ݵ���������, ���iUserDataSize��Ϊ0��˲�������ΪNULL
//				�û�����������С��64���ַ�
//return		�ɹ�����true, ʧ�ܷ��� false
///////////////////////////////////////////////////////////////////////////////
bool PerfManager::CreateFileHeader( const char* pPath, int iUserDataSize, const char** pUserDataName )
{
	if ( NULL == pPath )
	{
		return false;
	}
	if ( iUserDataSize > 0 && NULL == pUserDataName )
	{
		return false;
	}
	FILE *fp = fopen( pPath, "wb" );
	if ( NULL == fp )
	{
		return false;
	}
	
	PERF_FILE_HEADER fh;

	fh.dwFlag = g_dwFileFlag;
	fh.dwVersion = g_dwFileVersion;
	fh.i64CPUFreq = g_i64CpuFreq;
	fh.iNumNode = m_iTotalNode;
	fh.iUserDataSize = iUserDataSize;
	fh.dwReserve1 = 0;
	fh.dwReserve2 = 0;
	fh.dwReserve3 = 0;
	fh.dwReserve4 = 0;

#define FWRITE_OR_RETURN(data)	if (fwrite(&(data), sizeof(data), 1, fp) != 1)\
								{\
									fclose( fp );\
									return false;\
								}
	FWRITE_OR_RETURN(fh.dwFlag);
	FWRITE_OR_RETURN(fh.dwVersion);
	FWRITE_OR_RETURN(fh.i64CPUFreq);
	FWRITE_OR_RETURN(fh.iNumNode);
	FWRITE_OR_RETURN(fh.iUserDataSize);
	FWRITE_OR_RETURN(fh.dwReserve1);
	FWRITE_OR_RETURN(fh.dwReserve2);
	FWRITE_OR_RETURN(fh.dwReserve3);
	FWRITE_OR_RETURN(fh.dwReserve4);
	
	int iBufferSize = sizeOfNodeHeader() * m_iTotalNode
					+ MAX_NODE_NAME_LEN * iUserDataSize;
	//�����нڵ�����д��
	char *pBuffer = new char[iBufferSize];
	memset( pBuffer, 0, iBufferSize );
	ExportNodeHeader( m_pRoot, pBuffer );
	//�û�������Ҳд���ļ�ͷ
	char *pTemp = pBuffer + sizeOfNodeHeader() * m_iTotalNode;
	for ( int i = 0; i < iUserDataSize; i++ )
	{
		if ( pUserDataName[i] != NULL )
		{
			assert( strlen( pUserDataName[i] ) < MAX_NODE_NAME_LEN );
			strcpy( pTemp, pUserDataName[i] );
		}
		pTemp += MAX_NODE_NAME_LEN;
	}

	bool ret = true;
	if ( fwrite( pBuffer, iBufferSize, 1, fp ) != 1 )
	{
		ret = false;
	}
	delete[] pBuffer;
	fclose( fp );
	return ret;
}

///////////////////////////////////////////////////////////////////////////////
//�����µĲ����ڵ㱻����ʱ�����ڽڵ����仯�������ؽ�pfl�ļ�
bool PerfManager::RecreateDataFile( const char* pPath,
									 int iTotalNodeOld,
									 int iUserDataSize,
									 const char** pUserDataName
									)
{
	assert( m_iTotalNode > iTotalNodeOld );
	//����һ��
	char szBakFilename[MAX_PATH];
	sprintf( szBakFilename, "%s.bak.pfl", pPath );
	CopyFile( pPath, szBakFilename, false );	//����ļ������򸲸�
	//ɾ��
	DeleteFile( pPath );

	//�������ļ�ͷ
	if ( !CreateFileHeader( pPath, iUserDataSize, pUserDataName ) )
	{
		return false;
	}
	FILE *fpOld = fopen( szBakFilename, "rb" );
	if ( NULL == fpOld )
	{
		return false;
	}
	FILE *fpNew = fopen( pPath, "ab" );
	if ( NULL == fpNew )
	{
		fclose( fpOld );
		return false;
	}
	//����ԭ�ļ�������
	//֡�ߴ�
	int iFrameSizeOld	= sizeof( int ) * iUserDataSize		//�û�����
						+ sizeof( int )						//����֡��
						+ sizeof( __time64_t )					//ʱ�䣨1970������������
						+ sizeOfNodeData() * iTotalNodeOld;	//��������
	int iFrameSizeNew	= sizeof( int ) * iUserDataSize		//�û�����
						+ sizeof( int )						//����֡��
						+ sizeof( __time64_t )					//ʱ�䣨1970������������
						+ sizeOfNodeData() * m_iTotalNode;	//��������
	assert( iFrameSizeNew > iFrameSizeOld );
	//��һ֡����ƫ��
	int iPosOld	= sizeOfHeader()		//�ļ�ͷ
				+ sizeOfNodeHeader() * iTotalNodeOld	//�����ڵ�ͷ
				+ MAX_NODE_NAME_LEN * iUserDataSize;		//�û�������
	int iPosNew	= sizeOfHeader()		//�ļ�ͷ
				+ sizeOfNodeHeader() * m_iTotalNode	//�����ڵ�ͷ
				+ MAX_NODE_NAME_LEN * iUserDataSize;		//�û�������
	
	BYTE *pBuffer = new BYTE[iFrameSizeNew];
	memset( pBuffer, 0, iFrameSizeNew );
	while ( fseek( fpOld, iPosOld, SEEK_SET ) == 0 )
	{
		if ( fseek( fpNew, iPosNew, SEEK_SET ) != 0 )
		{
			break;
		}
		//�����360ƽ̨��ԭ��д��ʱ���ֽ����Ѿ���little-endian�ˣ���������ʲôҲ���ø�
		if ( fread( pBuffer, iFrameSizeOld, 1, fpOld ) != 1 )
		{
			break;
		}
		if ( fwrite( pBuffer, iFrameSizeNew, 1, fpNew ) != 1 )
		{
			break;
		}
		iPosOld += iFrameSizeOld;
		iPosNew += iFrameSizeNew;
	}
	delete[] pBuffer;
	fclose( fpOld );
	fclose( fpNew );

	DeleteFile( szBakFilename );
	return true;
}

///////////////////////////////////////////////////////////////////////////////
//�����ڵ�ṹ����
//pNode		�����ڵ�
//pHeader	����������
///////////////////////////////////////////////////////////////////////////////
void PerfManager::ExportNodeHeader( PerfNode *pNode, char* pHeader )
{
	assert( NULL != pNode );
	assert( NULL != pHeader );

	int index = pNode->GetIndex();
	char* dest = pHeader + index * sizeOfNodeHeader();
	strcpy(dest, pNode->GetName());
	dest += MAX_NODE_NAME_LEN;
	if ( pNode->GetParent() != NULL )
	{
		*((int*)dest) = pNode->GetParent()->GetIndex();
	}
	else
	{
		//�ٺ�-1����swap
		*((int*)dest) = -1;
	}
	dest += sizeof(int);
	if ( pNode->GetSibling() != NULL )
	{
		*((int*)dest) = pNode->GetSibling()->GetIndex();
		ExportNodeHeader( pNode->GetSibling(), pHeader );
	}
	else
	{
		*((int*)dest) = -1;
	}
	dest += sizeof(int);
	if ( pNode->GetChild() != NULL )
	{
		*((int*)dest) = pNode->GetChild()->GetIndex();
		ExportNodeHeader( pNode->GetChild(), pHeader );
	}
	else
	{
		*((int*)dest) = -1;
	}
}

///////////////////////////////////////////////////////////////////////////////
//�����ڵ����ݵ���
//pNode		�����ڵ�
//pData		����������
///////////////////////////////////////////////////////////////////////////////
void PerfManager::ExportNodeData( PerfNode *pNode, char* pData )
{
	assert( pNode != NULL );
	assert( pData != NULL );

	int iIndex = pNode->GetIndex();
	char* dest = pData + iIndex * sizeOfNodeData();
	*((int*)dest) = pNode->GetTotalCalls();
	dest += sizeof(int);
	*((long long*)dest) = pNode->GetTotalTime();
	if ( pNode->GetChild() != NULL )
	{
		ExportNodeData( pNode->GetChild(), pData );
	}
	if ( pNode->GetSibling() != NULL )
	{
		ExportNodeData( pNode->GetSibling(), pData );
	}
}

///////////////////////////////////////////////////////////////////////////////
//����ǰ���в����ڵ������д���ļ�
//pPath			�����ļ�·��
//pUserData		��Ҫһͬд���ļ����û�ͳ����������ָ��, ���Ϊ�������û���û�����
//iSize			pUserDataָ���int�����е����ݸ���(����ͬһ���ļ�,ÿ�ε��ñ�����ͬ)
//pUserDataName	�û�ͳ�����ݵ���������(����ͬһ���ļ�,ÿ�ε��ñ�����ͬ)
//				���pUserData��ΪNULL, ���������Ҳ����ΪNULL
//return		�ɹ�����true, ʧ�ܷ��� false

//�ļ��ṹ
//	int			�����ڵ���
//	int			�û����ݸ���
//	PERF_NODE_HEADER		�����ڵ�1	����ÿ����������֡����ͬ���Ĳ����ڵ�
//										�������ǵ��໥���ù�ϵҲ�ǹ̶���
//										���԰Ѳ����ڵ�����ֺ��໥��ϵ������ȡ����
//										�����ļ�ͷ��
//	PERF_NODE_HEADER		�����ڵ�2
//	...			
//	char[64]	�û�������1
//	char[64]	�û�������2
//	...
//	frame1		��������֡1
//	frame2		��������֡2
//	...

//��������֡�ṹ
//	userdata	�û�����
//	int			����֡��(��ָ��Ϸ�е�֡, ����ǰ��˵������֡)
//	PERF_NODE_DATA	�ڵ�����1
//	PERF_NODE_DATA	�ڵ�����2
//	...

//ע��:		�����ǰ�����ڵ������ļ��м��ص�Ҫ��, ���ļ��ᱻ�ؽ�, ֮ǰ�Ĳ�������ȫ����ʧ
//			��Ϊ���в����ڵ�������Ǽ�¼���ļ�ͷ�е�, ����ڵ�����һ��
//			��ڵ����ͽڵ����ݾ��޷���Ӧ��, ֻ�����´����ļ�

///////////////////////////////////////////////////////////////////////////////
bool PerfManager::SaveDataFrame( const char* pPath, int* pUserData, const char** pUserDataName, int iUserDataSize )
{
	if ( NULL == pPath )
	{
		return false;
	}

	if ( !HandleOldFile( pPath, iUserDataSize, pUserDataName ) )
	{
		return false;
	}

	FILE *fp = fopen( pPath, "ab" );
	if ( NULL == fp )
	{
		return false;
	}
	if ( NULL == pUserData )
	{
		iUserDataSize = 0;
	}

	int iBufferSize = sizeof( int ) * iUserDataSize	//�û�����
					+ sizeof( int )			//����֡��
					+ sizeof( __time64_t )		//ʱ�䣨1970������������
					+ sizeOfNodeData() * m_iTotalNode;	//�ڵ�����
	char *pBuffer = new char[iBufferSize];
	char *pCur = pBuffer;
	//�û�����
	if ( pUserData != NULL )
	{
		memcpy( pCur, pUserData, sizeof( int ) * iUserDataSize );
	}
	pCur += ( sizeof( int ) * iUserDataSize );
	//����֡��
	*((int*)pCur) = GetFrameCountSinceReset();
	pCur += sizeof( int );

	__time64_t Time;
	_time64( &Time );

	*((__time64_t*)pCur) = Time;

	pCur += sizeof( __time64_t );

	ExportNodeData( m_pRoot, pCur );

	//һ���Խ�����bufferд���ļ�
	bool ret = true;
	if ( fwrite( pBuffer, iBufferSize, 1, fp ) != 1 )
	{
		ret = false;
	}
	delete[] pBuffer;

	fclose( fp );
	return ret;
}

///////////////////////////////////////////////////////////////////////////////
//���º���ΪPerfSpyר��
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//��pfl�����ļ�
//pPath				�ļ�·��
//pTotalDataFrame	��������֡����
//pUserDataSize		�����û����ݸ���
//return			�ɹ�����true, ʧ�ܷ���false 
///////////////////////////////////////////////////////////////////////////////
bool PerfManager::OpenDataFile( const char* pPath, int* pTotalDataFrame, int* pUserDataSize )
{
	if ( NULL == pPath )
	{
		return false;
	}
	FILE *fp = fopen( pPath, "rb" );
	if ( NULL == fp )
	{
		return false;
	}
	PERF_FILE_HEADER fh;
	if (!readHeader(fp, fh))
	{
		fclose( fp );
		return false;
	}
	if ( !VerifyFileHeader( &fh ) )
	{
		return false;
	}

	if ( pUserDataSize != NULL )
	{
		*pUserDataSize = fh.iUserDataSize;
	}
	g_i64CpuFreq = fh.i64CPUFreq;

	//����Ϊ��ȡ�����ڵ�����ֲ���֯��״�ṹ
	if ( m_pNodeData != NULL )
	{
		delete[] m_pNodeData;
	}
	//����ڵ�����ֺ��໥��ϵ�Ļ�����
	m_pNodeData = new PERF_NODE_HEADER[fh.iNumNode];

	for (int i = 0; i < fh.iNumNode; ++i)
	{
		if (!readNodeHeader(fp, m_pNodeData[i]))
		{
			delete[] m_pNodeData;
			m_pNodeData = NULL;
			fclose( fp );
			return false;
		}
	}

	if ( m_pNodeArr != NULL )
	{
		delete[] m_pNodeArr;
	}
	m_pNodeArr = new PerfNode*[fh.iNumNode];
	memset( m_pNodeArr, 0, fh.iNumNode * sizeof( PerfNode* ) );
	//ɾ���ɵĽڵ�
	if ( m_pRoot->GetChild() )
	{
		//m_pRoot�������ֵ�, ֻɾ�����Ӿ�����
		delete m_pRoot->GetChild();
		m_pRoot->Child = NULL;
	}
	m_iTotalNode = 1;	//��ʣһ��m_pRoot, ������1
	for ( int i = 1; i < fh.iNumNode; i++ )	//��0���ڵ���m_pRoot, �����ٴ���
	{
		m_pNodeArr[i] = new PerfNode( m_pNodeData[i].szName );
		m_pNodeArr[i]->iIndex = i;
	}
	m_iTotalNode = fh.iNumNode;
	//��֯��ϵ
	for ( int i = 1; i < fh.iNumNode; i++ )
	{
		if ( m_pNodeData[i].iChild >= 0 )
		{
			m_pNodeArr[i]->Child = m_pNodeArr[m_pNodeData[i].iChild];
		}
		if ( m_pNodeData[i].iSibling >= 0 )
		{
			m_pNodeArr[i]->Sibling = m_pNodeArr[m_pNodeData[i].iSibling];
		}
		if ( 0 == m_pNodeData[i].iParent )
		{
			//�����Ǹ��ڵ�
			m_pNodeArr[i]->Parent = m_pRoot;
		}
		else
		{
			//���ײ��Ǹ��ڵ�
			m_pNodeArr[i]->Parent = m_pNodeArr[m_pNodeData[i].iParent];
		}
	}
	if ( fh.iNumNode > 1 )
	{
		m_pRoot->Child = m_pNodeArr[m_pNodeData[0].iChild];
	}

	//һ��ͳ������֡��
	int iFrameSize	= sizeof( int ) * fh.iUserDataSize	//�û�����
					+ sizeof( int )		//����֡��
					+ sizeof( __time64_t )	//ʱ�䣨1970������������
					+ sizeOfNodeData() * fh.iNumNode;	//��������

	int iHeaderSize = sizeOfHeader()		//�ļ�ͷ
					+ sizeOfNodeHeader() * fh.iNumNode	//�����ڵ�ͷ
					+ MAX_NODE_NAME_LEN * fh.iUserDataSize;	//�û�������
	//�ļ���С
	if ( fseek( fp, 0, SEEK_END ) != 0 )
	{
		fclose( fp );
		return false;
	}
	fpos_t pos;
	if ( fgetpos( fp, &pos ) != 0 )
	{
		fclose( fp );
		return false;
	}

	if ( pTotalDataFrame != NULL )
	{
		*pTotalDataFrame = (int)( pos - iHeaderSize ) / iFrameSize;
	}

	fclose( fp );

	return true;
}

///////////////////////////////////////////////////////////////////////////////
//��ָ���ļ��ж���һ����������֡
//pPath			�����ļ�·��
//iFrameIndex	���ļ��еĵڼ�������֡
//pUserData		���������û����ݵĻ�����, �û����������㹻�����������������
//				ΪNULL�򲻽����û�����
//pTime			���������֡ʱ��ʱ��
///////////////////////////////////////////////////////////////////////////////
bool PerfManager::LoadDataFrame( const char* pPath, int iFrameIndex, int* pUserData, __time64_t *pTime )
{
	if ( NULL == pPath )
	{
		return false;
	}
	if ( NULL == m_pNodeArr || NULL == m_pNodeData )
	{
		assert( false );
		return false;
	}
	FILE *fp = fopen( pPath, "rb" );
	if ( NULL == fp )
	{
		return false;
	}
	PERF_FILE_HEADER fh;
	if (!readHeader(fp, fh))
	{
		fclose( fp );
		return false;
	}
	if ( fh.iNumNode != m_iTotalNode )
	{
		fclose( fp );
		return false;
	}
	//��������֡��С
	int iFrameSize	= sizeof( int ) * fh.iUserDataSize	//�û�����
					+ sizeof( int )						//����֡��
					+ sizeof( __time64_t )					//ʱ�䣨1970������������
					+ sizeOfNodeData() * fh.iNumNode;	//��������
	//�ļ�ƫ��
	int iOffset = sizeOfHeader()		//�ļ�ͷ
				+ sizeOfNodeHeader() * fh.iNumNode	//�����ڵ�ͷ
				+ MAX_NODE_NAME_LEN * fh.iUserDataSize	//�û�������
				+ iFrameSize * iFrameIndex;

	if ( fseek( fp, iOffset, SEEK_SET ) != 0 )
	{
		fclose( fp );
		return false;
	}
	if ( pUserData != NULL )
	{
		if ( fread( pUserData, sizeof( int ) * fh.iUserDataSize, 1, fp ) != 1 )
		{
			fclose( fp );
			return false;
		}
	}
	else
	{
		if ( fseek( fp, sizeof( int ) * fh.iUserDataSize, SEEK_CUR ) != 0 )
		{
			fclose( fp );
			return false;
		}
	}
	//����֡��
	int iCount;
	if ( fread( &iCount, sizeof( int ), 1, fp ) != 1 )
	{
		fclose( fp );
		return false;
	}
	FrameCounter = iCount;

	//ʱ��
	__time64_t time;
	if ( fread( &time, sizeof( __time64_t ), 1, fp ) != 1 )
	{
		fclose( fp );
		return false;
	}
	if ( pTime != NULL )
	{
		*pTime = time;
	}

	//�����ڵ�����
	for ( int i = 0; i < fh.iNumNode; i++ )
	{
		PERF_NODE_DATA temp;
		if (!readNodeData(fp, temp))
		{
			fclose( fp );
			return false;
		}
		if (i > 0)
		{
			m_pNodeArr[i]->TotalCalls = temp.iTotalCalls;
			m_pNodeArr[i]->TotalTime = temp.iTotalTime;
		}
	}
	fclose( fp );
	return true;
}

///////////////////////////////////////////////////////////////////////////////
//��ȡָ���ļ��е��û�������
//pPath				�ļ�·��
//pName				�û������û����Ļ�����
//					�û������ߴ������� �û����ݸ��� x MAX_NODE_NAME_LEN
//return			�ɹ�����true, ʧ�ܷ���false 
///////////////////////////////////////////////////////////////////////////////
bool PerfManager::GetUserDataName( const char* pPath, char* pName )
{
	if ( NULL == pPath || NULL == pName )
	{
		return false;
	}
	FILE *fp = fopen( pPath, "rb" );
	if ( NULL == fp )
	{
		return false;
	}
	PERF_FILE_HEADER fh;
	if (!readHeader(fp, fh))
	{
		fclose( fp );
		return false;
	}
	int iPos = sizeOfHeader()
			 + sizeOfNodeHeader() * fh.iNumNode;
	if ( fseek( fp, iPos, SEEK_SET ) != 0 )
	{
		fclose( fp );
		return false;
	}
	char *pBuffer = new char[MAX_NODE_NAME_LEN * fh.iUserDataSize];
	if ( fread( pBuffer, MAX_NODE_NAME_LEN * fh.iUserDataSize, 1, fp ) != 1 )
	{
		delete[] pBuffer;
		fclose( fp );
		return false;
	}
	char *pSrc = pBuffer,
		 *pDst = pName;
	for ( int i = 0; i < fh.iUserDataSize; i++ )
	{
		assert( strlen( pSrc ) < MAX_NODE_NAME_LEN );
		strcpy( pDst, pSrc );
		pSrc += MAX_NODE_NAME_LEN;
		pDst += MAX_NODE_NAME_LEN;
	}
	delete[] pBuffer;
	fclose( fp );
	return true;
}

///////////////////////////////////////////////////////////////////////////////
//��ָ���ļ��л�ȡָ���ڵ�����������֡�ĺ�ʱ����
//pPath				�ļ�·��
//iNodeIndex		ָ���ڵ����
//pTime				�ܺ�ʱ���黺�������û����������㹻��
//pTimePerFr		ÿ֡��ʱ���黺�������û����������㹻��
//iMaxTime			���غ�ʱ���ֵ
//iMinTime			���غ�ʱ��Сֵ
//iMaxIndex			�������ֵ��Ӧ������֡���
//iMinIndex			������Сֵ��Ӧ������֡���
//iMaxTimePerFr		�������ÿ֡��ʱ
//return			�ɹ�����true, ʧ�ܷ���false
///////////////////////////////////////////////////////////////////////////////
bool PerfManager::GetNodeTimeArr( const char* pPath, int iNodeIndex,
									long long* pTime, long long* pTimePerFr,
									long long& iMaxTime, long long& iMinTime,
									int& iMaxIndex, int& iMinIndex, long long& iMaxTimePerFr )
{
	if ( NULL == pPath || NULL == pTime || NULL == pTimePerFr )
	{
		return false;
	}
	FILE *fp = fopen( pPath, "rb" );
	if ( NULL == fp )
	{
		return false;
	}
	PERF_FILE_HEADER fh;
	if (!readHeader(fp, fh))
	{
		fclose( fp );
		return false;
	}
	//��һ������֡��֡������ƫ��
	int iPosFr	= sizeOfHeader()
				+ sizeOfNodeHeader() * fh.iNumNode
				+ MAX_NODE_NAME_LEN * fh.iUserDataSize
				+ sizeof( int ) * fh.iUserDataSize;	//�û�����
	//��һ������֡��ָ���ڵ������ƫ��
	int iPos = sizeOfHeader()
		     + sizeOfNodeHeader() * fh.iNumNode
		     + MAX_NODE_NAME_LEN * fh.iUserDataSize
		     + sizeof( int ) * fh.iUserDataSize	//�û�����
		     + sizeof( int )					//����֡��
			 + sizeof( __time64_t )					//ʱ�䣨1970������������
		     + sizeOfNodeData() * iNodeIndex
		     + sizeof( int );		//����iTotalCall

	//֡��С
	int iFrameSize = sizeof( int ) * fh.iUserDataSize
				   + sizeof( int )
				   + sizeof( __time64_t )		//ʱ�䣨1970������������
				   + sizeOfNodeData() * fh.iNumNode;
				
	iMaxTime = _I64_MIN;
	iMaxIndex = 0;
	iMinTime = _I64_MAX;
	iMinIndex = 0;
	iMaxTimePerFr = _I64_MIN;
	int iDataFrameCount = 0;
	int iTotalFrame = 0;
	while ( true )
	{
		if ( fseek( fp, iPosFr, SEEK_SET ) != 0 )
		{
			break;
		}
		if ( fread( &iTotalFrame, sizeof( int ), 1, fp ) != 1 )
		{
			break;
		}
		if ( fseek( fp, iPos, SEEK_SET ) != 0 )
		{
			break;
		}
		if ( fread( pTime, sizeof( long long ), 1, fp ) != 1 )
		{
			break;
		}
		if ( *pTime > iMaxTime )
		{
			iMaxTime = *pTime;
			iMaxIndex = iDataFrameCount;
		}
		if ( *pTime < iMinTime )
		{
			iMinTime = *pTime;
			iMinIndex = iDataFrameCount;
		}
		if ( iTotalFrame > 0 )
		{
			*pTimePerFr = *pTime / iTotalFrame;
		}
		else
		{
			*pTimePerFr = 0;
		}
		if ( *pTimePerFr > iMaxTimePerFr )
		{
			iMaxTimePerFr = *pTimePerFr;
		}
		iPos += iFrameSize;
		iPosFr += iFrameSize;
		iDataFrameCount++;
		pTime++;
		pTimePerFr++;
	}
	fclose( fp );
	return true;
}

///////////////////////////////////////////////////////////////////////////////
//��ָ���ļ��л�ȡ��������֡�е�ָ���û�����
//pPath				�ļ�·��
//iIndex			ָ���û��������
//pData				�������ݻ�����, �û����������㹻��
//iMaxValue			�����û��������ֵ
//iMinValue			�����û�������Сֵ
//iMaxIndex			�������ֵ��Ӧ������֡���
//iMinIndex			������Сֵ��Ӧ������֡���
//return			�ɹ�����true, ʧ�ܷ���false
///////////////////////////////////////////////////////////////////////////////
bool PerfManager::GetUserDataArr( const char* pPath, int iIndex, int* pData,
								int& iMaxValue, int& iMinValue,
								int& iMaxIndex, int& iMinIndex )
{
	if ( NULL == pPath || NULL == pData )
	{
		return false;
	}
	FILE *fp = fopen( pPath, "rb" );
	if ( NULL == fp )
	{
		return false;
	}
	PERF_FILE_HEADER fh;
	if (!readHeader(fp, fh))
	{
		fclose( fp );
		return false;
	}
	//��һ������֡��ָ���û����ݵ�����ƫ��
	int iPos = sizeOfHeader()
		     + sizeOfNodeHeader() * fh.iNumNode
		     + MAX_NODE_NAME_LEN * fh.iUserDataSize
		     + sizeof( int ) * iIndex;

	//֡��С
	int iFrameSize = sizeof( int ) * fh.iUserDataSize
				   + sizeof( int )
				   + sizeof( __time64_t )		//ʱ�䣨1970������������
				   + sizeOfNodeData() * fh.iNumNode;

	iMaxValue = INT_MIN;
	iMinValue = INT_MAX;
	iMaxIndex = 0;
	iMinIndex = 0;
	int	iDataFrameCount = 0;
	while ( true )
	{
		if ( fseek( fp, iPos, SEEK_SET ) != 0 )
		{
			break;
		}
		if ( fread( pData, sizeof( int ), 1, fp ) != 1 )
		{
			break;
		}
		if ( *pData > iMaxValue )
		{
			iMaxValue = *pData;
			iMaxIndex = iDataFrameCount;
		}
		if ( *pData < iMinValue )
		{
			iMinValue = *pData;
			iMinIndex = iDataFrameCount;
		}
		iPos += iFrameSize;
		iDataFrameCount++;
		pData++;
	}
	fclose( fp );
	return true;
}

///////////////////////////////////////////////////////////////////////////////
//��ָ���ļ��л�ȡָ����Χ����֡�е�֡���ܺ�
//pPath				�ļ�·��
//iTotalFrame		�����߼�֡���ܺ�
//iFrameBegin		����ͳ�Ƶ�����֡��ŷ�Χ
//iFrameEnd			
//return			�ɹ�����true, ʧ�ܷ���false
///////////////////////////////////////////////////////////////////////////////
bool PerfManager::GetTotalFrame( const char* pPath, int& iTotalFrame,
									int iFrameBegin, int iFrameEnd )
{
	if ( NULL == pPath )
	{
		return false;
	}
	FILE *fp = fopen( pPath, "rb" );
	if ( NULL == fp )
	{
		return false;
	}
	PERF_FILE_HEADER fh;
	if (!readHeader(fp, fh))
	{
		fclose( fp );
		return false;
	}
	//��һ������֡��ָ���ڵ������ƫ��
	int iPos = sizeOfHeader()
		     + sizeOfNodeHeader() * fh.iNumNode
		     + MAX_NODE_NAME_LEN * fh.iUserDataSize
		     + sizeof( int ) * fh.iUserDataSize;//�û�����
	//֡��С
	int iFrameSize = sizeof( int ) * fh.iUserDataSize
				   + sizeof( int )
				   + sizeof( __time64_t )		//ʱ�䣨1970������������
				   + sizeOfNodeData() * fh.iNumNode;

	int iDataFrameIndex = 0;			
	iTotalFrame = 0;
	int iFrame;
	while ( true )
	{
		if ( fseek( fp, iPos, SEEK_SET ) != 0 )
		{
			break;
		}
		if ( iDataFrameIndex >= iFrameEnd )
		{
			break;
		}
		//ֻͳ��iFrameBegin��iFrameEnd֮�������
		if ( iDataFrameIndex >= iFrameBegin )
		{
			if ( fread( &iFrame, sizeof( int ), 1, fp ) != 1 )
			{
				break;
			}
			iTotalFrame += iFrame;
		}
		iPos += iFrameSize;
		iDataFrameIndex++;
	}
	fclose( fp );
	return true;
}

///////////////////////////////////////////////////////////////////////////////
//��ָ���ļ��л�ȡ��������֡�е������ܺ�
//pPath				�ļ�·��
//iIndex			ָ���ڵ����
//i64TotalTime		ָ���ڵ�����������֡�ĺ�ʱ�ܺ�
//iTotalCall		ָ���ڵ�����������֡�ĵ��ô����ܺ�
//iFrameBegin		����ͳ�Ƶ�����֡��ŷ�Χ
//iFrameEnd			
//return			�ɹ�����true, ʧ�ܷ���false
///////////////////////////////////////////////////////////////////////////////
bool PerfManager::GetNodeTotalData( const char* pPath, int iNodeIndex,
									 long long& i64TotalTime, int& iTotalCall,
									 int iFrameBegin, int iFrameEnd )
{
	if ( NULL == pPath )
	{
		return false;
	}
	FILE *fp = fopen( pPath, "rb" );
	if ( NULL == fp )
	{
		return false;
	}
	PERF_FILE_HEADER fh;
	if (!readHeader(fp, fh))
	{
		fclose( fp );
		return false;
	}
	//��һ������֡��ָ���ڵ������ƫ��
	int iPos = sizeOfHeader()
		     + sizeOfNodeHeader() * fh.iNumNode
		     + MAX_NODE_NAME_LEN * fh.iUserDataSize
		     + sizeof( int ) * fh.iUserDataSize	//�û�����
		     + sizeof( int )					//����֡��
			 + sizeof( __time64_t )					//ʱ�䣨1970������������
		     + sizeOfNodeData() * iNodeIndex;

	//֡��С
	int iFrameSize = sizeof( int ) * fh.iUserDataSize
				   + sizeof( int )
				   + sizeof( __time64_t )		//ʱ�䣨1970������������
				   + sizeOfNodeData() * fh.iNumNode;
				
	i64TotalTime = 0;
	iTotalCall = 0;
	PERF_NODE_DATA data;
	int iFrame = 0;
	while ( true )
	{
		if ( fseek( fp, iPos, SEEK_SET ) != 0 )
		{
			break;
		}
		if ( iFrame >= iFrameEnd )
		{
			break;
		}
		//ֻͳ��iFrameBegin��iFrameEnd֮�������
		if ( iFrame >= iFrameBegin )
		{
			if (!readNodeData(fp, data))
			{
				break;
			}
			i64TotalTime += data.iTotalTime;
			iTotalCall	 += data.iTotalCalls;
		}
		iPos += iFrameSize;
		iFrame++;
	}
	fclose( fp );
	return true;
}

///////////////////////////////////////////////////////////////////////////////
//��ָ���ļ��л�ȡ�����û����ݵ�����������֡�е��ܺ�
//pPath				�ļ�·��
//pTotalUserData	���������û������ܺ͵Ļ�����, �û����������㹻��
//iFrameBegin		����ͳ�Ƶ�����֡��ŷ�Χ
//iFrameEnd			
//return			�ɹ�����true, ʧ�ܷ���false
///////////////////////////////////////////////////////////////////////////////
bool PerfManager::GetTotalUserDataArr( const char* pPath, long long* pTotalUserData,
										  int iFrameBegin, int iFrameEnd )
{
	if ( NULL == pPath || NULL == pTotalUserData )
	{
		return false;
	}
	FILE *fp = fopen( pPath, "rb" );
	if ( NULL == fp )
	{
		return false;
	}
	PERF_FILE_HEADER fh;
	if (!readHeader(fp, fh))
	{
		fclose( fp );
		return false;
	}
	//��һ������֡�ĵ�һ���û����ݵ�ƫ��
	int iPos = sizeOfHeader()
		     + sizeOfNodeHeader() * fh.iNumNode
		     + MAX_NODE_NAME_LEN * fh.iUserDataSize;
	//֡��С
	int iFrameSize = sizeof( int ) * fh.iUserDataSize
				   + sizeof( int )
				   + sizeof( __time64_t )		//ʱ�䣨1970������������
				   + sizeOfNodeData() * fh.iNumNode;

	//��0
	memset( pTotalUserData, 0, sizeof( long long ) * fh.iUserDataSize );

	int *pUserData = new int[fh.iUserDataSize];
	int iFrame = 0;
	while ( true )
	{
		if ( fseek( fp, iPos, SEEK_SET ) != 0 )
		{
			break;
		}
		if ( iFrame >= iFrameEnd )
		{
			break;
		}
		//ֻͳ��iFrameBegin��iFrameEnd֮�������
		if ( iFrame >= iFrameBegin )
		{
			if ( fread( pUserData, sizeof( int ) * fh.iUserDataSize, 1, fp ) != 1 )
			{
				break;
			}
			for ( int i = 0; i < fh.iUserDataSize; i++ )
			{
				pTotalUserData[i] += pUserData[i];
			}
		}
		iPos += iFrameSize;
		iFrame++;
	}
	fclose( fp );
	delete[] pUserData;
	return true;
}
	
///////////////////////////////////////////////////////////////////////////////
void PerfManager::ReleaseMemory()
{
	if ( m_pNodeArr != NULL )
	{
		delete[] m_pNodeArr;
		m_pNodeArr = NULL;
	}
	if ( m_pNodeData != NULL )
	{
		delete[] m_pNodeData;
		m_pNodeData = NULL;
	}
	if ( m_pRoot != NULL )
	{
		delete m_pRoot;
	}
}

///////////////////////////////////////////////////////////////////////////////
bool PerfManager::VerifyFileHeader( PERF_FILE_HEADER * pHeader )
{
	assert( pHeader != NULL );

	return ( pHeader->dwFlag == g_dwFileFlag
			&& pHeader->dwVersion == g_dwFileVersion );
}

///////////////////////////////////////////////////////////////////////////////
//������ܴ��ڵľ������ļ�
///////////////////////////////////////////////////////////////////////////////
bool PerfManager::HandleOldFile( const char* pPath, int iUserDataSize, const char** pUserDataName )
{
	PERF_FILE_HEADER fh;

	FILE *fp = fopen( pPath, "rb" );
	if ( NULL == fp )
	{
		goto NewFile;
	}

	if (!readHeader(fp, fh))
	{
		fclose( fp );
		goto NewFile;
	}

	int iOldUserDataSize = fh.iUserDataSize;
	int iOldNumNode = fh.iNumNode;

	fclose( fp );
	if ( iOldUserDataSize != iUserDataSize )
	{	//�û����ݳߴ����
		goto NewFile;
	}

	if ( iOldNumNode == m_iTotalNode )
	{	//�ڵ���û�䣬ɶҲ����
		return true;
	}

	if ( iOldNumNode > m_iTotalNode )
	{	//�ڵ������ˣ�����ԭ�ļ�
		goto NewFile;
	}

	//�ڵ��������ˣ��޸�ԭ�����ļ�
	return RecreateDataFile( pPath, iOldNumNode, iUserDataSize, pUserDataName );

NewFile:
	return CreateFileHeader( pPath, iUserDataSize, pUserDataName );
}

///////////////////////////////////////////////////////////////////////////////
//��ȡ���ڵ�����ʱ
//���ڵ���ʱ�����������ӽڵ���ʱ�ĺ�
///////////////////////////////////////////////////////////////////////////////
long long PerfManager::GetRootTotalTime()
{
	long long i64Total = 0;
	PerfNode *pNode = m_pRoot->GetChild();

	while ( pNode != NULL )
	{
		i64Total += pNode->GetTotalTime();
		pNode = pNode->GetSibling();
	}
	return i64Total;
}

PerfNode* PerfManager::GetNode( int iIndex )
{
	if ( iIndex < 0 || iIndex >= m_iTotalNode || NULL == m_pNodeArr )
	{
		return NULL;
	}
	return m_pNodeArr[iIndex];
}

int PerfManager::sizeOfHeader() const
{
	return	sizeof(unsigned int)//	dwFlag;			//�ļ���ʶ
		+	sizeof(unsigned int)//	dwVersion;		//�汾
		+	sizeof(long long)//		i64CPUFreq;		//cpu��Ƶ
		+	sizeof(int)//			iNumNode;		//�����ڵ�����
		+	sizeof(int)//			iUserDataSize;	//�û���������
		+	sizeof(unsigned int)//	dwReserve1;		//����
		+	sizeof(unsigned int)//	dwReserve2;
		+	sizeof(unsigned int)//	dwReserve3;
		+	sizeof(unsigned int);//	dwReserve4;
}

#define READ_OR_RETURN(data)	if (fread(&data, sizeof(data), 1, fp) != 1)\
								{\
									return false;\
								}
bool PerfManager::readHeader(FILE *fp, PERF_FILE_HEADER& h)
{
	READ_OR_RETURN(h.dwFlag);
	READ_OR_RETURN(h.dwVersion);
	READ_OR_RETURN(h.i64CPUFreq);
	READ_OR_RETURN(h.iNumNode);
	READ_OR_RETURN(h.iUserDataSize);
	READ_OR_RETURN(h.dwReserve1);
	READ_OR_RETURN(h.dwReserve2);
	READ_OR_RETURN(h.dwReserve3);
	READ_OR_RETURN(h.dwReserve4);

	return true;
}

int PerfManager::sizeOfNodeHeader() const
{
	return	MAX_NODE_NAME_LEN	//�ڵ�����
		+	sizeof(int)//		iParent;				//���ڵ����
		+	sizeof(int)//		iSibling;				//��һ���ֵܽڵ����
		+	sizeof(int);//		iChild;					//�ӽڵ����
}

bool PerfManager::readNodeHeader(FILE* fp, PERF_NODE_HEADER& h)
{
	READ_OR_RETURN(h.szName);
	READ_OR_RETURN(h.iParent);
	READ_OR_RETURN(h.iSibling);
	READ_OR_RETURN(h.iChild);
	return true;
}

int PerfManager::sizeOfNodeData() const
{
	return	sizeof(int)//			iTotalCalls;	//�ܵ��ô���
		+	sizeof(long long);//		iTotalTime;		//�ܿ���ʱ��(cpu������)
}

bool PerfManager::readNodeData(FILE* fp, PERF_NODE_DATA& d)
{
	READ_OR_RETURN(d.iTotalCalls);
	READ_OR_RETURN(d.iTotalTime);
	return true;
}

#endif
