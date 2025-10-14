#include "UltimateEnginePCH.h"
#include "ModelLoader.h"

//-------------------------------------------------------------------------------------------------------------------
ModelLoader::ModelLoader()
{
	m_ListVertices.clear();
	m_ListIndices.clear();
}

//-------------------------------------------------------------------------------------------------------------------
ModelLoader::~ModelLoader()
{
}

//-------------------------------------------------------------------------------------------------------------------
void ModelLoader::LoadModel(const std::string& path, std::vector<UT::D3D12::DAS::VertexPNBT>& outVertices, std::vector<uint16_t>& outIndices)
{
	// Convert to the full path & widestring before passing it to the D3D function!
	const std::string filePath = UT::GLOBALS::GetExecutableFolderPath() + path;

	Assimp::Importer importer;
	const aiScene* pScene = importer.ReadFile(filePath.c_str(), aiProcess_Triangulate | aiProcess_JoinIdenticalVertices);

	if (!pScene || pScene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !pScene->mRootNode)
	{
		LOG_ERROR("{0} Mesh Reading Failed!", path.c_str());
		return;
	}

	// process root node recursively!
	ProcessNode(pScene->mRootNode, pScene);

	// send out ver
	outVertices = m_ListVertices;
	outIndices = m_ListIndices;
}

//-------------------------------------------------------------------------------------------------------------------
void ModelLoader::ProcessNode(aiNode* node, const aiScene* scene)
{
    // node only contains indices to actual objects in the scene. But scene,
    // conatins all the data, node is just to keep things organized.
    for (unsigned int i = 0; i < node->mNumMeshes; ++i)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        ProcessMesh(mesh, scene);
    }

    // Once we have processed all the meshes, we recursively process
    // each child node
    for (unsigned int i = 0; i < node->mNumChildren; ++i)
    {
        ProcessNode(node->mChildren[i], scene);
    }
}

//-------------------------------------------------------------------------------------------------------------------
void ModelLoader::ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		UT::D3D12::DAS::VertexPNBT vertex;

		vertex.Position = XMFLOAT3(mesh->mVertices[i][0], mesh->mVertices[i][1], mesh->mVertices[i][2]);
		vertex.Normal = XMFLOAT3(mesh->mNormals[i][0], mesh->mNormals[i][1], mesh->mNormals[i][2]);
		
		if (mesh->mTextureCoords[0])
		{
			vertex.TexCoord = XMFLOAT2(mesh->mTextureCoords[0][i][0], mesh->mTextureCoords[0][i][1]);
		}

		if(mesh->HasTangentsAndBitangents())
		{
			vertex.BiNormal = XMFLOAT3(mesh->mBitangents[i][0], mesh->mBitangents[i][1], mesh->mBitangents[i][2]);
		}

		m_ListVertices.push_back(vertex);
	}

	// process materials
	//if (mesh->mMaterialIndex >= 0)
	//{
	//	aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
	//	aiString aiName;
	//
	//	if (AI_SUCCESS == aiGetMaterialString(material, AI_MATKEY_NAME, &aiName))
	//	{
	//		std::string name = aiName.C_Str();
	//
	//		if (name.find("lambert") != std::string::npos)
	//		{
	//			// Extract texture info if filepath or flat color?
	//			Texture* textureInfo = nullptr;
	//
	//			// Look if material has texture info...
	//			aiString path;
	//			if (AI_SUCCESS == aiGetMaterialTexture(material, aiTextureType_DIFFUSE, 0, &path))
	//			{
	//				std::string filePath = std::string(path.C_Str());
	//				textureInfo = new ImageTexture("models/" + filePath);
	//				m_ptrMaterial = new Lambertian(textureInfo);
	//			}
	//			else
	//			{
	//				// Check if we have set albedo color explicitly or not, if not then use Maya's 
	//				// set color from the properties!
	//				glm::vec4 albedoCol = m_ptrMeshInfo->matInfo.albedoColor;
	//				if (glm::length(albedoCol) == 0)
	//				{
	//					aiColor4D diffuseColor;
	//					aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &diffuseColor);
	//					albedoCol = glm::vec4(diffuseColor.r, diffuseColor.g, diffuseColor.b, diffuseColor.a);
	//				}
	//
	//				if (m_ptrMeshInfo->isLightSource)
	//				{
	//					textureInfo = new ConstantTexture(albedoCol);
	//					m_ptrMaterial = new DiffuseLight(textureInfo);
	//				}
	//				else
	//				{
	//					textureInfo = new ConstantTexture(albedoCol);
	//					m_ptrMaterial = new Lambertian(textureInfo);
	//				}
	//			}
	//		}
	//		else if (name.find("metal") != std::string::npos)
	//		{
	//			// Extract texture info if filepath or flat color?
	//			Texture* textureInfo = nullptr;
	//			float roughness = m_ptrMeshInfo->matInfo.roughness;
	//
	//			// Look if material has texture info...
	//			aiString path;
	//			if (AI_SUCCESS == aiGetMaterialTexture(material, aiTextureType_DIFFUSE, 0, &path))
	//			{
	//				std::string filePath = std::string(path.C_Str());
	//				textureInfo = new ImageTexture("models/" + filePath);
	//				m_ptrMaterial = new Metal(textureInfo, roughness);
	//			}
	//			else
	//			{
	//				// Check if we have set albedo color explicitly or not, if not then use Maya's 
	//				// set color from the properties!
	//				glm::vec4 albedoCol = m_ptrMeshInfo->matInfo.albedoColor;
	//				if (glm::length(albedoCol) == 0)
	//				{
	//					aiColor4D diffuseColor;
	//					aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &diffuseColor);
	//					albedoCol = glm::vec4(diffuseColor.r, diffuseColor.g, diffuseColor.b, diffuseColor.a);
	//				}
	//
	//				textureInfo = new ConstantTexture(albedoCol);
	//				m_ptrMaterial = new Metal(textureInfo, roughness);
	//			}
	//		}
	//		else if (name.find("transparent") != std::string::npos)
	//		{
	//			// Extract texture info if filepath or flat color?
	//			Texture* textureInfo = nullptr;
	//			float r_i = m_ptrMeshInfo->matInfo.refrIndex;
	//
	//			// Check if we have set albedo color explicitly or not, if not then use Maya's 
	//			// set color from the properties!
	//			glm::vec4 albedoCol = m_ptrMeshInfo->matInfo.albedoColor;
	//			if (glm::length(albedoCol) == 0)
	//			{
	//				aiColor4D diffuseColor;
	//				aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &diffuseColor);
	//				albedoCol = glm::vec4(diffuseColor.r, diffuseColor.g, diffuseColor.b, diffuseColor.a);
	//			}
	//
	//			textureInfo = new ConstantTexture(albedoCol);
	//			m_ptrMaterial = new Transparent(textureInfo, r_i);
	//		}
	//		else
	//		{
	//			MessageBox(0, L"Unknown Material", L"Error", MB_OK);
	//			return;
	//		}
	//	}
	//}

	// Gets called per triangle primitive!
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		// Get a face
		const aiFace face = mesh->mFaces[i];

		// go through face's indices & add to the list
		for (uint16_t j = 0; j < face.mNumIndices; j++)
		{
			m_ListIndices.push_back(face.mIndices[j]);
		}
	}
}

