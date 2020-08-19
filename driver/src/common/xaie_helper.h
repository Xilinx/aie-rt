/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_helper.h
* @{
*
* This file contains inline helper functions for AIE drivers.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   09/24/2019  Initial creation
* 1.1   Tejus   12/09/2019  Include correct header file to avoid cyclic
*			    dependancy
* 1.2   Tejus   03/22/2020  Remove helper functions used by initial dma
*			    implementations
* 1.3   Tejus   04/13/2020  Add api to get tile type from Loc
* 1.4   Tejus   04/13/2020  Remove helper functions for range apis
* </pre>
*
******************************************************************************/
#ifndef XAIEHELPER_H
#define XAIEHELPER_H

/***************************** Include Files *********************************/
#include "xaiegbl_regdef.h"

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* Calculates the Tile Address from Row, Col of the AIE array/partition
*
* @param	DevInst: Device Instance
* @param	R: Row
* @param	C: Column
* @return	TileAddr
*
* @note		Internal API only.
*
******************************************************************************/
static inline u64 _XAie_GetTileAddr(XAie_DevInst *DevInst, int R, int C)
{
	return (R << DevInst->DevProp.RowShift) | (C << DevInst->DevProp.ColShift);
}

u8 _XAie_GetTileTypefromLoc(XAie_DevInst *DevInst, XAie_LocType Loc);
AieRC _XAie_CheckModule(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module);
#endif		/* end of protection macro */
/** @} */
