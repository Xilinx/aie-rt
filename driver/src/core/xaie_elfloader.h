/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_elfloader.h
* @{
*
* Header file for core elf loader functions
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   09/24/2019  Initial creation
* 1.1   Tejus   03/20/2020  Remove range apis
* </pre>
*
******************************************************************************/
#ifndef XAIELOADER_H
#define XAIELOADER_H

/***************************** Include Files *********************************/
#include <elf.h>
#include <stdlib.h>
#include <string.h>
#include "xaie_helper.h"
#include "xaiegbl.h"
#include "xaiegbl_defs.h"
#include "xaielib.h"

#ifdef __AIESIM__
#include "xaiesim.h"
#endif
/************************** Constant Definitions *****************************/
/************************** Variable Definitions *****************************/
typedef struct {
	u32 start;	/**< Stack start address */
	u32 end;	/**< Stack end address */
} XAieSim_StackSz;
/************************** Function Prototypes  *****************************/
AieRC XAie_LoadElf(XAie_DevInst *DevInst, XAie_LocType Loc, u8 *ElfPtr,
		u8 LoadSym);

#endif		/* end of protection macro */
/** @} */
