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
* @file xaie_dma.c
* @{
*
* This file contains routines for AIE DMA Controls.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   09/24/2019  Initial creation
* 1.1   Tejus	01/04/2020  Cleanup error messages
* 1.2   Tejus   03/22/2020  Remove initial dma api implementation
* 1.3   Tejus   03/22/2020  Dma api implementation
* 1.4   Tejus   04/09/2020  Remove unused argument from interleave enable api
* 1.5   Tejus   04/13/2020  Remove use of range in apis
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include <string.h>
#include "xaie_dma.h"
#include "xaie_helper.h"
#include "xaiegbl_regdef.h"

/************************** Constant Definitions *****************************/
#define XAIE_DMA_32BIT_TXFER_LEN			2U

#define XAIE_SHIM_BLEN_SHIFT				0x3
#define XAIE_DMA_CHCTRL_NUM_WORDS			2U
/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This API initializes the Dma Descriptor for AIE DMAs for a given range of
* tiles.
*
* @param	DevInst: Device Instance.
* @param	DmaDesc: Pointer to the user allocated dma descriptor.
* @param	Loc: Location of AIE Tile
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_DmaDescInit(XAie_DevInst *DevInst, XAie_DmaDesc *DmaDesc,
		XAie_LocType Loc)
{
	u8 TileType;
	const XAie_DmaMod *DmaMod;

	if((DevInst == XAIE_NULL) || (DmaDesc == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAieLib_print("Error: Invalid arguments\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_SHIMPL) {
		XAieLib_print("Error: Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	DmaMod = DevInst->DevProp.DevMod[TileType].DmaMod;

	memset((void *)DmaDesc, 0U, sizeof(XAie_DmaDesc));

	DmaMod->DmaBdInit(DmaDesc);
	DmaDesc->TileType = TileType;
	DmaDesc->IsReady = XAIE_COMPONENT_IS_READY;
	DmaDesc->DmaMod = DmaMod;
	DmaDesc->LockMod = DevInst->DevProp.DevMod[TileType].LockMod;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API initializes the Acquire and Release Locks for a a Dma descriptor.
*
* @param	DmaDesc: Initialized Dma Descriptor.
* @param	Acq: Lock object with acquire lock ID and lock value.
* @param	Rel: Lock object with release lock ID and lock value.
* @param 	AcqEn: XAIE_ENABLE/XAIE_DISABLE for enable or disable acquire
*		lock.
* @param 	RelEn: XAIE_ENABLE/XAIE_DISABLE for enable or disable release
*		lock.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal Only.
*
******************************************************************************/
static AieRC _XAie_DmaLockConfig(XAie_DmaDesc *DmaDesc, XAie_Lock Acq,
		XAie_Lock Rel, u8 AcqEn, u8 RelEn)
{
	const XAie_DmaMod *DmaMod;
	const XAie_LockMod *LockMod;

	if((DmaDesc == XAIE_NULL) ||
			(DmaDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAieLib_print("Error: Invalid Arguments\n");
		return XAIE_INVALID_ARGS;
	}

	LockMod = DmaDesc->LockMod;
	if((Acq.LockId >LockMod->NumLocks) ||
			(Acq.LockVal > LockMod->LockValUpperBound) ||
			(Rel.LockVal > LockMod->LockValUpperBound)) {
		XAieLib_print("Error: Invalid Lock\n");
		return XAIE_INVALID_LOCK_ID;
	}

	DmaMod = DmaDesc->DmaMod;

	return DmaMod->SetLock(DmaDesc, Acq, Rel, AcqEn, RelEn);
}

/*****************************************************************************/
/**
*
* This API initializes the Acquire and Release Locks for a a Dma descriptor and
* enables the lock in the dma descriptor.
*
* @param	DmaDesc: Initialized Dma Descriptor.
* @param	Acq: Lock object with acquire lock ID and lock value.
* @param	Rel: Lock object with release lock ID and lock value.
*
* @note		The API sets up the value in the dma descriptor and does not
*		configure the buffer descriptor field in the hardware.
*
******************************************************************************/
AieRC XAie_DmaSetLock(XAie_DmaDesc *DmaDesc, XAie_Lock Acq, XAie_Lock Rel)
{
	return _XAie_DmaLockConfig(DmaDesc, Acq, Rel, XAIE_ENABLE, XAIE_ENABLE);
}

/*****************************************************************************/
/**
*
* This api sets up the packet id and packet type for a dma descriptor.
*
* @param	DmaDesc: Initialized dma descriptor.
* @param	Pkt: Pkt object with acquire packet id and packet type.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		The API sets up the value in the dma descriptor and does not
*		configure the buffer descriptor field in the hardware.
*
******************************************************************************/
AieRC XAie_DmaSetPkt(XAie_DmaDesc *DmaDesc, XAie_Packet Pkt)
{
	if((DmaDesc == XAIE_NULL) ||
			(DmaDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAieLib_print("Error: Invalid Arguments\n");
		return XAIE_INVALID_ARGS;
	}

	DmaDesc->PktDesc.PktEn = XAIE_ENABLE;
	DmaDesc->PktDesc.PktId = Pkt.PktId;
	DmaDesc->PktDesc.PktType = Pkt.PktType;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This api sets up the value of inserted out of order ID field in packet header.
*
* @param	DmaDesc: Initialized dma descriptor.
* @param	OutofOrderBdId: Value of out of order ID field in packet header.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		The api sets up the value in the dma descriptor and does not
*		configure the buffer descriptor field in the hardware. This API
*		works only for AIE2 and has no effect on AIE. The buffer
*		descriptor with this feature enabled can work only for MM2S
*		channels.
*
******************************************************************************/
AieRC XAie_DmaSetOutofOrderBdId(XAie_DmaDesc *DmaDesc, u8 OutofOrderBdId)
{
	const XAie_DmaMod *DmaMod;

	if((DmaDesc == XAIE_NULL) ||
			(DmaDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAieLib_print("Error: Invalid Arguments\n");
		return XAIE_INVALID_ARGS;
	}

	DmaMod = DmaDesc->DmaMod;
	if(DmaMod->OutofOrderBdId == XAIE_FEATURE_UNAVAILABLE) {
		XAieLib_print("Error: Feature unavailable\n");
		return XAIE_FEATURE_NOT_SUPPORTED;
	}

	DmaDesc->BdEnDesc.OutofOrderBdId = OutofOrderBdId;
	DmaDesc->EnOutofOrderBdId = XAIE_ENABLE;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API setups the Buffer starting address, Acquire lock and release lock for
* DMA Descriptor and enables the double buffering mode. This API is supported
* for AIE only.
*
* @param	DmaDesc: Initialized Dma Descriptor.
* @param	Addr: Buffer Address for the 2nd buffer in double buffer mode.
* @param	Acq: Lock object with acquire lock ID and lock value.
* @param	Rel: Lock object with release lock ID and lock value.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		The Acquire and release locks for the double buffers are enabled
*		by default. The API sets up the value in the dma descriptor and
*		does not configure the buffer descriptor field in the hardware.
*
******************************************************************************/
AieRC XAie_DmaSetDoubleBuffer(XAie_DmaDesc *DmaDesc, u64 Addr, XAie_Lock Acq,
		XAie_Lock Rel)
{
	const XAie_DmaMod *DmaMod;
	const XAie_LockMod *LockMod;

	if((DmaDesc == XAIE_NULL) ||
			(DmaDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAieLib_print("Error: Invalid Arguments\n");
		return XAIE_INVALID_ARGS;
	}

	DmaMod = DmaDesc->DmaMod;
	if(DmaMod->DoubleBuffering == XAIE_FEATURE_UNAVAILABLE) {
		XAieLib_print("Error: Feature not supported\n");
		return XAIE_FEATURE_NOT_SUPPORTED;
	}

	if(((Addr & DmaMod->BdProp->AddrAlignMask) != 0U) ||
			((Addr + DmaDesc->AddrDesc.Length) &
			 ~DmaMod->BdProp->AddrMask)) {
		XAieLib_print("Error: Invalid Address\n");
		return XAIE_INVALID_ADDRESS;
	}

	LockMod = DmaDesc->LockMod;
	if((Acq.LockId > LockMod->NumLocks) || (Acq.LockId != Rel.LockId)) {
		XAieLib_print("Error: Invalid Lock\n");
		return XAIE_INVALID_LOCK_ID;
	}

	DmaDesc->AddrDesc_2.Address = Addr >> DmaMod->BdProp->AddrAlignShift;
	DmaDesc->EnDoubleBuff = XAIE_ENABLE;

	/* For AIE, Acquire and Release Lock IDs must be the same */
	DmaDesc->LockDesc_2.LockAcqId = Acq.LockId;
	DmaDesc->LockDesc_2.LockRelId = Rel.LockId;
	DmaDesc->LockDesc_2.LockAcqEn = XAIE_ENABLE;
	DmaDesc->LockDesc_2.LockRelEn = XAIE_ENABLE;

	if(Rel.LockVal != XAIE_LOCK_WITH_NO_VALUE) {
		DmaDesc->LockDesc_2.LockRelValEn = XAIE_ENABLE;
		DmaDesc->LockDesc_2.LockRelVal = Rel.LockVal;
	}

	if(Acq.LockVal != XAIE_LOCK_WITH_NO_VALUE) {
		DmaDesc->LockDesc_2.LockAcqValEn = XAIE_ENABLE;
		DmaDesc->LockDesc_2.LockAcqVal = Acq.LockVal;
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API setups the Buffer starting address and the buffer length of the DMA
* Descriptor.
*
* @param	DmaDesc: Initialized Dma Descriptor.
* @param	Addr: Buffer address.
* @param	Len: Length of the buffer in bytes.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		If the dma is being used in double buffer mode, the Len argument
*		passed to this API should include the size for both buffers.
*		The API sets up the value in the dma descriptor and does not
*		configure the buffer descriptor field in the hardware.
*
******************************************************************************/
AieRC XAie_DmaSetAddrLen(XAie_DmaDesc *DmaDesc, u64 Addr, u32 Len)
{
	const XAie_DmaMod *DmaMod;

	if((DmaDesc == XAIE_NULL) ||
			(DmaDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAieLib_print("Error: Invalid Arguments\n");
		return XAIE_INVALID_ARGS;
	}

	DmaMod = DmaDesc->DmaMod;
	if(((Addr & DmaMod->BdProp->AddrAlignMask) != 0U) ||
			((Addr + Len) & ~DmaMod->BdProp->AddrMask)) {
		XAieLib_print("Error: Invalid Address\n");
		return XAIE_INVALID_ADDRESS;
	}

	DmaDesc->AddrDesc.Address = Addr >> DmaMod->BdProp->AddrAlignShift;
	DmaDesc->AddrDesc.Length = (Len >> XAIE_DMA_32BIT_TXFER_LEN) -
		DmaMod->BdProp->LenActualOffset;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API setups the Buffer starting address, the buffer length and the address
* mode of the DMA Descriptor.
*
* @param	DmaDesc: Initialized Dma Descriptor.
* @param	Tensor: Dma Tensor describing the address mode of dma. The user
*		should setup the tensor based on the hardware generation.
* @param	Addr: Buffer address.
* @param	Len: Length of the buffer in bytes.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_DmaSetMultiDimAddr(XAie_DmaDesc *DmaDesc, XAie_DmaTensor *Tensor,
		u64 Addr, u32 Len)
{
	const XAie_DmaMod *DmaMod;

	if((DmaDesc == XAIE_NULL) || (Tensor == XAIE_NULL) ||
			(DmaDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAieLib_print("Error: Invalid Arguments\n");
		return XAIE_INVALID_ARGS;
	}

	DmaMod = DmaDesc->DmaMod;
	if(((Addr & DmaMod->BdProp->AddrAlignMask) != 0U) ||
			((Addr + Len) & ~DmaMod->BdProp->AddrMask)) {
		XAieLib_print("Error: Invalid Address\n");
		return XAIE_INVALID_ADDRESS;
	}

	if(Tensor->NumDim > DmaMod->NumAddrDim) {
		XAieLib_print("Error: Tensor dimension not supported\n");
		return XAIE_FEATURE_NOT_SUPPORTED;
	}

	DmaDesc->AddrDesc.Address = Addr >> DmaMod->BdProp->AddrAlignShift;
	DmaDesc->AddrDesc.Length = (Len >> XAIE_DMA_32BIT_TXFER_LEN) -
		DmaMod->BdProp->LenActualOffset;

	return	DmaMod->SetMultiDim(DmaDesc, Tensor);
}

/*****************************************************************************/
/**
*
* This API enables the compression bit in the DMA Descriptor.
*
* @param	DmaDesc: Initialized Dma Descriptor.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		The API sets up the value in the dma descriptor and does not
*		configure the buffer descriptor field in the hardware.
*
******************************************************************************/
AieRC XAie_DmaEnableCompression(XAie_DmaDesc *DmaDesc)
{
	const XAie_DmaMod *DmaMod;

	if((DmaDesc == XAIE_NULL) ||
			(DmaDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAieLib_print("Error: Invalid Arguments\n");
		return XAIE_INVALID_ARGS;
	}

	DmaMod = DmaDesc->DmaMod;
	if(DmaMod->Compression == XAIE_FEATURE_UNAVAILABLE) {
		XAieLib_print("Error: Feature not supported\n");
		return XAIE_FEATURE_NOT_SUPPORTED;
	}

	DmaDesc->EnCompression = XAIE_ENABLE;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API setups the Enable Bd, Next Bd and UseNxtBd fields in the DMA
* Descriptor.
*
* @param	DmaDesc: Initialized Dma Descriptor.
* @param	NextBd: NextBd to run after the current Bd finishes execution..
* @param	EnableNextBd: XAIE_ENABLE/XAIE_DISABLE to enable or disable
*		the next BD in DMA Descriptor.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		The API sets up the value in the dma descriptor and does not
*		configure the buffer descriptor field in the hardware.
*
******************************************************************************/
AieRC XAie_DmaSetNextBd(XAie_DmaDesc *DmaDesc, u8 NextBd, u8 EnableNextBd)
{
	const XAie_DmaMod *DmaMod;

	if((DmaDesc == XAIE_NULL) ||
			(DmaDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAieLib_print("Error: Invalid Arguments\n");
		return XAIE_INVALID_ARGS;
	}

	DmaMod = DmaDesc->DmaMod;
	if(NextBd > DmaMod->NumBds) {
		XAieLib_print("Error: Invalid Next Bd\n");
		return XAIE_INVALID_BD_NUM;
	}

	DmaDesc->BdEnDesc.NxtBd = NextBd;
	DmaDesc->BdEnDesc.UseNxtBd = EnableNextBd;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API enables the Enable Bd bit in the DMA Descriptor.
*
* @param	DmaDesc: Initialized Dma Descriptor.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		The API sets up the value in the dma descriptor and does not
*		configure the buffer descriptor field in the hardware.
*
******************************************************************************/
AieRC XAie_DmaEnableBd(XAie_DmaDesc *DmaDesc)
{
	if((DmaDesc == XAIE_NULL) ||
			(DmaDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAieLib_print("Error: Invalid Arguments\n");
		return XAIE_INVALID_ARGS;
	}

	DmaDesc->BdEnDesc.ValidBd = XAIE_ENABLE;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API disables the Enable Bd bit in the DMA Descriptor.
*
* @param	DmaDesc: Initialized Dma Descriptor.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		The API sets up the value in the dma descriptor and does not
*		configure the buffer descriptor field in the hardware.
*
******************************************************************************/
AieRC XAie_DmaDisableBd(XAie_DmaDesc *DmaDesc)
{
	if((DmaDesc == XAIE_NULL) ||
			(DmaDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAieLib_print("Error: Invalid Arguments\n");
		return XAIE_INVALID_ARGS;
	}

	DmaDesc->BdEnDesc.ValidBd = XAIE_DISABLE;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API setups the Enable Bd, Next Bd and UseNxtBd fields in the DMA
* Descriptor.
*
* @param	DmaDesc: Initialized Dma Descriptor.
* @param	Smid: SMID for the buffer descriptor.
* @param	BurstLen: BurstLength for the buffer descriptor (4, 8 or 16).
* @param	Qos: AXI Qos bits for the AXI-MM transfer.
* @param	Cache: AxCACHE bits for AXI-MM transfer.
* @param	Secure: Secure status of the transfer(1-Secure, 0-Non Secure).
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		The API sets up the value in the dma descriptor and does not
*		configure the buffer descriptor field in the hardware.
*
******************************************************************************/
AieRC XAie_DmaSetAxi(XAie_DmaDesc *DmaDesc, u8 Smid, u8 BurstLen, u8 Qos,
		u8 Cache, u8 Secure)
{
	if((DmaDesc == XAIE_NULL) ||
			(DmaDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAieLib_print("Error: Invalid Arguments\n");
		return XAIE_INVALID_ARGS;
	}

	if(DmaDesc->TileType != XAIEGBL_TILE_TYPE_SHIMNOC) {
		XAieLib_print("Error: No Axi properties for tile type\n");
		return XAIE_INVALID_TILE;
	}

	if((BurstLen != 4U) && (BurstLen != 8U) && (BurstLen != 16U)) {
		XAieLib_print("Error: Invalid Burst length\n");
		return XAIE_INVALID_BURST_LENGTH;
	}

	DmaDesc->AxiDesc.SMID = Smid;
	DmaDesc->AxiDesc.BurstLen = BurstLen >> XAIE_SHIM_BLEN_SHIFT;
	DmaDesc->AxiDesc.AxQos = Qos;
	DmaDesc->AxiDesc.AxCache = Cache;
	DmaDesc->AxiDesc.SecureAccess = Secure;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API setups DmaDesc with parameters to run dma in interleave mode.
*
* @param	DmaDesc: Initialized Dma Descriptor.
* @param	DoubleBuff: Double buffer to use(0 - A, 1-B)
* @param	IntrleaveCount: Interleaved count to use(to be 32b word aligned)
* @param	IntrleaveCurr: Interleave current pointer.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		The API sets up the value in the dma descriptor and does not
*		configure the buffer descriptor field in the hardware.
*
******************************************************************************/
AieRC XAie_DmaSetInterleaveEnable(XAie_DmaDesc *DmaDesc, u8 DoubleBuff,
		u8 IntrleaveCount, u16 IntrleaveCurr)
{
	const XAie_DmaMod *DmaMod;

	if((DmaDesc == XAIE_NULL) ||
			(DmaDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAieLib_print("Error: Invalid Arguments\n");
		return XAIE_INVALID_ARGS;
	}

	DmaMod = DmaDesc->DmaMod;
	if(DmaMod->InterleaveMode == XAIE_FEATURE_UNAVAILABLE) {
		XAieLib_print("Error: Feature unavailable\n");
		return XAIE_FEATURE_NOT_SUPPORTED;
	}

	return DmaMod->SetIntrleave(DmaDesc, DoubleBuff, IntrleaveCount,
			IntrleaveCurr);
}

/*****************************************************************************/
/**
*
* This API writes a Dma Descriptor which is initialized and setup by other APIs
* into the corresponding registers and register fields in the hardware.
*
* @param	DmaDesc: Initialized Dma Descriptor.
* @param	Loc: Location of AIE Tile
* @param	BdNum: Hardware BD number to be written to.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_DmaWriteBd(XAie_DevInst *DevInst, XAie_DmaDesc *DmaDesc,
		XAie_LocType Loc, u8 BdNum)
{
	const XAie_DmaMod *DmaMod;

	if((DmaDesc == XAIE_NULL) ||
			(DmaDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAieLib_print("Error: Invalid Arguments\n");
		return XAIE_INVALID_ARGS;
	}

	if(DmaDesc->TileType != _XAie_GetTileTypefromLoc(DevInst, Loc)) {
		XAieLib_print("Error: Tile type mismatch\n");
		return XAIE_INVALID_TILE;
	}

	DmaMod = DmaDesc->DmaMod;
	if(BdNum > DmaMod->NumBds) {
		XAieLib_print("Error: Invalid BD number\n");
		return XAIE_INVALID_BD_NUM;
	}

	return DmaMod->WriteBd(DevInst, DmaDesc, Loc, BdNum);
}

/*****************************************************************************/
/**
*
* This API resets a DMA Channel for a given AIE Tile.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	ChNum: Channel number of the DMA.
* @param	Dir: Direction of the DMA Channel. (MM2S or S2MM)
* @param	Reset: DMA_CHANNEL_UNRESET/RESET
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Returns error for SHIMNOC tiles.
*
******************************************************************************/
AieRC XAie_DmaChannelReset(XAie_DevInst *DevInst, XAie_LocType Loc, u8 ChNum,
		XAie_DmaDirection Dir, XAie_DmaChReset Reset)
{
	u8 TileType;
	u64 Addr;
	u32 Val;
	const XAie_DmaMod *DmaMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAieLib_print("Error: Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	if((TileType == XAIEGBL_TILE_TYPE_SHIMPL) ||
			(TileType == XAIEGBL_TILE_TYPE_SHIMNOC)) {
		XAieLib_print("Error: Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	DmaMod = DevInst->DevProp.DevMod[TileType].DmaMod;
	if(ChNum > DmaMod->NumChannels) {
		XAieLib_print("Error: Invalid Channel number\n");
		return XAIE_INVALID_CHANNEL_NUM;
	}

	Addr = DevInst->BaseAddr +
		_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
		DmaMod->ChCtrlBase + ChNum * DmaMod->ChIdxOffset +
		Dir * DmaMod->ChIdxOffset * DmaMod->NumChannels;

	Val = XAie_SetField(Reset, DmaMod->ChProp->Reset.Lsb,
			DmaMod->ChProp->Reset.Mask);

	XAieGbl_MaskWrite32(Addr, Val, DmaMod->ChProp->Reset.Mask);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API resets all the Dma Channels of an AIE Tile.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	Reset: DMA_CHANNEL_UNRESET/RESET
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Returns error for SHIMNOC tiles.
*
******************************************************************************/
AieRC XAie_DmaChannelResetAll(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_DmaChReset Reset)
{
	u8 TileType;
	AieRC RC;
	const XAie_DmaMod *DmaMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAieLib_print("Error: Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_SHIMPL) {
		XAieLib_print("Error: Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	DmaMod = DevInst->DevProp.DevMod[TileType].DmaMod;

	/* Reset MM2S */
	for(u8 i = 0U; i < DmaMod->NumChannels; i++) {
		RC = XAie_DmaChannelReset(DevInst, Loc, i, DMA_MM2S, Reset);
		if(RC != XAIE_OK) return RC;
	}

	/* Reset S2MM */
	for(u8 i = 0U; i < DmaMod->NumChannels; i++) {
		RC = XAie_DmaChannelReset(DevInst, Loc, i, DMA_S2MM, Reset);
		if(RC != XAIE_OK) return RC;
	}

	return  XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API pauses the stream via SHIM Dma for Shim NoC tiles.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	ChNum: Channel number of the DMA.
* @param	Dir: Direction of the SHIM DMA Channel. (MM2S or S2MM)
* @param	Pause: XAIE_ENABLE to Pause or XAIE_DISABLE to un-pause.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		For AIE Shim Noc Tiles only.
*
******************************************************************************/
AieRC XAie_DmaChannelPauseStream(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 ChNum, XAie_DmaDirection Dir, u8 Pause)
{
	u8 TileType;
	u32 Value;
	u64 Addr;
	const XAie_DmaMod *DmaMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAieLib_print("Error: Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	if(DevInst->DevProp.DevGen != XAIE_DEV_GEN_AIE) {
		XAieLib_print("Error: Shim stream pause not supported\n");
		return XAIE_ERR;
	}

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	if(TileType != XAIEGBL_TILE_TYPE_SHIMNOC) {
		XAieLib_print("Error: Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	DmaMod = DevInst->DevProp.DevMod[TileType].DmaMod;
	if(ChNum > DmaMod->NumChannels) {
		XAieLib_print("Error: Invalid Channel number\n");
		return XAIE_INVALID_CHANNEL_NUM;
	}

	Value = XAie_SetField(Pause, DmaMod->ChProp->PauseStream.Lsb,
			DmaMod->ChProp->PauseStream.Mask);

	Addr = DevInst->BaseAddr +
		_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
		DmaMod->ChCtrlBase + ChNum * DmaMod->ChIdxOffset +
		Dir * DmaMod->ChIdxOffset * DmaMod->NumChannels;

	XAieGbl_MaskWrite32(Addr, DmaMod->ChProp->PauseStream.Mask, Value);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API pauses the AXI-MM transactions on SHIM Dma for Shim NoC tiles.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	ChNum: Channel number of the DMA.
* @param	Dir: Direction of the SHIM DMA Channel. (MM2S or S2MM)
* @param	Pause: XAIE_ENABLE to Pause or XAIE_DISABLE to un-pause.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		For AIE Shim Noc Tiles only.
*
******************************************************************************/
AieRC XAie_DmaChannelPauseMem(XAie_DevInst *DevInst, XAie_LocType Loc, u8 ChNum,
		XAie_DmaDirection Dir, u8 Pause)
{
	u8 TileType;
	u32 Value;
	u64 Addr;
	const XAie_DmaMod *DmaMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAieLib_print("Error: Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	if(DevInst->DevProp.DevGen != XAIE_DEV_GEN_AIE) {
		XAieLib_print("Error: Shim stream pause not supported\n");
		return XAIE_ERR;
	}

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	if(TileType != XAIEGBL_TILE_TYPE_SHIMNOC) {
		XAieLib_print("Error: Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	DmaMod = DevInst->DevProp.DevMod[TileType].DmaMod;
	if(ChNum > DmaMod->NumChannels) {
		XAieLib_print("Error: Invalid Channel number\n");
		return XAIE_INVALID_CHANNEL_NUM;
	}

	Value = XAie_SetField(Pause, DmaMod->ChProp->PauseMem.Lsb,
			DmaMod->ChProp->PauseMem.Mask);
	Addr = DevInst->BaseAddr +
		_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
		DmaMod->ChCtrlBase + ChNum * DmaMod->ChIdxOffset +
		Dir * DmaMod->ChIdxOffset * DmaMod->NumChannels;

	XAieGbl_MaskWrite32(Addr, DmaMod->ChProp->PauseMem.Mask, Value);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API configures the Dma channel for a particular range of tiles. This API
* has to be called before enabling the channel. The OutofOrderBdId and
* Compression/Decompression are enabled automatically if the Dma Descriptor has
* these features enabled.
*
* @param	DevInst: Device Instance.
* @param	DmaDesc: Initialized Dma Descriptor.
* @param	Loc: Location of AIE Tile
* @param	ChNum: Channel number of the DMA.
* @param	Dir: Direction of the DMA Channel. (MM2S or S2MM)
* @param	RepeatCount: Number of times to repeat the task.
* @param	EnTokenIssue: XAIE_ENABLE to issue token when task is complete.
* @param	ControllerId: Task complete token controller ID field
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_DmaChannelConfig(XAie_DevInst *DevInst, XAie_DmaDesc *DmaDesc,
		XAie_LocType Loc, u8 ChNum, XAie_DmaDirection Dir,
		u8 RepeatCount, u8 EnTokenIssue, u8 ControllerId)
{
	u64 Addr;
	u32 ChWord[XAIE_DMA_CHCTRL_NUM_WORDS] = {0U};
	u32 ChWordMask[XAIE_DMA_CHCTRL_NUM_WORDS] = {0U};
	const XAie_DmaMod *DmaMod;

	if((DevInst == XAIE_NULL) || (DmaDesc == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAieLib_print("Error: Invalid arguments\n");
		return XAIE_INVALID_ARGS;
	}

	if(DevInst->DevProp.DevGen == XAIE_DEV_GEN_AIE) {
		XAieLib_print("Error: Feature not supported\n");
		return XAIE_FEATURE_NOT_SUPPORTED;
	}

	if((DmaDesc->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAieLib_print("Error: Dma descriptor not initilized\n");
		return XAIE_INVALID_DMA_DESC;
	}

	if(DmaDesc->TileType != _XAie_GetTileTypefromLoc(DevInst, Loc)) {
		XAieLib_print("Error: Tile type mismatch\n");
		return XAIE_INVALID_TILE;
	}

	DmaMod = DevInst->DevProp.DevMod[DmaDesc->TileType].DmaMod;
	if(ChNum > DmaMod->NumChannels) {
		XAieLib_print("Error: Invalid Channel number\n");
		return XAIE_INVALID_CHANNEL_NUM;
	}

	/*
	 * Register fields for these configurations are different for tile
	 * types. The below implementation automatically sets up the correct
	 * register values based on the tile type.
	 */
	ChWord[DmaMod->ChProp->CtrlId.Idx] |= XAie_SetField(ControllerId,
			DmaMod->ChProp->CtrlId.Lsb,
			DmaMod->ChProp->CtrlId.Mask);
	ChWordMask[DmaMod->ChProp->CtrlId.Idx] |= DmaMod->ChProp->CtrlId.Mask;

	ChWord[DmaMod->ChProp->RptCount.Idx] |= XAie_SetField(RepeatCount,
			DmaMod->ChProp->RptCount.Lsb,
			DmaMod->ChProp->RptCount.Mask);
	ChWordMask[DmaMod->ChProp->RptCount.Idx] |=
		DmaMod->ChProp->RptCount.Mask;

	ChWord[DmaMod->ChProp->EnToken.Idx] |= XAie_SetField(EnTokenIssue,
			DmaMod->ChProp->EnToken.Lsb,
			DmaMod->ChProp->EnToken.Mask);
	ChWordMask[DmaMod->ChProp->EnToken.Idx] |= DmaMod->ChProp->EnToken.Mask;

	ChWord[DmaMod->ChProp->EnCompression.Idx] |=
		XAie_SetField(DmaDesc->EnCompression,
				DmaMod->ChProp->EnCompression.Lsb,
				DmaMod->ChProp->EnCompression.Mask);
	ChWordMask[DmaMod->ChProp->EnCompression.Idx] |=
		DmaMod->ChProp->EnCompression.Mask;

	ChWord[DmaMod->ChProp->EnOutofOrder.Idx] |=
		XAie_SetField(DmaDesc->EnOutofOrderBdId,
				DmaMod->ChProp->EnOutofOrder.Lsb,
				DmaMod->ChProp->EnOutofOrder.Mask);
	ChWordMask[DmaMod->ChProp->EnOutofOrder.Idx] |=
		DmaMod->ChProp->EnOutofOrder.Mask;

	Addr = DevInst->BaseAddr +
		_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
		DmaMod->ChCtrlBase + ChNum * DmaMod->ChIdxOffset +
		Dir * DmaMod->ChIdxOffset * DmaMod->NumChannels;

	for(u8 i = 0U; i < XAIE_DMA_CHCTRL_NUM_WORDS; i++) {
		XAieGbl_MaskWrite32(Addr + (i * 4U),
				ChWordMask[i], ChWord[i]);
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API pushes a Buffer Descriptor onto the MM2S or S2MM Channel queue.
*
* @param	DevInst: Device Instance.
* @param	Loc: Location of AIE Tile
* @param	ChNum: Channel number of the DMA.
* @param	Dir: Direction of the DMA Channel. (MM2S or S2MM)
* @param	BdNum: Bd number to be pushed to the queue.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		If there is no bit to enable the channel in AIE DMAs,
*		pushing a Buffer descriptor number onto the queue starts the
*		channel.
*
******************************************************************************/
AieRC XAie_DmaChannelPushBdToQueue(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 ChNum, XAie_DmaDirection Dir, u8 BdNum)
{
	u8 TileType;
	u64 Addr;
	const XAie_DmaMod *DmaMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAieLib_print("Error: Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_SHIMPL) {
		XAieLib_print("Error: Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	DmaMod = DevInst->DevProp.DevMod[TileType].DmaMod;
	if(ChNum > DmaMod->NumChannels) {
		XAieLib_print("Error: Invalid Channel number\n");
		return XAIE_INVALID_CHANNEL_NUM;
	}

	if(BdNum > DmaMod->NumBds) {
		XAieLib_print("Error: Invalid BD number\n");
		return XAIE_INVALID_BD_NUM;
	}

	Addr = DevInst->BaseAddr +
		_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
		DmaMod->ChCtrlBase + ChNum * DmaMod->ChIdxOffset +
		Dir * DmaMod->ChIdxOffset * DmaMod->NumChannels;

	XAieGbl_MaskWrite32(Addr + (DmaMod->ChProp->StartBd.Idx * 4U),
			DmaMod->ChProp->StartBd.Mask, BdNum);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API Enables or Disables a S2MM or MM2S channel of AIE DMAs.
*
* @param	DevInst: Device Instance.
* @param	Loc: Location of AIE Tile
* @param	ChNum: Channel number of the DMA.
* @param	Dir: Direction of the DMA Channel. (MM2S or S2MM)
* @param	Enable: XAIE_ENABLE/XAIE_DISABLE to enable/disable.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only.
*
******************************************************************************/
static AieRC _XAie_DmaChannelControl(XAie_DevInst *DevInst, XAie_LocType Loc,
		u8 ChNum, XAie_DmaDirection Dir, u8 Enable)
{
	u8 TileType;
	u64 Addr;
	const XAie_DmaMod *DmaMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAieLib_print("Error: Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_SHIMPL) {
		XAieLib_print("Error: Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	DmaMod = DevInst->DevProp.DevMod[TileType].DmaMod;
	if(ChNum > DmaMod->NumChannels) {
		XAieLib_print("Error: Invalid Channel number\n");
		return XAIE_INVALID_CHANNEL_NUM;
	}

	Addr = DevInst->BaseAddr +
		_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
		DmaMod->ChCtrlBase + ChNum * DmaMod->ChIdxOffset +
		Dir * DmaMod->ChIdxOffset * DmaMod->NumChannels;

	XAieGbl_MaskWrite32(Addr + (DmaMod->ChProp->Enable.Idx * 4U),
			DmaMod->ChProp->Enable.Mask, Enable);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API Enables a S2MM or MM2S channel of AIE DMAs.
*
* @param	DevInst: Device Instance.
* @param	Loc: Location of AIE Tile
* @param	ChNum: Channel number of the DMA.
* @param	Dir: Direction of the DMA Channel. (MM2S or S2MM)
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_DmaChannelEnable(XAie_DevInst *DevInst, XAie_LocType Loc, u8 ChNum,
		XAie_DmaDirection Dir)
{
	return _XAie_DmaChannelControl(DevInst, Loc, ChNum, Dir, XAIE_ENABLE);
}

/*****************************************************************************/
/**
*
* This API Disables a S2MM or MM2S channel of AIE DMAs.
*
* @param	DevInst: Device Instance.
* @param	Loc: Location of AIE Tile
* @param	ChNum: Channel number of the DMA.
* @param	Dir: Direction of the DMA Channel. (MM2S or S2MM)
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_DmaChannelDisable(XAie_DevInst *DevInst, XAie_LocType Loc, u8 ChNum,
		XAie_DmaDirection Dir)
{
	return _XAie_DmaChannelControl(DevInst, Loc, ChNum, Dir, XAIE_DISABLE);
}

/** @} */
