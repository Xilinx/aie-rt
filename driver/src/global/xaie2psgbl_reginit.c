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

#ifdef XAIE_FEATURE_CORE_ENABLE
	#define AIE2PSCOREMOD &Aie2PSCoreMod
#else
	#define AIE2PSCOREMOD NULL
#endif

/*
 * AIE2P Module
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
	},
	{
		/*
		 * AIE2PS Shim Noc Module indexed using XAIEGBL_TILE_TYPE_SHIMNOC
		 */
		.NumModules = 1U,
		.CoreMod = NULL,
	},
	{
		/*
		 * AIE2PS Shim PL Module indexed using XAIEGBL_TILE_TYPE_SHIMPL
		 */
		.NumModules = 1U,
		.CoreMod = NULL,
	},
	{
		/*
		 * AIE2PS MemTile Module indexed using XAIEGBL_TILE_TYPE_MEMTILE
		 */
		.NumModules = 1U,
		.CoreMod = NULL,
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
