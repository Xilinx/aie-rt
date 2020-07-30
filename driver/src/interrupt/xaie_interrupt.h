/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_interrupt.h
* @{
*
* Header file for AIE interrupt module.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Nishad  07/21/2020  Initial creation
* 1.1   Nishad  07/23/2020  Add APIs to configure second level interrupt
*			    controller.
* </pre>
*
******************************************************************************/
#ifndef XAIE_INTERRUPT_H
#define XAIE_INTERRUPT_H

/***************************** Include Files *********************************/
#include "xaie_events.h"

/**************************** Type Definitions *******************************/
/************************** Function Prototypes  *****************************/
AieRC XAie_IntrCtrlL1Enable(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_BroadcastSw Switch, u8 IntrId);
AieRC XAie_IntrCtrlL1Disable(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_BroadcastSw Switch, u8 IntrId);
AieRC XAie_IntrCtrlL1IrqSet(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_BroadcastSw Switch, u8 BroadcastId);
AieRC XAie_IntrCtrlL1Event(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_BroadcastSw Switch, u8 IrqEventId, XAie_Events Event);
AieRC XAie_IntrCtrlL1BroadcastBlock(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_BroadcastSw Switch, u32 ChannelBitMap);
AieRC XAie_IntrCtrlL1BroadcastUnblock(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_BroadcastSw Switch, u32 ChannelBitMap);
AieRC XAie_IntrCtrlL2Enable(XAie_DevInst *DevInst, XAie_LocType Loc,
		u32 ChannelBitMap);
AieRC XAie_IntrCtrlL2Disable(XAie_DevInst *DevInst, XAie_LocType Loc,
		u32 ChannelBitMap);
AieRC XAie_IntrCtrlL2IrqSet(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 NoCIrqId);

#endif		/* end of protection macro */
