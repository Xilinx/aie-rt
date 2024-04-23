# Copyright (C) 2023 Advanced Micro Devices, Inc.. All rights reserved.
# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.15)
project(aienginev2)

find_package(common)
collector_create (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}")
collector_create (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}")
include_directories(${CMAKE_BINARY_DIR}/include/xaiengine)

#common
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/common/xaie_helper.c")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/common/xaie_helper.h")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/common/xaie_txn.h")

#core
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/core/xaie_core.c")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/core/xaie_core.h")
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/core/xaie_core_aie.c")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/core/xaie_core_aie.h")
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/core/xaie_core_aieml.c")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/core/xaie_core_aieml.h")
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/core/xaie_elfloader.c")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/core/xaie_elfloader.h")

#device
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/device/xaie_device_aie.c")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/device/xaie_device_aie.h")
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/device/xaie_device_aieml.c")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/device/xaie_device_aieml.h")

#dma
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/dma/xaie_dma.c")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/dma/xaie_dma.h")
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/dma/xaie_dma_aie.c")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/dma/xaie_dma_aie.h")
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/dma/xaie_dma_aieml.c")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/dma/xaie_dma_aieml.h")

#events
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/events/xaie_events.c")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/events/xaie_events.h")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/events/xaie_events_aie.h")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/events/xaie_events_aieml.h")


#global
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/global/xaiegbl.c")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/global/xaiegbl_defs.h")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/global/xaiegbl.h")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/global/xaiegbl_params.h")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/global/xaiemlgbl_params.h")
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/global/xaiegbl_reginit.c")
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/global/xaiemlgbl_reginit.c")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/global/xaiegbl_regdef.h")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/global/xaie_feature_config.h")

#interrupt
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/interrupt/xaie_interrupt_aie.c")
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/interrupt/xaie_interrupt_aieml.c")
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/interrupt/xaie_interrupt_backtrack.c")
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/interrupt/xaie_interrupt_control.c")
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/interrupt/xaie_interrupt_init.c")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/interrupt/xaie_interrupt.h")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/interrupt/xaie_interrupt_aie.h")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/interrupt/xaie_interrupt_aieml.h")


#io_backend
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/io_backend/xaie_io.c")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/io_backend/xaie_io.h")
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/io_backend/ext/xaie_baremetal.c")
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/io_backend/ext/xaie_cdo.c")
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/io_backend/ext/xaie_debug.c")
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/io_backend/ext/xaie_io_common.c")
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/io_backend/ext/xaie_linux.c")
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/io_backend/ext/xaie_metal.c")
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/io_backend/ext/xaie_sim.c")
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/io_backend/ext/xaie_socket.c")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/io_backend/ext/xaie_io_common.h")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/io_backend/ext/xlnx-ai-engine.h")
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/io_backend/privilege/xaie_io_privilege.c")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/io_backend/privilege/xaie_io_privilege.h")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/io_backend/xaie_io_internal.h")

#lite
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/lite/xaie_lite.c")
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/lite/xaie_lite_privilege.c")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/lite/xaie_lite.h")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/lite/xaie_lite_aie.h")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/lite/xaie_lite_aieml.h")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/lite/xaie_lite_hwcfg.h")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/lite/xaie_lite_io.h")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/lite/xaie_lite_npi.h")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/lite/xaie_lite_regdef_aie.h")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/lite/xaie_lite_regdef_aieml.h")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/lite/xaie_lite_shim_aie.h")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/lite/xaie_lite_util.h")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/lite/xaie_lite_internal.h")

#locks
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/locks/xaie_locks.c")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/locks/xaie_locks.h")
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/locks/xaie_locks_aie.c")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/locks/xaie_locks_aie.h")
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/locks/xaie_locks_aieml.c")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/locks/xaie_locks_aieml.h")

#memory
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/memory/xaie_mem.c")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/memory/xaie_mem.h")

#npi
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/npi/xaie_npi.c")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/npi/xaie_npi.h")
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/npi/xaie_npi_aie.c")
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/npi/xaie_npi_aieml.c")

#perfcnt
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/perfcnt/xaie_perfcnt.c")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/perfcnt/xaie_perfcnt.h")

#pl
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/pl/xaie_plif.c")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/pl/xaie_plif.h")

#pm
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/pm/xaie_clock.c")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/pm/xaie_clock.h")
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/pm/xaie_ecc.c")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/pm/xaie_ecc.h")
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/pm/xaie_reset.c")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/pm/xaie_reset.h")
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/pm/xaie_reset_aie.c")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/pm/xaie_reset_aie.h")
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/pm/xaie_reset_aieml.c")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/pm/xaie_reset_aieml.h")
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/pm/xaie_tilectrl.c")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/pm/xaie_tilectrl.h")

#stream_switch
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/stream_switch/xaie_ss.c")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/stream_switch/xaie_ss.h")
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/stream_switch/xaie_ss_aie.c")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/stream_switch/xaie_ss_aie.h")
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/stream_switch/xaie_ss_aieml.c")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/stream_switch/xaie_ss_aieml.h")

#timer
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/timer/xaie_timer.c")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/timer/xaie_timer.h")

#trace
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/trace/xaie_trace.c")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/trace/xaie_trace.h")

#util
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/util/xaie_util_events.c")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/util/xaie_util_events.h")
collect (PROJECT_LIB_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/util/xaie_util_status.c")
collect (PROJECT_LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/util/xaie_util_status.h")

collector_list (_sources PROJECT_LIB_SOURCES)
collector_list (_headers PROJECT_LIB_HEADERS)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/xaiengine.h DESTINATION ${CMAKE_BINARY_DIR}/include/)

file(COPY ${_headers} DESTINATION ${CMAKE_BINARY_DIR}/include/xaiengine)

add_library(aienginev2 STATIC ${_sources})

set_target_properties(aienginev2 PROPERTIES LINKER_LANGUAGE C)
target_compile_options(aienginev2 PRIVATE -Wall -Wextra -D__AIEBAREMETAL__)
