/******************************************************************************
* Copyright (C) 2024 AMD, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file uc_dma.c
* @{
*
* This file contains implementation for uc DMA.
******************************************************************************/

#include "xaie_uc_dma.h"
#include "xaie_queue.h"
#include "xaie_wait_handle.h"
#include "xaie_handshake.h"
#include "xaie_regdb_uc_module_core.h"
#include "xaie_regdb_uc_module.h"

// === Private ===

#define UC_DMA_TASK_QUEUE_DEPTH         16
#define UC_DMA_RESPONSE_IS_VALID(x)     (((x) & (1UL << 31)) != 0)
#define UC_DMA_RESPONSE_GET_BD_PTR(x)   (((x) & 0x7FFFFFFF))
#define UC_DMA_IS_RUNNING(x)  (((x) & 0x1F01) != 0)

static void * const REG_UC_DMA_DM2MM_TASK_QUEUE_PUSH       = (void *)REGDB_UC_MODULE_CORE_REGISTER(DMA_DM2MM_TASK_QUEUE_PUSH);
static void * const REG_UC_DMA_DM2MM_RESPONSE_QUEUE_POP    = (void *)REGDB_UC_MODULE_CORE_REGISTER(DMA_DM2MM_RESPONSE_QUEUE_POP);
static void * const REG_UC_DMA_MM2DM_TASK_QUEUE_PUSH       = (void *)REGDB_UC_MODULE_CORE_REGISTER(DMA_MM2DM_TASK_QUEUE_PUSH);
static void * const REG_UC_DMA_MM2DM_RESPONSE_QUEUE_POP    = (void *)REGDB_UC_MODULE_CORE_REGISTER(DMA_MM2DM_RESPONSE_QUEUE_POP);

// Buffer in Shared DM which is used by the uC DMA driver for internal BDs and data
static uint8_t uc_dma_work_buffer[32] __attribute__ ((aligned (16))) SHARED_DM_NOINIT;

#define DRIVER_BD_OFFSET                ((uint32_t)((void *)uc_dma_work_buffer - SHARED_DM_BASE))
#define DRIVER_BD_POINTER               ((void *)uc_dma_work_buffer)

typedef struct
{
#ifdef WITH_ASSERT
    uint32_t bd_ptr;
#endif
    uint16_t wait_handle;
}
uc_dma_active_task_t;

static uc_dma_active_task_t _uc_dma_dm2mm_active_tasks[UC_DMA_TASK_QUEUE_DEPTH] PRIVATE_DM_NOINIT;
static queue_t uc_dma_dm2mm_active_tasks PRIVATE_DM_NOINIT;

static uc_dma_active_task_t _uc_dma_mm2dm_active_tasks[UC_DMA_TASK_QUEUE_DEPTH] PRIVATE_DM_NOINIT;
static queue_t uc_dma_mm2dm_active_tasks PRIVATE_DM_NOINIT;

static inline uint32_t uc_dma_dm2mm_pop_response()
{
    return reg_read32(REG_UC_DMA_DM2MM_RESPONSE_QUEUE_POP);
}
static inline uint32_t uc_dma_mm2dm_pop_response()
{
    return reg_read32(REG_UC_DMA_MM2DM_RESPONSE_QUEUE_POP);
}
static inline uint32_t uc_dma_dm2mm_status()
{
    return reg_read32((void *)(REGDB_UC_MODULE_CORE_REGISTER(ARRAY_AXI_MM) | (column_index_rel << 25) | REGDB_UC_MODULE_REGISTER(DMA_DM2MM_STATUS)));
}
static inline uint32_t uc_dma_mm2dm_status()
{
    return reg_read32((void *)(REGDB_UC_MODULE_CORE_REGISTER(ARRAY_AXI_MM) | (column_index_rel << 25) | REGDB_UC_MODULE_REGISTER(DMA_MM2DM_STATUS)));
}

// === Public ===

void uc_dma_init()
{
    queue_init(&uc_dma_dm2mm_active_tasks, UC_DMA_TASK_QUEUE_DEPTH);
    queue_init(&uc_dma_mm2dm_active_tasks, UC_DMA_TASK_QUEUE_DEPTH);
}

void uc_dma_process_responses()
{
    uint32_t response;

    // DM2MM
    while (UC_DMA_RESPONSE_IS_VALID(response = uc_dma_dm2mm_pop_response()))
    {
        const unsigned int queue_index = queue_pop(&uc_dma_dm2mm_active_tasks, UC_DMA_TASK_QUEUE_DEPTH);
        assert(queue_index != INVALID_QUEUE_ENTRY);

        const uc_dma_active_task_t *task = &_uc_dma_dm2mm_active_tasks[queue_index];
#ifdef WITH_ASSERT
        assert(task->bd_ptr == UC_DMA_RESPONSE_GET_BD_PTR(response));
#endif
        wait_handle_release(task->wait_handle);
    }
}

void uc_dma_process_dm2mm_responses()
{
    uint32_t response;

    while (true)
    {
        response = uc_dma_dm2mm_pop_response();
        if (UC_DMA_RESPONSE_IS_VALID(response))
        {
            const unsigned int queue_index = queue_pop(&uc_dma_dm2mm_active_tasks, UC_DMA_TASK_QUEUE_DEPTH);
            assert(queue_index != INVALID_QUEUE_ENTRY);

            const uc_dma_active_task_t *task = &_uc_dma_dm2mm_active_tasks[queue_index];
#ifdef WITH_ASSERT
            assert(task->bd_ptr == UC_DMA_RESPONSE_GET_BD_PTR(response));
#endif
            wait_handle_release(task->wait_handle);
        }
        else
        {
            return;
        }
    }
}

void uc_dma_create_bd(void *dest,
                      uint16_t length,
                      bool ctrl_next_BD, bool ctrl_external, bool ctrl_local_relative, uint8_t ctrl_axi_mm_axuser,
                      uint32_t local_ptr,
                      uint32_t remote_ptr_high, uint32_t remote_ptr_low)
{
    uint32_t *dest_ = (uint32_t *)dest;
    dest_[3] = (remote_ptr_high & 0x1FFFFFF);
    dest_[2] = remote_ptr_low;
    dest_[1] = local_ptr;
    dest_[0] = ((uint32_t)length & 0x7FFF)
             | ((ctrl_next_BD & 1) << 16)
             | ((ctrl_external & 1) << 17)
             | ((ctrl_local_relative & 1) << 18)
             | ((ctrl_axi_mm_axuser & 0xF) << 19);
}

void uc_dma_create_bd_simple(void *dest, uint16_t length, uint32_t local_ptr, uint32_t remote_ptr_low)
{
    uint32_t *dest_ = (uint32_t *)dest;
    dest_[3] = 0;
    dest_[2] = remote_ptr_low;
    dest_[1] = local_ptr;
    dest_[0] = (uint32_t)length & 0x7FFF;
}

uint16_t uc_dma_dm2mm_push_task(const uint32_t bd_ptr)
{
    assert(bd_ptr < SHARED_DM_SIZE);

    if (UNLIKELY(queue_full(&uc_dma_dm2mm_active_tasks, UC_DMA_TASK_QUEUE_DEPTH)))
    {
        return INVALID_WAIT_HANDLE;
    }

    const uint16_t wait_handle = wait_handle_acquire();
    if (UNLIKELY(wait_handle == INVALID_WAIT_HANDLE))
    {
        return INVALID_WAIT_HANDLE;
    }

    const unsigned int queue_index = queue_push_nocheck(&uc_dma_dm2mm_active_tasks, UC_DMA_TASK_QUEUE_DEPTH);
    assert(queue_index != INVALID_QUEUE_ENTRY);

    uc_dma_active_task_t *task = &_uc_dma_dm2mm_active_tasks[queue_index];
#ifdef WITH_ASSERT
    task->bd_ptr = bd_ptr;
#endif
    task->wait_handle = wait_handle;

    reg_write32(REG_UC_DMA_DM2MM_TASK_QUEUE_PUSH, bd_ptr);

    return wait_handle;
}

uint16_t uc_dma_mm2dm_push_task(uint32_t bd_ptr)
{
    assert(bd_ptr < SHARED_DM_SIZE);

    if (UNLIKELY(queue_full(&uc_dma_mm2dm_active_tasks, UC_DMA_TASK_QUEUE_DEPTH)))
    {
        return INVALID_WAIT_HANDLE;
    }

    const uint16_t wait_handle = wait_handle_acquire();
    if (UNLIKELY(wait_handle == INVALID_WAIT_HANDLE))
    {
        return INVALID_WAIT_HANDLE;
    }

    const unsigned int queue_index = queue_push_nocheck(&uc_dma_mm2dm_active_tasks, UC_DMA_TASK_QUEUE_DEPTH);
    assert(queue_index != INVALID_QUEUE_ENTRY);

    uc_dma_active_task_t *task = &_uc_dma_mm2dm_active_tasks[queue_index];
#ifdef WITH_ASSERT
    task->bd_ptr = bd_ptr;
#endif
    task->wait_handle = wait_handle;

    reg_write32(REG_UC_DMA_MM2DM_TASK_QUEUE_PUSH, bd_ptr);

    return wait_handle;
}

#ifdef WITH_PCIE_ATOMICS
bool uc_dma_atomic_op(const uint32_t addr_high, const uint32_t addr_low,
    const uint8_t op, const uint64_t *payload, const unsigned int payload_dwords)
{
    uint64_t *payload_ptr = (uint64_t *)(DRIVER_BD_POINTER + 0x10);

    uc_dma_create_bd(DRIVER_BD_POINTER, payload_dwords * 2, false, CTRL_EXTERNAL, true, (op << 1) | (1 << 0), 0x10, addr_high, addr_low);
    for (unsigned int i = 0; i < payload_dwords; ++i)
    {
        payload_ptr[i] = payload[i];
    }
    mbar(1);

    // Push BD to DMA request queue
    reg_write32(REG_UC_DMA_DM2MM_TASK_QUEUE_PUSH, DRIVER_BD_OFFSET);

    // Wait for response in queue
    uint32_t response_value;
    while (!UC_DMA_RESPONSE_IS_VALID(response_value = reg_read32(REG_UC_DMA_DM2MM_RESPONSE_QUEUE_POP)))
        ;

    // Verify response value
    assert(UC_DMA_RESPONSE_GET_BD_PTR(response_value) == DRIVER_BD_OFFSET);

    // TODO Check for slverr?

    return true;
}
#else
bool uc_dma_atomic_op(const uint32_t addr_high, const uint32_t addr_low,
    const uint8_t op, const uint64_t *payload, const unsigned int payload_dwords)
{
    // No support for atomic -> execute in non-atomic way using normal transfers

    volatile uint64_t *data_ptr = (uint64_t *)(DRIVER_BD_POINTER + 0x10);

    if (op == ATOMICOP_NONE)
    {
        return true;
    }
    else if (op == ATOMICOP_FETCHADD)
    {
        assert(payload_dwords == 1);

        copy_mm2dm(
            (void *)data_ptr,
            addr_high,
            addr_low,
            sizeof(uint64_t));

        data_ptr[0] += payload[0];

        copy_dm2mm(
            addr_high,
            addr_low,
            (void *)data_ptr,
            sizeof(uint64_t));
    }
    else if (op == ATOMICOP_SWAP)
    {
        assert(payload_dwords == 1);

        data_ptr[0] = payload[0];

        copy_dm2mm(
            addr_high,
            addr_low,
            (void *)data_ptr,
            sizeof(uint64_t));
    }
    else if (op == ATOMICOP_CAS)
    {
        assert(payload_dwords == 2);

        copy_mm2dm(
            (void *)data_ptr,
            addr_high,
            addr_low,
            sizeof(uint64_t));

        if (data_ptr[0] == payload[0])
        {
            data_ptr[0] = payload[1];

            copy_dm2mm(
                addr_high,
                addr_low,
                (void *)data_ptr,
                sizeof(uint64_t));
        }
    }
    else
    {
        (void)payload_dwords; //avoid compile warning
        return false;
    }

    return true;
}
#endif

// Copy word-aligned data from memory to shared DM using the uC DMA.
// Block until copy is complete and all data is available in shared DM.
// The memory here means either external host ddr or internal memtile (for test purpose),
// which is specified by lx7_handshake
//
// @note  Only to be used when MM2DM channel is idle!
// @param dest      Destination pointer (in shared DM, word-aligned)
// @param src_high  Source pointer, high (in memory)
// @param src_low   Source pointer, low (in memory, word-aligned)
// @param length    Length (has to be word-aligned)
void copy_mm2dm(void *dest, uint32_t src_high, uint32_t src_low, unsigned int length)
{
    assert(dest >= SHARED_DM_BASE && dest < SHARED_DM_BASE + SHARED_DM_SIZE);
    assert(length % 4 == 0);
    assert(queue_empty(&uc_dma_mm2dm_active_tasks));

    uc_dma_create_bd(DRIVER_BD_POINTER, length >> 2, false, 1, false, 0, dest - SHARED_DM_BASE, src_high, src_low);
    mbar(1);

    reg_write32(REG_UC_DMA_MM2DM_TASK_QUEUE_PUSH, DRIVER_BD_OFFSET);

    uint32_t response;
    while (!UC_DMA_RESPONSE_IS_VALID(response = uc_dma_mm2dm_pop_response()))
        ;
    assert(UC_DMA_RESPONSE_GET_BD_PTR(response) == DRIVER_BD_OFFSET);
}

void copy_mm2dm_async(void *dest, uint32_t src_high, uint32_t src_low, unsigned int length)
{
    assert(dest >= SHARED_DM_BASE && dest < SHARED_DM_BASE + SHARED_DM_SIZE);
    assert(length % 4 == 0);
    assert(queue_empty(&uc_dma_mm2dm_active_tasks));

    uc_dma_create_bd(DRIVER_BD_POINTER, length >> 2, false, CTRL_EXTERNAL, false, 0, dest - SHARED_DM_BASE, src_high, src_low);
    mbar(1);

    reg_write32(REG_UC_DMA_MM2DM_TASK_QUEUE_PUSH, DRIVER_BD_OFFSET);
}

void copy_mm2dm_wait_done()
{
    uint32_t response;
    while (!UC_DMA_RESPONSE_IS_VALID(response = uc_dma_mm2dm_pop_response()))
        ;
    assert(UC_DMA_RESPONSE_GET_BD_PTR(response) == DRIVER_BD_OFFSET);
}

// Copy word-aligned data from shared DM to memory using the uC DMA.
// Block until copy is complete and all data has been copied to memory.
// The memory here means either external host ddr or internal memtile (for test purpose),
// which is specified by lx7_handshake
//
// @note  Only to be used when DM2MM channel is idle!
// @param dest_high Destination pointer, high (in memory)
// @param dest_low  Destination pointer, low (in memory, word-aligned)
// @param src       Source pointer (in shared DM, word-aligned)
// @param length    Length (has to be word-aligned)
void copy_dm2mm(uint32_t dest_high, uint32_t dest_low, void *src, unsigned int length)
{
    uint32_t status;

    // Need to wait for all ongoing dm2mm (in control code) complete if any
    // Note: there would be a case where there is already one completion in the response queue
    // because we enqueued task but didn't check response, but no task and/or outstanding
    // transaction in the task queue. so, a do-while loop is important here
    do
    {
      uc_dma_process_dm2mm_responses();
    }
    while (UC_DMA_IS_RUNNING(status = uc_dma_dm2mm_status()));

    assert(src >= SHARED_DM_BASE && src < SHARED_DM_BASE + SHARED_DM_SIZE);
    assert(length % 4 == 0);
    assert(queue_empty(&uc_dma_dm2mm_active_tasks));

    uc_dma_create_bd(DRIVER_BD_POINTER, length >> 2, false, 1, false, 0, src - SHARED_DM_BASE, dest_high, dest_low);
    mbar(1);

    reg_write32(REG_UC_DMA_DM2MM_TASK_QUEUE_PUSH, DRIVER_BD_OFFSET);

    uint32_t response;
    while (!UC_DMA_RESPONSE_IS_VALID(response = uc_dma_dm2mm_pop_response()))
        ;
    assert(UC_DMA_RESPONSE_GET_BD_PTR(response) == DRIVER_BD_OFFSET);
}

/** @} */