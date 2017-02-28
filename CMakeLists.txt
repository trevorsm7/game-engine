cmake_minimum_required(VERSION 2.8.4)
set(TARGET_NAME engine)
project(${TARGET_NAME})

# Set build flags (use -g for debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall")

# Set project directories
set(SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)
set(INCLUDE_DIR ${PROJECT_SOURCE_DIR}/src)

# Find source files
file(GLOB SOURCE_FILES ${SOURCE_DIR}/*.cpp)
file(GLOB HEADER_FILES ${INCLUDE_DIR}/*.hpp)
include_directories(${INCLUDE_DIR})

# Add path for local cmake modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")

# Find cross-platform libraries
find_package(Lua REQUIRED)
find_package(OpenGL REQUIRED)
include_directories(${LUA_INCLUDE_DIR} ${OPENGL_INCLUDE_DIR})
set(ALL_LIBS ${LUA_LIBRARIES} ${OPENGL_LIBRARIES})

set(USE_SDL TRUE)
if(USE_SDL)
    # Find SDL libraries
    find_package(SDL2 REQUIRED)
    find_package(SDL2_mixer REQUIRED)
    include_directories(${SDL2_INCLUDE_DIR} ${SDL2_MIXER_INCLUDE_DIR})
    set(ALL_LIBS ${ALL_LIBS} ${SDL2_LIBRARIES} ${SDL2_MIXER_LIBRARIES})

    # Find engine wrapper files
    file(GLOB SDL_SOURCE ${SOURCE_DIR}/SDL/*.cpp)
    set(SOURCE_FILES ${SOURCE_FILES} ${SDL_SOURCE})
    file(GLOB SDL_HEADERS ${INCLUDE_DIR}/SDL/*.hpp)
    set(HEADER_FILES ${HEADER_FILES} ${SDL_HEADERS})
    include_directories(${SOURCE_DIR}/SDL)
    add_definitions(-DPLATFORM_SDL)
endif()

if(USE_GLFW)
    # Find GLFW libraries
    find_library(GLFW glfw3 PATHS ${PROJECT_SOURCE_DIR}/lib)
    include_directories(${PROJECT_SOURCE_DIR}/lib)
    set(ALL_LIBS ${ALL_LIBS} ${GLFW})

    # Find engine wrapper files
    file(GLOB GLFW_SOURCE ${SOURCE_DIR}/GLFW/*.cpp)
    set(SOURCE_FILES ${SOURCE_FILES} ${GLFW_SOURCE})
    file(GLOB GLFW_HEADERS ${INCLUDE_DIR}/GLFW/*.hpp)
    set(HEADER_FILES ${HEADER_FILES} ${GLFW_HEADERS})
    include_directories(${SOURCE_DIR}/GLFW)
    add_definitions(-DPLATFORM_GLFW)
endif()

# Find platform-specific libraries
if(APPLE)
    find_library(COCOA Cocoa)
    find_library(IOKIT IOKit)
    find_library(CORE_VIDEO CoreVideo)
    set(ALL_LIBS ${ALL_LIBS} ${COCOA} ${IOKIT} ${CORE_VIDEO})
endif()

add_executable(${TARGET_NAME} ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(${TARGET_NAME} ${ALL_LIBS})
