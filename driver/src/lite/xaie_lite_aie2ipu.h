/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_lite_aie2ipu.h
* @{
*
* This header file defines a lightweight version of AIE driver APIs for AIE2 IPU
* device generation.

* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0  Nishad  08/30/2021  Initial creation
* </pre>
*
******************************************************************************/
#ifndef XAIE_LITE_AIE2IPU_H
#define XAIE_LITE_AIE2IPU_H

/***************************** Include Files *********************************/
#ifdef __AIEIPU__
#include <platform-hw-config.h>
#endif

/************************** Constant Definitions *****************************/
#ifndef XAIE_BASE_ADDR
#define XAIE_BASE_ADDR			IPU_AIE_BASEADDR
#endif

#ifndef XAIE_NUM_ROWS
#define XAIE_NUM_ROWS			6
#endif

#ifndef XAIE_NUM_COLS
#define XAIE_NUM_COLS			5
#endif

#define XAIE_COL_SHIFT			25
#define XAIE_ROW_SHIFT			20
#define XAIE_SHIM_ROW			0
#define XAIE_MEM_TILE_ROW_START		1
#define XAIE_MEM_TILE_NUM_ROWS		1
#define XAIE_AIE_TILE_ROW_START		2
#define XAIE_AIE_TILE_NUM_ROWS		4

#define UPDT_NEXT_NOC_TILE_LOC(Loc)	((Loc).Col == 0 ? (Loc).Col = 1 : (Loc).Col++)

#include "xaie_lite_regdef_aieml.h"
#include "xaie_lite_regops_aieml.h"

/************************** Variable Definitions *****************************/
/************************** Function Prototypes  *****************************/

#endif		/* end of protection macro */
/** @} */
