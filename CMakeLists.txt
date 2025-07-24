cmake_minimum_required(VERSION 3.10)
project(ComputerGraphics)

set(CMAKE_CXX_STANDARD 17)

# --- Add all include directories ---
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/external/glad/include
)

# --- Find required packages ---
# Find OpenGL
find_package(OpenGL REQUIRED)

# Find GLFW using pkg-config
find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)
include_directories(${GLFW_INCLUDE_DIRS})

# Find Assimp using pkg-config (This is the corrected part)
pkg_search_module(assimp REQUIRED assimp)
include_directories(${assimp_INCLUDE_DIRS})


# --- Add executable source files ---
add_executable(ComputerGraphics
    src/main.cpp
    external/glad/src/glad.c
)

# --- Link all libraries to the executable ---
# This single command links everything your project needs.
target_link_libraries(ComputerGraphics PRIVATE 
    ${OPENGL_LIBRARIES} 
    ${GLFW_LIBRARIES} 
    ${assimp_LIBRARIES}
)