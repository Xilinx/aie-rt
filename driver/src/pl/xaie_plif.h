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
* @file xaie_plif.h
* @{
*
* This file contains routines for AIE tile control.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   10/28/2019  Initial creation
* 1.1   Tejus   03/16/2020  Implementation of apis for Mux/Demux configuration
* </pre>
*
******************************************************************************/
#ifndef XAIEPLIF_H
#define XAIEPLIF_H

/***************************** Include Files *********************************/
#include "xaiegbl.h"
#include "xaielib.h"
/**************************** Type Definitions *******************************/
/*
 * This enum captures the AIE-PL interface bit widths available in the hardware.
 */
typedef enum {
	PLIF_WIDTH_32 = 32,
	PLIF_WIDTH_64 = 64,
	PLIF_WIDTH_128 = 128
} XAie_PlIfWidth;

/************************** Function Prototypes  *****************************/
AieRC XAie_PlIfBliBypassEnable(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum);
AieRC XAie_PlIfBliBypassDisable(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum);
AieRC XAie_PlIfBliBypassRangeDisable(XAie_DevInst *DevInst, XAie_LocRange Range,
		u8 PortNum);
AieRC XAie_PlIfBliBypassRangeEnable(XAie_DevInst *DevInst, XAie_LocRange Range,
		u8 PortNum);
AieRC XAie_PlIfDownSzrRangeEnable(XAie_DevInst *DevInst, XAie_LocRange Range,
		u8 PortNum);
AieRC XAie_PlIfDownSzrRangeDisable(XAie_DevInst *DevInst, XAie_LocRange Range,
		u8 PortNum);
AieRC XAie_PlIfDownSzrEnable(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum);
AieRC XAie_PlIfDownSzrDisable(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum);
AieRC XAie_PlToAieIntfRangeEnable(XAie_DevInst *DevInst, XAie_LocRange Range,
		u8 PortNum, XAie_PlIfWidth Width);
AieRC XAie_PlToAieIntfRangeDisable(XAie_DevInst *DevInst, XAie_LocRange Range,
		u8 PortNum, XAie_PlIfWidth Width);
AieRC XAie_PlToAieIntfEnable(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum, XAie_PlIfWidth Width);
AieRC XAie_PlToAieIntfDisable(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum, XAie_PlIfWidth Width);
AieRC XAie_AieToPlIntfRangeEnable(XAie_DevInst *DevInst, XAie_LocRange Range,
		u8 PortNum, XAie_PlIfWidth Width);
AieRC XAie_AieToPlIntfRangeDisable(XAie_DevInst *DevInst, XAie_LocRange Range,
		u8 PortNum, XAie_PlIfWidth Width);
AieRC XAie_AieToPlIntfEnable(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum, XAie_PlIfWidth Width);
AieRC XAie_AieToPlIntfDisable(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum, XAie_PlIfWidth Width);
AieRC XAie_EnableShimDmaToAieStrmPortRange(XAie_DevInst *DevInst,
		XAie_LocRange Range, u8 PortNum);
AieRC XAie_EnableShimDmaToAieStrmPort(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum);
AieRC XAie_EnableAieToShimDmaStrmPortRange(XAie_DevInst *DevInst,
		XAie_LocRange Range, u8 PortNum);
AieRC XAie_EnableAieToShimDmaStrmPort(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum);
AieRC XAie_EnableNoCToAieStrmPortRange(XAie_DevInst *DevInst,
		XAie_LocRange Range, u8 PortNum);
AieRC XAie_EnableNoCToAieStrmPort(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum);
AieRC XAie_EnableAieToNoCStrmPortRange(XAie_DevInst *DevInst,
		XAie_LocRange Range, u8 PortNum);
AieRC XAie_EnableAieToNoCStrmPort(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum);
AieRC XAie_EnablePlToAieStrmPortRange(XAie_DevInst *DevInst,
		XAie_LocRange Range, u8 PortNum);
AieRC XAie_EnablePlToAieStrmPort(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum);
AieRC XAie_EnableAieToPlStrmPortRange(XAie_DevInst *DevInst,
		XAie_LocRange Range, u8 PortNum);
AieRC XAie_EnableAieToPlStrmPort(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 PortNum);
#endif		/* end of protection macro */
