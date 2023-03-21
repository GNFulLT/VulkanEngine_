#include <iostream>

#include "manager/memory_manager.h"
#include "manager/logger_manager.h"
#include "manager/window_manager.h"
#include "manager/rendering/render_device.h"
#include "manager/thread_pool_manager.h"

#include "gui/windows/debug_window.h"
#include "gui/windows/text_editor_window.h"
#include "gui/windows/scene_window.h"

#include <stack>

MemoryManager* memory_manager;
LoggerManager* logger_manager;
ThreadPoolManager* thread_pool_manager;
WindowManager* window_manager;
RenderDevice* render_device;
int main()
{
    mi_stats_reset();  // ignore earlier allocations

    std::stack<SystemManager*> manager_stack;

    memory_manager = MemoryManager::create_singleton();

    logger_manager = memory_manager->create_singleton_object<LoggerManager>("LoggerManager");

    logger_manager->set_log_level_cout(Logger::DEBUG);

    logger_manager->log_cout(logger_manager,"LoggerManager Created",Logger::DEBUG);
       
    LoggerManager::set_singleton(logger_manager);

    manager_stack.push(logger_manager);
    
    thread_pool_manager = memory_manager->create_singleton_object<ThreadPoolManager>("ThreadPoolManager");

    thread_pool_manager->init();

    ThreadPoolManager::set_singleton(thread_pool_manager);

    manager_stack.push(thread_pool_manager);

    window_manager = memory_manager->create_singleton_object<WindowManager>("WindowManager");

    manager_stack.push(window_manager);

    bool inited = window_manager->init();
    
    WindowManager::set_singleton(window_manager);

    render_device = memory_manager->create_singleton_object<RenderDevice>("RenderDevice");

    RenderDevice::set_singleton(render_device);

    inited = render_device->init();
   

    manager_stack.push(render_device);

    if (inited)
    {
        tf::Taskflow renderTask;

        renderTask.emplace([dev = render_device]() {
            dev->beginFrame();
            //fture = pool->run_flow(flow);
            dev->ready_ui_data();
            });

        renderTask.emplace([dev = render_device]() {
            dev->reset_things();

            });
        renderTask.emplace([dev = render_device]() {
            dev->set_next_image();

            });

        tf::Taskflow preRenderTask;

        preRenderTask.emplace([dev = render_device]()
            {
                dev->validate_swapchain();

            });

        preRenderTask.emplace([dev = render_device]()
            {
                dev->pre_render();
            });

        TextEditorWindow* window;
        SceneWindow* scene_window;

        window_manager->create_window("Zort",&window);
        window_manager->register_window(window);
        window->set_file_to_edit("./text_edit.glsl");
        
        window_manager->create_window("Scene", &scene_window, memory_manager->new_object<RenderScene>("SceneRenderer"));
        window_manager->register_window(scene_window);

        //auto langDef = TextEditor::LanguageDefinition::GLSL();
        //window->SetLanguageDefinition(langDef);

       // const char* file_to_edit = "./text_edit.lsl";*/
        
        window_manager->show();

        // On Created
        {
            render_device->on_created();

            render_device->beginFrameW();
            render_device->pre_render();
        }


        while (!window_manager->wants_close())
        {
            window_manager->handle_window_events();
            if (window_manager->need_render())
            {
                if (render_device->does_swapchain_need_validate())
                {
                    render_device->beginFrameW();

                    thread_pool_manager->run_flow(preRenderTask).wait();


                }

                render_device->beginFrameW();

                thread_pool_manager->run_flow(renderTask).wait();

                render_device->fill_and_execute_cmd();

                render_device->swapbuffers();
            }
           
        }
    }
    


    while (!manager_stack.empty())
    {
        auto top = manager_stack.top();
        top->destroy();
        manager_stack.pop();
    }


    memory_manager->destroy();
    mi_stats_print(NULL);

    return 0;
}
