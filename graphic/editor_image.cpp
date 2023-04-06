#include "editor_image.h"
#include "../../imgui/imgui_impl_vulkan.h"
#include "../../manager/rendering/render_device.h"

EditorImage::~EditorImage()
{
	if (m_imageSampler != nullptr)
	{
		vkDestroySampler(RenderDevice::get_singleton()->get_render_device().logicalDevice, m_imageSampler, nullptr);
	}
}

bool EditorImage::load_impl()
{
	bool inited = VulkanImage::load_impl();
	if (!inited)
		return false;

	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;


	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(RenderDevice::get_singleton()->get_render_device().physicalDev.physicalDev, &properties);

	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;
	if (vkCreateSampler(RenderDevice::get_singleton()->get_render_device().logicalDevice, &samplerInfo, nullptr, &m_imageSampler) != VK_SUCCESS) {
		return false;
	}


	m_descriptorSet = ImGui_ImplVulkan_AddTexture(m_imageSampler, m_imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	return true;
}
