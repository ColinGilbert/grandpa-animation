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
#include "SlimXml.h"

bool CExporter::ExportPart()
{
    for (DWORD i = 0; i < m_meshes.size(); ++i)
    {
        grp::MeshExporter* mesh = m_meshes[i];

        slim::XmlDocument xmlDoc;

        slim::XmlNode* partNode = xmlDoc.addChild( L"part" );
        
        partNode->addAttribute( L"mesh", (m_mainFileName + L"_" + mesh->getName() + FILE_EXT_MESH_SKIN).c_str());
        partNode->addAttribute( L"type", mesh->getTypeDesc().c_str());

        //节点的材质
        
        Mtl* material = m_meshNodes[i]->GetMtl();   //一一对应
        if( material )
        {
            int subMaterialCount = material->NumSubMtls();

            if( subMaterialCount == 0 )
            {
                //单一材质
                slim::XmlNode* materialNode = partNode->addChild( L"material" );
                materialNode->addAttribute( L"filename", (m_mainFileName + L"_" + mesh->getName() + FILE_EXT_MATERIAL).c_str());
            }
            else
            {
                //多材质
                for( int i = 0; i < subMaterialCount; ++i )
                {
                    slim::XmlNode* materialNode = partNode->addChild( L"material" );
                    std::wstringstream wss;
                    wss << m_mainFileName << L'_' << mesh->getName() << + L"_" << i << FILE_EXT_MATERIAL;
                    materialNode->addAttribute( L"filename", wss.str().c_str());
                }
            }
        }

        //节点对应的文件名
        std::wstring strFilePath = m_exportPath + m_mainFileName;
        strFilePath += L"_";
        strFilePath += mesh->getName();
        strFilePath += FILE_EXT_PART;
        if( !xmlDoc.save( strFilePath.c_str()))
        {
            return false;
        }
    }
    return true;
}