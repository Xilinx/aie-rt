/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_npi.h
* @{
*
* This file contains the data structures and routines for AI engine NPI access
* operations for.
*
******************************************************************************/
#ifndef XAIE_NPI_H
#define XAIE_NPI_H
/***************************** Include Files *********************************/
#include "xaiegbl.h"
#include "xaiegbl_regdef.h"

/************************** Constant Definitions *****************************/

#ifndef XAIE_NPI_BASEADDR
#define XAIE_NPI_BASEADDR		0xF70A0000
#endif

/*
 * This typedef contains the attributes for AI engine NPI registers
 */
typedef struct XAie_NpiMod {
	u32 PcsrMaskOff;
	u32 PcsrCntrOff;
	u32 PcsrLockOff;
	u32 ProtRegOff;
	u32 PcsrUnlockCode;
	XAie_RegFldAttr ShimReset;
	XAie_RegFldAttr ProtRegEnable;
	XAie_RegFldAttr ProtRegFirstCol;
	XAie_RegFldAttr ProtRegLastCol;
	u32 (*SetProtectedRegField)(XAie_DevInst *DevInst, u8 Enable);
} XAie_NpiMod;

typedef void (*NpiWrite32Func)(void *IOInst, u32 RegOff, u32 RegVal);

/************************** Function Prototypes  *****************************/
void _XAie_NpiSetShimReset(XAie_DevInst *DevInst, u8 RstEnable);
void _XAie_NpiSetProtectedRegEnable(XAie_DevInst *DevInst, u8 Enable);

#endif	/* End of protection macro */

/** @} */
