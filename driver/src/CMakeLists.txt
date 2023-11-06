###############################################################################
# Copyright (C) 2022-2023, Advanced Micro Devices, Inc. All Rights Reserved.
# SPDX-License-Identifier: MIT
###############################################################################

cmake_minimum_required(VERSION 3.1)
project(xaiengine LANGUAGES C VERSION 3.5)

set(CMAKE_C_STANDARD 11)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)

file(
  GLOB_RECURSE
  SRCS
  "${CMAKE_CURRENT_SOURCE_DIR}/*/*.c"
  "${CMAKE_CURRENT_SOURCE_DIR}/*/*/*.c"
  )

file(
  GLOB_RECURSE
  HDRS
  "${CMAKE_CURRENT_SOURCE_DIR}/*/*.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/*/*/*.h"
  )

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/xaiengine.h DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/include/)

file(COPY ${HDRS} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/include/${PROJECT_NAME})

add_library(
  ${PROJECT_NAME}
  SHARED
  ${SRCS}
  )
target_include_directories(
  ${PROJECT_NAME}
  PRIVATE
  ${CMAKE_CURRENT_BINARY_DIR}/include
  ${CMAKE_CURRENT_BINARY_DIR}/include/${PROJECT_NAME}
  )
set_target_properties(
  ${PROJECT_NAME}
  PROPERTIES
  VERSION
  ${PROJECT_VERSION}
  SOVERSION
  ${PROJECT_VERSION_MAJOR}
  )

if (LINUX_BACKEND)
	list(APPEND IO_BACKEND -D__AIELINUX__)
endif (LINUX_BACKEND)
if (METAL_BACKEND)
	list(APPEND IO_BACKEND -D__AIEMETAL__)
endif (METAL_BACKEND)
if (SOCKET_BACKEND)
	list(APPEND IO_BACKEND -D__AIESOCKET__)
endif (SOCKET_BACKEND)
if (DEBUG_BACKEND)
	list(APPEND IO_BACKEND -D__AIEDEBUG__)
endif (DEBUG_BACKEND)

if(MSVC)
  target_compile_options(${PROJECT_NAME} PRIVATE /Wall -DXAIE_FEATURE_MSVC ${IO_BACKEND})
else()
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra ${IO_BACKEND})
endif()

include(GNUInstallDirs)

install(
  FILES
  ${HDRS}
  DESTINATION
  ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
  )

install(
  FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/xaiengine.h
  DESTINATION
  ${CMAKE_INSTALL_INCLUDEDIR}
  )

install(
  TARGETS
  ${PROJECT_NAME}
  EXPORT ${PROJECT_NAME}-targets
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR}
  ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
  )

install(
  EXPORT ${PROJECT_NAME}-targets
  NAMESPACE ${PROJECT_NAME}::
  COMPONENT runtime
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
  )
