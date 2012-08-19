///////////////////////////////////////////////////////////////////////////////
//TreeEnumProc.h
//
//描述：
//
//历史：
//		07-12-17	郑榕	创建
///////////////////////////////////////////////////////////////////////////////
#ifndef __TREE_ENUM_PROC_H__
#define __TREE_ENUM_PROC_H__

#include <vector>
#include <map>
#include "CoreSkeleton.h"

class CCoreMeshSkin;
class CCoreMeshRigid;
class ISkin;

class CTreeEnumProc : public ITreeEnumProc
{
public:
	CTreeEnumProc( Interface* pInterface )
		:	m_pRoot( NULL )
		,	m_pInterface( pInterface )
	{}

public:
    virtual int callback( INode *pNode );

	bool ExportSkeleton( const TCHAR* szFilename );

private:
	enum ENUM_NODE_TYPE
	{
		ENUM_NODE_UNKNOWN	= 0,
		ENUM_NODE_BONE		= 1,
		ENUM_NODE_SKIN		= 2,
		ENUM_NODE_DUMMY		= 3,
		ENUM_NODE_NOTEXPORT	= 0xffff,
	};

private:
	ENUM_NODE_TYPE CheckNodeType( INode* pNode );
	ENUM_NODE_TYPE CheckObjType( INode* pNode, Object* pObj );

	void AddNodeBone( INode* pNode );
	void AddNodeMesh( INode* pNode );

	void GetBoneTransform( INode* pNode, SCoreBone& bone );

	void BuildBoneTree();

	ISkin* GetSkinInterface( INode* pNode );

private:
	Interface*				m_pInterface;
	INode*					m_pRoot;

	CCoreSkeleton					m_Skeleton;
	std::vector<CCoreMeshSkin*>		m_vecMeshSkin;
	std::vector<CCoreMeshRigid*>	m_vecMeshRigid;

	std::vector<INode*>		m_vecNodeBone;
	std::map<INode*, int>	m_mapNodeBone;

	std::vector<INode*>		m_vecNodeMesh;
};

#endif