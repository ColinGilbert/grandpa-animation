///////////////////////////////////////////////////////////////////////////////////////////////////
//Lod计算
//
//描述：
//
//历史：
//		08-01-24	创建
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "Precompiled.h"
#include "Exporter.h"
#include "LodGenerator.h"
#include "MeshExporter.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
LodGenerator::LodGenerator(const std::vector<ExportVertex>& exportVertices, std::vector<grp::LodIndices>& buffer)
	:	m_exportVertices(exportVertices)
	,	m_buffer(buffer)
{
	assert(buffer.size() == 1);

	const std::vector<grp::Index32>& indices = buffer[0].indices;
	assert(indices.size() % 3 == 0);
	size_t faceCount = indices.size() / 3;

	m_lodVertices.resize(exportVertices.size());
	m_lodFaces.resize(faceCount);

	for (size_t faceIndex = 0; faceIndex < faceCount; ++faceIndex)
	{
		DWORD v0 = indices[faceIndex*3];
		DWORD v1 = indices[faceIndex*3+1];
		DWORD v2 = indices[faceIndex*3+2];
		assert(v0 < exportVertices.size());
		assert(v1 < exportVertices.size());
		assert(v2 < exportVertices.size());
		m_lodVertices[v0].neighbors.insert(v1);
		m_lodVertices[v1].neighbors.insert(v0);
		m_lodVertices[v0].neighbors.insert(v2);
		m_lodVertices[v2].neighbors.insert(v0);
		m_lodVertices[v1].neighbors.insert(v2);
		m_lodVertices[v2].neighbors.insert(v1);

		m_lodVertices[v0].faces.insert(faceIndex);
		m_lodVertices[v1].faces.insert(faceIndex);
		m_lodVertices[v2].faces.insert(faceIndex);

		m_lodFaces[faceIndex].vertices[0] = v0;
		m_lodFaces[faceIndex].vertices[1] = v1;
		m_lodFaces[faceIndex].vertices[2] = v2;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void LodGenerator::calculate(float errorTolerance, float lodScale)
{
	findBorder();

	//计算每个顶点的最佳塌陷方向以及相应的误差
	calculateVertexCollapse();

	size_t validVertexCount = 0;
	for (size_t i = 0; i < m_lodVertices.size(); ++i)
	{
		if (!m_lodVertices[i].faces.empty())
		{
			++validVertexCount;
		}
	}

	int lodIndex = validVertexCount - 1;

	int numVertexLeft = validVertexCount;
	float maxError = 0.0f;

	bool stop = false;
	while (!stop)
	{
		int numVertexToKill = static_cast<int>(numVertexLeft * lodScale);
		if (numVertexToKill <= 0)
		{
			break;
		}
		int numVertexKilled = 0;

		while (numVertexKilled < numVertexToKill)
		{
			float error;
			int collapse = whoIsNext(error);
			if (error > errorTolerance)
			{
				//误差超过设定值
				stop = true;
				break;
			}
			if (error > maxError)
			{
				maxError = error;
			}
			collapseVertex(collapse, lodIndex);
			--lodIndex;
			assert(lodIndex >= 0);
			++numVertexKilled;
		}
		numVertexLeft -= numVertexKilled;
		if (numVertexKilled < numVertexToKill / 2)
		{
			//防止最后两级的顶点数太接近
			break;
		}
		//整理该级别的IndexBuffer
		if (!generateLodIndices(maxError))
		{
			break;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void LodGenerator::findBorder()
{
	for (size_t vertexIndex = 0; vertexIndex < m_lodVertices.size(); ++vertexIndex)
	{
		findBorderForVertex(vertexIndex);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void LodGenerator::findBorderForVertex(size_t vertexIndex)
{
	LodVertex& vertex = m_lodVertices[vertexIndex];
	if (vertex.faces.empty() || vertex.neighbors.empty() || vertex.lodIndex >= 0)
	{
		return;
	}
	for (std::set<int>::iterator iterNeighbor = vertex.neighbors.begin();
		iterNeighbor != vertex.neighbors.end();
		++iterNeighbor)
	{
		int iFaceOn = 0;
		int borderFace;

		for (std::set<int>::iterator iterFace = vertex.faces.begin();
			iterFace != vertex.faces.end();
			++iterFace)
		{
			LodFace& face = m_lodFaces[*iterFace];
			if (face.vertices[0] == *iterNeighbor ||
				face.vertices[1] == *iterNeighbor ||
				face.vertices[2] == *iterNeighbor)
			{
				++iFaceOn;
				borderFace = *iterFace;
			}
		}
		if (iFaceOn == 1)
		{
			m_lodFaces[borderFace].isBorder = true;
			break;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void LodGenerator::calculateVertexCollapse()
{
	for (size_t vertexIndex = 0; vertexIndex < m_lodVertices.size(); ++vertexIndex)
	{
		LodVertex& vertex = m_lodVertices[vertexIndex];
		if (vertex.faces.empty() || vertex.lodIndex >= 0)	//不在任何三角形中（可能属于别的材质），或者已塌陷，不用计算
		{
			continue;
		}
		float minError = FLT_MAX;
		int	minErrorNeighbor = -1;
		for (std::set<int>::iterator iterNeighbor = vertex.neighbors.begin();
			  iterNeighbor != vertex.neighbors.end();
			  ++iterNeighbor)
		{
			assert(m_lodVertices[*iterNeighbor].lodIndex < 0);
			float error = calculateCollapseError(vertexIndex, *iterNeighbor);
			if (error < minError)
			{
				minError = error;
				minErrorNeighbor = *iterNeighbor;
			}
		}
		if (minErrorNeighbor >= 0)
		{
			vertex.collapseIndex = minErrorNeighbor;
			vertex.error = minError;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
float LodGenerator::calculateCollapseError(int vertexIndex, int collapse)
{
	LodVertex& src = m_lodVertices[vertexIndex];
	LodVertex& dst = m_lodVertices[collapse];

	if (src.faces.empty() || dst.faces.empty())
	{
		//这是可能的
		return 0.0f;
	}
	assert(src.lodIndex < 0 && dst.lodIndex < 0);
	assert(!src.neighbors.empty() && !dst.neighbors.empty());

	//移动矢量
	grp::Vector3 move = m_exportVertices[collapse].position - m_exportVertices[vertexIndex].position;
	float maxError = 0.0f;
	for (std::set<int>::iterator iterFace = src.faces.begin();
		iterFace != src.faces.end();
		++iterFace)
	{
		float error = 0.0f;
		LodFace& face = m_lodFaces[*iterFace];
		if (face.isBorder)
		{
			//what to do
			//temp
			error = move.length();
		}
		else
		{
			grp::Vector3 edge[2];
			int currentEdge = 0;
			for (int i = 0; i < 3; ++i)
			{
				if (face.vertices[i] != vertexIndex && face.vertices[i] != collapse && currentEdge < 2)
				{
					edge[currentEdge++] = m_exportVertices[collapse].position - m_exportVertices[face.vertices[i]].position;
				}
			}
			if (currentEdge == 2)
			{
				grp::Vector3 newNormal = edge[0].cross(edge[1]);
				newNormal.normalize();
				error = fabs(newNormal.dot(move));
			}
		}
		if (error > maxError)
		{
			maxError = error;
		}
	}
	return maxError;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int LodGenerator::whoIsNext(float& error)
{
	float minError = FLT_MAX;
	int minErrorVertex = -1;
	for (unsigned int vertexIndex = 0; vertexIndex < m_lodVertices.size(); ++vertexIndex)
	{
		LodVertex& vertex = m_lodVertices[vertexIndex];
		if (vertex.lodIndex < 0	//说明还健在
			&& !vertex.faces.empty()
			&& vertex.error < minError)
		{
			minError = vertex.error;
			minErrorVertex = vertexIndex;
		}
	}
	error = minError;
	return minErrorVertex;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void LodGenerator::collapseVertex(int vertexIndex, int lodIndex)
{
	LodVertex& vertex = m_lodVertices[vertexIndex];
	vertex.lodIndex = lodIndex;
	
	assert(vertex.collapseIndex >= 0);

	//删除塌陷的三角形
	for (std::set<int>::iterator iterFace = vertex.faces.begin();
		  iterFace != vertex.faces.end();)
	{
		int faceIndex = *iterFace;
		assert(faceIndex < m_lodFaces.size());
		LodFace& face = m_lodFaces[faceIndex];

		if (face.vertices[0] == vertex.collapseIndex
			|| face.vertices[1] == vertex.collapseIndex
			|| face.vertices[2] == vertex.collapseIndex)
		{
			face.collapsed = true;
			//从所有顶点中删除
			m_lodVertices[face.vertices[0]].faces.erase(faceIndex);
			m_lodVertices[face.vertices[1]].faces.erase(faceIndex);
			m_lodVertices[face.vertices[2]].faces.erase(faceIndex);
			iterFace = vertex.faces.begin();
		}
		else
		{
			++iterFace;
		}
	}

	//将剩余三角形里记录的顶点替换成新顶点
	for (std::set<int>::iterator iterFace = vertex.faces.begin();
		  iterFace != vertex.faces.end();
		  ++iterFace)
	{
		assert(*iterFace < m_lodFaces.size());
		LodFace& face = m_lodFaces[*iterFace];
		if (face.vertices[0] == vertexIndex)
		{
			face.vertices[0] = vertex.collapseIndex;
		}
		if (face.vertices[1] == vertexIndex)
		{
			face.vertices[1] = vertex.collapseIndex;
		}
		if (face.vertices[2] == vertexIndex)
		{
			face.vertices[2] = vertex.collapseIndex;
		}
		//向新顶点添加三角形
		assert(vertex.collapseIndex < m_lodVertices.size());
		m_lodVertices[vertex.collapseIndex].faces.insert(*iterFace);
	}

	vertex.faces.clear();

	//从所有邻居的邻居中把自己替换成新顶点，并把所有邻居加为新顶点的邻居
	for (std::set<int>::iterator iterNeighbor = vertex.neighbors.begin();
		  iterNeighbor != vertex.neighbors.end();
		  ++iterNeighbor)
	{		
		LodVertex& neighbor = m_lodVertices[*iterNeighbor];
		if (neighbor.neighbors.erase(vertexIndex) > 0)
		{
			if (*iterNeighbor != vertex.collapseIndex)
			{
				neighbor.neighbors.insert(vertex.collapseIndex);
				m_lodVertices[vertex.collapseIndex].neighbors.insert(*iterNeighbor);
			}
		}
		else
		{
			assert(false);
		}
	}

	//重新计算塌陷误差以及是否边界
	for (std::set<int>::iterator iterModified = vertex.neighbors.begin();
		iterModified != vertex.neighbors.end();
		++iterModified)
	{
		findBorderForVertex(*iterModified);
	}
	for (std::set<int>::iterator iterModified = vertex.neighbors.begin();
		iterModified != vertex.neighbors.end();
		++iterModified)
	{
		LodVertex& neighbor = m_lodVertices[*iterModified];

		float minError = FLT_MAX;
		int	minErrorNeighbor = -1;
		for (std::set<int>::iterator iterNeighbor = neighbor.neighbors.begin();
			iterNeighbor != neighbor.neighbors.end();
			++iterNeighbor)
		{
			assert(m_lodVertices[*iterNeighbor].lodIndex < 0);
			float error = calculateCollapseError(*iterModified, *iterNeighbor);
			if (error < minError)
			{
				minError = error;
				minErrorNeighbor = *iterNeighbor;
			}
		}
		if (minErrorNeighbor >= 0)
		{
			neighbor.collapseIndex = minErrorNeighbor;
			neighbor.error = minError;
		}
	}
	vertex.neighbors.clear();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//整理当前lod级别的indexbuffer
//此时还是对应原始的顶点顺序
bool LodGenerator::generateLodIndices(float maxError)
{
	grp::LodIndices lodIndices;
	lodIndices.maxError = maxError;

	for (int faceIndex = 0; faceIndex < m_lodFaces.size(); ++faceIndex)
	{
		LodFace& face = m_lodFaces[faceIndex];
		if (face.collapsed)
		{
			continue;
		}
		int v0 = getCollapseVertex(face.vertices[0]);
		int v1 = getCollapseVertex(face.vertices[1]);
		int v2 = getCollapseVertex(face.vertices[2]);
		lodIndices.indices.push_back(static_cast<grp::Index32>(v0));
		lodIndices.indices.push_back(static_cast<grp::Index32>(v1));
		lodIndices.indices.push_back(static_cast<grp::Index32>(v2));
	}
	if (lodIndices.indices.empty())
	{
		return false;
	}
	m_buffer.push_back(lodIndices);
	return true;
}
