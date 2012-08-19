///////////////////////////////////////////////////////////////////////////////
//TreeEnumProc.cpp
//
//描述：
//
//历史：
//		07-12-17	郑榕	创建
///////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "TreeEnumProc.h"
#include "CoreMeshSkin.h"
#include "CoreMeshRigid.h"
#include "ISkin.h"
#include <fstream>

using std::vector;
using std::map;
using std::make_pair;
using std::fstream;
using std::ios_base;
using std::string;

/////////////////////////////////////////////////////////////////////////////////////////////
int CTreeEnumProc::callback( INode *pNode )
{
	assert( pNode != NULL );
	if ( NULL == pNode->GetParentNode() )
	{
		m_pRoot = pNode;
	}

	ENUM_NODE_TYPE type = CheckNodeType( pNode );
	switch ( type )
	{
	case ENUM_NODE_BONE:
		AddNodeBone( pNode );
		break;

	case ENUM_NODE_SKIN:
		AddNodeMesh( pNode );
		break;

	default:
		break;
	}

	return TREE_CONTINUE;
}

/////////////////////////////////////////////////////////////////////////////////////////////
bool CTreeEnumProc::ExportSkeleton( const TCHAR* szFilename )
{
	BuildBoneTree();

	fstream file;
	file.open( szFilename, ios_base::out | ios_base::binary );
	if ( !file.is_open() )
	{
		return false;
	}
	bool bRet = m_Skeleton.Export( file );
	file.close();
	return bRet;
}

/////////////////////////////////////////////////////////////////////////////////////////////
void CTreeEnumProc::BuildBoneTree()
{
	for ( vector<INode*>::iterator iterNode = m_vecNodeBone.begin();
		iterNode != m_vecNodeBone.end();
		++iterNode )
	{
		INode* pNode = *iterNode;
		assert( pNode != NULL );

		map<INode*, int>::iterator iterFound = m_mapNodeBone.find( pNode );
		assert( iterFound != m_mapNodeBone.end() );
		SCoreBone* pBone = m_Skeleton.GetCoreBone( (*iterFound).second );
		assert( pBone != NULL );
		assert( pBone->iId == (*iterFound).second );

		INode* pNodeParent = pNode->GetParentNode();
		if ( NULL == pNodeParent ||
			ENUM_NODE_BONE != CheckNodeType( pNodeParent ) )
		{
			continue;
		}
		iterFound = m_mapNodeBone.find( pNodeParent );
		if ( iterFound == m_mapNodeBone.end() )
		{
			continue;
		}
		SCoreBone* pBoneParent = m_Skeleton.GetCoreBone( (*iterFound).second );
		assert( pBoneParent != NULL );
		assert( pBoneParent->iId == (*iterFound).second );

		pBone->iParentId = pBoneParent->iId;
		pBoneParent->vecChildId.push_back( pBone->iId );
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
CTreeEnumProc::ENUM_NODE_TYPE CTreeEnumProc::CheckNodeType( INode* pNode )
{
	assert( pNode != NULL );
	Object* pObj = pNode->GetObjectRef();
	if ( NULL == pObj )
	{
		return ENUM_NODE_UNKNOWN;
	}
	switch ( pObj->SuperClassID() )
	{
	case HELPER_CLASS_ID:
	case GEOMOBJECT_CLASS_ID:
		return CheckObjType( pNode, pObj );

	case GEN_DERIVOB_CLASS_ID:
		while ( pObj->SuperClassID() == GEN_DERIVOB_CLASS_ID )
		{
			pObj = ((IDerivedObject*)pObj)->GetObjRef();
		} 
		return CheckObjType( pNode, pObj );

	default:
		break;
	}
	return ENUM_NODE_UNKNOWN;
}

/////////////////////////////////////////////////////////////////////////////////////////////
CTreeEnumProc::ENUM_NODE_TYPE CTreeEnumProc::CheckObjType( INode* pNode, Object* pObj )
{
	assert( pNode != NULL );
	assert( pObj != NULL );

	SClass_ID superClassID = pObj->SuperClassID();
	Class_ID classId = pObj->ClassID();

	switch ( superClassID )
	{
	case HELPER_CLASS_ID:
		if ( Class_ID( BONE_CLASS_ID, 0 ) == pObj->ClassID() )
		{
			return ENUM_NODE_BONE;
		}
		break;

	case GEOMOBJECT_CLASS_ID:
		if ( BONE_OBJ_CLASSID == classId )
		{
			return ENUM_NODE_BONE;
		}
		else
		{
			Control* c = pNode->GetTMController();
			Class_ID id = c->ClassID();
			if ( BIPSLAVE_CONTROL_CLASS_ID == id
				|| BIPBODY_CONTROL_CLASS_ID == id
				|| FOOTPRINT_CLASS_ID == id )
			{
				return ENUM_NODE_BONE;
			}
			else
			{
				return ENUM_NODE_SKIN;
			}
		}
		break;

	default:
		break;
	}
	return ENUM_NODE_UNKNOWN;
}

/////////////////////////////////////////////////////////////////////////////////////////////
void CTreeEnumProc::AddNodeBone( INode* pNode )
{
	assert( pNode != NULL );
	
	SCoreBone bone;
	bone.strName = pNode->GetName();
	GetBoneTransform( pNode, bone );

	int iBoneId = m_Skeleton.AddCoreBone( bone );

	m_vecNodeBone.push_back( pNode );
	m_mapNodeBone.insert( make_pair( pNode, iBoneId ) );
}

/////////////////////////////////////////////////////////////////////////////////////////////
void CTreeEnumProc::AddNodeMesh( INode* pNode )
{
	assert( pNode != NULL );

	//ISkin* pSkin = GetSkinInterface( pNode );
	//if ( NULL == pSkin )
	//{
	//	return;
	//}
	//ISkinContextData* pData = pSkin->GetContextInterface( pNode );
	//if ( NULL == pData )
	//{
	//	return;
	//}
	//CCoreMeshSkin* pCoreMesh = new CCoreMeshSkin;
	//int iNumBone = pSkin->GetNumBones();
	//for ( int i = 0; i < iNumBone; ++i )
	//{
	//	string strBoneName = pSkin->GetBoneName( i );
	//	pCoreMesh->AddBone( strBoneName );
	//}

	m_vecNodeMesh.push_back( pNode );
}

/////////////////////////////////////////////////////////////////////////////////////////////
void GetTransformFromMatrix3( const Matrix3& mat3,
							  Vector3& vTranslation,
							  Quaternion& qRotation )
{
	Matrix m44;
	memcpy( &(m44.v[0]), &( mat3.GetRow( 0 ) ), sizeof(Point3) );
	memcpy( &(m44.v[1]), &( mat3.GetRow( 1 ) ), sizeof(Point3) );
	memcpy( &(m44.v[2]), &( mat3.GetRow( 2 ) ), sizeof(Point3) );
	memcpy( &(m44.v[3]), &( mat3.GetRow( 3 ) ), sizeof(Point3) );
	memcpy( &vTranslation, &( mat3.GetRow( 3 ) ), sizeof(Point3) );
	m44._14 = 0.0f;
	m44._24 = 0.0f;
	m44._34 = 0.0f;
	m44._44 = 1.0f;
	qRotation = m44;
}

/////////////////////////////////////////////////////////////////////////////////////////////
void CTreeEnumProc::GetBoneTransform( INode* pNode, SCoreBone& bone )
{
	assert( pNode != NULL );
	assert( m_pInterface != NULL );
	Matrix3 mNode = pNode->GetNodeTM( m_pInterface->GetTime() );
	Matrix3 mRelative;
	
	if ( pNode->IsRootNode() )
	{
		mRelative = mNode;
	}
	else
	{
		Matrix3 mParent = pNode->GetParentTM( m_pInterface->GetTime() );
        if ( memcmp( &mNode, &mParent, sizeof(Matrix3) ) == 0 )
        {
            mRelative.IdentityMatrix();
        }
        else
        {
            mRelative = mNode * Inverse( mParent );
        }
	}
	GetTransformFromMatrix3( mRelative, bone.vTranslation, bone.qRotation );
}

/////////////////////////////////////////////////////////////////////////////////////////////
ISkin* CTreeEnumProc::GetSkinInterface( INode* pNode )
{
	Object* pObj = pNode->GetObjectRef();
	if ( NULL == pObj )
	{
		return NULL;
	}

	while ( GEN_DERIVOB_CLASS_ID == pObj->SuperClassID() )
	{
		IDerivedObject* pDerObj = static_cast<IDerivedObject*>( pObj );

		int ModStackIndex = 0;
		while ( ModStackIndex < pDerObj->NumModifiers() )
		{
			Modifier* mod = pDerObj->GetModifier( ModStackIndex );
			if ( SKIN_CLASSID == mod->ClassID() )
			{
				return (ISkin*)(mod->GetInterface( I_SKIN ));
			}
			++ModStackIndex;
		}
		pObj = pDerObj->GetObjRef();
	}

	return NULL;
}