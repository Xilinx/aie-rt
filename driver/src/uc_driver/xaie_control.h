/******************************************************************************
* Copyright (C) 2024 AMD, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file control.h
* @{
*
* This file contains control state data structures.
******************************************************************************/

#ifndef XAIE_CONTROL_H
#define XAIE_CONTROL_H

#include <stddef.h>
#include "xaie_utils.h"
#include "xaie_queue.h"

#define MAX_PARTITION_COLUMNS		8
#define NUM_TASK_PAGES                  2
#define PROFILE_PAGE_SIZE               8192

#define TASK_PAGE_FLAG_HASNEXTPAGE      (1 << 0)

#define CONTROL_INVALID_REG_VAL         0xDEADBEEF

#define MAX_JOBS                        64
#define INVALID_JOB_ID                  (MAX_JOBS + 1)
#define NUM_REGISTERS                   8
#define NUM_GLOBAL_REGISTERS            16

#define NUM_LOCAL_BARRIERS              16

// Job block reasons
#define BLOCKED_ON_WAIT_HANDLE              1
#define BLOCKED_ON_LOCAL_BARRIER            2
#define BLOCKED_ON_REMOTE_BARRIER           3

#define UPDATE_TICK() do { \
	if (now.tick_low == 0xFFFFFFFF) \
	{ \
          now.tick_low = 0; \
          now.tick_high++; \
	} else \
	{ \
          now.tick_low++ ; \
	} \
} while (0)

typedef struct control_job_context_t
{
#if LOG_LEVEL < LOG_LEVEL_NONE
  uint16_t id;
#endif

  uint16_t blocked_on_wait_handle;
  struct control_job_context_t *blocked_on_local_barrier_next;

  const uint8_t *data_end;
  const uint8_t *pc;
  const uint8_t *npc;

  bool zf;

  uint32_t r[NUM_REGISTERS];
}
control_job_context_t;

typedef struct
{
  control_job_context_t *waiting_head;
  uint32_t num_participants;
}
local_barrier_t;

typedef struct
{
  uint32_t data_offset;
  uint16_t data_end_offset;

  control_job_context_t job_contexts[MAX_JOBS];
  control_job_context_t *current_job_context;

  control_job_context_t *_ready_jobs[MAX_JOBS];
  queue_t ready_jobs;

  uint32_t r[NUM_GLOBAL_REGISTERS];

  local_barrier_t local_barriers[NUM_LOCAL_BARRIERS];

  IRQ_VOLATILE unsigned int num_unfinished_jobs;
  IRQ_VOLATILE bool unblocked;
}
control_state_t;

volatile uint32_t fw_state;
control_state_t control_state;

int interrupt_init();
void interrupt_fini();
void control_init(uint8_t partition_col_start, uint8_t col_self);
uint32_t control_run_task(uint64_t task_page_ext_mem_address, const uint32_t *args, uint32_t args_len);
#endif

/** @} */