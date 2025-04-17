cmake_minimum_required(VERSION 3.10)

# Check for Unix platform - fail on Windows
if(WIN32)
    message(FATAL_ERROR "DevToolsTerminal is currently only supported on Unix-based systems (Linux/macOS)")
endif()

# Set the project name
project(DevToolsTerminal)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Platform-specific configurations
if(APPLE)
    set(MACOS_OUTPUT_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/macos)
    set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "Build architectures for macOS")
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum macOS deployment version")
endif()

# Linux build configuration
set(LINUX_OUTPUT_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/linux)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 20) # Ensure C++17 standard is used
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include the header files directory
include_directories(src/include)

# Add ExternalProject support
include(ExternalProject)

# Function to set up CURL download and build
function(setup_curl_download)
    message(STATUS "CURL not found - will download and build from source")
    ExternalProject_Add(curl_external
        URL https://curl.se/download/curl-8.1.2.tar.gz
        PREFIX ${CMAKE_BINARY_DIR}/external/curl
        CMAKE_ARGS
            -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/external/curl/install
            -DBUILD_SHARED_LIBS=OFF
            -DCMAKE_USE_OPENSSL=ON
            -DBUILD_TESTING=OFF
        )
    set(CURL_INCLUDE_DIRS ${CMAKE_BINARY_DIR}/external/curl/install/include PARENT_SCOPE)
    set(CURL_LIBRARIES ${CMAKE_BINARY_DIR}/external/curl/install/lib/${CMAKE_STATIC_LIBRARY_PREFIX}curl${CMAKE_STATIC_LIBRARY_SUFFIX} PARENT_SCOPE)
    
    # Fix: Don't create the imported target here - it will be created after the external project is built
    set(CURL_EXTERNAL_BUILT TRUE PARENT_SCOPE)
endfunction()

# Function to set up raylib download and build
function(setup_raylib_download)
    message(STATUS "raylib not found - will download and build from source")
    ExternalProject_Add(raylib_external
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 4.5.0
        PREFIX ${CMAKE_BINARY_DIR}/external/raylib
        CMAKE_ARGS
            -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/external/raylib/install
            -DBUILD_SHARED_LIBS=OFF
            -DUSE_EXTERNAL_GLFW=OFF
            -DBUILD_EXAMPLES=OFF
        )
    set(RAYLIB_INCLUDE_DIRS ${CMAKE_BINARY_DIR}/external/raylib/install/include PARENT_SCOPE)
    set(RAYLIB_LIBRARIES ${CMAKE_BINARY_DIR}/external/raylib/install/lib/${CMAKE_STATIC_LIBRARY_PREFIX}raylib${CMAKE_STATIC_LIBRARY_SUFFIX} PARENT_SCOPE)
    
    # Fix: Don't create the imported target here - it will be created after the external project is built
    set(RAYLIB_EXTERNAL_BUILT TRUE PARENT_SCOPE)
    
    # Additional dependencies needed by raylib on different platforms
    if(APPLE)
        find_library(COCOA_LIBRARY Cocoa)
        find_library(OPENGL_LIBRARY OpenGL)
        find_library(IOKIT_LIBRARY IOKit)
        find_library(COREVIDEO_LIBRARY CoreVideo)
        set(PLATFORM_LIBRARIES ${COCOA_LIBRARY} ${OPENGL_LIBRARY} ${IOKIT_LIBRARY} ${COREVIDEO_LIBRARY} PARENT_SCOPE)
    elseif(UNIX AND NOT APPLE)
        find_package(OpenGL REQUIRED)
        find_package(X11 REQUIRED)
        set(PLATFORM_LIBRARIES ${OPENGL_LIBRARIES} ${X11_LIBRARIES} pthread dl rt PARENT_SCOPE)
    endif()
endfunction()

# Try to find CURL, download if not found
find_package(CURL QUIET)
if(NOT CURL_FOUND)
    setup_curl_download()
    
    # Create the imported target outside of the function to ensure it exists
    if(CURL_EXTERNAL_BUILT)
        add_library(CURL::libcurl STATIC IMPORTED)
        set_target_properties(CURL::libcurl PROPERTIES
            IMPORTED_LOCATION "${CURL_LIBRARIES}"
            INTERFACE_INCLUDE_DIRECTORIES "${CURL_INCLUDE_DIRS}")
        add_dependencies(CURL::libcurl curl_external)
    endif()
endif()

# Try to find raylib, download if not found
find_package(raylib QUIET)
if(NOT raylib_FOUND)
    setup_raylib_download()
    
    # Create the imported target outside of the function to ensure it exists
    if(RAYLIB_EXTERNAL_BUILT)
        add_library(raylib STATIC IMPORTED)
        set_target_properties(raylib PROPERTIES
            IMPORTED_LOCATION "${RAYLIB_LIBRARIES}"
            INTERFACE_INCLUDE_DIRECTORIES "${RAYLIB_INCLUDE_DIRS}")
        add_dependencies(raylib raylib_external)
    endif()
endif()

find_package(Threads REQUIRED)  # Add this to find pthread library

# Source files
set(SOURCE_FILES
    src/main.cpp
    src/terminalpassthrough.cpp
    src/openaipromptengine.cpp
    src/pluginmanager.cpp
    src/thememanager.cpp
)

# Add the include directories for CURL
include_directories(${CURL_INCLUDE_DIRS})

# Add the include directories for raylib
if(raylib_FOUND)
    include_directories(${RAYLIB_INCLUDE_DIR})
else()
    include_directories(${RAYLIB_INCLUDE_DIRS})
endif()

# Find nlohmann_json package (Unix systems only)
find_path(JSON_INCLUDE_DIR NAMES nlohmann/json.hpp
    PATHS
    /usr/include
    /usr/local/include
    /opt/include
    /opt/local/include
    ${CMAKE_SOURCE_DIR}/external
)

if(JSON_INCLUDE_DIR)
    include_directories(${JSON_INCLUDE_DIR})
else()
    message(WARNING "nlohmann/json.hpp not found, using fallback include location")
    include_directories("/usr/include/nlohmann")
endif()

# Find dl library (needed for dynamic loading in pluginmanager)
find_library(DL_LIBRARY dl)
if(NOT DL_LIBRARY)
    message(FATAL_ERROR "dl library not found")
endif()

# Option for static linking (better portability across Linux distributions)
option(STATIC_LINKING "Link libraries statically for better portability" OFF)

# Platform-specific libraries for raylib
if(NOT PLATFORM_LIBRARIES)
    if(APPLE)
        find_library(COCOA_LIBRARY Cocoa)
        find_library(OPENGL_LIBRARY OpenGL)
        find_library(IOKIT_LIBRARY IOKit)
        find_library(COREVIDEO_LIBRARY CoreVideo)
        set(PLATFORM_LIBRARIES ${COCOA_LIBRARY} ${OPENGL_LIBRARY} ${IOKIT_LIBRARY} ${COREVIDEO_LIBRARY})
    elseif(UNIX AND NOT APPLE)
        find_package(OpenGL REQUIRED)
        find_package(X11 REQUIRED)
        set(PLATFORM_LIBRARIES ${OPENGL_LIBRARIES} ${X11_LIBRARIES} pthread dl rt)
    endif()
endif()

# Modify the executable target to create platform-specific builds
if(APPLE)
    # macOS build
    add_executable(DevToolsTerminal_macOS ${SOURCE_FILES})
    set_target_properties(DevToolsTerminal_macOS PROPERTIES
        OUTPUT_NAME "DevToolsTerminal"
        RUNTIME_OUTPUT_DIRECTORY ${MACOS_OUTPUT_DIR}
    )
    target_link_libraries(DevToolsTerminal_macOS 
        $<IF:$<TARGET_EXISTS:CURL::libcurl>,CURL::libcurl,${CURL_LIBRARIES}>
        $<IF:$<TARGET_EXISTS:raylib>,raylib,${RAYLIB_LIBRARIES}>
        ${PLATFORM_LIBRARIES}
        Threads::Threads
    )
endif()

# Linux build
add_executable(DevToolsTerminal_Linux ${SOURCE_FILES})
set_target_properties(DevToolsTerminal_Linux PROPERTIES
    OUTPUT_NAME "DevToolsTerminal"
    RUNTIME_OUTPUT_DIRECTORY ${LINUX_OUTPUT_DIR}
)
target_link_libraries(DevToolsTerminal_Linux 
    $<IF:$<TARGET_EXISTS:CURL::libcurl>,CURL::libcurl,${CURL_LIBRARIES}>
    $<IF:$<TARGET_EXISTS:raylib>,raylib,${RAYLIB_LIBRARIES}>
    ${PLATFORM_LIBRARIES}
    Threads::Threads
    ${DL_LIBRARY}
)

# Set rpath for Linux builds
if(UNIX AND NOT APPLE)
    set_target_properties(DevToolsTerminal_Linux PROPERTIES
        INSTALL_RPATH "$ORIGIN/../lib"
        BUILD_WITH_INSTALL_RPATH TRUE
    )
endif()

# If static linking is enabled, try to find static versions of libraries
if(STATIC_LINKING)
    set(CMAKE_FIND_LIBRARY_SUFFIXES .a .so)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
endif()

# Package configuration
include(InstallRequiredSystemLibraries)
set(CPACK_GENERATOR "DEB;RPM;TGZ")
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Developer Tools Terminal")
set(CPACK_PACKAGE_VENDOR "DevToolsTerminal")

# Include CPack to enable packaging
include(CPack)

# Update install locations
install(TARGETS DevToolsTerminal_Linux
    RUNTIME DESTINATION bin/linux
    CONFIGURATIONS Release
)

if(APPLE)
    install(TARGETS DevToolsTerminal_macOS
        RUNTIME DESTINATION bin/macos
        CONFIGURATIONS Release
    )
endif()
