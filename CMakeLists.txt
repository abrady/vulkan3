cmake_minimum_required(VERSION 3.0.0)
project(Vulkan VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
enable_testing()

# CFLAGS = -std=c++20 -O2 -I/home/abrady/github/stb -I/home/abrady/github/tinyobjloader
# LDFLAGS = -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

message(STATUS "CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")
add_executable(Vulkan main.cpp Vulkan.cpp Vulkan.h)
target_link_libraries(Vulkan glfw dl pthread X11 Xxf86vm Xrandr Xi vulkan)
target_compile_features(Vulkan PUBLIC cxx_std_20)
target_include_directories(Vulkan PUBLIC /home/abrady/github/stb)
target_include_directories(Vulkan PUBLIC /home/abrady/github/tinyobjloader)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

# Declare the list here in the global scope
set(SPIRV_BINARY_FILES "")

# Function to compile shaders in a given source directory
function(compile_shaders SHADER_TYPE)
    # Directory for compiled shaders
    set(SHADER_OUTPUT_DIR "${CMAKE_BINARY_DIR}/shaders/${SHADER_TYPE}")

    # Make sure the directory exists
    file(MAKE_DIRECTORY ${SHADER_OUTPUT_DIR})

    # Directory containing GLSL source shaders
    set(SHADER_SOURCE_DIR "${CMAKE_SOURCE_DIR}/shaders/${SHADER_TYPE}")
    message(STATUS "Compiling shaders in ${SHADER_SOURCE_DIR}")

    # Collect all GLSL source files
    file(GLOB SHADER_SOURCES ${SHADER_SOURCE_DIR}/*.glsl)

    # Compile each GLSL source file
    foreach(SHADER ${SHADER_SOURCES})
        #message(STATUS "Compiling shader ${SHADER}")
        get_filename_component(SHADER_NAME ${SHADER} NAME_WE)
        set(SPIRV "${SHADER_OUTPUT_DIR}/${SHADER_NAME}.spv")
        #message(STATUS "Compiling shader ${SHADER} to ${SPIRV}")

        add_custom_command(
            OUTPUT ${SPIRV}
            COMMAND echo "Compiling shader ${SHADER} to ${SPIRV}"
            COMMAND glslc -fshader-stage=${SHADER_TYPE} ${SHADER} -o ${SPIRV}
            DEPENDS ${SHADER}
            IMPLICIT_DEPENDS CXX ${SHADER}
            VERBATIM
        )
        list(APPEND SPIRV_BINARY_FILES ${SPIRV})
    endforeach(SHADER)
    # Make the changes visible in the parent scope
    set(SPIRV_BINARY_FILES ${SPIRV_BINARY_FILES} PARENT_SCOPE)
endfunction()

# Compile vertex shaders
compile_shaders("vert")

# Compile fragment shaders
compile_shaders("frag")

# Dummy target to run the shader compilation
message(STATUS "SPIRV_BINARY_FILES: ${SPIRV_BINARY_FILES}")
add_custom_target(
    Shaders
    DEPENDS ${SPIRV_BINARY_FILES}
)
add_dependencies(Vulkan Shaders)
