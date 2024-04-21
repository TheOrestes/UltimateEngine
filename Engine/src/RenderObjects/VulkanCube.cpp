
#include "UltimateEnginePCH.h"
#include "VulkanMesh.h"
#include "VulkanMeshData.h"
#include "GameObject.h"
#include "VulkanCube.h"
#include "VulkanMaterial.h"
#include "VulkanTexture.h"
#include "../World/Camera.h"
#include "../VulkanRenderer/VulkanDevice.h"
#include "../VulkanRenderer/VulkanGlobals.h"

//---------------------------------------------------------------------------------------------------------------------
VulkanCube::VulkanCube(const std::string& name, const glm::vec4 color) : GameObject(name)
{
	m_Color = color;

	m_pShaderDataBuffer = nullptr;
	m_pMesh = nullptr;
	m_pMaterial = nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
VulkanCube::~VulkanCube()
{
	m_ListVertices.clear();
	m_ListIndices.clear();

	SAFE_DELETE(m_pMaterial);
	SAFE_DELETE(m_pMesh);
	SAFE_DELETE(m_pShaderDataBuffer);
}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanCube::Initialize(const void* pDevice)
{
	const auto* pVulkanDevice = static_cast<const VulkanDevice*>(pDevice);

	// Vertex Data!
	m_ListVertices.resize(8);

	m_ListVertices[0] = VertexPNTBT(glm::vec3(-1, -1, 1),	glm::vec3(1), glm::vec3(0), glm::vec3(0), glm::vec2(0,0));
	m_ListVertices[1] = VertexPNTBT(glm::vec3(1, -1, 1),	glm::vec3(1), glm::vec3(0), glm::vec3(0), glm::vec2(1,0));
	m_ListVertices[2] = VertexPNTBT(glm::vec3(1, 1, 1),		glm::vec3(1), glm::vec3(0), glm::vec3(0), glm::vec2(1,1));
	m_ListVertices[3] = VertexPNTBT(glm::vec3(-1, 1, 1),	glm::vec3(1), glm::vec3(0), glm::vec3(0), glm::vec2(0,1));
	m_ListVertices[4] = VertexPNTBT(glm::vec3(-1, -1, -1),	glm::vec3(1), glm::vec3(0), glm::vec3(0), glm::vec2(0,0));
	m_ListVertices[5] = VertexPNTBT(glm::vec3(1, -1, -1),	glm::vec3(1), glm::vec3(0), glm::vec3(0), glm::vec2(1,0));
	m_ListVertices[6] = VertexPNTBT(glm::vec3(1, 1, -1),	glm::vec3(1), glm::vec3(0), glm::vec3(0), glm::vec2(1,1));
	m_ListVertices[7] = VertexPNTBT(glm::vec3(-1, 1, -1),	glm::vec3(1), glm::vec3(0), glm::vec3(0), glm::vec2(0,1));

	// Index Data!
	m_ListIndices.resize(36);

	m_ListIndices[0] = 0;				m_ListIndices[1] = 1;			m_ListIndices[2] = 2;
	m_ListIndices[3] = 0;				m_ListIndices[4] = 2;			m_ListIndices[5] = 3;

	m_ListIndices[6] = 1;				m_ListIndices[7] = 5;			m_ListIndices[8] = 6;
	m_ListIndices[9] = 1;				m_ListIndices[10] = 6;			m_ListIndices[11] = 2;

	m_ListIndices[12] = 5;				m_ListIndices[13] = 4;			m_ListIndices[14] = 7;
	m_ListIndices[15] = 5;				m_ListIndices[16] = 7;			m_ListIndices[17] = 6;

	m_ListIndices[18] = 4;				m_ListIndices[19] = 0;			m_ListIndices[20] = 3;
	m_ListIndices[21] = 4;				m_ListIndices[22] = 3;			m_ListIndices[23] = 7;

	m_ListIndices[24] = 3;				m_ListIndices[25] = 2;			m_ListIndices[26] = 6;
	m_ListIndices[27] = 3;				m_ListIndices[28] = 6;			m_ListIndices[29] = 7;

	m_ListIndices[30] = 0;				m_ListIndices[31] = 4;			m_ListIndices[32] = 5;
	m_ListIndices[33] = 0;				m_ListIndices[34] = 5;			m_ListIndices[35] = 1;

	m_pMesh = new VulkanMesh(pVulkanDevice, m_ListVertices, m_ListIndices);

	m_pMaterial = new VulkanMaterial();
	m_pMaterial->CreateMaterial(pVulkanDevice, "Assets/Textures/Cube/DefaultWhite.png", TextureType::TEXTURE_ALBEDO, m_Color, m_Color);

	CHECK_LOG(SetupDescriptors(pVulkanDevice), "{0}'s Setup Descriptor FAILED!", GameObject::getName());

	// Create pipeline layout!
	const std::array<vk::DescriptorSetLayout, 1> setLayouts = { m_vkDescriptorSetLayout };
	vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
	pipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
	pipelineLayoutCreateInfo.pSetLayouts = setLayouts.data();
	pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

	m_vkRenderingPipelineLayout = pVulkanDevice->GetDevice().createPipelineLayout(pipelineLayoutCreateInfo, nullptr);

	LOG_DEBUG("{0} Gameobject Initialized", GameObject::getName());

	return true;
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanCube::Render(const VulkanDevice* pDevice, uint32_t index) const
{
	const std::vector<vk::Buffer> vertexBuffers = { m_pMesh->m_vkVertexBuffer.buffer };
	const VkBuffer indexBuffer = m_pMesh->m_vkIndexBuffer.buffer;
	const std::vector<vk::DeviceSize> offsets = { 0 };

	// Bind VB & IB
	const vk::CommandBuffer gfxCmdBuffer = pDevice->GetGraphicsCommandBuffer(index);

	//gfxCmdBuffer.bindVertexBuffers(0, vertexBuffers, offsets);
	gfxCmdBuffer.bindVertexBuffers(0, 1, vertexBuffers.data(), offsets.data());
	gfxCmdBuffer.bindIndexBuffer(indexBuffer, 0, vk::IndexType::eUint32);

	gfxCmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_vkRenderingPipelineLayout, 0, 1, &(m_ListDescriptorSets[index]), 0, nullptr);

	// Draw
	gfxCmdBuffer.drawIndexed(m_pMesh->m_uiIndexCount, 1, 0, 0, 0);
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanCube::Update(const Camera* pCamera, float dt) const
{
	static float fCurrentAngle = 0.0f;
	//fCurrentAngle += dt * 0.5f;
	if (fCurrentAngle > 360.0f) { fCurrentAngle = 0.0f; }

	m_pShaderDataBuffer->shaderData.matWorld = glm::mat4(1);
	m_pShaderDataBuffer->shaderData.matWorld = glm::translate(m_pShaderDataBuffer->shaderData.matWorld, m_vecPosition);
	m_pShaderDataBuffer->shaderData.matWorld = glm::rotate(m_pShaderDataBuffer->shaderData.matWorld, glm::degrees(m_fRotation), m_vecRotationAxis);
	m_pShaderDataBuffer->shaderData.matWorld = glm::scale(m_pShaderDataBuffer->shaderData.matWorld, m_vecScale);

	const float aspect = (float)(UT::VkGlobals::GCurrentResolution.x)/ (float)(UT::VkGlobals::GCurrentResolution.y);
	m_pShaderDataBuffer->shaderData.matProjection = pCamera->m_matProjection; //glm::perspective(glm::radians(45.0f), aspect, 0.1f, 1000.0f);
	m_pShaderDataBuffer->shaderData.matProjection[1][1] *= -1.0f;

	m_pShaderDataBuffer->shaderData.matView = pCamera->m_matView; //glm::lookAt(glm::vec3(0.0f, 2.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanCube::UpdateUniforms(vk::Device vkDevice, uint32_t imageIndex) const
{
	void* data;

	vkMapMemory(vkDevice, m_pShaderDataBuffer->listBuffers[imageIndex].deviceMemory, 0, sizeof(MeshUniformData), 0, &data);
	memcpy(data, &(m_pShaderDataBuffer->shaderData), sizeof(MeshUniformData));
	vkUnmapMemory(vkDevice, m_pShaderDataBuffer->listBuffers[imageIndex].deviceMemory);
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanCube::Cleanup(void* pDevice)
{
	const VulkanDevice* ptrDevice = static_cast<const VulkanDevice*>(pDevice);
	const vk::Device vkDevice = ptrDevice->GetDevice();

	m_pMesh->Cleanup(ptrDevice->GetDevice());
	m_pShaderDataBuffer->Cleanup(ptrDevice);

	vkDevice.destroyDescriptorPool(m_vkDescriptorPool);
	vkDevice.destroyDescriptorSetLayout(m_vkDescriptorSetLayout);

	m_ListVertices.clear();
	m_ListIndices.clear();
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanCube::CleanupOnWindowsResize(VulkanDevice* pDevice)
{
}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanCube::SetupDescriptors(const VulkanDevice* pDevice)
{
	m_pShaderDataBuffer = new MeshUniformDataBuffer();
	m_pShaderDataBuffer->CreateUniformDataBuffers(pDevice);

	// Set default material info!
	m_pShaderDataBuffer->shaderData.albedoColor = m_Color;
	m_pShaderDataBuffer->shaderData.emissionColor = m_Color;
	m_pShaderDataBuffer->shaderData.hasTextureAEN = glm::vec3(0);
	m_pShaderDataBuffer->shaderData.hasTextureRMO = glm::vec3(0);
	m_pShaderDataBuffer->shaderData.metalness = 0.0f;
	m_pShaderDataBuffer->shaderData.occlusion = 1.0f;
	m_pShaderDataBuffer->shaderData.roughness = 1.0f;

	// Descriptor Pool
	CHECK(CreateDescriptorPool(pDevice));

	// Descriptor Set Layout
	CHECK(CreateDescriptorSetLayout(pDevice));

	// Descriptor Sets
	CHECK(CreateDescriptorSets(pDevice));
}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanCube::CreateDescriptorPool(const VulkanDevice* pDevice)
{
	std::array<vk::DescriptorPoolSize, 2> arrDescriptorPoolSize = {};

	//-- Uniform buffers
	arrDescriptorPoolSize[0].descriptorCount = pDevice->GetSwapchainImageCount();

	//-- Texture samplers
	arrDescriptorPoolSize[1].descriptorCount = m_pMaterial->GetTexturesCount();

	vk::DescriptorPoolCreateInfo poolCreateInfo = {};
	poolCreateInfo.maxSets = pDevice->GetSwapchainImageCount() + m_pMaterial->GetTexturesCount();
	poolCreateInfo.poolSizeCount = static_cast<uint32_t>(arrDescriptorPoolSize.size());
	poolCreateInfo.pPoolSizes = arrDescriptorPoolSize.data();

	// Create Descriptor Pool!
	m_vkDescriptorPool = pDevice->GetDevice().createDescriptorPool(poolCreateInfo);

	return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanCube::CreateDescriptorSetLayout(const VulkanDevice* pDevice)
{
	std::array < vk::DescriptorSetLayoutBinding, 2> layoutBindings;

	// Uniform buffer
	layoutBindings[0].binding = 0;
	layoutBindings[0].descriptorType = vk::DescriptorType::eUniformBuffer;
	layoutBindings[0].descriptorCount = 1;
	layoutBindings[0].stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment;

	// Albedo texture
	layoutBindings[1].binding = 1;
	layoutBindings[1].descriptorType = vk::DescriptorType::eCombinedImageSampler;
	layoutBindings[1].descriptorCount = 1;
	layoutBindings[1].stageFlags = vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment;
	layoutBindings[1].pImmutableSamplers = nullptr;

	vk::DescriptorSetLayoutCreateInfo layoutCreateInfo = {};
	layoutCreateInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
	layoutCreateInfo.pBindings = layoutBindings.data();

	m_vkDescriptorSetLayout = pDevice->GetDevice().createDescriptorSetLayout(layoutCreateInfo);

	return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanCube::CreateDescriptorSets(const VulkanDevice* pDevice)
{
	//-- Descriptor Sets!
	m_ListDescriptorSets.resize(pDevice->GetSwapchainImageCount());

	// create copy of descriptor set layout for each swap chain image!
	const std::vector<vk::DescriptorSetLayout> listSetLayouts(pDevice->GetSwapchainImageCount(), m_vkDescriptorSetLayout);

	vk::DescriptorSetAllocateInfo setAllocInfo = {};
	setAllocInfo.descriptorPool = m_vkDescriptorPool;
	setAllocInfo.descriptorSetCount = static_cast<uint32_t>(listSetLayouts.size());
	setAllocInfo.pSetLayouts = listSetLayouts.data();

	m_ListDescriptorSets = pDevice->GetDevice().allocateDescriptorSets(setAllocInfo);

	//-- Update all the descriptor set bindings!
	for (uint16_t i = 0; i < pDevice->GetSwapchainImageCount(); i++)
	{
		//-- Uniform buffer
		vk::DescriptorBufferInfo ubBufferInfo = {};
		ubBufferInfo.buffer = m_pShaderDataBuffer->listBuffers[i].buffer;
		ubBufferInfo.offset = 0;
		ubBufferInfo.range = sizeof(MeshUniformData);

		vk::WriteDescriptorSet ubWriteSet = {};
		ubWriteSet.descriptorCount = 1;
		ubWriteSet.descriptorType = vk::DescriptorType::eUniformBuffer;
		ubWriteSet.dstArrayElement = 0;
		ubWriteSet.dstBinding = 0;
		ubWriteSet.dstSet = m_ListDescriptorSets[i];
		ubWriteSet.pBufferInfo = &ubBufferInfo;

		//-- Albedo Texture
		vk::DescriptorImageInfo albedoImageInfo = {};
		albedoImageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		albedoImageInfo.imageView = m_pMaterial->GetVulkanTexture(TextureType::TEXTURE_ALBEDO)->getVkImageView();
		albedoImageInfo.sampler = m_pMaterial->GetVulkanTexture(TextureType::TEXTURE_ALBEDO)->getVkSampler();

		vk::WriteDescriptorSet albedoWriteSet = {};
		albedoWriteSet.dstSet = m_ListDescriptorSets[i];
		albedoWriteSet.dstBinding = 1;
		albedoWriteSet.dstArrayElement = 0;
		albedoWriteSet.descriptorType = vk::DescriptorType::eCombinedImageSampler;
		albedoWriteSet.descriptorCount = 1;
		albedoWriteSet.pImageInfo = &albedoImageInfo;

		// List of all Descriptor set writes!
		std::array<vk::WriteDescriptorSet, 2> listWriteSets = { ubWriteSet , albedoWriteSet };

		// Update the descriptor sets with buffers/binding info
		pDevice->GetDevice().updateDescriptorSets(listWriteSets, nullptr);
	}

	return true;
}
