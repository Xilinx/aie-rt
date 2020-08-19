/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_perfcnt.h
* @{
*
* Header file for performance counter implementations.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who      Date     Changes
* ----- ------   -------- -----------------------------------------------------
* 1.0   Dishita  11/21/2019  Initial creation
* </pre>
*
******************************************************************************/
#ifndef XAIEPERFCNT_H
#define XAIEPERFCNT_H

/***************************** Include Files *********************************/
#include "xaie_events.h"
#include "xaie_helper.h"
#include "xaiegbl.h"
#include "xaiegbl_defs.h"
#include "xaielib.h"

/************************** Enum *********************************************/
/*
 * This enum contains all the Counters for all modules: Core, Memory of AIE tile
 * MEM tile and PL tile. The XAIAE_MEMPERFCOUNTER_0 and XAIAE_MEMPERFCOUNTER_1
 * corresponds to the Counter 0 and Counter 1 respectively of Memory Module of
 * AIE Tile.
 */
typedef enum {
	XAIE_PERFCOUNTER_0,
	XAIE_PERFCOUNTER_1,
	XAIE_PERFCOUNTER_2,
	XAIE_PERFCOUNTER_3,
	XAIE_MEMPERFCOUNTER_0,
	XAIE_MEMPERFCOUNTER_1,
} XAie_PerfCounters;

/************************** Function Prototypes  *****************************/
u32 XAie_PerfCounterGet(XAie_DevInst *DevInst, XAie_LocType Loc,
				XAie_PerfCounters Counter);
AieRC XAie_PerfCounterControlSet(XAie_DevInst *DevInst, XAie_LocType Loc,
				XAie_PerfCounters Counter, XAie_Events StartEvent,
				XAie_Events StopEvent);
AieRC XAie_PerfCounterResetControlSet(XAie_DevInst *DevInst, XAie_LocType Loc,
				XAie_PerfCounters Counter, XAie_Events ResetEvent);
AieRC XAie_PerfCounterSet(XAie_DevInst *DevInst, XAie_LocType Loc,
				XAie_PerfCounters Counter, u32 CounterVal);
AieRC XAie_PerfCounterEventValueSet(XAie_DevInst *DevInst, XAie_LocType Loc,
				XAie_PerfCounters Counter, u32 EventVal);
#endif		/* end of protection macro */
