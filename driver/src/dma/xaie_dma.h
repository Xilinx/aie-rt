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
* @file xaie_dma.h
* @{
*
* Header file for dma functions
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   09/24/2019  Initial creation
* 1.1   Tejus   03/22/2020  Remove initial dma implemenatation
* 1.2   Tejus   03/22/2020  Dma apis for aie
* 1.3   Tejus   04/09/2020  Remove unused argument from interleave enable api
* </pre>
*
******************************************************************************/
#ifndef XAIEDMA_H
#define XAIEDMA_H
/***************************** Include Files *********************************/
#include "xaiegbl.h"
#include "xaielib.h"

/************************** Function Prototypes  *****************************/
AieRC XAie_DmaDescInit(XAie_DevInst *DevInst, XAie_DmaDesc *DmaDesc,
		XAie_LocType Loc);
AieRC XAie_DmaSetLock(XAie_DmaDesc *DmaDesc, XAie_Lock Acq, XAie_Lock Rel);
AieRC XAie_DmaSetPkt(XAie_DmaDesc *DmaDesc, XAie_Packet Pkt);
AieRC XAie_DmaSetOutofOrderBdId(XAie_DmaDesc *DmaDesc, u8 OutofOrderBdId);
AieRC XAie_DmaSetDoubleBuffer(XAie_DmaDesc *DmaDesc, u64 Addr, XAie_Lock Acq,
		XAie_Lock Rel);
AieRC XAie_DmaSetAddrLen(XAie_DmaDesc *DmaDesc, u64 Addr, u32 Len);
AieRC XAie_DmaSetMultiDimAddr(XAie_DmaDesc *DmaDesc, XAie_DmaTensor *Tensor,
		u64 Addr, u32 Len);
AieRC XAie_DmaEnableCompression(XAie_DmaDesc *DmaDesc);
AieRC XAie_DmaSetNextBd(XAie_DmaDesc *DmaDesc, u8 NextBd, u8 EnableNextBd);
AieRC XAie_DmaEnableBd(XAie_DmaDesc *DmaDesc);
AieRC XAie_DmaDisableBd(XAie_DmaDesc *DmaDesc);
AieRC XAie_DmaSetAxi(XAie_DmaDesc *DmaDesc, u8 Smid, u8 BurstLen, u8 Qos,
		u8 Cache, u8 Secure);
AieRC XAie_DmaSetInterleaveEnable(XAie_DmaDesc *DmaDesc, u8 DoubleBuff,
		u8 IntrleaveCount, u16 IntrleaveCurr);
AieRC XAie_DmaWriteBd(XAie_DevInst *DevInst, XAie_DmaDesc *DmaDesc,
		XAie_LocType Loc, u8 BdNum);
AieRC XAie_DmaChannelResetAll(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_DmaChReset Reset);
AieRC XAie_DmaChannelReset(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 ChNum, XAie_DmaDirection Dir, XAie_DmaChReset Reset);
AieRC XAie_DmaChannelPauseStream(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 ChNum, XAie_DmaDirection Dir, u8 Pause);
AieRC XAie_DmaChannelPauseMem(XAie_DevInst *DevInst, XAie_LocType Loc, u8 ChNum,
		XAie_DmaDirection Dir, u8 Pause);
AieRC XAie_DmaChannelConfig(XAie_DevInst *DevInst, XAie_DmaDesc *DmaDesc,
		XAie_LocType Loc, u8 ChNum, XAie_DmaDirection Dir,
		u8 RepeatCount, u8 EnTokenIssue, u8 ControllerId);
AieRC XAie_DmaChannelPushBdToQueue(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 ChNum, XAie_DmaDirection Dir, u8 BdNum);
AieRC XAie_DmaChannelEnable(XAie_DevInst *DevInst, XAie_LocType Loc, u8 ChNum,
		XAie_DmaDirection Dir);
AieRC XAie_DmaChannelDisable(XAie_DevInst *DevInst, XAie_LocType Loc, u8 ChNum,
		XAie_DmaDirection Dir);

#endif		/* end of protection macro */
