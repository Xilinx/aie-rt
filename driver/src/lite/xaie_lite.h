/******************************************************************************
* Copyright (C) 2021 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_lite.h
* @{
*
* This header file defines a lightweight version of AIE driver APIs.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0  Nishad  08/30/2021  Initial creation
* </pre>
*
******************************************************************************/
#ifndef XAIE_LITE_H
#define XAIE_LITE_H

#ifdef XAIE_FEATURE_LITE

#include "xaiegbl.h"
#include "xaiegbl_defs.h"
#include "xaie_helper.h"

#define XAie_LDeclareDevInst(DevInst, _BaseAddr, _StartCol, _NumCols) \
	XAie_DevInst DevInst = { \
		.BaseAddr = (_BaseAddr), \
		.StartCol = (_StartCol), \
		.NumCols = (_NumCols), \
	}

#if XAIE_DEV_SINGLE_GEN == XAIE_DEV_GEN_AIE
#include "xaie_lite_aie.h"
#include "xaie_lite_shim_aie.h"
#elif XAIE_DEV_SINGLE_GEN == XAIE_DEV_GEN_AIEML
#include "xaie_lite_aieml.h"
#include "xaie_lite_shim_aie.h"
#else
#include <xaie_custom_device.h>
#endif

#define XAIE_ERROR_MSG(...)						\
	"[AIE ERROR] %s():%d: %s", __func__, __LINE__, __VA_ARGS__

#ifdef XAIE_ENABLE_INPUT_CHECK
#define XAIE_ERROR_RETURN(ERRCON, RET, ...) {	\
	if (ERRCON) {				\
		printf(__VA_ARGS__);		\
		return (RET);			\
	}					\
}
#else
#define XAIE_ERROR_RETURN(...)
#endif

/************************** Variable Definitions *****************************/
/************************** Function Prototypes  *****************************/
AieRC XAie_IsPartitionIdle(XAie_DevInst *DevInst);
AieRC XAie_ClearPartitionContext(XAie_DevInst *DevInst);

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This is API returns the tile type for a given device instance and tile
* location.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of the AIE tile.
* @return	TileType (AIETILE/MEMTILE/SHIMPL/SHIMNOC on success and MAX on
*		error)
*
* @note		Internal only.
*
******************************************************************************/
static inline u8 _XAie_LGetTTypefromLoc(XAie_DevInst *DevInst, XAie_LocType Loc)
{
	XAIE_ERROR_RETURN((Loc.Col >= XAIE_NUM_COLS), XAIEGBL_TILE_TYPE_MAX,
			XAIE_ERROR_MSG("Invalid column: %d\n", Loc.Col));

	if(Loc.Row == 0U) {
		return _XAie_LGetShimTTypefromLoc(DevInst, Loc);
	} else if(Loc.Row >= XAIE_MEM_TILE_ROW_START &&
			(Loc.Row < (XAIE_MEM_TILE_ROW_START +
				    XAIE_MEM_TILE_NUM_ROWS))) {
		return XAIEGBL_TILE_TYPE_MEMTILE;
	} else if (Loc.Row >= XAIE_AIE_TILE_ROW_START &&
			(Loc.Row < (XAIE_AIE_TILE_ROW_START +
				    XAIE_AIE_TILE_NUM_ROWS))) {
		return XAIEGBL_TILE_TYPE_AIETILE;
	}

	XAIE_ERROR_RETURN(1U, XAIEGBL_TILE_TYPE_MAX,
			XAIE_ERROR_MSG("Cannot find Tile Type\n"));

	return XAIEGBL_TILE_TYPE_MAX;
}

/*****************************************************************************/
/**
*
* This is API returns the location next NoC tile.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of the AIE tile.
*
* @note		Internal only.
*
******************************************************************************/
__FORCE_INLINE__
static inline XAie_LocType XAie_LPartGetNextNocTile(XAie_DevInst *DevInst,
		XAie_LocType Loc)
{
	XAie_LocType lLoc = XAie_TileLoc((Loc.Col + DevInst->StartCol),
			Loc.Row);

	UPDT_NEXT_NOC_TILE_LOC(lLoc);
	return lLoc;
}

/**
*
* This API checks if an AI engine tile is in use.
*
* @param	DevInst: Device Instance.
* @param	Loc: Tile location.
*
* @return	XAIE_ENABLE if a tile is in use, otherwise XAIE_DISABLE.
*
* @note		Internal only.
*
******************************************************************************/
static inline u8 _XAie_LPmIsTileRequested(XAie_DevInst *DevInst,
		XAie_LocType Loc)
{
	u8 TileType;
	TileType = _XAie_LGetTTypefromLoc(DevInst, Loc);

	if (TileType == XAIEGBL_TILE_TYPE_MAX) {
		return XAIE_DISABLE;
	}
	if (TileType == XAIEGBL_TILE_TYPE_SHIMNOC || TileType == XAIEGBL_TILE_TYPE_SHIMPL) {
		return XAIE_ENABLE;
	}

	return _XAie_LPmIsArrayTileRequested(DevInst, Loc);
}

#endif /* XAIE_FEATURE_LITE */

#endif /* XAIE_LITE_H */

/** @} */
