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
bool VulkanMaterial::CreateMaterial(const VulkanDevice* pDevice, const std::string& filePath, TextureType type,
	const glm::vec4& albedoColor, const glm::vec4 emissiveColor)
{
	m_colAlbedo = albedoColor;
	m_colEmission = emissiveColor;

	if(type != TextureType::TEXTURE_NONE)
		CHECK(LoadTexture(pDevice, filePath, type))
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
		case TextureType::TEXTURE_NORMAL:
		case TextureType::TEXTURE_ROUGHNESS:
		case TextureType::TEXTURE_METALNESS:
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

	// Mark it that we have this "type" of texture within this material as bookkeeping!
	SetHasTexture(type);

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

//-----------------------------------------------------------------------------------------------------------------------
VulkanTexture* VulkanMaterial::GetVulkanTexture(TextureType type) const
{
	VulkanTexture* pTexture = new VulkanTexture();

	(HasTexture(type)) ? pTexture = m_umapTextures.at(type) : pTexture = nullptr;

	return pTexture;
}

//-----------------------------------------------------------------------------------------------------------------------
bool VulkanMaterial::HasTexture(TextureType type) const
{
	bool result = false;

	switch (type)
	{
	case TextureType::TEXTURE_ALBEDO:
			(m_hasTextureAEN.x == 1) ? result = true : result = false;
			break;
		case TextureType::TEXTURE_METALNESS:
			(m_hasTextureRMO.y == 1) ? result = true : result = false;
			break;
		case TextureType::TEXTURE_NORMAL:
			(m_hasTextureAEN.z == 1) ? result = true : result = false;
			break;
		case TextureType::TEXTURE_ROUGHNESS:
			(m_hasTextureRMO.x == 1) ? result = true : result = false;
			break;
		case TextureType::TEXTURE_AO:
			(m_hasTextureRMO.z == 1) ? result = true : result = false;
			break;
		case TextureType::TEXTURE_EMISSIVE:
			(m_hasTextureAEN.y == 1) ? result = true : result = false;
			break;
		default:
			break;
	}

	return result;
}

//-----------------------------------------------------------------------------------------------------------------------
void VulkanMaterial::SetHasTexture(TextureType type)
{
	switch (type)
	{
		case TextureType::TEXTURE_ALBEDO:
			m_hasTextureAEN.x = 1;
			break;
		case TextureType::TEXTURE_METALNESS:
			m_hasTextureRMO.y = 1;
			break;
		case TextureType::TEXTURE_NORMAL:
			m_hasTextureAEN.z = 1;
			break;
		case TextureType::TEXTURE_ROUGHNESS:
			m_hasTextureRMO.x = 1;
			break;
		case TextureType::TEXTURE_AO:
			m_hasTextureRMO.z = 1;
			break;
		case TextureType::TEXTURE_EMISSIVE:
			m_hasTextureAEN.y = 1;
			break;
		default:
			break;
	}
}
