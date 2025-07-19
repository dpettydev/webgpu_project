cmake_minimum_required(VERSION 3.20)
project(WebGPUTriangle LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#Options for build type
option(BUILD_NATIVE "Build for native platform with Dawn" ON)
option(BUILD_EMSCRIPTEN "Build for web with Emscripten" OFF)

if(BUILD_EMSCRIPTEN AND BUILD_NATIVE)
    message(FATAL_ERROR "Cannot enable both BUILD_NATIVE and BUILD_EMSCRIPTEN simultaneously")
endif()

#Common source files
file(GLOB SOURCES
    src/*.cpp
    src/**/*.cpp
    src/core/*.cpp
    src/core/**/*.cpp
    src/system/*.cpp
    src/system/**/*.cpp
    src/render/*.cpp
    src/render/**/*.cpp
    src/util/*.cpp
    src/util/**/*.cpp
    src/util/dib/*.cpp
    src/util/dib/**/*.cpp
    src/extern/*.cpp
    src/extern/**/*.cpp
)

if(BUILD_NATIVE)
    #GLFW setup (manually downloaded)
    set(GLFW_DIR "${CMAKE_SOURCE_DIR}/extern/glfw")
    find_library(GLFW_LIBRARY NAMES glfw3 glfw
        PATHS "${GLFW_DIR}/lib" "${GLFW_DIR}/lib-vc2022" "${GLFW_DIR}/lib-vc2019"
        NO_DEFAULT_PATH
    )
    find_path(GLFW_INCLUDE_DIR NAMES GLFW/glfw3.h
        PATHS "${GLFW_DIR}/include"
        NO_DEFAULT_PATH
    )
    
    if(NOT GLFW_LIBRARY OR NOT GLFW_INCLUDE_DIR)
        message(FATAL_ERROR "GLFW not found in ${GLFW_DIR}. Ensure GLFW is extracted to extern/glfw with include and lib directories.")
    endif()
    
    #Dawn setup (manually built)
    set(DAWN_DIR "${CMAKE_SOURCE_DIR}/extern/dawn")
    find_path(DAWN_INCLUDE_DIR NAMES webgpu/webgpu_cpp.h
        PATHS "${DAWN_DIR}/include"
        NO_DEFAULT_PATH
    )
    find_path(DAWN_GEN_INCLUDE_DIR NAMES webgpu/webgpu.h
        PATHS "${DAWN_DIR}/include" "${DAWN_DIR}/out/Debug/gen/src"
        NO_DEFAULT_PATH
    )

    #Find Dawn libraries
    find_library(DAWN_NATIVE_LIB NAMES webgpu_dawn
        PATHS "${DAWN_DIR}/lib" "${DAWN_DIR}/out/Debug"
        NO_DEFAULT_PATH
    )
    find_library(DAWN_WIRE_LIB NAMES dawn_wire
        PATHS "${DAWN_DIR}/lib" "${DAWN_DIR}/out/Debug"
        NO_DEFAULT_PATH
    )
    find_library(DAWN_UTILS_LIB NAMES dawn_utils
        PATHS "${DAWN_DIR}/lib" "${DAWN_DIR}/out/Debug"
        NO_DEFAULT_PATH
    )
    find_library(DAWN_CPP_LIB NAMES dawncpp
        PATHS "${DAWN_DIR}/lib" "${DAWN_DIR}/out/Debug"
        NO_DEFAULT_PATH
    )
    find_library(WEBGPU_DAWN_LIB NAMES webgpu_dawn
        PATHS "${DAWN_DIR}/lib" "${DAWN_DIR}/out/Debug"
        NO_DEFAULT_PATH
    )
    find_library(WEBGPU_GLFW_LIB NAMES webgpu_glfw
        PATHS "${DAWN_DIR}/lib" "${DAWN_DIR}/out/Debug"
        NO_DEFAULT_PATH
    )

    if(NOT DAWN_INCLUDE_DIR)
        message(FATAL_ERROR "Dawn DAWN_INCLUDE_DIR are present.")
    endif()
    if(NOT DAWN_GEN_INCLUDE_DIR)
        message(FATAL_ERROR "Dawn DAWN_GEN_INCLUDE_DIR are present.")
    endif()
    if(NOT DAWN_NATIVE_LIB)
        message(FATAL_ERROR "Dawn DAWN_NATIVE_LIB are present.")
    endif()

    #Include directories
    include_directories(
        ${GLFW_INCLUDE_DIR}
        ${DAWN_INCLUDE_DIR}
        ${DAWN_GEN_INCLUDE_DIR}
    )

    #Define executable
    add_executable(${PROJECT_NAME} ${SOURCES})

    add_compile_options(-fno-fast-math -Wall -Wextra -O2)
    
    #Link libraries
    target_link_libraries(${PROJECT_NAME} PRIVATE
        ${GLFW_LIBRARY}
        ${DAWN_NATIVE_LIB}
        #${DAWN_WIRE_LIB}
        #${DAWN_UTILS_LIB}
        #${DAWN_CPP_LIB}
        #${WEBGPU_DAWN_LIB}
        d3d12
        dxgi
#        opengl32
    )

    #Platform-specific linking
    if(UNIX AND NOT APPLE)
        target_link_libraries(${PROJECT_NAME} PRIVATE X11)
    elseif(APPLE)
        target_link_libraries(${PROJECT_NAME} PRIVATE "-framework Cocoa -framework IOKit -framework CoreFoundation")
    endif()

    #if(MSVC)
        target_compile_definitions(${PROJECT_NAME} PUBLIC _WINDOWS)
    #endif()

    #Add compile definitions
    target_compile_definitions(${PROJECT_NAME} PRIVATE WEBGPU_BACKEND_DAWN)
else()
    if(NOT EMSCRIPTEN)
        message(FATAL_ERROR "Emscripten toolchain required for BUILD_EMSCRIPTEN")
    endif()

    #Define executable
    add_executable(${PROJECT_NAME} ${SOURCES})

    #Emscripten-specific settings
    set_target_properties(${PROJECT_NAME} PROPERTIES
        SUFFIX ".html"
        #LINK_FLAGS "-s USE_WEBGPU=1 -s USE_GLFW=3 -s EXPORTED_FUNCTIONS=['_main'] -s EXPORTED_RUNTIME_METHODS=['ccall','cwrap'] -s ENVIRONMENT=web -s MODULARIZE=1 -s EXPORT_NAME='createModule' -O3"
        LINK_FLAGS "-s USE_WEBGPU=1 -s USE_GLFW=3 -s EXPORTED_FUNCTIONS=['_main'] -s EXPORTED_RUNTIME_METHODS=['ccall','cwrap'] -s ASYNCIFY -s ENVIRONMENT=web -O3"
    )

    #Ensure WebGPU headers are included
    target_include_directories(${PROJECT_NAME} PRIVATE
        ${EMSCRIPTEN_SYSROOT}/cache/sysroot/include
    )
endif()

#Set include directories for the project
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
)

#VSCode-specific settings
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
#if(EXISTS "${CMAKE_SOURCE_DIR}/.vscode")
#    configure_file(${CMAKE_BINARY_DIR}/compile_commands.json ${CMAKE_SOURCE_DIR}/compile_commands.json COPYONLY)
#endif()

#Ensure Windows uses correct generator
if(BUILD_NATIVE)
    if(NOT CMAKE_GENERATOR MATCHES "Visual Studio" AND NOT CMAKE_GENERATOR MATCHES "Ninja")
    message(WARNING "Recommend using 'Ninja' or 'Visual Studio' generator for Windows. Run 'cmake -G Ninja' or use VSCode CMake Tools.")
    endif()
endif()




