/******************************************************************************
*
* Copyright (C) 2019 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
******************************************************************************/

/*****************************************************************************/
/**
* @file xaiegbl.c
* @{
*
* This file contains the global initialization functions for the Tile.
* This is applicable for both the AIE tiles and Shim tiles.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   09/24/2019  Initial creation
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xaiegbl.h"
#include "xaiegbl_defs.h"
#include "xaiegbl_regdef.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/**************************** Macro Definitions ******************************/

/************************** Variable Definitions *****************************/
extern XAie_TileMod Aie2Mod[XAIEGBL_TILE_TYPE_MAX];

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This is the global initialization function for all the tiles of the AIE array
* The function sets up the Device Instance pointer with the appropriate values
* from the ConfigPtr.
*
* @param	InstPtr - Global AIE instance structure.
* @param	ConfigPtr - Global AIE configuration pointer.
*
* @return	XAIE_OK on success and error code on failure
*
* @note		None.
*
******************************************************************************/
AieRC XAie_CfgInitialize(XAie_DevInst *InstPtr, XAie_Config *ConfigPtr)
{

	if((InstPtr == XAIE_NULL) || (ConfigPtr == XAIE_NULL)) {
		XAieLib_print("Error %d: Invalid input arguments\n",
				XAIE_INVALID_ARGS);
		return XAIE_INVALID_ARGS;
	}

	if(InstPtr->IsReady)
		return XAIE_OK;

	/* Initialize device property according to Device Type */
	if(ConfigPtr->AieGen == XAIE_DEV_GEN_AIE2) {
		InstPtr->DevProp.DevMod = Aie2Mod;
		InstPtr->DevProp.DevGen = XAIE_DEV_GEN_AIE2;
	} else {
		XAieLib_print("Error %d: Invalid device\n",
				XAIE_INVALID_DEVICE);
		return XAIE_INVALID_DEVICE;
	}

	InstPtr->IsReady = XAIE_COMPONENT_IS_READY;
	InstPtr->DevProp.RowShift = ConfigPtr->RowShift;
	InstPtr->DevProp.ColShift = ConfigPtr->ColShift;
	InstPtr->BaseAddr = ConfigPtr->BaseAddr;
	InstPtr->NumRows = ConfigPtr->NumRows;
	InstPtr->NumCols = ConfigPtr->NumCols;
	InstPtr->ShimRow = ConfigPtr->ShimRowNum;
	InstPtr->MemTileRowStart = ConfigPtr->MemTileRowStart;
	InstPtr->MemTileNumRows = ConfigPtr->MemTileNumRows;
	InstPtr->AieTileRowStart = ConfigPtr->AieTileRowStart;
	InstPtr->AieTileNumRows = ConfigPtr->AieTileNumRows;


	XAieLib_InitDev();

	return XAIE_OK;
}

/** @} */
