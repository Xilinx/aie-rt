/******************************************************************************
* Copyright (C) 2024 AMD, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file uc_dma.h
* @{
*
* This file contains implementation for uc DMA.
******************************************************************************/

#ifndef XAIE_UC_DMA_H
#define XAIE_UC_DMA_H

//#include "utils.h"

#include <stdint.h>
#include <stdbool.h>

#define ATOMICOP_NONE           0
#define ATOMICOP_FETCHADD       1
#define ATOMICOP_SWAP           2
#define ATOMICOP_CAS            3
#define INVALID_WAIT_HANDLE         0xFFFF

#define HSA_LOCATION 0
#define CTRL_EXTERNAL (HSA_LOCATION == EXTERNAL_AXI_MM)

enum hsa_location_t
{
  EXTERNAL_AXI_MM = 0,//by default, hsa queue resides in host ddr
  INTERNAL_AXI_MM = 1, //for bringup test purpose
};

void uc_dma_init();

void uc_dma_process_responses();
void uc_dma_process_dm2mm_responses();

void uc_dma_create_bd(void *dest,
                      uint16_t length,
                      bool ctrl_next_BD, bool ctrl_external, bool ctrl_local_relative, uint8_t ctrl_axi_mm_axuser,
                      uint32_t local_ptr,
                      uint32_t remote_ptr_high, uint32_t remote_ptr_low);
void uc_dma_create_bd_simple(void *dest, uint16_t length, uint32_t local_ptr, uint32_t remote_ptr_low);

uint16_t uc_dma_dm2mm_push_task(const uint32_t bd_ptr);
uint16_t uc_dma_mm2dm_push_task(const uint32_t bd_ptr);

bool uc_dma_atomic_op(const uint32_t addr_high, const uint32_t addr_low,
    const uint8_t op, const uint64_t *payload, const unsigned int payload_dwords);

void copy_mm2dm(void *dest, uint32_t src_high, uint32_t src_low, unsigned int length);

void copy_mm2dm_async(void *dest, uint32_t src_high, uint32_t src_low, unsigned int length);
void copy_mm2dm_wait_done();

void copy_dm2mm(uint32_t dest_high, uint32_t dest_low, void *src, unsigned int length);

#endif

/** @} */