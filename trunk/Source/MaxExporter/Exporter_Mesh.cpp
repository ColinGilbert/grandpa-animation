///////////////////////////////////////////////////////////////////////////////////////////////////
//Exporter.cpp
//
//描述：
//		Mesh相关导出代码
//
//历史：
//		07-12-17	创建
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "Precompiled.h"
#include "Exporter.h"
#include "SkeletonFile.h"
#include "SkinnedMeshExporter.h"
#include "RigidMeshExporter.h"
//#include "Bsp.h"
#include "ISkin.h"
#include "MathConversion.h"
#include "Mesh.h"
#include "IMesh.h"
#include "LodGenerator.h"
#include "StrSafe.h"
#include <fstream>
#include <algorithm>
#include <MeshNormalSpec.h>

BOOL GetVertexNormalUsingSmoothGroup(Point3& VN, Mesh& mesh, int faceId, int globalvertexId, int _FaceVertexIdx);

///////////////////////////////////////////////////////////////////////////////////////////////////
bool CExporter::buildOneMeshRigid(grp::RigidMeshExporter* rigidMesh,
								   INode* node,
								   Mesh* mesh)
{
	//顶点信息
	std::vector<ExportVertex> exportVertices;
	//索引
	std::vector< std::vector<grp::LodIndices> > buffers;

	//构建顶点信息
	if (!generateMeshVertex(node, mesh, exportVertices, buffers))
	{
		return false;
	}
	if (!buildGrpRigidMesh(rigidMesh, node, mesh, exportVertices, buffers))
	{
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool CExporter::buildGrpRigidMesh(grp::RigidMeshExporter* rigidMesh,
								   INode* node,
								   Mesh* mesh,
								   const std::vector<ExportVertex>& exportVertices,
								   const std::vector< std::vector<grp::LodIndices> >& buffers)
{
	assert(rigidMesh != NULL);
	assert(node != NULL);

	wchar_t unicodeString[256];
	mbstowcs(unicodeString, node->GetName(), 255);
	rigidMesh->m_name = unicodeString;
	rigidMesh->m_positions.resize(exportVertices.size());
	rigidMesh->m_type = grp::MESH_RIGID;
	rigidMesh->m_vertexFormat = grp::POSITION;

	if ((m_options.exportType & EXP_MESH_NORMAL) != 0)
	{
		rigidMesh->m_vertexFormat |= grp::NORMAL;
	}
	if ((m_options.exportType & EXP_MESH_TANGENT) != 0)
	{
		rigidMesh->m_vertexFormat |= grp::TANGENT;
	}
	if ((m_options.exportType & EXP_MESH_TEXCOORD) != 0)
	{
		rigidMesh->m_vertexFormat |= grp::TEXCOORD;
	}
	if ((m_options.exportType & EXP_MESH_TEXCOORD2) != 0)
	{
		rigidMesh->m_vertexFormat |= grp::TEXCOORD2;
	}
	if ((m_options.exportType & EXP_MESH_VERTEX_COLOR) != 0)
	{
		rigidMesh->m_vertexFormat |= grp::COLOR;
	}

	//纹理坐标层数
	int texCoordCount = mesh->getNumMaps() - 1;	//0层为顶点颜色，1层为第一层纹理坐标
	assert(texCoordCount >= 0);
	texCoordCount = std::min(2, texCoordCount);
	if (mesh->tvFace == NULL)
	{
		texCoordCount = 0;
	}
	if ((m_options.exportType & EXP_MESH_TEXCOORD) == 0)
	{
		texCoordCount = 0;
	}
	if (texCoordCount == 0)
	{
		rigidMesh->m_vertexFormat &= ~grp::TEXCOORD;
		rigidMesh->m_vertexFormat &= ~grp::TEXCOORD2;
		rigidMesh->m_vertexFormat &= ~grp::TANGENT;
	}
	else if (texCoordCount == 1)
	{
		rigidMesh->m_vertexFormat &= ~grp::TEXCOORD2;
	}
	else if ((rigidMesh->m_vertexFormat & grp::TEXCOORD2) == 0)
	{
		texCoordCount = 1;
	}
	if (mesh->vcFace == NULL)
	{
		rigidMesh->m_vertexFormat &= ~grp::COLOR;
	}

	if (rigidMesh->checkVertexFormat(grp::NORMAL))
	{
		rigidMesh->m_normals.resize(exportVertices.size());
	}
	if (rigidMesh->checkVertexFormat(grp::TANGENT))
	{
		rigidMesh->m_tangents.resize(exportVertices.size());
		rigidMesh->m_binormals.resize(exportVertices.size());
	}
	if (rigidMesh->checkVertexFormat(grp::TEXCOORD))
	{
		rigidMesh->m_texCoordsArray.resize(texCoordCount);
		for (int i = 0; i < texCoordCount; ++i)
		{
			rigidMesh->m_texCoordsArray[i].resize(exportVertices.size());
		}
	}
	if (rigidMesh->checkVertexFormat(grp::COLOR))
	{
		rigidMesh->m_colors.resize(exportVertices.size());
	}
	
	for (int iVertex = 0; iVertex < exportVertices.size(); ++iVertex)
	{
		const ExportVertex& vertexExp = exportVertices[iVertex];
		//pos
		grp::Vector3& vPos = rigidMesh->m_positions[iVertex];
		::Vector3FromPoint3(vPos, mesh->verts[vertexExp.maxVertexIndex]);
		//normal
		if (rigidMesh->checkVertexFormat(grp::NORMAL))
		{
			grp::Vector3& normal = rigidMesh->m_normals[iVertex];
			normal = vertexExp.normal;
			normal.normalize();
		}
		//tangent/binormal
		if (rigidMesh->checkVertexFormat(grp::TANGENT))
		{
			grp::Vector3& tangent = rigidMesh->m_tangents[iVertex];
			grp::Vector3& binormal = rigidMesh->m_binormals[iVertex];
			tangent = vertexExp.tangent;
			binormal = vertexExp.binormal;
			tangent.normalize();
			binormal.normalize();
		}
		//纹理坐标
		for (int i = 0; i < texCoordCount; ++i)
		{
			grp::Vector2& texcoord = rigidMesh->m_texCoordsArray[i][iVertex];
			if (i == 0)
			{
				::Vector2FromPoint3(texcoord, mesh->tVerts[vertexExp.texcoord[i]]);
			}
			else
			{
				UVVert* pUV = mesh->mapVerts(i + 1);
				::Vector2FromPoint3(texcoord, pUV[vertexExp.texcoord[i]]);
			}
			texcoord.Y = 1.0f - texcoord.Y;
			//纹理坐标超出0,1范围就不压缩
			if ((m_options.exportType & EXP_MESH_COMPRESS_TEXCOORD) != 0 &&
				(texcoord.X < 0.0f || texcoord.X > 1.0f ||	texcoord.Y < 0.0f || texcoord.Y > 1.0f))
			{
				m_options.exportType &= (~EXP_MESH_COMPRESS_TEXCOORD);
			}
		}
		//顶点颜色
		if (rigidMesh->checkVertexFormat(grp::COLOR))
		{
			::DWORDFromPoint3(rigidMesh->m_colors[iVertex], mesh->vertCol[vertexExp.color]);
		}
	}//for (int iVertex = 0; iVertex < iNumVertexMax; ++iVertex)

	rigidMesh->m_meshBuffers = buffers;

	INode* parent = node->GetParentNode();
	if (parent != NULL && !parent->IsRootNode())
	{
		mbstowcs(unicodeString, parent->GetName(), 255);
		rigidMesh->m_attachedBoneName = unicodeString;

		Matrix3 nodeTM = node->GetObjTMBeforeWSM(0);
		Matrix3 parentTM = parent->GetNodeTM(0);
		::MatrixFromMatrix3(rigidMesh->m_transform, nodeTM * Inverse(parentTM));
	}
	else
	{
		::MatrixFromMatrix3(rigidMesh->m_transform, node->GetObjTMBeforeWSM(0));
	}
	if ((m_options.exportType & EXP_MESH_PROPERTY) != 0)
	{
		TSTR buffer;
		node->GetUserPropBuffer(buffer);
		wchar_t* unicodeString = new wchar_t[buffer.length() + 1];
		mbstowcs(unicodeString, buffer.data(), buffer.length() + 1);
		rigidMesh->m_property = unicodeString;
		delete[] unicodeString;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool CExporter::exportMesh(const std::wstring& strFilename)
{
	checkMeshExpType();

	if (!buildMesh())
	{
		return false;
	}
	for (DWORD i = 0; i < m_meshes.size(); ++i)
	{
		grp::MeshExporter* mesh = m_meshes[i];
		std::wstring strFilePath = strFilename;
		if (m_meshes.size() > 1)
		{
			strFilePath += L"_";
			strFilePath += mesh->getName();
		}
		strFilePath += FILE_EXT_MESH_SKIN;

		std::fstream file;
		file.open(strFilePath.c_str(), std::ios_base::out | std::ios_base::binary);
		if (!file.is_open())
		{
			char szError[1024];
			::StringCchPrintf(szError, sizeof(szError), "Failed to open file [%s]", strFilePath.c_str());
			setLastError(szError);
			return false;
		}

		if (!mesh->exportTo(file,
							  (m_options.exportType & EXP_MESH_COMPRESS_POS) != 0,
							  (m_options.exportType & EXP_MESH_COMPRESS_NORMAL) != 0,
							  (m_options.exportType & EXP_MESH_COMPRESS_TEXCOORD) != 0,
							  (m_options.exportType & EXP_MESH_COMPRESS_WEIGHT) != 0))
		{
			file.close();
			return false;
		}
		file.close();
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool CExporter::buildMesh()
{
	m_meshes.clear();
	
	for (std::vector<INode*>::iterator iterNode = m_meshNodes.begin();
		  iterNode != m_meshNodes.end();
		  ++iterNode)
	{
		INode* node = *iterNode;
		assert(node != NULL);

		ISkin* skin = NULL;
		ISkinContextData* data = NULL;
		Mesh* mesh = NULL;
		if (!getMeshInterface(node, skin, data, mesh))
		{
			continue;
		}
		grp::MeshExporter* meshFile = NULL;
		if (skin != NULL)
		{
			grp::SkinnedMeshExporter *skinnedMesh = new grp::SkinnedMeshExporter;
			if (!buildOneMeshSkin(skinnedMesh, node, skin, data, mesh))
			{
				delete skinnedMesh;
				return false;
			}
			meshFile = skinnedMesh;
		}
		else
		{
			grp::RigidMeshExporter *rigidMesh = new grp::RigidMeshExporter;
			if (!buildOneMeshRigid(rigidMesh, node, mesh))
			{
				delete rigidMesh;
				return false;
			}
			meshFile = rigidMesh;
		}
		assert(meshFile != NULL);
		m_meshes.push_back(meshFile);
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool CExporter::getMeshInterface(INode* node,
								  ISkin* &skin,
								  ISkinContextData* &contextData,
								  Mesh* &mesh)
{
	mesh = NULL;
	skin = NULL;
	contextData = NULL;

	Object* object = node->GetObjectRef();
	if (object == NULL)
	{
		return false;
	}
	mesh = getMeshPtr(node, object);
	if (mesh == NULL)
	{
		return false;
	}

	skin = getISkinPtr(object);
	if (skin == NULL)
	{
		return true;
	}
	contextData = skin->GetContextInterface(node);
	if (contextData == NULL)
	{
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool CExporter::buildOneMeshSkin(grp::SkinnedMeshExporter* skinnedMesh,
								  INode* node,
								  ISkin* skin,
								  ISkinContextData* data,
								  Mesh* mesh)
{
	//顶点信息
	std::vector<ExportVertex> exportVertices;
	//索引
	std::vector< std::vector<grp::LodIndices> > buffers;	//每个子mesh，每级lod的index buffer

	//构建顶点信息
	if (!generateMeshVertex(node, mesh, exportVertices, buffers))
	{
		return false;
	}
	if (!buildGrpSkinnedMesh(skinnedMesh,	node, skin,	data, mesh, exportVertices, buffers))
	{
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool CExporter::generateSkinBone(grp::SkinnedMeshExporter* skinnedMesh, INode* node, ISkin* skin)
{
	Matrix3 mSkinOffset;
	if (!skin->GetSkinInitTM(node, mSkinOffset, true))
	{
		setLastError("获取Skin偏移矩阵失败");
		return false;
	}

	int iNumBone = skin->GetNumBones();
	skinnedMesh->m_boneNames.resize(iNumBone);
	skinnedMesh->m_offsetMatrices.resize(iNumBone);
	for (int i = 0; i < iNumBone; ++i)
	{
		//mbs only
		INode* pBone = skin->GetBone(i);
		if (NULL == pBone)
		{
			setLastError("找不到skin依赖的骨骼");
			return false;
		}
		if (ENUM_NODE_DUMMY == checkNodeType(pBone))
		{
			char szError[1024];
			::StringCchPrintf(szError, sizeof(szError), "发现绑定到虚拟体[%s]的顶点", pBone->GetName());
			setLastError(szError);
			return false;
		}

		wchar_t unicodeString[256];
		mbstowcs(unicodeString, pBone->GetName(), 255);
		skinnedMesh->m_boneNames[i] = unicodeString;

		Matrix3 mBoneOffset;
		skin->GetBoneInitTM(pBone, mBoneOffset);
		mBoneOffset.Invert();
		Matrix3 meshOffset = node->GetObjectTM(0);
		mBoneOffset = meshOffset * mBoneOffset;
		//mBoneOffset = mSkinOffset * mBoneOffset;
		grp::Matrix& mCore = skinnedMesh->m_offsetMatrices[i];
		::MatrixFromMatrix3(mCore, mBoneOffset);
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool CExporter::generateMeshVertex(INode* node,
									Mesh* mesh,
									std::vector<ExportVertex>& exportVertices,
									std::vector< std::vector<grp::LodIndices> >& buffers)
{
	int iNumVertexMax = mesh->numVerts;

	//标志数组：max顶点是否被引用过
	bool* aReferenced = new bool[iNumVertexMax];
	memset(aReferenced, 0, sizeof(bool) * iNumVertexMax);

	assert(node != NULL);
	BOOL swapTriWind = (node->GetNodeTM(0).Parity());

	//前iNumVertexMax个与max顶点一一对应
	//此外由于法线，纹理坐标或顶点颜色的不同还会添加一些冗余顶点
	exportVertices.resize(iNumVertexMax);

	//纹理坐标层数
	int texCoordCount = mesh->getNumMaps() - 1;	//0层为顶点颜色，1层为第一层纹理坐标
	assert(texCoordCount >= 0);
	texCoordCount = std::min(2, texCoordCount);
	//有时getNumMaps返回2但tvFace却为空，不明白怎么回事
	if (NULL == mesh->tvFace)
	{
		texCoordCount = 0;
	}

	mesh->checkNormals(TRUE);
	bool hasVertexColor = (NULL != mesh->vcFace);

	for (int faceIndex = 0; faceIndex < mesh->numFaces; ++faceIndex)
	{
		//每个三角形……
		Face& face = mesh->faces[faceIndex];

		//三个顶点用相同的normal,tangent,binormal
		grp::Vector3 faceNormal, tangent, binormal;
		::Vector3FromPoint3(faceNormal, mesh->getFaceNormal(faceIndex));
		grp::Vector3 v[3];
		grp::Vector2 uv[3];
		if (texCoordCount > 0)
		{
			for (int i = 0; i < 3; ++i)
			{
				::Vector3FromPoint3(v[i], mesh->verts[face.v[i]]);
				int iUVIndex = mesh->tvFace[faceIndex].t[i];
				::Vector2FromPoint3(uv[i], mesh->tVerts[iUVIndex]);
				uv[i].Y = 1.0f - uv[i].Y;
			}
			//::CalculateTangent(v, uv, tangent, binormal); 
		}
		DWORD tempIndex[3];
		for (int pointIndex = 0; pointIndex < 3; ++pointIndex)
		{
			//每个顶点……
			DWORD maxIndex = face.v[pointIndex];

            grp::Vector3 vertexNormal;
            Point3 vertexNormalPoint3;
            if( GetVertexNormalUsingSmoothGroup( vertexNormalPoint3, *mesh, faceIndex, maxIndex, pointIndex ))
            {
                vertexNormal.set( vertexNormalPoint3.x, vertexNormalPoint3.y, vertexNormalPoint3.z );
            }
            else
            {
                assert( false );
                vertexNormal.set( faceNormal.X, faceNormal.Y, faceNormal.Z );
            }

			grp::Vector3 position;
			::Vector3FromPoint3(position, mesh->verts[maxIndex]);
			//::Vector3FromPoint3(normal, mesh->getNormal(maxIndex));
			if (texCoordCount > 0)
			{
				::CalculateTangent(v, uv, pointIndex, tangent, binormal); 
			}
			int texcoords[MAX_TEXCOORD_NUM];
			for (int i = 0; i < texCoordCount; ++i)
			{
				if (0 == i)
				{
					texcoords[i] = mesh->tvFace[faceIndex].t[pointIndex];
				}
				else
				{
					texcoords[i] = mesh->mapFaces(i + 1)[faceIndex].t[pointIndex];
				}
			}
			int colorIndex = hasVertexColor ? mesh->vcFace[faceIndex].getTVert(pointIndex) : -1;

			DWORD index = addExportVertex(exportVertices,
											aReferenced,
											maxIndex,
											face.smGroup,
											position,
											vertexNormal,
											tangent,
											binormal,
											texcoords,
											texCoordCount,
											colorIndex);
			tempIndex[pointIndex] = index;
		}//for (int pointIndex = 0; pointIndex < 3; ++pointIndex)
		//为了保证绕向为默认的CCW，YZ轴互换后必须颠倒三角形绕向
		bool mergeBuffer = ((m_options.exportType & EXP_MESH_MERGEBUFFER) != 0);
		if (swapTriWind)
		{
			addTriangle(buffers, mergeBuffer ? 0 : face.getMatID(), tempIndex[0], tempIndex[2], tempIndex[1]);
		}
		else
		{
			addTriangle(buffers, mergeBuffer ? 0 : face.getMatID(), tempIndex[0], tempIndex[1], tempIndex[2]);
		}
	}//for (int faceIndex = 0; faceIndex < mesh->numFaces; ++faceIndex)

	//归一化
	for (int i = 0; i < exportVertices.size(); ++i)
	{
		exportVertices[i].normal.normalize();
		exportVertices[i].tangent.normalize();
		exportVertices[i].binormal.normalize();
	}

	delete[] aReferenced;

	//剔除空buffer
	for (std::vector< std::vector<grp::LodIndices> >::iterator iter = buffers.begin();
		iter != buffers.end();)
	{
		if ((*iter).empty())
		{
			iter = buffers.erase(iter);
		}
		else
		{
			++iter;
		}
	}

	//LOD计算
	if ((m_options.exportType & EXP_MESH_LOD) != 0)
	{
		return buildMeshLod(exportVertices, buffers);
	}
	//vecLodVertexNum.push_back(exportVertices.size());
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CExporter::addTriangle(std::vector< std::vector<grp::LodIndices> >& buffers, MtlID materialId,
							 DWORD index0, DWORD index1, DWORD index2)
{
	if (materialId >= buffers.size())
	{
		buffers.resize(materialId + 1);
	}
	if (buffers[materialId].empty())
	{
		buffers[materialId].resize(1);
		buffers[materialId][0].maxError = 0.0f;
		buffers[materialId][0].maxIndex = 0;
	}
	buffers[materialId][0].indices.push_back(index0);
	buffers[materialId][0].indices.push_back(index1);
	buffers[materialId][0].indices.push_back(index2);
	if (index0 > buffers[materialId][0].maxIndex)
	{
		buffers[materialId][0].maxIndex = index0;
	}
	if (index1 > buffers[materialId][0].maxIndex)
	{
		buffers[materialId][0].maxIndex = index1;
	}
	if (index2 > buffers[materialId][0].maxIndex)
	{
		buffers[materialId][0].maxIndex = index2;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool CExporter::buildMeshLod(std::vector<ExportVertex>& exportVertices, std::vector< std::vector<grp::LodIndices> >& buffers)
{
	std::vector<LodGenerator*> generators(buffers.size());
	for (size_t i = 0; i < buffers.size(); ++i)
	{
		generators[i] = new LodGenerator(exportVertices, buffers[i]);
		generators[i]->calculate(m_options.lodMaxError, m_options.lodLevelScale);
	}

	//所有顶点按塌陷顺序重新排序
	std::map<int, int> vertexMap;
	int newIndex = 0;
	//以误差从大到小的顺序填充所有mesh buffer的所有lod级别用到的顶点
	std::vector<int> lastLevel(buffers.size());
	for (size_t i = 0; i < buffers.size(); ++i)
	{
		assert(buffers[i].size() > 0);
		lastLevel[i] = buffers[i].size() - 1;
	}
	while (true)
	{
		int maxErrorBuffer = -1;
		float maxError = -1.0f;
		for (size_t i = 0; i < lastLevel.size(); ++i)
		{
			if (lastLevel[i] < 0)
			{
				continue;
			}
			//这里有点费解，但又懒得解释
			if (lastLevel[i] == buffers[i].size() - 1)
			{
				maxErrorBuffer = i;
				maxError = FLT_MAX;
			}
			else if (buffers[i][lastLevel[i] + 1].maxError > maxError)
			{
				maxErrorBuffer = i;
				maxError = buffers[i][lastLevel[i] + 1].maxError;
			}
		}
		if (maxErrorBuffer < 0)
		{
			break;
		}
		//填充顶点
		grp::LodIndices& lodIndices = buffers[maxErrorBuffer][lastLevel[maxErrorBuffer]];
		for (size_t i = 0; i < lodIndices.indices.size(); ++i)
		{
			int index = lodIndices.indices[i];
			if (vertexMap.find(index) == vertexMap.end())
			{
				vertexMap.insert(std::make_pair(index, newIndex));
				++newIndex;
			}
		}
		--lastLevel[maxErrorBuffer];
	}
	//顶点重新排序
	std::vector<ExportVertex> sorted(vertexMap.size());
	for (std::map<int, int>::iterator iterMap = vertexMap.begin();
		iterMap != vertexMap.end();
		++iterMap)
	{
		assert(iterMap->second < sorted.size());
		sorted[iterMap->second] = exportVertices[iterMap->first];
	}
	//整理index buffer
	for (size_t bufferIndex = 0; bufferIndex < buffers.size(); ++bufferIndex)
	{
		std::vector<grp::LodIndices>& buffer = buffers[bufferIndex];
		for (size_t levelIndex = 0; levelIndex < buffer.size(); ++levelIndex)
		{
			grp::LodIndices& lodIndice = buffer[levelIndex];
			lodIndice.maxIndex = 0;
			for (size_t indexIndex = 0; indexIndex < lodIndice.indices.size(); ++indexIndex)
			{
				grp::Index32& index = lodIndice.indices[indexIndex];
				assert(vertexMap.find(index) != vertexMap.end());
				index = vertexMap[index];
				if (index > lodIndice.maxIndex)
				{
					lodIndice.maxIndex = index;
				}
			}

		}
	}
	//整理冗余关系
	for (size_t vertexIndex = 0; vertexIndex < sorted.size(); ++vertexIndex)
	{
		ExportVertex& vertex = sorted[vertexIndex];
		std::map<int, int>::iterator found = vertexMap.find(vertex.copyPos);
		if (found != vertexMap.end())
		{
			vertex.copyPos = found->second;
		}
		found = vertexMap.find(vertex.copyNormal);
		if (found != vertexMap.end())
		{
			vertex.copyNormal = found->second;
		}
	}
	//纠正冗余关系，保证copyPos和copyNormal总是小于顶点下标
	for (size_t vertexIndex = 0; vertexIndex < sorted.size(); ++vertexIndex)
	{
		ExportVertex& vertex = sorted[vertexIndex];
		if (vertex.copyPos >= 0)
		{
			changeCopyPos(sorted, vertexIndex, vertex.copyPos);
		}
		if (vertex.copyNormal >= 0)
		{
			changeCopyNormal(sorted, vertexIndex, vertex.copyNormal);
		}
	}
	exportVertices.swap(sorted);

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CExporter::changeCopyPos(std::vector<ExportVertex>& exportVertices, int src, int dst)
{
	assert(src != dst);
	if (dst < src)
	{
		exportVertices[src].copyPos = dst;
	}
	else if (exportVertices[dst].copyPos >= 0)
	{
		changeCopyPos(exportVertices, src, exportVertices[dst].copyPos);
	}
	else
	{
		exportVertices[src].copyPos = -1;
		exportVertices[dst].copyPos = src;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CExporter::changeCopyNormal(std::vector<ExportVertex>& exportVertices, int src, int dst)
{
	assert(src != dst);
	if (dst < src)
	{
		exportVertices[src].copyNormal = dst;
	}
	else if (exportVertices[dst].copyNormal >= 0)
	{
		changeCopyNormal(exportVertices, src, exportVertices[dst].copyNormal);
	}
	else
	{
		exportVertices[src].copyNormal = -1;
		exportVertices[dst].copyNormal = src;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool CExporter::buildGrpSkinnedMesh(grp::SkinnedMeshExporter* skinnedMesh,
								   INode* node,
								   ISkin* skin,
								   ISkinContextData* data,
								   Mesh* mesh,
								   const std::vector<ExportVertex>& exportVertices,
								   const std::vector< std::vector<grp::LodIndices> >& buffers)
{
	assert(skinnedMesh != NULL);
	assert(node != NULL);

	wchar_t unicodeString[256];
	mbstowcs(unicodeString, node->GetName(), 255);
	skinnedMesh->m_name = unicodeString;
	skinnedMesh->m_vertexFormat = grp::POSITION;

	if (!generateSkinBone(skinnedMesh, node, skin))
	{
		return false;
	}

	skinnedMesh->m_skinVertices.resize(exportVertices.size());
	skinnedMesh->m_positions.resize(exportVertices.size());

	skinnedMesh->m_type = 0;
	if ((m_options.exportType & EXP_MESH_NORMAL) != 0)
	{
		skinnedMesh->m_vertexFormat |= grp::NORMAL;
		skinnedMesh->m_normals.resize(exportVertices.size());
	}
	if ((m_options.exportType & EXP_MESH_TANGENT) != 0)
	{
		skinnedMesh->m_vertexFormat |= grp::TANGENT;
		skinnedMesh->m_tangents.resize(exportVertices.size());
		skinnedMesh->m_binormals.resize(exportVertices.size());
	}
	if ((m_options.exportType & EXP_MESH_TEXCOORD) != 0)
	{
		skinnedMesh->m_vertexFormat |= grp::TEXCOORD;
	}
	if ((m_options.exportType & EXP_MESH_TEXCOORD2) != 0)
	{
		skinnedMesh->m_vertexFormat |= grp::TEXCOORD2;
	}
	if ((m_options.exportType & EXP_MESH_VERTEX_COLOR) != 0)
	{
		skinnedMesh->m_vertexFormat |= grp::COLOR;
	}

	//纹理坐标层数
	int texCoordCount = mesh->getNumMaps() - 1;	//0层为顶点颜色，1层为第一层纹理坐标
	assert(texCoordCount >= 0);
	texCoordCount = std::min(2, texCoordCount);
	if (mesh->tvFace == NULL)
	{
		texCoordCount = 0;
	}
	if ((m_options.exportType & EXP_MESH_TEXCOORD) == 0)
	{
		texCoordCount = 0;
	}
	if (texCoordCount == 0)
	{
		skinnedMesh->m_vertexFormat &= ~grp::TEXCOORD;
		skinnedMesh->m_vertexFormat &= ~grp::TEXCOORD2;
		skinnedMesh->m_vertexFormat &= ~grp::TANGENT;
	}
	else if (texCoordCount == 1)
	{
		skinnedMesh->m_vertexFormat &= ~grp::TEXCOORD2;
	}
	else if ((skinnedMesh->m_vertexFormat & grp::TEXCOORD2) == 0)
	{
		texCoordCount = 1;
	}
	if (mesh->vcFace == NULL)
	{
		skinnedMesh->m_vertexFormat &= ~grp::COLOR;
	}

	if (skinnedMesh->checkVertexFormat(grp::TEXCOORD))
	{
		skinnedMesh->m_texCoordsArray.resize(texCoordCount);
		for (int i = 0; i < texCoordCount; ++i)
		{
			skinnedMesh->m_texCoordsArray[i].resize(exportVertices.size());
		}
	}
	if (skinnedMesh->checkVertexFormat(grp::COLOR))
	{
		skinnedMesh->m_colors.resize(exportVertices.size());
	}

	bool influenceIgnored = false;

	for (int iVertex = 0; iVertex < exportVertices.size(); ++iVertex)
	{
		grp::SkinVertex& skinVertex = skinnedMesh->m_skinVertices[iVertex];
		const ExportVertex& vertexExp = exportVertices[iVertex];

		//骨骼权重
		size_t influenceCount = data->GetNumAssignedBones(vertexExp.maxVertexIndex);
		if (influenceCount <= 0)
		{
			setLastError("Vertex has no bone weight.");
			return false;
		}
		std::vector<grp::VertexInfluence> vecInflu;
		vecInflu.resize(std::max(influenceCount, grp::MAX_VERTEX_INFLUENCE));
		for (int iInfluence = 0; iInfluence < influenceCount; ++iInfluence)
		{
			//每个影响顶点的骨骼
			grp::VertexInfluence influence;
			influence.boneIndex = data->GetAssignedBone(vertexExp.maxVertexIndex, iInfluence);
			influence.weight = data->GetBoneWeight(vertexExp.maxVertexIndex, iInfluence);
			if (influence.weight < grp::MIN_VERTEX_WEIGHT)
			{
				influence.weight = 0.0f;
			}
			vecInflu[iInfluence] = influence;
		}
		//按权重从大到小排序
		sort(vecInflu.begin(), vecInflu.end());
		if (vecInflu[grp::MAX_VERTEX_INFLUENCE - 1].weight > 0.0f)
		{
			influenceIgnored = true;
		}
		memcpy(skinVertex.influences,
				&(vecInflu[0]),
				grp::MAX_VERTEX_INFLUENCE * sizeof(grp::VertexInfluence));

		//严格保证总权重为1，这个很重要
		float fWeightTotal = 0.0f;
		for (int i = 0; i < grp::MAX_VERTEX_INFLUENCE; ++i)
		{
			fWeightTotal += skinVertex.influences[i].weight;
		}
		if (fWeightTotal > 0.0f)
		{
			for (int i = 0; i < grp::MAX_VERTEX_INFLUENCE; ++i)
			{
				skinVertex.influences[i].weight /= fWeightTotal;
			}
		}
		else
		{
			setLastError("Vertex has no bone weight.");
			return false;
		}
	
		::Vector3FromPoint3(skinnedMesh->m_positions[iVertex], mesh->verts[vertexExp.maxVertexIndex]);
		if (skinnedMesh->m_normals.size() > iVertex)
		{
			skinnedMesh->m_normals[iVertex] = vertexExp.normal;
		}
		if (skinnedMesh->m_tangents.size() > iVertex)
		{
			skinnedMesh->m_tangents[iVertex] = vertexExp.tangent;
		}
		if (skinnedMesh->m_binormals.size() > iVertex)
		{
			skinnedMesh->m_binormals[iVertex] = vertexExp.binormal;
		}

		float weightError = calcWeightError(*skinnedMesh, skinVertex, skinnedMesh->m_positions[iVertex]);
		if (weightError > skinnedMesh->m_weightLodError)
		{
			skinnedMesh->m_weightLodError = weightError;
		}

		//纹理坐标	
		for (int i = 0; i < texCoordCount; ++i)
		{
			grp::Vector2& texcoord = skinnedMesh->m_texCoordsArray[i][iVertex];
			if (i == 0)
			{
				::Vector2FromPoint3(texcoord, mesh->tVerts[vertexExp.texcoord[i]]);
			}
			else
			{
				UVVert* pUV = mesh->mapVerts(i + 1);
				::Vector2FromPoint3(texcoord, pUV[vertexExp.texcoord[i]]);
			}
			texcoord.Y = 1.0f - texcoord.Y;
			//纹理坐标超出0,1范围就不压缩
			if ((m_options.exportType & EXP_MESH_COMPRESS_TEXCOORD) != 0 &&
				(texcoord.X < 0.0f || texcoord.X > 1.0f ||	texcoord.Y < 0.0f || texcoord.Y > 1.0f))
			{
				m_options.exportType &= (~EXP_MESH_COMPRESS_TEXCOORD);
			}
		}
		//顶点颜色
		if (skinnedMesh->checkVertexFormat(grp::COLOR))
		{
			::DWORDFromPoint3(skinnedMesh->m_colors[iVertex], mesh->vertCol[vertexExp.color]);
		}
		skinVertex.copyPosition = vertexExp.copyPos;
		skinVertex.copyNormal = vertexExp.copyNormal;
	}//for (int vertexIndex = 0; vertexIndex < iNumVertexMax; ++vertexIndex)

	skinnedMesh->m_meshBuffers = buffers;

	if (influenceIgnored)
	{
		::MessageBox(NULL, "影响顶点的骨骼数超过4，超出的已被忽略。", "提示", MB_OK | MB_ICONINFORMATION);
	}
	if ((m_options.exportType & EXP_MESH_PROPERTY) != 0)
	{
		TSTR buffer;
		node->GetUserPropBuffer(buffer);
		wchar_t* unicodeString = new wchar_t[buffer.length() + 1];
		mbstowcs(unicodeString, buffer.data(), buffer.length() + 1);
		skinnedMesh->m_property = unicodeString;
		delete[] unicodeString;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
float CExporter::calcWeightError(grp::SkinnedMeshExporter& skinnedMesh, grp::SkinVertex& skinVertex, grp::Vector3& position)
{
	grp::Vector3 correctPosition(grp::Vector3::ZERO);
	grp::Vector3 lodPosition;
	for (int i = 0; i < grp::MAX_VERTEX_INFLUENCE; ++i)
	{
		grp::VertexInfluence& influence = skinVertex.influences[i];
		if (influence.weight <= 0)
		{
			break;
		}
		assert(influence.boneIndex < skinnedMesh.m_offsetMatrices.size());
		grp::Vector3 influencePos = skinnedMesh.m_offsetMatrices[influence.boneIndex].transformVector3(position);
		if (i == 0)
		{
			lodPosition = influencePos;
		}
		correctPosition += (influence.weight * influencePos);
	}
	return correctPosition.distance(lodPosition);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
ISkin* CExporter::getISkinPtr(Object* object)
{
	assert(object != NULL);
	while (GEN_DERIVOB_CLASS_ID == object->SuperClassID())
	{
		IDerivedObject* pDerObj = static_cast<IDerivedObject*>(object);

		for (int i = 0; i < pDerObj->NumModifiers(); ++i)
		{
			Modifier* mod = pDerObj->GetModifier(i);
			if (SKIN_CLASSID == mod->ClassID())
			{
				return (ISkin*)(mod->GetInterface(I_SKIN));
			}
		}
		object = pDerObj->GetObjRef();
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
Mesh* CExporter::getMeshPtr(INode* node, Object* object)
{
	assert(object != NULL);

	while (GEN_DERIVOB_CLASS_ID == object->SuperClassID())
	{
		object = ((IDerivedObject*)object)->GetObjRef();
	}

	if (GEOMOBJECT_CLASS_ID != object->SuperClassID())
	{
		return NULL;
	}

	Control* pCtrl = node->GetTMController();
	Class_ID id = pCtrl->ClassID();
	if (id == BIPSLAVE_CONTROL_CLASS_ID
		|| id == BIPBODY_CONTROL_CLASS_ID
		|| id == FOOTPRINT_CLASS_ID)
	{
		return NULL;
	}

	if (!object->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0)))
	{
		return NULL;
	}
	assert(m_interface != NULL);
	TriObject* pTriObj = (TriObject*)(object->ConvertToType(m_interface->GetTime(),
															Class_ID(TRIOBJ_CLASS_ID, 0)));
	if (NULL == pTriObj)
	{
		return NULL;
	}
	return &(pTriObj->mesh);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
DWORD CExporter::addExportVertex(std::vector<ExportVertex>& exportVertices,
								  bool* aReferenced,
								  int maxVertexIndex,				//max顶点索引
								  int smoothGroup,
								  const grp::Vector3& position,
								  const grp::Vector3& normal,
								  const grp::Vector3& tangent,
								  const grp::Vector3& binormal,
								  const int* texcoords,
								  int texCoordCount,
								  int colorIndex)
{
	if (!aReferenced[maxVertexIndex])
	{
		//对应的max顶点第一次出现
		ExportVertex& vertex = exportVertices[maxVertexIndex];
		aReferenced[maxVertexIndex] = true;
		vertex.maxVertexIndex = maxVertexIndex;
		vertex.smoothGroup = smoothGroup;
		vertex.color = colorIndex;
		vertex.position = position;
		vertex.normal = normal;
		vertex.tangent = tangent;
		vertex.binormal = binormal;
		assert(texCoordCount <= 2);
		memcpy(vertex.texcoord, texcoords, sizeof(int) * texCoordCount);
		return static_cast<DWORD>(maxVertexIndex);
	}
	//查找原有顶点
	ExportVertex* curVertex = NULL;
	grp::Vector3 tangentTotal = tangent;
	grp::Vector3 binormalTotal = binormal;
	int copyPos = maxVertexIndex;
	int copyNormal = -1;
	int copyTangent = -1;
	int same = -1;
	for (int iCurVertex = maxVertexIndex; iCurVertex >= 0; iCurVertex = curVertex->nextIndex)
	{
		assert(iCurVertex < exportVertices.size());
		curVertex = &(exportVertices[iCurVertex]);
		if (curVertex->smoothGroup != smoothGroup && (m_options.exportType & EXP_MESH_NORMAL) !=  0)
		{	//不导出法线的话就不用考虑平滑组了
			continue;
		}

		if (copyNormal < 0)
		{
			copyNormal = iCurVertex;
		}
		if (texCoordCount <= 0 || (m_options.exportType & EXP_MESH_TEXCOORD) == 0)
		{
			//没有纹理坐标，不用考虑tangent和binormal
			if (same < 0 &&
				(curVertex->color == colorIndex || (m_options.exportType & EXP_MESH_VERTEX_COLOR) == 0))
			{
				same = iCurVertex;
			}
			continue;
		}
		//10-04-30纹理坐标不同也共享tangent和binormal，否则法线空间可能不连续
		//if (curVertex->texcoord[0] != texcoords[0])
		//{
		//	continue;
		//}
		//第一层纹理坐标相同的顶点共享tangent和binormal
		tangentTotal  += curVertex->tangent;
		binormalTotal += curVertex->binormal;
		curVertex->tangent  += tangent;
		curVertex->binormal += binormal;
		if (copyTangent < 0)
		{
			copyTangent = iCurVertex;
		}
		if (same < 0 
			&& ((m_options.exportType & EXP_MESH_VERTEX_COLOR) == 0 || curVertex->color == colorIndex)
			&& 0 == memcmp(curVertex->texcoord, texcoords, sizeof(int) * texCoordCount))	//走到这里一定是需要导出纹理坐标的
		{
			same = iCurVertex;
		}
	}
	if (same >= 0)
	{
		return static_cast<DWORD>(same);
	}
	//创建新顶点
	ExportVertex vertexNew;
	vertexNew.maxVertexIndex = maxVertexIndex;
	vertexNew.color = colorIndex;
	vertexNew.smoothGroup = smoothGroup;
	vertexNew.position = position;
	vertexNew.normal = normal;
	vertexNew.tangent = tangentTotal;
	vertexNew.binormal = binormalTotal;
	vertexNew.copyPos = copyPos;
	vertexNew.copyNormal = copyNormal;
	memcpy(vertexNew.texcoord, texcoords, sizeof(int) * texCoordCount);

	DWORD newIndex = exportVertices.size();
	assert(curVertex->nextIndex < 0);
	curVertex->nextIndex = newIndex;

	exportVertices.push_back(vertexNew);

	return newIndex;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CExporter::checkMeshExpType()
{
	//如果没有纹理坐标或没有法线那么也不需要切线
	if ((m_options.exportType & EXP_MESH_TANGENT) != 0
		&& ((m_options.exportType & EXP_MESH_TEXCOORD) == 0 || (m_options.exportType & EXP_MESH_NORMAL) == 0))
	{
		m_options.exportType &= (~EXP_MESH_TANGENT);
	}
}

BOOL GetVertexNormalUsingSmoothGroup(Point3& VN, Mesh& mesh, int faceId, int globalvertexId, int _FaceVertexIdx)
{
    //_FaceVertexIdx is between 0 and 2 local to a triangle

    //THIS IS WHAT YOU NEED IF SOMEONE HAS USED THE EDIT NORMAL MODIFIER
    MeshNormalSpec * normalspec = mesh.GetSpecifiedNormals();
    if (normalspec)
    {
        const int NumFaces      = normalspec->GetNumFaces   ();
        const int NumNormals   = normalspec->GetNumNormals   ();

        if (NumFaces && NumNormals)
        {
            const int normID = normalspec->Face(faceId).GetNormalID(_FaceVertexIdx);
            VN = normalspec->Normal(normID).Normalize();
            return TRUE;
        }
    }

    // get the "rendered" vertex
    RVertex *pRVertex = mesh.getRVertPtr(globalvertexId);
    if(! pRVertex)return FALSE;

    // get the face
    const Face& Face = mesh.faces[faceId];

    // get the smoothing group of the face
    const DWORD smGroup = Face.smGroup;

    // get the number of normals
    const int normalCount = pRVertex->rFlags & NORCT_MASK;

    // check if the normal is specified ...
    if(pRVertex->rFlags & SPECIFIED_NORMAL)
    {
        VN = pRVertex->rn.getNormal();
        return TRUE;
    }
    // ... otherwise, check for a smoothing group
    else if((normalCount > 0) && (smGroup != 0))
    {
        // If there is only one vertex is found in the rn member.
        if(normalCount == 1)
        {
            VN = pRVertex->rn.getNormal();
            return TRUE;
        }
        else
        {
            for(int normalId = 0; normalId < normalCount; normalId++)
            {
                if(pRVertex->ern[normalId].getSmGroup() & smGroup)
                {
                    VN = pRVertex->ern[normalId].getNormal();
                    return TRUE;
                }
            }
        }
    }

    // if all failed, return the face normal
    VN = mesh.getFaceNormal(faceId);
    return TRUE;
} 