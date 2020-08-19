/******************************************************************************
*
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
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
