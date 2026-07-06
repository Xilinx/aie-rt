/******************************************************************************
* Copyright (C) 2024 AMD, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file utils.h
* @{
*
* This file contains supporting functions.
******************************************************************************/

#ifndef XAIE_UTILS_H
#define XAIE_UTILS_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "xaie_regdb_uc_module_core.h"
#include "xaie_handshake.h"

//#define mbar(mask)      ({  __asm__ __volatile__ ("mbar\t" stringify(mask) ); })
#define mbar(mask)
uint32_t column_index_rel = 0; // temporarily setting it to zero but it needs to calculated from partition start

#define LIKELY(x)           __builtin_expect(!!(x), 1)
#define UNLIKELY(x)         __builtin_expect(!!(x), 0)
#define UNUSED_ARG(x) UNUSED_ ## x __attribute__((__unused__))
#define UNUSED_FUNC(x) __attribute__((__unused__)) UNUSED_ ## x

//#define WITH_DYNAMIC_TRACE

#ifndef CLOCK_FREQUENCY
#   define CLOCK_FREQUENCY              1000000000U
#endif

#ifndef TICK_RATE_HZ
#   define TICK_RATE_HZ            1000     // tick with 1000 Hz -> every 1 ms
#endif
#define TICK_TIMER_ID               0
#define TICK_TIMER_INTERRUPT        3
#define SCRUB_INTERVAL              2  // scrub every 2ms

//#define WITH_PROFILING_IN_FAST_INTERRUPT
#ifdef WITH_PROFILING_IN_FAST_INTERRUPT
#   ifndef PROFILING_RATE_HZ
#       define PROFILING_RATE_HZ        200000000
#   endif
#   define PROFILING_TIMER_ID           1
#   define PROFILING_TIMER_INTERRUPT    4
#endif

#define EVENT_ACTION_INTERRUPT        21
#define GENERIC_TIMER_LOW_ID 3
#define GENERIC_TIMER_HIGH_ID 2
#define GENERIC_TIMER_RELOAD 0xFFFFFFFF

// extern XIOModule xIOModuleInstance;

#if defined(WITH_INTERRUPTS_FAST)
#   define IRQ_VOLATILE                 volatile
#   define DM2MM_RESPONSE_INTERRUPT     16
#   define MM2DM_RESPONSE_INTERRUPT     17
#   define AXIS_SLAVE_INTERRUPT         18
#   define ENABLE_IRQ()                 do { \
                                            XIOModule_Enable(&xIOModuleInstance, DM2MM_RESPONSE_INTERRUPT); \
                                            XIOModule_Enable(&xIOModuleInstance, MM2DM_RESPONSE_INTERRUPT); \
                                            XIOModule_Enable(&xIOModuleInstance, AXIS_SLAVE_INTERRUPT); \
                                        } while(0)
#   define DISABLE_IRQ()                do { \
                                            XIOModule_Disable(&xIOModuleInstance, DM2MM_RESPONSE_INTERRUPT); \
                                            XIOModule_Disable(&xIOModuleInstance, MM2DM_RESPONSE_INTERRUPT); \
                                            XIOModule_Disable(&xIOModuleInstance, AXIS_SLAVE_INTERRUPT); \
                                        } while(0)
#else
#   define ENABLE_IRQ()
#   define DISABLE_IRQ()
#   define IRQ_VOLATILE
#endif

//#define WITH_ASSERT
#ifdef WITH_ASSERT
#   define assert(x)        if (UNLIKELY(!(x))) { xil_printf("Assertion failed @ line %u!\n", __LINE__); uint32_t result = 0xFF; cputfsl(result, 0); mb_sleep(); }
#else
#   define assert(x)
#endif

#define SHARED_DM_BASE      ((void *)REGDB_UC_MODULE_CORE_REGISTER(MODULE_DATA_MEMORY))
#define SHARED_DM_SIZE      0x8000

#define AXIMM_BASE          ((void *)REGDB_UC_MODULE_CORE_REGISTER(ARRAY_AXI_MM))

// #define IO_MODULE_TO_USE             XPAR_AIE2P_S_MICROBLAZE_IOMODULE1_DEVICE_ID

#define PRIVATE_DM_NOINIT   __attribute__((section (".noinit")))
#define SHARED_DM_NOINIT    __attribute__((section (".shareddm")))

// #define compiler_barrier()  asm volatile ("" : : : "memory")

static inline void reg_write32(void *addr, uint32_t value)
{
    *((volatile uint32_t *)addr) = value;
}
static inline uint32_t reg_read32(void *addr)
{
    return *((volatile uint32_t *)addr);
}

static inline uint64_t time_elapsed(uint32_t end_high, uint32_t end_low, uint32_t begin_high, uint32_t begin_low)
{
  return ((((uint64_t)end_high << 32) + end_low) - (((uint64_t)begin_high << 32) + begin_low));
}

void log_init();
#endif

/** @} */