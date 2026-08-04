/******************************************************************************
* Copyright (C) 2024 AMD, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file wait_handle.h
* @{
*
* This file contains wait handle implementation.
******************************************************************************/

#ifndef XAIE_WAIT_HANDLE_H
#define XAIE_WAIT_HANDLE_H

#include <stdbool.h>
#include <stdint.h>

#include "xaie_utils.h"
#include "xaie_control.h"

#define INVALID_WAIT_HANDLE         0xFFFF
#define MAX_WAIT_HANDLES            255

#define WH_PTR_UNUSED               0xFFFF
#define WH_IS_USED(wh)              (wh->waiting_job_ptr != WH_PTR_UNUSED)

/*
 * Wait handle is the fundamental building block to realize job block & wakeup
 * It is used by a job either does ucdma or memtile/shimtile (with tct enabled).
 * After a job issues a dma, it needs to acquire a dma handle and associate the
 * job pointer with it and block itself before the dma is done, at same time other
 * ready jobs have the chance to run. Once the dma id one, the job will be woken up
 * and put on ready list for running.
 *
 * Wait handle, essentially, is an index to the array of wait_handle_entry, which in turn contains
 * a pointer to the job which blocks on the wait handle.
 * For aie2ps in Turin, the pDM is at [0x8000, 0xc000], so 16bit is enough to be a pointer
 * In this way, we can save 2k+ memory
 *
 * Regarding wait handle reuse, e.g
 * JOB 0
 *     acquire_wait_handle
 *     ...
 *                               release_wait_handle in tct or ucdma response
 *     ...                                                                        JOB 1
 *                                                                                acquire_wait_handle
 *                                                                                (same handle)
 *     ...
 *     try_to_block_on_handle
 *     ...
 *
 * In this example, between JOB 0 acquiring the handle and using the handle, the handle may have
 * been released and acquired by JOB 1. so JOB 0 doesn't not need to block since the dma is already
 * done. JOB 0 has to know the wait handle is not the same to the one it acquired.
 * One solution is to add age info (serial no.) in wait handle -- each time a handle is release,
 * the age info will change. This works at the cost of memory consumption to save age info.
 * Actually, we can use the job pointer itself for this purpose under some assumptions/conditions.
 *   1. wait handle is only allocated in an active job, i.e not in checking tct or ucdma response
 *   2. wait handle is allocated and used in same job, i.e no job will allocate wait handle for
 *   other job
 *   3. job pointer is attached to the wait handle right after the handle is allocated in the job,
 *   instead of doing it when job tries to block on it.
 *   4. job pointer attach and wait handle alloc are not interrupted by a wait_handle_release.
 *
 * We are ok with all above assumptions, so we can have same number of wait handle support with
 * minimum memory consumption.
 *
 * There is one special case where using serial_no(age) and job pointer have a bit different
 * behavior
 * e.g
 * JOB 0
 *   UC_DMA_WRITE_DES (acquire_wait_handle)
 *   ...
 *   UC_DMA_WRITE_DES (acquire_wait_handle)
 *   ...
 *   WAIT_UC_DMA
 *   ...
 *   WAIT_UC_DMA
 *   ...
 *
 * In this example, if when 2nd time UC_DMA_WRITE_DES starts, the 1st has been competed, then
 * potentially 2nd time can get same wait_handle to 1st. In this case,
 * for serial_no(age) solution, JOB 0 will block on 2nd WAIT_UC_DMA,
 * for job pointer solution, JOB 0 will block on 1st WAIT_UC_DMA, but 2nd will not block any more
 * There is no big deal because,
 * 1. UC_DMA_WRITE_DES_SYNC doesn't have the issue
 * 2. WAIT_TCTS doesn't have the issue either
 * 3. UC_DMA_WRITE_DES has that issue only when 1st & 2nd get same wait handle, and even in this
 * case JOB 0 will block anyway.
 */

typedef struct
{
  uint16_t waiting_job_ptr; // 16bit as a pointer is enough for current aie2ps
}
wait_handle_entry_t;

typedef struct
{
  uint16_t max_index;
  wait_handle_entry_t handles[MAX_WAIT_HANDLES];
}
wait_handle_state_t;

wait_handle_state_t wait_handle_state;

void wait_handle_init(void);
void wait_handle_reset(void);

static inline void wait_handle_attach_job(uint16_t index)
{
  wait_handle_entry_t *wh = &wait_handle_state.handles[index];
  // attach job here
  wh->waiting_job_ptr = (uint32_t)(uintptr_t)control_state.current_job_context & 0xFFFF;
}

static inline wait_handle_entry_t *wait_handle_get(uint16_t wait_handle)
{
  assert(wait_handle < MAX_WAIT_HANDLES);
  return &wait_handle_state.handles[wait_handle];
}

static inline uint16_t wait_handle_acquire(void)
{
  if (LIKELY(wait_handle_state.max_index != MAX_WAIT_HANDLES))
  {
    uint16_t ret = wait_handle_state.max_index++;
    wait_handle_attach_job(ret);
    return ret;
  }

  assert(wait_handle_state.max_index == MAX_WAIT_HANDLES);

  // TODO optimize - keep pointer of an unused one or use an alloc mask w/ clz?
  for (uint16_t i = 0; i < MAX_WAIT_HANDLES; ++i)
  {
    wait_handle_entry_t *wh = &wait_handle_state.handles[i];

    if (!WH_IS_USED(wh))
    {
      wait_handle_attach_job(i);
      return i;
    }
  }

  return INVALID_WAIT_HANDLE;
}

void job_unblock_func(uint16_t wait_handle, void *waiting_job_ptr)
{
	(void)wait_handle;
	(void)waiting_job_ptr;
    //Need to implement
    return;
}

static inline void wait_handle_release(uint16_t wait_handle)
{
  wait_handle_entry_t *wh = wait_handle_get(wait_handle);

  if (wh->waiting_job_ptr != 0)
  {
    job_unblock_func(wait_handle, (void *)((uintptr_t)wh->waiting_job_ptr));
  }

  wh->waiting_job_ptr = WH_PTR_UNUSED;
}

static inline bool wait_handle_is_valid(uint16_t wait_handle)
{
  const wait_handle_entry_t *wh = wait_handle_get(wait_handle);
  return (wh->waiting_job_ptr == (uint32_t)(uintptr_t)control_state.current_job_context);
}

#endif
/** @} */
