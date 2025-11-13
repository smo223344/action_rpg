cmake_minimum_required(VERSION 3.15)
project(ActionRPG VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# GLFW
find_package(glfw3 3.3 REQUIRED)

# OpenGL
find_package(OpenGL REQUIRED)

# GLM - header-only library
find_package(glm QUIET)

# GLAD (we'll include this as source)
set(GLAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/glad)

# Source files
set(SOURCES
    src/main.cpp
    src/renderer.cpp
    src/game.cpp
    src/entity.cpp
    src/input.cpp
    ${GLAD_DIR}/src/glad.c
)

set(HEADERS
    include/renderer.h
    include/game.h
    include/entity.h
    include/input.h
)

# Executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${GLAD_DIR}/include
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    glfw
    OpenGL::GL
    ${CMAKE_DL_LIBS}
)

# Platform-specific settings
if(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE m pthread)
endif()

# Compiler warnings
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()
