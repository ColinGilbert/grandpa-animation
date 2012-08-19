///////////////////////////////////////////////////////////////////////////////
//Lod计算
//
//描述：
//		仅根据顶点位置进行lod，没有考虑纹理坐标，法线等其他因素
//
//历史：
//		08-01-24	创建
///////////////////////////////////////////////////////////////////////////////
#ifndef __LOD_GENERATOR_H__
#define __LOD_GENERATOR_H__

#include <vector>
#include <set>

class Mesh;
struct ExportVertex;

namespace grp
{
struct LodIndices;
}

class LodGenerator
{
public:
	LodGenerator( const std::vector<ExportVertex>& exportVertices, std::vector<grp::LodIndices>& buffer );

	//lodScale		lod系数，也就是每级别的顶点数相对于上一级的削减比例
	//return		生成的lod级别数
	void calculate( float errorTolerance, float lodScale = 0.4f );

public:
	struct LodVertex
	{
		LodVertex()
			:	lodIndex( -1 )
			,	collapseIndex( -1 )
			,	error( 0.0f )
		{}
		int				lodIndex;		//被lod的顺序，越大越先丢弃
		int				collapseIndex;	//塌陷目标顶点index
		float			error;			//塌陷引起的误差
		std::set<int>	neighbors;		//相邻的顶点
		std::set<int>	faces;			//所在的三角形
	};

	struct LodFace
	{
		LodFace()
			:	collapsed( false )
			,	isBorder( false )
		{}
		bool	collapsed;		//是否已塌陷
		int		vertices[3];	//顶点
		bool	isBorder;		//是否边界三角形。如果三角形有至少一个不和其它任何三角形共享的边则这个三角形为边界
	};

public:
	const std::vector<ExportVertex>&	m_exportVertices;
	std::vector<grp::LodIndices>&		m_buffer;
	std::vector<LodVertex>				m_lodVertices;
	std::vector<LodFace>				m_lodFaces;

private:
	void findBorder();

	void findBorderForVertex( size_t vertexIndex );

	void calculateVertexCollapse();

	float calculateCollapseError( int iVertex, int iCollapse );

	int whoIsNext( float& error );

	void collapseVertex( int iVertex, int iLodIndex );

	bool generateLodIndices( float maxError );

	int getCollapseVertex( int iVertex );
};

///////////////////////////////////////////////////////////////////////////////
inline int LodGenerator::getCollapseVertex( int iVertex )
{
	while ( m_lodVertices[iVertex].lodIndex >= 0 
		&& m_lodVertices[iVertex].collapseIndex >= 0 )
	{
		iVertex = m_lodVertices[iVertex].collapseIndex;
	}
	return iVertex;
}

#endif