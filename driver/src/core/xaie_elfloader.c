/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_elfloader.c
* @{
*
* The file has implementations of routines for elf loading.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   09/24/2019  Initial creation
* 1.1   Tejus	01/04/2020  Cleanup error messages
* 1.2   Tejus   03/20/2020  Make internal functions static
* 1.3   Tejus   04/13/2020  Remove range apis and change to single tile apis
* 1.4   Tejus   05/26/2020  Remove elf loader implementation for refactoring
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xaie_elfloader.h"

/************************** Constant Definitions *****************************/
/************************** Function Definitions *****************************/
AieRC XAie_LoadElf(XAie_DevInst *DevInst, XAie_LocType Loc, u8 *ElfPtr,
		u8 LoadSym)
{
	return XAIE_ERR;
}

/** @} */
