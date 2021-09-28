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
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   09/07/2021  Initial creation
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xaie_core_aieml.h"
#include "xaie_device_aieml.h"
#include "xaie_dma_aieml.h"
#include "xaie_locks_aieml.h"
#include "xaie_reset_aieml.h"
#include "xaie_ss_aieml.h"
#include "xaie2pgbl_params.h"
#include "xaiegbl_regdef.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/**************************** Macro Definitions ******************************/

/************************** Variable Definitions *****************************/
#ifdef XAIE_FEATURE_CORE_ENABLE
/*
 * Global instance for Core module Core_Control register.
 */
static const  XAie_RegCoreCtrl Aie2PCoreCtrlReg =
{
	XAIE2PGBL_CORE_MODULE_CORE_CONTROL,
	{XAIE2PGBL_CORE_MODULE_CORE_CONTROL_ENABLE_LSB, XAIE2PGBL_CORE_MODULE_CORE_CONTROL_ENABLE_MASK},
	{XAIE2PGBL_CORE_MODULE_CORE_CONTROL_RESET_LSB, XAIE2PGBL_CORE_MODULE_CORE_CONTROL_RESET_MASK}
};

/*
 * Global instance for Core module Core_Status register.
 */
static const  XAie_RegCoreSts Aie2PCoreStsReg =
{
	XAIE2PGBL_CORE_MODULE_CORE_STATUS,
	{XAIE2PGBL_CORE_MODULE_CORE_STATUS_CORE_DONE_LSB, XAIE2PGBL_CORE_MODULE_CORE_STATUS_CORE_DONE_MASK},
	{XAIE2PGBL_CORE_MODULE_CORE_STATUS_RESET_LSB, XAIE2PGBL_CORE_MODULE_CORE_STATUS_RESET_MASK},
	{XAIE2PGBL_CORE_MODULE_CORE_STATUS_ENABLE_LSB, XAIE2PGBL_CORE_MODULE_CORE_STATUS_ENABLE_MASK}
};

/*
 * Global instance for Core module for core debug registers.
 */
static const XAie_RegCoreDebug Aie2PCoreDebugReg =
{
	.RegOff = XAIE2PGBL_CORE_MODULE_DEBUG_CONTROL0,
	.DebugCtrl1Offset = XAIE2PGBL_CORE_MODULE_DEBUG_CONTROL1,
	.DebugHaltCoreEvent1.Lsb = XAIE2PGBL_CORE_MODULE_DEBUG_CONTROL1_DEBUG_HALT_CORE_EVENT1_LSB,
	.DebugHaltCoreEvent1.Mask = XAIE2PGBL_CORE_MODULE_DEBUG_CONTROL1_DEBUG_HALT_CORE_EVENT1_MASK,
	.DebugHaltCoreEvent0.Lsb = XAIE2PGBL_CORE_MODULE_DEBUG_CONTROL1_DEBUG_HALT_CORE_EVENT0_LSB,
	.DebugHaltCoreEvent0.Mask = XAIE2PGBL_CORE_MODULE_DEBUG_CONTROL1_DEBUG_HALT_CORE_EVENT0_MASK,
	.DebugSStepCoreEvent.Lsb = XAIE2PGBL_CORE_MODULE_DEBUG_CONTROL1_DEBUG_SINGLESTEP_CORE_EVENT_LSB,
	.DebugSStepCoreEvent.Mask = XAIE2PGBL_CORE_MODULE_DEBUG_CONTROL1_DEBUG_SINGLESTEP_CORE_EVENT_MASK,
	.DebugResumeCoreEvent.Lsb = XAIE2PGBL_CORE_MODULE_DEBUG_CONTROL1_DEBUG_RESUME_CORE_EVENT_LSB,
	.DebugResumeCoreEvent.Mask = XAIE2PGBL_CORE_MODULE_DEBUG_CONTROL1_DEBUG_RESUME_CORE_EVENT_MASK,
	.DebugHalt.Lsb = XAIE2PGBL_CORE_MODULE_DEBUG_CONTROL0_DEBUG_HALT_BIT_LSB,
	.DebugHalt.Mask = XAIE2PGBL_CORE_MODULE_DEBUG_CONTROL0_DEBUG_HALT_BIT_MASK
};

static const XAie_RegCoreDebugStatus Aie2PCoreDebugStatus =
{
	.RegOff = XAIE2PGBL_CORE_MODULE_DEBUG_STATUS,
	.DbgEvent1Halt.Lsb = XAIE2PGBL_CORE_MODULE_DEBUG_STATUS_DEBUG_EVENT1_HALTED_LSB,
	.DbgEvent1Halt.Mask = XAIE2PGBL_CORE_MODULE_DEBUG_STATUS_DEBUG_EVENT1_HALTED_MASK,
	.DbgEvent0Halt.Lsb = XAIE2PGBL_CORE_MODULE_DEBUG_STATUS_DEBUG_EVENT0_HALTED_LSB,
	.DbgEvent0Halt.Mask = XAIE2PGBL_CORE_MODULE_DEBUG_STATUS_DEBUG_EVENT0_HALTED_MASK,
	.DbgStrmStallHalt.Lsb = XAIE2PGBL_CORE_MODULE_DEBUG_STATUS_STREAM_STALL_HALTED_LSB,
	.DbgStrmStallHalt.Mask = XAIE2PGBL_CORE_MODULE_DEBUG_STATUS_STREAM_STALL_HALTED_MASK,
	.DbgLockStallHalt.Lsb = XAIE2PGBL_CORE_MODULE_DEBUG_STATUS_LOCK_STALL_HALTED_LSB,
	.DbgLockStallHalt.Mask = XAIE2PGBL_CORE_MODULE_DEBUG_STATUS_LOCK_STALL_HALTED_MASK,
	.DbgMemStallHalt.Lsb = XAIE2PGBL_CORE_MODULE_DEBUG_STATUS_MEMORY_STALL_HALTED_LSB,
	.DbgMemStallHalt.Mask = XAIE2PGBL_CORE_MODULE_DEBUG_STATUS_MEMORY_STALL_HALTED_MASK,
	.DbgPCEventHalt.Lsb = XAIE2PGBL_CORE_MODULE_DEBUG_STATUS_PC_EVENT_HALTED_LSB,
	.DbgPCEventHalt.Mask = XAIE2PGBL_CORE_MODULE_DEBUG_STATUS_PC_EVENT_HALTED_MASK,
	.DbgHalt.Lsb = XAIE2PGBL_CORE_MODULE_DEBUG_STATUS_DEBUG_HALTED_LSB,
	.DbgHalt.Mask = XAIE2PGBL_CORE_MODULE_DEBUG_STATUS_DEBUG_HALTED_MASK,
};

/*
 * Global instance for core event registers in the core module.
 */
static const XAie_RegCoreEvents Aie2PCoreEventReg =
{
	.EnableEventOff = XAIE2PGBL_CORE_MODULE_ENABLE_EVENTS,
	.DisableEventOccurred.Lsb = XAIE2PGBL_CORE_MODULE_ENABLE_EVENTS_DISABLE_EVENT_OCCURRED_LSB,
	.DisableEventOccurred.Mask = XAIE2PGBL_CORE_MODULE_ENABLE_EVENTS_DISABLE_EVENT_OCCURRED_MASK,
	.DisableEvent.Lsb = XAIE2PGBL_CORE_MODULE_ENABLE_EVENTS_DISABLE_EVENT_LSB,
	.DisableEvent.Mask = XAIE2PGBL_CORE_MODULE_ENABLE_EVENTS_DISABLE_EVENT_MASK,
	.EnableEvent.Lsb = XAIE2PGBL_CORE_MODULE_ENABLE_EVENTS_ENABLE_EVENT_LSB,
	.EnableEvent.Mask = XAIE2PGBL_CORE_MODULE_ENABLE_EVENTS_ENABLE_EVENT_MASK,
};

/*
 * Global instance for core accumulator control register.
 */
static const XAie_RegCoreAccumCtrl Aie2PCoreAccumCtrlReg =
{
	.RegOff = XAIE2PGBL_CORE_MODULE_ACCUMULATOR_CONTROL,
	.CascadeInput.Lsb = XAIE2PGBL_CORE_MODULE_ACCUMULATOR_CONTROL_INPUT_LSB,
	.CascadeInput.Mask = XAIE2PGBL_CORE_MODULE_ACCUMULATOR_CONTROL_INPUT_MASK,
	.CascadeOutput.Lsb = XAIE2PGBL_CORE_MODULE_ACCUMULATOR_CONTROL_OUTPUT_LSB,
	.CascadeOutput.Mask = XAIE2PGBL_CORE_MODULE_ACCUMULATOR_CONTROL_OUTPUT_MASK,
};
#endif /* XAIE_FEATURE_CORE_ENABLE */

#ifdef XAIE_FEATURE_CORE_ENABLE
/* Register field attribute for core process bus control */
static const XAie_RegCoreProcBusCtrl Aie2PCoreProcBusCtrlReg =
{
	.RegOff = XAIE2PGBL_CORE_MODULE_CORE_PROCESSOR_BUS,
	.CtrlEn = {XAIE2PGBL_CORE_MODULE_CORE_PROCESSOR_BUS_ENABLE_LSB, XAIE2PGBL_CORE_MODULE_CORE_PROCESSOR_BUS_ENABLE_MASK}
};

/* Core Module */
static const  XAie_CoreMod Aie2PCoreMod =
{
	.IsCheckerBoard = 0U,
	.ProgMemAddr = 0x0,
	.ProgMemSize = 16 * 1024,
	.DataMemAddr = 0x40000,
	.ProgMemHostOffset = XAIE2PGBL_CORE_MODULE_PROGRAM_MEMORY,
	.DataMemSize = 64 * 1024,		/* AIE2P Tile Memory is 64kB */
	.DataMemShift = 16,
	.EccEvntRegOff = XAIE2PGBL_CORE_MODULE_ECC_SCRUBBING_EVENT,
	.CoreCtrl = &Aie2PCoreCtrlReg,
	.CoreDebugStatus = &Aie2PCoreDebugStatus,
	.CoreSts = &Aie2PCoreStsReg,
	.CoreDebug = &Aie2PCoreDebugReg,
	.CoreEvent = &Aie2PCoreEventReg,
	.CoreAccumCtrl = &Aie2PCoreAccumCtrlReg,
	.ProcBusCtrl = &Aie2PCoreProcBusCtrlReg,
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
static const  XAie_StrmPort Aie2PTileStrmMstr[SS_PORT_TYPE_MAX] =
{
	{	/* Core */
		.NumPorts = 1,
		.PortBaseAddr = XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0,
	},
	{	/* DMA */
		.NumPorts = 2,
		.PortBaseAddr = XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_DMA0,
	},
	{	/* Ctrl */
		.NumPorts = 1,
		.PortBaseAddr = XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_TILE_CTRL,
	},
	{	/* Fifo */
		.NumPorts = 1,
		.PortBaseAddr = XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_FIFO0,
	},
	{	/* South */
		.NumPorts = 4,
		.PortBaseAddr = XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_SOUTH0,
	},
	{	/* West */
		.NumPorts = 4,
		.PortBaseAddr = XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_WEST0,
	},
	{	/* North */
		.NumPorts = 6,
		.PortBaseAddr = XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_NORTH0,
	},
	{	/* East */
		.NumPorts = 4,
		.PortBaseAddr = XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_EAST0,
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
static const  XAie_StrmPort Aie2PTileStrmSlv[SS_PORT_TYPE_MAX] =
{
	{	/* Core */
		.NumPorts = 1,
		.PortBaseAddr = XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_AIE_CORE0,
	},
	{	/* DMA */
		.NumPorts = 2,
		.PortBaseAddr = XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_DMA_0,
	},
	{	/* Ctrl */
		.NumPorts = 1,
		.PortBaseAddr = XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_TILE_CTRL,
	},
	{	/* Fifo */
		.NumPorts = 1,
		.PortBaseAddr = XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_FIFO_0,
	},
	{	/* South */
		.NumPorts = 6,
		.PortBaseAddr = XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_SOUTH_0,
	},
	{	/* West */
		.NumPorts = 4,
		.PortBaseAddr = XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_WEST_0,
	},
	{	/* North */
		.NumPorts = 4,
		.PortBaseAddr = XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_NORTH_0,
	},
	{	/* East */
		.NumPorts = 4,
		.PortBaseAddr = XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_EAST_0,
	},
	{	/* Trace */
		.NumPorts = 2,
		.PortBaseAddr = XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_MEM_TRACE
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
		.PortBaseAddr = XAIE2PGBL_PL_MODULE_STREAM_SWITCH_MASTER_CONFIG_TILE_CTRL,
	},
	{	/* Fifo */
		.NumPorts = 1,
		.PortBaseAddr = XAIE2PGBL_PL_MODULE_STREAM_SWITCH_MASTER_CONFIG_FIFO0,
	},
	{	/* South */
		.NumPorts = 6,
		.PortBaseAddr = XAIE2PGBL_PL_MODULE_STREAM_SWITCH_MASTER_CONFIG_SOUTH0,
	},
	{	/* West */
		.NumPorts = 4,
		.PortBaseAddr = XAIE2PGBL_PL_MODULE_STREAM_SWITCH_MASTER_CONFIG_WEST0,
	},
	{	/* North */
		.NumPorts = 6,
		.PortBaseAddr = XAIE2PGBL_PL_MODULE_STREAM_SWITCH_MASTER_CONFIG_NORTH0,
	},
	{	/* East */
		.NumPorts = 4,
		.PortBaseAddr = XAIE2PGBL_PL_MODULE_STREAM_SWITCH_MASTER_CONFIG_EAST0,
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
		.PortBaseAddr = XAIE2PGBL_PL_MODULE_STREAM_SWITCH_SLAVE_CONFIG_TILE_CTRL,
	},
	{	/* Fifo */
		.NumPorts = 1,
		.PortBaseAddr = XAIE2PGBL_PL_MODULE_STREAM_SWITCH_SLAVE_CONFIG_FIFO_0,
	},
	{	/* South */
		.NumPorts = 8,
		.PortBaseAddr = XAIE2PGBL_PL_MODULE_STREAM_SWITCH_SLAVE_CONFIG_SOUTH_0,
	},
	{	/* West */
		.NumPorts = 4,
		.PortBaseAddr = XAIE2PGBL_PL_MODULE_STREAM_SWITCH_SLAVE_CONFIG_WEST_0,
	},
	{	/* North */
		.NumPorts = 4,
		.PortBaseAddr = XAIE2PGBL_PL_MODULE_STREAM_SWITCH_SLAVE_CONFIG_NORTH_0,
	},
	{	/* East */
		.NumPorts = 4,
		.PortBaseAddr = XAIE2PGBL_PL_MODULE_STREAM_SWITCH_SLAVE_CONFIG_EAST_0,
	},
	{	/* Trace */
		.NumPorts = 2,
		.PortBaseAddr = XAIE2PGBL_PL_MODULE_STREAM_SWITCH_SLAVE_CONFIG_TRACE
	}
};

/*
 * Array of all Mem Tile Stream Switch Master Config registers
 * The data structure contains number of ports and the register offsets
 */
static const  XAie_StrmPort Aie2PMemTileStrmMstr[SS_PORT_TYPE_MAX] =
{
	{	/* Core */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* DMA */
		.NumPorts = 6,
		.PortBaseAddr = XAIE2PGBL_MEM_TILE_MODULE_STREAM_SWITCH_MASTER_CONFIG_DMA0,
	},
	{	/* Ctrl */
		.NumPorts = 1,
		.PortBaseAddr = XAIE2PGBL_MEM_TILE_MODULE_STREAM_SWITCH_MASTER_CONFIG_TILE_CTRL,
	},
	{	/* Fifo */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* South */
		.NumPorts = 4,
		.PortBaseAddr = XAIE2PGBL_MEM_TILE_MODULE_STREAM_SWITCH_MASTER_CONFIG_SOUTH0,
	},
	{	/* West */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* North */
		.NumPorts = 6,
		.PortBaseAddr = XAIE2PGBL_MEM_TILE_MODULE_STREAM_SWITCH_MASTER_CONFIG_NORTH0,
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
static const  XAie_StrmPort Aie2PMemTileStrmSlv[SS_PORT_TYPE_MAX] =
{
	{	/* Core */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* DMA */
		.NumPorts = 6,
		.PortBaseAddr = XAIE2PGBL_MEM_TILE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_DMA_0,
	},
	{	/* Ctrl */
		.NumPorts = 1,
		.PortBaseAddr = XAIE2PGBL_MEM_TILE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_TILE_CTRL,
	},
	{	/* Fifo */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* South */
		.NumPorts = 6,
		.PortBaseAddr = XAIE2PGBL_MEM_TILE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_SOUTH_0,
	},
	{	/* West */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* North */
		.NumPorts = 4,
		.PortBaseAddr = XAIE2PGBL_MEM_TILE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_NORTH_0,
	},
	{	/* East */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* Trace */
		.NumPorts = 1,
		.PortBaseAddr = XAIE2PGBL_MEM_TILE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_TRACE
	}
};

/*
 * Array of all Shim NOC/PL Stream Switch Slave Slot Config registers of AIE2P.
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
		.PortBaseAddr = XAIE2PGBL_PL_MODULE_STREAM_SWITCH_SLAVE_TILE_CTRL_SLOT0,
	},
	{	/* Fifo */
		.NumPorts = 1,
		.PortBaseAddr = XAIE2PGBL_PL_MODULE_STREAM_SWITCH_SLAVE_FIFO_0_SLOT0,
	},
	{	/* South */
		.NumPorts = 8,
		.PortBaseAddr = XAIE2PGBL_PL_MODULE_STREAM_SWITCH_SLAVE_SOUTH_0_SLOT0,
	},
	{	/* West */
		.NumPorts = 4,
		.PortBaseAddr = XAIE2PGBL_PL_MODULE_STREAM_SWITCH_SLAVE_WEST_0_SLOT0,
	},
	{	/* North */
		.NumPorts = 4,
		.PortBaseAddr = XAIE2PGBL_PL_MODULE_STREAM_SWITCH_SLAVE_NORTH_0_SLOT0,
	},
	{	/* East */
		.NumPorts = 4,
		.PortBaseAddr = XAIE2PGBL_PL_MODULE_STREAM_SWITCH_SLAVE_EAST_0_SLOT0,
	},
	{	/* Trace */
		.NumPorts = 1,
		.PortBaseAddr = XAIE2PGBL_PL_MODULE_STREAM_SWITCH_SLAVE_TRACE_SLOT0
	}
};

/*
 * Array of all AIE2P Tile Stream Switch Slave Slot Config registers.
 * The data structure contains number of ports and the register offsets
 */
static const  XAie_StrmPort Aie2PTileStrmSlaveSlot[SS_PORT_TYPE_MAX] =
{
	{	/* Core */
		.NumPorts = 1,
		.PortBaseAddr = XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0,
	},
	{	/* DMA */
		.NumPorts = 2,
		.PortBaseAddr = XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_DMA_0_SLOT0,
	},
	{	/* Ctrl */
		.NumPorts = 1,
		.PortBaseAddr = XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_TILE_CTRL_SLOT0,
	},
	{	/* Fifo */
		.NumPorts = 1,
		.PortBaseAddr = XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_FIFO_0_SLOT0,
	},
	{	/* South */
		.NumPorts = 6,
		.PortBaseAddr = XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_SOUTH_0_SLOT0,
	},
	{	/* West */
		.NumPorts = 4,
		.PortBaseAddr = XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_WEST_0_SLOT0,
	},
	{	/* North */
		.NumPorts = 4,
		.PortBaseAddr = XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_NORTH_0_SLOT0,
	},
	{	/* East */
		.NumPorts = 4,
		.PortBaseAddr = XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_EAST_0_SLOT0,
	},
	{	/* Trace */
		.NumPorts = 1,
		.PortBaseAddr = XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_TRACE_SLOT0
	}
};

/*
 * Array of all AIE2P Mem Tile Stream Switch Slave Slot Config registers
 * The data structure contains number of ports and the register offsets
 */
static const  XAie_StrmPort Aie2PMemTileStrmSlaveSlot[SS_PORT_TYPE_MAX] =
{
	{	/* Core */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* DMA */
		.NumPorts = 6,
		.PortBaseAddr = XAIE2PGBL_MEM_TILE_MODULE_STREAM_SWITCH_SLAVE_DMA_0_SLOT0,
	},
	{	/* Ctrl */
		.NumPorts = 1,
		.PortBaseAddr = XAIE2PGBL_MEM_TILE_MODULE_STREAM_SWITCH_SLAVE_TILE_CTRL_SLOT0,
	},
	{	/* Fifo */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* South */
		.NumPorts = 6,
		.PortBaseAddr = XAIE2PGBL_MEM_TILE_MODULE_STREAM_SWITCH_SLAVE_SOUTH_0_SLOT0,
	},
	{	/* West */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* North */
		.NumPorts = 4,
		.PortBaseAddr = XAIE2PGBL_MEM_TILE_MODULE_STREAM_SWITCH_SLAVE_NORTH_0_SLOT0,
	},
	{	/* East */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* Trace */
		.NumPorts = 1,
		.PortBaseAddr = XAIE2PGBL_MEM_TILE_MODULE_STREAM_SWITCH_SLAVE_TRACE_SLOT0,
	}
};

static const XAie_StrmSwPortMap Aie2PTileStrmSwMasterPortMap[] =
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

static const XAie_StrmSwPortMap Aie2PTileStrmSwSlavePortMap[] =
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

static const XAie_StrmSwPortMap Aie2PMemTileStrmSwMasterPortMap[] =
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

static const XAie_StrmSwPortMap Aie2PMemTileStrmSwSlavePortMap[] =
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
 * AIE2P Tiles.
 */
static const XAie_StrmSwDetMerge Aie2PAieTileStrmSwDetMerge = {
	.NumArbitors = 2U,
	.NumPositions = 4U,
	.ArbConfigOffset = 0x10,
	.ConfigBase = XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1,
	.EnableBase = XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_CTRL,
	.SlvId0.Lsb = XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_SLAVE_ID_0_LSB,
	.SlvId0.Mask = XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_SLAVE_ID_0_MASK,
	.SlvId1.Lsb = XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_SLAVE_ID_1_LSB,
	.SlvId1.Mask = XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_SLAVE_ID_1_MASK,
	.PktCount0.Lsb = XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_PACKET_COUNT_0_LSB,
	.PktCount0.Mask = XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_PACKET_COUNT_0_MASK,
	.PktCount1.Lsb = XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_PACKET_COUNT_1_LSB,
	.PktCount1.Mask = XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_PACKET_COUNT_1_MASK,
	.Enable.Lsb = XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_CTRL_ENABLE_LSB,
	.Enable.Mask = XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_CTRL_ENABLE_MASK,
};

/*
 * Data structure to capture stream switch deterministic merge properties for
 * AIE2P Mem Tiles.
 */
static const XAie_StrmSwDetMerge Aie2PMemTileStrmSwDetMerge = {
	.NumArbitors = 2U,
	.NumPositions = 4U,
	.ArbConfigOffset = 0x10,
	.ConfigBase = XAIE2PGBL_MEM_TILE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1,
	.EnableBase = XAIE2PGBL_MEM_TILE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_CTRL,
	.SlvId0.Lsb = XAIE2PGBL_MEM_TILE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_SLAVE_ID_0_LSB,
	.SlvId0.Mask = XAIE2PGBL_MEM_TILE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_SLAVE_ID_0_MASK,
	.SlvId1.Lsb = XAIE2PGBL_MEM_TILE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_SLAVE_ID_1_LSB,
	.SlvId1.Mask = XAIE2PGBL_MEM_TILE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_SLAVE_ID_1_MASK,
	.PktCount0.Lsb = XAIE2PGBL_MEM_TILE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_PACKET_COUNT_0_LSB,
	.PktCount0.Mask = XAIE2PGBL_MEM_TILE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_PACKET_COUNT_0_MASK,
	.PktCount1.Lsb = XAIE2PGBL_MEM_TILE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_PACKET_COUNT_1_LSB,
	.PktCount1.Mask = XAIE2PGBL_MEM_TILE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_PACKET_COUNT_1_MASK,
	.Enable.Lsb = XAIE2PGBL_MEM_TILE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_CTRL_ENABLE_LSB,
	.Enable.Mask = XAIE2PGBL_MEM_TILE_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_CTRL_ENABLE_MASK,
};

/*
 * Data structure to capture stream switch deterministic merge properties for
 * AIE2P SHIM PL Tiles.
 */
static const XAie_StrmSwDetMerge Aie2PShimTileStrmSwDetMerge = {
	.NumArbitors = 2U,
	.NumPositions = 4U,
	.ArbConfigOffset = 0x10,
	.ConfigBase = XAIE2PGBL_PL_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1,
	.EnableBase = XAIE2PGBL_PL_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_CTRL,
	.SlvId0.Lsb = XAIE2PGBL_PL_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_SLAVE_ID_0_LSB,
	.SlvId0.Mask = XAIE2PGBL_PL_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_SLAVE_ID_0_MASK,
	.SlvId1.Lsb = XAIE2PGBL_PL_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_SLAVE_ID_1_LSB,
	.SlvId1.Mask = XAIE2PGBL_PL_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_SLAVE_ID_1_MASK,
	.PktCount0.Lsb = XAIE2PGBL_PL_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_PACKET_COUNT_0_LSB,
	.PktCount0.Mask = XAIE2PGBL_PL_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_PACKET_COUNT_0_MASK,
	.PktCount1.Lsb = XAIE2PGBL_PL_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_PACKET_COUNT_1_LSB,
	.PktCount1.Mask = XAIE2PGBL_PL_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_SLAVE0_1_PACKET_COUNT_1_MASK,
	.Enable.Lsb = XAIE2PGBL_PL_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_CTRL_ENABLE_LSB,
	.Enable.Mask = XAIE2PGBL_PL_MODULE_STREAM_SWITCH_DETERMINISTIC_MERGE_ARB0_CTRL_ENABLE_MASK,
};

/*
 * Data structure to capture all stream configs for XAIEGBL_TILE_TYPE_AIETILE
 */
static const  XAie_StrmMod Aie2PTileStrmSw =
{
	.SlvConfigBaseAddr = XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_AIE_CORE0,
	.MstrConfigBaseAddr = XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0,
	.PortOffset = 0x4,
	.NumSlaveSlots = 4U,
	.SlotOffsetPerPort = 0x10,
	.SlotOffset = 0x4,
	.DetMergeFeature = XAIE_FEATURE_AVAILABLE,
	.MstrEn = {XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_MASTER_ENABLE_LSB, XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_MASTER_ENABLE_MASK},
	.MstrPktEn = {XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_PACKET_ENABLE_LSB, XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_PACKET_ENABLE_MASK},
	.DrpHdr = {XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_DROP_HEADER_LSB, XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_DROP_HEADER_MASK},
	.Config = {XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_CONFIGURATION_LSB, XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_CONFIGURATION_MASK},
	.SlvEn = {XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_AIE_CORE0_SLAVE_ENABLE_LSB, XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_AIE_CORE0_SLAVE_ENABLE_MASK},
	.SlvPktEn = {XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_AIE_CORE0_PACKET_ENABLE_LSB, XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_AIE_CORE0_PACKET_ENABLE_MASK},
	.SlotPktId = {XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ID_LSB, XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ID_MASK},
	.SlotMask = {XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_MASK_LSB, XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_MASK_MASK},
	.SlotEn = {XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ENABLE_LSB, XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ENABLE_MASK},
	.SlotMsel = {XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_MSEL_LSB, XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_MSEL_MASK},
	.SlotArbitor = {XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ARBIT_LSB, XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ARBIT_MASK},
	.MstrConfig = Aie2PTileStrmMstr,
	.SlvConfig = Aie2PTileStrmSlv,
	.SlvSlotConfig = Aie2PTileStrmSlaveSlot,
	.MaxMasterPhyPortId = 22U,
	.MaxSlavePhyPortId = 24U,
	.MasterPortMap = Aie2PTileStrmSwMasterPortMap,
	.SlavePortMap = Aie2PTileStrmSwSlavePortMap,
	.DetMerge = &Aie2PAieTileStrmSwDetMerge,
	.PortVerify = _XAieMl_AieTile_StrmSwCheckPortValidity,
};

/*
 * Data structure to capture all stream configs for XAIEGBL_TILE_TYPE_SHIMNOC/PL
 */
static const  XAie_StrmMod Aie2PShimStrmSw =
{
	.SlvConfigBaseAddr = XAIE2PGBL_PL_MODULE_STREAM_SWITCH_SLAVE_CONFIG_TILE_CTRL,
	.MstrConfigBaseAddr = XAIE2PGBL_PL_MODULE_STREAM_SWITCH_MASTER_CONFIG_TILE_CTRL,
	.PortOffset = 0x4,
	.NumSlaveSlots = 4U,
	.SlotOffsetPerPort = 0x10,
	.SlotOffset = 0x4,
	.DetMergeFeature = XAIE_FEATURE_AVAILABLE,
	.MstrEn = {XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_MASTER_ENABLE_LSB, XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_MASTER_ENABLE_MASK},
	.MstrPktEn = {XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_PACKET_ENABLE_LSB, XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_PACKET_ENABLE_MASK},
	.DrpHdr = {XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_DROP_HEADER_LSB, XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_DROP_HEADER_MASK},
	.Config = {XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_CONFIGURATION_LSB, XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_CONFIGURATION_MASK},
	.SlvEn = {XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_AIE_CORE0_SLAVE_ENABLE_LSB, XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_AIE_CORE0_SLAVE_ENABLE_MASK},
	.SlvPktEn = {XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_AIE_CORE0_PACKET_ENABLE_LSB, XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_AIE_CORE0_PACKET_ENABLE_MASK},
	.SlotPktId = {XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ID_LSB, XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ID_MASK},
	.SlotMask = {XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_MASK_LSB, XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_MASK_MASK},
	.SlotEn = {XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ENABLE_LSB, XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ENABLE_MASK},
	.SlotMsel = {XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_MSEL_LSB, XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_MSEL_MASK},
	.SlotArbitor = {XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ARBIT_LSB, XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ARBIT_MASK},
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
static const  XAie_StrmMod Aie2PMemTileStrmSw =
{
	.SlvConfigBaseAddr = XAIE2PGBL_MEM_TILE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_DMA_0,
	.MstrConfigBaseAddr = XAIE2PGBL_MEM_TILE_MODULE_STREAM_SWITCH_MASTER_CONFIG_DMA0,
	.PortOffset = 0x4,
	.NumSlaveSlots = 4U,
	.SlotOffsetPerPort = 0x10,
	.SlotOffset = 0x4,
	.DetMergeFeature = XAIE_FEATURE_AVAILABLE,
	.MstrEn = {XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_MASTER_ENABLE_LSB, XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_MASTER_ENABLE_MASK},
	.MstrPktEn = {XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_PACKET_ENABLE_LSB, XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_PACKET_ENABLE_MASK},
	.DrpHdr = {XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_DROP_HEADER_LSB, XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_DROP_HEADER_MASK},
	.Config = {XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_CONFIGURATION_LSB, XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_MASTER_CONFIG_AIE_CORE0_CONFIGURATION_MASK},
	.SlvEn = {XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_AIE_CORE0_SLAVE_ENABLE_LSB, XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_AIE_CORE0_SLAVE_ENABLE_MASK},
	.SlvPktEn = {XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_AIE_CORE0_PACKET_ENABLE_LSB, XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_CONFIG_AIE_CORE0_PACKET_ENABLE_MASK},
	.SlotPktId = {XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ID_LSB, XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ID_MASK},
	.SlotMask = {XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_MASK_LSB, XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_MASK_MASK},
	.SlotEn = {XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ENABLE_LSB, XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ENABLE_MASK},
	.SlotMsel = {XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_MSEL_LSB, XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_MSEL_MASK},
	.SlotArbitor = {XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ARBIT_LSB, XAIE2PGBL_CORE_MODULE_STREAM_SWITCH_SLAVE_AIE_CORE0_SLOT0_ARBIT_MASK},
	.MstrConfig = Aie2PMemTileStrmMstr,
	.SlvConfig = Aie2PMemTileStrmSlv,
	.SlvSlotConfig = Aie2PMemTileStrmSlaveSlot,
	.MaxMasterPhyPortId = 16U,
	.MaxSlavePhyPortId = 17U,
	.MasterPortMap = Aie2PMemTileStrmSwMasterPortMap,
	.SlavePortMap = Aie2PMemTileStrmSwSlavePortMap,
	.DetMerge = &Aie2PMemTileStrmSwDetMerge,
	.PortVerify = _XAieMl_MemTile_StrmSwCheckPortValidity,
};
#endif /* XAIE_FEATURE_SS_ENABLE */

#ifdef XAIE_FEATURE_DMA_ENABLE
static const  XAie_DmaBdEnProp Aie2PMemTileDmaBdEnProp =
{
	.NxtBd.Idx = 1U,
	.NxtBd.Lsb = XAIE2PGBL_MEM_TILE_MODULE_DMA_BD0_1_NEXT_BD_LSB,
	.NxtBd.Mask = XAIE2PGBL_MEM_TILE_MODULE_DMA_BD0_1_NEXT_BD_MASK,
	.UseNxtBd.Idx = 1U,
	.UseNxtBd.Lsb = XAIE2PGBL_MEM_TILE_MODULE_DMA_BD0_1_USE_NEXT_BD_LSB,
	.UseNxtBd.Mask = XAIE2PGBL_MEM_TILE_MODULE_DMA_BD0_1_USE_NEXT_BD_MASK,
	.ValidBd.Idx = 7U,
	.ValidBd.Lsb = XAIE2PGBL_MEM_TILE_MODULE_DMA_BD0_7_VALID_BD_LSB,
	.ValidBd.Mask = XAIE2PGBL_MEM_TILE_MODULE_DMA_BD0_7_VALID_BD_MASK,
	.OutofOrderBdId.Idx = 0U,
	.OutofOrderBdId.Lsb = XAIE2PGBL_MEM_TILE_MODULE_DMA_BD0_0_OUT_OF_ORDER_BD_ID_LSB,
	.OutofOrderBdId.Mask = XAIE2PGBL_MEM_TILE_MODULE_DMA_BD0_0_OUT_OF_ORDER_BD_ID_MASK,
	.TlastSuppress.Idx = 2U,
	.TlastSuppress.Lsb = XAIE2PGBL_MEM_TILE_MODULE_DMA_BD0_2_TLAST_SUPPRESS_LSB,
	.TlastSuppress.Mask = XAIE2PGBL_MEM_TILE_MODULE_DMA_BD0_2_TLAST_SUPPRESS_MASK,
};

static const  XAie_DmaBdPkt Aie2PMemTileDmaBdPktProp =
{
	.EnPkt.Idx = 0U,
	.EnPkt.Lsb = XAIE2PGBL_MEM_TILE_MODULE_DMA_BD0_0_ENABLE_PACKET_LSB,
	.EnPkt.Mask = XAIE2PGBL_MEM_TILE_MODULE_DMA_BD0_0_ENABLE_PACKET_MASK,
	.PktId.Idx = 0U,
	.PktId.Lsb = XAIE2PGBL_MEM_TILE_MODULE_DMA_BD0_0_PACKET_ID_LSB,
	.PktId.Mask = XAIE2PGBL_MEM_TILE_MODULE_DMA_BD0_0_PACKET_ID_MASK,
	.PktType.Idx = 0U,
	.PktType.Lsb = XAIE2PGBL_MEM_TILE_MODULE_DMA_BD0_0_PACKET_TYPE_LSB,
	.PktType.Mask = XAIE2PGBL_MEM_TILE_MODULE_DMA_BD0_0_PACKET_TYPE_MASK,
};

static const  XAie_DmaBdLock Aie2PMemTileDmaLockProp =
{
	.AieMlDmaLock.LckRelVal.Idx = 7U,
	.AieMlDmaLock.LckRelVal.Lsb = XAIE2PGBL_MEM_TILE_MODULE_DMA_BD0_7_LOCK_REL_VALUE_LSB,
	.AieMlDmaLock.LckRelVal.Mask = XAIE2PGBL_MEM_TILE_MODULE_DMA_BD0_7_LOCK_REL_VALUE_MASK,
	.AieMlDmaLock.LckRelId.Idx = 7U,
	.AieMlDmaLock.LckRelId.Lsb = XAIE2PGBL_MEM_TILE_MODULE_DMA_BD0_7_LOCK_REL_ID_LSB,
	.AieMlDmaLock.LckRelId.Mask = XAIE2PGBL_MEM_TILE_MODULE_DMA_BD0_7_LOCK_REL_ID_MASK,
	.AieMlDmaLock.LckAcqEn.Idx = 7U,
	.AieMlDmaLock.LckAcqEn.Lsb = XAIE2PGBL_MEM_TILE_MODULE_DMA_BD0_7_LOCK_ACQ_ENABLE_LSB,
	.AieMlDmaLock.LckAcqEn.Mask = XAIE2PGBL_MEM_TILE_MODULE_DMA_BD0_7_LOCK_ACQ_ENABLE_MASK,
	.AieMlDmaLock.LckAcqVal.Idx = 7U,
	.AieMlDmaLock.LckAcqVal.Lsb = XAIE2PGBL_MEM_TILE_MODULE_DMA_BD0_7_LOCK_ACQ_VALUE_LSB,
	.AieMlDmaLock.LckAcqVal.Mask = XAIE2PGBL_MEM_TILE_MODULE_DMA_BD0_7_LOCK_ACQ_VALUE_MASK,
	.AieMlDmaLock.LckAcqId.Idx = 7U,
	.AieMlDmaLock.LckAcqId.Lsb = XAIE2PGBL_MEM_TILE_MODULE_DMA_BD0_7_LOCK_ACQ_ID_LSB,
	.AieMlDmaLock.LckAcqId.Mask = XAIE2PGBL_MEM_TILE_MODULE_DMA_BD0_7_LOCK_ACQ_ID_MASK,
};

static const  XAie_DmaBdBuffer Aie2PMemTileBufferProp =
{
	.TileDmaBuff.BaseAddr.Idx = 1U,
	.TileDmaBuff.BaseAddr.Lsb = XAIE2PGBL_MEM_TILE_MODULE_DMA_BD0_1_BASE_ADDRESS_LSB,
	.TileDmaBuff.BaseAddr.Mask = XAIE2PGBL_MEM_TILE_MODULE_DMA_BD0_1_BASE_ADDRESS_MASK,
	.TileDmaBuff.BufferLen.Idx = 0U,
	.TileDmaBuff.BufferLen.Lsb = XAIE2PGBL_MEM_TILE_MODULE_DMA_BD0_0_BUFFER_LENGTH_LSB,
	.TileDmaBuff.BufferLen.Mask = XAIE2PGBL_MEM_TILE_MODULE_DMA_BD0_0_BUFFER_LENGTH_MASK,
};

static const XAie_DmaBdDoubleBuffer Aie2PMemTileDoubleBufferProp =
{
	.EnDoubleBuff = {0U},
	.BaseAddr_B = {0U},
	.FifoMode = {0U},
	.EnIntrleaved = {0U},
	.IntrleaveCnt = {0U},
	.BuffSelect = {0U},
};

static const  XAie_DmaBdMultiDimAddr Aie2PMemTileMultiDimProp =
{
	.AieMlMultiDimAddr.DmaDimProp[1U].StepSize.Idx = 3U,
	.AieMlMultiDimAddr.DmaDimProp[1U].StepSize.Lsb = XAIE2PGBL_MEM_TILE_MODULE_DMA_BD0_3_D1_STEPSIZE_LSB,
	.AieMlMultiDimAddr.DmaDimProp[1U].StepSize.Mask = XAIE2PGBL_MEM_TILE_MODULE_DMA_BD0_3_D1_STEPSIZE_MASK,
	.AieMlMultiDimAddr.DmaDimProp[0U].StepSize.Idx = 2U,
	.AieMlMultiDimAddr.DmaDimProp[0U].StepSize.Lsb = XAIE2PGBL_MEM_TILE_MODULE_DMA_BD0_2_D0_STEPSIZE_LSB,
	.AieMlMultiDimAddr.DmaDimProp[0U].StepSize.Mask = XAIE2PGBL_MEM_TILE_MODULE_DMA_BD0_2_D0_STEPSIZE_MASK,
	.AieMlMultiDimAddr.DmaDimProp[1U].Wrap.Idx = 3U,
	.AieMlMultiDimAddr.DmaDimProp[1U].Wrap.Lsb = XAIE2PGBL_MEM_TILE_MODULE_DMA_BD0_3_D1_WRAP_LSB,
	.AieMlMultiDimAddr.DmaDimProp[1U].Wrap.Mask = XAIE2PGBL_MEM_TILE_MODULE_DMA_BD0_3_D1_WRAP_MASK,
	.AieMlMultiDimAddr.DmaDimProp[0U].Wrap.Idx = 2U,
	.AieMlMultiDimAddr.DmaDimProp[0U].Wrap.Lsb = XAIE2PGBL_MEM_TILE_MODULE_DMA_BD0_2_D0_WRAP_LSB,
	.AieMlMultiDimAddr.DmaDimProp[0U].Wrap.Mask = XAIE2PGBL_MEM_TILE_MODULE_DMA_BD0_2_D0_WRAP_MASK,
	.AieMlMultiDimAddr.DmaDimProp[2U].StepSize.Idx = 4U,
	.AieMlMultiDimAddr.DmaDimProp[2U].StepSize.Lsb = XAIE2PGBL_MEM_TILE_MODULE_DMA_BD0_4_D2_STEPSIZE_LSB,
	.AieMlMultiDimAddr.DmaDimProp[2U].StepSize.Mask = XAIE2PGBL_MEM_TILE_MODULE_DMA_BD0_4_D2_STEPSIZE_MASK,
	.AieMlMultiDimAddr.DmaDimProp[2U].Wrap.Idx = 4U,
	.AieMlMultiDimAddr.DmaDimProp[2U].Wrap.Lsb = XAIE2PGBL_MEM_TILE_MODULE_DMA_BD0_4_D2_WRAP_LSB,
	.AieMlMultiDimAddr.DmaDimProp[2U].Wrap.Mask = XAIE2PGBL_MEM_TILE_MODULE_DMA_BD0_4_D2_WRAP_MASK,
	.AieMlMultiDimAddr.IterCurr.Idx = 6U,
	.AieMlMultiDimAddr.IterCurr.Lsb = XAIE2PGBL_MEM_TILE_MODULE_DMA_BD0_6_ITERATION_CURRENT_LSB,
	.AieMlMultiDimAddr.IterCurr.Mask = XAIE2PGBL_MEM_TILE_MODULE_DMA_BD0_6_ITERATION_CURRENT_MASK,
	.AieMlMultiDimAddr.Iter.Wrap.Idx = 6U,
	.AieMlMultiDimAddr.Iter.Wrap.Lsb = XAIE2PGBL_MEM_TILE_MODULE_DMA_BD0_6_ITERATION_WRAP_LSB,
	.AieMlMultiDimAddr.Iter.Wrap.Mask = XAIE2PGBL_MEM_TILE_MODULE_DMA_BD0_6_ITERATION_WRAP_MASK,
	.AieMlMultiDimAddr.Iter.StepSize.Idx = 6U,
	.AieMlMultiDimAddr.Iter.StepSize.Lsb = XAIE2PGBL_MEM_TILE_MODULE_DMA_BD0_6_ITERATION_STEPSIZE_LSB,
	.AieMlMultiDimAddr.Iter.StepSize.Mask = XAIE2PGBL_MEM_TILE_MODULE_DMA_BD0_6_ITERATION_STEPSIZE_MASK,
	.AieMlMultiDimAddr.DmaDimProp[3U].StepSize.Idx = 5U,
	.AieMlMultiDimAddr.DmaDimProp[3U].StepSize.Lsb = XAIE2PGBL_MEM_TILE_MODULE_DMA_BD0_5_D3_STEPSIZE_LSB,
	.AieMlMultiDimAddr.DmaDimProp[3U].StepSize.Mask = XAIE2PGBL_MEM_TILE_MODULE_DMA_BD0_5_D3_STEPSIZE_MASK,
};

static const  XAie_DmaBdZeroPad Aie2PMemTileZeroPadProp =
{
	.D0_ZeroBefore.Idx = 1U,
	.D0_ZeroBefore.Lsb = XAIE2PGBL_MEM_TILE_MODULE_DMA_BD0_1_D0_PAD_BEFORE_LSB,
	.D0_ZeroBefore.Mask = XAIE2PGBL_MEM_TILE_MODULE_DMA_BD0_1_D0_PAD_BEFORE_MASK,
	.D1_ZeroBefore.Idx = 3U,
	.D1_ZeroBefore.Lsb = XAIE2PGBL_MEM_TILE_MODULE_DMA_BD0_3_D1_PAD_BEFORE_LSB,
	.D1_ZeroBefore.Mask = XAIE2PGBL_MEM_TILE_MODULE_DMA_BD0_3_D1_PAD_BEFORE_MASK,
	.D2_ZeroBefore.Idx = 4U,
	.D2_ZeroBefore.Lsb = XAIE2PGBL_MEM_TILE_MODULE_DMA_BD0_4_D2_PAD_BEFORE_LSB,
	.D2_ZeroBefore.Mask = XAIE2PGBL_MEM_TILE_MODULE_DMA_BD0_4_D2_PAD_BEFORE_MASK,
	.D0_ZeroAfter.Idx = 5U,
	.D0_ZeroAfter.Lsb = XAIE2PGBL_MEM_TILE_MODULE_DMA_BD0_5_D0_PAD_AFTER_LSB,
	.D0_ZeroAfter.Mask = XAIE2PGBL_MEM_TILE_MODULE_DMA_BD0_5_D0_PAD_AFTER_MASK,
	.D1_ZeroAfter.Idx = 5U,
	.D1_ZeroAfter.Lsb = XAIE2PGBL_MEM_TILE_MODULE_DMA_BD0_5_D1_PAD_AFTER_LSB,
	.D1_ZeroAfter.Mask = XAIE2PGBL_MEM_TILE_MODULE_DMA_BD0_5_D1_PAD_AFTER_MASK,
	.D2_ZeroAfter.Idx = 5U,
	.D2_ZeroAfter.Lsb = XAIE2PGBL_MEM_TILE_MODULE_DMA_BD0_5_D2_PAD_AFTER_LSB,
	.D2_ZeroAfter.Mask = XAIE2PGBL_MEM_TILE_MODULE_DMA_BD0_5_D2_PAD_AFTER_MASK,
};

static const  XAie_DmaBdCompression Aie2PMemTileCompressionProp =
{
	.EnCompression.Idx = 4U,
	.EnCompression.Lsb = XAIE2PGBL_MEM_TILE_MODULE_DMA_BD0_4_ENABLE_COMPRESSION_LSB,
	.EnCompression.Mask = XAIE2PGBL_MEM_TILE_MODULE_DMA_BD0_4_ENABLE_COMPRESSION_MASK,
};

/* Data structure to capture register offsets and masks for Mem Tile Dma */
static const  XAie_DmaBdProp Aie2PMemTileDmaProp =
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
	.Buffer = &Aie2PMemTileBufferProp,
	.DoubleBuffer = &Aie2PMemTileDoubleBufferProp,
	.Lock = &Aie2PMemTileDmaLockProp,
	.Pkt = &Aie2PMemTileDmaBdPktProp,
	.BdEn = &Aie2PMemTileDmaBdEnProp,
	.AddrMode = &Aie2PMemTileMultiDimProp,
	.ZeroPad = &Aie2PMemTileZeroPadProp,
	.Compression = &Aie2PMemTileCompressionProp,
	.SysProp = NULL
};

static const XAie_DmaChStatus Aie2PMemTileDmaChStatus =
{
	/* This database is common for mm2s and s2mm channels */
	.AieMlDmaChStatus.Status.Lsb = XAIE2PGBL_MEM_TILE_MODULE_DMA_S2MM_STATUS_0_STATUS_LSB,
	.AieMlDmaChStatus.Status.Mask = XAIE2PGBL_MEM_TILE_MODULE_DMA_S2MM_STATUS_0_STATUS_MASK,
	.AieMlDmaChStatus.TaskQSize.Lsb = XAIE2PGBL_MEM_TILE_MODULE_DMA_S2MM_STATUS_0_TASK_QUEUE_SIZE_LSB,
	.AieMlDmaChStatus.TaskQSize.Mask = XAIE2PGBL_MEM_TILE_MODULE_DMA_S2MM_STATUS_0_TASK_QUEUE_SIZE_MASK,
	.AieMlDmaChStatus.StalledLockAcq.Lsb = XAIE2PGBL_MEM_TILE_MODULE_DMA_S2MM_STATUS_0_STALLED_LOCK_ACQ_LSB,
	.AieMlDmaChStatus.StalledLockAcq.Mask = XAIE2PGBL_MEM_TILE_MODULE_DMA_S2MM_STATUS_0_STALLED_LOCK_ACQ_MASK,
	.AieMlDmaChStatus.StalledLockRel.Lsb = XAIE2PGBL_MEM_TILE_MODULE_DMA_S2MM_STATUS_0_STALLED_LOCK_REL_LSB,
	.AieMlDmaChStatus.StalledLockRel.Mask = XAIE2PGBL_MEM_TILE_MODULE_DMA_S2MM_STATUS_0_STALLED_LOCK_REL_MASK,
	.AieMlDmaChStatus.StalledStreamStarve.Lsb = XAIE2PGBL_MEM_TILE_MODULE_DMA_S2MM_STATUS_0_STALLED_STREAM_STARVATION_LSB,
	.AieMlDmaChStatus.StalledStreamStarve.Mask = XAIE2PGBL_MEM_TILE_MODULE_DMA_S2MM_STATUS_0_STALLED_STREAM_STARVATION_MASK,
	.AieMlDmaChStatus.StalledTCT.Lsb = XAIE2PGBL_MEM_TILE_MODULE_DMA_S2MM_STATUS_0_STALLED_TCT_OR_COUNT_FIFO_FULL_LSB,
	.AieMlDmaChStatus.StalledTCT.Mask = XAIE2PGBL_MEM_TILE_MODULE_DMA_S2MM_STATUS_0_STALLED_TCT_OR_COUNT_FIFO_FULL_MASK,
};

static const  XAie_DmaChProp Aie2PMemTileDmaChProp =
{
	.HasFoTMode = XAIE_FEATURE_AVAILABLE,
	.HasControllerId = XAIE_FEATURE_AVAILABLE,
	.HasEnCompression = XAIE_FEATURE_AVAILABLE,
	.HasEnOutOfOrder = XAIE_FEATURE_AVAILABLE,
	.MaxFoTMode = DMA_FoT_COUNTS_FROM_MM_REG,
	.MaxRepeatCount = 256U,
	.ControllerId.Idx = 0,
	.ControllerId.Lsb = XAIE2PGBL_MEM_TILE_MODULE_DMA_S2MM_0_CTRL_CONTROLLER_ID_LSB,
	.ControllerId.Mask =XAIE2PGBL_MEM_TILE_MODULE_DMA_S2MM_0_CTRL_CONTROLLER_ID_MASK ,
	.EnCompression.Idx = 0,
	.EnCompression.Lsb = XAIE2PGBL_MEM_TILE_MODULE_DMA_S2MM_0_CTRL_DECOMPRESSION_ENABLE_LSB,
	.EnCompression.Mask = XAIE2PGBL_MEM_TILE_MODULE_DMA_S2MM_0_CTRL_DECOMPRESSION_ENABLE_MASK,
	.EnOutofOrder.Idx = 0,
	.EnOutofOrder.Lsb = XAIE2PGBL_MEM_TILE_MODULE_DMA_S2MM_0_CTRL_ENABLE_OUT_OF_ORDER_LSB,
	.EnOutofOrder.Mask = XAIE2PGBL_MEM_TILE_MODULE_DMA_S2MM_0_CTRL_ENABLE_OUT_OF_ORDER_MASK,
	.FoTMode.Idx = 0,
	.FoTMode.Lsb = XAIE2PGBL_MEM_TILE_MODULE_DMA_S2MM_0_CTRL_FOT_MODE_LSB,
	.FoTMode.Mask = XAIE2PGBL_MEM_TILE_MODULE_DMA_S2MM_0_CTRL_FOT_MODE_MASK ,
	.Reset.Idx = 0,
	.Reset.Lsb = XAIE2PGBL_MEM_TILE_MODULE_DMA_S2MM_0_CTRL_RESET_LSB,
	.Reset.Mask = XAIE2PGBL_MEM_TILE_MODULE_DMA_S2MM_0_CTRL_RESET_MASK,
	.EnToken.Idx = 1,
	.EnToken.Lsb = XAIE2PGBL_MEM_TILE_MODULE_DMA_S2MM_0_START_QUEUE_ENABLE_TOKEN_ISSUE_LSB,
	.EnToken.Mask = XAIE2PGBL_MEM_TILE_MODULE_DMA_S2MM_0_START_QUEUE_ENABLE_TOKEN_ISSUE_MASK,
	.RptCount.Idx = 1,
	.RptCount.Lsb = XAIE2PGBL_MEM_TILE_MODULE_DMA_S2MM_0_START_QUEUE_REPEAT_COUNT_LSB,
	.RptCount.Mask = XAIE2PGBL_MEM_TILE_MODULE_DMA_S2MM_0_START_QUEUE_REPEAT_COUNT_MASK,
	.StartBd.Idx = 1,
	.StartBd.Lsb = XAIE2PGBL_MEM_TILE_MODULE_DMA_S2MM_0_START_QUEUE_START_BD_ID_LSB,
	.StartBd.Mask = XAIE2PGBL_MEM_TILE_MODULE_DMA_S2MM_0_START_QUEUE_START_BD_ID_MASK,
	.PauseStream = {0U},
	.PauseMem = {0U},
	.Enable = {0U},
	.StartQSizeMax = 4U,
	.DmaChStatus = &Aie2PMemTileDmaChStatus,
};

/* Mem Tile Dma Module */
static const  XAie_DmaMod Aie2PMemTileDmaMod =
{
	.BaseAddr = XAIE2PGBL_MEM_TILE_MODULE_DMA_BD0_0,
	.IdxOffset = 0x20,  /* This is the offset between each BD */
	.NumBds = 48,	   /* Number of BDs for AIE2P Tile DMA */
	.NumLocks = 192U,
	.NumAddrDim = 4U,
	.DoubleBuffering = XAIE_FEATURE_UNAVAILABLE,
	.Compression = XAIE_FEATURE_AVAILABLE,
	.ZeroPadding = XAIE_FEATURE_AVAILABLE,
	.OutofOrderBdId = XAIE_FEATURE_AVAILABLE,
	.InterleaveMode = XAIE_FEATURE_UNAVAILABLE,
	.FifoMode = XAIE_FEATURE_UNAVAILABLE,
	.EnTokenIssue = XAIE_FEATURE_AVAILABLE,
	.RepeatCount = XAIE_FEATURE_AVAILABLE,
	.TlastSuppress = XAIE_FEATURE_AVAILABLE,
	.StartQueueBase = XAIE2PGBL_MEM_TILE_MODULE_DMA_S2MM_0_START_QUEUE,
	.ChCtrlBase = XAIE2PGBL_MEM_TILE_MODULE_DMA_S2MM_0_CTRL,
	.NumChannels = 6,  /* number of s2mm/mm2s channels */
	.ChIdxOffset = 0x8,  /* This is the offset between each channel */
	.ChStatusBase = XAIE2PGBL_MEM_TILE_MODULE_DMA_S2MM_STATUS_0,
	.ChStatusOffset = 0x20,
	.BdProp = &Aie2PMemTileDmaProp,
	.ChProp = &Aie2PMemTileDmaChProp,
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

static const  XAie_DmaBdEnProp Aie2PTileDmaBdEnProp =
{
	.NxtBd.Idx = 5U,
	.NxtBd.Lsb = XAIE2PGBL_MEMORY_MODULE_DMA_BD0_5_NEXT_BD_LSB,
	.NxtBd.Mask = XAIE2PGBL_MEMORY_MODULE_DMA_BD0_5_NEXT_BD_MASK,
	.UseNxtBd.Idx = 5U,
	.UseNxtBd.Lsb = XAIE2PGBL_MEMORY_MODULE_DMA_BD0_5_USE_NEXT_BD_LSB,
	.UseNxtBd.Mask = XAIE2PGBL_MEMORY_MODULE_DMA_BD0_5_USE_NEXT_BD_MASK,
	.ValidBd.Idx = 5U,
	.ValidBd.Lsb = XAIE2PGBL_MEMORY_MODULE_DMA_BD0_5_VALID_BD_LSB,
	.ValidBd.Mask = XAIE2PGBL_MEMORY_MODULE_DMA_BD0_5_VALID_BD_MASK,
	.OutofOrderBdId.Idx = 1U,
	.OutofOrderBdId.Lsb = XAIE2PGBL_MEMORY_MODULE_DMA_BD0_1_OUT_OF_ORDER_BD_ID_LSB,
	.OutofOrderBdId.Mask = XAIE2PGBL_MEMORY_MODULE_DMA_BD0_1_OUT_OF_ORDER_BD_ID_MASK,
	.TlastSuppress.Idx = 5U,
	.TlastSuppress.Lsb = XAIE2PGBL_MEMORY_MODULE_DMA_BD0_5_TLAST_SUPPRESS_LSB,
	.TlastSuppress.Mask = XAIE2PGBL_MEMORY_MODULE_DMA_BD0_5_TLAST_SUPPRESS_MASK,
};

static const  XAie_DmaBdPkt Aie2PTileDmaBdPktProp =
{
	.EnPkt.Idx = 1U,
	.EnPkt.Lsb = XAIE2PGBL_MEMORY_MODULE_DMA_BD0_1_ENABLE_PACKET_LSB,
	.EnPkt.Mask = XAIE2PGBL_MEMORY_MODULE_DMA_BD0_1_ENABLE_PACKET_MASK,
	.PktId.Idx = 1U,
	.PktId.Lsb = XAIE2PGBL_MEMORY_MODULE_DMA_BD0_1_PACKET_ID_LSB,
	.PktId.Mask = XAIE2PGBL_MEMORY_MODULE_DMA_BD0_1_PACKET_ID_MASK,
	.PktType.Idx = 1U,
	.PktType.Lsb = XAIE2PGBL_MEMORY_MODULE_DMA_BD0_1_PACKET_TYPE_LSB,
	.PktType.Mask = XAIE2PGBL_MEMORY_MODULE_DMA_BD0_1_PACKET_TYPE_MASK,
};

static const  XAie_DmaBdLock Aie2PTileDmaLockProp =
{
	.AieMlDmaLock.LckRelVal.Idx = 5U,
	.AieMlDmaLock.LckRelVal.Lsb = XAIE2PGBL_MEMORY_MODULE_DMA_BD0_5_LOCK_REL_VALUE_LSB,
	.AieMlDmaLock.LckRelVal.Mask = XAIE2PGBL_MEMORY_MODULE_DMA_BD0_5_LOCK_REL_VALUE_MASK,
	.AieMlDmaLock.LckRelId.Idx = 5U,
	.AieMlDmaLock.LckRelId.Lsb = XAIE2PGBL_MEMORY_MODULE_DMA_BD0_5_LOCK_REL_ID_LSB,
	.AieMlDmaLock.LckRelId.Mask = XAIE2PGBL_MEMORY_MODULE_DMA_BD0_5_LOCK_REL_ID_MASK,
	.AieMlDmaLock.LckAcqEn.Idx = 5U,
	.AieMlDmaLock.LckAcqEn.Lsb = XAIE2PGBL_MEMORY_MODULE_DMA_BD0_5_LOCK_ACQ_ENABLE_LSB,
	.AieMlDmaLock.LckAcqEn.Mask = XAIE2PGBL_MEMORY_MODULE_DMA_BD0_5_LOCK_ACQ_ENABLE_MASK,
	.AieMlDmaLock.LckAcqVal.Idx = 5U,
	.AieMlDmaLock.LckAcqVal.Lsb = XAIE2PGBL_MEMORY_MODULE_DMA_BD0_5_LOCK_ACQ_VALUE_LSB,
	.AieMlDmaLock.LckAcqVal.Mask = XAIE2PGBL_MEMORY_MODULE_DMA_BD0_5_LOCK_ACQ_VALUE_MASK,
	.AieMlDmaLock.LckAcqId.Idx = 5U,
	.AieMlDmaLock.LckAcqId.Lsb = XAIE2PGBL_MEMORY_MODULE_DMA_BD0_5_LOCK_ACQ_ID_LSB,
	.AieMlDmaLock.LckAcqId.Mask = XAIE2PGBL_MEMORY_MODULE_DMA_BD0_5_LOCK_ACQ_ID_MASK,
};

static const  XAie_DmaBdBuffer Aie2PTileDmaBufferProp =
{
	.TileDmaBuff.BaseAddr.Idx = 0U,
	.TileDmaBuff.BaseAddr.Lsb = XAIE2PGBL_MEMORY_MODULE_DMA_BD0_0_BASE_ADDRESS_LSB,
	.TileDmaBuff.BaseAddr.Mask = XAIE2PGBL_MEMORY_MODULE_DMA_BD0_0_BASE_ADDRESS_MASK,
	.TileDmaBuff.BufferLen.Idx = 0U,
	.TileDmaBuff.BufferLen.Lsb = XAIE2PGBL_MEMORY_MODULE_DMA_BD0_0_BUFFER_LENGTH_LSB,
	.TileDmaBuff.BufferLen.Mask = XAIE2PGBL_MEMORY_MODULE_DMA_BD0_0_BUFFER_LENGTH_MASK,
};

static const XAie_DmaBdDoubleBuffer Aie2PTileDmaDoubleBufferProp =
{
	.EnDoubleBuff = {0U},
	.BaseAddr_B = {0U},
	.FifoMode = {0U},
	.EnIntrleaved = {0U},
	.IntrleaveCnt = {0U},
	.BuffSelect = {0U},
};

static const  XAie_DmaBdMultiDimAddr Aie2PTileDmaMultiDimProp =
{
	.AieMlMultiDimAddr.DmaDimProp[0U].StepSize.Idx = 2U,
	.AieMlMultiDimAddr.DmaDimProp[0U].StepSize.Lsb = XAIE2PGBL_MEMORY_MODULE_DMA_BD0_2_D0_STEPSIZE_LSB,
	.AieMlMultiDimAddr.DmaDimProp[0U].StepSize.Mask = XAIE2PGBL_MEMORY_MODULE_DMA_BD0_2_D0_STEPSIZE_MASK,
	.AieMlMultiDimAddr.DmaDimProp[0U].Wrap.Idx = 3U,
	.AieMlMultiDimAddr.DmaDimProp[0U].Wrap.Lsb = XAIE2PGBL_MEMORY_MODULE_DMA_BD0_3_D0_WRAP_LSB,
	.AieMlMultiDimAddr.DmaDimProp[0U].Wrap.Mask = XAIE2PGBL_MEMORY_MODULE_DMA_BD0_3_D0_WRAP_MASK,
	.AieMlMultiDimAddr.DmaDimProp[1U].StepSize.Idx = 2U,
	.AieMlMultiDimAddr.DmaDimProp[1U].StepSize.Lsb = XAIE2PGBL_MEMORY_MODULE_DMA_BD0_2_D1_STEPSIZE_LSB,
	.AieMlMultiDimAddr.DmaDimProp[1U].StepSize.Mask = XAIE2PGBL_MEMORY_MODULE_DMA_BD0_2_D1_STEPSIZE_MASK,
	.AieMlMultiDimAddr.DmaDimProp[1U].Wrap.Idx = 3U,
	.AieMlMultiDimAddr.DmaDimProp[1U].Wrap.Lsb = XAIE2PGBL_MEMORY_MODULE_DMA_BD0_3_D1_WRAP_LSB,
	.AieMlMultiDimAddr.DmaDimProp[1U].Wrap.Mask = XAIE2PGBL_MEMORY_MODULE_DMA_BD0_3_D1_WRAP_MASK,
	.AieMlMultiDimAddr.DmaDimProp[2U].StepSize.Idx = 3U,
	.AieMlMultiDimAddr.DmaDimProp[2U].StepSize.Lsb = XAIE2PGBL_MEMORY_MODULE_DMA_BD0_3_D2_STEPSIZE_LSB,
	.AieMlMultiDimAddr.DmaDimProp[2U].StepSize.Mask = XAIE2PGBL_MEMORY_MODULE_DMA_BD0_3_D2_STEPSIZE_MASK,
	.AieMlMultiDimAddr.IterCurr.Idx = 4U,
	.AieMlMultiDimAddr.IterCurr.Lsb = XAIE2PGBL_MEMORY_MODULE_DMA_BD0_4_ITERATION_CURRENT_LSB,
	.AieMlMultiDimAddr.IterCurr.Mask = XAIE2PGBL_MEMORY_MODULE_DMA_BD0_4_ITERATION_CURRENT_MASK,
	.AieMlMultiDimAddr.Iter.Wrap.Idx = 4U,
	.AieMlMultiDimAddr.Iter.Wrap.Lsb = XAIE2PGBL_MEMORY_MODULE_DMA_BD0_4_ITERATION_WRAP_LSB,
	.AieMlMultiDimAddr.Iter.Wrap.Mask = XAIE2PGBL_MEMORY_MODULE_DMA_BD0_4_ITERATION_WRAP_MASK,
	.AieMlMultiDimAddr.Iter.StepSize.Idx = 4U,
	.AieMlMultiDimAddr.Iter.StepSize.Lsb = XAIE2PGBL_MEMORY_MODULE_DMA_BD0_4_ITERATION_STEPSIZE_LSB,
	.AieMlMultiDimAddr.Iter.StepSize.Mask = XAIE2PGBL_MEMORY_MODULE_DMA_BD0_4_ITERATION_STEPSIZE_MASK,
	.AieMlMultiDimAddr.DmaDimProp[2U].Wrap = {0U},
	.AieMlMultiDimAddr.DmaDimProp[3U].Wrap = {0U},
	.AieMlMultiDimAddr.DmaDimProp[3U].StepSize = {0U}
};

static const  XAie_DmaBdCompression Aie2PTileDmaCompressionProp =
{
	.EnCompression.Idx = 1U,
	.EnCompression.Lsb = XAIE2PGBL_MEMORY_MODULE_DMA_BD0_1_ENABLE_COMPRESSION_LSB,
	.EnCompression.Mask = XAIE2PGBL_MEMORY_MODULE_DMA_BD0_1_ENABLE_COMPRESSION_MASK,
};

/* Data structure to capture register offsets and masks for Tile Dma */
static const  XAie_DmaBdProp Aie2PTileDmaProp =
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
	.Buffer = &Aie2PTileDmaBufferProp,
	.DoubleBuffer = &Aie2PTileDmaDoubleBufferProp,
	.Lock = &Aie2PTileDmaLockProp,
	.Pkt = &Aie2PTileDmaBdPktProp,
	.BdEn = &Aie2PTileDmaBdEnProp,
	.AddrMode = &Aie2PTileDmaMultiDimProp,
	.ZeroPad = NULL,
	.Compression = &Aie2PTileDmaCompressionProp,
	.SysProp = NULL
};

static const XAie_DmaChStatus Aie2PTileDmaChStatus =
{
	/* This database is common for mm2s and s2mm channels */
	.AieMlDmaChStatus.Status.Lsb = XAIE2PGBL_MEMORY_MODULE_DMA_S2MM_STATUS_0_STATUS_LSB,
	.AieMlDmaChStatus.Status.Mask = XAIE2PGBL_MEMORY_MODULE_DMA_S2MM_STATUS_0_STATUS_MASK,
	.AieMlDmaChStatus.TaskQSize.Lsb = XAIE2PGBL_MEMORY_MODULE_DMA_S2MM_STATUS_0_TASK_QUEUE_SIZE_LSB,
	.AieMlDmaChStatus.TaskQSize.Mask = XAIE2PGBL_MEMORY_MODULE_DMA_S2MM_STATUS_0_TASK_QUEUE_SIZE_MASK,
	.AieMlDmaChStatus.StalledLockAcq.Lsb = XAIE2PGBL_MEMORY_MODULE_DMA_S2MM_STATUS_0_STALLED_LOCK_ACQ_LSB,
	.AieMlDmaChStatus.StalledLockAcq.Mask = XAIE2PGBL_MEMORY_MODULE_DMA_S2MM_STATUS_0_STALLED_LOCK_ACQ_MASK,
	.AieMlDmaChStatus.StalledLockRel.Lsb = XAIE2PGBL_MEMORY_MODULE_DMA_S2MM_STATUS_0_STALLED_LOCK_REL_LSB,
	.AieMlDmaChStatus.StalledLockRel.Mask = XAIE2PGBL_MEMORY_MODULE_DMA_S2MM_STATUS_0_STALLED_LOCK_REL_MASK,
	.AieMlDmaChStatus.StalledStreamStarve.Lsb = XAIE2PGBL_MEMORY_MODULE_DMA_S2MM_STATUS_0_STALLED_STREAM_STARVATION_LSB,
	.AieMlDmaChStatus.StalledStreamStarve.Mask = XAIE2PGBL_MEMORY_MODULE_DMA_S2MM_STATUS_0_STALLED_STREAM_STARVATION_MASK,
	.AieMlDmaChStatus.StalledTCT.Lsb = XAIE2PGBL_MEMORY_MODULE_DMA_S2MM_STATUS_0_STALLED_TCT_OR_COUNT_FIFO_FULL_LSB,
	.AieMlDmaChStatus.StalledTCT.Mask = XAIE2PGBL_MEMORY_MODULE_DMA_S2MM_STATUS_0_STALLED_TCT_OR_COUNT_FIFO_FULL_MASK,
};

/* Data structure to capture register offsets and masks for Mem Tile and
 * Tile Dma Channels
 */
static const  XAie_DmaChProp Aie2PDmaChProp =
{
	.HasFoTMode = XAIE_FEATURE_AVAILABLE,
	.HasControllerId = XAIE_FEATURE_AVAILABLE,
	.HasEnCompression = XAIE_FEATURE_AVAILABLE,
	.HasEnOutOfOrder = XAIE_FEATURE_AVAILABLE,
	.MaxFoTMode = DMA_FoT_COUNTS_FROM_MM_REG,
	.MaxRepeatCount = 256U,
	.ControllerId.Idx = 0,
	.ControllerId.Lsb = XAIE2PGBL_MEMORY_MODULE_DMA_S2MM_0_CTRL_CONTROLLER_ID_LSB,
	.ControllerId.Mask =XAIE2PGBL_MEMORY_MODULE_DMA_S2MM_0_CTRL_CONTROLLER_ID_MASK ,
	.EnCompression.Idx = 0,
	.EnCompression.Lsb = XAIE2PGBL_MEMORY_MODULE_DMA_S2MM_0_CTRL_DECOMPRESSION_ENABLE_LSB,
	.EnCompression.Mask = XAIE2PGBL_MEMORY_MODULE_DMA_S2MM_0_CTRL_DECOMPRESSION_ENABLE_MASK,
	.EnOutofOrder.Idx = 0,
	.EnOutofOrder.Lsb = XAIE2PGBL_MEMORY_MODULE_DMA_S2MM_0_CTRL_ENABLE_OUT_OF_ORDER_LSB,
	.EnOutofOrder.Mask = XAIE2PGBL_MEMORY_MODULE_DMA_S2MM_0_CTRL_ENABLE_OUT_OF_ORDER_MASK,
	.FoTMode.Idx = 0,
	.FoTMode.Lsb = XAIE2PGBL_MEMORY_MODULE_DMA_S2MM_0_CTRL_FOT_MODE_LSB,
	.FoTMode.Mask = XAIE2PGBL_MEMORY_MODULE_DMA_S2MM_0_CTRL_FOT_MODE_MASK ,
	.Reset.Idx = 0,
	.Reset.Lsb = XAIE2PGBL_MEMORY_MODULE_DMA_S2MM_0_CTRL_RESET_LSB,
	.Reset.Mask = XAIE2PGBL_MEMORY_MODULE_DMA_S2MM_0_CTRL_RESET_MASK,
	.EnToken.Idx = 1,
	.EnToken.Lsb = XAIE2PGBL_MEMORY_MODULE_DMA_S2MM_0_START_QUEUE_ENABLE_TOKEN_ISSUE_LSB,
	.EnToken.Mask = XAIE2PGBL_MEMORY_MODULE_DMA_S2MM_0_START_QUEUE_ENABLE_TOKEN_ISSUE_MASK,
	.RptCount.Idx = 1,
	.RptCount.Lsb = XAIE2PGBL_MEMORY_MODULE_DMA_S2MM_0_START_QUEUE_REPEAT_COUNT_LSB,
	.RptCount.Mask = XAIE2PGBL_MEMORY_MODULE_DMA_S2MM_0_START_QUEUE_REPEAT_COUNT_MASK,
	.StartBd.Idx = 1,
	.StartBd.Lsb = XAIE2PGBL_MEMORY_MODULE_DMA_S2MM_0_START_QUEUE_START_BD_ID_LSB,
	.StartBd.Mask = XAIE2PGBL_MEMORY_MODULE_DMA_S2MM_0_START_QUEUE_START_BD_ID_MASK,
	.PauseStream = {0U},
	.PauseMem = {0U},
	.Enable = {0U},
	.StartQSizeMax = 4U,
	.DmaChStatus = &Aie2PTileDmaChStatus,
};

/* Tile Dma Module */
static const  XAie_DmaMod Aie2PTileDmaMod =
{
	.BaseAddr = XAIE2PGBL_MEMORY_MODULE_DMA_BD0_0,
	.IdxOffset = 0x20,  	/* This is the offset between each BD */
	.NumBds = 16U,	   	/* Number of BDs for AIE2P Tile DMA */
	.NumLocks = 16U,
	.NumAddrDim = 3U,
	.DoubleBuffering = XAIE_FEATURE_UNAVAILABLE,
	.Compression = XAIE_FEATURE_AVAILABLE,
	.ZeroPadding = XAIE_FEATURE_UNAVAILABLE,
	.OutofOrderBdId = XAIE_FEATURE_AVAILABLE,
	.InterleaveMode = XAIE_FEATURE_UNAVAILABLE,
	.FifoMode = XAIE_FEATURE_UNAVAILABLE,
	.EnTokenIssue = XAIE_FEATURE_AVAILABLE,
	.RepeatCount = XAIE_FEATURE_AVAILABLE,
	.TlastSuppress = XAIE_FEATURE_AVAILABLE,
	.StartQueueBase = XAIE2PGBL_MEMORY_MODULE_DMA_S2MM_0_START_QUEUE,
	.ChCtrlBase = XAIE2PGBL_MEMORY_MODULE_DMA_S2MM_0_CTRL,
	.NumChannels = 2U,  /* Number of s2mm/mm2s channels */
	.ChIdxOffset = 0x8,  /* This is the offset between each channel */
	.ChStatusBase = XAIE2PGBL_MEMORY_MODULE_DMA_S2MM_STATUS_0,
	.ChStatusOffset = 0x10,
	.BdProp = &Aie2PTileDmaProp,
	.ChProp = &Aie2PDmaChProp,
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

static const  XAie_DmaBdEnProp Aie2PShimDmaBdEnProp =
{
	.NxtBd.Idx = 7U,
	.NxtBd.Lsb = XAIE2PGBL_NOC_MODULE_DMA_BD0_7_NEXT_BD_LSB,
	.NxtBd.Mask = XAIE2PGBL_NOC_MODULE_DMA_BD0_7_NEXT_BD_MASK,
	.UseNxtBd.Idx = 7U,
	.UseNxtBd.Lsb = XAIE2PGBL_NOC_MODULE_DMA_BD0_7_USE_NEXT_BD_LSB,
	.UseNxtBd.Mask = XAIE2PGBL_NOC_MODULE_DMA_BD0_7_USE_NEXT_BD_MASK,
	.ValidBd.Idx = 7U,
	.ValidBd.Lsb = XAIE2PGBL_NOC_MODULE_DMA_BD0_7_VALID_BD_LSB,
	.ValidBd.Mask = XAIE2PGBL_NOC_MODULE_DMA_BD0_7_VALID_BD_MASK,
	.OutofOrderBdId.Idx = 2U,
	.OutofOrderBdId.Lsb = XAIE2PGBL_NOC_MODULE_DMA_BD0_2_OUT_OF_ORDER_BD_ID_LSB,
	.OutofOrderBdId.Mask = XAIE2PGBL_NOC_MODULE_DMA_BD0_2_OUT_OF_ORDER_BD_ID_MASK,
	.TlastSuppress.Idx = 7U,
	.TlastSuppress.Lsb = XAIE2PGBL_NOC_MODULE_DMA_BD0_7_TLAST_SUPPRESS_LSB,
	.TlastSuppress.Mask = XAIE2PGBL_NOC_MODULE_DMA_BD0_7_TLAST_SUPPRESS_MASK,
};

static const  XAie_DmaBdPkt Aie2PShimDmaBdPktProp =
{
	.EnPkt.Idx = 2U,
	.EnPkt.Lsb = XAIE2PGBL_NOC_MODULE_DMA_BD0_2_ENABLE_PACKET_LSB,
	.EnPkt.Mask = XAIE2PGBL_NOC_MODULE_DMA_BD0_2_ENABLE_PACKET_MASK,
	.PktId.Idx = 2U,
	.PktId.Lsb = XAIE2PGBL_NOC_MODULE_DMA_BD0_2_PACKET_ID_LSB,
	.PktId.Mask = XAIE2PGBL_NOC_MODULE_DMA_BD0_2_PACKET_ID_MASK,
	.PktType.Idx = 2U,
	.PktType.Lsb = XAIE2PGBL_NOC_MODULE_DMA_BD0_2_PACKET_TYPE_LSB,
	.PktType.Mask = XAIE2PGBL_NOC_MODULE_DMA_BD0_2_PACKET_TYPE_MASK,
};

static const  XAie_DmaBdLock Aie2PShimDmaLockProp =
{
	.AieMlDmaLock.LckRelVal.Idx = 7U,
	.AieMlDmaLock.LckRelVal.Lsb = XAIE2PGBL_NOC_MODULE_DMA_BD0_7_LOCK_REL_VALUE_LSB,
	.AieMlDmaLock.LckRelVal.Mask = XAIE2PGBL_NOC_MODULE_DMA_BD0_7_LOCK_REL_VALUE_MASK,
	.AieMlDmaLock.LckRelId.Idx = 7U,
	.AieMlDmaLock.LckRelId.Lsb = XAIE2PGBL_NOC_MODULE_DMA_BD0_7_LOCK_REL_ID_LSB,
	.AieMlDmaLock.LckRelId.Mask = XAIE2PGBL_NOC_MODULE_DMA_BD0_7_LOCK_REL_ID_MASK,
	.AieMlDmaLock.LckAcqEn.Idx = 7U,
	.AieMlDmaLock.LckAcqEn.Lsb = XAIE2PGBL_NOC_MODULE_DMA_BD0_7_LOCK_ACQ_ENABLE_LSB,
	.AieMlDmaLock.LckAcqEn.Mask = XAIE2PGBL_NOC_MODULE_DMA_BD0_7_LOCK_ACQ_ENABLE_MASK,
	.AieMlDmaLock.LckAcqVal.Idx = 7U,
	.AieMlDmaLock.LckAcqVal.Lsb = XAIE2PGBL_NOC_MODULE_DMA_BD0_7_LOCK_ACQ_VALUE_LSB,
	.AieMlDmaLock.LckAcqVal.Mask = XAIE2PGBL_NOC_MODULE_DMA_BD0_7_LOCK_ACQ_VALUE_MASK,
	.AieMlDmaLock.LckAcqId.Idx = 7U,
	.AieMlDmaLock.LckAcqId.Lsb = XAIE2PGBL_NOC_MODULE_DMA_BD0_7_LOCK_ACQ_ID_LSB,
	.AieMlDmaLock.LckAcqId.Mask = XAIE2PGBL_NOC_MODULE_DMA_BD0_7_LOCK_ACQ_ID_MASK,
};

static const  XAie_DmaBdBuffer Aie2PShimDmaBufferProp =
{
	.ShimDmaBuff.AddrLow.Idx = 1U,
	.ShimDmaBuff.AddrLow.Lsb = XAIE2PGBL_NOC_MODULE_DMA_BD0_1_BASE_ADDRESS_LOW_LSB,
	.ShimDmaBuff.AddrLow.Mask = XAIE2PGBL_NOC_MODULE_DMA_BD0_1_BASE_ADDRESS_LOW_MASK,
	.ShimDmaBuff.AddrHigh.Idx = 2U,
	.ShimDmaBuff.AddrHigh.Lsb = XAIE2PGBL_NOC_MODULE_DMA_BD0_2_BASE_ADDRESS_HIGH_LSB,
	.ShimDmaBuff.AddrHigh.Mask = XAIE2PGBL_NOC_MODULE_DMA_BD0_2_BASE_ADDRESS_HIGH_MASK,
	.ShimDmaBuff.BufferLen.Idx = 0U,
	.ShimDmaBuff.BufferLen.Lsb = XAIE2PGBL_NOC_MODULE_DMA_BD0_0_BUFFER_LENGTH_LSB,
	.ShimDmaBuff.BufferLen.Mask = XAIE2PGBL_NOC_MODULE_DMA_BD0_0_BUFFER_LENGTH_MASK,
};

static const  XAie_DmaBdDoubleBuffer Aie2PShimDmaDoubleBufferProp =
{
	.EnDoubleBuff = {0U},
	.BaseAddr_B = {0U},
	.FifoMode = {0U},
	.EnIntrleaved = {0U},
	.IntrleaveCnt = {0U},
	.BuffSelect = {0U},
};

static const  XAie_DmaBdMultiDimAddr Aie2PShimDmaMultiDimProp =
{
	.AieMlMultiDimAddr.DmaDimProp[0U].StepSize.Idx = 3U,
	.AieMlMultiDimAddr.DmaDimProp[0U].StepSize.Lsb = XAIE2PGBL_NOC_MODULE_DMA_BD0_3_D0_STEPSIZE_LSB,
	.AieMlMultiDimAddr.DmaDimProp[0U].StepSize.Mask = XAIE2PGBL_NOC_MODULE_DMA_BD0_3_D0_STEPSIZE_MASK,
	.AieMlMultiDimAddr.DmaDimProp[0U].Wrap.Idx = 3U,
	.AieMlMultiDimAddr.DmaDimProp[0U].Wrap.Lsb = XAIE2PGBL_NOC_MODULE_DMA_BD0_3_D0_WRAP_LSB,
	.AieMlMultiDimAddr.DmaDimProp[0U].Wrap.Mask = XAIE2PGBL_NOC_MODULE_DMA_BD0_3_D0_WRAP_MASK,
	.AieMlMultiDimAddr.DmaDimProp[1U].StepSize.Idx =3U ,
	.AieMlMultiDimAddr.DmaDimProp[1U].StepSize.Lsb = XAIE2PGBL_NOC_MODULE_DMA_BD0_4_D1_STEPSIZE_LSB,
	.AieMlMultiDimAddr.DmaDimProp[1U].StepSize.Mask = XAIE2PGBL_NOC_MODULE_DMA_BD0_4_D1_STEPSIZE_MASK,
	.AieMlMultiDimAddr.DmaDimProp[1U].Wrap.Idx = 3U,
	.AieMlMultiDimAddr.DmaDimProp[1U].Wrap.Lsb = XAIE2PGBL_NOC_MODULE_DMA_BD0_4_D1_WRAP_LSB,
	.AieMlMultiDimAddr.DmaDimProp[1U].Wrap.Mask = XAIE2PGBL_NOC_MODULE_DMA_BD0_4_D1_WRAP_MASK,
	.AieMlMultiDimAddr.DmaDimProp[2U].StepSize.Idx = 5U,
	.AieMlMultiDimAddr.DmaDimProp[2U].StepSize.Lsb = XAIE2PGBL_NOC_MODULE_DMA_BD0_5_D2_STEPSIZE_LSB,
	.AieMlMultiDimAddr.DmaDimProp[2U].StepSize.Mask = XAIE2PGBL_NOC_MODULE_DMA_BD0_5_D2_STEPSIZE_MASK,
	.AieMlMultiDimAddr.IterCurr.Idx = 6U,
	.AieMlMultiDimAddr.IterCurr.Lsb = XAIE2PGBL_NOC_MODULE_DMA_BD0_6_ITERATION_CURRENT_LSB,
	.AieMlMultiDimAddr.IterCurr.Mask = XAIE2PGBL_NOC_MODULE_DMA_BD0_6_ITERATION_CURRENT_MASK,
	.AieMlMultiDimAddr.Iter.Wrap.Idx = 6U,
	.AieMlMultiDimAddr.Iter.Wrap.Lsb = XAIE2PGBL_NOC_MODULE_DMA_BD0_6_ITERATION_WRAP_LSB,
	.AieMlMultiDimAddr.Iter.Wrap.Mask = XAIE2PGBL_NOC_MODULE_DMA_BD0_6_ITERATION_WRAP_MASK,
	.AieMlMultiDimAddr.Iter.StepSize.Idx = 6U,
	.AieMlMultiDimAddr.Iter.StepSize.Lsb = XAIE2PGBL_NOC_MODULE_DMA_BD0_6_ITERATION_STEPSIZE_LSB,
	.AieMlMultiDimAddr.Iter.StepSize.Mask = XAIE2PGBL_NOC_MODULE_DMA_BD0_6_ITERATION_STEPSIZE_MASK,
	.AieMlMultiDimAddr.DmaDimProp[2U].Wrap = {0U},
	.AieMlMultiDimAddr.DmaDimProp[3U].Wrap = {0U},
	.AieMlMultiDimAddr.DmaDimProp[3U].StepSize = {0U}
};

static const  XAie_DmaSysProp Aie2PShimDmaSysProp =
{
	.SMID.Idx = 5U,
	.SMID.Lsb = XAIE2PGBL_NOC_MODULE_DMA_BD0_5_SMID_LSB,
	.SMID.Mask = XAIE2PGBL_NOC_MODULE_DMA_BD0_5_SMID_MASK,
	.BurstLen.Idx = 4U,
	.BurstLen.Lsb = XAIE2PGBL_NOC_MODULE_DMA_BD0_4_BURST_LENGTH_LSB,
	.BurstLen.Mask = XAIE2PGBL_NOC_MODULE_DMA_BD0_4_BURST_LENGTH_MASK,
	.AxQos.Idx = 5U,
	.AxQos.Lsb = XAIE2PGBL_NOC_MODULE_DMA_BD0_5_AXQOS_LSB,
	.AxQos.Mask = XAIE2PGBL_NOC_MODULE_DMA_BD0_5_AXQOS_MASK,
	.SecureAccess.Idx = 3U,
	.SecureAccess.Lsb = XAIE2PGBL_NOC_MODULE_DMA_BD0_3_SECURE_ACCESS_LSB,
	.SecureAccess.Mask = XAIE2PGBL_NOC_MODULE_DMA_BD0_3_SECURE_ACCESS_MASK,
	.AxCache.Idx = 5U,
	.AxCache.Lsb = XAIE2PGBL_NOC_MODULE_DMA_BD0_5_AXCACHE_LSB,
	.AxCache.Mask = XAIE2PGBL_NOC_MODULE_DMA_BD0_5_AXCACHE_MASK,
};

/* Data structure to capture register offsets and masks for Tile Dma */
static const  XAie_DmaBdProp Aie2PShimDmaProp =
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
	.Buffer = &Aie2PShimDmaBufferProp,
	.DoubleBuffer = &Aie2PShimDmaDoubleBufferProp,
	.Lock = &Aie2PShimDmaLockProp,
	.Pkt = &Aie2PShimDmaBdPktProp,
	.BdEn = &Aie2PShimDmaBdEnProp,
	.AddrMode = &Aie2PShimDmaMultiDimProp,
	.ZeroPad = NULL,
	.Compression = NULL,
	.SysProp = &Aie2PShimDmaSysProp
};

static const XAie_DmaChStatus Aie2PShimDmaChStatus =
{
	/* This database is common for mm2s and s2mm channels */
	.AieMlDmaChStatus.Status.Lsb = XAIE2PGBL_NOC_MODULE_DMA_S2MM_STATUS_0_STATUS_LSB,
	.AieMlDmaChStatus.Status.Mask = XAIE2PGBL_NOC_MODULE_DMA_S2MM_STATUS_0_STATUS_MASK,
	.AieMlDmaChStatus.TaskQSize.Lsb = XAIE2PGBL_NOC_MODULE_DMA_S2MM_STATUS_0_TASK_QUEUE_SIZE_LSB,
	.AieMlDmaChStatus.TaskQSize.Mask = XAIE2PGBL_NOC_MODULE_DMA_S2MM_STATUS_0_TASK_QUEUE_SIZE_MASK,
	.AieMlDmaChStatus.StalledLockAcq.Lsb = XAIE2PGBL_NOC_MODULE_DMA_S2MM_STATUS_0_STALLED_LOCK_ACQ_LSB,
	.AieMlDmaChStatus.StalledLockAcq.Mask = XAIE2PGBL_NOC_MODULE_DMA_S2MM_STATUS_0_STALLED_LOCK_ACQ_MASK,
	.AieMlDmaChStatus.StalledLockRel.Lsb = XAIE2PGBL_NOC_MODULE_DMA_S2MM_STATUS_0_STALLED_LOCK_REL_LSB,
	.AieMlDmaChStatus.StalledLockRel.Mask = XAIE2PGBL_NOC_MODULE_DMA_S2MM_STATUS_0_STALLED_LOCK_REL_MASK,
	.AieMlDmaChStatus.StalledStreamStarve.Lsb = XAIE2PGBL_NOC_MODULE_DMA_S2MM_STATUS_0_STALLED_STREAM_STARVATION_LSB,
	.AieMlDmaChStatus.StalledStreamStarve.Mask = XAIE2PGBL_NOC_MODULE_DMA_S2MM_STATUS_0_STALLED_STREAM_STARVATION_MASK,
	.AieMlDmaChStatus.StalledTCT.Lsb = XAIE2PGBL_NOC_MODULE_DMA_S2MM_STATUS_0_STALLED_TCT_OR_COUNT_FIFO_FULL_LSB,
	.AieMlDmaChStatus.StalledTCT.Mask = XAIE2PGBL_NOC_MODULE_DMA_S2MM_STATUS_0_STALLED_TCT_OR_COUNT_FIFO_FULL_MASK,
};

/* Data structure to capture register offsets and masks for Mem Tile and
 * Tile Dma Channels
 */
static const  XAie_DmaChProp Aie2PShimDmaChProp =
{
	.HasFoTMode = XAIE_FEATURE_AVAILABLE,
	.HasControllerId = XAIE_FEATURE_AVAILABLE,
	.HasEnCompression = XAIE_FEATURE_AVAILABLE,
	.HasEnOutOfOrder = XAIE_FEATURE_AVAILABLE,
	.MaxFoTMode = DMA_FoT_COUNTS_FROM_MM_REG,
	.MaxRepeatCount = 256U,
	.ControllerId.Idx = 0U,
	.ControllerId.Lsb = XAIE2PGBL_NOC_MODULE_DMA_S2MM_0_CTRL_CONTROLLER_ID_LSB ,
	.ControllerId.Mask = XAIE2PGBL_NOC_MODULE_DMA_S2MM_0_CTRL_CONTROLLER_ID_MASK ,
	.EnCompression.Idx = 0U,
	.EnCompression.Lsb = 0U,
	.EnCompression.Mask = 0U,
	.EnOutofOrder.Idx = 0U,
	.EnOutofOrder.Lsb = XAIE2PGBL_NOC_MODULE_DMA_S2MM_0_CTRL_ENABLE_OUT_OF_ORDER_LSB,
	.EnOutofOrder.Mask = XAIE2PGBL_NOC_MODULE_DMA_S2MM_0_CTRL_ENABLE_OUT_OF_ORDER_MASK,
	.FoTMode.Idx = 0,
	.FoTMode.Lsb = XAIE2PGBL_NOC_MODULE_DMA_S2MM_0_CTRL_FOT_MODE_LSB,
	.FoTMode.Mask = XAIE2PGBL_NOC_MODULE_DMA_S2MM_0_CTRL_FOT_MODE_MASK ,
	.Reset.Idx = 0U,
	.Reset.Lsb = 0U,
	.Reset.Mask = 0U,
	.EnToken.Idx = 1U,
	.EnToken.Lsb = XAIE2PGBL_NOC_MODULE_DMA_S2MM_0_TASK_QUEUE_ENABLE_TOKEN_ISSUE_LSB,
	.EnToken.Mask = XAIE2PGBL_NOC_MODULE_DMA_S2MM_0_TASK_QUEUE_ENABLE_TOKEN_ISSUE_MASK,
	.RptCount.Idx = 1U,
	.RptCount.Lsb = XAIE2PGBL_NOC_MODULE_DMA_S2MM_0_TASK_QUEUE_REPEAT_COUNT_LSB,
	.RptCount.Mask = XAIE2PGBL_NOC_MODULE_DMA_S2MM_0_TASK_QUEUE_REPEAT_COUNT_MASK,
	.StartBd.Idx = 1U,
	.StartBd.Lsb = XAIE2PGBL_NOC_MODULE_DMA_S2MM_0_TASK_QUEUE_START_BD_ID_LSB,
	.StartBd.Mask = XAIE2PGBL_NOC_MODULE_DMA_S2MM_0_TASK_QUEUE_START_BD_ID_MASK,
	.PauseStream = {0U},
	.PauseMem = {0U},
	.Enable = {0U},
	.StartQSizeMax = 4U,
	.DmaChStatus = &Aie2PShimDmaChStatus,
};

/* Tile Dma Module */
static const  XAie_DmaMod Aie2PShimDmaMod =
{
	.BaseAddr = XAIE2PGBL_NOC_MODULE_DMA_BD0_0,
	.IdxOffset = 0x20,  	/* This is the offset between each BD */
	.NumBds = 16U,	   	/* Number of BDs for AIE2P Tile DMA */
	.NumLocks = 16U,
	.NumAddrDim = 3U,
	.DoubleBuffering = XAIE_FEATURE_UNAVAILABLE,
	.Compression = XAIE_FEATURE_UNAVAILABLE,
	.ZeroPadding = XAIE_FEATURE_UNAVAILABLE,
	.OutofOrderBdId = XAIE_FEATURE_AVAILABLE,
	.InterleaveMode = XAIE_FEATURE_UNAVAILABLE,
	.FifoMode = XAIE_FEATURE_UNAVAILABLE,
	.EnTokenIssue = XAIE_FEATURE_AVAILABLE,
	.RepeatCount = XAIE_FEATURE_AVAILABLE,
	.TlastSuppress = XAIE_FEATURE_AVAILABLE,
	.StartQueueBase = XAIE2PGBL_NOC_MODULE_DMA_S2MM_0_TASK_QUEUE,
	.ChCtrlBase = XAIE2PGBL_NOC_MODULE_DMA_S2MM_0_CTRL,
	.NumChannels = 2U,  /* Number of s2mm/mm2s channels */
	.ChIdxOffset = 0x8,  /* This is the offset between each channel */
	.ChStatusBase = XAIE2PGBL_NOC_MODULE_DMA_S2MM_STATUS_0,
	.ChStatusOffset = 0x8,
	.BdProp = &Aie2PShimDmaProp,
	.ChProp = &Aie2PShimDmaChProp,
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
static const  XAie_MemMod Aie2PTileMemMod =
{
	.Size = 0x10000,
	.MemAddr = XAIE2PGBL_MEMORY_MODULE_DATAMEMORY,
	.EccEvntRegOff = XAIE2PGBL_MEMORY_MODULE_ECC_SCRUBBING_EVENT,
};

/* Data Memory Module for Mem Tile data memory*/
static const  XAie_MemMod Aie2PMemTileMemMod =
{
	.Size = 0x80000,
	.MemAddr = XAIE2PGBL_MEM_TILE_MODULE_DATAMEMORY,
	.EccEvntRegOff = XAIE2PGBL_MEM_TILE_MODULE_ECC_SCRUBBING_EVENT,
};
#endif /* XAIE_FEATURE_DATAMEM_ENABLE */

#ifdef XAIE_FEATURE_PL_ENABLE
/* Register field attributes for PL interface down sizer for 32 and 64 bits */
static const  XAie_RegFldAttr Aie2PDownSzr32_64Bit[] =
{
	{XAIE2PGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH0_LSB, XAIE2PGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH0_MASK},
	{XAIE2PGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH1_LSB, XAIE2PGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH1_MASK},
	{XAIE2PGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH2_LSB, XAIE2PGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH2_MASK},
	{XAIE2PGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH3_LSB, XAIE2PGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH3_MASK},
	{XAIE2PGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH4_LSB, XAIE2PGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH4_MASK},
	{XAIE2PGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH5_LSB, XAIE2PGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH5_MASK},
	{XAIE2PGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH6_LSB, XAIE2PGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH6_MASK},
	{XAIE2PGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH7_LSB, XAIE2PGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH7_MASK}
};

/* Register field attributes for PL interface down sizer for 128 bits */
static const  XAie_RegFldAttr Aie2PDownSzr128Bit[] =
{
	{XAIE2PGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH0_SOUTH1_128_COMBINE_LSB, XAIE2PGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH0_SOUTH1_128_COMBINE_MASK},
	{XAIE2PGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH2_SOUTH3_128_COMBINE_LSB, XAIE2PGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH2_SOUTH3_128_COMBINE_MASK},
	{XAIE2PGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH4_SOUTH5_128_COMBINE_LSB, XAIE2PGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH4_SOUTH5_128_COMBINE_MASK},
	{XAIE2PGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH6_SOUTH7_128_COMBINE_LSB, XAIE2PGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG_SOUTH6_SOUTH7_128_COMBINE_MASK}
};

/* Register field attributes for PL interface up sizer */
static const  XAie_RegFldAttr Aie2PUpSzr32_64Bit[] =
{
	{XAIE2PGBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH0_LSB, XAIE2PGBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH0_MASK},
	{XAIE2PGBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH1_LSB, XAIE2PGBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH1_MASK},
	{XAIE2PGBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH2_LSB, XAIE2PGBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH2_MASK},
	{XAIE2PGBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH3_LSB, XAIE2PGBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH3_MASK},
	{XAIE2PGBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH4_LSB, XAIE2PGBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH4_MASK},
	{XAIE2PGBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH5_LSB, XAIE2PGBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH5_MASK}
};

/* Register field attributes for PL interface up sizer for 128 bits */
static const  XAie_RegFldAttr Aie2PUpSzr128Bit[] =
{
	{XAIE2PGBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH0_SOUTH1_128_COMBINE_LSB, XAIE2PGBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH0_SOUTH1_128_COMBINE_MASK},
	{XAIE2PGBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH2_SOUTH3_128_COMBINE_LSB, XAIE2PGBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH2_SOUTH3_128_COMBINE_MASK},
	{XAIE2PGBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH4_SOUTH5_128_COMBINE_LSB, XAIE2PGBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG_SOUTH4_SOUTH5_128_COMBINE_MASK}
};

/* Register field attributes for PL interface down sizer bypass */
static const  XAie_RegFldAttr Aie2PDownSzrByPass[] =
{
	{XAIE2PGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS_SOUTH0_LSB, XAIE2PGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS_SOUTH0_MASK},
	{XAIE2PGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS_SOUTH1_LSB, XAIE2PGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS_SOUTH1_MASK},
	{XAIE2PGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS_SOUTH2_LSB, XAIE2PGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS_SOUTH2_MASK},
	{XAIE2PGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS_SOUTH4_LSB, XAIE2PGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS_SOUTH4_MASK},
	{XAIE2PGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS_SOUTH5_LSB, XAIE2PGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS_SOUTH5_MASK},
	{XAIE2PGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS_SOUTH6_LSB, XAIE2PGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS_SOUTH6_MASK}
};

/* Register field attributes for PL interface down sizer enable */
static const  XAie_RegFldAttr Aie2PDownSzrEnable[] =
{
	{XAIE2PGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH0_LSB, XAIE2PGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH0_MASK},
	{XAIE2PGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH1_LSB, XAIE2PGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH1_MASK},
	{XAIE2PGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH2_LSB, XAIE2PGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH2_MASK},
	{XAIE2PGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH3_LSB, XAIE2PGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH3_MASK},
	{XAIE2PGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH4_LSB, XAIE2PGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH4_MASK},
	{XAIE2PGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH5_LSB, XAIE2PGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH5_MASK},
	{XAIE2PGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH6_LSB, XAIE2PGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH6_MASK},
	{XAIE2PGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH7_LSB, XAIE2PGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE_SOUTH7_MASK}
};

/* Register field attributes for SHIMNOC Mux configuration */
static const  XAie_RegFldAttr Aie2PShimMuxConfig[] =
{
	{XAIE2PGBL_NOC_MODULE_MUX_CONFIG_SOUTH2_LSB, XAIE2PGBL_NOC_MODULE_MUX_CONFIG_SOUTH2_MASK},
	{XAIE2PGBL_NOC_MODULE_MUX_CONFIG_SOUTH3_LSB, XAIE2PGBL_NOC_MODULE_MUX_CONFIG_SOUTH3_MASK},
	{XAIE2PGBL_NOC_MODULE_MUX_CONFIG_SOUTH6_LSB, XAIE2PGBL_NOC_MODULE_MUX_CONFIG_SOUTH6_MASK},
	{XAIE2PGBL_NOC_MODULE_MUX_CONFIG_SOUTH7_LSB, XAIE2PGBL_NOC_MODULE_MUX_CONFIG_SOUTH7_MASK},
};

/* Register field attributes for SHIMNOC DeMux configuration */
static const  XAie_RegFldAttr Aie2PShimDeMuxConfig[] =
{
	{XAIE2PGBL_NOC_MODULE_DEMUX_CONFIG_SOUTH2_LSB, XAIE2PGBL_NOC_MODULE_DEMUX_CONFIG_SOUTH2_MASK},
	{XAIE2PGBL_NOC_MODULE_DEMUX_CONFIG_SOUTH3_LSB, XAIE2PGBL_NOC_MODULE_DEMUX_CONFIG_SOUTH3_MASK},
	{XAIE2PGBL_NOC_MODULE_DEMUX_CONFIG_SOUTH4_LSB, XAIE2PGBL_NOC_MODULE_DEMUX_CONFIG_SOUTH4_MASK},
	{XAIE2PGBL_NOC_MODULE_DEMUX_CONFIG_SOUTH5_LSB, XAIE2PGBL_NOC_MODULE_DEMUX_CONFIG_SOUTH5_MASK}
};

#ifdef XAIE_FEATURE_PRIVILEGED_ENABLE
/* Register to set SHIM clock buffer control */
static const XAie_ShimClkBufCntr Aie2PShimClkBufCntr =
{
	.RegOff = 0xFFF20,
	.RstEnable = XAIE_DISABLE,
	.ClkBufEnable = {0, 0x1}
};

static const XAie_ShimRstMod Aie2PShimTileRst =
{
	.RegOff = 0,
	.RstCntr = {0},
	.RstShims = _XAieMl_RstShims,
};

/* Register feild attributes for Shim AXI MM config for NSU Errors */
static const XAie_ShimNocAxiMMConfig Aie2PShimNocAxiMMConfig =
{
	.RegOff = XAIE2PGBL_NOC_MODULE_ME_AXIMM_CONFIG,
	.NsuSlvErr = {XAIE2PGBL_NOC_MODULE_ME_AXIMM_CONFIG_SLVERR_BLOCK_LSB, XAIE2PGBL_NOC_MODULE_ME_AXIMM_CONFIG_SLVERR_BLOCK_MASK},
	.NsuDecErr = {XAIE2PGBL_NOC_MODULE_ME_AXIMM_CONFIG_DECERR_BLOCK_LSB, XAIE2PGBL_NOC_MODULE_ME_AXIMM_CONFIG_DECERR_BLOCK_MASK}
};
#endif /* XAIE_FEATURE_PRIVILEGED_ENABLE */

/* PL Interface module for SHIMPL Tiles */
static const  XAie_PlIfMod Aie2PPlIfMod =
{
	.UpSzrOff = XAIE2PGBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG,
	.DownSzrOff = XAIE2PGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG,
	.DownSzrEnOff = XAIE2PGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE,
	.DownSzrByPassOff = XAIE2PGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS,
	.ColRstOff = 0xFFF28,
	.NumUpSzrPorts = 0x6,
	.MaxByPassPortNum = 0x6,
	.NumDownSzrPorts = 0x8,
	.UpSzr32_64Bit = Aie2PUpSzr32_64Bit,
	.UpSzr128Bit = Aie2PUpSzr128Bit,
	.DownSzr32_64Bit = Aie2PDownSzr32_64Bit,
	.DownSzr128Bit = Aie2PDownSzr128Bit,
	.DownSzrEn = Aie2PDownSzrEnable,
	.DownSzrByPass = Aie2PDownSzrByPass,
	.ShimNocMuxOff = 0x0,
	.ShimNocDeMuxOff = 0x0,
	.ShimNocMux = NULL,
	.ShimNocDeMux = NULL,
	.ColRst = {0, 0x1},
#ifdef XAIE_FEATURE_PRIVILEGED_ENABLE
	.ClkBufCntr = &Aie2PShimClkBufCntr,
	.ShimTileRst = &Aie2PShimTileRst,
#else
	.ClkBufCntr = NULL,
	.ShimTileRst = NULL,
#endif /* XAIE_FEATURE_PRIVILEGED_ENABLE */
	.ShimNocAxiMM = NULL,
};

/* PL Interface module for SHIMNOC Tiles */
static const  XAie_PlIfMod Aie2PShimTilePlIfMod =
{
	.UpSzrOff = XAIE2PGBL_PL_MODULE_PL_INTERFACE_UPSIZER_CONFIG,
	.DownSzrOff = XAIE2PGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_CONFIG,
	.DownSzrEnOff = XAIE2PGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_ENABLE,
	.DownSzrByPassOff = XAIE2PGBL_PL_MODULE_PL_INTERFACE_DOWNSIZER_BYPASS,
	.ColRstOff = 0xFFF28,
	.NumUpSzrPorts = 0x6,
	.MaxByPassPortNum = 0x6,
	.NumDownSzrPorts = 0x8,
	.UpSzr32_64Bit = Aie2PUpSzr32_64Bit,
	.UpSzr128Bit = Aie2PUpSzr128Bit,
	.DownSzr32_64Bit = Aie2PDownSzr32_64Bit,
	.DownSzr128Bit = Aie2PDownSzr128Bit,
	.DownSzrEn = Aie2PDownSzrEnable,
	.DownSzrByPass = Aie2PDownSzrByPass,
	.ShimNocMuxOff = XAIE2PGBL_NOC_MODULE_MUX_CONFIG,
	.ShimNocDeMuxOff = XAIE2PGBL_NOC_MODULE_DEMUX_CONFIG,
	.ShimNocMux = Aie2PShimMuxConfig,
	.ShimNocDeMux = Aie2PShimDeMuxConfig,
	.ColRst = {0, 0x1},
#ifdef XAIE_FEATURE_PRIVILEGED_ENABLE
	.ClkBufCntr = &Aie2PShimClkBufCntr,
	.ShimTileRst = &Aie2PShimTileRst,
	.ShimNocAxiMM = &Aie2PShimNocAxiMMConfig,
#else
	.ClkBufCntr = NULL,
	.ShimTileRst = NULL,
	.ShimNocAxiMM = NULL,
#endif /* XAIE_FEATURE_PRIVILEGED_ENABLE */
};
#endif /* XAIE_FEATURE_PL_ENABLE */

#ifdef XAIE_FEATURE_LOCK_ENABLE
static const XAie_RegFldAttr Aie2PTileLockInit =
{
	.Lsb = XAIE2PGBL_MEMORY_MODULE_LOCK0_VALUE_LOCK_VALUE_LSB,
	.Mask = XAIE2PGBL_MEMORY_MODULE_LOCK0_VALUE_LOCK_VALUE_MASK,
};

/* Lock Module for AIE Tiles  */
static const  XAie_LockMod Aie2PTileLockMod =
{
	.BaseAddr = XAIE2PGBL_MEMORY_MODULE_LOCK_REQUEST,
	.NumLocks = 16U,
	.LockIdOff = 0x400,
	.RelAcqOff = 0x200,
	.LockValOff = 0x4,
	.LockValUpperBound = 63,
	.LockValLowerBound = -64,
	.LockSetValBase = XAIE2PGBL_MEMORY_MODULE_LOCK0_VALUE,
	.LockSetValOff = 0x10,
	.LockInit = &Aie2PTileLockInit,
	.Acquire = &_XAieMl_LockAcquire,
	.Release = &_XAieMl_LockRelease,
	.SetValue = &_XAieMl_LockSetValue,
};

static const XAie_RegFldAttr Aie2PShimNocLockInit =
{
	.Lsb = XAIE2PGBL_NOC_MODULE_LOCK0_VALUE_LOCK_VALUE_LSB,
	.Mask = XAIE2PGBL_NOC_MODULE_LOCK0_VALUE_LOCK_VALUE_MASK,
};

/* Lock Module for SHIM NOC Tiles  */
static const  XAie_LockMod Aie2PShimNocLockMod =
{
	.BaseAddr = XAIE2PGBL_NOC_MODULE_LOCK_REQUEST,
	.NumLocks = 16U,
	.LockIdOff = 0x400,
	.RelAcqOff = 0x200,
	.LockValOff = 0x4,
	.LockValUpperBound = 63,
	.LockValLowerBound = -64,
	.LockSetValBase = XAIE2PGBL_NOC_MODULE_LOCK0_VALUE,
	.LockSetValOff = 0x10,
	.LockInit = &Aie2PShimNocLockInit,
	.Acquire = &_XAieMl_LockAcquire,
	.Release = &_XAieMl_LockRelease,
	.SetValue = &_XAieMl_LockSetValue,
};

static const XAie_RegFldAttr Aie2PMemTileLockInit =
{
	.Lsb = XAIE2PGBL_MEM_TILE_MODULE_LOCK0_VALUE_LOCK_VALUE_LSB,
	.Mask = XAIE2PGBL_MEM_TILE_MODULE_LOCK0_VALUE_LOCK_VALUE_MASK,
};

/* Lock Module for Mem Tiles  */
static const  XAie_LockMod Aie2PMemTileLockMod =
{
	.BaseAddr = XAIE2PGBL_MEM_TILE_MODULE_LOCK_REQUEST,
	.NumLocks = 64U,
	.LockIdOff = 0x400,
	.RelAcqOff = 0x200,
	.LockValOff = 0x4,
	.LockValUpperBound = 63,
	.LockValLowerBound = -64,
	.LockSetValBase = XAIE2PGBL_MEM_TILE_MODULE_LOCK0_VALUE,
	.LockSetValOff = 0x10,
	.LockInit = &Aie2PMemTileLockInit,
	.Acquire = &_XAieMl_LockAcquire,
	.Release = &_XAieMl_LockRelease,
	.SetValue = &_XAieMl_LockSetValue,
};
#endif /* XAIE_FEATURE_LOCK_ENABLE */

#ifdef XAIE_FEATURE_CORE_ENABLE
	#define AIE2PCOREMOD &Aie2PCoreMod
#else
	#define AIE2PCOREMOD NULL
#endif
#ifdef XAIE_FEATURE_SS_ENABLE
	#define AIE2PTILESTRMSW &Aie2PTileStrmSw
	#define AIE2PSHIMSTRMSW &Aie2PShimStrmSw
	#define AIE2PMEMTILESTRMSW &Aie2PMemTileStrmSw
#else
	#define AIE2PTILESTRMSW NULL
	#define AIE2PSHIMSTRMSW NULL
	#define AIE2PMEMTILESTRMSW NULL
#endif
#ifdef XAIE_FEATURE_DMA_ENABLE
	#define AIE2PTILEDMAMOD &Aie2PTileDmaMod
	#define AIE2PSHIMDMAMOD &Aie2PShimDmaMod
	#define AIE2PMEMTILEDMAMOD &Aie2PMemTileDmaMod
#else
	#define AIE2PTILEDMAMOD NULL
	#define AIE2PSHIMDMAMOD NULL
	#define AIE2PMEMTILEDMAMOD NULL
#endif
#ifdef XAIE_FEATURE_DATAMEM_ENABLE
	#define AIE2PTILEMEMMOD &Aie2PTileMemMod
	#define AIE2PMEMTILEMEMMOD &Aie2PMemTileMemMod
#else
	#define AIE2PTILEMEMMOD NULL
	#define AIE2PMEMTILEMEMMOD NULL
#endif
#ifdef XAIE_FEATURE_PL_ENABLE
	#define AIE2PSHIMTILEPLIFMOD &Aie2PShimTilePlIfMod
	#define AIE2PPLIFMOD &Aie2PPlIfMod
#else
	#define AIE2PSHIMTILEPLIFMOD NULL
	#define AIE2PPLIFMOD NULL
#endif
#ifdef XAIE_FEATURE_LOCK_ENABLE
	#define AIE2PTILELOCKMOD &Aie2PTileLockMod
	#define AIE2PSHIMNOCLOCKMOD &Aie2PShimNocLockMod
	#define AIE2PMEMTILELOCKMOD &Aie2PMemTileLockMod
#else
	#define AIE2PTILELOCKMOD NULL
	#define AIE2PSHIMNOCLOCKMOD NULL
	#define AIE2PMEMTILELOCKMOD NULL
#endif

/*
 * AIE2P Module
 * This data structure captures all the modules for each tile type.
 * Depending on the tile type, this data strcuture can be used to access all
 * hardware properties of individual modules.
 */
XAie_TileMod Aie2PMod[] =
{
	{
		/*
		 * AIE2P Tile Module indexed using XAIEGBL_TILE_TYPE_AIETILE
		 */
		.NumModules = 2U,
		.CoreMod = AIE2PCOREMOD,
		.StrmSw  = AIE2PTILESTRMSW,
		.DmaMod  = AIE2PTILEDMAMOD,
		.MemMod  = AIE2PTILEMEMMOD,
		.PlIfMod = NULL,
		.LockMod = AIE2PTILELOCKMOD,
	},
	{
		/*
		 * AIE2P Shim Noc Module indexed using XAIEGBL_TILE_TYPE_SHIMNOC
		 */
		.NumModules = 1U,
		.CoreMod = NULL,
		.StrmSw  = AIE2PSHIMSTRMSW,
		.DmaMod  = AIE2PSHIMDMAMOD,
		.MemMod  = NULL,
		.PlIfMod = AIE2PSHIMTILEPLIFMOD,
		.LockMod = AIE2PSHIMNOCLOCKMOD,
	},
	{
		/*
		 * AIE2P Shim PL Module indexed using XAIEGBL_TILE_TYPE_SHIMPL
		 */
		.NumModules = 1U,
		.CoreMod = NULL,
		.StrmSw  = AIE2PSHIMSTRMSW,
		.DmaMod  = NULL,
		.MemMod  = NULL,
		.PlIfMod = AIE2PPLIFMOD,
		.LockMod = NULL,
	},
	{
		/*
		 * AIE2P MemTile Module indexed using XAIEGBL_TILE_TYPE_MEMTILE
		 */
		.NumModules = 1U,
		.CoreMod = NULL,
		.StrmSw  = AIE2PMEMTILESTRMSW,
		.DmaMod  = AIE2PMEMTILEDMAMOD,
		.MemMod  = AIE2PMEMTILEMEMMOD,
		.PlIfMod = NULL,
		.LockMod = AIE2PMEMTILELOCKMOD,
	},
};

/* Device level operations for aieml */
XAie_DeviceOps Aie2PDevOps =
{
	.IsCheckerBoard = 0U,
	.GetTTypefromLoc = &_XAieMl_GetTTypefromLoc,
	.SetPartColShimReset = NULL,
	.SetPartColClockAfterRst = NULL,
	.SetPartIsolationAfterRst = NULL,
	.PartMemZeroInit = NULL,
	.RequestTiles = NULL,
};

/** @} */
