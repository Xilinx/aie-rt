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
* @file xaie_core.h
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
* </pre>
*
******************************************************************************/
#ifndef XAIEDMA_H
#define XAIEDMA_H

/***************************** Include Files *********************************/
#include "xaiegbl.h"
#include "xaielib.h"
#include "xaie_helper.h"


/************************** Function Prototypes  *****************************/
AieRC XAie_DmaBdConfig_1D(XAie_DevInst *DevInst, XAie_LocType Loc, u8 BdNum,
		u32 BaseAddr, u32 Len, XAie_LockDesc Acq, XAie_LockDesc Rel,
		XAie_DmaBdAttr Attr);
AieRC XAie_DmaBdConfig_1DRange(XAie_DevInst *DevInst, XAie_LocRange Range,
		u8 BdNum, u32 BaseAddr, u32 Len, XAie_LockDesc Acq,
		XAie_LockDesc Rel, XAie_DmaBdAttr Attr);

/* Channel Control APIs */
AieRC XAie_DmaEnChannelRange(XAie_DevInst *DevInst, XAie_LocRange Range,
		u8 ChNum, u8 StartBd, XAie_DmaDirection Dir, u8 RptCnt,
		u8 EnToken, u8 CtrlId, XAie_DmaBdAttr Attr);
AieRC XAie_DmaEnChannel(XAie_DevInst *DevInst, XAie_LocType Loc, u8 ChNum,
		u8 StartBd, XAie_DmaDirection Dir, u8 RptCnt, u8 EnToken,
		u8 CtrlId, XAie_DmaBdAttr Attr);

#endif		/* end of protection macro */
/** @} */
