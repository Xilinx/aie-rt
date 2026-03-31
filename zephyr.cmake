# SPDX-License-Identifier: Apache-2.0

function(add_aie_rt_library TARGET_NAME)
  if(NOT DEFINED AIE_RT_SRC_DIR OR "${AIE_RT_SRC_DIR}" STREQUAL "")
    message(FATAL_ERROR "AIE_RT_SRC_DIR is not set. Please set AIE_RT_SRC_DIR to the path of the aie-rt source directory.")
  endif()

  target_sources(${TARGET_NAME} PRIVATE
    ${AIE_RT_SRC_DIR}/driver/src/common/xaie_helper.c

    ${AIE_RT_SRC_DIR}/driver/src/core/xaie_core.c
    ${AIE_RT_SRC_DIR}/driver/src/core/xaie_core_aie.c
    ${AIE_RT_SRC_DIR}/driver/src/core/xaie_core_aieml.c
    ${AIE_RT_SRC_DIR}/driver/src/core/xaie_elfloader.c
    ${AIE_RT_SRC_DIR}/driver/src/core/xaie_uc.c

    ${AIE_RT_SRC_DIR}/driver/src/device/xaie_device_aie.c
    ${AIE_RT_SRC_DIR}/driver/src/device/xaie_device_aie2ipu.c
    ${AIE_RT_SRC_DIR}/driver/src/device/xaie_device_aie2p.c
    ${AIE_RT_SRC_DIR}/driver/src/device/xaie_device_aie2ps.c
    ${AIE_RT_SRC_DIR}/driver/src/device/xaie_device_aieml.c

    ${AIE_RT_SRC_DIR}/driver/src/dma/xaie_dma.c
    ${AIE_RT_SRC_DIR}/driver/src/dma/xaie_dma_aie.c
    ${AIE_RT_SRC_DIR}/driver/src/dma/xaie_dma_aie2ps.c
    ${AIE_RT_SRC_DIR}/driver/src/dma/xaie_dma_aieml.c

    ${AIE_RT_SRC_DIR}/driver/src/events/xaie_events.c

    ${AIE_RT_SRC_DIR}/driver/src/global/xaie2ipugbl_reginit.c
    ${AIE_RT_SRC_DIR}/driver/src/global/xaie2pgbl_reginit.c
    ${AIE_RT_SRC_DIR}/driver/src/global/xaie2psgbl_reginit.c
    ${AIE_RT_SRC_DIR}/driver/src/global/xaiegbl.c
    ${AIE_RT_SRC_DIR}/driver/src/global/xaiegbl_reginit.c
    ${AIE_RT_SRC_DIR}/driver/src/global/xaiemlgbl_reginit.c

    ${AIE_RT_SRC_DIR}/driver/src/interrupt/xaie_interrupt_aie.c
    ${AIE_RT_SRC_DIR}/driver/src/interrupt/xaie_interrupt_aie2ipu.c
    ${AIE_RT_SRC_DIR}/driver/src/interrupt/xaie_interrupt_aie2ps.c
    ${AIE_RT_SRC_DIR}/driver/src/interrupt/xaie_interrupt_aieml.c
    ${AIE_RT_SRC_DIR}/driver/src/interrupt/xaie_interrupt_backtrack.c
    ${AIE_RT_SRC_DIR}/driver/src/interrupt/xaie_interrupt_control.c
    ${AIE_RT_SRC_DIR}/driver/src/interrupt/xaie_interrupt_init.c

    ${AIE_RT_SRC_DIR}/driver/src/io_backend/xaie_io.c
    ${AIE_RT_SRC_DIR}/driver/src/io_backend/ext/xaie_baremetal.c
    ${AIE_RT_SRC_DIR}/driver/src/io_backend/ext/xaie_cdo.c
    ${AIE_RT_SRC_DIR}/driver/src/io_backend/ext/xaie_controlcode.c
    ${AIE_RT_SRC_DIR}/driver/src/io_backend/ext/xaie_debug.c
    ${AIE_RT_SRC_DIR}/driver/src/io_backend/ext/xaie_io_common.c
    ${AIE_RT_SRC_DIR}/driver/src/io_backend/ext/xaie_ipu.c
    ${AIE_RT_SRC_DIR}/driver/src/io_backend/ext/xaie_linux.c
    ${AIE_RT_SRC_DIR}/driver/src/io_backend/ext/xaie_metal.c
    ${AIE_RT_SRC_DIR}/driver/src/io_backend/ext/xaie_sim.c
    ${AIE_RT_SRC_DIR}/driver/src/io_backend/ext/xaie_socket.c
    ${AIE_RT_SRC_DIR}/driver/src/io_backend/ext/xaie_zephyr.c

    ${AIE_RT_SRC_DIR}/driver/src/io_backend/privilege/xaie_io_privilege.c

    ${AIE_RT_SRC_DIR}/driver/src/lite/xaie_lite.c
    ${AIE_RT_SRC_DIR}/driver/src/lite/xaie_lite_privilege.c

    ${AIE_RT_SRC_DIR}/driver/src/locks/xaie_locks.c
    ${AIE_RT_SRC_DIR}/driver/src/locks/xaie_locks_aie.c
    ${AIE_RT_SRC_DIR}/driver/src/locks/xaie_locks_aieml.c

    ${AIE_RT_SRC_DIR}/driver/src/memory/xaie_mem.c

    ${AIE_RT_SRC_DIR}/driver/src/npi/xaie_npi.c
    ${AIE_RT_SRC_DIR}/driver/src/npi/xaie_npi_aie.c
    ${AIE_RT_SRC_DIR}/driver/src/npi/xaie_npi_aieml.c

    ${AIE_RT_SRC_DIR}/driver/src/perfcnt/xaie_perfcnt.c

    ${AIE_RT_SRC_DIR}/driver/src/pl/xaie_plif.c

    ${AIE_RT_SRC_DIR}/driver/src/pm/xaie_clock.c
    ${AIE_RT_SRC_DIR}/driver/src/pm/xaie_ecc.c
    ${AIE_RT_SRC_DIR}/driver/src/pm/xaie_reset.c
    ${AIE_RT_SRC_DIR}/driver/src/pm/xaie_reset_aie.c
    ${AIE_RT_SRC_DIR}/driver/src/pm/xaie_reset_aieml.c
    ${AIE_RT_SRC_DIR}/driver/src/pm/xaie_tilectrl.c

    ${AIE_RT_SRC_DIR}/driver/src/routing/xaie_routing.c

    ${AIE_RT_SRC_DIR}/driver/src/stream_switch/xaie_ss.c
    ${AIE_RT_SRC_DIR}/driver/src/stream_switch/xaie_ss_aie.c
    ${AIE_RT_SRC_DIR}/driver/src/stream_switch/xaie_ss_aie2ps.c
    ${AIE_RT_SRC_DIR}/driver/src/stream_switch/xaie_ss_aieml.c

    ${AIE_RT_SRC_DIR}/driver/src/timer/xaie_timer.c

    ${AIE_RT_SRC_DIR}/driver/src/trace/xaie_trace.c

    ${AIE_RT_SRC_DIR}/driver/src/util/btree4.c
    ${AIE_RT_SRC_DIR}/driver/src/util/xaie_util_status.c
    ${AIE_RT_SRC_DIR}/driver/src/util/xaie_util_events.c
  )

  target_include_directories(${TARGET_NAME} PUBLIC
    ${AIE_RT_SRC_DIR}/driver/src
    ${AIE_RT_SRC_DIR}/driver/src/common
    ${AIE_RT_SRC_DIR}/driver/src/core
    ${AIE_RT_SRC_DIR}/driver/src/device
    ${AIE_RT_SRC_DIR}/driver/src/dma
    ${AIE_RT_SRC_DIR}/driver/src/events
    ${AIE_RT_SRC_DIR}/driver/src/global
    ${AIE_RT_SRC_DIR}/driver/src/interrupt
    ${AIE_RT_SRC_DIR}/driver/src/io_backend
    ${AIE_RT_SRC_DIR}/driver/src/io_backend/ext
    ${AIE_RT_SRC_DIR}/driver/src/io_backend/privilege
    ${AIE_RT_SRC_DIR}/driver/src/lite
    ${AIE_RT_SRC_DIR}/driver/src/locks
    ${AIE_RT_SRC_DIR}/driver/src/memory
    ${AIE_RT_SRC_DIR}/driver/src/npi
    ${AIE_RT_SRC_DIR}/driver/src/perfcnt
    ${AIE_RT_SRC_DIR}/driver/src/pl
    ${AIE_RT_SRC_DIR}/driver/src/pm
    ${AIE_RT_SRC_DIR}/driver/src/routing
    ${AIE_RT_SRC_DIR}/driver/src/stream_switch
    ${AIE_RT_SRC_DIR}/driver/src/timer
    ${AIE_RT_SRC_DIR}/driver/src/trace
    ${AIE_RT_SRC_DIR}/driver/src/uc_driver
    ${AIE_RT_SRC_DIR}/driver/src/util
    ${AIE_RT_SRC_DIR}/driver/include
    ${AIE_RT_SRC_DIR}/driver/include/xaiengine
  )

  target_compile_definitions(${TARGET_NAME} PUBLIC
    __AIEZEPHYR__=1
  )

  execute_process (
    COMMAND make -f Makefile.Linux include
    WORKING_DIRECTORY ${AIE_RT_SRC_DIR}/driver/src/
  )
endfunction()
