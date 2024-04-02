/******************************************************************************
* Copyright (C) 2021 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_io_common.c
* @{
*
* This file contains routines for common io backend.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date        Changes
* ----- ------  --------    ---------------------------------------------------
* 1.0   Dishita 03/08/2021  Initial creation
*
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include <stdlib.h>
#include <string.h>

#include "xaie_feature_config.h"
#include "xaie_io.h"
#include "xaie_helper.h"
/*****************************************************************************/
/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
* This API marks the bitmap with for the tiles which are clock enabled.
*
* @param	DevInst: AI engine partition device instance pointer
* @param	Args: Backend tile args
*
* @return       XAIE_OK on success, error code on failure
*
* @note		Internal only.
*
*******************************************************************************/
void _XAie_IOCommon_MarkTilesInUse(XAie_DevInst *DevInst,
		XAie_BackendTilesArray *Args)
{
	/* Setup the requested tiles bitmap locally */
	if (Args->Locs == NULL) {
		u32 StartBit, NumTiles;

		NumTiles = (u32)(DevInst->NumCols * (DevInst->NumRows - 1U));
		/* Loc is NULL, it suggests all tiles are requested */
		StartBit = _XAie_GetTileBitPosFromLoc(DevInst,
					XAie_TileLoc(0, 1));
		_XAie_SetBitInBitmap(DevInst->DevOps->TilesInUse, StartBit,
				NumTiles);
	} else {
		for(u32 i = 0; i < Args->NumTiles; i++) {
			u32 Bit;

			if(Args->Locs[i].Row == 0U) {
				continue;
			}

			/*
			 * If a tile is ungated, the rows below it are
			 * ungated.
			 */
			Bit = _XAie_GetTileBitPosFromLoc(DevInst,
					XAie_TileLoc(Args->Locs[i].Col, 1));
			_XAie_SetBitInBitmap(DevInst->DevOps->TilesInUse,
					Bit, Args->Locs[i].Row);
		}
	}
}

/** @} */
