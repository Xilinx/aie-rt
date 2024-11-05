# Copyright (C) 2023 Advanced Micro Devices, Inc.. All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.15)
project(aienginev2)

find_package(common)
include_directories(${CMAKE_BINARY_DIR}/include/xaiengine)

file(GLOB_RECURSE  SOURCES  "${CMAKE_CURRENT_SOURCE_DIR}/*/*.c"  "${CMAKE_CURRENT_SOURCE_DIR}/*/*/*.c")

file(GLOB_RECURSE  HEADERS  "${CMAKE_CURRENT_SOURCE_DIR}/*/*.h"  "${CMAKE_CURRENT_SOURCE_DIR}/*/*/*.h")

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/xaiengine.h DESTINATION ${CMAKE_BINARY_DIR}/include/)

file(COPY ${HEADERS} DESTINATION ${CMAKE_BINARY_DIR}/include/xaiengine)

add_library(aienginev2 STATIC ${SOURCES})

set_target_properties(aienginev2 PROPERTIES LINKER_LANGUAGE C)
target_compile_options(aienginev2 PRIVATE -Wall -Wextra -D__AIEBAREMETAL__)
