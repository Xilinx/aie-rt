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
* @file xaiegbl_regdef.h
* @{
*
* Header to include type definitions for the register bit field definitions
* of Core, Memory, NoC and PL module registers.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus  09/26/2019  Initial creation
* 1.1   Tejus  10/21/2019  Optimize stream switch data structures
* 1.2   Tejus  10/28/2019  Add data structures for pl interface module
* 1.3   Tejus  12/09/2019  Forward declaration of structures
* 1.4	Tejus  03/16/2020  Add register properties for Mux/Demux registers
* 1.5   Tejus  03/17/2020  Add data structures for lock module
* 1.6   Tejus  03/21/2020  Add data structures for stream switch slot registers
* </pre>
*
******************************************************************************/
#ifndef XAIEGBL_REGDEF_H /* prevent circular inclusions */
#define XAIEGBL_REGDEF_H /* by using protection macros */

/***************************** Include Files *********************************/
#include "xaiegbl.h"
#include "xaiegbl_defs.h"
/************************** Constant Definitions *****************************/
/**
 * This typedef contains the attributes for the register bit fields.
 */
typedef struct {
	u32 Lsb;	/**< Bit position of the register bit-field in the 32-bit value */
	u32 Mask;	/**< Bit mask of the register bit-field in the 32-bit value */
} XAie_RegFldAttr;

/**
 * This typedef contains the attributes for the register bit fields of buffer descriptors.
 */
typedef struct {
	u8  Idx;
	u32 Lsb;	/**< Bit position of the register bit-field in the 32-bit value */
	u32 Mask;	/**< Bit mask of the register bit-field in the 32-bit value */
} XAie_RegBdFldAttr;

/**
 * This typedef contains the attributes for the Core control register.
 */
typedef struct {
	u32 RegOff;			/**< Register offset */
	XAie_RegFldAttr CtrlEn;	/**< Enable field attributes */
	XAie_RegFldAttr CtrlRst;	/**< Reset field attributes */
} XAie_RegCoreCtrl;

/**
 * This typedef contains the attributes for the Core status register.
 */
typedef struct {
	u32 RegOff;			/**< Register offset */
	XAie_RegFldAttr Done;	/**< Done value field attributes */
	XAie_RegFldAttr Rst;	/**< Reset value field attributes */
	XAie_RegFldAttr En;		/**< Enable value field attributes */
} XAie_RegCoreSts;

/*
 * This typedef captures port base address and number of slave ports available
 * for stream switch master and salve ports
 */
typedef struct {
	u8 NumPorts;
	u32 PortBaseAddr;
} XAie_StrmPort;

/*
 * This typedef contains the attributes for Stream Switch Module
 */
typedef struct {
	u8 NumSlaveSlots;
	u32 SlvConfigBaseAddr;
	u32 PortOffset;		  /**< Offset between ports */
	u32 SlotOffsetPerPort;
	u32 SlotOffset;
	XAie_RegFldAttr MstrEn;	  /**< Enable bit field attributes */
	XAie_RegFldAttr MstrPktEn;/**< Packet enable bit field attributes */
	XAie_RegFldAttr DrpHdr;   /**< Drop header bit field attributes */
	XAie_RegFldAttr Config;	  /**< Configuration bit field attributes */
	XAie_RegFldAttr SlvEn;	  /**< Enable bit field attributes */
	XAie_RegFldAttr SlvPktEn; /**< Packet enable bit field attributes */
	XAie_RegFldAttr SlotPktId;
	XAie_RegFldAttr SlotMask;
	XAie_RegFldAttr SlotEn;
	XAie_RegFldAttr SlotMsel;
	XAie_RegFldAttr SlotArbitor;
	const XAie_StrmPort *MstrConfig;
	const XAie_StrmPort *SlvConfig;
	const XAie_StrmPort *SlvSlotConfig;
} XAie_StrmMod;

/*
 * The typedef contains the attributes of Core Modules
 */
typedef struct {
	u32 ProgMemAddr;
	u32 CoreMemSize;
	u32 CoreEastAddrStart;
	u32 CoreEastAddrEnd;
	u32 CoreWestAddrStart;
	u32 CoreWestAddrEnd;
	u32 CoreSouthAddrStart;
	u32 CoreSouthAddrEnd;
	u32 CoreNorthAddrStart;
	u32 CoreNorthAddrEnd;
	const XAie_RegCoreSts *CoreSts;
	const XAie_RegCoreCtrl *CoreCtrl;
} XAie_CoreMod;

/*
 * The typedef contains the attributes of the BDs of Tile Dma and Mem Tile Dma.
 */
typedef struct {
	XAie_RegBdFldAttr BaseAddr;
	XAie_RegBdFldAttr BufferLen;
	XAie_RegBdFldAttr EnCompression;
	XAie_RegBdFldAttr EnPkt;
	XAie_RegBdFldAttr OutofOrderBdId;
	XAie_RegBdFldAttr PktId;
	XAie_RegBdFldAttr PktType;
	XAie_RegBdFldAttr D1_StepSize;
	XAie_RegBdFldAttr D0_StepSize;
	XAie_RegBdFldAttr D1_Wrap;
	XAie_RegBdFldAttr D0_Wrap;
	XAie_RegBdFldAttr D2_StepSize;
	XAie_RegBdFldAttr IterCurr;
	XAie_RegBdFldAttr IterWrap;
	XAie_RegBdFldAttr IterStepSize;
	XAie_RegBdFldAttr NxtBd;
	XAie_RegBdFldAttr UseNxtBd;
	XAie_RegBdFldAttr ValidBd;
	XAie_RegBdFldAttr LckRelVal;
	XAie_RegBdFldAttr LckRelId;
	XAie_RegBdFldAttr LckAcqEn;
	XAie_RegBdFldAttr LckAcqVal;
	XAie_RegBdFldAttr LckAcqId;
	XAie_RegBdFldAttr D0_ZeroBefore;
	XAie_RegBdFldAttr D1_ZeroBefore;
	XAie_RegBdFldAttr D2_ZeroBefore;
	XAie_RegBdFldAttr D0_ZeroAfter;
	XAie_RegBdFldAttr D1_ZeroAfter;
	XAie_RegBdFldAttr D2_ZeroAfter;
	XAie_RegBdFldAttr D2_Wrap;
	XAie_RegBdFldAttr D3_StepSize;
} XAie_DmaBdProp;

/*
 * The typedef contains the attributes of the Dma Channels
 */
typedef struct {
	XAie_RegBdFldAttr EnToken;
	XAie_RegBdFldAttr RptCount;
	XAie_RegBdFldAttr StartBd;
	XAie_RegBdFldAttr CtrlId;
	XAie_RegBdFldAttr EnCompression;
	XAie_RegBdFldAttr EnOutofOrder;
	XAie_RegBdFldAttr Reset;
} XAie_DmaChProp;

/*
 * The typedef contains attributes of Dma Modules for AIE Tiles and Mem Tiles
 */
typedef struct {
	u8  NumBdReg;
	u8  NumBds;
	u8  NumLocks;
	u32 BaseAddr;
	u32 IdxOffset;
	u32 ChCtrlBase;
	u32 NumChannels;
	u8  ChIdxOffset;
	const XAie_DmaBdProp *BdProp;
	const XAie_DmaChProp *ChProp;
} XAie_DmaMod;

/*
 * The typedef contains the attributes of Memory Module
 */
typedef struct {
	u32 Size;
	u32 MemAddr;
} XAie_MemMod;

/*
 * The typedef contains attributes of PL interface module
 */
typedef struct {
	u32 UpSzrOff;
	u32 DownSzrOff;
	u32 DownSzrEnOff;
	u32 DownSzrByPassOff;
	u32 ShimNocMuxOff;
	u32 ShimNocDeMuxOff;
	u8  NumUpSzrPorts;
	u8  MaxByPassPortNum;
	u8  NumDownSzrPorts;
	const XAie_RegFldAttr	*UpSzr32_64Bit;
	const XAie_RegFldAttr *UpSzr128Bit;
	const XAie_RegFldAttr	*DownSzr32_64Bit;
	const XAie_RegFldAttr *DownSzr128Bit;
	const XAie_RegFldAttr *DownSzrEn;
	const XAie_RegFldAttr *DownSzrByPass;
	const XAie_RegFldAttr *ShimNocMux;
	const XAie_RegFldAttr *ShimNocDeMux;
} XAie_PlIfMod;

/*
 * The typdef contains attributes of Lock modules.
 * The lock acquire and release mechanism for lock module are different across
 * hardware generations. In the first generation, the lock is acquired by
 * reading a register via AXI-MM path. The register address is specific to
 * the lock number and whether the lock is being acquired or released with
 * value. However, in subsequent generations, the lock access mechanism is
 * different. The register address to read from is computed differently.
 * To hide this change in the architecture, the information captured in the
 * below data strcuture does not use the register database directly. For more
 * details, please refer to the AI Engine hardware architecture specification
 * document.
 */
typedef struct XAie_LockMod {
	u8  NumLocks;		/* Number of lock in the module */
	s8  LockValUpperBound; 	/* Upper bound of the lock value */
	s8  LockValLowerBound; 	/* Lower bound of the lock value */
	u32 BaseAddr;		/* Base address of the lock module */
	u32 LockIdOff;		/* Offset between conseccutive locks */
	u32 RelAcqOff;  	/* Offset between Release and Acquire locks */
	u32 LockValOff; 	/* Offset thats added to the lock address for a value. */
	AieRC (*Acquire)(XAie_DevInst *DevInst,
			const struct XAie_LockMod *LockMod, XAie_LocType Loc,
			XAie_Lock Lock, u32 TimeOut);
	AieRC (*Release)(XAie_DevInst *DevInst,
			const struct XAie_LockMod *LockMod, XAie_LocType Loc,
			XAie_Lock Lock, u32 TimeOut);
} XAie_LockMod;

/*
 * This typedef contains all the modules for a Tile type
 */
typedef struct XAie_TileMod {
	const XAie_CoreMod *CoreMod;
	const XAie_StrmMod *StrmSw;
	const XAie_DmaMod  *DmaMod;
	const XAie_MemMod  *MemMod;
	const XAie_PlIfMod *PlIfMod;
	const XAie_LockMod *LockMod;
} XAie_TileMod;

#endif

/** @} */
