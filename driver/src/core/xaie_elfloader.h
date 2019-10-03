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
* @file xaie_elfloader.h
* @{
*
* Header file for core elf loader functions
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
#ifndef XAIELOADER_H
#define XAIELOADER_H

/***************************** Include Files *********************************/
#include <elf.h>
#include <stdlib.h>
#include <string.h>
#include "xaie_helper.h"
#include "xaiegbl.h"
#include "xaiegbl_defs.h"
#include "xaielib.h"

/************************** Constant Definitions *****************************/
/************************** Variable Definitions *****************************/
/************************** Function Prototypes  *****************************/
AieRC XAie_LoadElfRange(XAie_DevInst *DevInst, XAie_LocRange Range, u8 *ElfPtr,
		u8 LoadSym);
AieRC XAie_LoadElf(XAie_DevInst *DevInst, XAie_LocType Loc, u8 *ElfPtr,
		u8 LoadSym);

#endif		/* end of protection macro */
/** @} */
