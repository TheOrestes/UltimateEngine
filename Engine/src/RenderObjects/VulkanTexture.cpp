#include "UltimateEnginePCH.h"

#include "VulkanTexture.h"
#include "../VulkanRenderer/VulkanDevice.h"
#include "../VulkanRenderer/VulkanGlobals.h"
#include "../EngineHeader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//---------------------------------------------------------------------------------------------------------------------
VulkanTexture::VulkanTexture(): m_iTextureWidth(0), m_iTextureHeight(0), m_iTextureChannels(0), m_vkTextureDeviceSize(0)
{
	m_pImage = nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
VulkanTexture::~VulkanTexture()
{
	SAFE_DELETE(m_pImage);
}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanTexture::CreateTexture(const VulkanDevice* pDevice, const std::string& filename, vk::Format format)
{
	m_pImage = new UT::VkStructs::VulkanImage();

	CHECK(CreateImage(pDevice, filename, format));

	// Create Sampler
	CHECK(CreateTextureSampler(pDevice));

	LOG_DEBUG("Created Vulkan Texture for {0}", filename);

	return true;
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanTexture::Cleanup(const VulkanDevice* pDevice)
{
	const vk::Device vkDevice = pDevice->GetDevice();

	vkDevice.destroySampler(m_vkTextureSampler);
	m_pImage->DestroyAll(vkDevice);
}

//---------------------------------------------------------------------------------------------------------------------
void VulkanTexture::CleanupOnWindowResize(const VulkanDevice* pDevice)
{
}

//---------------------------------------------------------------------------------------------------------------------
unsigned char* VulkanTexture::LoadImageData(const std::string& filename)
{
	// Number of channels in image
	int channels = 0;

	// Load pixel data for an image
	unsigned char* imageData = stbi_load(filename.c_str(), &m_iTextureWidth, &m_iTextureHeight, &m_iTextureChannels, STBI_rgb_alpha);
	if (!imageData)
	{
		LOG_ERROR(("Failed to load a Texture file! (" + filename + ")").c_str());
	}

	// Calculate image size using given data
	m_vkTextureDeviceSize = m_iTextureWidth * m_iTextureHeight * 4;

	return imageData;
}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanTexture::CreateImage(const VulkanDevice* pDevice, const std::string& filename, vk::Format format)
{
	// Load image data!
	stbi_uc* imgData = LoadImageData(filename);
	
	// Create staging buffer to hold the loaded data, ready to copy to device
	UT::VkStructs::VulkanBuffer stagingBuffer;

	pDevice->CreateBuffer(m_vkTextureDeviceSize,
						  vk::BufferUsageFlagBits::eTransferSrc,
						  vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
						  &stagingBuffer);

	// Grab vk::Device handle!
	const vk::Device vkDevice = pDevice->GetDevice();

	// Copy image data to staging buffer
	void* data;
	vkMapMemory(vkDevice, stagingBuffer.deviceMemory, 0, m_vkTextureDeviceSize, 0, &data);
	memcpy(data, imgData, static_cast<uint32_t>(m_vkTextureDeviceSize));
	vkUnmapMemory(vkDevice, stagingBuffer.deviceMemory);

	// Free original image data
	stbi_image_free(imgData);

	// Create Image...
	pDevice->CreateImage2D(	m_iTextureWidth, 
							m_iTextureHeight,
							format, 
							vk::ImageTiling::eOptimal,
							vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
							vk::MemoryPropertyFlagBits::eDeviceLocal,
							vk::ImageAspectFlagBits::eColor,
							m_pImage);

	// Transition image to be DST for copy operation
	pDevice->TransitionImageLayout(m_pImage->image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, VK_NULL_HANDLE);

	// COPY DATA TO IMAGE!
	pDevice->CopyBufferToImage(stagingBuffer.buffer, m_pImage->extent.width, m_pImage->extent.height, &(m_pImage->image));

	// Transition image to be Shader Readable for shader usage
	pDevice->TransitionImageLayout(m_pImage->image, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, VK_NULL_HANDLE);

	// Destroy staging buffers
	stagingBuffer.DestroyAll(vkDevice);

	return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool VulkanTexture::CreateTextureSampler(const VulkanDevice* pDevice)
{
	//-- Sampler creation Info
	vk::SamplerCreateInfo samplerCreateInfo = {};
	samplerCreateInfo.magFilter = vk::Filter::eLinear;							// how to render when image is magnified on screen
	samplerCreateInfo.minFilter = vk::Filter::eLinear;							// how to render when image is minified on screen			
	samplerCreateInfo.addressModeU = vk::SamplerAddressMode::eRepeat;			// how to handle texture wrap in U (x) direction
	samplerCreateInfo.addressModeV = vk::SamplerAddressMode::eRepeat;			// how to handle texture wrap in V (y) direction
	samplerCreateInfo.addressModeW = vk::SamplerAddressMode::eRepeat;			// how to handle texture wrap in W (z) direction
	samplerCreateInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;			// border beyond texture (only works for border clamp)
	samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;						// whether values of texture coords between [0,1] i.e. normalized
	samplerCreateInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;				// Mipmap interpolation mode
	samplerCreateInfo.mipLodBias = 0.0f;										// Level of detail bias for mip level
	samplerCreateInfo.minLod = 0.0f;											// minimum level of detail to pick mip level
	samplerCreateInfo.maxLod = 0.0f;											// maximum level of detail to pick mip level
	samplerCreateInfo.anisotropyEnable = VK_FALSE;								// Enable Anisotropy or not? Check physical device features to see if anisotropy is supported or not!
	samplerCreateInfo.maxAnisotropy = 16;										// Anisotropy sample level

	m_vkTextureSampler = pDevice->GetDevice().createSampler(samplerCreateInfo);

	return true;
}
