/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_core.h
* @{
*
* Header file for core control and wait functions
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
#ifndef XAIECORE_H
#define XAIECORE_H

/***************************** Include Files *********************************/
#include "xaiegbl.h"
#include "xaielib.h"
#include "xaie_helper.h"

/************************** Constant Definitions *****************************/
/************************** Function Prototypes  *****************************/
AieRC XAie_CoreDisable(XAie_DevInst *DevInst, XAie_LocType Loc);
AieRC XAie_CoreEnable(XAie_DevInst *DevInst, XAie_LocType Loc);
AieRC XAie_CoreWaitForDone(XAie_DevInst *DevInst, XAie_LocType Loc,
		u32 TimeOut);
AieRC XAie_CoreWaitForDisable(XAie_DevInst *DevInst, XAie_LocType Loc,
		u32 TimeOut);

#endif		/* end of protection macro */
/** @} */
