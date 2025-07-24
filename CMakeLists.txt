cmake_minimum_required(VERSION 3.10)
project(ComputerGraphics)

set(CMAKE_CXX_STANDARD 17)

# Add include directories
include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/external/glad/include
)

# Add source files
add_executable(ComputerGraphics
    src/main.cpp
    external/glad/src/glad.c
    # add other .cpp files here...
)

# Link OpenGL
find_package(OpenGL REQUIRED)
target_link_libraries(ComputerGraphics ${OPENGL_LIBRARIES})
