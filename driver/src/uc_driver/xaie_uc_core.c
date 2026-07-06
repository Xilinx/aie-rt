/******************************************************************************
* Copyright (C) 2024 AMD, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file uc.c
* @{
*
* This file contains implementation of core control functionality from uC.
******************************************************************************/

#ifdef __linux__
#include <unistd.h>
#endif

#include "xaie_uc_core.h"
#include "xaie_events_aie.h"
#include "xaie_feature_config.h"
#include "xaie2psgbl_params.h"
#include "xaie_core_aieml.h"
#include "xaie_feature_config.h"
#include "xaiemlgbl_params.h"
#include "xaiegbl.h"
#include "xaiegbl_regdef.h"
#include "xaiegbl_params.h"
#include "xaie_txn_buffer.h"

#define ROW_SHIFT 20
#define COL_SHIFT 25

#define get_field(val, lsb, mask)	(((uint32_t)(val) & (mask)) >> (lsb))

static inline u64 get_tile_address(uint8_t row, uint8_t col)
{
	return (((uint64_t)row & 0xFFU) << ROW_SHIFT) |
		(((uint64_t)col & 0xFFU) << COL_SHIFT);
}
void uc_core_enable(uint8_t row, uint8_t col)
{
	uint32_t mask, value;
	uint64_t reg_addr;

	/* Clear the disable event occurred bit */
	mask = XAIE2PSGBL_CORE_MODULE_ENABLE_EVENTS_DISABLE_EVENT_OCCURRED_MASK |
		XAIEGBL_CORE_ENAEVE_ENAEVTOCC_MASK;
	value = (uint32_t)(1U << XAIE2PSGBL_CORE_MODULE_ENABLE_EVENTS_DISABLE_EVENT_OCCURRED_LSB);
	reg_addr = XAIE2PSGBL_CORE_MODULE_ENABLE_EVENTS +
		get_tile_address(row, col);

	uc_mask_write32(reg_addr, mask, value);

	/* Enable the core */
	mask = XAIE2PSGBL_CORE_MODULE_CORE_PROCESSOR_BUS_ENABLE_MASK;
	value = (uint32_t)(1U << XAIE2PSGBL_CORE_MODULE_CORE_PROCESSOR_BUS_ENABLE_LSB);
	reg_addr = XAIE2PSGBL_CORE_MODULE_CORE_CONTROL +
		get_tile_address(row, col);

	return uc_mask_write32(reg_addr, mask, value);
}

void uc_get_core_status(uint8_t row, uint8_t col, uint32_t* core_status)
{
	uint64_t reg_addr;
	uint32_t reg_val;
	uint32_t mask;

	mask = XAIEMLGBL_CORE_MODULE_CORE_STATUS_CORE_PROCESSOR_BUS_STALL_MASK |
		XAIEMLGBL_CORE_MODULE_CORE_STATUS_CORE_DONE_MASK |
		XAIEMLGBL_CORE_MODULE_CORE_STATUS_ERROR_HALT_MASK |
		XAIEMLGBL_CORE_MODULE_CORE_STATUS_ECC_SCRUBBING_STALL_MASK |
		XAIEMLGBL_CORE_MODULE_CORE_STATUS_ECC_ERROR_STALL_MASK |
		XAIEMLGBL_CORE_MODULE_CORE_STATUS_DEBUG_HALT_MASK |
		XAIEMLGBL_CORE_MODULE_CORE_STATUS_CASCADE_STALL_MCD_MASK |
		XAIEMLGBL_CORE_MODULE_CORE_STATUS_CASCADE_STALL_SCD_MASK |
		XAIEMLGBL_CORE_MODULE_CORE_STATUS_STREAM_STALL_MS0_MASK |
		XAIEMLGBL_CORE_MODULE_CORE_STATUS_STREAM_STALL_SS0_MASK |
		XAIEMLGBL_CORE_MODULE_CORE_STATUS_STREAM_STALL_SS0_MASK |
		XAIEMLGBL_CORE_MODULE_CORE_STATUS_LOCK_STALL_E_MASK |
		XAIEMLGBL_CORE_MODULE_CORE_STATUS_LOCK_STALL_N_MASK |
		XAIEMLGBL_CORE_MODULE_CORE_STATUS_LOCK_STALL_W_MASK |
		XAIEMLGBL_CORE_MODULE_CORE_STATUS_LOCK_STALL_S_MASK |
		XAIEMLGBL_CORE_MODULE_CORE_STATUS_MEMORY_STALL_E_MASK |
		XAIEMLGBL_CORE_MODULE_CORE_STATUS_MEMORY_STALL_N_MASK |
		XAIEMLGBL_CORE_MODULE_CORE_STATUS_MEMORY_STALL_W_MASK |
		XAIEMLGBL_CORE_MODULE_CORE_STATUS_MEMORY_STALL_S_MASK |
		XAIEMLGBL_CORE_MODULE_CORE_STATUS_RESET_MASK |
		XAIEMLGBL_CORE_MODULE_CORE_STATUS_RESET_MASK |
		XAIEMLGBL_CORE_MODULE_CORE_STATUS_ENABLE_MASK;

	/* Read core status register */
	reg_addr = XAIE2PSGBL_CORE_MODULE_CORE_STATUS +
		get_tile_address(row, col);

	uc_read32(reg_addr, &reg_val);

	*core_status = get_field(reg_val, 0U, mask);

	return;
}

/** @} */