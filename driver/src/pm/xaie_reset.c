/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_reset.c
* @{
*
* This file contains routines for AI engine resets
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xaie_clock.h"
#include "xaie_helper.h"
#include "xaie_reset.h"
#include "xaiegbl.h"

/*****************************************************************************/
/***************************** Macro Definitions *****************************/
/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This API set the tile column reset
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE SHIM tile
* @param	RstEnable: XAIE_ENABLE to assert reset, XAIE_DISABLE to
*			   deassert reset.
*
* @return	none
*
* @note		It is not required to check the DevInst and the Loc tile type
*		as the caller function should provide the correct value.
*
******************************************************************************/
static void  _XAie_RstSetColumnReset(XAie_DevInst *DevInst,
		XAie_LocType Loc, u8 RstEnable)
{
	u8 TileType;
	u32 FldVal;
	u64 RegAddr;
	const XAie_PlIfMod *PlIfMod;

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	PlIfMod = DevInst->DevProp.DevMod[TileType].PlIfMod;
	RegAddr = PlIfMod->ColRstOff +
		_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);
	FldVal = XAie_SetField(RstEnable,
			PlIfMod->ColRst.Lsb,
			PlIfMod->ColRst.Mask);

	XAie_Write32(DevInst, RegAddr, FldVal);
}

/*****************************************************************************/
/**
*
* This API set the tile columns reset for every column in the partition
*
* @param	DevInst: Device Instance
* @param	RstEnable: XAIE_ENABLE to assert reset, XAIE_DISABLE to
*			   deassert reset.
*
* @return	none
*
* @note		It is not required to check the DevInst as the caller function
*		should provide the correct value.
*
******************************************************************************/
static void  _XAie_RstSetAllColumnsReset(XAie_DevInst *DevInst, u8 RstEnable)
{
	for (u32 C = 0; C < DevInst->NumCols; C++) {
		XAie_LocType Loc = XAie_TileLoc(C, 0);

		_XAie_RstSetColumnReset(DevInst, Loc, RstEnable);
	}
}

/*****************************************************************************/
/**
*
* This API set the SHIM tile reset
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE SHIM tile
* @param	RstEnable: XAIE_ENABLE to enable reset, XAIE_DISABLE to
*			   disable reset.
*
* @return	none
*
* @note		It is not required to check the DevInst and the Loc tile type
*		as the caller function should provide the correct value.
*
******************************************************************************/
static void _XAie_RstSetShimReset(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 RstEnable)
{
	u8 TileType;
	u32 FldVal;
	u64 RegAddr;
	const XAie_PlIfMod *PlIfMod;
	const XAie_RstMod *ShimTileRst;

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	PlIfMod = DevInst->DevProp.DevMod[TileType].PlIfMod;
	ShimTileRst = PlIfMod->ShimTileRst;
	if (ShimTileRst == NULL) {
		return;
	}

	RegAddr = ShimTileRst->RegOff +
		_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);
	FldVal = XAie_SetField(RstEnable,
			ShimTileRst->RstCntr.Lsb,
			ShimTileRst->RstCntr.Mask);

	XAie_Write32(DevInst, RegAddr, FldVal);
}

/*****************************************************************************/
/**
*
* This API set the SHIM reset for every column in the partition
*
* @param	DevInst: Device Instance
* @param	RstEnable: XAIE_ENABLE to assert reset, XAIE_DISABLE to
*			   deassert reset.
*
* @return	none
*
* @note		It is not required to check the DevInst as the caller function
*		should provide the correct value.
*
******************************************************************************/
static void  _XAie_RstSetAllShimsReset(XAie_DevInst *DevInst, u8 RstEnable)
{
	for (u32 C = 0; C < DevInst->NumCols; C++) {
		XAie_LocType Loc = XAie_TileLoc(C, 0);

		_XAie_RstSetShimReset(DevInst, Loc, RstEnable);
	}
}

/*****************************************************************************/
/**
*
* This API to set if to block NSU AXI MM slave error and decode error. If NSU
* errors is blocked, it will only generate error events.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE SHIM tile
* @param	Enable: XAIE_ENABLE to block NSU AXI MM errors, or XAIE_DISABLE
*			to unblock NSU AXI MM errors.
*
* @return	none
*
* @note		It is not required to check the DevInst and the Loc tile type
*		as the caller function should provide the correct value.
*
******************************************************************************/
static void _XAie_RstSetBlockShimNocAxiMmNsuErr(XAie_DevInst *DevInst,
		XAie_LocType Loc, u8 Enable)
{
	u8 TileType;
	u32 FldVal;
	u64 RegAddr;
	const XAie_PlIfMod *PlIfMod;
	const XAie_ShimNocAxiMMConfig *ShimNocAxiMM;

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	PlIfMod = DevInst->DevProp.DevMod[TileType].PlIfMod;
	ShimNocAxiMM = PlIfMod->ShimNocAxiMM;
	RegAddr = ShimNocAxiMM->RegOff +
		_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);
	FldVal = XAie_SetField(Enable,
			ShimNocAxiMM->NsuSlvErr.Lsb,
			ShimNocAxiMM->NsuSlvErr.Mask);
	FldVal |= XAie_SetField(Enable,
			ShimNocAxiMM->NsuDecErr.Lsb,
			ShimNocAxiMM->NsuDecErr.Mask);

	XAie_Write32(DevInst, RegAddr, FldVal);
}

/*****************************************************************************/
/**
*
* This API set if to block the NSU AXI MM slave error and decode error config
* for all the SHIM NOCs in the full partition.
*
* @param	DevInst: Device Instance
* @param	Enable: XAIE_ENABLE to enable NSU AXI MM errors, XAIE_DISABLE to
*			disable.
*
* @return	none
*
* @note		It is not required to check the DevInst as the caller function
*		should provide the correct value.
*
******************************************************************************/
static void  _XAie_RstSetBlockAllShimsNocAxiMmNsuErr(XAie_DevInst *DevInst,
		u8 Enable)
{
	for (u32 C = 0; C < DevInst->NumCols; C++) {
		XAie_LocType Loc = XAie_TileLoc(C, 0);
		u8 TileType;

		TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
		if (TileType != XAIEGBL_TILE_TYPE_SHIMNOC) {
			continue;
		}
		_XAie_RstSetBlockShimNocAxiMmNsuErr(DevInst, Loc, Enable);
	}
}

/*****************************************************************************/
/**
*
* This API resets the AI engine partition pointed by the AI engine device
* instance. The AI engine partition is the column based groups of AI engine
* tiles represented by the AI engine device instance. It will reset the SHIMs
* and columns in the partition, gate all the columns, and it will configure the
* SHIMNOC to block the slave and decode errors.
*
* @param	DevInst: Device Instance
*
* @return	XAIE_OK on success.
*		XAIE_INVALID_ARGS if any argument is invalid
*
* @note		The reset partition device should be in one single backend call
*		to avoid any other thread to change some registers which can
*		cause system to system to crash. For some backend, such as CDO
*		backend	and debug backend, there is no such issue as they don't
*		run on real hardware. This function defines the AI engine
*		partition reset sequence for the backends which allow the reset
*		partition device with multiple backend calls.
*		Here is the reset sequece:
*		* reset columns
*		* enable protected registers
*		* enable shim reset for every columns
*		* assert shim reset in NPI
*		* release shim reset in NPI
*		* setup AXI MM config to block NSU errors
*		* disable protected registers
*		* gate all the tiles
*******************************************************************************/
AieRC XAie_ResetPartition(XAie_DevInst *DevInst)
{
	if((DevInst == XAIE_NULL) ||
		(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAieLib_print("Error: Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	_XAie_RstSetAllColumnsReset(DevInst, XAIE_ENABLE);

	XAie_RunOp(DevInst, XAIE_BACKEND_OP_SET_PROTREG,
			(void *)(uintptr_t)XAIE_ENABLE);

	_XAie_RstSetAllShimsReset(DevInst, XAIE_ENABLE);

	XAie_RunOp(DevInst, XAIE_BACKEND_OP_ASSERT_SHIMRST,
			(void *)(uintptr_t)XAIE_ENABLE);

	XAie_RunOp(DevInst, XAIE_BACKEND_OP_ASSERT_SHIMRST,
			(void *)(uintptr_t)XAIE_DISABLE);

	_XAie_RstSetAllShimsReset(DevInst, XAIE_DISABLE);

	_XAie_RstSetBlockAllShimsNocAxiMmNsuErr(DevInst, XAIE_ENABLE);

	XAie_RunOp(DevInst, XAIE_BACKEND_OP_SET_PROTREG,
			(void *)(uintptr_t)XAIE_DISABLE);

	_XAie_PmSetPartitionClock(DevInst, XAIE_DISABLE);

	return XAIE_OK;
}

/** @} */
