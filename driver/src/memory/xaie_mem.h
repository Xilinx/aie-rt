/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_mem.h
* @{
*
* Header file for data memory implementations.
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
#ifndef XAIEMEM_H
#define XAIEMEM_H

/***************************** Include Files *********************************/
#include "xaie_helper.h"
#include "xaiegbl.h"
#include "xaielib.h"

/************************** Function Prototypes  *****************************/
AieRC XAie_DataMemWrWord(XAie_DevInst *DevInst, XAie_LocType Loc,
		u32 Addr, u32 Data);
AieRC XAie_DataMemRdWord(XAie_DevInst *DevInst, XAie_LocType Loc,
		u32 Addr, u32 *Data);

#endif		/* end of protection macro */

/** @} */
