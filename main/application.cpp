#include "application.h"

#include "../manager/memory_manager.h"
#include "../manager/logger_manager.h"
#include "../manager/window_manager.h"
#include "../manager/rendering/render_device.h"
#include "../manager/resource_manager.h"
#include "../manager/timer_manager.h"
#include "../manager/rendering/render_scene_ui.h"
int Application::run()
{
	auto initCode = before_start();

	if (initCode != 0)
	{
		return initCode;
	}

	auto timerManager = TimerManager::get_singleton();
	m_applicationStartupTime = timerManager->get_cpu_timer_time();

	WindowManager::get_singleton()->show();
	
	while (!m_wantsExit && !WindowManager::get_singleton()->wants_close())
	{
		//X Can handle different thread. Need benchmarks 
		auto deltaTime = timerManager->calculate_delta_time();
		int fps = 0;
		auto inSecond = timerManager->calc_fps(fps);
		
		process_events();

		if (inSecond)
		{
			fixed_update_impl();
		}

		update();

		if (begin_frame())
		{
			render();

			after_frame();
		}

	}


	//X DEALLOCATIONS
}

int Application::before_start()
{
	auto singl = MemoryManager::create_singleton();
	MemoryManager::set_singleton(singl);
	
	TimerManager* sng = singl->create_singleton_object<TimerManager>("TimerManager");
	TimerManager::set_singleton(sng);

	ThreadPoolManager* singltpm = singl->create_singleton_object<ThreadPoolManager>("ThreadPoolManager");
	ThreadPoolManager::set_singleton(singltpm);
	
	if (singltpm->init() != 0)
	{
		return -1;
	}

	auto wndm = MemoryManager::get_singleton()->create_singleton_object<WindowManager>("WindowManager");
	WindowManager::set_singleton(wndm);
	if (wndm->init() >= 0)
	{
		m_managerLoads[APPLICATION_MANAGER::APPLICATION_MANAGER_WINDOW_MANAGER].store(true);
	}


	//X Thread Management higher priorty
	m_loadedSystemManagers.push(singltpm);

	auto loggingm = singl->create_singleton_object<LoggerManager>("LoggerManager");
	
	if (loggingm->init() != 0)
	{
		return -1;
	}
	tf::Taskflow managersInitFlow;

	//X TODO : Initialize Managers
	
	auto ftr = initialize_managers(managersInitFlow);

	//X TODO : Do Version checks in here



	//X Manager Checks goes here
	ftr.wait();

	//x Check there is any manager that is not initialized

	bool initedAll = true;
	for (int i = 0; i < APPLICATION_MANAGER::APPLICATION_MANAGER_COUNT; i++)
	{
		if (!m_managerLoads[i].load())
		{
			LoggerManager::get_singleton()->log_cout(LoggerManager::get_singleton(), APPLICATION_MANAGER::_from_index(i)._to_string(),Logger::ERROR);
			initedAll = false;
		}
	}

	if (!initedAll)
	{
		return -1;
	}

	//X Bind Functions

	process_events = std::bind(&WindowManager::handle_window_events, WindowManager::get_singleton());
	present_image = std::bind(&RenderDevice::swapbuffers, RenderDevice::get_singleton());
	set_next_image = std::bind(&RenderDevice::set_next_image, RenderDevice::get_singleton());
	//X TODO : If all managers can be initalized try to initalize application specific init
	
	render_scene_impl();

	//X Set Renderpass of swapchain
	//m_renderScene->set_vulkan_renderpass();
	return before_start_impl();
}

void Application::update()
{
	update_impl();
}

bool Application::begin_frame()
{
	RenderDevice::get_singleton()->reset_things();
	set_next_image();
	return begin_frame_impl();
}

void Application::render()
{
	//X CPU Profilers

	//X Custom Render Impl
	render_impl();


	//X Main render Function needs to swapchain
	m_renderScene->fill_cmd(RenderDevice::get_singleton()->get_render_device().pSceneCommandBuffer);

	//X Submit task
	VkSubmitInfo inf = {};
	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	inf.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	inf.pNext = nullptr;
	inf.commandBufferCount = 1;
	inf.pCommandBuffers = &RenderDevice::get_singleton()->get_render_device().pSceneCommandBuffer;
	inf.signalSemaphoreCount = 1;
	inf.pSignalSemaphores = &RenderDevice::get_singleton()->get_render_device().renderCompleteSemaphore;
	inf.pWaitDstStageMask = &waitStage;
	vkQueueSubmit(RenderDevice::get_singleton()->get_render_device().mainQueue, 1, &inf, RenderDevice::get_singleton()->get_render_device().mainQueueFinishedFence);

	//X Doesn't need this. It will be needed when changing the render flow
}

void Application::after_frame()
{
	after_frame_impl();

	present_image();
}

void Application::before_exit()
{
	before_exit_impl();
}

tf::Future<void> Application::initialize_managers(tf::Taskflow& managersInitFlow)
{
	auto logm = managersInitFlow.emplace([systems = &(this->m_managerLoads)]() {
		auto loggingm = MemoryManager::get_singleton()->create_singleton_object<LoggerManager>("LoggerManager");
		LoggerManager::set_singleton(loggingm);
		if (loggingm->init() >= 0)
		{
			systems->operator[](APPLICATION_MANAGER::APPLICATION_MANAGER_LOGGER_MANAGER).store(true);
		}
	});

	auto wndrndrm = managersInitFlow.emplace([systems = &(this->m_managerLoads)]() {
		
		auto renderDevice = MemoryManager::get_singleton()->create_singleton_object<RenderDevice>("RenderDevice");
		RenderDevice::set_singleton(renderDevice);
		if (renderDevice->init() >= 0)
		{
			systems->operator[](APPLICATION_MANAGER::APPLICATION_MANAGER_RENDER_DEVICE).store(true);
		}
		else
		{
			return;
		}
	});

	auto rsrcmng = managersInitFlow.emplace([systems = &(this->m_managerLoads)]() {
		auto resourcem = MemoryManager::get_singleton()->create_singleton_object<ResourceManager>("ResourceManager");
		ResourceManager::set_singleton(resourcem);
		if (resourcem->init() >= 0)
		{
			systems->operator[](APPLICATION_MANAGER::APPLICATION_MANAGER_RESOURCE_MANAGER).store(true);
		}
		});

	return ThreadPoolManager::get_singleton()->run_flow(managersInitFlow);
}

void Application::render_scene_impl()
{
	m_renderScene = MemoryManager::get_singleton()->new_object<RenderSceneUI>("RenderSceneUI");
	m_renderScene->init(*WindowManager::get_singleton()->get_size()->get(), &RenderDevice::get_singleton()->get_swapchain()->renderPass);
}
