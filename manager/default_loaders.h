#ifndef DEFAULT_LOADERS_H
#define DEFAULT_LOADERS_H

#include <concepts>

#include "glfw_window.h"
#include "memory_manager.h"
#include "logger_manager.h"
#include "rendering/presentation_ui_manager.h"
template<typename T>
concept SysManager = std::convertible_to<T*, SystemManager*>;

template<SysManager manager>
void create_manager_singleton_default();

template<>
void create_manager_singleton_default<WindowManager>()
{
	auto wndm = MemoryManager::get_singleton()->create_singleton_object<GlfwWindowManager>("GlfwWindowManager");
	WindowManager::set_singleton(wndm);
}


template<>
void create_manager_singleton_default<LoggerManager>()
{
	auto logger = MemoryManager::get_singleton()->create_singleton_object<LoggerManager>("LoggerManager");
	LoggerManager::set_singleton(logger);
}
template<>
void create_manager_singleton_default<PresentationManager>()
{
	auto presentation = MemoryManager::get_singleton()->create_singleton_object<PresentationUIManager>("PresentationUI");
	PresentationManager::set_singleton(presentation);
}

#endif // DEFAULT_LOARDERS_H
