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

#ifdef XAIE_FEATURE_CORE_ENABLE
	#define AIE2PCOREMOD &Aie2PCoreMod
#else
	#define AIE2PCOREMOD NULL
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
	},
	{
		/*
		 * AIE2P Shim Noc Module indexed using XAIEGBL_TILE_TYPE_SHIMNOC
		 */
		.NumModules = 1U,
		.CoreMod = NULL,
	}
	{
		/*
		 * AIE2P Shim PL Module indexed using XAIEGBL_TILE_TYPE_SHIMPL
		 */
		.NumModules = 1U,
		.CoreMod = NULL,
	},
	{
		/*
		 * AIE2P MemTile Module indexed using XAIEGBL_TILE_TYPE_MEMTILE
		 */
		.NumModules = 1U,
		.CoreMod = NULL,
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
