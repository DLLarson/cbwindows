# Copyright (c) 2023 By William Su, All Rights Reserved.

cmake_minimum_required(VERSION 2.8.12)
# set VERSION variables
cmake_policy(SET CMP0048 NEW)
# provide CMAKE_MSVC_RUNTIME_LIBRARY
cmake_policy(SET CMP0091 NEW)
# disable default warning flags
cmake_policy(SET CMP0092 NEW)

project(CyberBoard VERSION 4.0 LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/runtime)
file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

# std::format requires c++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_MFC_FLAG 1)
if (MSVC)
	add_compile_definitions($<$<CONFIG:Debug>:_CRTDBG_MAP_ALLOC>)
	add_compile_options(/EHs)
endif()

# wxWidgets
# KLUDGE:  must be cache variables to prevent wx_option() from
# overriding them
set(wxBUILD_INSTALL OFF CACHE BOOL "skip")
set(wxBUILD_SHARED OFF CACHE BOOL "skip")
set(wxBUILD_USE_STATIC_RUNTIME ON CACHE BOOL "skip")
set(wxUSE_MEDIACTRL OFF CACHE BOOL "skip")
set(wxUSE_POSTSCRIPT OFF CACHE BOOL "skip")
set(wxUSE_STC OFF CACHE BOOL "skip")
set(wxUSE_WEBVIEW OFF CACHE BOOL "skip")
set(wxUSE_WINRT OFF CACHE BOOL "skip")
if (MSVC)
	set(wxBUILD_MSVC_MULTIPROC ON CACHE BOOL "skip")
endif()
set(wxBUILD_SAMPLES SOME CACHE STRING "skip")
add_subdirectory(deps/wxWidgets EXCLUDE_FROM_ALL)

add_compile_definitions($<IF:$<CONFIG:Debug>,_,N>DEBUG)
if (MSVC)
	add_compile_options(/W4)
endif()

add_subdirectory(GM)
add_subdirectory(GP)
# help compiler only available from MSVC
if (MSVC)
	add_subdirectory(GHelp)
endif()
