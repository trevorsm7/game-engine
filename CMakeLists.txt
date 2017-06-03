cmake_minimum_required(VERSION 3.0.0)
set(TARGET_NAME engine)
project(${TARGET_NAME} CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS FALSE)

# Add path for local cmake modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")

# Find source files
file(GLOB SOURCE_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp)
file(GLOB HEADER_FILES ${PROJECT_SOURCE_DIR}/src/*.hpp)
add_executable(${TARGET_NAME} ${SOURCE_FILES} ${HEADER_FILES})
target_include_directories(${TARGET_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/src)

# Find Lua (can set env LUA_DIR)
find_package(Lua REQUIRED)
add_library(Lua UNKNOWN IMPORTED)
set_property(TARGET Lua PROPERTY IMPORTED_LOCATION ${LUA_LIBRARIES})
set_property(TARGET Lua PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${LUA_INCLUDE_DIR})
target_link_libraries(${TARGET_NAME} PRIVATE Lua)

# Find OpenGL
find_package(OpenGL REQUIRED)
add_library(OpenGL INTERFACE)
target_link_libraries(OpenGL INTERFACE ${OPENGL_LIBRARIES})
target_include_directories(OpenGL INTERFACE ${OPENGL_INCLUDE_DIR})

# Find platform-specific libraries
if(APPLE)
    find_library(COCOA Cocoa)
    find_library(IOKIT IOKit)
    find_library(CORE_VIDEO CoreVideo)
    target_link_libraries(${TARGET_NAME} PRIVATE ${COCOA} ${IOKIT} ${CORE_VIDEO})
endif()

option(USE_SDL "Compile with SDL front-end" TRUE)
if(USE_SDL)
    # Find SDL libraries
    find_package(SDL2 REQUIRED)
    find_package(SDL2_mixer REQUIRED)
    add_library(SDL2 INTERFACE)
    target_link_libraries(SDL2 INTERFACE ${SDL2_LIBRARIES} ${SDL2_MIXER_LIBRARIES})
    target_include_directories(SDL2 INTERFACE ${SDL2_INCLUDE_DIR} ${SDL2_MIXER_INCLUDE_DIR})

    # Find engine wrapper files
    file(GLOB SDL_SOURCE ${PROJECT_SOURCE_DIR}/src/SDL/*.cpp)
    file(GLOB SDL_HEADERS ${PROJECT_SOURCE_DIR}/src/SDL/*.hpp)
    add_library(SDLFrontEnd STATIC ${SDL_SOURCE} ${SDL_HEADERS})
    target_link_libraries(SDLFrontEnd PUBLIC SDL2 Lua OpenGL)
    target_include_directories(SDLFrontEnd PUBLIC ${PROJECT_SOURCE_DIR}/src/SDL PRIVATE ${PROJECT_SOURCE_DIR}/src)
    target_compile_definitions(SDLFrontEnd INTERFACE -DPLATFORM_SDL)

    target_link_libraries(${TARGET_NAME} PRIVATE SDLFrontEnd)
endif()

option(USE_GLFW "Compile with GLFW front-end" FALSE)
if(USE_GLFW)
    # Find GLFW libraries
    find_library(GLFW_LIBRARY glfw3 PATHS ${PROJECT_SOURCE_DIR}/lib)
    add_library(GLFW INTERFACE)
    target_link_libraries(GLFW INTERFACE ${GLFW_LIBRARY})
    target_include_directories(GLFW INTERFACE ${PROJECT_SOURCE_DIR}/lib)

    # Find engine wrapper files
    file(GLOB GLFW_SOURCE ${PROJECT_SOURCE_DIR}/src/GLFW/*.cpp)
    file(GLOB GLFW_HEADERS ${PROJECT_SOURCE_DIR}/src/GLFW/*.hpp)
    add_library(GLFWFrontEnd STATIC ${GLFW_SOURCE} ${GLFW_HEADERS})
    target_link_libraries(GLFWFrontEnd PUBLIC GLFW Lua OpenGL)
    target_include_directories(GLFWFrontEnd PUBLIC ${PROJECT_SOURCE_DIR}/src/GLFW PRIVATE ${PROJECT_SOURCE_DIR}/src)
    target_compile_definitions(GLFWFrontEnd INTERFACE -DPLATFORM_GLFW)

    target_link_libraries(${TARGET_NAME} PRIVATE GLFWFrontEnd)
endif()
