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
* @file xaie_timer.h
* @{
*
* Header file for timer implementation.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who      Date     Changes
* ----- ------   -------- -----------------------------------------------------
* 1.0   Dishita  04/05/2020  Initial creation
* </pre>
*
******************************************************************************/
#ifndef XAIETIMER_H
#define XAIETIMER_H

/***************************** Include Files *********************************/
#include "xaie_events.h"
#include "xaiegbl.h"

/************************** Enum *********************************************/

/************************** Function Prototypes  *****************************/
AieRC XAie_SetTimerTrigEventVal(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, u32 LowEventValue, u32 HighEventValue);
AieRC XAie_ResetTimer(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module);
AieRC XAie_SetTimerResetEvent(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, XAie_Events Event,
		XAie_Reset Reset);
u64 XAie_CoreReadTimer(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module);
#endif		/* end of protection macro */
