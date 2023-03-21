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

#include "../../core/vec2.h"
#include "../system_manager.h"

class CreationServer;
class ImGuiDraw;

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
		VkSurfaceKHR surface = nullptr;
		VkDebugUtilsMessengerEXT messenger = nullptr;
		VkDebugReportCallbackEXT reportCallback = nullptr;
		VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
		VkSurfaceFormatKHR format;
		int surfaceImageCount;
		VkExtent2D surfaceExtent;
		EnabledProps enabledProps;
	};

	struct VulkanPhysicalDevice final
	{
		VkPhysicalDevice physicalDev = nullptr;
		VkPhysicalDeviceFeatures physicalDevFeatures;
		VkPhysicalDeviceProperties physicalDevProperties;
	};

	struct VulkanSwapChain final
	{
		VkSwapchainKHR swapchain = nullptr;

		//X This is images created with swapchain
		std::vector<VkImage> swapchainImages;

		//X This is image views for swapchainImages to able writing into image
		std::vector<VkImageView> swapchainImageViews;

		//X This is to make able imageview be targeted as a frame buffer by presentation
		std::vector<VkFramebuffer> frameBuffers;

		VkFramebuffer currentFrameBuffer = nullptr;

		VkRenderPass renderPass = nullptr;
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

		std::unordered_map<unsigned int, std::vector<VkCommandBuffer>> commandBuffers;


		VkCommandPool presentCommandPool = nullptr;
		VkCommandBuffer presentCommandBuffer = nullptr;

		//! Sync
		VkSemaphore renderCompleteSemaphore = nullptr;
		VkSemaphore imageAcquiredSemaphore = nullptr;


		VkFence	mainQueueFinishedFence = nullptr;
		VkFence presentQueueFinishedFence = nullptr;
	};

	~RenderDevice();

	bool init() override;

	bool validate_swapchain();

	void beginFrame();

	void on_created();

	void pre_render();
	
	void render_ui(tf::Subflow& subflow);

	_INLINE_ VulkanRenderDevice& get_render_device()
	{
		return m_renderDevice;
	}

	void beginFrameW();

	void ready_ui_data();

	void set_next_image();

	void reset_things();

	void fill_and_execute_cmd();

	void render2();
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

	void swapbuffers();
	_INLINE_ VkSubmitInfo* get_main_submit_info(uint32_t cmdCount, VkCommandBuffer* buffs)
	{
		static VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		static VkSubmitInfo submitInfo
		{
			VK_STRUCTURE_TYPE_SUBMIT_INFO,
			nullptr,
			1,
			&m_renderDevice.imageAcquiredSemaphore,
			&waitStage,
			0,
			nullptr,
			1,
			&m_renderDevice.renderCompleteSemaphore
		};

		submitInfo.commandBufferCount = cmdCount;
		submitInfo.pCommandBuffers = buffs;

		return &submitInfo;
	}

	_F_INLINE_ ConfigProperty<bool>* get_quitting()
	{
		return &m_quitting;
	}

	_INLINE_ VkRenderPassBeginInfo* get_main_renderpass_begin_inf()
	{
		static VkClearValue clearValue
		{
			 { { 0.0f, 0.0f, 0.0f, 0.0f } }
		};

		static VkRenderPassBeginInfo beginInf{
			VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			nullptr,
			m_swapchain.renderPass,
			m_swapchain.currentFrameBuffer,
			{{(int32_t)0,(int32_t)0},{m_instance.surfaceExtent.width,m_instance.surfaceExtent.height}},
			1,
			&clearValue
		};

		beginInf.renderArea.extent = m_instance.surfaceExtent;
		beginInf.renderArea.offset = { 0,0 };

		beginInf.framebuffer = m_swapchain.currentFrameBuffer;
		return &beginInf;
	}
	bool init_vk_swapchain();

	void handleError();
	void fillCmd(VkCommandBuffer buff);
private:
	bool swapchain_needs_validate = false;

	VulkanInstance m_instance;
	// Initialize with save physical dev
	VulkanRenderDevice m_renderDevice;
	VulkanSwapChain m_swapchain;
	SwapChainSupportDetails m_swapChainDetails;
	bool m_instanceLoaded = false;
	bool m_isDebugEnabled = false;
	// These are loading methods for upper structs
private:
	bool init_vk_instance();
	bool init_vk_device();
	bool save_vk_physical_device();
	bool init_vk_logical_device();
	void expose_queues();
	bool create_command_pools();
	bool init_vk_syncs();
	bool init_command_buffers();
	void init_subs();
	bool init_renderpass();
	void on_size_changed(const GNF_UVec2& newSize);
	boost::signals2::connection windowResizeConnection;
	uint32_t currentImageIndex = 0;
	_INLINE_ void reset_cmd_pool(int index)
	{
		vkResetCommandPool(m_renderDevice.logicalDevice, m_renderDevice.mainQueueCommandPools[index], 0);
	}


	ImGuiDraw* imguiDraw;
	bool m_canContinue = true;

	ConfigProperty<bool> m_quitting;
private:
	friend class CreationServer;
	friend class ImGuiDraw;
};

#endif // RENDER_DEVICE_H