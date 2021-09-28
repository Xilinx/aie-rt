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
	},
	{
		/*
		 * AIE2P Shim Noc Module indexed using XAIEGBL_TILE_TYPE_SHIMNOC
		 */
		.NumModules = 1U,
		.CoreMod = NULL,
		.StrmSw  = AIE2PSHIMSTRMSW,
	},
	{
		/*
		 * AIE2P Shim PL Module indexed using XAIEGBL_TILE_TYPE_SHIMPL
		 */
		.NumModules = 1U,
		.CoreMod = NULL,
		.StrmSw  = AIE2PSHIMSTRMSW,
	},
	{
		/*
		 * AIE2P MemTile Module indexed using XAIEGBL_TILE_TYPE_MEMTILE
		 */
		.NumModules = 1U,
		.CoreMod = NULL,
		.StrmSw  = AIE2PMEMTILESTRMSW,
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
