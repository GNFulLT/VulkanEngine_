#ifndef APPLICATON_H
#define APPLICATON_H

#include "../core/typedefs.h"
#include "../manager/system_manager.h"
#include "../manager/thread_pool_manager.h"

#include "boost/date_time/posix_time/posix_time.hpp"
#include "../core/serialize/serializable_enum.h"

#include "../manager/rendering/render_scene.h"

#include <boost/timer/timer.hpp>
#include <boost/chrono.hpp>

#include <functional>
#include <atomic>
#include <stack>
#include <utility>
#include <optional>

BETTER_ENUM(APPLICATION_MANAGER, uint8_t,
	APPLICATION_MANAGER_LOGGER_MANAGER,
	APPLICATION_MANAGER_WINDOW_MANAGER,
	APPLICATION_MANAGER_RENDER_DEVICE,
	APPLICATION_MANAGER_PRESENTATION_MANAGER,
	APPLICATION_MANAGER_RESOURCE_MANAGER,
	APPLICATION_MANAGER_COUNT)

using ManagerFactoryCreatorFnc = std::function<SystemManager*()>;
using ManagerFactoryDestroyerFnc = std::function<void(SystemManager*)>;


class Application
{
public:
	virtual ~Application() = default;
	
	_F_INLINE_ static void exit() _NO_EXCEPT_
	{
		m_wantsExit = true;
	}

	int run();
	int before_start();

	static void add_wait_semaphore(VkSemaphore semaphore);
	static void add_signal_semaphore(VkSemaphore semaphore);
	//X Update
	void update();

	bool begin_frame();

	//X Render
	void render();

	void after_frame();

	void before_exit();
protected:
	void set_factory(APPLICATION_MANAGER::_enumerated manager, const std::pair<ManagerFactoryCreatorFnc, ManagerFactoryDestroyerFnc>& factory);

	virtual int before_start_impl() { return 0; }
	virtual void update_impl() {}
	virtual void render_impl() {}
	virtual bool begin_frame_impl() { return true; }
	//X Update that occurs in every 1 second.
	virtual void fixed_update_impl() {}

	virtual void before_exit_impl() {}
	virtual void after_frame_impl() {}
	

	virtual void override_loaders() {};

	virtual void render_scene_impl();


	std::function<void()> process_events;
	std::function<void()> present_image;
	std::function<void()> set_next_image;
	std::function<bool()> need_handle_for_resize;
	std::function<void()> handle_resize;
	std::function<bool()> needed_render;
	std::mutex m_managerMutex;

	RenderScene* m_renderScene = nullptr;
private:
	tf::Future<void> initialize_managers(tf::Taskflow& flow);
private:
	std::array<std::atomic_bool, APPLICATION_MANAGER::APPLICATION_MANAGER_COUNT> m_managerLoads;
	std::array<std::optional<std::pair<ManagerFactoryCreatorFnc, ManagerFactoryDestroyerFnc>>, APPLICATION_MANAGER::APPLICATION_MANAGER_COUNT> m_managerFactories;
	boost::timer::cpu_times m_applicationStartupTime;
	std::stack<SystemManager*> m_loadedSystemManagers;
	inline static volatile bool m_wantsExit = false;
	inline static std::vector<VkSemaphore> m_waitSemaphores;
	inline static std::vector<VkSemaphore> m_signalSemaphores;

};


#endif // APPLICATON_H