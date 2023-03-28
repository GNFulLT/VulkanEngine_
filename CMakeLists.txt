cmake_minimum_required(VERSION 3.5)

project(VulkanEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)



find_package(Vulkan REQUIRED FATAL_ERROR)
include(${CMAKE_CURRENT_LIST_DIR}/boost.cmake)


include(${CMAKE_CURRENT_LIST_DIR}/config/config.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/core/core.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/exception/exception.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/manager/manager.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/imgui/imgui.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/gui/gui.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/graphic/graphic.cmake)

file(GLOB VulkanEngine_SRC
    main.cpp
    ${VulkanEngine_CONFIG_SRC}
    ${VulkanEngine_CORE_SRC}
    ${VulkanEngine_EXCEPTION_SRC}
    ${VulkanEngine_MANAGER_SRC}
    ${VulkanEngine_IMGUI_SRC}
    ${VulkanEngine_GUI_SRC}
    ${VulkanEngine_GRAPHIC_SRC}
)



add_executable(VulkanEngine ${VulkanEngine_SRC})

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/deps/glfw)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/deps/spdlog)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/deps/taskflow)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/deps/mimalloc)

target_include_directories(VulkanEngine PUBLIC deps/mimalloc/include)
target_include_directories(VulkanEngine PUBLIC deps/vulkan/include)
target_include_directories(VulkanEngine PUBLIC deps/taskflow)
target_include_directories(VulkanEngine PUBLIC deps/glfw/include)
target_include_directories(VulkanEngine PUBLIC deps/spdlog/include)


target_link_libraries(VulkanEngine PUBLIC ${Vulkan_LIBRARIES})
target_link_libraries(VulkanEngine PRIVATE ${Boost_LIBRARIES})
target_link_libraries(VulkanEngine PUBLIC mimalloc-static)
target_link_libraries(VulkanEngine PRIVATE glfw)

install(TARGETS VulkanEngine
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
