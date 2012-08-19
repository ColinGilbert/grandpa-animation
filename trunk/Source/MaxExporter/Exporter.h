///////////////////////////////////////////////////////////////////////////////////////////////////
//Exporter.h
//
//描述：
//
//历史：
//		07-12-17	创建
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef __EXPORTER_H__
#define __EXPORTER_H__

#include <string>
#include <vector>
#include <set>

class Interface;
class INode;
class ISkin;
class ISkinContextData;
class TriObject;

namespace grp
{
class SkeletonExporter;
class AnimationExporter;
class MeshExporter;
class SkinnedMeshExporter;
class RigidMeshExporter;
struct BoneTrack;
struct CoreBone;
struct LodIndices;
struct SkinVertex;
}

#define MAX_TEXCOORD_NUM	8

#define EXP_SKELETON					0x00000001L		//导出骨架
#define EXP_MESH						0x00000002L		//导出网格
#define EXP_ANIMATION					0x00000004L		//导出动画

#define EXP_ANIM_SAMPLE_STEP			0x00000010L		//单步采样动画
#define EXP_ANIM_SAMPLE_LINEAR			0x00000020L		//线性采样动画
#define EXP_ANIM_SAMPLE_SPLINE			0x00000030L		//样条采样动画
#define EXP_ANIM_SAMPLE_MASK			0x00000030L
#define EXP_ANIM_COMPRESS_QUATERNION	0x00000040L		//压缩四元数	

#define EXP_MESH_TEXCOORD				0x00000100L		//导出纹理坐标
#define EXP_MESH_TEXCOORD2				0x00000200L		//导出第二层纹理坐标
#define EXP_MESH_NORMAL					0x00000400L		//导出法线
#define EXP_MESH_TANGENT				0x00000800L		//导出切线和副法线（如果不导出纹理坐标或不导出法线就一定不导出切线副法线）
#define EXP_MESH_VERTEX_COLOR			0x00001000L		//导出顶点颜色
#define EXP_MESH_LOD					0x00002000L		//导出LOD信息
#define EXP_MESH_COMPRESS_POS			0x00004000L		//压缩顶点位置
#define EXP_MESH_COMPRESS_NORMAL		0x00008000L		//压缩法线/切线
#define EXP_MESH_COMPRESS_TEXCOORD		0x00010000L		//压缩纹理坐标
#define EXP_MESH_COMPRESS_WEIGHT		0x00020000L		//压缩权重
#define EXP_MESH_MERGEBUFFER			0x00040000L		//合并所有mesh buffer
#define EXP_MESH_PROPERTY				0x00080000L		//导出自定义属性

#define EXP_SKEL_COMPRESS				0x00100000L		//压缩骨架数据
#define EXP_SKEL_PROPERTY				0x00200000L		//导出骨骼自定义属性

#define	FILE_EXT_SKELETON		L".gsk"
#define	FILE_EXT_MESH_SKIN		L".gms"
#define	FILE_EXT_MESH_RIGID		L".gms" 
#define	FILE_EXT_ANIMATION		L".gan"
#define FILE_EXT_BSP			L".gbs"
#define FILE_EXT_MODEL          L".gmd"
#define FILE_EXT_PART           L".gpt"
#define FILE_EXT_MATERIAL       L".gmt"

const DWORD DEFAULT_SETTINGS = EXP_SKELETON
							  | EXP_ANIMATION
							  | EXP_MESH
							  | EXP_MESH_NORMAL
							  | EXP_MESH_TANGENT
							  | EXP_MESH_TEXCOORD
							  |	EXP_MESH_COMPRESS_POS
							  |	EXP_MESH_COMPRESS_NORMAL
							  |	EXP_MESH_COMPRESS_TEXCOORD
							  |	EXP_MESH_COMPRESS_WEIGHT
							  | EXP_MESH_MERGEBUFFER
							  | EXP_ANIM_SAMPLE_SPLINE;

///////////////////////////////////////////////////////////////////////////////////////////////////
struct ExportOptions
{
	DWORD					exportType;
	std::vector<INode*>*	skeletonBoneNodes;
	std::vector<INode*>*	animationBoneNodes;
	std::vector<INode*>*	meshNodes;
	float					positionTolerance;
	float					rotationTolerance;
	float					scaleTolerance;
	float					lodLevelScale;
	float					lodMaxError;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
struct ExportVertex
{
	ExportVertex()
		:	nextIndex(-1)
		,	maxVertexIndex(-1)
		,	smoothGroup(-1)
		,	copyPos(-1)
		,	copyNormal(-1)
		,	color(-1)
	{
	}

	int		nextIndex;		//下一个对应到相同max顶点的导出顶点
	int		maxVertexIndex;
	grp::Vector3	position;
	grp::Vector3	normal;
	grp::Vector3	tangent;
	grp::Vector3	binormal;
	int		smoothGroup;
	int		copyPos;
	int		copyNormal;
	int		texcoord[MAX_TEXCOORD_NUM];
	int		color;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class CExporter
{
public:
	CExporter(Interface* pInterface);
	~CExporter();

public:
	enum ENUM_NODE_TYPE
	{
		ENUM_NODE_UNKNOWN	= 0,
		ENUM_NODE_ROOT,
		ENUM_NODE_BONE,
		ENUM_NODE_MESH,
		ENUM_NODE_DUMMY,
	};

public:
	bool doExport(LPCWSTR szFilename);

	const std::string& getLastError() const;

	//for OptionDlg :)
	static ENUM_NODE_TYPE checkNodeType(INode* pNode);

private:
	void EnumTree(INode* pNode);
	
	bool exportRigid(const std::wstring& strFilename);
	bool ExportSkeleton(const std::wstring& strFilename);
	bool exportMesh(const std::wstring& strFilename);
	bool ExportAnimation(const std::wstring& strFilename);

    bool ExportModel();

    bool ExportPart();

    bool ExportMaterial(const std::wstring& strFilename);
    bool writeMaterialFile( Mtl* material, const std::wstring& strFilename );

    static std::wstring strtowstr( const std::string& str );
    static std::wstring getFilenameFromFullpath( const std::wstring& filepath );
    static std::wstring getFilePath(const std::wstring& fileFullPath);

	bool BuildSkeleton();
	bool buildMesh();
	bool buildMeshRigid();
	bool BuildAnimation();

	bool buildOneMeshSkin(grp::SkinnedMeshExporter* skinnedMesh,
						   INode* pNode,
						   ISkin* skin,
						   ISkinContextData* pData,
						   Mesh* pMesh);

	bool buildOneMeshRigid(grp::RigidMeshExporter* rigidMesh,
							INode* pNode,
							Mesh* pMesh);

	void getBoneTransform(INode* pNode, grp::CoreBone& bone);

	bool getMeshInterface(INode* pNode,
						   ISkin* &skin,
						   ISkinContextData* &pContextData,
						   Mesh* &pMesh);
	ISkin* getISkinPtr(Object* pObj);
	Mesh* getMeshPtr(INode* pNode, Object* pObj);

	bool generateSkinBone(grp::SkinnedMeshExporter* skinnedMesh, INode* pNode, ISkin* skin);
	bool generateMeshVertex(INode* pNode,
							 Mesh* pMesh,
							 std::vector<ExportVertex>& exportVertices,
							 std::vector< std::vector<grp::LodIndices> >& vecVecIndex);
	bool buildMeshLod(std::vector<ExportVertex>& exportVertices, std::vector< std::vector<grp::LodIndices> >& buffers);

	void changeCopyPos(std::vector<ExportVertex>& exportVertices, int src, int dst);
	void changeCopyNormal(std::vector<ExportVertex>& exportVertices, int src, int dst);

	bool buildGrpSkinnedMesh(grp::SkinnedMeshExporter* skinnedMesh,
							INode* pNode,
							ISkin* skin,
							ISkinContextData* pData,
							Mesh* pMesh,
							const std::vector<ExportVertex>& exportVertices,
							const std::vector< std::vector<grp::LodIndices> >& buffers);
	bool buildGrpRigidMesh(grp::RigidMeshExporter* skinnedMesh,
						 INode* pNode,
						 Mesh* pMesh,
						 const std::vector<ExportVertex>& exportVertices,
						 const std::vector< std::vector<grp::LodIndices> >& buffers);

	DWORD addExportVertex(std::vector<ExportVertex>& vecVertex,
						   bool* aReferenced,
						   int iVertex,				//max顶点索引
						   int iSmGroup,
						   const grp::Vector3& position,
						   const grp::Vector3& vNormal,
						   const grp::Vector3& vTangent,
						   const grp::Vector3& vBinormal,
						   const int* aTexCoord,
						   int iNumTexCoord,
						   int iColor);
	void addTriangle(std::vector< std::vector<grp::LodIndices> >& buffers, MtlID materialId,
						DWORD index0, DWORD index1, DWORD index2);

	bool buildBoneTrack(INode* pNode,
						 grp::BoneTrack& boneTrack,
						 int iTimeStart, 
						 int iTimeEnd,
						 int iFrameRate,
						 int iFrameTime);

	float calcWeightError(grp::SkinnedMeshExporter& skinnedMesh, grp::SkinVertex& skinVertex, grp::Vector3& position);

	void checkMeshExpType();

	void setLastError(const std::string& strError);

private:
	Interface*						m_interface;

	grp::SkeletonExporter*			m_skeleton;
	grp::AnimationExporter*			m_animation;
	std::vector<grp::MeshExporter*>	m_meshes;

	std::vector<INode*>				m_skeletonBoneNodes;
	std::vector<INode*>				m_animationBoneNodes;
	std::vector<INode*>				m_meshNodes;

	ExportOptions					m_options;

	std::string						m_lastError;

    std::wstring                    m_mainFileName;
    std::wstring                    m_exportPath;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
inline const std::string& CExporter::getLastError() const
{
	return m_lastError;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
inline void CExporter::setLastError(const std::string& strError)
{
	m_lastError = strError;
}

DWORD WINAPI NullFunc(LPVOID arg);

#endif