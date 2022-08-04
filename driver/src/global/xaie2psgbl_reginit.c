/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie2pgbl_reginit.c
* @{
*
* This file contains the instances of the register bit field definitions for the
* Core, Memory, NoC and PL module registers.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who        Date         Changes
* ----- ------     --------     -----------------------------------------------------
* 1.0   Sankarji   07/011/2022  Initial creation
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xaie_core_aieml.h"
#include "xaie_device_aie2ipu.h"
#include "xaie_dma_aieml.h"
#include "xaie_locks_aieml.h"
#include "xaie_reset_aieml.h"
#include "xaie_ss_aieml.h"
#include "xaie2psgbl_params.h"
#include "xaiegbl_regdef.h"


/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/**************************** Macro Definitions ******************************/
#define XAIE2PS_TILES_BITMAPSIZE	0U

/************************** Variable Definitions *****************************/
/* bitmaps to capture modules being used by the application */
static u32 Aie2PSTilesInUse[XAIE2PS_TILES_BITMAPSIZE];
static u32 Aie2PSMemInUse[XAIE2PS_TILES_BITMAPSIZE];
static u32 Aie2PSCoreInUse[XAIE2PS_TILES_BITMAPSIZE];

#ifdef XAIE_FEATURE_CORE_ENABLE
/*
 * Global instance for Core module Core_Control register.
 */
static const  XAie_RegCoreCtrl Aie2PSCoreCtrlReg =
{
	XAIE2PSGBL_CORE_MODULE_CORE_CONTROL,
	{XAIE2PSGBL_CORE_MODULE_CORE_CONTROL_ENABLE_LSB, XAIE2PSGBL_CORE_MODULE_CORE_CONTROL_ENABLE_MASK},
	{XAIE2PSGBL_CORE_MODULE_CORE_CONTROL_RESET_LSB, XAIE2PSGBL_CORE_MODULE_CORE_CONTROL_RESET_MASK}
};

/*
 * Global instance for Core module Core_Status register.
 */
static const  XAie_RegCoreSts Aie2PSCoreStsReg =
{
	.RegOff = XAIE2PSGBL_CORE_MODULE_CORE_STATUS,
	.Mask = XAIE2PSGBL_CORE_MODULE_CORE_STATUS_CORE_PROCESSOR_BUS_STALL_MASK |
		XAIE2PSGBL_CORE_MODULE_CORE_STATUS_CORE_DONE_MASK |
		XAIE2PSGBL_CORE_MODULE_CORE_STATUS_ERROR_HALT_MASK |
		XAIE2PSGBL_CORE_MODULE_CORE_STATUS_ECC_SCRUBBING_STALL_MASK |
		XAIE2PSGBL_CORE_MODULE_CORE_STATUS_ECC_ERROR_STALL_MASK |
		XAIE2PSGBL_CORE_MODULE_CORE_STATUS_DEBUG_HALT_MASK |
		XAIE2PSGBL_CORE_MODULE_CORE_STATUS_CASCADE_STALL_MCD_MASK |
		XAIE2PSGBL_CORE_MODULE_CORE_STATUS_CASCADE_STALL_SCD_MASK |
		XAIE2PSGBL_CORE_MODULE_CORE_STATUS_STREAM_STALL_MS0_MASK |
		XAIE2PSGBL_CORE_MODULE_CORE_STATUS_STREAM_STALL_SS0_MASK |
		XAIE2PSGBL_CORE_MODULE_CORE_STATUS_STREAM_STALL_SS0_MASK |
		XAIE2PSGBL_CORE_MODULE_CORE_STATUS_LOCK_STALL_E_MASK |
		XAIE2PSGBL_CORE_MODULE_CORE_STATUS_LOCK_STALL_N_MASK |
		XAIE2PSGBL_CORE_MODULE_CORE_STATUS_LOCK_STALL_W_MASK |
		XAIE2PSGBL_CORE_MODULE_CORE_STATUS_LOCK_STALL_S_MASK |
		XAIE2PSGBL_CORE_MODULE_CORE_STATUS_MEMORY_STALL_E_MASK |
		XAIE2PSGBL_CORE_MODULE_CORE_STATUS_MEMORY_STALL_N_MASK |
		XAIE2PSGBL_CORE_MODULE_CORE_STATUS_MEMORY_STALL_W_MASK |
		XAIE2PSGBL_CORE_MODULE_CORE_STATUS_MEMORY_STALL_S_MASK |
		XAIE2PSGBL_CORE_MODULE_CORE_STATUS_RESET_MASK |
		XAIE2PSGBL_CORE_MODULE_CORE_STATUS_RESET_MASK |
		XAIE2PSGBL_CORE_MODULE_CORE_STATUS_ENABLE_MASK,
	.Done = {XAIE2PSGBL_CORE_MODULE_CORE_STATUS_CORE_DONE_LSB,
		XAIE2PSGBL_CORE_MODULE_CORE_STATUS_CORE_DONE_MASK},
	.Rst = {XAIE2PSGBL_CORE_MODULE_CORE_STATUS_RESET_LSB,
		XAIE2PSGBL_CORE_MODULE_CORE_STATUS_RESET_MASK},
	.En = {XAIE2PSGBL_CORE_MODULE_CORE_STATUS_ENABLE_LSB,
		XAIE2PSGBL_CORE_MODULE_CORE_STATUS_ENABLE_MASK}
};

/*
 * Global instance for Core module for core debug registers.
 */
static const XAie_RegCoreDebug Aie2PSCoreDebugReg =
{
	.RegOff = XAIE2PSGBL_CORE_MODULE_DEBUG_CONTROL0,
	.DebugCtrl1Offset = XAIE2PSGBL_CORE_MODULE_DEBUG_CONTROL1,
	.DebugHaltCoreEvent1.Lsb = XAIE2PSGBL_CORE_MODULE_DEBUG_CONTROL1_DEBUG_HALT_CORE_EVENT1_LSB,
	.DebugHaltCoreEvent1.Mask = XAIE2PSGBL_CORE_MODULE_DEBUG_CONTROL1_DEBUG_HALT_CORE_EVENT1_MASK,
	.DebugHaltCoreEvent0.Lsb = XAIE2PSGBL_CORE_MODULE_DEBUG_CONTROL1_DEBUG_HALT_CORE_EVENT0_LSB,
	.DebugHaltCoreEvent0.Mask = XAIE2PSGBL_CORE_MODULE_DEBUG_CONTROL1_DEBUG_HALT_CORE_EVENT0_MASK,
	.DebugSStepCoreEvent.Lsb = XAIE2PSGBL_CORE_MODULE_DEBUG_CONTROL1_DEBUG_SINGLESTEP_CORE_EVENT_LSB,
	.DebugSStepCoreEvent.Mask = XAIE2PSGBL_CORE_MODULE_DEBUG_CONTROL1_DEBUG_SINGLESTEP_CORE_EVENT_MASK,
	.DebugResumeCoreEvent.Lsb = XAIE2PSGBL_CORE_MODULE_DEBUG_CONTROL1_DEBUG_RESUME_CORE_EVENT_LSB,
	.DebugResumeCoreEvent.Mask = XAIE2PSGBL_CORE_MODULE_DEBUG_CONTROL1_DEBUG_RESUME_CORE_EVENT_MASK,
	.DebugHalt.Lsb = XAIE2PSGBL_CORE_MODULE_DEBUG_CONTROL0_DEBUG_HALT_BIT_LSB,
	.DebugHalt.Mask = XAIE2PSGBL_CORE_MODULE_DEBUG_CONTROL0_DEBUG_HALT_BIT_MASK
};

static const XAie_RegCoreDebugStatus Aie2PSCoreDebugStatus =
{
	.RegOff = XAIE2PSGBL_CORE_MODULE_DEBUG_STATUS,
	.DbgEvent1Halt.Lsb = XAIE2PSGBL_CORE_MODULE_DEBUG_STATUS_DEBUG_EVENT1_HALTED_LSB,
	.DbgEvent1Halt.Mask = XAIE2PSGBL_CORE_MODULE_DEBUG_STATUS_DEBUG_EVENT1_HALTED_MASK,
	.DbgEvent0Halt.Lsb = XAIE2PSGBL_CORE_MODULE_DEBUG_STATUS_DEBUG_EVENT0_HALTED_LSB,
	.DbgEvent0Halt.Mask = XAIE2PSGBL_CORE_MODULE_DEBUG_STATUS_DEBUG_EVENT0_HALTED_MASK,
	.DbgStrmStallHalt.Lsb = XAIE2PSGBL_CORE_MODULE_DEBUG_STATUS_STREAM_STALL_HALTED_LSB,
	.DbgStrmStallHalt.Mask = XAIE2PSGBL_CORE_MODULE_DEBUG_STATUS_STREAM_STALL_HALTED_MASK,
	.DbgLockStallHalt.Lsb = XAIE2PSGBL_CORE_MODULE_DEBUG_STATUS_LOCK_STALL_HALTED_LSB,
	.DbgLockStallHalt.Mask = XAIE2PSGBL_CORE_MODULE_DEBUG_STATUS_LOCK_STALL_HALTED_MASK,
	.DbgMemStallHalt.Lsb = XAIE2PSGBL_CORE_MODULE_DEBUG_STATUS_MEMORY_STALL_HALTED_LSB,
	.DbgMemStallHalt.Mask = XAIE2PSGBL_CORE_MODULE_DEBUG_STATUS_MEMORY_STALL_HALTED_MASK,
	.DbgPCEventHalt.Lsb = XAIE2PSGBL_CORE_MODULE_DEBUG_STATUS_PC_EVENT_HALTED_LSB,
	.DbgPCEventHalt.Mask = XAIE2PSGBL_CORE_MODULE_DEBUG_STATUS_PC_EVENT_HALTED_MASK,
	.DbgHalt.Lsb = XAIE2PSGBL_CORE_MODULE_DEBUG_STATUS_DEBUG_HALTED_LSB,
	.DbgHalt.Mask = XAIE2PSGBL_CORE_MODULE_DEBUG_STATUS_DEBUG_HALTED_MASK,
};

/*
 * Global instance for core event registers in the core module.
 */
static const XAie_RegCoreEvents Aie2PSCoreEventReg =
{
	.EnableEventOff = XAIE2PSGBL_CORE_MODULE_ENABLE_EVENTS,
	.DisableEventOccurred.Lsb = XAIE2PSGBL_CORE_MODULE_ENABLE_EVENTS_DISABLE_EVENT_OCCURRED_LSB,
	.DisableEventOccurred.Mask = XAIE2PSGBL_CORE_MODULE_ENABLE_EVENTS_DISABLE_EVENT_OCCURRED_MASK,
	.DisableEvent.Lsb = XAIE2PSGBL_CORE_MODULE_ENABLE_EVENTS_DISABLE_EVENT_LSB,
	.DisableEvent.Mask = XAIE2PSGBL_CORE_MODULE_ENABLE_EVENTS_DISABLE_EVENT_MASK,
	.EnableEvent.Lsb = XAIE2PSGBL_CORE_MODULE_ENABLE_EVENTS_ENABLE_EVENT_LSB,
	.EnableEvent.Mask = XAIE2PSGBL_CORE_MODULE_ENABLE_EVENTS_ENABLE_EVENT_MASK,
};

/*
 * Global instance for core accumulator control register.
 */
static const XAie_RegCoreAccumCtrl Aie2PSCoreAccumCtrlReg =
{
	.RegOff = XAIE2PSGBL_CORE_MODULE_ACCUMULATOR_CONTROL,
	.CascadeInput.Lsb = XAIE2PSGBL_CORE_MODULE_ACCUMULATOR_CONTROL_INPUT_LSB,
	.CascadeInput.Mask = XAIE2PSGBL_CORE_MODULE_ACCUMULATOR_CONTROL_INPUT_MASK,
	.CascadeOutput.Lsb = XAIE2PSGBL_CORE_MODULE_ACCUMULATOR_CONTROL_OUTPUT_LSB,
	.CascadeOutput.Mask = XAIE2PSGBL_CORE_MODULE_ACCUMULATOR_CONTROL_OUTPUT_MASK,
};
#endif /* XAIE_FEATURE_CORE_ENABLE */

#ifdef XAIE_FEATURE_CORE_ENABLE
/* Register field attribute for core process bus control */
static const XAie_RegCoreProcBusCtrl Aie2PSCoreProcBusCtrlReg =
{
	.RegOff = XAIE2PSGBL_CORE_MODULE_CORE_PROCESSOR_BUS,
	.CtrlEn = {XAIE2PSGBL_CORE_MODULE_CORE_PROCESSOR_BUS_ENABLE_LSB, XAIE2PSGBL_CORE_MODULE_CORE_PROCESSOR_BUS_ENABLE_MASK}
};

/* Core Module */
static const  XAie_CoreMod Aie2PSCoreMod =
{
	.IsCheckerBoard = 0U,
	.ProgMemAddr = 0x0,
	.ProgMemSize = 16 * 1024,
	.DataMemAddr = 0x40000,
	.ProgMemHostOffset = XAIE2PSGBL_CORE_MODULE_PROGRAM_MEMORY,
	.DataMemSize = 64 * 1024,		/* AIE2PS Tile Memory is 64kB */
	.DataMemShift = 16,
	.EccEvntRegOff = XAIE2PSGBL_CORE_MODULE_ECC_SCRUBBING_EVENT,
	.CoreCtrl = &Aie2PSCoreCtrlReg,
	.CoreDebugStatus = &Aie2PSCoreDebugStatus,
	.CoreSts = &Aie2PSCoreStsReg,
	.CoreDebug = &Aie2PSCoreDebugReg,
	.CoreEvent = &Aie2PSCoreEventReg,
	.CoreAccumCtrl = &Aie2PSCoreAccumCtrlReg,
	.ProcBusCtrl = &Aie2PSCoreProcBusCtrlReg,
	.ConfigureDone = &_XAieMl_CoreConfigureDone,
	.Enable = &_XAieMl_CoreEnable,
	.WaitForDone = &_XAieMl_CoreWaitForDone,
	.ReadDoneBit = &_XAieMl_CoreReadDoneBit,
};
#endif /* XAIE_FEATURE_CORE_ENABLE */

#ifdef XAIE_FEATURE_SS_ENABLE
/*
 * Array of all Tile Stream Switch Master Config registers
 * The data structure contains number of ports and the register offsets
 */
static const  XAie_StrmPort Aie2PSTileStrmMstr[SS_PORT_TYPE_MAX] =
{
	{	/* Core */
		.NumPorts = 1,
		.PortBaseAddr = XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0,
	},
	{	/* DMA */
		.NumPorts = 2,
		.PortBaseAddr = XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_DMA0,
	},
	{	/* Ctrl */
		.NumPorts = 1,
		.PortBaseAddr = XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_TILE_CTRL,
	},
	{	/* Fifo */
		.NumPorts = 1,
		.PortBaseAddr = XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_FIFO0,
	},
	{	/* South */
		.NumPorts = 4,
		.PortBaseAddr = XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_SOUTH0,
	},
	{	/* West */
		.NumPorts = 4,
		.PortBaseAddr = XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_WEST0,
	},
	{	/* North */
		.NumPorts = 6,
		.PortBaseAddr = XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_NORTH0,
	},
	{	/* East */
		.NumPorts = 4,
		.PortBaseAddr = XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_EAST0,
	},
	{	/* Trace */
		.NumPorts = 0,
		.PortBaseAddr = 0
	}
};

/*
 * Array of all Tile Stream Switch Slave Config registers
 * The data structure contains number of ports and the register offsets
 */
static const  XAie_StrmPort Aie2PSTileStrmSlv[SS_PORT_TYPE_MAX] =
{
	{	/* Core */
		.NumPorts = 1,
		.PortBaseAddr = XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_AIE_CORE0,
	},
	{	/* DMA */
		.NumPorts = 2,
		.PortBaseAddr = XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_DMA_0,
	},
	{	/* Ctrl */
		.NumPorts = 1,
		.PortBaseAddr = XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_TILE_CTRL,
	},
	{	/* Fifo */
		.NumPorts = 1,
		.PortBaseAddr = XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_FIFO_0,
	},
	{	/* South */
		.NumPorts = 6,
		.PortBaseAddr = XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_SOUTH_0,
	},
	{	/* West */
		.NumPorts = 4,
		.PortBaseAddr = XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_WEST_0,
	},
	{	/* North */
		.NumPorts = 4,
		.PortBaseAddr = XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_NORTH_0,
	},
	{	/* East */
		.NumPorts = 4,
		.PortBaseAddr = XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_EAST_0,
	},
	{	/* Trace */
		.NumPorts = 2,
		.PortBaseAddr = XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_MEM_TRACE
	}
};

/*
 * Array of all Shim NOC/PL Stream Switch Master Config registers
 * The data structure contains number of ports and the register offsets
 */
static const  XAie_StrmPort Aie2PShimStrmMstr[SS_PORT_TYPE_MAX] =
{
	{	/* Core */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* DMA */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* Ctrl */
		.NumPorts = 1,
		.PortBaseAddr = XAIE2PSGBL_PL_MODULE_STREAM_SWITCH_MASTER_CONFIG_TILE_CTRL,
	},
	{	/* Fifo */
		.NumPorts = 1,
		.PortBaseAddr = XAIE2PSGBL_PL_MODULE_STREAM_SWITCH_MASTER_CONFIG_FIFO0,
	},
	{	/* South */
		.NumPorts = 6,
		.PortBaseAddr = XAIE2PSGBL_PL_MODULE_STREAM_SWITCH_MASTER_CONFIG_SOUTH0,
	},
	{	/* West */
		.NumPorts = 4,
		.PortBaseAddr = XAIE2PSGBL_PL_MODULE_STREAM_SWITCH_MASTER_CONFIG_WEST0,
	},
	{	/* North */
		.NumPorts = 6,
		.PortBaseAddr = XAIE2PSGBL_PL_MODULE_STREAM_SWITCH_MASTER_CONFIG_NORTH0,
	},
	{	/* East */
		.NumPorts = 4,
		.PortBaseAddr = XAIE2PSGBL_PL_MODULE_STREAM_SWITCH_MASTER_CONFIG_EAST0,
	},
	{	/* Trace */
		.NumPorts = 0,
		.PortBaseAddr = 0
	}
};

/*
 * Array of all Shim NOC/PL Stream Switch Slave Config registers
 * The data structure contains number of ports and the register offsets
 */
static const  XAie_StrmPort Aie2PShimStrmSlv[SS_PORT_TYPE_MAX] =
{
	{	/* Core */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* DMA */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* Ctrl */
		.NumPorts = 1,
		.PortBaseAddr = XAIE2PSGBL_PL_MODULE_STREAM_SWITCH_SLAVE_CONFIG_TILE_CTRL,
	},
	{	/* Fifo */
		.NumPorts = 1,
		.PortBaseAddr = XAIE2PSGBL_PL_MODULE_STREAM_SWITCH_SLAVE_CONFIG_FIFO_0,
	},
	{	/* South */
		.NumPorts = 8,
		.PortBaseAddr = XAIE2PSGBL_PL_MODULE_STREAM_SWITCH_SLAVE_CONFIG_SOUTH_0,
	},
	{	/* West */
		.NumPorts = 4,
		.PortBaseAddr = XAIE2PSGBL_PL_MODULE_STREAM_SWITCH_SLAVE_CONFIG_WEST_0,
	},
	{	/* North */
		.NumPorts = 4,
		.PortBaseAddr = XAIE2PSGBL_PL_MODULE_STREAM_SWITCH_SLAVE_CONFIG_NORTH_0,
	},
	{	/* East */
		.NumPorts = 4,
		.PortBaseAddr = XAIE2PSGBL_PL_MODULE_STREAM_SWITCH_SLAVE_CONFIG_EAST_0,
	},
	{	/* Trace */
		.NumPorts = 2,
		.PortBaseAddr = XAIE2PSGBL_PL_MODULE_STREAM_SWITCH_SLAVE_CONFIG_TRACE
	}
};

/*
 * Array of all Mem Tile Stream Switch Master Config registers
 * The data structure contains number of ports and the register offsets
 */
static const  XAie_StrmPort Aie2PSMemTileStrmMstr[SS_PORT_TYPE_MAX] =
{
	{	/* Core */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* DMA */
		.NumPorts = 6,
		.PortBaseAddr = XAIE2PSGBL_MEM_TILE_MODULE_STREAM_SWITCH_MASTER_CONFIG_DMA0,
	},
	{	/* Ctrl */
		.NumPorts = 1,
		.PortBaseAddr = XAIE2PSGBL_MEM_TILE_MODULE_STREAM_SWITCH_MASTER_CONFIG_TILE_CTRL,
	},
	{	/* Fifo */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* South */
		.NumPorts = 4,
		.PortBaseAddr = XAIE2PSGBL_MEM_TILE_MODULE_STREAM_SWITCH_MASTER_CONFIG_SOUTH0,
	},
	{	/* West */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* North */
		.NumPorts = 6,
		.PortBaseAddr = XAIE2PSGBL_MEM_TILE_MODULE_STREAM_SWITCH_MASTER_CONFIG_NORTH0,
	},
	{	/* East */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* Trace */
		.NumPorts = 0,
		.PortBaseAddr = 0
	}
};

/*
 * Array of all Mem Tile Stream Switch Slave Config registers
 * The data structure contains number of ports and the register offsets
 */
static const  XAie_StrmPort Aie2PSMemTileStrmSlv[SS_PORT_TYPE_MAX] =
{
	{	/* Core */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* DMA */
		.NumPorts = 6,
		.PortBaseAddr = XAIE2PSGBL_MEM_TILE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_DMA_0,
	},
	{	/* Ctrl */
		.NumPorts = 1,
		.PortBaseAddr = XAIE2PSGBL_MEM_TILE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_TILE_CTRL,
	},
	{	/* Fifo */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* South */
		.NumPorts = 6,
		.PortBaseAddr = XAIE2PSGBL_MEM_TILE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_SOUTH_0,
	},
	{	/* West */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* North */
		.NumPorts = 4,
		.PortBaseAddr = XAIE2PSGBL_MEM_TILE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_NORTH_0,
	},
	{	/* East */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* Trace */
		.NumPorts = 1,
		.PortBaseAddr = XAIE2PSGBL_MEM_TILE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_TRACE
	}
};

/*
 * Array of all Shim NOC/PL Stream Switch Slave Slot Config registers of AIE2PS.
 * The data structure contains number of ports and the register base address.
 */
static const  XAie_StrmPort Aie2PShimStrmSlaveSlot[SS_PORT_TYPE_MAX] =
{
	{	/* Core */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* DMA */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* Ctrl */
		.NumPorts = 1,
		.PortBaseAddr = XAIE2PSGBL_PL_MODULE_STREAM_SWITCH_SLAVE_TILE_CTRL_SLOT0,
	},
	{	/* Fifo */
		.NumPorts = 1,
		.PortBaseAddr = XAIE2PSGBL_PL_MODULE_STREAM_SWITCH_SLAVE_FIFO_0_SLOT0,
	},
	{	/* South */
		.NumPorts = 8,
		.PortBaseAddr = XAIE2PSGBL_PL_MODULE_STREAM_SWITCH_SLAVE_SOUTH_0_SLOT0,
	},
	{	/* West */
		.NumPorts = 4,
		.PortBaseAddr = XAIE2PSGBL_PL_MODULE_STREAM_SWITCH_SLAVE_WEST_0_SLOT0,
	},
	{	/* North */
		.NumPorts = 4,
		.PortBaseAddr = XAIE2PSGBL_PL_MODULE_STREAM_SWITCH_SLAVE_NORTH_0_SLOT0,
	},
	{	/* East */
		.NumPorts = 4,
		.PortBaseAddr = XAIE2PSGBL_PL_MODULE_STREAM_SWITCH_SLAVE_EAST_0_SLOT0,
	},
	{	/* Trace */
		.NumPorts = 1,
		.PortBaseAddr = XAIE2PSGBL_PL_MODULE_STREAM_SWITCH_SLAVE_TRACE_SLOT0
	}
};

/*
 * Array of all AIE2PS Tile Stream Switch Slave Slot Config registers.
 * The data structure contains number of ports and the register offsets
 */
static const  XAie_StrmPort Aie2PSTileStrmSlaveSlot[SS_PORT_TYPE_MAX] =
{
	{	/* Core */
		.NumPorts = 1,
		.PortBaseAddr = XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0,
	},
	{	/* DMA */
		.NumPorts = 2,
		.PortBaseAddr = XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_DMA_0_SLOT0,
	},
	{	/* Ctrl */
		.NumPorts = 1,
		.PortBaseAddr = XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_TILE_CTRL_SLOT0,
	},
	{	/* Fifo */
		.NumPorts = 1,
		.PortBaseAddr = XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_FIFO_0_SLOT0,
	},
	{	/* South */
		.NumPorts = 6,
		.PortBaseAddr = XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_SOUTH_0_SLOT0,
	},
	{	/* West */
		.NumPorts = 4,
		.PortBaseAddr = XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_WEST_0_SLOT0,
	},
	{	/* North */
		.NumPorts = 4,
		.PortBaseAddr = XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_NORTH_0_SLOT0,
	},
	{	/* East */
		.NumPorts = 4,
		.PortBaseAddr = XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_EAST_0_SLOT0,
	},
	{	/* Trace */
		.NumPorts = 1,
		.PortBaseAddr = XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_TRACE_SLOT0
	}
};

/*
 * Array of all AIE2PS Mem Tile Stream Switch Slave Slot Config registers
 * The data structure contains number of ports and the register offsets
 */
static const  XAie_StrmPort Aie2PSMemTileStrmSlaveSlot[SS_PORT_TYPE_MAX] =
{
	{	/* Core */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* DMA */
		.NumPorts = 6,
		.PortBaseAddr = XAIE2PSGBL_MEM_TILE_MODULE_STREAM_SWITCH_SLAVE_DMA_0_SLOT0,
	},
	{	/* Ctrl */
		.NumPorts = 1,
		.PortBaseAddr = XAIE2PSGBL_MEM_TILE_MODULE_STREAM_SWITCH_SLAVE_TILE_CTRL_SLOT0,
	},
	{	/* Fifo */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* South */
		.NumPorts = 6,
		.PortBaseAddr = XAIE2PSGBL_MEM_TILE_MODULE_STREAM_SWITCH_SLAVE_SOUTH_0_SLOT0,
	},
	{	/* West */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* North */
		.NumPorts = 4,
		.PortBaseAddr = XAIE2PSGBL_MEM_TILE_MODULE_STREAM_SWITCH_SLAVE_NORTH_0_SLOT0,
	},
	{	/* East */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* Trace */
		.NumPorts = 1,
		.PortBaseAddr = XAIE2PSGBL_MEM_TILE_MODULE_STREAM_SWITCH_SLAVE_TRACE_SLOT0,
	}
};

static const XAie_StrmSwPortMap Aie2PSTileStrmSwMasterPortMap[] =
{
	{
		/* PhyPort 0 */
		.PortType = CORE,
		.PortNum = 0,
	},
	{
		/* PhyPort 1 */
		.PortType = DMA,
		.PortNum = 0,
	},
	{
		/* PhyPort 2 */
		.PortType = DMA,
		.PortNum = 1,
	},
	{
		/* PhyPort 3 */
		.PortType = CTRL,
		.PortNum = 0,
	},
	{
		/* PhyPort 4 */
		.PortType = FIFO,
		.PortNum = 0,
	},
	{
		/* PhyPort 5 */
		.PortType = SOUTH,
		.PortNum = 0,
	},
	{
		/* PhyPort 6 */
		.PortType = SOUTH,
		.PortNum = 1,
	},
	{
		/* PhyPort 7 */
		.PortType = SOUTH,
		.PortNum = 2,
	},
	{
		/* PhyPort 8 */
		.PortType = SOUTH,
		.PortNum = 3,
	},
	{
		/* PhyPort 9 */
		.PortType = WEST,
		.PortNum = 0,
	},
	{
		/* PhyPort 10 */
		.PortType = WEST,
		.PortNum = 1,
	},
	{
		/* PhyPort 11 */
		.PortType = WEST,
		.PortNum = 2,
	},
	{
		/* PhyPort 12 */
		.PortType = WEST,
		.PortNum = 3,
	},
	{
		/* PhyPort 13 */
		.PortType = NORTH,
		.PortNum = 0,
	},
	{
		/* PhyPort 14 */
		.PortType = NORTH,
		.PortNum = 1,
	},
	{
		/* PhyPort 15 */
		.PortType = NORTH,
		.PortNum = 2,
	},
	{
		/* PhyPort 16 */
		.PortType = NORTH,
		.PortNum = 3,
	},
	{
		/* PhyPort 17 */
		.PortType = NORTH,
		.PortNum = 4,
	},
	{
		/* PhyPort 18 */
		.PortType = NORTH,
		.PortNum = 5,
	},
	{
		/* PhyPort 19 */
		.PortType = EAST,
		.PortNum = 0,
	},
	{
		/* PhyPort 20 */
		.PortType = EAST,
		.PortNum = 1,
	},
	{
		/* PhyPort 21 */
		.PortType = EAST,
		.PortNum = 2,
	},
	{
		/* PhyPort 22 */
		.PortType = EAST,
		.PortNum = 3,
	},
};

static const XAie_StrmSwPortMap Aie2PSTileStrmSwSlavePortMap[] =
{
	{
		/* PhyPort 0 */
		.PortType = CORE,
		.PortNum = 0,
	},
	{
		/* PhyPort 1 */
		.PortType = DMA,
		.PortNum = 0,
	},
	{
		/* PhyPort 2 */
		.PortType = DMA,
		.PortNum = 1,
	},
	{
		/* PhyPort 3 */
		.PortType = CTRL,
		.PortNum = 0,
	},
	{
		/* PhyPort 4 */
		.PortType = FIFO,
		.PortNum = 0,
	},
	{
		/* PhyPort 5 */
		.PortType = SOUTH,
		.PortNum = 0,
	},
	{
		/* PhyPort 6 */
		.PortType = SOUTH,
		.PortNum = 1,
	},
	{
		/* PhyPort 7 */
		.PortType = SOUTH,
		.PortNum = 2,
	},
	{
		/* PhyPort 8 */
		.PortType = SOUTH,
		.PortNum = 3,
	},
	{
		/* PhyPort 9 */
		.PortType = SOUTH,
		.PortNum = 4,
	},
	{
		/* PhyPort 10 */
		.PortType = SOUTH,
		.PortNum = 5,
	},
	{
		/* PhyPort 11 */
		.PortType = WEST,
		.PortNum = 0,
	},
	{
		/* PhyPort 12 */
		.PortType = WEST,
		.PortNum = 1,
	},
	{
		/* PhyPort 13 */
		.PortType = WEST,
		.PortNum = 2,
	},
	{
		/* PhyPort 14 */
		.PortType = WEST,
		.PortNum = 3,
	},
	{
		/* PhyPort 15 */
		.PortType = NORTH,
		.PortNum = 0,
	},
	{
		/* PhyPort 16 */
		.PortType = NORTH,
		.PortNum = 1,
	},
	{
		/* PhyPort 17 */
		.PortType = NORTH,
		.PortNum = 2,
	},
	{
		/* PhyPort 18 */
		.PortType = NORTH,
		.PortNum = 3,
	},
	{
		/* PhyPort 19 */
		.PortType = EAST,
		.PortNum = 0,
	},
	{
		/* PhyPort 20 */
		.PortType = EAST,
		.PortNum = 1,
	},
	{
		/* PhyPort 21 */
		.PortType = EAST,
		.PortNum = 2,
	},
	{
		/* PhyPort 22 */
		.PortType = EAST,
		.PortNum = 3,
	},
	{
		/* PhyPort 23 */
		.PortType = TRACE,
		.PortNum = 0,
	},
	{
		/* PhyPort 24 */
		.PortType = TRACE,
		.PortNum = 1,
	},
};

static const XAie_StrmSwPortMap Aie2PShimStrmSwMasterPortMap[] =
{
	{
		/* PhyPort 0 */
		.PortType = CTRL,
		.PortNum = 0,
	},
	{
		/* PhyPort 1 */
		.PortType = FIFO,
		.PortNum = 0,
	},
	{
		/* PhyPort 2 */
		.PortType = SOUTH,
		.PortNum = 0,
	},
	{
		/* PhyPort 3 */
		.PortType = SOUTH,
		.PortNum = 1,
	},
	{
		/* PhyPort 4 */
		.PortType = SOUTH,
		.PortNum = 2,
	},
	{
		/* PhyPort 5 */
		.PortType = SOUTH,
		.PortNum = 3,
	},
	{
		/* PhyPort 6 */
		.PortType = SOUTH,
		.PortNum = 4,
	},
	{
		/* PhyPort 7 */
		.PortType = SOUTH,
		.PortNum = 5,
	},
	{
		/* PhyPort 8 */
		.PortType = WEST,
		.PortNum = 0,
	},
	{
		/* PhyPort 9 */
		.PortType = WEST,
		.PortNum = 1,
	},
	{
		/* PhyPort 10 */
		.PortType = WEST,
		.PortNum = 2,
	},
	{
		/* PhyPort 11 */
		.PortType = WEST,
		.PortNum = 3,
	},
	{
		/* PhyPort 12 */
		.PortType = NORTH,
		.PortNum = 0,
	},
	{
		/* PhyPort 13 */
		.PortType = NORTH,
		.PortNum = 1,
	},
	{
		/* PhyPort 14 */
		.PortType = NORTH,
		.PortNum = 2,
	},
	{
		/* PhyPort 15 */
		.PortType = NORTH,
		.PortNum = 3,
	},
	{
		/* PhyPort 16 */
		.PortType = NORTH,
		.PortNum = 4,
	},
	{
		/* PhyPort 17 */
		.PortType = NORTH,
		.PortNum = 5,
	},
	{
		/* PhyPort 18 */
		.PortType = EAST,
		.PortNum = 0,
	},
	{
		/* PhyPort 19 */
		.PortType = EAST,
		.PortNum = 1,
	},
	{
		/* PhyPort 20 */
		.PortType = EAST,
		.PortNum = 2,
	},
	{
		/* PhyPort 21 */
		.PortType = EAST,
		.PortNum = 3,
	},
};

static const XAie_StrmSwPortMap Aie2PShimStrmSwSlavePortMap[] =
{
	{
		/* PhyPort 0 */
		.PortType = CTRL,
		.PortNum = 0,
	},
	{
		/* PhyPort 1 */
		.PortType = FIFO,
		.PortNum = 0,
	},
	{
		/* PhyPort 2 */
		.PortType = SOUTH,
		.PortNum = 0,
	},
	{
		/* PhyPort 3 */
		.PortType = SOUTH,
		.PortNum = 1,
	},
	{
		/* PhyPort 4 */
		.PortType = SOUTH,
		.PortNum = 2,
	},
	{
		/* PhyPort 5 */
		.PortType = SOUTH,
		.PortNum = 3,
	},
	{
		/* PhyPort 6 */
		.PortType = SOUTH,
		.PortNum = 4,
	},
	{
		/* PhyPort 7 */
		.PortType = SOUTH,
		.PortNum = 5,
	},
	{
		/* PhyPort 8 */
		.PortType = SOUTH,
		.PortNum = 6,
	},
	{
		/* PhyPort 9 */
		.PortType = SOUTH,
		.PortNum = 7,
	},
	{
		/* PhyPort 10 */
		.PortType = WEST,
		.PortNum = 0,
	},
	{
		/* PhyPort 11 */
		.PortType = WEST,
		.PortNum = 1,
	},
	{
		/* PhyPort 12 */
		.PortType = WEST,
		.PortNum = 2,
	},
	{
		/* PhyPort 13 */
		.PortType = WEST,
		.PortNum = 3,
	},
	{
		/* PhyPort 14 */
		.PortType = NORTH,
		.PortNum = 0,
	},
	{
		/* PhyPort 15 */
		.PortType = NORTH,
		.PortNum = 1,
	},
	{
		/* PhyPort 16 */
		.PortType = NORTH,
		.PortNum = 2,
	},
	{
		/* PhyPort 17 */
		.PortType = NORTH,
		.PortNum = 3,
	},
	{
		/* PhyPort 18 */
		.PortType = EAST,
		.PortNum = 0,
	},
	{
		/* PhyPort 19 */
		.PortType = EAST,
		.PortNum = 1,
	},
	{
		/* PhyPort 20 */
		.PortType = EAST,
		.PortNum = 2,
	},
	{
		/* PhyPort 21 */
		.PortType = EAST,
		.PortNum = 3,
	},
	{
		/* PhyPort 22 */
		.PortType = TRACE,
		.PortNum = 0,
	},
};

static const XAie_StrmSwPortMap Aie2PSMemTileStrmSwMasterPortMap[] =
{
	{
		/* PhyPort 0 */
		.PortType = DMA,
		.PortNum = 0,
	},
	{
		/* PhyPort 1 */
		.PortType = DMA,
		.PortNum = 1,
	},
	{
		/* PhyPort 2 */
		.PortType = DMA,
		.PortNum = 2,
	},
	{
		/* PhyPort 3 */
		.PortType = DMA,
		.PortNum = 3,
	},
	{
		/* PhyPort 4 */
		.PortType = DMA,
		.PortNum = 4,
	},
	{
		/* PhyPort 5 */
		.PortType = DMA,
		.PortNum = 5,
	},
	{
		/* PhyPort 6 */
		.PortType = CTRL,
		.PortNum = 0,
	},
	{
		/* PhyPort 7 */
		.PortType = SOUTH,
		.PortNum = 0,
	},
	{
		/* PhyPort 8 */
		.PortType = SOUTH,
		.PortNum = 1,
	},
	{
		/* PhyPort 9 */
		.PortType = SOUTH,
		.PortNum = 2,
	},
	{
		/* PhyPort 10 */
		.PortType = SOUTH,
		.PortNum = 3,
	},
	{
		/* PhyPort 11 */
		.PortType = NORTH,
		.PortNum = 0,
	},
	{
		/* PhyPort 12 */
		.PortType = NORTH,
		.PortNum = 1,
	},
	{
		/* PhyPort 13 */
		.PortType = NORTH,
		.PortNum = 2,
	},
	{
		/* PhyPort 14 */
		.PortType = NORTH,
		.PortNum = 3,
	},
	{
		/* PhyPort 15 */
		.PortType = NORTH,
		.PortNum = 4,
	},
	{
		/* PhyPort 16 */
		.PortType = NORTH,
		.PortNum = 5,
	},
};

static const XAie_StrmSwPortMap Aie2PSMemTileStrmSwSlavePortMap[] =
{
	{
		/* PhyPort 0 */
		.PortType = DMA,
		.PortNum = 0,
	},
	{
		/* PhyPort 1 */
		.PortType = DMA,
		.PortNum = 1,
	},
	{
		/* PhyPort 2 */
		.PortType = DMA,
		.PortNum = 2,
	},
	{
		/* PhyPort 3 */
		.PortType = DMA,
		.PortNum = 3,
	},
	{
		/* PhyPort 4 */
		.PortType = DMA,
		.PortNum = 4,
	},
	{
		/* PhyPort 5 */
		.PortType = DMA,
		.PortNum = 5,
	},
	{
		/* PhyPort 6 */
		.PortType = CTRL,
		.PortNum = 0,
	},
	{
		/* PhyPort 7 */
		.PortType = SOUTH,
		.PortNum = 0,
	},
	{
		/* PhyPort 8 */
		.PortType = SOUTH,
		.PortNum = 1,
	},
	{
		/* PhyPort 9 */
		.PortType = SOUTH,
		.PortNum = 2,
	},
	{
		/* PhyPort 10 */
		.PortType = SOUTH,
		.PortNum = 3,
	},
	{
		/* PhyPort 11 */
		.PortType = NORTH,
		.PortNum = 0,
	},
	{
		/* PhyPort 12 */
		.PortType = NORTH,
		.PortNum = 1,
	},
	{
		/* PhyPort 13 */
		.PortType = NORTH,
		.PortNum = 2,
	},
	{
		/* PhyPort 14 */
		.PortType = NORTH,
		.PortNum = 3,
	},
	{
		/* PhyPort 15 */
		.PortType = NORTH,
		.PortNum = 4,
	},
	{
		/* PhyPort 16 */
		.PortType = NORTH,
		.PortNum = 5,
	},
	{
		/* PhyPort 17 */
		.PortType = TRACE,
		.PortNum = 0,
	},
};

/*
 * Data structure to capture stream switch deterministic merge properties for
 * AIE2PS Tiles.
 */
static const XAie_StrmSwDetMerge Aie2PSAieTileStrmSwDetMerge = {
	.NumArbitors = 2U,
	.NumPositions = 4U,
	.ArbConfigOffset = 0x10,
	.ConfigBase = XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1,
	.EnableBase = XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_CTRL,
	.SlvId0.Lsb = XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_SLAVE_ID_0_LSB,
	.SlvId0.Mask = XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_SLAVE_ID_0_MASK,
	.SlvId1.Lsb = XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_SLAVE_ID_1_LSB,
	.SlvId1.Mask = XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_SLAVE_ID_1_MASK,
	.PktCount0.Lsb = XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_PACKET_COUNT_0_LSB,
	.PktCount0.Mask = XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_PACKET_COUNT_0_MASK,
	.PktCount1.Lsb = XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_PACKET_COUNT_1_LSB,
	.PktCount1.Mask = XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_PACKET_COUNT_1_MASK,
	.Enable.Lsb = XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_CTRL_ENABLE_LSB,
	.Enable.Mask = XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_CTRL_ENABLE_MASK,
};

/*
 * Data structure to capture stream switch deterministic merge properties for
 * AIE2PS Mem Tiles.
 */
static const XAie_StrmSwDetMerge Aie2PSMemTileStrmSwDetMerge = {
	.NumArbitors = 2U,
	.NumPositions = 4U,
	.ArbConfigOffset = 0x10,
	.ConfigBase = XAIE2PSGBL_MEM_TILE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1,
	.EnableBase = XAIE2PSGBL_MEM_TILE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_CTRL,
	.SlvId0.Lsb = XAIE2PSGBL_MEM_TILE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_SLAVE_ID_0_LSB,
	.SlvId0.Mask = XAIE2PSGBL_MEM_TILE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_SLAVE_ID_0_MASK,
	.SlvId1.Lsb = XAIE2PSGBL_MEM_TILE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_SLAVE_ID_1_LSB,
	.SlvId1.Mask = XAIE2PSGBL_MEM_TILE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_SLAVE_ID_1_MASK,
	.PktCount0.Lsb = XAIE2PSGBL_MEM_TILE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_PACKET_COUNT_0_LSB,
	.PktCount0.Mask = XAIE2PSGBL_MEM_TILE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_PACKET_COUNT_0_MASK,
	.PktCount1.Lsb = XAIE2PSGBL_MEM_TILE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_PACKET_COUNT_1_LSB,
	.PktCount1.Mask = XAIE2PSGBL_MEM_TILE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_PACKET_COUNT_1_MASK,
	.Enable.Lsb = XAIE2PSGBL_MEM_TILE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_CTRL_ENABLE_LSB,
	.Enable.Mask = XAIE2PSGBL_MEM_TILE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_CTRL_ENABLE_MASK,
};

/*
 * Data structure to capture stream switch deterministic merge properties for
 * AIE2PS SHIM PL Tiles.
 */
static const XAie_StrmSwDetMerge Aie2PShimTileStrmSwDetMerge = {
	.NumArbitors = 2U,
	.NumPositions = 4U,
	.ArbConfigOffset = 0x10,
	.ConfigBase = XAIE2PSGBL_PL_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1,
	.EnableBase = XAIE2PSGBL_PL_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_CTRL,
	.SlvId0.Lsb = XAIE2PSGBL_PL_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_SLAVE_ID_0_LSB,
	.SlvId0.Mask = XAIE2PSGBL_PL_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_SLAVE_ID_0_MASK,
	.SlvId1.Lsb = XAIE2PSGBL_PL_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_SLAVE_ID_1_LSB,
	.SlvId1.Mask = XAIE2PSGBL_PL_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_SLAVE_ID_1_MASK,
	.PktCount0.Lsb = XAIE2PSGBL_PL_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_PACKET_COUNT_0_LSB,
	.PktCount0.Mask = XAIE2PSGBL_PL_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_PACKET_COUNT_0_MASK,
	.PktCount1.Lsb = XAIE2PSGBL_PL_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_PACKET_COUNT_1_LSB,
	.PktCount1.Mask = XAIE2PSGBL_PL_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_PACKET_COUNT_1_MASK,
	.Enable.Lsb = XAIE2PSGBL_PL_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_CTRL_ENABLE_LSB,
	.Enable.Mask = XAIE2PSGBL_PL_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_CTRL_ENABLE_MASK,
};

/*
 * Data structure to capture all stream configs for XAIEGBL_TILE_TYPE_AIETILE
 */
static const  XAie_StrmMod Aie2PSTileStrmSw =
{
	.SlvConfigBaseAddr = XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_AIE_CORE0,
	.MstrConfigBaseAddr = XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0,
	.PortOffset = 0x4,
	.NumSlaveSlots = 4U,
	.SlotOffsetPerPort = 0x10,
	.SlotOffset = 0x4,
	.DetMergeFeature = XAIE_FEATURE_AVAILABLE,
	.MstrEn = {XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_MASTER_ENABLE_LSB, XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_MASTER_ENABLE_MASK},
	.MstrPktEn = {XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_PACKET_ENABLE_LSB, XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_PACKET_ENABLE_MASK},
	.DrpHdr = {XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_DROP_HEADER_LSB, XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_DROP_HEADER_MASK},
	.Config = {XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_CONFIGURATION_LSB, XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_CONFIGURATION_MASK},
	.SlvEn = {XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_AIE_CORE0_SLAVE_ENABLE_LSB, XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_AIE_CORE0_SLAVE_ENABLE_MASK},
	.SlvPktEn = {XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_AIE_CORE0_PACKET_ENABLE_LSB, XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_AIE_CORE0_PACKET_ENABLE_MASK},
	.SlotPktId = {XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ID_LSB, XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ID_MASK},
	.SlotMask = {XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_MASK_LSB, XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_MASK_MASK},
	.SlotEn = {XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ENABLE_LSB, XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ENABLE_MASK},
	.SlotMsel = {XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_MSEL_LSB, XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_MSEL_MASK},
	.SlotArbitor = {XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ARBIT_LSB, XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ARBIT_MASK},
	.MstrConfig = Aie2PSTileStrmMstr,
	.SlvConfig = Aie2PSTileStrmSlv,
	.SlvSlotConfig = Aie2PSTileStrmSlaveSlot,
	.MaxMasterPhyPortId = 22U,
	.MaxSlavePhyPortId = 24U,
	.MasterPortMap = Aie2PSTileStrmSwMasterPortMap,
	.SlavePortMap = Aie2PSTileStrmSwSlavePortMap,
	.DetMerge = &Aie2PSAieTileStrmSwDetMerge,
	.PortVerify = _XAieMl_AieTile_StrmSwCheckPortValidity,
};

/*
 * Data structure to capture all stream configs for XAIEGBL_TILE_TYPE_SHIMNOC/PL
 */
static const  XAie_StrmMod Aie2PSShimTileStrmSw =
{
	.SlvConfigBaseAddr = XAIE2PSGBL_PL_MODULE_STREAM_SWITCH_SLAVE_CONFIG_TILE_CTRL,
	.MstrConfigBaseAddr = XAIE2PSGBL_PL_MODULE_STREAM_SWITCH_MASTER_CONFIG_TILE_CTRL,
	.PortOffset = 0x4,
	.NumSlaveSlots = 4U,
	.SlotOffsetPerPort = 0x10,
	.SlotOffset = 0x4,
	.DetMergeFeature = XAIE_FEATURE_AVAILABLE,
	.MstrEn = {XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_MASTER_ENABLE_LSB, XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_MASTER_ENABLE_MASK},
	.MstrPktEn = {XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_PACKET_ENABLE_LSB, XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_PACKET_ENABLE_MASK},
	.DrpHdr = {XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_DROP_HEADER_LSB, XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_DROP_HEADER_MASK},
	.Config = {XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_CONFIGURATION_LSB, XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_CONFIGURATION_MASK},
	.SlvEn = {XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_AIE_CORE0_SLAVE_ENABLE_LSB, XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_AIE_CORE0_SLAVE_ENABLE_MASK},
	.SlvPktEn = {XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_AIE_CORE0_PACKET_ENABLE_LSB, XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_AIE_CORE0_PACKET_ENABLE_MASK},
	.SlotPktId = {XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ID_LSB, XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ID_MASK},
	.SlotMask = {XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_MASK_LSB, XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_MASK_MASK},
	.SlotEn = {XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ENABLE_LSB, XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ENABLE_MASK},
	.SlotMsel = {XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_MSEL_LSB, XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_MSEL_MASK},
	.SlotArbitor = {XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ARBIT_LSB, XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ARBIT_MASK},
	.MstrConfig = Aie2PShimStrmMstr,
	.SlvConfig = Aie2PShimStrmSlv,
	.SlvSlotConfig = Aie2PShimStrmSlaveSlot,
	.MaxMasterPhyPortId = 21U,
	.MaxSlavePhyPortId = 22U,
	.MasterPortMap = Aie2PShimStrmSwMasterPortMap,
	.SlavePortMap = Aie2PShimStrmSwSlavePortMap,
	.DetMerge = &Aie2PShimTileStrmSwDetMerge,
	.PortVerify = _XAieMl_ShimTile_StrmSwCheckPortValidity,
};

/*
 * Data structure to capture all stream configs for XAIEGBL_TILE_TYPE_MEMTILE
 */
static const  XAie_StrmMod Aie2PSMemTileStrmSw =
{
	.SlvConfigBaseAddr = XAIE2PSGBL_MEM_TILE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_DMA_0,
	.MstrConfigBaseAddr = XAIE2PSGBL_MEM_TILE_MODULE_STREAM_SWITCH_MASTER_CONFIG_DMA0,
	.PortOffset = 0x4,
	.NumSlaveSlots = 4U,
	.SlotOffsetPerPort = 0x10,
	.SlotOffset = 0x4,
	.DetMergeFeature = XAIE_FEATURE_AVAILABLE,
	.MstrEn = {XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_MASTER_ENABLE_LSB, XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_MASTER_ENABLE_MASK},
	.MstrPktEn = {XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_PACKET_ENABLE_LSB, XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_PACKET_ENABLE_MASK},
	.DrpHdr = {XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_DROP_HEADER_LSB, XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_DROP_HEADER_MASK},
	.Config = {XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_CONFIGURATION_LSB, XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_CONFIGURATION_MASK},
	.SlvEn = {XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_AIE_CORE0_SLAVE_ENABLE_LSB, XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_AIE_CORE0_SLAVE_ENABLE_MASK},
	.SlvPktEn = {XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_AIE_CORE0_PACKET_ENABLE_LSB, XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_AIE_CORE0_PACKET_ENABLE_MASK},
	.SlotPktId = {XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ID_LSB, XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ID_MASK},
	.SlotMask = {XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_MASK_LSB, XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_MASK_MASK},
	.SlotEn = {XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ENABLE_LSB, XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ENABLE_MASK},
	.SlotMsel = {XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_MSEL_LSB, XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_MSEL_MASK},
	.SlotArbitor = {XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ARBIT_LSB, XAIE2PSGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ARBIT_MASK},
	.MstrConfig = Aie2PSMemTileStrmMstr,
	.SlvConfig = Aie2PSMemTileStrmSlv,
	.SlvSlotConfig = Aie2PSMemTileStrmSlaveSlot,
	.MaxMasterPhyPortId = 16U,
	.MaxSlavePhyPortId = 17U,
	.MasterPortMap = Aie2PSMemTileStrmSwMasterPortMap,
	.SlavePortMap = Aie2PSMemTileStrmSwSlavePortMap,
	.DetMerge = &Aie2PSMemTileStrmSwDetMerge,
	.PortVerify = _XAieMl_MemTile_StrmSwCheckPortValidity,
};
#endif /* XAIE_FEATURE_SS_ENABLE */


#ifdef XAIE_FEATURE_DMA_ENABLE
static const  XAie_DmaBdEnProp Aie2PSMemTileDmaBdEnProp =
{
	.NxtBd.Idx = 1U,
	.NxtBd.Lsb = XAIE2PSGBL_MEM_TILE_MODULE_DMA_BD0_1_NEXT_BD_LSB,
	.NxtBd.Mask = XAIE2PSGBL_MEM_TILE_MODULE_DMA_BD0_1_NEXT_BD_MASK,
	.UseNxtBd.Idx = 1U,
	.UseNxtBd.Lsb = XAIE2PSGBL_MEM_TILE_MODULE_DMA_BD0_1_USE_NEXT_BD_LSB,
	.UseNxtBd.Mask = XAIE2PSGBL_MEM_TILE_MODULE_DMA_BD0_1_USE_NEXT_BD_MASK,
	.ValidBd.Idx = 7U,
	.ValidBd.Lsb = XAIE2PSGBL_MEM_TILE_MODULE_DMA_BD0_7_VALID_BD_LSB,
	.ValidBd.Mask = XAIE2PSGBL_MEM_TILE_MODULE_DMA_BD0_7_VALID_BD_MASK,
	.OutofOrderBdId.Idx = 0U,
	.OutofOrderBdId.Lsb = XAIE2PSGBL_MEM_TILE_MODULE_DMA_BD0_0_OUT_OF_ORDER_BD_ID_LSB,
	.OutofOrderBdId.Mask = XAIE2PSGBL_MEM_TILE_MODULE_DMA_BD0_0_OUT_OF_ORDER_BD_ID_MASK,
	.TlastSuppress.Idx = 2U,
	.TlastSuppress.Lsb = XAIE2PSGBL_MEM_TILE_MODULE_DMA_BD0_2_TLAST_SUPPRESS_LSB,
	.TlastSuppress.Mask = XAIE2PSGBL_MEM_TILE_MODULE_DMA_BD0_2_TLAST_SUPPRESS_MASK,
};

static const  XAie_DmaBdPkt Aie2PSMemTileDmaBdPktProp =
{
	.EnPkt.Idx = 0U,
	.EnPkt.Lsb = XAIE2PSGBL_MEM_TILE_MODULE_DMA_BD0_0_ENABLE_PACKET_LSB,
	.EnPkt.Mask = XAIE2PSGBL_MEM_TILE_MODULE_DMA_BD0_0_ENABLE_PACKET_MASK,
	.PktId.Idx = 0U,
	.PktId.Lsb = XAIE2PSGBL_MEM_TILE_MODULE_DMA_BD0_0_PACKET_ID_LSB,
	.PktId.Mask = XAIE2PSGBL_MEM_TILE_MODULE_DMA_BD0_0_PACKET_ID_MASK,
	.PktType.Idx = 0U,
	.PktType.Lsb = XAIE2PSGBL_MEM_TILE_MODULE_DMA_BD0_0_PACKET_TYPE_LSB,
	.PktType.Mask = XAIE2PSGBL_MEM_TILE_MODULE_DMA_BD0_0_PACKET_TYPE_MASK,
};

static const  XAie_DmaBdLock Aie2PSMemTileDmaLockProp =
{
	.AieMlDmaLock.LckRelVal.Idx = 7U,
	.AieMlDmaLock.LckRelVal.Lsb = XAIE2PSGBL_MEM_TILE_MODULE_DMA_BD0_7_LOCK_REL_VALUE_LSB,
	.AieMlDmaLock.LckRelVal.Mask = XAIE2PSGBL_MEM_TILE_MODULE_DMA_BD0_7_LOCK_REL_VALUE_MASK,
	.AieMlDmaLock.LckRelId.Idx = 7U,
	.AieMlDmaLock.LckRelId.Lsb = XAIE2PSGBL_MEM_TILE_MODULE_DMA_BD0_7_LOCK_REL_ID_LSB,
	.AieMlDmaLock.LckRelId.Mask = XAIE2PSGBL_MEM_TILE_MODULE_DMA_BD0_7_LOCK_REL_ID_MASK,
	.AieMlDmaLock.LckAcqEn.Idx = 7U,
	.AieMlDmaLock.LckAcqEn.Lsb = XAIE2PSGBL_MEM_TILE_MODULE_DMA_BD0_7_LOCK_ACQ_ENABLE_LSB,
	.AieMlDmaLock.LckAcqEn.Mask = XAIE2PSGBL_MEM_TILE_MODULE_DMA_BD0_7_LOCK_ACQ_ENABLE_MASK,
	.AieMlDmaLock.LckAcqVal.Idx = 7U,
	.AieMlDmaLock.LckAcqVal.Lsb = XAIE2PSGBL_MEM_TILE_MODULE_DMA_BD0_7_LOCK_ACQ_VALUE_LSB,
	.AieMlDmaLock.LckAcqVal.Mask = XAIE2PSGBL_MEM_TILE_MODULE_DMA_BD0_7_LOCK_ACQ_VALUE_MASK,
	.AieMlDmaLock.LckAcqId.Idx = 7U,
	.AieMlDmaLock.LckAcqId.Lsb = XAIE2PSGBL_MEM_TILE_MODULE_DMA_BD0_7_LOCK_ACQ_ID_LSB,
	.AieMlDmaLock.LckAcqId.Mask = XAIE2PSGBL_MEM_TILE_MODULE_DMA_BD0_7_LOCK_ACQ_ID_MASK,
};

static const  XAie_DmaBdBuffer Aie2PSMemTileBufferProp =
{
	.TileDmaBuff.BaseAddr.Idx = 1U,
	.TileDmaBuff.BaseAddr.Lsb = XAIE2PSGBL_MEM_TILE_MODULE_DMA_BD0_1_BASE_ADDRESS_LSB,
	.TileDmaBuff.BaseAddr.Mask = XAIE2PSGBL_MEM_TILE_MODULE_DMA_BD0_1_BASE_ADDRESS_MASK,
};

static const XAie_DmaBdDoubleBuffer Aie2PSMemTileDoubleBufferProp =
{
	.EnDoubleBuff = {0U},
	.BaseAddr_B = {0U},
	.FifoMode = {0U},
	.EnIntrleaved = {0U},
	.IntrleaveCnt = {0U},
	.BuffSelect = {0U},
};

static const  XAie_DmaBdMultiDimAddr Aie2PSMemTileMultiDimProp =
{
	.AieMlMultiDimAddr.DmaDimProp[1U].StepSize.Idx = 3U,
	.AieMlMultiDimAddr.DmaDimProp[1U].StepSize.Lsb = XAIE2PSGBL_MEM_TILE_MODULE_DMA_BD0_3_D1_STEPSIZE_LSB,
	.AieMlMultiDimAddr.DmaDimProp[1U].StepSize.Mask = XAIE2PSGBL_MEM_TILE_MODULE_DMA_BD0_3_D1_STEPSIZE_MASK,
	.AieMlMultiDimAddr.DmaDimProp[0U].StepSize.Idx = 2U,
	.AieMlMultiDimAddr.DmaDimProp[0U].StepSize.Lsb = XAIE2PSGBL_MEM_TILE_MODULE_DMA_BD0_2_D0_STEPSIZE_LSB,
	.AieMlMultiDimAddr.DmaDimProp[0U].StepSize.Mask = XAIE2PSGBL_MEM_TILE_MODULE_DMA_BD0_2_D0_STEPSIZE_MASK,
	.AieMlMultiDimAddr.DmaDimProp[1U].Wrap.Idx = 3U,
	.AieMlMultiDimAddr.DmaDimProp[1U].Wrap.Lsb = XAIE2PSGBL_MEM_TILE_MODULE_DMA_BD0_3_D1_WRAP_LSB,
	.AieMlMultiDimAddr.DmaDimProp[1U].Wrap.Mask = XAIE2PSGBL_MEM_TILE_MODULE_DMA_BD0_3_D1_WRAP_MASK,
	.AieMlMultiDimAddr.DmaDimProp[0U].Wrap.Idx = 2U,
	.AieMlMultiDimAddr.DmaDimProp[0U].Wrap.Lsb = XAIE2PSGBL_MEM_TILE_MODULE_DMA_BD0_2_D0_WRAP_LSB,
	.AieMlMultiDimAddr.DmaDimProp[0U].Wrap.Mask = XAIE2PSGBL_MEM_TILE_MODULE_DMA_BD0_2_D0_WRAP_MASK,
	.AieMlMultiDimAddr.DmaDimProp[2U].StepSize.Idx = 4U,
	.AieMlMultiDimAddr.DmaDimProp[2U].StepSize.Lsb = XAIE2PSGBL_MEM_TILE_MODULE_DMA_BD0_4_D2_STEPSIZE_LSB,
	.AieMlMultiDimAddr.DmaDimProp[2U].StepSize.Mask = XAIE2PSGBL_MEM_TILE_MODULE_DMA_BD0_4_D2_STEPSIZE_MASK,
	.AieMlMultiDimAddr.DmaDimProp[2U].Wrap.Idx = 4U,
	.AieMlMultiDimAddr.DmaDimProp[2U].Wrap.Lsb = XAIE2PSGBL_MEM_TILE_MODULE_DMA_BD0_4_D2_WRAP_LSB,
	.AieMlMultiDimAddr.DmaDimProp[2U].Wrap.Mask = XAIE2PSGBL_MEM_TILE_MODULE_DMA_BD0_4_D2_WRAP_MASK,
	.AieMlMultiDimAddr.IterCurr.Idx = 6U,
	.AieMlMultiDimAddr.IterCurr.Lsb = XAIE2PSGBL_MEM_TILE_MODULE_DMA_BD0_6_ITERATION_CURRENT_LSB,
	.AieMlMultiDimAddr.IterCurr.Mask = XAIE2PSGBL_MEM_TILE_MODULE_DMA_BD0_6_ITERATION_CURRENT_MASK,
	.AieMlMultiDimAddr.Iter.Wrap.Idx = 6U,
	.AieMlMultiDimAddr.Iter.Wrap.Lsb = XAIE2PSGBL_MEM_TILE_MODULE_DMA_BD0_6_ITERATION_WRAP_LSB,
	.AieMlMultiDimAddr.Iter.Wrap.Mask = XAIE2PSGBL_MEM_TILE_MODULE_DMA_BD0_6_ITERATION_WRAP_MASK,
	.AieMlMultiDimAddr.Iter.StepSize.Idx = 6U,
	.AieMlMultiDimAddr.Iter.StepSize.Lsb = XAIE2PSGBL_MEM_TILE_MODULE_DMA_BD0_6_ITERATION_STEPSIZE_LSB,
	.AieMlMultiDimAddr.Iter.StepSize.Mask = XAIE2PSGBL_MEM_TILE_MODULE_DMA_BD0_6_ITERATION_STEPSIZE_MASK,
	.AieMlMultiDimAddr.DmaDimProp[3U].StepSize.Idx = 5U,
	.AieMlMultiDimAddr.DmaDimProp[3U].StepSize.Lsb = XAIE2PSGBL_MEM_TILE_MODULE_DMA_BD0_5_D3_STEPSIZE_LSB,
	.AieMlMultiDimAddr.DmaDimProp[3U].StepSize.Mask = XAIE2PSGBL_MEM_TILE_MODULE_DMA_BD0_5_D3_STEPSIZE_MASK,
};

static const  XAie_DmaBdPad Aie2PSMemTilePadProp =
{
	.D0_PadBefore.Idx = 1U,
	.D0_PadBefore.Lsb = XAIE2PSGBL_MEM_TILE_MODULE_DMA_BD0_1_D0_PAD_BEFORE_LSB,
	.D0_PadBefore.Mask = XAIE2PSGBL_MEM_TILE_MODULE_DMA_BD0_1_D0_PAD_BEFORE_MASK,
	.D1_PadBefore.Idx = 3U,
	.D1_PadBefore.Lsb = XAIE2PSGBL_MEM_TILE_MODULE_DMA_BD0_3_D1_PAD_BEFORE_LSB,
	.D1_PadBefore.Mask = XAIE2PSGBL_MEM_TILE_MODULE_DMA_BD0_3_D1_PAD_BEFORE_MASK,
	.D2_PadBefore.Idx = 4U,
	.D2_PadBefore.Lsb = XAIE2PSGBL_MEM_TILE_MODULE_DMA_BD0_4_D2_PAD_BEFORE_LSB,
	.D2_PadBefore.Mask = XAIE2PSGBL_MEM_TILE_MODULE_DMA_BD0_4_D2_PAD_BEFORE_MASK,
	.D0_PadAfter.Idx = 5U,
	.D0_PadAfter.Lsb = XAIE2PSGBL_MEM_TILE_MODULE_DMA_BD0_5_D0_PAD_AFTER_LSB,
	.D0_PadAfter.Mask = XAIE2PSGBL_MEM_TILE_MODULE_DMA_BD0_5_D0_PAD_AFTER_MASK,
	.D1_PadAfter.Idx = 5U,
	.D1_PadAfter.Lsb = XAIE2PSGBL_MEM_TILE_MODULE_DMA_BD0_5_D1_PAD_AFTER_LSB,
	.D1_PadAfter.Mask = XAIE2PSGBL_MEM_TILE_MODULE_DMA_BD0_5_D1_PAD_AFTER_MASK,
	.D2_PadAfter.Idx = 5U,
	.D2_PadAfter.Lsb = XAIE2PSGBL_MEM_TILE_MODULE_DMA_BD0_5_D2_PAD_AFTER_LSB,
	.D2_PadAfter.Mask = XAIE2PSGBL_MEM_TILE_MODULE_DMA_BD0_5_D2_PAD_AFTER_MASK,
};

static const  XAie_DmaBdCompression Aie2PSMemTileCompressionProp =
{
	.EnCompression.Idx = 4U,
	.EnCompression.Lsb = XAIE2PSGBL_MEM_TILE_MODULE_DMA_BD0_4_ENABLE_COMPRESSION_LSB,
	.EnCompression.Mask = XAIE2PSGBL_MEM_TILE_MODULE_DMA_BD0_4_ENABLE_COMPRESSION_MASK,
};

/* Data structure to capture register offsets and masks for Mem Tile Dma */
static const  XAie_DmaBdProp Aie2PSMemTileDmaProp =
{
	.AddrAlignMask = 0x3,
	.AddrAlignShift = 0x2,
	.AddrMax = 0x180000,
	.LenActualOffset = 0U,
	.StepSizeMax = (1U << 17) - 1U,
	.WrapMax = (1U << 10U) - 1U,
	.IterStepSizeMax = (1U << 17) - 1U,
	.IterWrapMax = (1U << 6U) - 1U,
	.IterCurrMax = (1U << 6) - 1U,
	.BufferLen.Idx = 0U,
	.BufferLen.Lsb = XAIE2PSGBL_MEM_TILE_MODULE_DMA_BD0_0_BUFFER_LENGTH_LSB,
	.BufferLen.Mask = XAIE2PSGBL_MEM_TILE_MODULE_DMA_BD0_0_BUFFER_LENGTH_MASK,
	.Buffer = &Aie2PSMemTileBufferProp,
	.DoubleBuffer = &Aie2PSMemTileDoubleBufferProp,
	.Lock = &Aie2PSMemTileDmaLockProp,
	.Pkt = &Aie2PSMemTileDmaBdPktProp,
	.BdEn = &Aie2PSMemTileDmaBdEnProp,
	.AddrMode = &Aie2PSMemTileMultiDimProp,
	.Pad = &Aie2PSMemTilePadProp,
	.Compression = &Aie2PSMemTileCompressionProp,
	.SysProp = NULL
};

static const XAie_DmaChStatus Aie2PSMemTileDmaChStatus =
{
	/* This database is common for mm2s and s2mm channels */
	.AieMlDmaChStatus.Status.Lsb = XAIE2PSGBL_MEM_TILE_MODULE_DMA_S2MM_STATUS_0_STATUS_LSB,
	.AieMlDmaChStatus.Status.Mask = XAIE2PSGBL_MEM_TILE_MODULE_DMA_S2MM_STATUS_0_STATUS_MASK,
	.AieMlDmaChStatus.TaskQSize.Lsb = XAIE2PSGBL_MEM_TILE_MODULE_DMA_S2MM_STATUS_0_TASK_QUEUE_SIZE_LSB,
	.AieMlDmaChStatus.TaskQSize.Mask = XAIE2PSGBL_MEM_TILE_MODULE_DMA_S2MM_STATUS_0_TASK_QUEUE_SIZE_MASK,
	.AieMlDmaChStatus.StalledLockAcq.Lsb = XAIE2PSGBL_MEM_TILE_MODULE_DMA_S2MM_STATUS_0_STALLED_LOCK_ACQ_LSB,
	.AieMlDmaChStatus.StalledLockAcq.Mask = XAIE2PSGBL_MEM_TILE_MODULE_DMA_S2MM_STATUS_0_STALLED_LOCK_ACQ_MASK,
	.AieMlDmaChStatus.StalledLockRel.Lsb = XAIE2PSGBL_MEM_TILE_MODULE_DMA_S2MM_STATUS_0_STALLED_LOCK_REL_LSB,
	.AieMlDmaChStatus.StalledLockRel.Mask = XAIE2PSGBL_MEM_TILE_MODULE_DMA_S2MM_STATUS_0_STALLED_LOCK_REL_MASK,
	.AieMlDmaChStatus.StalledStreamStarve.Lsb = XAIE2PSGBL_MEM_TILE_MODULE_DMA_S2MM_STATUS_0_STALLED_STREAM_STARVATION_LSB,
	.AieMlDmaChStatus.StalledStreamStarve.Mask = XAIE2PSGBL_MEM_TILE_MODULE_DMA_S2MM_STATUS_0_STALLED_STREAM_STARVATION_MASK,
	.AieMlDmaChStatus.StalledTCT.Lsb = XAIE2PSGBL_MEM_TILE_MODULE_DMA_S2MM_STATUS_0_STALLED_TCT_OR_COUNT_FIFO_FULL_LSB,
	.AieMlDmaChStatus.StalledTCT.Mask = XAIE2PSGBL_MEM_TILE_MODULE_DMA_S2MM_STATUS_0_STALLED_TCT_OR_COUNT_FIFO_FULL_MASK,
};

static const  XAie_DmaChProp Aie2PSMemTileDmaChProp =
{
	.HasFoTMode = XAIE_FEATURE_AVAILABLE,
	.HasControllerId = XAIE_FEATURE_AVAILABLE,
	.HasEnCompression = XAIE_FEATURE_AVAILABLE,
	.HasEnOutOfOrder = XAIE_FEATURE_AVAILABLE,
	.MaxFoTMode = DMA_FoT_COUNTS_FROM_MM_REG,
	.MaxRepeatCount = 256U,
	.ControllerId.Idx = 0,
	.ControllerId.Lsb = XAIE2PSGBL_MEM_TILE_MODULE_DMA_S2MM_0_CTRL_CONTROLLER_ID_LSB,
	.ControllerId.Mask =XAIE2PSGBL_MEM_TILE_MODULE_DMA_S2MM_0_CTRL_CONTROLLER_ID_MASK ,
	.EnCompression.Idx = 0,
	.EnCompression.Lsb = XAIE2PSGBL_MEM_TILE_MODULE_DMA_S2MM_0_CTRL_DECOMPRESSION_ENABLE_LSB,
	.EnCompression.Mask = XAIE2PSGBL_MEM_TILE_MODULE_DMA_S2MM_0_CTRL_DECOMPRESSION_ENABLE_MASK,
	.EnOutofOrder.Idx = 0,
	.EnOutofOrder.Lsb = XAIE2PSGBL_MEM_TILE_MODULE_DMA_S2MM_0_CTRL_ENABLE_OUT_OF_ORDER_LSB,
	.EnOutofOrder.Mask = XAIE2PSGBL_MEM_TILE_MODULE_DMA_S2MM_0_CTRL_ENABLE_OUT_OF_ORDER_MASK,
	.FoTMode.Idx = 0,
	.FoTMode.Lsb = XAIE2PSGBL_MEM_TILE_MODULE_DMA_S2MM_0_CTRL_FOT_MODE_LSB,
	.FoTMode.Mask = XAIE2PSGBL_MEM_TILE_MODULE_DMA_S2MM_0_CTRL_FOT_MODE_MASK ,
	.Reset.Idx = 0,
	.Reset.Lsb = XAIE2PSGBL_MEM_TILE_MODULE_DMA_S2MM_0_CTRL_RESET_LSB,
	.Reset.Mask = XAIE2PSGBL_MEM_TILE_MODULE_DMA_S2MM_0_CTRL_RESET_MASK,
	.EnToken.Idx = 1,
	.EnToken.Lsb = XAIE2PSGBL_MEM_TILE_MODULE_DMA_S2MM_0_START_QUEUE_ENABLE_TOKEN_ISSUE_LSB,
	.EnToken.Mask = XAIE2PSGBL_MEM_TILE_MODULE_DMA_S2MM_0_START_QUEUE_ENABLE_TOKEN_ISSUE_MASK,
	.RptCount.Idx = 1,
	.RptCount.Lsb = XAIE2PSGBL_MEM_TILE_MODULE_DMA_S2MM_0_START_QUEUE_REPEAT_COUNT_LSB,
	.RptCount.Mask = XAIE2PSGBL_MEM_TILE_MODULE_DMA_S2MM_0_START_QUEUE_REPEAT_COUNT_MASK,
	.StartBd.Idx = 1,
	.StartBd.Lsb = XAIE2PSGBL_MEM_TILE_MODULE_DMA_S2MM_0_START_QUEUE_START_BD_ID_LSB,
	.StartBd.Mask = XAIE2PSGBL_MEM_TILE_MODULE_DMA_S2MM_0_START_QUEUE_START_BD_ID_MASK,
	.PauseStream = {0U},
	.PauseMem = {0U},
	.Enable = {0U},
	.StartQSizeMax = 4U,
	.DmaChStatus = &Aie2PSMemTileDmaChStatus,
};

/* Mem Tile Dma Module */
static const  XAie_DmaMod Aie2PSMemTileDmaMod =
{
	.BaseAddr = XAIE2PSGBL_MEM_TILE_MODULE_DMA_BD0_0,
	.IdxOffset = 0x20,  /* This is the offset between each BD */
	.NumBds = 48,	   /* Number of BDs for AIE2PS Tile DMA */
	.NumLocks = 192U,
	.NumAddrDim = 4U,
	.DoubleBuffering = XAIE_FEATURE_UNAVAILABLE,
	.Compression = XAIE_FEATURE_AVAILABLE,
	.Padding = XAIE_FEATURE_AVAILABLE,
	.OutofOrderBdId = XAIE_FEATURE_AVAILABLE,
	.InterleaveMode = XAIE_FEATURE_UNAVAILABLE,
	.FifoMode = XAIE_FEATURE_UNAVAILABLE,
	.EnTokenIssue = XAIE_FEATURE_AVAILABLE,
	.RepeatCount = XAIE_FEATURE_AVAILABLE,
	.TlastSuppress = XAIE_FEATURE_AVAILABLE,
	.StartQueueBase = XAIE2PSGBL_MEM_TILE_MODULE_DMA_S2MM_0_START_QUEUE,
	.ChCtrlBase = XAIE2PSGBL_MEM_TILE_MODULE_DMA_S2MM_0_CTRL,
	.NumChannels = 6,  /* number of s2mm/mm2s channels */
	.ChIdxOffset = 0x8,  /* This is the offset between each channel */
	.ChStatusBase = XAIE2PSGBL_MEM_TILE_MODULE_DMA_S2MM_STATUS_0,
	.ChStatusOffset = 0x20,
	.PadValueBase = XAIE2PSGBL_MEM_TILE_MODULE_DMA_MM2S_0_CONSTANT_PAD_VALUE,
	.BdProp = &Aie2PSMemTileDmaProp,
	.ChProp = &Aie2PSMemTileDmaChProp,
	.DmaBdInit = &_XAieMl_MemTileDmaInit,
	.SetLock = &_XAieMl_DmaSetLock,
	.SetIntrleave = NULL,
	.SetMultiDim = &_XAieMl_DmaSetMultiDim,
	.SetBdIter = &_XAieMl_DmaSetBdIteration,
	.WriteBd = &_XAieMl_MemTileDmaWriteBd,
	.PendingBd = &_XAieMl_DmaGetPendingBdCount,
	.WaitforDone = &_XAieMl_DmaWaitForDone,
	.BdChValidity = &_XAieMl_MemTileDmaCheckBdChValidity,
	.UpdateBdLen = &_XAieMl_DmaUpdateBdLen,
	.UpdateBdAddr = &_XAieMl_DmaUpdateBdAddr,
};

static const  XAie_DmaBdEnProp Aie2PSTileDmaBdEnProp =
{
	.NxtBd.Idx = 5U,
	.NxtBd.Lsb = XAIE2PSGBL_MEMORY_MODULE_DMA_BD0_5_NEXT_BD_LSB,
	.NxtBd.Mask = XAIE2PSGBL_MEMORY_MODULE_DMA_BD0_5_NEXT_BD_MASK,
	.UseNxtBd.Idx = 5U,
	.UseNxtBd.Lsb = XAIE2PSGBL_MEMORY_MODULE_DMA_BD0_5_USE_NEXT_BD_LSB,
	.UseNxtBd.Mask = XAIE2PSGBL_MEMORY_MODULE_DMA_BD0_5_USE_NEXT_BD_MASK,
	.ValidBd.Idx = 5U,
	.ValidBd.Lsb = XAIE2PSGBL_MEMORY_MODULE_DMA_BD0_5_VALID_BD_LSB,
	.ValidBd.Mask = XAIE2PSGBL_MEMORY_MODULE_DMA_BD0_5_VALID_BD_MASK,
	.OutofOrderBdId.Idx = 1U,
	.OutofOrderBdId.Lsb = XAIE2PSGBL_MEMORY_MODULE_DMA_BD0_1_OUT_OF_ORDER_BD_ID_LSB,
	.OutofOrderBdId.Mask = XAIE2PSGBL_MEMORY_MODULE_DMA_BD0_1_OUT_OF_ORDER_BD_ID_MASK,
	.TlastSuppress.Idx = 5U,
	.TlastSuppress.Lsb = XAIE2PSGBL_MEMORY_MODULE_DMA_BD0_5_TLAST_SUPPRESS_LSB,
	.TlastSuppress.Mask = XAIE2PSGBL_MEMORY_MODULE_DMA_BD0_5_TLAST_SUPPRESS_MASK,
};

static const  XAie_DmaBdPkt Aie2PSTileDmaBdPktProp =
{
	.EnPkt.Idx = 1U,
	.EnPkt.Lsb = XAIE2PSGBL_MEMORY_MODULE_DMA_BD0_1_ENABLE_PACKET_LSB,
	.EnPkt.Mask = XAIE2PSGBL_MEMORY_MODULE_DMA_BD0_1_ENABLE_PACKET_MASK,
	.PktId.Idx = 1U,
	.PktId.Lsb = XAIE2PSGBL_MEMORY_MODULE_DMA_BD0_1_PACKET_ID_LSB,
	.PktId.Mask = XAIE2PSGBL_MEMORY_MODULE_DMA_BD0_1_PACKET_ID_MASK,
	.PktType.Idx = 1U,
	.PktType.Lsb = XAIE2PSGBL_MEMORY_MODULE_DMA_BD0_1_PACKET_TYPE_LSB,
	.PktType.Mask = XAIE2PSGBL_MEMORY_MODULE_DMA_BD0_1_PACKET_TYPE_MASK,
};

static const  XAie_DmaBdLock Aie2PSTileDmaLockProp =
{
	.AieMlDmaLock.LckRelVal.Idx = 5U,
	.AieMlDmaLock.LckRelVal.Lsb = XAIE2PSGBL_MEMORY_MODULE_DMA_BD0_5_LOCK_REL_VALUE_LSB,
	.AieMlDmaLock.LckRelVal.Mask = XAIE2PSGBL_MEMORY_MODULE_DMA_BD0_5_LOCK_REL_VALUE_MASK,
	.AieMlDmaLock.LckRelId.Idx = 5U,
	.AieMlDmaLock.LckRelId.Lsb = XAIE2PSGBL_MEMORY_MODULE_DMA_BD0_5_LOCK_REL_ID_LSB,
	.AieMlDmaLock.LckRelId.Mask = XAIE2PSGBL_MEMORY_MODULE_DMA_BD0_5_LOCK_REL_ID_MASK,
	.AieMlDmaLock.LckAcqEn.Idx = 5U,
	.AieMlDmaLock.LckAcqEn.Lsb = XAIE2PSGBL_MEMORY_MODULE_DMA_BD0_5_LOCK_ACQ_ENABLE_LSB,
	.AieMlDmaLock.LckAcqEn.Mask = XAIE2PSGBL_MEMORY_MODULE_DMA_BD0_5_LOCK_ACQ_ENABLE_MASK,
	.AieMlDmaLock.LckAcqVal.Idx = 5U,
	.AieMlDmaLock.LckAcqVal.Lsb = XAIE2PSGBL_MEMORY_MODULE_DMA_BD0_5_LOCK_ACQ_VALUE_LSB,
	.AieMlDmaLock.LckAcqVal.Mask = XAIE2PSGBL_MEMORY_MODULE_DMA_BD0_5_LOCK_ACQ_VALUE_MASK,
	.AieMlDmaLock.LckAcqId.Idx = 5U,
	.AieMlDmaLock.LckAcqId.Lsb = XAIE2PSGBL_MEMORY_MODULE_DMA_BD0_5_LOCK_ACQ_ID_LSB,
	.AieMlDmaLock.LckAcqId.Mask = XAIE2PSGBL_MEMORY_MODULE_DMA_BD0_5_LOCK_ACQ_ID_MASK,
};

static const  XAie_DmaBdBuffer Aie2PSTileDmaBufferProp =
{
	.TileDmaBuff.BaseAddr.Idx = 0U,
	.TileDmaBuff.BaseAddr.Lsb = XAIE2PSGBL_MEMORY_MODULE_DMA_BD0_0_BASE_ADDRESS_LSB,
	.TileDmaBuff.BaseAddr.Mask = XAIE2PSGBL_MEMORY_MODULE_DMA_BD0_0_BASE_ADDRESS_MASK,
};

static const XAie_DmaBdDoubleBuffer Aie2PSTileDmaDoubleBufferProp =
{
	.EnDoubleBuff = {0U},
	.BaseAddr_B = {0U},
	.FifoMode = {0U},
	.EnIntrleaved = {0U},
	.IntrleaveCnt = {0U},
	.BuffSelect = {0U},
};

static const  XAie_DmaBdMultiDimAddr Aie2PSTileDmaMultiDimProp =
{
	.AieMlMultiDimAddr.DmaDimProp[0U].StepSize.Idx = 2U,
	.AieMlMultiDimAddr.DmaDimProp[0U].StepSize.Lsb = XAIE2PSGBL_MEMORY_MODULE_DMA_BD0_2_D0_STEPSIZE_LSB,
	.AieMlMultiDimAddr.DmaDimProp[0U].StepSize.Mask = XAIE2PSGBL_MEMORY_MODULE_DMA_BD0_2_D0_STEPSIZE_MASK,
	.AieMlMultiDimAddr.DmaDimProp[0U].Wrap.Idx = 3U,
	.AieMlMultiDimAddr.DmaDimProp[0U].Wrap.Lsb = XAIE2PSGBL_MEMORY_MODULE_DMA_BD0_3_D0_WRAP_LSB,
	.AieMlMultiDimAddr.DmaDimProp[0U].Wrap.Mask = XAIE2PSGBL_MEMORY_MODULE_DMA_BD0_3_D0_WRAP_MASK,
	.AieMlMultiDimAddr.DmaDimProp[1U].StepSize.Idx = 2U,
	.AieMlMultiDimAddr.DmaDimProp[1U].StepSize.Lsb = XAIE2PSGBL_MEMORY_MODULE_DMA_BD0_2_D1_STEPSIZE_LSB,
	.AieMlMultiDimAddr.DmaDimProp[1U].StepSize.Mask = XAIE2PSGBL_MEMORY_MODULE_DMA_BD0_2_D1_STEPSIZE_MASK,
	.AieMlMultiDimAddr.DmaDimProp[1U].Wrap.Idx = 3U,
	.AieMlMultiDimAddr.DmaDimProp[1U].Wrap.Lsb = XAIE2PSGBL_MEMORY_MODULE_DMA_BD0_3_D1_WRAP_LSB,
	.AieMlMultiDimAddr.DmaDimProp[1U].Wrap.Mask = XAIE2PSGBL_MEMORY_MODULE_DMA_BD0_3_D1_WRAP_MASK,
	.AieMlMultiDimAddr.DmaDimProp[2U].StepSize.Idx = 3U,
	.AieMlMultiDimAddr.DmaDimProp[2U].StepSize.Lsb = XAIE2PSGBL_MEMORY_MODULE_DMA_BD0_3_D2_STEPSIZE_LSB,
	.AieMlMultiDimAddr.DmaDimProp[2U].StepSize.Mask = XAIE2PSGBL_MEMORY_MODULE_DMA_BD0_3_D2_STEPSIZE_MASK,
	.AieMlMultiDimAddr.IterCurr.Idx = 4U,
	.AieMlMultiDimAddr.IterCurr.Lsb = XAIE2PSGBL_MEMORY_MODULE_DMA_BD0_4_ITERATION_CURRENT_LSB,
	.AieMlMultiDimAddr.IterCurr.Mask = XAIE2PSGBL_MEMORY_MODULE_DMA_BD0_4_ITERATION_CURRENT_MASK,
	.AieMlMultiDimAddr.Iter.Wrap.Idx = 4U,
	.AieMlMultiDimAddr.Iter.Wrap.Lsb = XAIE2PSGBL_MEMORY_MODULE_DMA_BD0_4_ITERATION_WRAP_LSB,
	.AieMlMultiDimAddr.Iter.Wrap.Mask = XAIE2PSGBL_MEMORY_MODULE_DMA_BD0_4_ITERATION_WRAP_MASK,
	.AieMlMultiDimAddr.Iter.StepSize.Idx = 4U,
	.AieMlMultiDimAddr.Iter.StepSize.Lsb = XAIE2PSGBL_MEMORY_MODULE_DMA_BD0_4_ITERATION_STEPSIZE_LSB,
	.AieMlMultiDimAddr.Iter.StepSize.Mask = XAIE2PSGBL_MEMORY_MODULE_DMA_BD0_4_ITERATION_STEPSIZE_MASK,
	.AieMlMultiDimAddr.DmaDimProp[2U].Wrap = {0U},
	.AieMlMultiDimAddr.DmaDimProp[3U].Wrap = {0U},
	.AieMlMultiDimAddr.DmaDimProp[3U].StepSize = {0U}
};

static const  XAie_DmaBdCompression Aie2PSTileDmaCompressionProp =
{
	.EnCompression.Idx = 1U,
	.EnCompression.Lsb = XAIE2PSGBL_MEMORY_MODULE_DMA_BD0_1_ENABLE_COMPRESSION_LSB,
	.EnCompression.Mask = XAIE2PSGBL_MEMORY_MODULE_DMA_BD0_1_ENABLE_COMPRESSION_MASK,
};

/* Data structure to capture register offsets and masks for Tile Dma */
static const  XAie_DmaBdProp Aie2PSTileDmaProp =
{
	.AddrAlignMask = 0x3,
	.AddrAlignShift = 0x2,
	.AddrMax = 0x20000,
	.LenActualOffset = 0U,
	.StepSizeMax = (1U << 13) - 1U,
	.WrapMax = (1U << 8U) - 1U,
	.IterStepSizeMax = (1U << 13) - 1U,
	.IterWrapMax = (1U << 6U) - 1U,
	.IterCurrMax = (1U << 6) - 1U,
	.BufferLen.Idx = 0U,
	.BufferLen.Lsb = XAIE2PSGBL_MEMORY_MODULE_DMA_BD0_0_BUFFER_LENGTH_LSB,
	.BufferLen.Mask = XAIE2PSGBL_MEMORY_MODULE_DMA_BD0_0_BUFFER_LENGTH_MASK,
	.Buffer = &Aie2PSTileDmaBufferProp,
	.DoubleBuffer = &Aie2PSTileDmaDoubleBufferProp,
	.Lock = &Aie2PSTileDmaLockProp,
	.Pkt = &Aie2PSTileDmaBdPktProp,
	.BdEn = &Aie2PSTileDmaBdEnProp,
	.AddrMode = &Aie2PSTileDmaMultiDimProp,
	.Pad = NULL,
	.Compression = &Aie2PSTileDmaCompressionProp,
	.SysProp = NULL
};

static const XAie_DmaChStatus Aie2PSTileDmaChStatus =
{
	/* This database is common for mm2s and s2mm channels */
	.AieMlDmaChStatus.Status.Lsb = XAIE2PSGBL_MEMORY_MODULE_DMA_S2MM_STATUS_0_STATUS_LSB,
	.AieMlDmaChStatus.Status.Mask = XAIE2PSGBL_MEMORY_MODULE_DMA_S2MM_STATUS_0_STATUS_MASK,
	.AieMlDmaChStatus.TaskQSize.Lsb = XAIE2PSGBL_MEMORY_MODULE_DMA_S2MM_STATUS_0_TASK_QUEUE_SIZE_LSB,
	.AieMlDmaChStatus.TaskQSize.Mask = XAIE2PSGBL_MEMORY_MODULE_DMA_S2MM_STATUS_0_TASK_QUEUE_SIZE_MASK,
	.AieMlDmaChStatus.StalledLockAcq.Lsb = XAIE2PSGBL_MEMORY_MODULE_DMA_S2MM_STATUS_0_STALLED_LOCK_ACQ_LSB,
	.AieMlDmaChStatus.StalledLockAcq.Mask = XAIE2PSGBL_MEMORY_MODULE_DMA_S2MM_STATUS_0_STALLED_LOCK_ACQ_MASK,
	.AieMlDmaChStatus.StalledLockRel.Lsb = XAIE2PSGBL_MEMORY_MODULE_DMA_S2MM_STATUS_0_STALLED_LOCK_REL_LSB,
	.AieMlDmaChStatus.StalledLockRel.Mask = XAIE2PSGBL_MEMORY_MODULE_DMA_S2MM_STATUS_0_STALLED_LOCK_REL_MASK,
	.AieMlDmaChStatus.StalledStreamStarve.Lsb = XAIE2PSGBL_MEMORY_MODULE_DMA_S2MM_STATUS_0_STALLED_STREAM_STARVATION_LSB,
	.AieMlDmaChStatus.StalledStreamStarve.Mask = XAIE2PSGBL_MEMORY_MODULE_DMA_S2MM_STATUS_0_STALLED_STREAM_STARVATION_MASK,
	.AieMlDmaChStatus.StalledTCT.Lsb = XAIE2PSGBL_MEMORY_MODULE_DMA_S2MM_STATUS_0_STALLED_TCT_OR_COUNT_FIFO_FULL_LSB,
	.AieMlDmaChStatus.StalledTCT.Mask = XAIE2PSGBL_MEMORY_MODULE_DMA_S2MM_STATUS_0_STALLED_TCT_OR_COUNT_FIFO_FULL_MASK,
};

/* Data structure to capture register offsets and masks for Mem Tile and
 * Tile Dma Channels
 */
static const  XAie_DmaChProp Aie2PSDmaChProp =
{
	.HasFoTMode = XAIE_FEATURE_AVAILABLE,
	.HasControllerId = XAIE_FEATURE_AVAILABLE,
	.HasEnCompression = XAIE_FEATURE_AVAILABLE,
	.HasEnOutOfOrder = XAIE_FEATURE_AVAILABLE,
	.MaxFoTMode = DMA_FoT_COUNTS_FROM_MM_REG,
	.MaxRepeatCount = 256U,
	.ControllerId.Idx = 0,
	.ControllerId.Lsb = XAIE2PSGBL_MEMORY_MODULE_DMA_S2MM_0_CTRL_CONTROLLER_ID_LSB,
	.ControllerId.Mask =XAIE2PSGBL_MEMORY_MODULE_DMA_S2MM_0_CTRL_CONTROLLER_ID_MASK ,
	.EnCompression.Idx = 0,
	.EnCompression.Lsb = XAIE2PSGBL_MEMORY_MODULE_DMA_S2MM_0_CTRL_DECOMPRESSION_ENABLE_LSB,
	.EnCompression.Mask = XAIE2PSGBL_MEMORY_MODULE_DMA_S2MM_0_CTRL_DECOMPRESSION_ENABLE_MASK,
	.EnOutofOrder.Idx = 0,
	.EnOutofOrder.Lsb = XAIE2PSGBL_MEMORY_MODULE_DMA_S2MM_0_CTRL_ENABLE_OUT_OF_ORDER_LSB,
	.EnOutofOrder.Mask = XAIE2PSGBL_MEMORY_MODULE_DMA_S2MM_0_CTRL_ENABLE_OUT_OF_ORDER_MASK,
	.FoTMode.Idx = 0,
	.FoTMode.Lsb = XAIE2PSGBL_MEMORY_MODULE_DMA_S2MM_0_CTRL_FOT_MODE_LSB,
	.FoTMode.Mask = XAIE2PSGBL_MEMORY_MODULE_DMA_S2MM_0_CTRL_FOT_MODE_MASK ,
	.Reset.Idx = 0,
	.Reset.Lsb = XAIE2PSGBL_MEMORY_MODULE_DMA_S2MM_0_CTRL_RESET_LSB,
	.Reset.Mask = XAIE2PSGBL_MEMORY_MODULE_DMA_S2MM_0_CTRL_RESET_MASK,
	.EnToken.Idx = 1,
	.EnToken.Lsb = XAIE2PSGBL_MEMORY_MODULE_DMA_S2MM_0_START_QUEUE_ENABLE_TOKEN_ISSUE_LSB,
	.EnToken.Mask = XAIE2PSGBL_MEMORY_MODULE_DMA_S2MM_0_START_QUEUE_ENABLE_TOKEN_ISSUE_MASK,
	.RptCount.Idx = 1,
	.RptCount.Lsb = XAIE2PSGBL_MEMORY_MODULE_DMA_S2MM_0_START_QUEUE_REPEAT_COUNT_LSB,
	.RptCount.Mask = XAIE2PSGBL_MEMORY_MODULE_DMA_S2MM_0_START_QUEUE_REPEAT_COUNT_MASK,
	.StartBd.Idx = 1,
	.StartBd.Lsb = XAIE2PSGBL_MEMORY_MODULE_DMA_S2MM_0_START_QUEUE_START_BD_ID_LSB,
	.StartBd.Mask = XAIE2PSGBL_MEMORY_MODULE_DMA_S2MM_0_START_QUEUE_START_BD_ID_MASK,
	.PauseStream = {0U},
	.PauseMem = {0U},
	.Enable = {0U},
	.StartQSizeMax = 4U,
	.DmaChStatus = &Aie2PSTileDmaChStatus,
};

/* Tile Dma Module */
static const  XAie_DmaMod Aie2PSTileDmaMod =
{
	.BaseAddr = XAIE2PSGBL_MEMORY_MODULE_DMA_BD0_0,
	.IdxOffset = 0x20,  	/* This is the offset between each BD */
	.NumBds = 16U,	   	/* Number of BDs for AIE2PS Tile DMA */
	.NumLocks = 16U,
	.NumAddrDim = 3U,
	.DoubleBuffering = XAIE_FEATURE_UNAVAILABLE,
	.Compression = XAIE_FEATURE_AVAILABLE,
	.Padding = XAIE_FEATURE_UNAVAILABLE,
	.OutofOrderBdId = XAIE_FEATURE_AVAILABLE,
	.InterleaveMode = XAIE_FEATURE_UNAVAILABLE,
	.FifoMode = XAIE_FEATURE_UNAVAILABLE,
	.EnTokenIssue = XAIE_FEATURE_AVAILABLE,
	.RepeatCount = XAIE_FEATURE_AVAILABLE,
	.TlastSuppress = XAIE_FEATURE_AVAILABLE,
	.StartQueueBase = XAIE2PSGBL_MEMORY_MODULE_DMA_S2MM_0_START_QUEUE,
	.ChCtrlBase = XAIE2PSGBL_MEMORY_MODULE_DMA_S2MM_0_CTRL,
	.NumChannels = 2U,  /* Number of s2mm/mm2s channels */
	.ChIdxOffset = 0x8,  /* This is the offset between each channel */
	.ChStatusBase = XAIE2PSGBL_MEMORY_MODULE_DMA_S2MM_STATUS_0,
	.ChStatusOffset = 0x10,
	.PadValueBase = 0x0,
	.BdProp = &Aie2PSTileDmaProp,
	.ChProp = &Aie2PSDmaChProp,
	.DmaBdInit = &_XAieMl_TileDmaInit,
	.SetLock = &_XAieMl_DmaSetLock,
	.SetIntrleave = NULL,
	.SetMultiDim = &_XAieMl_DmaSetMultiDim,
	.SetBdIter = &_XAieMl_DmaSetBdIteration,
	.WriteBd = &_XAieMl_TileDmaWriteBd,
	.PendingBd = &_XAieMl_DmaGetPendingBdCount,
	.WaitforDone = &_XAieMl_DmaWaitForDone,
	.BdChValidity = &_XAieMl_DmaCheckBdChValidity,
	.UpdateBdLen = &_XAieMl_DmaUpdateBdLen,
	.UpdateBdAddr = &_XAieMl_DmaUpdateBdAddr,
};

static const  XAie_DmaBdEnProp Aie2PSShimDmaBdEnProp =
{
	.NxtBd.Idx = 7U,
	.NxtBd.Lsb = XAIE2PSGBL_NOC_MODULE_DMA_BD0_7_NEXT_BD_LSB,
	.NxtBd.Mask = XAIE2PSGBL_NOC_MODULE_DMA_BD0_7_NEXT_BD_MASK,
	.UseNxtBd.Idx = 7U,
	.UseNxtBd.Lsb = XAIE2PSGBL_NOC_MODULE_DMA_BD0_7_USE_NEXT_BD_LSB,
	.UseNxtBd.Mask = XAIE2PSGBL_NOC_MODULE_DMA_BD0_7_USE_NEXT_BD_MASK,
	.ValidBd.Idx = 7U,
	.ValidBd.Lsb = XAIE2PSGBL_NOC_MODULE_DMA_BD0_7_VALID_BD_LSB,
	.ValidBd.Mask = XAIE2PSGBL_NOC_MODULE_DMA_BD0_7_VALID_BD_MASK,
	.OutofOrderBdId.Idx = 2U,
	.OutofOrderBdId.Lsb = XAIE2PSGBL_NOC_MODULE_DMA_BD0_2_OUT_OF_ORDER_BD_ID_LSB,
	.OutofOrderBdId.Mask = XAIE2PSGBL_NOC_MODULE_DMA_BD0_2_OUT_OF_ORDER_BD_ID_MASK,
	.TlastSuppress.Idx = 7U,
	.TlastSuppress.Lsb = XAIE2PSGBL_NOC_MODULE_DMA_BD0_7_TLAST_SUPPRESS_LSB,
	.TlastSuppress.Mask = XAIE2PSGBL_NOC_MODULE_DMA_BD0_7_TLAST_SUPPRESS_MASK,
};

static const  XAie_DmaBdPkt Aie2PSShimDmaBdPktProp =
{
	.EnPkt.Idx = 2U,
	.EnPkt.Lsb = XAIE2PSGBL_NOC_MODULE_DMA_BD0_2_ENABLE_PACKET_LSB,
	.EnPkt.Mask = XAIE2PSGBL_NOC_MODULE_DMA_BD0_2_ENABLE_PACKET_MASK,
	.PktId.Idx = 2U,
	.PktId.Lsb = XAIE2PSGBL_NOC_MODULE_DMA_BD0_2_PACKET_ID_LSB,
	.PktId.Mask = XAIE2PSGBL_NOC_MODULE_DMA_BD0_2_PACKET_ID_MASK,
	.PktType.Idx = 2U,
	.PktType.Lsb = XAIE2PSGBL_NOC_MODULE_DMA_BD0_2_PACKET_TYPE_LSB,
	.PktType.Mask = XAIE2PSGBL_NOC_MODULE_DMA_BD0_2_PACKET_TYPE_MASK,
};

static const  XAie_DmaBdLock Aie2PSShimDmaLockProp =
{
	.AieMlDmaLock.LckRelVal.Idx = 7U,
	.AieMlDmaLock.LckRelVal.Lsb = XAIE2PSGBL_NOC_MODULE_DMA_BD0_7_LOCK_REL_VALUE_LSB,
	.AieMlDmaLock.LckRelVal.Mask = XAIE2PSGBL_NOC_MODULE_DMA_BD0_7_LOCK_REL_VALUE_MASK,
	.AieMlDmaLock.LckRelId.Idx = 7U,
	.AieMlDmaLock.LckRelId.Lsb = XAIE2PSGBL_NOC_MODULE_DMA_BD0_7_LOCK_REL_ID_LSB,
	.AieMlDmaLock.LckRelId.Mask = XAIE2PSGBL_NOC_MODULE_DMA_BD0_7_LOCK_REL_ID_MASK,
	.AieMlDmaLock.LckAcqEn.Idx = 7U,
	.AieMlDmaLock.LckAcqEn.Lsb = XAIE2PSGBL_NOC_MODULE_DMA_BD0_7_LOCK_ACQ_ENABLE_LSB,
	.AieMlDmaLock.LckAcqEn.Mask = XAIE2PSGBL_NOC_MODULE_DMA_BD0_7_LOCK_ACQ_ENABLE_MASK,
	.AieMlDmaLock.LckAcqVal.Idx = 7U,
	.AieMlDmaLock.LckAcqVal.Lsb = XAIE2PSGBL_NOC_MODULE_DMA_BD0_7_LOCK_ACQ_VALUE_LSB,
	.AieMlDmaLock.LckAcqVal.Mask = XAIE2PSGBL_NOC_MODULE_DMA_BD0_7_LOCK_ACQ_VALUE_MASK,
	.AieMlDmaLock.LckAcqId.Idx = 7U,
	.AieMlDmaLock.LckAcqId.Lsb = XAIE2PSGBL_NOC_MODULE_DMA_BD0_7_LOCK_ACQ_ID_LSB,
	.AieMlDmaLock.LckAcqId.Mask = XAIE2PSGBL_NOC_MODULE_DMA_BD0_7_LOCK_ACQ_ID_MASK,
};

static const  XAie_DmaBdBuffer Aie2PSShimDmaBufferProp =
{
	.ShimDmaBuff.AddrLow.Idx = 1U,
	.ShimDmaBuff.AddrLow.Lsb = XAIE2PSGBL_NOC_MODULE_DMA_BD0_1_BASE_ADDRESS_LOW_LSB,
	.ShimDmaBuff.AddrLow.Mask = XAIE2PSGBL_NOC_MODULE_DMA_BD0_1_BASE_ADDRESS_LOW_MASK,
	.ShimDmaBuff.AddrHigh.Idx = 2U,
	.ShimDmaBuff.AddrHigh.Lsb = XAIE2PSGBL_NOC_MODULE_DMA_BD0_2_BASE_ADDRESS_HIGH_LSB,
	.ShimDmaBuff.AddrHigh.Mask = XAIE2PSGBL_NOC_MODULE_DMA_BD0_2_BASE_ADDRESS_HIGH_MASK,
};

static const  XAie_DmaBdDoubleBuffer Aie2PSShimDmaDoubleBufferProp =
{
	.EnDoubleBuff = {0U},
	.BaseAddr_B = {0U},
	.FifoMode = {0U},
	.EnIntrleaved = {0U},
	.IntrleaveCnt = {0U},
	.BuffSelect = {0U},
};

static const  XAie_DmaBdMultiDimAddr Aie2PSShimDmaMultiDimProp =
{
	.AieMlMultiDimAddr.DmaDimProp[0U].StepSize.Idx = 3U,
	.AieMlMultiDimAddr.DmaDimProp[0U].StepSize.Lsb = XAIE2PSGBL_NOC_MODULE_DMA_BD0_3_D0_STEPSIZE_LSB,
	.AieMlMultiDimAddr.DmaDimProp[0U].StepSize.Mask = XAIE2PSGBL_NOC_MODULE_DMA_BD0_3_D0_STEPSIZE_MASK,
	.AieMlMultiDimAddr.DmaDimProp[0U].Wrap.Idx = 3U,
	.AieMlMultiDimAddr.DmaDimProp[0U].Wrap.Lsb = XAIE2PSGBL_NOC_MODULE_DMA_BD0_3_D0_WRAP_LSB,
	.AieMlMultiDimAddr.DmaDimProp[0U].Wrap.Mask = XAIE2PSGBL_NOC_MODULE_DMA_BD0_3_D0_WRAP_MASK,
	.AieMlMultiDimAddr.DmaDimProp[1U].StepSize.Idx =3U ,
	.AieMlMultiDimAddr.DmaDimProp[1U].StepSize.Lsb = XAIE2PSGBL_NOC_MODULE_DMA_BD0_4_D1_STEPSIZE_LSB,
	.AieMlMultiDimAddr.DmaDimProp[1U].StepSize.Mask = XAIE2PSGBL_NOC_MODULE_DMA_BD0_4_D1_STEPSIZE_MASK,
	.AieMlMultiDimAddr.DmaDimProp[1U].Wrap.Idx = 3U,
	.AieMlMultiDimAddr.DmaDimProp[1U].Wrap.Lsb = XAIE2PSGBL_NOC_MODULE_DMA_BD0_4_D1_WRAP_LSB,
	.AieMlMultiDimAddr.DmaDimProp[1U].Wrap.Mask = XAIE2PSGBL_NOC_MODULE_DMA_BD0_4_D1_WRAP_MASK,
	.AieMlMultiDimAddr.DmaDimProp[2U].StepSize.Idx = 5U,
	.AieMlMultiDimAddr.DmaDimProp[2U].StepSize.Lsb = XAIE2PSGBL_NOC_MODULE_DMA_BD0_5_D2_STEPSIZE_LSB,
	.AieMlMultiDimAddr.DmaDimProp[2U].StepSize.Mask = XAIE2PSGBL_NOC_MODULE_DMA_BD0_5_D2_STEPSIZE_MASK,
	.AieMlMultiDimAddr.IterCurr.Idx = 6U,
	.AieMlMultiDimAddr.IterCurr.Lsb = XAIE2PSGBL_NOC_MODULE_DMA_BD0_6_ITERATION_CURRENT_LSB,
	.AieMlMultiDimAddr.IterCurr.Mask = XAIE2PSGBL_NOC_MODULE_DMA_BD0_6_ITERATION_CURRENT_MASK,
	.AieMlMultiDimAddr.Iter.Wrap.Idx = 6U,
	.AieMlMultiDimAddr.Iter.Wrap.Lsb = XAIE2PSGBL_NOC_MODULE_DMA_BD0_6_ITERATION_WRAP_LSB,
	.AieMlMultiDimAddr.Iter.Wrap.Mask = XAIE2PSGBL_NOC_MODULE_DMA_BD0_6_ITERATION_WRAP_MASK,
	.AieMlMultiDimAddr.Iter.StepSize.Idx = 6U,
	.AieMlMultiDimAddr.Iter.StepSize.Lsb = XAIE2PSGBL_NOC_MODULE_DMA_BD0_6_ITERATION_STEPSIZE_LSB,
	.AieMlMultiDimAddr.Iter.StepSize.Mask = XAIE2PSGBL_NOC_MODULE_DMA_BD0_6_ITERATION_STEPSIZE_MASK,
	.AieMlMultiDimAddr.DmaDimProp[2U].Wrap = {0U},
	.AieMlMultiDimAddr.DmaDimProp[3U].Wrap = {0U},
	.AieMlMultiDimAddr.DmaDimProp[3U].StepSize = {0U}
};

static const  XAie_DmaSysProp Aie2PSShimDmaSysProp =
{
	.SMID.Idx = 5U,
	.SMID.Mask = XAIE2PSGBL_NOC_MODULE_SMID_SMID_MASK,
	.SMID.Lsb = XAIE2PSGBL_NOC_MODULE_SMID_SMID_LSB,
	.BurstLen.Idx = 4U,
	.BurstLen.Lsb = XAIE2PSGBL_NOC_MODULE_DMA_BD0_4_BURST_LENGTH_LSB,
	.BurstLen.Mask = XAIE2PSGBL_NOC_MODULE_DMA_BD0_4_BURST_LENGTH_MASK,
	.AxQos.Idx = 5U,
	.AxQos.Lsb = XAIE2PSGBL_NOC_MODULE_DMA_BD0_5_AXQOS_LSB,
	.AxQos.Mask = XAIE2PSGBL_NOC_MODULE_DMA_BD0_5_AXQOS_MASK,
	.SecureAccess.Idx = 3U,
	.SecureAccess.Lsb = XAIE2PSGBL_NOC_MODULE_DMA_BD0_3_SECURE_ACCESS_LSB,
	.SecureAccess.Mask = XAIE2PSGBL_NOC_MODULE_DMA_BD0_3_SECURE_ACCESS_MASK,
	.AxCache.Idx = 5U,
	.AxCache.Lsb = XAIE2PSGBL_NOC_MODULE_DMA_BD0_5_AXCACHE_LSB,
	.AxCache.Mask = XAIE2PSGBL_NOC_MODULE_DMA_BD0_5_AXCACHE_MASK,
};

/* Data structure to capture register offsets and masks for Tile Dma */
static const  XAie_DmaBdProp Aie2PSShimDmaProp =
{
	.AddrAlignMask = 0x3,
	.AddrAlignShift = 0U,
	.AddrMax = 0x1000000000000,
	.LenActualOffset = 0U,
	.StepSizeMax = (1U << 20) - 1U,
	.WrapMax = (1U << 10U) - 1U,
	.IterStepSizeMax = (1U << 20) - 1U,
	.IterWrapMax = (1U << 6U) - 1U,
	.IterCurrMax = (1U << 6) - 1U,
	.BufferLen.Idx = 0U,
	.BufferLen.Lsb = XAIE2PSGBL_NOC_MODULE_DMA_BD0_0_BUFFER_LENGTH_LSB,
	.BufferLen.Mask = XAIE2PSGBL_NOC_MODULE_DMA_BD0_0_BUFFER_LENGTH_MASK,
	.Buffer = &Aie2PSShimDmaBufferProp,
	.DoubleBuffer = &Aie2PSShimDmaDoubleBufferProp,
	.Lock = &Aie2PSShimDmaLockProp,
	.Pkt = &Aie2PSShimDmaBdPktProp,
	.BdEn = &Aie2PSShimDmaBdEnProp,
	.AddrMode = &Aie2PSShimDmaMultiDimProp,
	.Pad = NULL,
	.Compression = NULL,
	.SysProp = &Aie2PSShimDmaSysProp
};

static const XAie_DmaChStatus Aie2PSShimDmaChStatus =
{
	/* This database is common for mm2s and s2mm channels */
	.AieMlDmaChStatus.Status.Lsb = XAIE2PSGBL_NOC_MODULE_DMA_S2MM_STATUS_0_STATUS_LSB,
	.AieMlDmaChStatus.Status.Mask = XAIE2PSGBL_NOC_MODULE_DMA_S2MM_STATUS_0_STATUS_MASK,
	.AieMlDmaChStatus.TaskQSize.Lsb = XAIE2PSGBL_NOC_MODULE_DMA_S2MM_STATUS_0_TASK_QUEUE_SIZE_LSB,
	.AieMlDmaChStatus.TaskQSize.Mask = XAIE2PSGBL_NOC_MODULE_DMA_S2MM_STATUS_0_TASK_QUEUE_SIZE_MASK,
	.AieMlDmaChStatus.StalledLockAcq.Lsb = XAIE2PSGBL_NOC_MODULE_DMA_S2MM_STATUS_0_STALLED_LOCK_ACQ_LSB,
	.AieMlDmaChStatus.StalledLockAcq.Mask = XAIE2PSGBL_NOC_MODULE_DMA_S2MM_STATUS_0_STALLED_LOCK_ACQ_MASK,
	.AieMlDmaChStatus.StalledLockRel.Lsb = XAIE2PSGBL_NOC_MODULE_DMA_S2MM_STATUS_0_STALLED_LOCK_REL_LSB,
	.AieMlDmaChStatus.StalledLockRel.Mask = XAIE2PSGBL_NOC_MODULE_DMA_S2MM_STATUS_0_STALLED_LOCK_REL_MASK,
	.AieMlDmaChStatus.StalledStreamStarve.Lsb = XAIE2PSGBL_NOC_MODULE_DMA_S2MM_STATUS_0_STALLED_STREAM_STARVATION_LSB,
	.AieMlDmaChStatus.StalledStreamStarve.Mask = XAIE2PSGBL_NOC_MODULE_DMA_S2MM_STATUS_0_STALLED_STREAM_STARVATION_MASK,
	.AieMlDmaChStatus.StalledTCT.Lsb = XAIE2PSGBL_NOC_MODULE_DMA_S2MM_STATUS_0_STALLED_TCT_OR_COUNT_FIFO_FULL_LSB,
	.AieMlDmaChStatus.StalledTCT.Mask = XAIE2PSGBL_NOC_MODULE_DMA_S2MM_STATUS_0_STALLED_TCT_OR_COUNT_FIFO_FULL_MASK,
};

/* Data structure to capture register offsets and masks for Mem Tile and
 * Tile Dma Channels
 */
static const  XAie_DmaChProp Aie2PSShimDmaChProp =
{
	.HasFoTMode = XAIE_FEATURE_AVAILABLE,
	.HasControllerId = XAIE_FEATURE_AVAILABLE,
	.HasEnCompression = XAIE_FEATURE_AVAILABLE,
	.HasEnOutOfOrder = XAIE_FEATURE_AVAILABLE,
	.MaxFoTMode = DMA_FoT_COUNTS_FROM_MM_REG,
	.MaxRepeatCount = 256U,
	.ControllerId.Idx = 0U,
	.ControllerId.Lsb = XAIE2PSGBL_NOC_MODULE_DMA_S2MM_0_CTRL_CONTROLLER_ID_LSB ,
	.ControllerId.Mask = XAIE2PSGBL_NOC_MODULE_DMA_S2MM_0_CTRL_CONTROLLER_ID_MASK ,
	.EnCompression.Idx = 0U,
	.EnCompression.Lsb = 0U,
	.EnCompression.Mask = 0U,
	.EnOutofOrder.Idx = 0U,
	.EnOutofOrder.Lsb = XAIE2PSGBL_NOC_MODULE_DMA_S2MM_0_CTRL_ENABLE_OUT_OF_ORDER_LSB,
	.EnOutofOrder.Mask = XAIE2PSGBL_NOC_MODULE_DMA_S2MM_0_CTRL_ENABLE_OUT_OF_ORDER_MASK,
	.FoTMode.Idx = 0,
	.FoTMode.Lsb = XAIE2PSGBL_NOC_MODULE_DMA_S2MM_0_CTRL_FOT_MODE_LSB,
	.FoTMode.Mask = XAIE2PSGBL_NOC_MODULE_DMA_S2MM_0_CTRL_FOT_MODE_MASK ,
	.Reset.Idx = 0U,
	.Reset.Lsb = 0U,
	.Reset.Mask = 0U,
	.EnToken.Idx = 1U,
	.EnToken.Lsb = XAIE2PSGBL_NOC_MODULE_DMA_S2MM_0_TASK_QUEUE_ENABLE_TOKEN_ISSUE_LSB,
	.EnToken.Mask = XAIE2PSGBL_NOC_MODULE_DMA_S2MM_0_TASK_QUEUE_ENABLE_TOKEN_ISSUE_MASK,
	.RptCount.Idx = 1U,
	.RptCount.Lsb = XAIE2PSGBL_NOC_MODULE_DMA_S2MM_0_TASK_QUEUE_REPEAT_COUNT_LSB,
	.RptCount.Mask = XAIE2PSGBL_NOC_MODULE_DMA_S2MM_0_TASK_QUEUE_REPEAT_COUNT_MASK,
	.StartBd.Idx = 1U,
	.StartBd.Lsb = XAIE2PSGBL_NOC_MODULE_DMA_S2MM_0_TASK_QUEUE_START_BD_ID_LSB,
	.StartBd.Mask = XAIE2PSGBL_NOC_MODULE_DMA_S2MM_0_TASK_QUEUE_START_BD_ID_MASK,
	.PauseStream = {0U},
	.PauseMem = {0U},
	.Enable = {0U},
	.StartQSizeMax = 4U,
	.DmaChStatus = &Aie2PSShimDmaChStatus,
};

/* Tile Dma Module */
static const  XAie_DmaMod Aie2PSShimDmaMod =
{
	.BaseAddr = XAIE2PSGBL_NOC_MODULE_DMA_BD0_0,
	.IdxOffset = 0x20,  	/* This is the offset between each BD */
	.NumBds = 16U,	   	/* Number of BDs for AIE2PS Tile DMA */
	.NumLocks = 16U,
	.NumAddrDim = 3U,
	.DoubleBuffering = XAIE_FEATURE_UNAVAILABLE,
	.Compression = XAIE_FEATURE_UNAVAILABLE,
	.Padding = XAIE_FEATURE_UNAVAILABLE,
	.OutofOrderBdId = XAIE_FEATURE_AVAILABLE,
	.InterleaveMode = XAIE_FEATURE_UNAVAILABLE,
	.FifoMode = XAIE_FEATURE_UNAVAILABLE,
	.EnTokenIssue = XAIE_FEATURE_AVAILABLE,
	.RepeatCount = XAIE_FEATURE_AVAILABLE,
	.TlastSuppress = XAIE_FEATURE_AVAILABLE,
	.StartQueueBase = XAIE2PSGBL_NOC_MODULE_DMA_S2MM_0_TASK_QUEUE,
	.ChCtrlBase = XAIE2PSGBL_NOC_MODULE_DMA_S2MM_0_CTRL,
	.NumChannels = 2U,  /* Number of s2mm/mm2s channels */
	.ChIdxOffset = 0x8,  /* This is the offset between each channel */
	.ChStatusBase = XAIE2PSGBL_NOC_MODULE_DMA_S2MM_STATUS_0,
	.ChStatusOffset = 0x8,
	.PadValueBase = 0x0,
	.BdProp = &Aie2PSShimDmaProp,
	.ChProp = &Aie2PSShimDmaChProp,
	.DmaBdInit = &_XAieMl_ShimDmaInit,
	.SetLock = &_XAieMl_DmaSetLock,
	.SetIntrleave = NULL,
	.SetMultiDim = &_XAieMl_DmaSetMultiDim,
	.SetBdIter = &_XAieMl_DmaSetBdIteration,
	.WriteBd = &_XAieMl_ShimDmaWriteBd,
	.PendingBd = &_XAieMl_DmaGetPendingBdCount,
	.WaitforDone = &_XAieMl_DmaWaitForDone,
	.BdChValidity = &_XAieMl_DmaCheckBdChValidity,
	.UpdateBdLen = &_XAieMl_ShimDmaUpdateBdLen,
	.UpdateBdAddr = &_XAieMl_ShimDmaUpdateBdAddr,
};
#endif /* XAIE_FEATURE_DMA_ENABLE */

#ifdef XAIE_FEATURE_DATAMEM_ENABLE
/* Data Memory Module for Tile data memory*/
static const  XAie_MemMod Aie2PSTileMemMod =
{
	.Size = 0x10000,
	.MemAddr = XAIE2PSGBL_MEMORY_MODULE_DATAMEMORY,
	.EccEvntRegOff = XAIE2PSGBL_MEMORY_MODULE_ECC_SCRUBBING_EVENT,
};

/* Data Memory Module for Mem Tile data memory*/
static const  XAie_MemMod Aie2PSMemTileMemMod =
{
	.Size = 0x80000,
	.MemAddr = XAIE2PSGBL_MEM_TILE_MODULE_DATAMEMORY,
	.EccEvntRegOff = XAIE2PSGBL_MEM_TILE_MODULE_ECC_SCRUBBING_EVENT,
};
#endif /* XAIE_FEATURE_DATAMEM_ENABLE */

#ifdef XAIE_FEATURE_PL_ENABLE
/* Register field attributes for PL interface down sizer for 32 and 64 bits */
static const  XAie_RegFldAttr Aie2PSDownSzr32_64Bit[] =
{
	{XAIE2PSGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH0_LSB, XAIE2PSGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH0_MASK},
	{XAIE2PSGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH1_LSB, XAIE2PSGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH1_MASK},
	{XAIE2PSGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH2_LSB, XAIE2PSGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH2_MASK},
	{XAIE2PSGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH3_LSB, XAIE2PSGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH3_MASK},
	{XAIE2PSGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH4_LSB, XAIE2PSGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH4_MASK},
	{XAIE2PSGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH5_LSB, XAIE2PSGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH5_MASK},
	{XAIE2PSGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH6_LSB, XAIE2PSGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH6_MASK},
	{XAIE2PSGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH7_LSB, XAIE2PSGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH7_MASK}
};

/* Register field attributes for PL interface down sizer for 128 bits */
static const  XAie_RegFldAttr Aie2PSDownSzr128Bit[] =
{
	{XAIE2PSGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH0_SOUTH1_128_COMBINE_LSB, XAIE2PSGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH0_SOUTH1_128_COMBINE_MASK},
	{XAIE2PSGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH2_SOUTH3_128_COMBINE_LSB, XAIE2PSGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH2_SOUTH3_128_COMBINE_MASK},
	{XAIE2PSGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH4_SOUTH5_128_COMBINE_LSB, XAIE2PSGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH4_SOUTH5_128_COMBINE_MASK},
	{XAIE2PSGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH6_SOUTH7_128_COMBINE_LSB, XAIE2PSGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH6_SOUTH7_128_COMBINE_MASK}
};

/* Register field attributes for PL interface up sizer */
static const  XAie_RegFldAttr Aie2PSUpSzr32_64Bit[] =
{
	{XAIE2PSGBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH0_LSB, XAIE2PSGBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH0_MASK},
	{XAIE2PSGBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH1_LSB, XAIE2PSGBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH1_MASK},
	{XAIE2PSGBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH2_LSB, XAIE2PSGBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH2_MASK},
	{XAIE2PSGBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH3_LSB, XAIE2PSGBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH3_MASK},
	{XAIE2PSGBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH4_LSB, XAIE2PSGBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH4_MASK},
	{XAIE2PSGBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH5_LSB, XAIE2PSGBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH5_MASK}
};

/* Register field attributes for PL interface up sizer for 128 bits */
static const  XAie_RegFldAttr Aie2PSUpSzr128Bit[] =
{
	{XAIE2PSGBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH0_SOUTH1_128_COMBINE_LSB, XAIE2PSGBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH0_SOUTH1_128_COMBINE_MASK},
	{XAIE2PSGBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH2_SOUTH3_128_COMBINE_LSB, XAIE2PSGBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH2_SOUTH3_128_COMBINE_MASK},
	{XAIE2PSGBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH4_SOUTH5_128_COMBINE_LSB, XAIE2PSGBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH4_SOUTH5_128_COMBINE_MASK}
};

/* Register field attributes for PL interface down sizer bypass */
static const  XAie_RegFldAttr Aie2PSDownSzrByPass[] =
{
	{XAIE2PSGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS_SOUTH0_LSB, XAIE2PSGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS_SOUTH0_MASK},
	{XAIE2PSGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS_SOUTH1_LSB, XAIE2PSGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS_SOUTH1_MASK},
	{XAIE2PSGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS_SOUTH2_LSB, XAIE2PSGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS_SOUTH2_MASK},
	{XAIE2PSGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS_SOUTH4_LSB, XAIE2PSGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS_SOUTH4_MASK},
	{XAIE2PSGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS_SOUTH5_LSB, XAIE2PSGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS_SOUTH5_MASK},
	{XAIE2PSGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS_SOUTH6_LSB, XAIE2PSGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS_SOUTH6_MASK}
};

/* Register field attributes for PL interface down sizer enable */
static const  XAie_RegFldAttr Aie2PSDownSzrEnable[] =
{
	{XAIE2PSGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH0_LSB, XAIE2PSGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH0_MASK},
	{XAIE2PSGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH1_LSB, XAIE2PSGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH1_MASK},
	{XAIE2PSGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH2_LSB, XAIE2PSGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH2_MASK},
	{XAIE2PSGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH3_LSB, XAIE2PSGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH3_MASK},
	{XAIE2PSGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH4_LSB, XAIE2PSGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH4_MASK},
	{XAIE2PSGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH5_LSB, XAIE2PSGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH5_MASK},
	{XAIE2PSGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH6_LSB, XAIE2PSGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH6_MASK},
	{XAIE2PSGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH7_LSB, XAIE2PSGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH7_MASK}
};

/* Register field attributes for SHIMNOC Mux configuration */
static const  XAie_RegFldAttr Aie2PSShimMuxConfig[] =
{
	{XAIE2PSGBL_NOC_MODULE_MUX_CONFIG_SOUTH1_LSB, XAIE2PSGBL_NOC_MODULE_MUX_CONFIG_SOUTH1_MASK},
	{XAIE2PSGBL_NOC_MODULE_MUX_CONFIG_SOUTH3_LSB, XAIE2PSGBL_NOC_MODULE_MUX_CONFIG_SOUTH3_MASK},
	{XAIE2PSGBL_NOC_MODULE_MUX_CONFIG_SOUTH5_LSB, XAIE2PSGBL_NOC_MODULE_MUX_CONFIG_SOUTH5_MASK},
	{XAIE2PSGBL_NOC_MODULE_MUX_CONFIG_SOUTH7_LSB, XAIE2PSGBL_NOC_MODULE_MUX_CONFIG_SOUTH7_MASK},
};

/* Register field attributes for SHIMNOC DeMux configuration */
static const  XAie_RegFldAttr Aie2PSShimDeMuxConfig[] =
{
	{XAIE2PSGBL_NOC_MODULE_DEMUX_CONFIG_SOUTH1_LSB, XAIE2PSGBL_NOC_MODULE_DEMUX_CONFIG_SOUTH1_MASK},
	{XAIE2PSGBL_NOC_MODULE_DEMUX_CONFIG_SOUTH2_LSB, XAIE2PSGBL_NOC_MODULE_DEMUX_CONFIG_SOUTH2_MASK},
	{XAIE2PSGBL_NOC_MODULE_DEMUX_CONFIG_SOUTH3_LSB, XAIE2PSGBL_NOC_MODULE_DEMUX_CONFIG_SOUTH3_MASK},
	{XAIE2PSGBL_NOC_MODULE_DEMUX_CONFIG_SOUTH5_LSB, XAIE2PSGBL_NOC_MODULE_DEMUX_CONFIG_SOUTH5_MASK}
};

#ifdef XAIE_FEATURE_PRIVILEGED_ENABLE
/* Register to set SHIM clock buffer control */
static const XAie_ShimClkBufCntr Aie2PSShimClkBufCntr =
{
	.RegOff = 0xFFF20,
	.RstEnable = XAIE_DISABLE,
	.ClkBufEnable = {0, 0x1}
};

static const XAie_ShimRstMod Aie2PSShimTileRst =
{
	.RegOff = 0,
	.RstCntr = {0},
	.RstShims = _XAieMl_RstShims,
};

/* Register feild attributes for Shim AXI MM config for NSU Errors */
static const XAie_ShimNocAxiMMConfig Aie2PSShimNocAxiMMConfig =
{
	.RegOff = XAIE2PSGBL_NOC_MODULE_ME_AXIMM_CONFIG,
	.NsuSlvErr = {XAIE2PSGBL_NOC_MODULE_ME_AXIMM_CONFIG_SLVERR_BLOCK_LSB, XAIE2PSGBL_NOC_MODULE_ME_AXIMM_CONFIG_SLVERR_BLOCK_MASK},
	.NsuDecErr = {XAIE2PSGBL_NOC_MODULE_ME_AXIMM_CONFIG_DECERR_BLOCK_LSB, XAIE2PSGBL_NOC_MODULE_ME_AXIMM_CONFIG_DECERR_BLOCK_MASK}
};
#endif /* XAIE_FEATURE_PRIVILEGED_ENABLE */

/* PL Interface module for SHIMPL Tiles */
static const  XAie_PlIfMod Aie2PSPlIfMod =
{
	.UpSzrOff = XAIE2PSGBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG,
	.DownSzrOff = XAIE2PSGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG,
	.DownSzrEnOff = XAIE2PSGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE,
	.DownSzrByPassOff = XAIE2PSGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS,
	.ColRstOff = 0xFFF28,
	.NumUpSzrPorts = 0x6,
	.MaxByPassPortNum = 0x6,
	.NumDownSzrPorts = 0x8,
	.UpSzr32_64Bit = Aie2PSUpSzr32_64Bit,
	.UpSzr128Bit = Aie2PSUpSzr128Bit,
	.DownSzr32_64Bit = Aie2PSDownSzr32_64Bit,
	.DownSzr128Bit = Aie2PSDownSzr128Bit,
	.DownSzrEn = Aie2PSDownSzrEnable,
	.DownSzrByPass = Aie2PSDownSzrByPass,
	.ShimNocMuxOff = 0x0,
	.ShimNocDeMuxOff = 0x0,
	.ShimNocMux = NULL,
	.ShimNocDeMux = NULL,
	.ColRst = {0, 0x1},
#ifdef XAIE_FEATURE_PRIVILEGED_ENABLE
	.ClkBufCntr = &Aie2PSShimClkBufCntr,
	.ShimTileRst = &Aie2PSShimTileRst,
#else
	.ClkBufCntr = NULL,
	.ShimTileRst = NULL,
#endif /* XAIE_FEATURE_PRIVILEGED_ENABLE */
	.ShimNocAxiMM = NULL,
};

/* PL Interface module for SHIMNOC Tiles */
static const  XAie_PlIfMod Aie2PSShimTilePlIfMod =
{
	.UpSzrOff = XAIE2PSGBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG,
	.DownSzrOff = XAIE2PSGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG,
	.DownSzrEnOff = XAIE2PSGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE,
	.DownSzrByPassOff = XAIE2PSGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS,
	.ColRstOff = 0xFFF28,
	.NumUpSzrPorts = 0x6,
	.MaxByPassPortNum = 0x6,
	.NumDownSzrPorts = 0x8,
	.UpSzr32_64Bit = Aie2PSUpSzr32_64Bit,
	.UpSzr128Bit = Aie2PSUpSzr128Bit,
	.DownSzr32_64Bit = Aie2PSDownSzr32_64Bit,
	.DownSzr128Bit = Aie2PSDownSzr128Bit,
	.DownSzrEn = Aie2PSDownSzrEnable,
	.DownSzrByPass = Aie2PSDownSzrByPass,
	.ShimNocMuxOff = XAIE2PSGBL_NOC_MODULE_MUX_CONFIG,
	.ShimNocDeMuxOff = XAIE2PSGBL_NOC_MODULE_DEMUX_CONFIG,
	.ShimNocMux = Aie2PSShimMuxConfig,
	.ShimNocDeMux = Aie2PSShimDeMuxConfig,
	.ColRst = {0, 0x1},
#ifdef XAIE_FEATURE_PRIVILEGED_ENABLE
	.ClkBufCntr = &Aie2PSShimClkBufCntr,
	.ShimTileRst = &Aie2PSShimTileRst,
	.ShimNocAxiMM = &Aie2PSShimNocAxiMMConfig,
#else
	.ClkBufCntr = NULL,
	.ShimTileRst = NULL,
	.ShimNocAxiMM = NULL,
#endif /* XAIE_FEATURE_PRIVILEGED_ENABLE */
};
#endif /* XAIE_FEATURE_PL_ENABLE */

#ifdef XAIE_FEATURE_LOCK_ENABLE
static const XAie_RegFldAttr Aie2PSTileLockInit =
{
	.Lsb = XAIE2PSGBL_MEMORY_MODULE_LOCK0_VALUE_LOCK_VALUE_LSB,
	.Mask = XAIE2PSGBL_MEMORY_MODULE_LOCK0_VALUE_LOCK_VALUE_MASK,
};

/* Lock Module for AIE Tiles  */
static const  XAie_LockMod Aie2PSTileLockMod =
{
	.BaseAddr = XAIE2PSGBL_MEMORY_MODULE_LOCK_REQUEST,
	.NumLocks = 16U,
	.LockIdOff = 0x400,
	.RelAcqOff = 0x200,
	.LockValOff = 0x4,
	.LockValUpperBound = 63,
	.LockValLowerBound = -64,
	.LockSetValBase = XAIE2PSGBL_MEMORY_MODULE_LOCK0_VALUE,
	.LockSetValOff = 0x10,
	.LockInit = &Aie2PSTileLockInit,
	.Acquire = &_XAieMl_LockAcquire,
	.Release = &_XAieMl_LockRelease,
	.SetValue = &_XAieMl_LockSetValue,
};

static const XAie_RegFldAttr Aie2PSShimNocLockInit =
{
	.Lsb = XAIE2PSGBL_NOC_MODULE_LOCK0_VALUE_LOCK_VALUE_LSB,
	.Mask = XAIE2PSGBL_NOC_MODULE_LOCK0_VALUE_LOCK_VALUE_MASK,
};

/* Lock Module for SHIM NOC Tiles  */
static const  XAie_LockMod Aie2PSShimNocLockMod =
{
	.BaseAddr = XAIE2PSGBL_NOC_MODULE_LOCK_REQUEST,
	.NumLocks = 16U,
	.LockIdOff = 0x400,
	.RelAcqOff = 0x200,
	.LockValOff = 0x4,
	.LockValUpperBound = 63,
	.LockValLowerBound = -64,
	.LockSetValBase = XAIE2PSGBL_NOC_MODULE_LOCK0_VALUE,
	.LockSetValOff = 0x10,
	.LockInit = &Aie2PSShimNocLockInit,
	.Acquire = &_XAieMl_LockAcquire,
	.Release = &_XAieMl_LockRelease,
	.SetValue = &_XAieMl_LockSetValue,
};

static const XAie_RegFldAttr Aie2PSMemTileLockInit =
{
	.Lsb = XAIE2PSGBL_MEM_TILE_MODULE_LOCK0_VALUE_LOCK_VALUE_LSB,
	.Mask = XAIE2PSGBL_MEM_TILE_MODULE_LOCK0_VALUE_LOCK_VALUE_MASK,
};

/* Lock Module for Mem Tiles  */
static const  XAie_LockMod Aie2PSMemTileLockMod =
{
	.BaseAddr = XAIE2PSGBL_MEM_TILE_MODULE_LOCK_REQUEST,
	.NumLocks = 64U,
	.LockIdOff = 0x400,
	.RelAcqOff = 0x200,
	.LockValOff = 0x4,
	.LockValUpperBound = 63,
	.LockValLowerBound = -64,
	.LockSetValBase = XAIE2PSGBL_MEM_TILE_MODULE_LOCK0_VALUE,
	.LockSetValOff = 0x10,
	.LockInit = &Aie2PSMemTileLockInit,
	.Acquire = &_XAieMl_LockAcquire,
	.Release = &_XAieMl_LockRelease,
	.SetValue = &_XAieMl_LockSetValue,
};
#endif /* XAIE_FEATURE_LOCK_ENABLE */

#ifdef XAIE_FEATURE_PERFCOUNT_ENABLE
/*
 * Data structure to capture registers & offsets for Core and memory Module of
 * performance counter.
 */
static const XAie_PerfMod Aie2PSTilePerfCnt[] =
{
	{	.MaxCounterVal = 2U,
		.StartStopShift = 16U,
		.ResetShift = 8U,
		.PerfCounterOffsetAdd = 0X4,
		.PerfCtrlBaseAddr = XAIE2PSGBL_MEMORY_MODULE_PERFORMANCE_CONTROL0,
		.PerfCtrlOffsetAdd = 0x4,
		.PerfCtrlResetBaseAddr = XAIE2PSGBL_MEMORY_MODULE_PERFORMANCE_CONTROL1,
		.PerfCounterBaseAddr = XAIE2PSGBL_MEMORY_MODULE_PERFORMANCE_COUNTER0,
		.PerfCounterEvtValBaseAddr = XAIE2PSGBL_MEMORY_MODULE_PERFORMANCE_COUNTER0_EVENT_VALUE,
		{XAIE2PSGBL_MEMORY_MODULE_PERFORMANCE_CONTROL0_CNT0_START_EVENT_LSB, XAIE2PSGBL_MEMORY_MODULE_PERFORMANCE_CONTROL0_CNT0_START_EVENT_MASK},
		{XAIE2PSGBL_MEMORY_MODULE_PERFORMANCE_CONTROL0_CNT0_STOP_EVENT_LSB, XAIE2PSGBL_MEMORY_MODULE_PERFORMANCE_CONTROL0_CNT0_STOP_EVENT_MASK},
		{XAIE2PSGBL_MEMORY_MODULE_PERFORMANCE_CONTROL1_CNT0_RESET_EVENT_LSB,XAIE2PSGBL_MEMORY_MODULE_PERFORMANCE_CONTROL1_CNT0_RESET_EVENT_MASK},
	},
	{	.MaxCounterVal = 4U,
		.StartStopShift = 16U,
		.ResetShift = 8U,
		.PerfCounterOffsetAdd = 0X4,
		.PerfCtrlBaseAddr = XAIE2PSGBL_CORE_MODULE_PERFORMANCE_CONTROL0,
		.PerfCtrlOffsetAdd = 0x4,
		.PerfCtrlResetBaseAddr = XAIE2PSGBL_CORE_MODULE_PERFORMANCE_CONTROL2,
		.PerfCounterBaseAddr = XAIE2PSGBL_CORE_MODULE_PERFORMANCE_COUNTER0,
		.PerfCounterEvtValBaseAddr = XAIE2PSGBL_CORE_MODULE_PERFORMANCE_COUNTER0_EVENT_VALUE,
		{XAIE2PSGBL_CORE_MODULE_PERFORMANCE_CONTROL0_CNT0_START_EVENT_LSB, XAIE2PSGBL_CORE_MODULE_PERFORMANCE_CONTROL0_CNT0_START_EVENT_MASK},
		{XAIE2PSGBL_CORE_MODULE_PERFORMANCE_CONTROL0_CNT0_STOP_EVENT_LSB, XAIE2PSGBL_CORE_MODULE_PERFORMANCE_CONTROL0_CNT0_STOP_EVENT_MASK},
		{XAIE2PSGBL_CORE_MODULE_PERFORMANCE_CONTROL2_CNT0_RESET_EVENT_LSB, XAIE2PSGBL_CORE_MODULE_PERFORMANCE_CONTROL2_CNT0_RESET_EVENT_MASK},
	}
};

/*
 * Data structure to capture registers & offsets for PL Module of performance
 * counter.
 */
static const XAie_PerfMod Aie2PSPlPerfCnt =
{
	.MaxCounterVal = 2U,
	.StartStopShift = 16U,
	.ResetShift = 8U,
	.PerfCounterOffsetAdd = 0x4,
	.PerfCtrlBaseAddr = XAIE2PSGBL_PL_MODULE_PERFORMANCE_CTRL0,
	.PerfCtrlOffsetAdd = 0x0,
	.PerfCtrlResetBaseAddr = XAIE2PSGBL_PL_MODULE_PERFORMANCE_CTRL1,
	.PerfCounterBaseAddr = XAIE2PSGBL_PL_MODULE_PERFORMANCE_COUNTER0,
	.PerfCounterEvtValBaseAddr = XAIE2PSGBL_PL_MODULE_PERFORMANCE_COUNTER0_EVENT_VALUE,
	{XAIE2PSGBL_PL_MODULE_PERFORMANCE_CTRL0_CNT0_START_EVENT_LSB, XAIE2PSGBL_PL_MODULE_PERFORMANCE_CTRL0_CNT0_START_EVENT_MASK},
	{XAIE2PSGBL_PL_MODULE_PERFORMANCE_CTRL0_CNT0_STOP_EVENT_LSB, XAIE2PSGBL_PL_MODULE_PERFORMANCE_CTRL0_CNT0_STOP_EVENT_MASK},
	{XAIE2PSGBL_PL_MODULE_PERFORMANCE_CTRL1_CNT0_RESET_EVENT_LSB,XAIE2PSGBL_PL_MODULE_PERFORMANCE_CTRL1_CNT0_RESET_EVENT_MASK},};

/*
 * Data structure to capture registers & offsets for Mem tile Module of
 * performance counter.
 */
static const XAie_PerfMod Aie2PSMemTilePerfCnt =
{
	.MaxCounterVal = 4U,
	.StartStopShift = 16U,
	.ResetShift = 8U,
	.PerfCounterOffsetAdd = 0X4,
	.PerfCtrlBaseAddr = XAIE2PSGBL_MEM_TILE_MODULE_PERFORMANCE_CONTROL0,
	.PerfCtrlOffsetAdd = 0x4,
	.PerfCtrlResetBaseAddr = XAIE2PSGBL_MEM_TILE_MODULE_PERFORMANCE_CONTROL2,
	.PerfCounterBaseAddr = XAIE2PSGBL_MEM_TILE_MODULE_PERFORMANCE_COUNTER0,
	.PerfCounterEvtValBaseAddr = XAIE2PSGBL_MEM_TILE_MODULE_PERFORMANCE_COUNTER0_EVENT_VALUE,
	{XAIE2PSGBL_MEM_TILE_MODULE_PERFORMANCE_CONTROL0_CNT0_START_EVENT_LSB, XAIE2PSGBL_MEM_TILE_MODULE_PERFORMANCE_CONTROL0_CNT0_START_EVENT_MASK},
	{XAIE2PSGBL_MEM_TILE_MODULE_PERFORMANCE_CONTROL0_CNT0_STOP_EVENT_LSB, XAIE2PSGBL_MEM_TILE_MODULE_PERFORMANCE_CONTROL0_CNT0_STOP_EVENT_MASK},
	{XAIE2PSGBL_MEM_TILE_MODULE_PERFORMANCE_CONTROL2_CNT0_RESET_EVENT_LSB, XAIE2PSGBL_MEM_TILE_MODULE_PERFORMANCE_CONTROL2_CNT0_RESET_EVENT_MASK},
};
#endif /* XAIE_FEATURE_PERFCOUNT_ENABLE */

#ifdef XAIE_FEATURE_CORE_ENABLE
	#define AIE2PSCOREMOD &Aie2PSCoreMod
#else
	#define AIE2PSCOREMOD NULL
#endif
#ifdef XAIE_FEATURE_SS_ENABLE
	#define AIE2PSTILESTRMSW &Aie2PSTileStrmSw
	#define AIE2PSSHIMSTRMSW &Aie2PSShimTileStrmSw
	#define AIE2PSMEMTILESTRMSW &Aie2PSMemTileStrmSw
#else
	#define AIE2PSTILESTRMSW NULL
	#define AIE2PSSHIMSTRMSW NULL
	#define AIE2PSMEMTILESTRMSW NULL
#endif
#ifdef XAIE_FEATURE_DMA_ENABLE
	#define AIE2PSTILEDMAMOD &Aie2PSTileDmaMod
	#define AIE2PSSHIMDMAMOD &Aie2PSShimDmaMod
	#define AIE2PSMEMTILEDMAMOD &Aie2PSMemTileDmaMod
#else
	#define AIE2PSTILEDMAMOD NULL
	#define AIE2PSSHIMDMAMOD NULL
	#define AIE2PSMEMTILEDMAMOD NULL
#endif
#ifdef XAIE_FEATURE_DATAMEM_ENABLE
	#define AIE2PSTILEMEMMOD &Aie2PSTileMemMod
	#define AIE2PSMEMTILEMEMMOD &Aie2PSMemTileMemMod
#else
	#define AIE2PSTILEMEMMOD NULL
	#define AIE2PSMEMTILEMEMMOD NULL
#endif
#ifdef XAIE_FEATURE_PL_ENABLE
	#define AIE2PSSHIMTILEPLIFMOD &Aie2PSShimTilePlIfMod
	#define AIE2PSPLIFMOD &Aie2PSPlIfMod
#else
	#define AIE2PSSHIMTILEPLIFMOD NULL
	#define AIE2PSPLIFMOD NULL
#endif
#ifdef XAIE_FEATURE_LOCK_ENABLE
	#define AIE2PSTILELOCKMOD &Aie2PSTileLockMod
	#define AIE2PSSHIMNOCLOCKMOD &Aie2PSShimNocLockMod
	#define AIE2PSMEMTILELOCKMOD &Aie2PSMemTileLockMod
#else
	#define AIE2PSTILELOCKMOD NULL
	#define AIE2PSSHIMNOCLOCKMOD NULL
	#define AIE2PSMEMTILELOCKMOD NULL
#endif
#ifdef XAIE_FEATURE_PERFCOUNT_ENABLE
	#define AIE2PSTILEPERFCNT Aie2PSTilePerfCnt
	#define AIE2PSPLPERFCNT &Aie2PSPlPerfCnt
	#define AIE2PSMEMTILEPERFCNT &Aie2PSMemTilePerfCnt
#else
	#define AIE2PSTILEPERFCNT NULL
	#define AIE2PSPLPERFCNT NULL
	#define AIE2PSMEMTILEPERFCNT NULL
#endif

/*
 * AIE2PS Module
 * This data structure captures all the modules for each tile type.
 * Depending on the tile type, this data strcuture can be used to access all
 * hardware properties of individual modules.
 */
XAie_TileMod Aie2PSMod[] =
{
	{
		/*
		 * AIE2PS Tile Module indexed using XAIEGBL_TILE_TYPE_AIETILE
		 */
		.NumModules = 2U,
		.CoreMod = AIE2PSCOREMOD,
		.StrmSw  = AIE2PSTILESTRMSW,
		.DmaMod  = AIE2PSTILEDMAMOD,
		.MemMod  = AIE2PSTILEMEMMOD,
		.PlIfMod = NULL,
		.LockMod = AIE2PSTILELOCKMOD,
		.PerfMod = AIE2PSTILEPERFCNT,
	},
	{
		/*
		 * AIE2PS Shim Noc Module indexed using XAIEGBL_TILE_TYPE_SHIMNOC
		 */
		.NumModules = 1U,
		.CoreMod = NULL,
		.StrmSw  = AIE2PSSHIMSTRMSW,
		.DmaMod  = AIE2PSSHIMDMAMOD,
		.MemMod  = NULL,
		.PlIfMod = AIE2PSSHIMTILEPLIFMOD,
		.LockMod = AIE2PSSHIMNOCLOCKMOD,
		.PerfMod = AIE2PSPLPERFCNT,
	},
	{
		/*
		 * AIE2PS Shim PL Module indexed using XAIEGBL_TILE_TYPE_SHIMPL
		 */
		.NumModules = 1U,
		.CoreMod = NULL,
		.StrmSw  = AIE2PSSHIMSTRMSW,
		.DmaMod  = NULL,
		.MemMod  = NULL,
		.PlIfMod = AIE2PSPLIFMOD,
		.LockMod = NULL,
		.PerfMod = AIE2PSPLPERFCNT,
	},
	{
		/*
		 * AIE2PS MemTile Module indexed using XAIEGBL_TILE_TYPE_MEMTILE
		 */
		.NumModules = 1U,
		.CoreMod = NULL,
		.StrmSw  = AIE2PSMEMTILESTRMSW,
		.DmaMod  = AIE2PSMEMTILEDMAMOD,
		.MemMod  = AIE2PSMEMTILEMEMMOD,
		.PlIfMod = NULL,
		.LockMod = AIE2PSMEMTILELOCKMOD,
		.PerfMod = AIE2PSMEMTILEPERFCNT,
	},
};

/* Device level operations for aieml */
XAie_DeviceOps Aie2PSDevOps =
{
	.IsCheckerBoard = 0U,
	.TilesInUse = Aie2PSTilesInUse,
	.MemInUse = Aie2PSMemInUse,
	.CoreInUse = Aie2PSCoreInUse,
	.GetTTypefromLoc = &_XAie2Ipu_GetTTypefromLoc,
	.SetPartColShimReset = NULL,
	.SetPartColClockAfterRst = NULL,
	.SetPartIsolationAfterRst = NULL,
	.PartMemZeroInit = NULL,
	.RequestTiles = NULL,
};

/** @} */
