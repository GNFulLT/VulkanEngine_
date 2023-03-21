#ifndef UTILS_VULKAN_H
#define UTILS_VULKAN_H


#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include <unordered_map>
#include <vector>
#include <set>

#include "../../core/typedefs.h"


struct QueueCreateInf
{
public:
	QueueCreateInf()
	{
		m_priorities = new std::vector<float>();
	}
	~QueueCreateInf()
	{
		if (m_priorities)
			delete m_priorities;
	}

	_INLINE_ uint32_t get_queue_create_inf_count() const
	{
		return (uint32_t)infos.size();
	}

	_INLINE_ const VkDeviceQueueCreateInfo* data() const
	{
		return infos.data();
	}

	_INLINE_ void add_create_info(uint32_t index, const std::vector<float>& priorities)
	{
		for (int i = 0; i < priorities.size(); i++)
		{
			m_priorities->push_back(priorities[i]);
		}

		VkDeviceQueueCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		info.pNext = nullptr;
		info.queueFamilyIndex = index;
		info.queueCount = (uint32_t)priorities.size();
		info.pQueuePriorities = &(*m_priorities)[m_priorities->size() - priorities.size()];
		infos.push_back(info);


	}

private:
	std::vector<VkDeviceQueueCreateInfo> infos;
	std::vector<float>* m_priorities;

};


struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

struct VulkanTexture {
	VkImage image;
	VkDeviceMemory imageMemory;
	VkImageView imageView;
};



_F_INLINE_ uint32_t find_memory_type(VkPhysicalDevice device, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{

	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(device, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	return (uint32_t)-1;
}


_INLINE_ bool create_image_view(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, VkImageView* imageView, VkImageViewType viewType, uint32_t layerCount, uint32_t mipLevels)
{
	const VkImageViewCreateInfo viewInfo =
	{
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.image = image,
		.viewType = viewType,
		.format = format,
		.subresourceRange =
		{
			.aspectMask = aspectFlags,
			.baseMipLevel = 0,
			.levelCount = mipLevels,
			.baseArrayLayer = 0,
			.layerCount = layerCount
		}
	};

	return (vkCreateImageView(device, &viewInfo, nullptr, imageView) == VK_SUCCESS);
}



_INLINE_ bool create_image(VkDevice device, VkPhysicalDevice physicalDevice, uint32_t width, uint32_t height,
	VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
{
	const VkImageCreateInfo imageInfo = { .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,    .pNext = nullptr,    .flags = 0,    .imageType = VK_IMAGE_TYPE_2D,    .format = format,    .extent = VkExtent3D           {.width = width, .height = height,.depth = 1},    .mipLevels = 1,    .arrayLayers = 1,    .samples = VK_SAMPLE_COUNT_1_BIT,    .tiling = tiling,    .usage = usage,    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,    .queueFamilyIndexCount = 0,    .pQueueFamilyIndices = nullptr,    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED };
	if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS)
		return false;
	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(device, image, &memRequirements);
	const VkMemoryAllocateInfo ai = { .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,    .pNext = nullptr,    .allocationSize = memRequirements.size,    .memoryTypeIndex = find_memory_type(physicalDevice,      memRequirements.memoryTypeBits, properties) };
	if (vkAllocateMemory(device, &ai, nullptr, &imageMemory) != VK_SUCCESS)
	{
		// Destroy created Image
		vkDestroyImage(device, image, nullptr);
		return false;
	}
	vkBindImageMemory(device, image, imageMemory, 0);
	return true;
}

_IMP_RETURN_ _F_INLINE_ bool get_all_instance_layers(std::vector<VkLayerProperties>& layerProps)
{
	uint32_t layer_count = 0;
	if (vkEnumerateInstanceLayerProperties(&layer_count, nullptr) != VK_SUCCESS)
	{
		return false;
	}
	layerProps = std::vector<VkLayerProperties>(layer_count);
	if (vkEnumerateInstanceLayerProperties(&layer_count, layerProps.data()) != VK_SUCCESS)
	{
		return false;
	}
	return true;
}

_IMP_RETURN_ _F_INLINE_ bool find_layer_by_name(const std::vector<VkLayerProperties>* layerProps, const char* name, int& index)
{
	index = -1;

	for (int i = 0; i < layerProps->size(); i++)
	{
		if (strcmp((*layerProps)[i].layerName, name) == 0)
		{
			index = i;
			return true;
		}
	}


	return false;
}

_IMP_RETURN_ _F_INLINE_ bool get_instance_exs_by_layer(const char* forLayer, std::vector<VkExtensionProperties>& exs)
{
	uint32_t ex_count = 0;
	if (vkEnumerateInstanceExtensionProperties(forLayer, &ex_count, nullptr) != VK_SUCCESS)
	{
		return false;
	}
	exs = std::vector<VkExtensionProperties>(ex_count);
	if (vkEnumerateInstanceExtensionProperties(forLayer, &ex_count, exs.data()) != VK_SUCCESS)
	{
		return false;
	}
	return true;
}

_F_INLINE_ bool get_instance_implicit_exs(std::vector<VkExtensionProperties>& exs)
{
	return get_instance_exs_by_layer(nullptr, exs);
}

_IMP_RETURN_ _F_INLINE_ std::unordered_map<std::string, std::vector<VkExtensionProperties>> get_all_instance_exs_by_layers(std::vector<const char*>* layers)
{
	std::unordered_map<std::string, std::vector<VkExtensionProperties>> map;
	for (int i = 0; i < layers->size(); i++)
	{
		std::vector<VkExtensionProperties> props;

		if (!get_instance_exs_by_layer((*layers)[i], props))
			continue;
		map.emplace(std::string((*layers)[i]), props);
	}
	return map;
}

_F_INLINE_ bool find_instance_exs_by_names(const std::vector<VkExtensionProperties>* exs, const std::vector<const char*>* names, std::vector<int>& indexes)
{
	std::set<std::string> searchedNames(names->begin(), names->end());

	for (int i = 0; i < exs->size(); i++)
	{
		if (searchedNames.find((*exs)[i].extensionName) != searchedNames.end())
		{
			indexes.push_back(i);
			searchedNames.erase((*exs)[i].extensionName);
		}
	}

	return indexes.size() == names->size();
}

_IMP_RETURN_ _F_INLINE_  bool create_debug_messenger(VkInstance instance, VkDebugUtilsMessengerEXT* pDebugMessenger, VkDebugReportCallbackEXT* pDebugReporter,
	PFN_vkDebugUtilsMessengerCallbackEXT vk_debug_messenger_callback, PFN_vkDebugReportCallbackEXT VulkanDebugReportCallback) {
	// Load the function pointer for vkCreateDebugUtilsMessengerEXT()
	PFN_vkCreateDebugUtilsMessengerEXT pfnCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
	if (pfnCreateDebugUtilsMessengerEXT == nullptr) {
		return false;
	}

	// Fill in the debug messenger creation info structure
	VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = vk_debug_messenger_callback;
	createInfo.pUserData = nullptr;
	// Create the debug messenger
	VkResult result = pfnCreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, pDebugMessenger);

	if (result != VK_SUCCESS)
		return false;
	const VkDebugReportCallbackCreateInfoEXT ci = {
			.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,
			.pNext = nullptr,
			.flags =
				VK_DEBUG_REPORT_WARNING_BIT_EXT |
				VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
				VK_DEBUG_REPORT_ERROR_BIT_EXT |
				VK_DEBUG_REPORT_DEBUG_BIT_EXT,
			.pfnCallback = VulkanDebugReportCallback,
			.pUserData = nullptr
	};
	auto pvkCreateDebugReportCallbackEXT = PFN_vkCreateDebugReportCallbackEXT(vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT"));
	if (pvkCreateDebugReportCallbackEXT == nullptr)
		return false;
	result = pvkCreateDebugReportCallbackEXT(instance, &ci, nullptr, pDebugReporter);
	return result == VK_SUCCESS;
}

_IMP_RETURN_ _F_INLINE_ bool get_all_physical_devices(VkInstance inst, std::vector<VkPhysicalDevice>& physicalDevs)
{
	uint32_t physicalDevCount = 0;
	if (vkEnumeratePhysicalDevices(inst, &physicalDevCount, nullptr) != VK_SUCCESS)
	{
		return false;
	}

	physicalDevs = std::vector<VkPhysicalDevice>(physicalDevCount);

	if (vkEnumeratePhysicalDevices(inst, &physicalDevCount, physicalDevs.data()) != VK_SUCCESS)
	{
		return false;
	}

	return true;
}

_IMP_RETURN_ _F_INLINE_  bool check_device_extension_support(VkPhysicalDevice device, const std::vector<std::string>* deviceExtensions, std::vector<VkExtensionProperties>& deviceExtensionProps)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions->begin(), deviceExtensions->end());

	for (const auto& extension : availableExtensions) {
		if (requiredExtensions.find(extension.extensionName) != requiredExtensions.end())
		{
			requiredExtensions.erase(extension.extensionName);
			deviceExtensionProps.push_back(extension);
		}
	}

	return requiredExtensions.empty();
}

_F_INLINE_ std::vector<VkQueueFamilyProperties> get_all_queue_families_by_device(VkPhysicalDevice device)
{
	uint32_t count;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
	std::vector<VkQueueFamilyProperties> families(count);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &count, families.data());
	return families;
}

_IMP_RETURN_ _F_INLINE_ bool check_queue_support(VkPhysicalDevice device, VkQueueFlags flags, int& index)
{
	auto queues = get_all_queue_families_by_device(device);
	for (int i = 0; i < queues.size(); i++)
	{
		if ((queues[i].queueFlags & flags) == flags)
		{
			index = i;
			return true;
		}
	}

	return false;
}

_IMP_RETURN_ _F_INLINE_ bool get_swap_chain_support_details(VkPhysicalDevice dev, VkSurfaceKHR surface, SwapChainSupportDetails& detail)
{
	uint32_t surfaceCount = 0;
	if (VK_SUCCESS != vkGetPhysicalDeviceSurfacePresentModesKHR(dev, surface, &surfaceCount, nullptr))
		return false;

	detail.presentModes = std::vector<VkPresentModeKHR>(surfaceCount);


	if (VK_SUCCESS != vkGetPhysicalDeviceSurfacePresentModesKHR(dev, surface, &surfaceCount, detail.presentModes.data()))
		return false;


	if (VK_SUCCESS != vkGetPhysicalDeviceSurfaceCapabilitiesKHR(dev, surface, &detail.capabilities))
		return false;


	uint32_t formatCount;
	if (VK_SUCCESS != vkGetPhysicalDeviceSurfaceFormatsKHR(dev, surface, &formatCount, nullptr))
		return false;


	detail.formats = std::vector<VkSurfaceFormatKHR>(formatCount);

	// Here causes heap corruption
	if (VK_SUCCESS != vkGetPhysicalDeviceSurfaceFormatsKHR(dev, surface, &formatCount, detail.formats.data()))
		return false;

	return true;
}

_IMP_RETURN_ _F_INLINE_ bool get_all_device_layers(VkPhysicalDevice dev, std::vector<VkLayerProperties>& props)
{
	uint32_t propCount = 0;
	if (VK_SUCCESS != vkEnumerateDeviceLayerProperties(dev, &propCount, nullptr))
		return false;
	props = std::vector<VkLayerProperties>(propCount);
	if (VK_SUCCESS != vkEnumerateDeviceLayerProperties(dev, &propCount, props.data()))
		return false;
	return true;
}

_F_INLINE_ bool get_device_implicit_exs(VkPhysicalDevice dev, std::vector<VkExtensionProperties>& exs)
{
	uint32_t propCount = 0;
	if (VK_SUCCESS != vkEnumerateDeviceExtensionProperties(dev, nullptr, &propCount, nullptr))
		return false;
	exs = std::vector<VkExtensionProperties>(propCount);
	if (VK_SUCCESS != vkEnumerateDeviceExtensionProperties(dev, nullptr, &propCount, exs.data()))
		return false;
	return true;
}

#endif
