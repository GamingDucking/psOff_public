cmake_minimum_required(VERSION 3.24)
include(../setupModule.cmake)

set(libName libScePad)
project(${libName})

add_library(${libName} SHARED entry.cpp)

add_dependencies(${libName} core)
target_link_libraries(${libName} PRIVATE glfw3 ${Vulkan_LIBRARIES} core.lib)

setupModule(${libName})