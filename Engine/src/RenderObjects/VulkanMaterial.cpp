#include "UltimateEnginePCH.h"
#include "VulkanTexture.h"
#include "VulkanMaterial.h"
#include "../VulkanRenderer/VulkanDevice.h"

//-----------------------------------------------------------------------------------------------------------------------
VulkanMaterial::VulkanMaterial()
{
	m_umapTextures.clear();

	//m_pTextureAlbedo = nullptr;
	//m_pTextureEmission = nullptr;
	//m_pTextureMetalness = nullptr;
	//m_pTextureNormal = nullptr;
	//m_pTextureOcclusion = nullptr;
	//m_pTextureRoughness = nullptr;
	//m_pTextureHDRI = nullptr;
	//m_pTextureError = nullptr;

	m_hasTextureAEN = glm::vec3(0);
	m_hasTextureRMO = glm::vec3(0);

	m_uiNumTextures = 0;
}

//-----------------------------------------------------------------------------------------------------------------------
VulkanMaterial::~VulkanMaterial()
{
	std::unordered_map<TextureType, VulkanTexture*>::iterator iter = m_umapTextures.begin();

	for( ; iter != m_umapTextures.end() ; ++iter)
	{
		SAFE_DELETE(iter->second);
	}

	m_umapTextures.clear();

	//SAFE_DELETE(m_pTextureAlbedo);
	//SAFE_DELETE(m_pTextureEmission);
	//SAFE_DELETE(m_pTextureMetalness);
	//SAFE_DELETE(m_pTextureNormal);
	//SAFE_DELETE(m_pTextureOcclusion);
	//SAFE_DELETE(m_pTextureRoughness);
	//SAFE_DELETE(m_pTextureHDRI);
	//SAFE_DELETE(m_pTextureError);
}

//-----------------------------------------------------------------------------------------------------------------------
bool VulkanMaterial::LoadTexture(const VulkanDevice* pDevice, const std::string& filePath, TextureType type)
{
	VulkanTexture* pTexture = new VulkanTexture();
	
	vk::Format textureFormat = vk::Format::eUndefined;

	switch (type)
	{
		case TextureType::TEXTURE_ALBEDO:
		{
			textureFormat = vk::Format::eR8G8B8A8Srgb;
			break;
		}

		case TextureType::TEXTURE_EMISSIVE:
		case TextureType::TEXTURE_METALNESS:
		case TextureType::TEXTURE_NORMAL:
		case TextureType::TEXTURE_ROUGHNESS:
		case TextureType::TEXTURE_AO:
		case TextureType::TEXTURE_ERROR:
		{
			textureFormat = vk::Format::eR8G8B8A8Unorm;
			break;
		}

		case TextureType::TEXTURE_HDRI:
		{
			textureFormat = vk::Format::eR32G32B32A32Sfloat;
			break;
		}

		default:
			break;
		
	}

	CHECK(pTexture->CreateTexture(pDevice, filePath, textureFormat));
	m_umapTextures.insert(std::make_pair(type, pTexture));

	return true;
}

//-----------------------------------------------------------------------------------------------------------------------
void VulkanMaterial::Cleanup(const VulkanDevice* pDevice)
{
	std::unordered_map<TextureType, VulkanTexture*>::iterator iter = m_umapTextures.begin();

	for (; iter != m_umapTextures.end(); ++iter)
	{
		if(iter->second != nullptr)
		{
			iter->second->Cleanup(pDevice);
		}
	}
}

//-----------------------------------------------------------------------------------------------------------------------
void VulkanMaterial::CleanupOnWindowResize(const VulkanDevice* pDevice)
{
}
