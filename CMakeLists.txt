cmake_minimum_required(VERSION 2.8.4)
set(TARGET_NAME engine)
project(${TARGET_NAME})

# Set project directories
set(BUILD_DIR ${PROJECT_SOURCE_DIR}/build)
set(SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)
set(INCLUDE_DIR ${PROJECT_SOURCE_DIR}/src)
set(LIBRARY_DIR ${PROJECT_SOURCE_DIR}/lib)
#set(ASSETS_DIR ${PROJECT_SOURCE_DIR}/assets)
include_directories(${SOURCE_DIR} ${LIBRARY_DIR})

# Find source files
file(GLOB SOURCE_FILES ${SOURCE_DIR}/*.cpp)

# Find libraries
find_library(COCOA Cocoa)
find_library(IOKIT IOKit)
find_library(CORE_VIDEO CoreVideo)
find_library(OPENGL OpenGL)
find_library(LUA lua PATHS ${LIBRARY_DIR})
set(ALL_LIBS ${COCOA} ${IOKIT} ${CORE_VIDEO} ${OPENGL} ${LUA})

set(USE_SDL TRUE)
if(USE_SDL)
    find_library(SDL SDL2)
    find_library(MIX SDL2_mixer)
    set(ALL_LIBS ${ALL_LIBS} ${SDL} ${MIX})
    file(GLOB SDL_SOURCE ${SOURCE_DIR}/SDL/*.cpp)
    set(SOURCE_FILES ${SOURCE_FILES} ${SDL_SOURCE})
    include_directories(${SOURCE_DIR}/SDL)
    add_definitions(-DPLATFORM_SDL)
endif()

if(USE_GLFW)
    find_library(GLFW glfw3 PATHS ${LIBRARY_DIR})
    set(ALL_LIBS ${ALL_LIBS} ${GLFW})
    file(GLOB GLFW_SOURCE ${SOURCE_DIR}/GLFW/*.cpp)
    set(SOURCE_FILES ${SOURCE_FILES} ${GLFW_SOURCE})
    include_directories(${SOURCE_DIR}/GLFW)
    add_definitions(-DPLATFORM_GLFW)
endif()

# Copy assets to build folder
#file(GLOB COPY_FILES ${ASSETS_DIR}/*.lua)
#file(COPY ${COPY_FILES} DESTINATION ${BUILD_DIR})

# Create build rules (use flag -g for debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BUILD_DIR})
add_executable(${TARGET_NAME} ${SOURCE_FILES})
target_link_libraries(${TARGET_NAME} ${ALL_LIBS})
