///////////////////////////////////////////////////////////////////////////////////////////////////
//Exporter.cpp
//
//描述：
//		Material相关导出代码
//
//历史：
//		2012-6-22	创建
///////////////////////////////////////////////////////////////////////////////////////////////////
#include "Precompiled.h"
#include "Exporter.h"

#include <sstream>
#include <strsafe.h>
#pragma warning(disable : 4995)
#include "SlimXml.h"


bool CExporter::ExportMaterial( const std::wstring& strFilename )
{
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

        //节点对应的文件名
        std::wstring strFilePath = strFilename;
        if (m_meshes.size() > 1)
        {
            strFilePath += L"_";

            wchar_t unicodeString[256];
            mbstowcs(unicodeString, node->GetName(), 255);
            strFilePath += unicodeString;
        }

        //节点的材质
        Mtl* material = node->GetMtl();

        if( !material )
        {
            //没有材质
            continue;
        }

        int subMaterialCount = material->NumSubMtls();

        //如果只有一个材质,那么直接使用material
        if( subMaterialCount == 0 )
        {
            strFilePath += FILE_EXT_MATERIAL;
            writeMaterialFile( material, strFilePath );
        }
        else
        {
            //迭代所有子材质
            for( int i = 0; i < subMaterialCount; ++i )
            {
                Mtl* subMtl = material->GetSubMtl( i );
                assert( subMtl );

                std::wstringstream wss;
                wss << strFilePath << L"_" << i << FILE_EXT_MATERIAL;

                writeMaterialFile( subMtl, wss.str());
            }
        }

    }

    return true;
}


bool CExporter::writeMaterialFile( Mtl* material, const std::wstring& strFilename )
{
    //构建xml文档

    slim::XmlDocument xmlDoc;
    slim::XmlNode* modelNode = xmlDoc.addChild( L"material" );

    std::string materilaName = material->GetName();
    std::wstring materilaNameW = strtowstr( materilaName );
    modelNode->addAttribute( L"type", materilaNameW.c_str());

    //纹理
    for( int textureIndex = ID_DI; textureIndex <= ID_DP; ++ textureIndex )
    {
        Texmap* texture = material->GetSubTexmap( textureIndex );

        if( !texture || ( texture->ClassID() != Class_ID( BMTEX_CLASS_ID, 0 )))
        {
            continue;
        }

        BitmapTex* bmpTexture = (BitmapTex *)texture;

        slim::XmlNode* textureNode = modelNode->addChild( L"texture" );

        std::string textureName = bmpTexture->GetMapName();
        std::wstring textureNameW = strtowstr( textureName ); // 转码
        std::wstring textureFilename = getFilenameFromFullpath( textureNameW );   //去掉路径
        textureNode->addAttribute( L"filename", textureFilename.c_str());
    }

    //光照参数
    std::wstringstream wss;

    slim::XmlNode* ambientProperty = modelNode->addChild( L"property" );
    Color ambient = material->GetAmbient();
    wss << ambient.r << L',' << ambient.g << L',' << ambient.b;
    ambientProperty->addAttribute( L"ambient", wss.str().c_str());
    wss.str( L"" );
    wss.clear();

    slim::XmlNode* diffuseProperty = modelNode->addChild( L"property" );
    Color diffuse = material->GetDiffuse();
    wss << diffuse.r << L',' << diffuse.g << L',' << diffuse.b;
    diffuseProperty->addAttribute( L"diffuse", wss.str().c_str());
    wss.str( L"" );
    wss.clear();

    slim::XmlNode* specularProperty = modelNode->addChild( L"property" );
    Color specular = material->GetSpecular();
    wss << specular.r << L',' << specular.g << L',' << specular.b;
    specularProperty->addAttribute( L"specular", wss.str().c_str());
    wss.str( L"" );
    wss.clear();

    slim::XmlNode* shininessProperty = modelNode->addChild( L"property" );
    float shininess = material->GetShininess();
    shininessProperty->addAttribute( L"shininess", shininess );

    xmlDoc.save( strFilename.c_str());
    return true;
}

std::wstring CExporter::strtowstr( const std::string& str )
{
    wchar_t unicodeString[512];
    mbstowcs(unicodeString, str.c_str(), 510);
    return unicodeString;
}

std::wstring CExporter::getFilePath(const std::wstring& fileFullPath)
{
    std::wstring path;

    size_t slashPos = fileFullPath.find_last_of( L'/' );
    size_t backSlashPos = fileFullPath.find_last_of( L'\\' );
    if (slashPos == std::wstring::npos)
    {
        if (backSlashPos == std::wstring::npos)
        {
            path = L"";
        }
        else
        {
            path = fileFullPath.substr(0, backSlashPos + 1);
        }
    }
    else
    {
        if (backSlashPos == std::wstring::npos)
        {
            path = fileFullPath.substr(0, slashPos + 1);
        }
        else
        {
            path = fileFullPath.substr(0, std::max(slashPos, backSlashPos) + 1);
        }
    }

    return path;
}

std::wstring CExporter::getFilenameFromFullpath( const std::wstring& filepath )
{
    std::wstring fullPath = filepath;

    size_t slashPos = fullPath.find_last_of( L'/');
    size_t backSlashPos = fullPath.find_last_of( L'\\' );
    std::wstring retFilename;
    if (slashPos == std::wstring::npos)
    {
        if (backSlashPos == std::wstring::npos)
        {
            retFilename = fullPath;
        }
        else
        {
            retFilename = fullPath.substr( backSlashPos + 1, fullPath.size());
        }
    }
    else
    {
        if (backSlashPos == std::wstring::npos)
        {
            retFilename = fullPath.substr( slashPos + 1, fullPath.size());
        }
        else
        {
            retFilename = fullPath.substr( std::max(slashPos, backSlashPos) + 1, fullPath.size());
        }
    }
    return retFilename;
}