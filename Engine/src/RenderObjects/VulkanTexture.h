#pragma once

#include "../VulkanRenderer/VulkanGlobals.h"

class VulkanDevice;
enum class TextureType;

//---------------------------------------------------------------------------------------------------------------------
class UT_API VulkanTexture
{
public:
	VulkanTexture();
	~VulkanTexture();

	bool						CreateTexture(const VulkanDevice* pDevice, const std::string& filename, vk::Format format);
	void						Cleanup(const VulkanDevice* pDevice);
	void						CleanupOnWindowResize(const VulkanDevice* pDevice);

public:
	inline vk::Image			getVkImage()	 const	{ return m_pImage->image; }
	inline vk::ImageView		getVkImageView() const	{ return m_pImage->imageView; }
	inline vk::Sampler			getVkSampler()	 const	{ return m_vkTextureSampler; }

private:
	UT::VkStructs::VulkanImage*	m_pImage;
	vk::Sampler					m_vkTextureSampler;
								
private:						
	unsigned char*				LoadImageData(const std::string& filename);
	bool						CreateImage(const VulkanDevice* pDevice, const std::string& filename, vk::Format format);
	bool						CreateTextureSampler(const VulkanDevice* pDevice);
								
	int							m_iTextureWidth;
	int							m_iTextureHeight;
	int							m_iTextureChannels;
	vk::DeviceSize				m_vkTextureDeviceSize;
};

