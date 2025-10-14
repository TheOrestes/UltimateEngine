#pragma once

#include "D3D12Renderer/D3DGlobals.h"

#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

class D3DMesh;

class ModelLoader
{
public:
	ModelLoader();
	~ModelLoader();

	void									LoadModel(const std::string& path, std::vector<UT::D3D12::DAS::VertexPNBT>& outVertices, std::vector<uint16_t>& outIndices);

private:
	// Actual vertices& indices data
	std::vector<UT::D3D12::DAS::VertexPNBT>	m_ListVertices;
	std::vector<uint16_t>					m_ListIndices;
	std::string								m_strFilaPath;

	void									ProcessNode(aiNode* node, const aiScene* scene);
	void									ProcessMesh(aiMesh* mesh, const aiScene* scene);
};

