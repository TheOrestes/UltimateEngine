#pragma once

#include "../VulkanRenderer/VulkanGlobals.h"
#include "VulkanMesh.h"

class GameObject;
class VulkanDevice;
struct VulkanMeshData;
class VulkanMaterial;
class Camera;

class UT_API VulkanCube : public GameObject
{
public:
	VulkanCube(const std::string& name, const glm::vec4 color);
	//VulkanCube(const glm::vec3& color);

	~VulkanCube() override;

	virtual bool						Initialize(const void* pDevice) override;
	void								Render(const VulkanDevice* pDevice, uint32_t index) const;
	void								Update(const Camera* pCamera, float dt) const;
	void								UpdateUniforms(vk::Device vkDevice, uint32_t imageIndex) const;
	void								Cleanup(void* pDevice);
	void								CleanupOnWindowsResize(VulkanDevice* pDevice);

private:
	bool								SetupDescriptors(const VulkanDevice* pDevice);
	bool								CreateDescriptorPool(const VulkanDevice* pDevice);
	bool								CreateDescriptorSetLayout(const VulkanDevice* pDevice);
	bool								CreateDescriptorSets(const VulkanDevice* pDevice);

public:
	inline vk::PipelineLayout			GetPipelineLayout() const { return m_vkRenderingPipelineLayout; }

private:
	vk::DescriptorPool					m_vkDescriptorPool;
	vk::DescriptorSetLayout				m_vkDescriptorSetLayout;
	std::vector<vk::DescriptorSet>		m_ListDescriptorSets;

	vk::PipelineLayout					m_vkRenderingPipelineLayout;

	VulkanMesh*							m_pMesh;
	MeshUniformDataBuffer*				m_pShaderDataBuffer;
	VulkanMaterial*						m_pMaterial;

	std::vector<VertexPNTBT>			m_ListVertices;
	std::vector<uint32_t>				m_ListIndices;

	glm::vec4							m_Color;
};
