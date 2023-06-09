#ifndef RENDER_DEVICE_H
#define RENDER_DEVICE_H

#include <taskflow/taskflow.hpp>

#include <vector>
#include <unordered_map>

#include "../../core/object/object.h"
#include "../../core/utils_vulkan.h"
#include "../../config/config_property.h"
#define IMPLICIT_EXTENSIONS_NAME "ENGINE_EXS"


#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.h>

#include <boost/signals2.hpp>
#include "../../graphic/vulkan_renderpass.h"
#include "../../core/vec2.h"
#include "../system_manager.h"
#include "../window_manager.h"

class CreationServer;

class RenderDevice : public SystemManager
{
	SINGLETON(RenderDevice)
	OBJECT_DEF(RenderDevice, SystemManager)

public:
	RenderDevice();
	struct EnabledProps final
	{
		std::vector<VkLayerProperties> enabledLayers;
		std::unordered_map<std::string, std::vector<VkExtensionProperties>> enabledExtensions;
	};

	struct VulkanInstance final
	{
		VkInstance instance = nullptr;
		VkDebugUtilsMessengerEXT messenger = nullptr;
		VkDebugReportCallbackEXT reportCallback = nullptr;
		EnabledProps enabledProps;
	};

	struct VulkanPhysicalDevice final
	{
		VkPhysicalDevice physicalDev = nullptr;
		VkPhysicalDeviceFeatures physicalDevFeatures;
		VkPhysicalDeviceProperties physicalDevProperties;
	};

	struct VulkanRenderDevice final
	{
		VulkanPhysicalDevice physicalDev;


		int mainQueueFamilyIndex;
		int mainQueueIndex;
		VkQueue mainQueue = nullptr;

		int presentQueueFamilyIndex;
		int presentQueueIndex;
		VkQueue presentQueue = nullptr;


		VkDevice logicalDevice = nullptr;
		EnabledProps enabledProps;


		//! CMD

		std::vector<VkCommandPool> mainQueueCommandPools;

		VkCommandBuffer pMainCommandBuffer = nullptr;
		VkCommandBuffer pSceneCommandBuffer = nullptr;
	
		std::unordered_map<unsigned int, std::vector<VkCommandBuffer>> commandBuffers;


		VkCommandPool presentCommandPool = nullptr;
		VkCommandBuffer presentCommandBuffer = nullptr;

		//! Sync
		VkSemaphore renderCompleteSemaphore = nullptr;
		VkSemaphore imageAcquiredSemaphore = nullptr;
		VkSemaphore renderSceneCompleteSemaphore = nullptr;

		VkFence	mainQueueFinishedFence = nullptr;
		VkFence presentQueueFinishedFence = nullptr;
	};

	~RenderDevice();

	bool init() override;

	void beginFrame();
	
	VkCommandBuffer begin_single_time_command();

	void finish_exec_single_time_command(VkCommandBuffer buff);

	_INLINE_ VulkanRenderDevice& get_render_device()
	{
		return m_renderDevice;
	}

	void beginFrameW();

	void reset_things();

	bool render_things(tf::Subflow& subflow);
	_INLINE_ VkCommandBuffer get_main_cmd() const noexcept
	{
		return m_renderDevice.pMainCommandBuffer;
	}

	_INLINE_ VulkanInstance get_instance() const noexcept
	{
		return m_instance;
	}

	_INLINE_ VkCommandBuffer get_cmd(int poolIndex, int cmdIndex)
	{
		return m_renderDevice.commandBuffers[poolIndex][cmdIndex];
	}
	_INLINE_ VkCommandBufferBeginInfo* get_main_begin_inf()
	{
		static VkCommandBufferBeginInfo beginInf{
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			nullptr,
			0,
			nullptr
		};

		return &beginInf;
	}

	_INLINE_ bool does_swapchain_need_validate()
	{
		return swapchain_needs_validate;
	}

	_INLINE_ uint32_t* get_current_image()
	{
		return &currentImageIndex;
	}

	_INLINE_ VkSubmitInfo* get_main_submit_info(uint32_t cmdCount, VkCommandBuffer* buffs)
	{
		static VkPipelineStageFlags waitStage[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		static VkSemaphore* semaphores[] = { &m_renderDevice.imageAcquiredSemaphore,&m_renderDevice.renderSceneCompleteSemaphore };
		static VkSubmitInfo submitInfo
		{
			VK_STRUCTURE_TYPE_SUBMIT_INFO,
			nullptr,
			2,
			semaphores[0],
			waitStage,
			0,
			nullptr,
			1,
			&m_renderDevice.renderCompleteSemaphore
		};
		if (m_renderScene)
		{
			
			submitInfo.pWaitSemaphores = semaphores[0];
			submitInfo.waitSemaphoreCount = 2;
		}
		else
		{
			submitInfo.pWaitSemaphores = &m_renderDevice.imageAcquiredSemaphore;
			submitInfo.waitSemaphoreCount = 1;
		}
		submitInfo.commandBufferCount = cmdCount;
		submitInfo.pCommandBuffers = buffs;

		return &submitInfo;
	}

	_F_INLINE_ ConfigProperty<bool>* get_quitting()
	{
		return &m_quitting;
	}


	void handleError();
private:
	bool swapchain_needs_validate = false;

	VulkanInstance m_instance;
	// Initialize with save physical dev
	VulkanRenderDevice m_renderDevice;
	bool m_instanceLoaded = false;
	bool m_isDebugEnabled = false;
	// These are loading methods for upper structs
private:
	bool init_vk_instance();
	bool save_vk_physical_device();
	bool init_vk_device();
	bool init_vk_logical_device();
	void expose_queues();
	bool create_command_pools();
	bool init_vk_syncs();
	bool init_command_buffers();
	bool init_renderpass();
	boost::signals2::connection windowResizeConnection;
	uint32_t currentImageIndex = 0;
	_INLINE_ void reset_cmd_pool(int index)
	{
		vkResetCommandPool(m_renderDevice.logicalDevice, m_renderDevice.mainQueueCommandPools[index], 0);
	}

	bool m_canContinue = true;
	bool m_renderScene = true;
	ConfigProperty<bool> m_quitting;
private:
	friend class CreationServer;
	friend class ImGuiDraw;
};

#endif // RENDER_DEVICE_H