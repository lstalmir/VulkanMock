# Copyright (c) 2024 Lukasz Stalmirski
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

cmake_minimum_required (VERSION 3.8...3.31)

if (MSVC)
    # Link MSVC runtime libraries statically to avoid compatibility issues.
    if (CMAKE_VERSION VERSION_LESS 3.15.0)
        add_compile_options ("/MT$<$<CONFIG:Debug>:d>")
    else ()
        cmake_policy (SET CMP0091 NEW)
        set (CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif ()
endif ()

# Mock ICD
project (vk_mock CXX)

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set (VK_MOCK_ICD_ARCH 64)
else ()
    set (VK_MOCK_ICD_ARCH 32)
endif ()

# Generate version info
set (VK_MOCK_ICD_VER_MAJOR 1)
set (VK_MOCK_ICD_VER_MINOR 0)
set (VK_MOCK_ICD_VER_BUILD 9999)
set (VK_MOCK_ICD_VER_PATCH 0)
set (VK_MOCK_ICD_VER_IMPLEMENTATION 1)
set (VK_MOCK_ICD_NAME vk_mock_icd)
set (VK_MOCK_ICD_API_VERSION "1.1.106")
set (VK_MOCK_ICD_PROJECTNAME vk_mock_icd${VK_MOCK_ICD_ARCH})
set (VK_MOCK_ICD_COMPANYNAME )
set (VK_MOCK_ICD_FILEDESCRIPTION "Mock ICD used for testing Vulkan(R) layers.")
set (VK_MOCK_ICD_LEGALCOPYRIGHT )
set (VK_MOCK_ICD_PRODUCTNAME "Vulkan(R) Mock ICD")

# Find Vulkan headers
if (NOT VULKAN_HEADERS_INSTALL_DIR)
    set (VULKAN_HEADERS_INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/External/Vulkan-Headers")
endif ()

# Common directories
set (VK_MOCK_ICD_SCRIPTS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/scripts")

find_package (Python3 REQUIRED)

# Generate version info
execute_process (
    COMMAND "${Python3_EXECUTABLE}"
        "${VK_MOCK_ICD_SCRIPTS_DIR}/gen_version.py"
        "${CMAKE_BINARY_DIR}/vk_mock_icd_version.cmake"
    RESULT_VARIABLE VK_MOCK_ICD_GEN_VERSION_INFO_RESULT)

if (VK_MOCK_ICD_GEN_VERSION_INFO_RESULT STREQUAL "0")
    include ("${CMAKE_BINARY_DIR}/vk_mock_icd_version.cmake")
endif ()

# Get Vulkan headers version
function (get_vulkan_api_version)
    set (vulkan_ver_major 1)
    set (vulkan_ver_minor 1)
    set (vulkan_ver_headers 106)

    set (vulkan_core_path "${VULKAN_HEADERS_INSTALL_DIR}/include/vulkan/vulkan_core.h")
    if (EXISTS ${vulkan_core_path})
        file (READ ${vulkan_core_path} ver)
        if (ver MATCHES "#define[ ]+VK_HEADER_VERSION_COMPLETE[ ]+VK_MAKE_API_VERSION\\([ ]*[0-9]+,[ ]*([0-9]+),[ ]*([0-9]+),[ ]*VK_HEADER_VERSION[ ]*\\)")
            set (vulkan_ver_major ${CMAKE_MATCH_1})
            set (vulkan_ver_minor ${CMAKE_MATCH_2})
        endif ()
        if (ver MATCHES "#define[ ]+VK_HEADER_VERSION[ ]+([0-9]+)")
            set (vulkan_ver_headers ${CMAKE_MATCH_1})
        endif ()
    endif ()

    set (VK_MOCK_ICD_API_VERSION "${vulkan_ver_major}.${vulkan_ver_minor}.${vulkan_ver_headers}" PARENT_SCOPE)
endfunction ()
get_vulkan_api_version ()

# Generate mock ICD
add_custom_command (
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/vk_mock_icd_base.h"
    COMMAND "${Python3_EXECUTABLE}"
        "${VK_MOCK_ICD_SCRIPTS_DIR}/gen_icd.py"
        --vk_xml="${VULKAN_HEADERS_INSTALL_DIR}/registry/vk.xml"
        --output="${CMAKE_CURRENT_BINARY_DIR}/vk_mock_icd_base.h"
    DEPENDS "${VULKAN_HEADERS_INSTALL_DIR}/registry/vk.xml"
    DEPENDS "${VK_MOCK_ICD_SCRIPTS_DIR}/gen_icd.py")
list (APPEND VK_MOCK_ICD_CODEGEN_FILES "${CMAKE_CURRENT_BINARY_DIR}/vk_mock_icd_base.h")

# Configure JSON file
if (WIN32)
    # WA: Windows uses backslash as directory separator, which needs to be
    # escaped twice (once here and once in the json, thus 4 backslashes)
    set (THIS_DIR ".\\\\")
else ()
    set (THIS_DIR "./")
endif ()

configure_file (Source/vk_mock_icd.json.in "${CMAKE_CURRENT_BINARY_DIR}/vk_mock_icd.json_configured" @ONLY)
configure_file (Source/vk_mock_icd.rc.in   "${CMAKE_CURRENT_BINARY_DIR}/vk_mock_icd.rc" @ONLY)

set (VK_MOCK_JSON "${CMAKE_CURRENT_BINARY_DIR}/vk_mock_icd${VK_MOCK_ICD_ARCH}.json")
file (GENERATE
    OUTPUT "${VK_MOCK_JSON}"
    INPUT "${CMAKE_CURRENT_BINARY_DIR}/vk_mock_icd.json_configured")

add_custom_target (vk_mock_icd_codegen
    DEPENDS ${VK_MOCK_ICD_CODEGEN_FILES})

add_library (vk_mock_icd SHARED
    "${CMAKE_CURRENT_BINARY_DIR}/vk_mock_icd.rc"
    "Source/vk_mock_icd.def"
    "Source/vk_mock_buffer.h"
    "Source/vk_mock_command_buffer.h"
    "Source/vk_mock_command_buffer.cpp"
    "Source/vk_mock_device.h"
    "Source/vk_mock_device.cpp"
    "Source/vk_mock_device_memory.h"
    "Source/vk_mock_icd.h"
    "Source/vk_mock_icd.cpp"
    "Source/vk_mock_icd_helpers.h"
    "Source/vk_mock_image.h"
    "Source/vk_mock_instance.h"
    "Source/vk_mock_instance.cpp"
    "Source/vk_mock_physical_device.h"
    "Source/vk_mock_physical_device.cpp"
    "Source/vk_mock_query_pool.h"
    "Source/vk_mock_queue.h"
    "Source/vk_mock_queue.cpp"
    "Source/vk_mock_surface.h"
    "Source/vk_mock_swapchain.h")

add_dependencies (vk_mock_icd vk_mock_icd_codegen)

set_target_properties (vk_mock_icd PROPERTIES
    OUTPUT_NAME "vk_mock_icd${VK_MOCK_ICD_ARCH}")

target_include_directories (vk_mock_icd
    PRIVATE "${CMAKE_CURRENT_BINARY_DIR}"
    PRIVATE "${VULKAN_HEADERS_INSTALL_DIR}/include")

# Enable platform mocks
if (WIN32)
    target_compile_definitions (vk_mock_icd
        PRIVATE VK_USE_PLATFORM_WIN32_KHR
        PRIVATE NOMINMAX)
endif ()

if (UNIX)
    # XCB
    if (CMAKE_VERSION VERSION_GREATER_EQUAL 3.16.0)
        find_package (ECM NO_MODULE)
        if (ECM_FOUND)
            set (CMAKE_MODULE_PATH ${ECM_FIND_MODULE_DIR})
            find_package (XCB)
            if (XCB_FOUND)
                target_compile_definitions (vk_mock_icd PRIVATE VK_USE_PLATFORM_XCB_KHR)
            endif ()
        endif ()
    endif ()

    # XLIB
    find_package (X11)
    if (X11_FOUND)
        target_compile_definitions (vk_mock_icd PRIVATE VK_USE_PLATFORM_XLIB_KHR)
    endif ()
endif ()

# Install ICD
include (GNUInstallDirs)
install (TARGETS vk_mock_icd
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")

if (WIN32)
    install (FILES "${VK_MOCK_JSON}" DESTINATION "${CMAKE_INSTALL_BINDIR}/.")
else ()
    install (FILES "${VK_MOCK_JSON}" DESTINATION "${CMAKE_INSTALL_LIBDIR}/.")
endif ()
