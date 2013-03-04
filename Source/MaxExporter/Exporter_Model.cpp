///////////////////////////////////////////////////////////////////////////////////////////////////
//Exporter.cpp
//
//描述：
//		Material相关导出代码
//
//历史：
//		2012-6-24	创建
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "Precompiled.h"
#include "Exporter.h"

#include <sstream>
#include <strsafe.h>

#include "MeshExporter.h"
#pragma warning(disable : 4995)
#include "SlimXml.h"


bool CExporter::ExportModel()
{
    slim::XmlDocument xmlDoc;

    slim::XmlNode* modelNode = xmlDoc.addChild( L"model" );
    if( !m_skeletonBoneNodes.empty())
    {
        modelNode->addAttribute( L"skeleton", (m_mainFileName + FILE_EXT_SKELETON ).c_str());
    }

    for (DWORD i = 0; i < m_meshes.size(); ++i)
    {
        grp::MeshExporter* mesh = m_meshes[i];

        slim::XmlNode* partNode = modelNode->addChild( L"part" );
        partNode->addAttribute( L"slot", mesh->getName().c_str());
        partNode->addAttribute( L"filename", (m_mainFileName + L"_" + mesh->getName() + FILE_EXT_PART).c_str());
    }

    if( !xmlDoc.save((m_exportPath + m_mainFileName + FILE_EXT_MODEL).c_str()))
    {
        return false;
    }
    return true;
}