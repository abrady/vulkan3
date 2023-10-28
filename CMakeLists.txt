cmake_minimum_required(VERSION 3.0.0)
project(Vulkan VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
enable_testing()

# CFLAGS = -std=c++20 -O2 -I/home/abrady/github/stb -I/home/abrady/github/tinyobjloader
# LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi


add_executable(Vulkan main.cpp Vulkan.cpp Vulkan.h)
target_link_libraries(Vulkan glfw dl pthread X11 Xxf86vm Xrandr Xi vulkan)
target_compile_features(Vulkan PUBLIC cxx_std_20)
target_include_directories(Vulkan PUBLIC /home/abrady/github/stb)
target_include_directories(Vulkan PUBLIC /home/abrady/github/tinyobjloader)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
