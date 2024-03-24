#pragma once

#include "glm/glm.hpp"

class VulkanTexture;
class VulkanDevice;

//---------------------------------------------------------------------------------------------------------------------
enum class TextureType
{
	TEXTURE_ALBEDO = 0,
	TEXTURE_METALNESS,
	TEXTURE_NORMAL,
	TEXTURE_ROUGHNESS,
	TEXTURE_AO,
	TEXTURE_EMISSIVE,
	TEXTURE_HDRI,
	TEXTURE_ERROR,
	TEXTURE_END
};

//---------------------------------------------------------------------------------------------------------------------
class VulkanMaterial
{
public:
	VulkanMaterial();
	~VulkanMaterial();

	bool					LoadTexture(const VulkanDevice* pContext, const std::string& filePath, TextureType type);
	void					Cleanup(const VulkanDevice* pContext);
	void					CleanupOnWindowResize(const VulkanDevice* pContext);

	inline uint32_t			GetTexturesCount() const					{ return static_cast<uint32_t>(m_umapTextures.size()); }
	VulkanTexture*			GetVulkanTexture(TextureType type) const;

public:
	bool					HasTexture(TextureType type) const;
	void					SetHasTexture(TextureType type);

private:
	// Has Textures?		
	glm::ivec3				m_hasTextureAEN;		// Albedo | Emissive | Normal
	glm::ivec3				m_hasTextureRMO;		// Roughness | Metallic | Occlusion

public:
	// Textures
	std::unordered_map<TextureType, VulkanTexture*> m_umapTextures;
	
	// Properties
	glm::vec4				m_colAlbedo;
	glm::vec4				m_colEmission;
	float					m_fRoughess;
	float					m_fMetallic;
	float					m_fOcclusion;

	uint32_t				m_uiNumTextures;
};

