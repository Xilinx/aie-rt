/******************************************************************************
* Copyright (C) 2021 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xaie_feature_config.h
* @{
*
* This file contains macros to optionally enable AI engine feaures during
* compilation to reduce binary size.
* We have one macro per feature, and for easier to use, we have feature groups
* macros. Here are the groups macros:
*  * XAIE_FEATURE_APP_BASIC: basic features commonly needed for runtime on
*		target application. It includes the following features:
*    * XAIE_FEATURE_CORE_ENABLE: AIE tile core module APIs
*    * XAIE_FEATURE_DMA_ENABLE: AIE DMA APIs
*    * XAIE_FEATURE_LOCK_ENABLE: AIE lock APIs
*    * XAIE_FEATURE_DATAMEM_ENABLE: AIE data memory APIs
*  * XAIE_FEATURE_PRIVILEGED: features commonly needed for the library which
*		needs privileged permission for platform management
*    * XAIE_FEATURE_PRIVILEGED_ENABLE: AIE APIs which needs to access
*		privileged registers. As such operations are column reset,
*		shim reset, AXI bus error events setting, NPI interrupt
*		setting, and memory clearing. It will enable the following
*		features:
*      * XAIE_FEATURE_PL_ENABLE: AIE SHIM PL APIs
*      * XAIE_FEATURE_DATAMEM_ENABLE
*      * XAIE_FEATURE_INTR_CTRL_ENABLE: AIE interrupt enable/disable APIs
*    * XAIE_FEATURE_INTR_BTRK_ENABLE: AIE interrupt back tracking
*  * XAIE_FEATURE_ALL: all AIE APIs
*    * XAIE_FEATURE_CORE_ENABLE
*    * XAIE_FEATURE_DMA_ENABLE
*    * XAIE_FEATURE_LOCK_ENABLE
*    * XAIE_FEATURE_PRIVILEGED_ENABLE
*    * XAIE_FEATURE_DATAMEM_ENABLE
*    * XAIE_FEATURE_PERFCOUNT_ENABLE: AIE performance counter APIs
*    * XAIE_FEATURE_TIMER_ENABLE: AIE timer APIs
*    * XAIE_FEATURE_TRACE_ENABLE: AIE tracing APIs
*    * XAIE_FEATURE_SS_ENABLE: AIE stream switch APIs
*    * XAIE_FEATURE_EVENTS_ENABLE: AIE events APIs
*    * XAIE_FEATURE_ELF_ENABLE: AIE ELF loader APIs
*    * XAIE_FEATURE_UC_ENABLE: AIE SHIM microcontroller APIs
*    * XAIE_FEATURE_INTR_INIT_ENABLE: AIE interrupt network initialization APIs
*    * XAIE_FEATURE_UTIL_ENABLE: AIE Utility APIs
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Wendy   07/27/2021  Initial creation
* </pre>
*
******************************************************************************/
#ifndef XAIE_FEATURE_CONFIG_H
#define XAIE_FEATURE_CONFIG_H

#ifndef XAIE_FEATURE_APP_BASIC
#ifndef XAIE_FEATURE_PRIVILEGED
#ifndef XAIE_FEATURE_ALL
/* Define is all features */
#define XAIE_FEATURE_ALL
#endif
#endif
#endif

#ifdef XAIE_FEATURE_APP_BASIC
#define XAIE_FEATURE_CORE_ENABLE
#define XAIE_FEATURE_DMA_ENABLE
#define XAIE_FEATURE_LOCK_ENABLE
#define XAIE_FEATURE_DATAMEM_ENABLE
#endif /* XAIE_FEATURE_APP_BASIC */

#ifdef XAIE_FEATURE_PRIVILEGED
#define XAIE_FEATURE_PRIVILEGED_ENABLE
#define XAIE_FEATURE_INTR_BTRK_ENABLE
#endif

#ifdef XAIE_FEATURE_ALL
#define XAIE_FEATURE_PRIVILEGED_ENABLE
#define XAIE_FEATURE_PL_ENABLE
#define XAIE_FEATURE_DMA_ENABLE
#define XAIE_FEATURE_LOCK_ENABLE
#define XAIE_FEATURE_DATAMEM_ENABLE
#define XAIE_FEATURE_PERFCOUNT_ENABLE
#define XAIE_FEATURE_TIMER_ENABLE
#define XAIE_FEATURE_TRACE_ENABLE
#define XAIE_FEATURE_SS_ENABLE
#define XAIE_FEATURE_EVENTS_ENABLE
#define XAIE_FEATURE_CORE_ENABLE
#define XAIE_FEATURE_ELF_ENABLE
#define XAIE_FEATURE_UC_ENABLE
#define XAIE_FEATURE_INTR_INIT_ENABLE
#define XAIE_FEATURE_INTR_BTRK_ENABLE
#define XAIE_FEATURE_UTIL_ENABLE
#define XAIE_FEATURE_UTIL_STATUS_ENABLE
#define XAIE_FEATURE_ROUTING_ENABLE
#endif /* XAIE_FEATURE_FULL */

#if defined(XAIE_FEATURE_INTR_INIT_ENABLE) ||	\
    defined(XAIE_FEATURE_INTR_CTRL_ENABLE) ||	\
    defined(XAIE_FEATURE_INTR_BTRK_ENABLE)
#ifndef XAIE_FEATURE_INTR_L1_ENABLE
#define XAIE_FEATURE_INTR_L1_ENABLE
#endif
#ifndef XAIE_FEATURE_INTR_L2_ENABLE
#define XAIE_FEATURE_INTR_L2_ENABLE
#endif
#endif

#ifdef XAIE_FEATURE_PRIVILEGED_ENABLE
#ifndef XAIE_FEATURE_PL_ENABLE
#define XAIE_FEATURE_PL_ENABLE
#endif
#ifndef XAIE_FEATURE_INTR_CTRL_ENABLE
#define XAIE_FEATURE_INTR_CTRL_ENABLE
#endif
#ifndef XAIE_FEATURE_DATAMEM_ENABLE
#define XAIE_FEATURE_DATAMEM_ENABLE
#endif
#endif

/*
 * Master switch for Arm Certified FuSa C library targets.
 *
 * Building for a target that links the Arm Certified FuSa C library
 * requires all three opt-outs below (no filesystem, no stdio logging, no
 * libc malloc), since that library provides none of those services.
 * Define XAIE_FEATURE_FUSA_LIBC once to enable all three instead of
 * having to know and pass each XAIE_FEATURE_NO_* macro individually. Any
 * of the three can still be set/unset on its own to mix and match.
 */
#ifdef XAIE_FEATURE_FUSA_LIBC
#ifndef XAIE_FEATURE_NO_FILESYSTEM
#define XAIE_FEATURE_NO_FILESYSTEM
#endif
#ifndef XAIE_FEATURE_NO_STDIO_LOGGING
#define XAIE_FEATURE_NO_STDIO_LOGGING
#endif
#ifndef XAIE_FEATURE_NO_LIBC_MALLOC
#define XAIE_FEATURE_NO_LIBC_MALLOC
#endif
#endif /* XAIE_FEATURE_FUSA_LIBC */

/*
 * File-system based image loading.
 *
 * XAie_LoadElf(), XAie_LoadElfPartial() and XAie_LoadUc() read the image from
 * a file using hosted stdio services (fopen/fread/fseek/ftell/fclose). These
 * are not available on bare-metal / safety (FuSa) targets whose C library has
 * no filesystem. Define XAIE_FEATURE_NO_FILESYSTEM to compile the file-based
 * loaders out; callers must then use the memory-based loaders
 * XAie_LoadElfMem() / XAie_LoadUcMem().
 */
#ifndef XAIE_FEATURE_NO_FILESYSTEM
#ifndef XAIE_FEATURE_ELF_FILE_ENABLE
#define XAIE_FEATURE_ELF_FILE_ENABLE
#endif
#endif

/*
 * Hosted stdio-based logging and diagnostics.
 *
 * XAie_Log() and a handful of debug/dump helpers use hosted stdio services
 * (fprintf/vfprintf/printf/putchar) to emit messages, and a constructor
 * calls getenv("XAIE_LOG_LEVEL") to pick the runtime log level. These
 * services are not available on bare-metal / safety (FuSa) targets.
 * Define XAIE_FEATURE_NO_STDIO_LOGGING to redirect all logging through
 * direct, bounded snprintf() calls (no vsnprintf() -- the FuSa C library
 * doesn't provide it) and a lightweight, overridable sink function
 * (_XAie_LogSink()) instead of hosted stdio, and to select
 * the log level at compile time via XAIE_LOG_LEVEL_DEFAULT instead of the
 * "XAIE_LOG_LEVEL" environment variable. XAie_LoggerInit() remains
 * available for callers that want to set the log level explicitly at
 * runtime without depending on getenv().
 */
#ifndef XAIE_FEATURE_NO_STDIO_LOGGING
#ifndef XAIE_FEATURE_STDIO_LOGGING_ENABLE
#define XAIE_FEATURE_STDIO_LOGGING_ENABLE
#endif
#endif

/*
 * Hosted libc dynamic memory allocation.
 *
 * Several internal data structures (transaction command buffers, the
 * memory-instance b-tree index, ELF/uC BSS sections, routing/timer helper
 * arrays) use malloc()/calloc()/realloc()/free()/aligned_alloc() from the
 * hosted C library. The Arm Certified FuSa C library provides malloc()
 * only: there is no free(), calloc(), realloc() or aligned_alloc() at all,
 * so any code that references them fails to link on a FuSa target.
 * Define XAIE_FEATURE_NO_LIBC_MALLOC to redirect all dynamic allocation
 * through a small bounded allocator (_XAie_Heap*(), see xaie_mem_alloc.h)
 * backed by a static arena instead of the hosted C library. The arena
 * size is controlled by XAIE_FUSA_HEAP_SIZE (see xaie_mem_alloc.h);
 * integrators must size it for their actual worst-case usage.
 */
#ifndef XAIE_FEATURE_NO_LIBC_MALLOC
#ifndef XAIE_FEATURE_LIBC_MALLOC_ENABLE
#define XAIE_FEATURE_LIBC_MALLOC_ENABLE
#endif
#endif

#endif /* XAIE_FEATURE_CONFIG_H */
/** @} */
