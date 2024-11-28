/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_dma_aie.c
* @{
*
* This file contains routines for AIE DMA configuration and controls.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   03/23/2020  Initial creation
* 1.1   Tejus   06/10/2020  Switch to new io backend apis.
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xaie_feature_config.h"
#include "xaie_helper.h"
#include "xaie_io.h"
#include "xaiegbl.h"
#include "xaiegbl_regdef.h"
#include "xaie_helper_internal.h"

#ifdef XAIE_FEATURE_DMA_ENABLE

/************************** Constant Definitions *****************************/
#define XAIE_DMA_TILEDMA_2DX_DEFAULT_INCR		0U
#define XAIE_DMA_TILEDMA_2DX_DEFAULT_WRAP 		255U
#define XAIE_DMA_TILEDMA_2DX_DEFAULT_OFFSET		1U
#define XAIE_DMA_TILEDMA_2DY_DEFAULT_INCR 		255U
#define XAIE_DMA_TILEDMA_2DY_DEFAULT_WRAP 		255U
#define XAIE_DMA_TILEDMA_2DY_DEFAULT_OFFSET		256U

#define XAIE_TILEDMA_NUM_BD_WORDS			7U
#define XAIE_SHIMDMA_NUM_BD_WORDS			5U

#define XAIE_TILE_DMA_NUM_DIMS_MAX			2U
#define XAIE_DMA_STATUS_IDLE				0x0U
/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This API initializes the Dma Descriptor for AIE 1 Tile Dma.
*
* @param	Desc: Dma Descriptor
*
* @return	None.
*
* @note		Internal Only.
*
******************************************************************************/
void _XAie_TileDmaInit(XAie_DmaDesc *Desc)
{
	Desc->MultiDimDesc.AieMultiDimDesc.X_Incr = XAIE_DMA_TILEDMA_2DX_DEFAULT_INCR;
	Desc->MultiDimDesc.AieMultiDimDesc.X_Wrap = XAIE_DMA_TILEDMA_2DX_DEFAULT_WRAP;
	Desc->MultiDimDesc.AieMultiDimDesc.X_Offset = XAIE_DMA_TILEDMA_2DX_DEFAULT_OFFSET;
	Desc->MultiDimDesc.AieMultiDimDesc.Y_Incr = XAIE_DMA_TILEDMA_2DY_DEFAULT_INCR;
	Desc->MultiDimDesc.AieMultiDimDesc.Y_Wrap = XAIE_DMA_TILEDMA_2DY_DEFAULT_WRAP;
	Desc->MultiDimDesc.AieMultiDimDesc.Y_Offset = XAIE_DMA_TILEDMA_2DY_DEFAULT_OFFSET;

	return;
}

/*****************************************************************************/
/**
*
* This API initializes the Dma Descriptor for AIE 1 Shim Dma.
*
* @param	Desc: Dma Descriptor
*
* @return	None.
*
* @note		Internal Only.
*
******************************************************************************/
void _XAie_ShimDmaInit(XAie_DmaDesc *Desc)
{
	(void)Desc;
	return;
}

/*****************************************************************************/
/**
*
* This API initializes the Acquire and Release Locks for a a Dma for AIE
* descriptor.
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
* @note		Internal Only. Should not be called directly. This function is
*		called from the internal Dma Module data structure.
*
******************************************************************************/
AieRC _XAie_DmaSetLock(XAie_DmaDesc *DmaDesc, XAie_Lock Acq, XAie_Lock Rel,
		u8 AcqEn, u8 RelEn)
{
	/* For AIE, Acquire and Release Lock IDs must be the same */
	if((Acq.LockId != Rel.LockId)) {
		XAIE_ERROR("Lock ID is invalid\n");
		return XAIE_INVALID_LOCK_ID;
	}

	DmaDesc->LockDesc.LockAcqId = Acq.LockId;
	DmaDesc->LockDesc.LockRelId = Rel.LockId;
	DmaDesc->LockDesc.LockAcqEn = AcqEn;
	DmaDesc->LockDesc.LockRelEn = RelEn;
	DmaDesc->LockDesc.LockRelValEn = 0U;
	DmaDesc->LockDesc.LockAcqValEn = 0U;

	/* If lock release value is invalid,then lock released with no value */
	if(Rel.LockVal != XAIE_LOCK_WITH_NO_VALUE) {
		DmaDesc->LockDesc.LockRelValEn = XAIE_ENABLE;
		DmaDesc->LockDesc.LockRelVal = Rel.LockVal;
	}

	/* If lock acquire value is invalid,then lock acquired with no value */
	if(Acq.LockVal != XAIE_LOCK_WITH_NO_VALUE) {
		DmaDesc->LockDesc.LockAcqValEn = XAIE_ENABLE;
		DmaDesc->LockDesc.LockAcqVal = Acq.LockVal;
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API setups the DmaDesc with the register fields required for the dma
* addressing mode of AIE.
*
* @param	DmaDesc: Initialized Dma Descriptor.
* @param	Tensor: Dma Tensor describing the address mode of dma.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal API only.
*
******************************************************************************/
AieRC _XAie_DmaSetMultiDim(XAie_DmaDesc *DmaDesc, XAie_DmaTensor *Tensor)
{
	DmaDesc->MultiDimDesc.AieMultiDimDesc.X_Offset =
		(u16)Tensor->Dim[0].AieDimDesc.Offset;
	DmaDesc->MultiDimDesc.AieMultiDimDesc.X_Wrap =
		(u8)Tensor->Dim[0].AieDimDesc.Wrap - 1U;
	DmaDesc->MultiDimDesc.AieMultiDimDesc.X_Incr =
		(u8)Tensor->Dim[0].AieDimDesc.Incr - 1U;
	DmaDesc->MultiDimDesc.AieMultiDimDesc.Y_Offset =
		(u16)Tensor->Dim[1].AieDimDesc.Offset;
	DmaDesc->MultiDimDesc.AieMultiDimDesc.Y_Wrap =
		(u8)Tensor->Dim[1].AieDimDesc.Wrap - 1U;
	DmaDesc->MultiDimDesc.AieMultiDimDesc.Y_Incr =
		(u8)Tensor->Dim[1].AieDimDesc.Incr - 1U;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API setups DmaDesc with parameters to run dma in interleave mode for AIE
*
* @param	DmaDesc: Initialized Dma Descriptor.
* @param	DoubleBuff: Double buffer to use(0 - A, 1-B)
* @param	IntrleaveCount: Interleaved count to use(to be 32b word aligned)
* @param	IntrleaveCurr: Interleave current pointer.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only. The API sets up the value in the dma descriptor
*		and does not configure the buffer descriptor field in the
*		hardware.
*
******************************************************************************/
AieRC _XAie_DmaSetInterleaveEnable(XAie_DmaDesc *DmaDesc, u8 DoubleBuff,
		u8 IntrleaveCount, u16 IntrleaveCurr)
{
	DmaDesc->MultiDimDesc.AieMultiDimDesc.EnInterleaved = XAIE_ENABLE;
	DmaDesc->MultiDimDesc.AieMultiDimDesc.IntrleaveBufSelect = DoubleBuff;
	DmaDesc->MultiDimDesc.AieMultiDimDesc.IntrleaveCount = IntrleaveCount;
	DmaDesc->MultiDimDesc.AieMultiDimDesc.CurrPtr = IntrleaveCurr;

	return XAIE_OK;
}
/*****************************************************************************/
/**
*
* This API writes a Dma Descriptor which is initialized and setup by other APIs
* into the corresponding registers and register fields in the hardware. This API
* is specific to AIE Shim Tiles only.
*
* @param	DevInst: Device Instance
* @param	DmaDesc: Initialized Dma Descriptor.
* @param	Loc: Location of AIE Tile
* @param	BdNum: Hardware BD number to be written to.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only. For AIE Shim Tiles only.
*
******************************************************************************/
AieRC _XAie_ShimDmaWriteBd(XAie_DevInst *DevInst , XAie_DmaDesc *DmaDesc,
		XAie_LocType Loc, u8 BdNum)
{
	u64 Addr;
	u64 BdBaseAddr;
	u32 BdWord[XAIE_SHIMDMA_NUM_BD_WORDS];
	XAie_ShimDmaBdArgs Args;
	const XAie_DmaMod *DmaMod;
	const XAie_DmaBdProp *BdProp;

	DmaMod = DevInst->DevProp.DevMod[DmaDesc->TileType].DmaMod;
	BdProp = DmaMod->BdProp;

	BdBaseAddr = (u64)(DmaMod->BaseAddr + BdNum * (u64)DmaMod->IdxOffset);

	if (_XAie_CheckPrecisionExceeds(BdProp->Buffer->ShimDmaBuff.AddrLow.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->AddrDesc.Address & 0xFFFFFFFFU),
			MAX_VALID_AIE_REG_BIT_INDEX)) {
			XAIE_ERROR("Check Precision Exceeds Failed\n");
			return XAIE_ERR;
		}

	BdWord[0U] = XAie_SetField(DmaDesc->AddrDesc.Address & 0xFFFFFFFFU,
			BdProp->Buffer->ShimDmaBuff.AddrLow.Lsb,
			BdProp->Buffer->ShimDmaBuff.AddrLow.Mask);

	if (_XAie_CheckPrecisionExceeds(BdProp->BufferLen.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->AddrDesc.Length),MAX_VALID_AIE_REG_BIT_INDEX)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}

	BdWord[1U] = XAie_SetField(DmaDesc->AddrDesc.Length,
			BdProp->BufferLen.Lsb, BdProp->BufferLen.Mask);

	if ((_XAie_CheckPrecisionExceeds(BdProp->BdEn->UseNxtBd.Lsb,
				_XAie_MaxBitsNeeded(DmaDesc->BdEnDesc.UseNxtBd),MAX_VALID_AIE_REG_BIT_INDEX)) ||
		(_XAie_CheckPrecisionExceeds(BdProp->BdEn->NxtBd.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->BdEnDesc.NxtBd),MAX_VALID_AIE_REG_BIT_INDEX))||
		(_XAie_CheckPrecisionExceeds(BdProp->Lock->AieDmaLock.LckId_A.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->LockDesc.LockAcqId),MAX_VALID_AIE_REG_BIT_INDEX))||
		(_XAie_CheckPrecisionExceeds(BdProp->Lock->AieDmaLock.LckRelEn_A.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->LockDesc.LockRelEn),MAX_VALID_AIE_REG_BIT_INDEX))||
		(_XAie_CheckPrecisionExceeds(BdProp->Lock->AieDmaLock.LckRelUseVal_A.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->LockDesc.LockRelValEn),MAX_VALID_AIE_REG_BIT_INDEX))||
		(_XAie_CheckPrecisionExceeds(BdProp->Lock->AieDmaLock.LckAcqEn_A.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->LockDesc.LockAcqEn),MAX_VALID_AIE_REG_BIT_INDEX))||
		(_XAie_CheckPrecisionExceeds(BdProp->Lock->AieDmaLock.LckAcqUseVal_A.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->LockDesc.LockAcqValEn),MAX_VALID_AIE_REG_BIT_INDEX))||
		(_XAie_CheckPrecisionExceeds(BdProp->BdEn->ValidBd.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->BdEnDesc.ValidBd),MAX_VALID_AIE_REG_BIT_INDEX))) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}

	BdWord[2U] = XAie_SetField((DmaDesc->AddrDesc.Address >> 32U),
			BdProp->Buffer->ShimDmaBuff.AddrHigh.Lsb,
			BdProp->Buffer->ShimDmaBuff.AddrHigh.Mask) |
		XAie_SetField(DmaDesc->BdEnDesc.UseNxtBd,
				BdProp->BdEn->UseNxtBd.Lsb,
				BdProp->BdEn->UseNxtBd.Mask) |
		XAie_SetField(DmaDesc->BdEnDesc.NxtBd,
				BdProp->BdEn->NxtBd.Lsb,
				BdProp->BdEn->NxtBd.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockAcqId,
				BdProp->Lock->AieDmaLock.LckId_A.Lsb,
				BdProp->Lock->AieDmaLock.LckId_A.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockRelEn,
				BdProp->Lock->AieDmaLock.LckRelEn_A.Lsb,
				BdProp->Lock->AieDmaLock.LckRelEn_A.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockRelVal,
				BdProp->Lock->AieDmaLock.LckRelVal_A.Lsb,
				BdProp->Lock->AieDmaLock.LckRelVal_A.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockRelValEn,
				BdProp->Lock->AieDmaLock.LckRelUseVal_A.Lsb,
				BdProp->Lock->AieDmaLock.LckRelUseVal_A.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockAcqEn,
				BdProp->Lock->AieDmaLock.LckAcqEn_A.Lsb,
				BdProp->Lock->AieDmaLock.LckAcqEn_A.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockAcqVal,
				BdProp->Lock->AieDmaLock.LckAcqVal_A.Lsb,
				BdProp->Lock->AieDmaLock.LckAcqVal_A.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockAcqValEn,
				BdProp->Lock->AieDmaLock.LckAcqUseVal_A.Lsb,
				BdProp->Lock->AieDmaLock.LckAcqUseVal_A.Mask) |
		XAie_SetField(DmaDesc->BdEnDesc.ValidBd,
				BdProp->BdEn->ValidBd.Lsb,
				BdProp->BdEn->ValidBd.Mask);

	if ((_XAie_CheckPrecisionExceeds(BdProp->SysProp->SMID.Lsb,
				_XAie_MaxBitsNeeded(DmaDesc->AxiDesc.SMID),MAX_VALID_AIE_REG_BIT_INDEX)) ||
		(_XAie_CheckPrecisionExceeds(BdProp->SysProp->BurstLen.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->AxiDesc.BurstLen),MAX_VALID_AIE_REG_BIT_INDEX))||
		(_XAie_CheckPrecisionExceeds(BdProp->SysProp->AxQos.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->AxiDesc.AxQos),MAX_VALID_AIE_REG_BIT_INDEX))||
		(_XAie_CheckPrecisionExceeds(BdProp->SysProp->SecureAccess.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->AxiDesc.SecureAccess),MAX_VALID_AIE_REG_BIT_INDEX))||
		(_XAie_CheckPrecisionExceeds(BdProp->SysProp->AxCache.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->AxiDesc.AxCache),MAX_VALID_AIE_REG_BIT_INDEX))) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}


	BdWord[3U] = XAie_SetField(DmaDesc->AxiDesc.SMID,
			BdProp->SysProp->SMID.Lsb, BdProp->SysProp->SMID.Mask) |
		XAie_SetField(DmaDesc->AxiDesc.BurstLen,
				BdProp->SysProp->BurstLen.Lsb,
				BdProp->SysProp->BurstLen.Mask) |
		XAie_SetField(DmaDesc->AxiDesc.AxQos,
				BdProp->SysProp->AxQos.Lsb,
				BdProp->SysProp->AxQos.Mask) |
		XAie_SetField(DmaDesc->AxiDesc.SecureAccess,
				BdProp->SysProp->SecureAccess.Lsb,
				BdProp->SysProp->SecureAccess.Mask) |
		XAie_SetField(DmaDesc->AxiDesc.AxCache,
				BdProp->SysProp->AxCache.Lsb,
				BdProp->SysProp->AxCache.Mask);

	if ((_XAie_CheckPrecisionExceeds(BdProp->Pkt->PktId.Lsb,
				_XAie_MaxBitsNeeded(DmaDesc->PktDesc.PktId),MAX_VALID_AIE_REG_BIT_INDEX)) ||
		(_XAie_CheckPrecisionExceeds(BdProp->Pkt->PktType.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->PktDesc.PktType),MAX_VALID_AIE_REG_BIT_INDEX)) ||
		(_XAie_CheckPrecisionExceeds(BdProp->Pkt->EnPkt.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->PktDesc.PktEn),MAX_VALID_AIE_REG_BIT_INDEX))) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}

	BdWord[4U] = XAie_SetField(DmaDesc->PktDesc.PktId,
			BdProp->Pkt->PktId.Lsb, BdProp->Pkt->PktId.Mask) |
		XAie_SetField(DmaDesc->PktDesc.PktType,
				BdProp->Pkt->PktType.Lsb,
				BdProp->Pkt->PktType.Mask) |
		XAie_SetField(DmaDesc->PktDesc.PktEn,
				BdProp->Pkt->EnPkt.Lsb,
				BdProp->Pkt->EnPkt.Mask);

	Addr = BdBaseAddr + XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	Args.NumBdWords = XAIE_SHIMDMA_NUM_BD_WORDS;
	Args.BdWords = &BdWord[0U];
	Args.Loc = Loc;
	Args.VAddr = DmaDesc->AddrDesc.Address;
	Args.BdNum = BdNum;
	Args.Addr = Addr;
	Args.MemInst = DmaDesc->MemInst;

	XAie_RunOp(DevInst, XAIE_BACKEND_OP_CONFIG_SHIMDMABD, (void *)&Args);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API reads a the data from the buffer descriptor registers to fill the
* DmaDesc structure. This API is meant for AIE Shim Tiles only.
*
* @param	DevInst: Device Instance
* @param	DmaDesc: Dma Descriptor to be filled.
* @param	Loc: Location of AIE Tile
* @param	BdNum: Hardware BD number to be read from.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only. For AIE Shim Tiles only.
*
******************************************************************************/
AieRC _XAie_ShimDmaReadBd(XAie_DevInst *DevInst, XAie_DmaDesc *DmaDesc,
		XAie_LocType Loc, u8 BdNum)
{
	AieRC RC;
	u64 Addr;
	u64 BdBaseAddr;
	u32 BdWord[XAIE_SHIMDMA_NUM_BD_WORDS];
	const XAie_DmaBdProp *BdProp;

	BdProp = DmaDesc->DmaMod->BdProp;
	BdBaseAddr = (u64)(DmaDesc->DmaMod->BaseAddr +
			BdNum * (u64)DmaDesc->DmaMod->IdxOffset);
	Addr = BdBaseAddr + XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	/* Setup DmaDesc with values read from bd registers */
	for(u8 i = 0; i < XAIE_SHIMDMA_NUM_BD_WORDS; i++) {
		RC = XAie_Read32(DevInst, Addr, &BdWord[i]);
		if (RC != XAIE_OK) {
			return RC;
		}
		Addr += 4U;
	}

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->Buffer->ShimDmaBuff.AddrLow.Lsb,
			BdProp->Buffer->ShimDmaBuff.AddrLow.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}


	DmaDesc->AddrDesc.Address |= (u64)XAie_GetField(BdWord[0U],
				BdProp->Buffer->ShimDmaBuff.AddrLow.Lsb,
				BdProp->Buffer->ShimDmaBuff.AddrLow.Mask);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->BufferLen.Lsb,
			BdProp->BufferLen.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}

	DmaDesc->AddrDesc.Length = XAie_GetField(BdWord[1U] ,
				BdProp->BufferLen.Lsb, BdProp->BufferLen.Mask);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->Buffer->ShimDmaBuff.AddrHigh.Lsb,
			BdProp->Buffer->ShimDmaBuff.AddrHigh.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}

	DmaDesc->AddrDesc.Address |= (u64)XAie_GetField(BdWord[2U],
				BdProp->Buffer->ShimDmaBuff.AddrHigh.Lsb,
				BdProp->Buffer->ShimDmaBuff.AddrHigh.Mask) << 32U;

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->BdEn->UseNxtBd.Lsb,
			BdProp->BdEn->UseNxtBd.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->BdEnDesc.UseNxtBd = (u8)(XAie_GetField(BdWord[2U],
				BdProp->BdEn->UseNxtBd.Lsb,
				BdProp->BdEn->UseNxtBd.Mask) & 0xFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->BdEn->NxtBd.Lsb,
			BdProp->BdEn->NxtBd.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->BdEnDesc.NxtBd = (u8)(XAie_GetField(BdWord[2U],
				BdProp->BdEn->NxtBd.Lsb,
				BdProp->BdEn->NxtBd.Mask) & 0xFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->Lock->AieDmaLock.LckId_A.Lsb,
			BdProp->Lock->AieDmaLock.LckId_A.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->LockDesc.LockAcqId = (u8)(XAie_GetField(BdWord[2U],
				BdProp->Lock->AieDmaLock.LckId_A.Lsb,
				BdProp->Lock->AieDmaLock.LckId_A.Mask) & 0xFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->Lock->AieDmaLock.LckRelEn_A.Lsb,
			BdProp->Lock->AieDmaLock.LckRelEn_A.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->LockDesc.LockRelEn = (u8)(XAie_GetField(BdWord[2U],
				BdProp->Lock->AieDmaLock.LckRelEn_A.Lsb,
				BdProp->Lock->AieDmaLock.LckRelEn_A.Mask) & 0xFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->Lock->AieDmaLock.LckRelVal_A.Lsb,
			BdProp->Lock->AieDmaLock.LckRelVal_A.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->LockDesc.LockRelVal = (s8)XAie_GetField(BdWord[2U],
				BdProp->Lock->AieDmaLock.LckRelVal_A.Lsb,
				BdProp->Lock->AieDmaLock.LckRelVal_A.Mask);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->Lock->AieDmaLock.LckRelUseVal_A.Lsb,
			BdProp->Lock->AieDmaLock.LckRelUseVal_A.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->LockDesc.LockRelValEn = (u8)(XAie_GetField(BdWord[2U],
				BdProp->Lock->AieDmaLock.LckRelUseVal_A.Lsb,
				BdProp->Lock->AieDmaLock.LckRelUseVal_A.Mask) & 0xFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->Lock->AieDmaLock.LckAcqEn_A.Lsb,
			BdProp->Lock->AieDmaLock.LckAcqEn_A.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->LockDesc.LockAcqEn = (u8)(XAie_GetField(BdWord[2U],
				BdProp->Lock->AieDmaLock.LckAcqEn_A.Lsb,
				BdProp->Lock->AieDmaLock.LckAcqEn_A.Mask) & 0xFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->Lock->AieDmaLock.LckAcqVal_A.Lsb,
			BdProp->Lock->AieDmaLock.LckAcqVal_A.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->LockDesc.LockAcqVal = (s8)XAie_GetField(BdWord[2U],
				BdProp->Lock->AieDmaLock.LckAcqVal_A.Lsb,
				BdProp->Lock->AieDmaLock.LckAcqVal_A.Mask);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->Lock->AieDmaLock.LckAcqUseVal_A.Lsb,
			BdProp->Lock->AieDmaLock.LckAcqUseVal_A.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->LockDesc.LockAcqValEn = (u8)(XAie_GetField(BdWord[2U],
				BdProp->Lock->AieDmaLock.LckAcqUseVal_A.Lsb,
				BdProp->Lock->AieDmaLock.LckAcqUseVal_A.Mask) & 0xFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->BdEn->ValidBd.Lsb,
			BdProp->BdEn->ValidBd.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->BdEnDesc.ValidBd = (u8)(XAie_GetField(BdWord[2U],
				BdProp->BdEn->ValidBd.Lsb,
				BdProp->BdEn->ValidBd.Mask) & 0xFFU);


	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->SysProp->SMID.Lsb,
			BdProp->SysProp->SMID.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->AxiDesc.SMID = (u8)(XAie_GetField(BdWord[3U],
				BdProp->SysProp->SMID.Lsb,
				BdProp->SysProp->SMID.Mask) & 0xFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->SysProp->BurstLen.Lsb,
			BdProp->SysProp->BurstLen.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->AxiDesc.BurstLen = (u8)(XAie_GetField(BdWord[3U],
				BdProp->SysProp->BurstLen.Lsb,
				BdProp->SysProp->BurstLen.Mask) & 0xFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->SysProp->AxQos.Lsb,
			BdProp->SysProp->AxQos.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->AxiDesc.AxQos = (u8)(XAie_GetField(BdWord[3U],
				BdProp->SysProp->AxQos.Lsb,
				BdProp->SysProp->AxQos.Mask) & 0xFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->SysProp->SecureAccess.Lsb,
			BdProp->SysProp->SecureAccess.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->AxiDesc.SecureAccess = (u8)(XAie_GetField(BdWord[3U],
				BdProp->SysProp->SecureAccess.Lsb,
				BdProp->SysProp->SecureAccess.Mask) & 0xFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->SysProp->AxCache.Lsb,
			BdProp->SysProp->AxCache.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->AxiDesc.AxCache = (u8)(XAie_GetField(BdWord[3U],
				BdProp->SysProp->AxCache.Lsb,
				BdProp->SysProp->AxCache.Mask) & 0xFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->Pkt->PktId.Lsb,
			BdProp->Pkt->PktId.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->PktDesc.PktId = (u8)(XAie_GetField(BdWord[4U],
				BdProp->Pkt->PktId.Lsb,
				BdProp->Pkt->PktId.Mask) & 0xFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->Pkt->PktType.Lsb,
			BdProp->Pkt->PktType.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->PktDesc.PktType = (u8)(XAie_GetField(BdWord[4U],
				BdProp->Pkt->PktType.Lsb,
				BdProp->Pkt->PktType.Mask) & 0xFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->Pkt->EnPkt.Lsb,
			BdProp->Pkt->EnPkt.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->PktDesc.PktEn = (u8)(XAie_GetField(BdWord[4U],
				BdProp->Pkt->EnPkt.Lsb,
				BdProp->Pkt->EnPkt.Mask) & 0xFFU);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API writes a Dma Descriptor which is initialized and setup by other APIs
* into the corresponding registers and register fields in the hardware. This API
* is specific to AIE Tiles only.
*
* @param	DevInst: Device Instance
* @param	DmaDesc: Initialized Dma Descriptor.
* @param	Loc: Location of AIE Tile
* @param	BdNum: Hardware BD number to be written to.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only. For AIE Tiles only.
*
******************************************************************************/
AieRC _XAie_TileDmaWriteBd(XAie_DevInst *DevInst , XAie_DmaDesc *DmaDesc,
		XAie_LocType Loc, u8 BdNum)
{
	u64 Addr;
	u64 BdBaseAddr;
	u32 BdWord[XAIE_TILEDMA_NUM_BD_WORDS];
	const XAie_DmaMod *DmaMod;
	const XAie_DmaBdProp *BdProp;

	DmaMod = DevInst->DevProp.DevMod[DmaDesc->TileType].DmaMod;
	BdProp = DmaMod->BdProp;

	BdBaseAddr = (u64)(DmaMod->BaseAddr + BdNum * (u64)DmaMod->IdxOffset);

	if ((_XAie_CheckPrecisionExceeds(BdProp->Lock->AieDmaLock.LckId_A.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->LockDesc.LockAcqId),MAX_VALID_AIE_REG_BIT_INDEX))||
		(_XAie_CheckPrecisionExceeds(BdProp->Lock->AieDmaLock.LckRelEn_A.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->LockDesc.LockRelEn),MAX_VALID_AIE_REG_BIT_INDEX))||
		(_XAie_CheckPrecisionExceeds(BdProp->Lock->AieDmaLock.LckRelUseVal_A.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->LockDesc.LockRelValEn),MAX_VALID_AIE_REG_BIT_INDEX))||
		(_XAie_CheckPrecisionExceeds(BdProp->Lock->AieDmaLock.LckAcqEn_A.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->LockDesc.LockAcqEn),MAX_VALID_AIE_REG_BIT_INDEX))||
		(_XAie_CheckPrecisionExceeds(BdProp->Lock->AieDmaLock.LckAcqUseVal_A.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->LockDesc.LockAcqValEn),MAX_VALID_AIE_REG_BIT_INDEX)) ||
		(_XAie_CheckPrecisionExceeds(BdProp->Buffer->TileDmaBuff.BaseAddr.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->AddrDesc.Address & 0xFFFFFFFFU),MAX_VALID_AIE_REG_BIT_INDEX))) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}

	/* AcqLockId and RelLockId are the same in AIE */
	BdWord[0U] = XAie_SetField(DmaDesc->LockDesc.LockAcqId,
			BdProp->Lock->AieDmaLock.LckId_A.Lsb,
			BdProp->Lock->AieDmaLock.LckId_A.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockRelEn,
				BdProp->Lock->AieDmaLock.LckRelEn_A.Lsb,
				BdProp->Lock->AieDmaLock.LckRelEn_A.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockRelVal,
				BdProp->Lock->AieDmaLock.LckRelVal_A.Lsb,
				BdProp->Lock->AieDmaLock.LckRelVal_A.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockRelValEn,
				BdProp->Lock->AieDmaLock.LckRelUseVal_A.Lsb,
				BdProp->Lock->AieDmaLock.LckRelUseVal_A.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockAcqEn,
				BdProp->Lock->AieDmaLock.LckAcqEn_A.Lsb,
				BdProp->Lock->AieDmaLock.LckAcqEn_A.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockAcqVal,
				BdProp->Lock->AieDmaLock.LckAcqVal_A.Lsb,
				BdProp->Lock->AieDmaLock.LckAcqVal_A.Mask) |
		XAie_SetField(DmaDesc->LockDesc.LockAcqValEn,
				BdProp->Lock->AieDmaLock.LckAcqUseVal_A.Lsb,
				BdProp->Lock->AieDmaLock.LckAcqUseVal_A.Mask) |
		XAie_SetField(DmaDesc->AddrDesc.Address & 0xFFFFFFFFU,
				BdProp->Buffer->TileDmaBuff.BaseAddr.Lsb,
				BdProp->Buffer->TileDmaBuff.BaseAddr.Mask);

	if ((_XAie_CheckPrecisionExceeds(BdProp->Lock->AieDmaLock.LckId_B.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->LockDesc_2.LockAcqId),MAX_VALID_AIE_REG_BIT_INDEX))||
		(_XAie_CheckPrecisionExceeds(BdProp->Lock->AieDmaLock.LckRelEn_B.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->LockDesc_2.LockRelEn),MAX_VALID_AIE_REG_BIT_INDEX))||
		(_XAie_CheckPrecisionExceeds(BdProp->Lock->AieDmaLock.LckRelUseVal_B.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->LockDesc_2.LockRelValEn),MAX_VALID_AIE_REG_BIT_INDEX))||
		(_XAie_CheckPrecisionExceeds(BdProp->Lock->AieDmaLock.LckAcqEn_B.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->LockDesc_2.LockAcqEn),MAX_VALID_AIE_REG_BIT_INDEX))||
		(_XAie_CheckPrecisionExceeds(BdProp->DoubleBuffer->BaseAddr_B.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->AddrDesc_2.Address & 0xFFFFFFFFU),MAX_VALID_AIE_REG_BIT_INDEX))||
		(_XAie_CheckPrecisionExceeds(BdProp->Lock->AieDmaLock.LckAcqUseVal_B.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->LockDesc_2.LockAcqValEn),MAX_VALID_AIE_REG_BIT_INDEX))) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}

	BdWord[1U] = XAie_SetField(DmaDesc->LockDesc_2.LockAcqId,
			BdProp->Lock->AieDmaLock.LckId_B.Lsb,
			BdProp->Lock->AieDmaLock.LckId_B.Mask) |
		XAie_SetField(DmaDesc->LockDesc_2.LockRelEn,
				BdProp->Lock->AieDmaLock.LckRelEn_B.Lsb,
				BdProp->Lock->AieDmaLock.LckRelEn_B.Mask) |
		XAie_SetField(DmaDesc->LockDesc_2.LockRelVal,
				BdProp->Lock->AieDmaLock.LckRelVal_B.Lsb,
				BdProp->Lock->AieDmaLock.LckRelVal_B.Mask) |
		XAie_SetField(DmaDesc->LockDesc_2.LockRelValEn,
				BdProp->Lock->AieDmaLock.LckRelUseVal_B.Lsb,
				BdProp->Lock->AieDmaLock.LckRelUseVal_B.Mask) |
		XAie_SetField(DmaDesc->LockDesc_2.LockAcqEn,
				BdProp->Lock->AieDmaLock.LckAcqEn_B.Lsb,
				BdProp->Lock->AieDmaLock.LckAcqEn_B.Mask) |
		XAie_SetField(DmaDesc->LockDesc_2.LockAcqVal,
				BdProp->Lock->AieDmaLock.LckAcqVal_B.Lsb,
				BdProp->Lock->AieDmaLock.LckAcqVal_B.Mask) |
		XAie_SetField(DmaDesc->LockDesc_2.LockAcqValEn,
				BdProp->Lock->AieDmaLock.LckAcqUseVal_B.Lsb,
				BdProp->Lock->AieDmaLock.LckAcqUseVal_B.Mask) |
		XAie_SetField(DmaDesc->AddrDesc_2.Address,
				BdProp->DoubleBuffer->BaseAddr_B.Lsb,
				BdProp->DoubleBuffer->BaseAddr_B.Mask);

	if ((_XAie_CheckPrecisionExceeds(BdProp->AddrMode->AieMultiDimAddr.X_Incr.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->MultiDimDesc.AieMultiDimDesc.X_Incr),MAX_VALID_AIE_REG_BIT_INDEX))||
		(_XAie_CheckPrecisionExceeds(BdProp->AddrMode->AieMultiDimAddr.X_Wrap.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->MultiDimDesc.AieMultiDimDesc.X_Wrap),MAX_VALID_AIE_REG_BIT_INDEX))||
		(_XAie_CheckPrecisionExceeds(BdProp->AddrMode->AieMultiDimAddr.X_Offset.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->MultiDimDesc.AieMultiDimDesc.X_Offset),MAX_VALID_AIE_REG_BIT_INDEX))) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}


	BdWord[2U] = XAie_SetField(
			DmaDesc->MultiDimDesc.AieMultiDimDesc.X_Incr,
			BdProp->AddrMode->AieMultiDimAddr.X_Incr.Lsb,
			BdProp->AddrMode->AieMultiDimAddr.X_Incr.Mask) |
		XAie_SetField(DmaDesc->MultiDimDesc.AieMultiDimDesc.X_Wrap,
				BdProp->AddrMode->AieMultiDimAddr.X_Wrap.Lsb,
				BdProp->AddrMode->AieMultiDimAddr.X_Wrap.Mask) |
		XAie_SetField(DmaDesc->MultiDimDesc.AieMultiDimDesc.X_Offset,
				BdProp->AddrMode->AieMultiDimAddr.X_Offset.Lsb,
				BdProp->AddrMode->AieMultiDimAddr.X_Offset.Mask);

	if ((_XAie_CheckPrecisionExceeds(BdProp->AddrMode->AieMultiDimAddr.Y_Incr.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->MultiDimDesc.AieMultiDimDesc.Y_Incr),MAX_VALID_AIE_REG_BIT_INDEX))||
		(_XAie_CheckPrecisionExceeds(BdProp->AddrMode->AieMultiDimAddr.Y_Wrap.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->MultiDimDesc.AieMultiDimDesc.Y_Wrap),MAX_VALID_AIE_REG_BIT_INDEX))||
		(_XAie_CheckPrecisionExceeds(BdProp->AddrMode->AieMultiDimAddr.Y_Offset.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->MultiDimDesc.AieMultiDimDesc.Y_Offset),MAX_VALID_AIE_REG_BIT_INDEX))) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}

	BdWord[3U] = XAie_SetField(
			DmaDesc->MultiDimDesc.AieMultiDimDesc.Y_Incr,
			BdProp->AddrMode->AieMultiDimAddr.Y_Incr.Lsb,
			BdProp->AddrMode->AieMultiDimAddr.Y_Incr.Mask) |
		XAie_SetField(DmaDesc->MultiDimDesc.AieMultiDimDesc.X_Wrap,
				BdProp->AddrMode->AieMultiDimAddr.Y_Wrap.Lsb,
				BdProp->AddrMode->AieMultiDimAddr.Y_Wrap.Mask) |
		XAie_SetField(DmaDesc->MultiDimDesc.AieMultiDimDesc.Y_Offset,
				BdProp->AddrMode->AieMultiDimAddr.Y_Offset.Lsb,
				BdProp->AddrMode->AieMultiDimAddr.Y_Offset.Mask);

	if ((_XAie_CheckPrecisionExceeds(BdProp->Pkt->PktId.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->PktDesc.PktId),MAX_VALID_AIE_REG_BIT_INDEX))||
		(_XAie_CheckPrecisionExceeds(BdProp->Pkt->PktType.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->PktDesc.PktType),MAX_VALID_AIE_REG_BIT_INDEX))) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}

	BdWord[4U] = XAie_SetField(DmaDesc->PktDesc.PktId,
			BdProp->Pkt->PktId.Lsb, BdProp->Pkt->PktId.Mask) |
		XAie_SetField(DmaDesc->PktDesc.PktType,
				BdProp->Pkt->PktType.Lsb,
				BdProp->Pkt->PktType.Mask);

	if ((_XAie_CheckPrecisionExceeds(BdProp->DoubleBuffer->BuffSelect.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->MultiDimDesc.AieMultiDimDesc.IntrleaveBufSelect),MAX_VALID_AIE_REG_BIT_INDEX))||
		(_XAie_CheckPrecisionExceeds(BdProp->AddrMode->AieMultiDimAddr.CurrPtr.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->MultiDimDesc.AieMultiDimDesc.CurrPtr),MAX_VALID_AIE_REG_BIT_INDEX))) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}

	BdWord[5U] = XAie_SetField(
			DmaDesc->MultiDimDesc.AieMultiDimDesc.IntrleaveBufSelect,
			BdProp->DoubleBuffer->BuffSelect.Lsb,
			BdProp->DoubleBuffer->BuffSelect.Mask) |
		XAie_SetField(DmaDesc->MultiDimDesc.AieMultiDimDesc.CurrPtr,
				BdProp->AddrMode->AieMultiDimAddr.CurrPtr.Lsb,
				BdProp->AddrMode->AieMultiDimAddr.CurrPtr.Mask);

	if ((_XAie_CheckPrecisionExceeds(BdProp->BdEn->ValidBd.Lsb,
				_XAie_MaxBitsNeeded(DmaDesc->BdEnDesc.ValidBd),MAX_VALID_AIE_REG_BIT_INDEX)) ||
		(_XAie_CheckPrecisionExceeds(BdProp->BdEn->UseNxtBd.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->BdEnDesc.UseNxtBd),MAX_VALID_AIE_REG_BIT_INDEX))||
		(_XAie_CheckPrecisionExceeds(BdProp->DoubleBuffer->EnDoubleBuff.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->EnDoubleBuff),MAX_VALID_AIE_REG_BIT_INDEX))||
		(_XAie_CheckPrecisionExceeds(BdProp->DoubleBuffer->FifoMode.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->FifoMode),MAX_VALID_AIE_REG_BIT_INDEX))||
		(_XAie_CheckPrecisionExceeds(BdProp->Pkt->EnPkt.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->PktDesc.PktEn),MAX_VALID_AIE_REG_BIT_INDEX))||
		(_XAie_CheckPrecisionExceeds(BdProp->DoubleBuffer->EnIntrleaved.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->MultiDimDesc.AieMultiDimDesc.EnInterleaved),MAX_VALID_AIE_REG_BIT_INDEX))||
		(_XAie_CheckPrecisionExceeds(BdProp->DoubleBuffer->IntrleaveCnt.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->MultiDimDesc.AieMultiDimDesc.IntrleaveCount - 1U),MAX_VALID_AIE_REG_BIT_INDEX))||
		(_XAie_CheckPrecisionExceeds(BdProp->BdEn->NxtBd.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->BdEnDesc.NxtBd),MAX_VALID_AIE_REG_BIT_INDEX))||
		(_XAie_CheckPrecisionExceeds(BdProp->BufferLen.Lsb,
			_XAie_MaxBitsNeeded(DmaDesc->AddrDesc.Length),MAX_VALID_AIE_REG_BIT_INDEX))) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}

	BdWord[6U] = XAie_SetField(DmaDesc->BdEnDesc.ValidBd,
			BdProp->BdEn->ValidBd.Lsb, BdProp->BdEn->ValidBd.Mask) |
		XAie_SetField(DmaDesc->BdEnDesc.UseNxtBd,
				BdProp->BdEn->UseNxtBd.Lsb,
				BdProp->BdEn->UseNxtBd.Mask) |
		XAie_SetField(DmaDesc->EnDoubleBuff,
				BdProp->DoubleBuffer->EnDoubleBuff.Lsb,
				BdProp->DoubleBuffer->EnDoubleBuff.Mask) |
		XAie_SetField(DmaDesc->FifoMode,
				BdProp->DoubleBuffer->FifoMode.Lsb,
				BdProp->DoubleBuffer->FifoMode.Mask) |
		XAie_SetField(DmaDesc->PktDesc.PktEn, BdProp->Pkt->EnPkt.Lsb,
				BdProp->Pkt->EnPkt.Mask) |
		XAie_SetField(DmaDesc->MultiDimDesc.AieMultiDimDesc.EnInterleaved,
				BdProp->DoubleBuffer->EnIntrleaved.Lsb,
				BdProp->DoubleBuffer->EnIntrleaved.Mask) |
		XAie_SetField((DmaDesc->MultiDimDesc.AieMultiDimDesc.IntrleaveCount - 1U),
				BdProp->DoubleBuffer->IntrleaveCnt.Lsb,
				BdProp->DoubleBuffer->IntrleaveCnt.Mask) |
		XAie_SetField(DmaDesc->BdEnDesc.NxtBd,
				BdProp->BdEn->NxtBd.Lsb,
				BdProp->BdEn->NxtBd.Mask) |
		XAie_SetField(DmaDesc->AddrDesc.Length,
				BdProp->BufferLen.Lsb,
				BdProp->BufferLen.Mask);

	Addr = BdBaseAddr + XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	return XAie_BlockWrite32(DevInst, Addr, BdWord, XAIE_TILEDMA_NUM_BD_WORDS);
}

/*****************************************************************************/
/**
*
* This API reads a the data from the buffer descriptor registers to fill the
* DmaDesc structure. This API is meant for AIE Tiles only.
*
* @param	DevInst: Device Instance
* @param	DmaDesc: Dma Descriptor to be filled.
* @param	Loc: Location of AIE Tile
* @param	BdNum: Hardware BD number to be read from.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only. For AIE Tiles only.
*
******************************************************************************/
AieRC _XAie_TileDmaReadBd(XAie_DevInst *DevInst , XAie_DmaDesc *DmaDesc,
		XAie_LocType Loc, u8 BdNum)
{
	AieRC RC;
	u64 Addr;
	u64 BdBaseAddr;
	u32 BdWord[XAIE_TILEDMA_NUM_BD_WORDS];
	const XAie_DmaBdProp *BdProp;

	BdProp = DmaDesc->DmaMod->BdProp;
	BdBaseAddr = (u64)(DmaDesc->DmaMod->BaseAddr +
			BdNum * (u64)DmaDesc->DmaMod->IdxOffset);
	Addr = BdBaseAddr + XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	/* Setup DmaDesc with values read from bd registers */
	for(u8 i = 0; i < XAIE_TILEDMA_NUM_BD_WORDS; i++) {
		RC = XAie_Read32(DevInst, Addr, &BdWord[i]);
		if (RC != XAIE_OK) {
			return RC;
		}
		Addr += 4U;
	}

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->Lock->AieDmaLock.LckId_A.Lsb,
			BdProp->Lock->AieDmaLock.LckId_A.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->LockDesc.LockAcqId = (u8)(XAie_GetField(BdWord[0U],
				BdProp->Lock->AieDmaLock.LckId_A.Lsb,
				BdProp->Lock->AieDmaLock.LckId_A.Mask) & 0xFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->Lock->AieDmaLock.LckRelEn_A.Lsb,
			BdProp->Lock->AieDmaLock.LckRelEn_A.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->LockDesc.LockRelEn = (u8)(XAie_GetField(BdWord[0U],
				BdProp->Lock->AieDmaLock.LckRelEn_A.Lsb,
				BdProp->Lock->AieDmaLock.LckRelEn_A.Mask) & 0xFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->Lock->AieDmaLock.LckRelVal_A.Lsb,
			BdProp->Lock->AieDmaLock.LckRelVal_A.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->LockDesc.LockRelVal = (s8)XAie_GetField(BdWord[0U],
				BdProp->Lock->AieDmaLock.LckRelVal_A.Lsb,
				BdProp->Lock->AieDmaLock.LckRelVal_A.Mask);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->Lock->AieDmaLock.LckRelUseVal_A.Lsb,
			BdProp->Lock->AieDmaLock.LckRelUseVal_A.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->LockDesc.LockRelValEn = (u8)(XAie_GetField(BdWord[0U],
				BdProp->Lock->AieDmaLock.LckRelUseVal_A.Lsb,
				BdProp->Lock->AieDmaLock.LckRelUseVal_A.Mask) & 0xFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->Lock->AieDmaLock.LckAcqEn_A.Lsb,
			BdProp->Lock->AieDmaLock.LckAcqEn_A.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->LockDesc.LockAcqEn = (u8)(XAie_GetField(BdWord[0U],
				BdProp->Lock->AieDmaLock.LckAcqEn_A.Lsb,
				BdProp->Lock->AieDmaLock.LckAcqEn_A.Mask) & 0xFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->Lock->AieDmaLock.LckAcqVal_A.Lsb,
			BdProp->Lock->AieDmaLock.LckAcqVal_A.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->LockDesc.LockAcqVal = (s8)XAie_GetField(BdWord[0U],
				BdProp->Lock->AieDmaLock.LckAcqVal_A.Lsb,
				BdProp->Lock->AieDmaLock.LckAcqVal_A.Mask);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->Lock->AieDmaLock.LckAcqUseVal_A.Lsb,
			BdProp->Lock->AieDmaLock.LckAcqUseVal_A.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->LockDesc.LockAcqValEn = (u8)(XAie_GetField(BdWord[0U],
				BdProp->Lock->AieDmaLock.LckAcqUseVal_A.Lsb,
				BdProp->Lock->AieDmaLock.LckAcqUseVal_A.Mask) & 0xFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->Buffer->TileDmaBuff.BaseAddr.Lsb,
			BdProp->Buffer->TileDmaBuff.BaseAddr.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->AddrDesc.Address = (u64)XAie_GetField(BdWord[0U],
				BdProp->Buffer->TileDmaBuff.BaseAddr.Lsb,
				BdProp->Buffer->TileDmaBuff.BaseAddr.Mask);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->Lock->AieDmaLock.LckId_B.Lsb,
			BdProp->Lock->AieDmaLock.LckId_B.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->LockDesc_2.LockAcqId = (u8)(XAie_GetField(BdWord[1U],
				BdProp->Lock->AieDmaLock.LckId_B.Lsb,
				BdProp->Lock->AieDmaLock.LckId_B.Mask) & 0xFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->Lock->AieDmaLock.LckRelEn_B.Lsb,
			BdProp->Lock->AieDmaLock.LckRelEn_B.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->LockDesc_2.LockRelEn = (u8)(XAie_GetField(BdWord[1U],
				BdProp->Lock->AieDmaLock.LckRelEn_B.Lsb,
				BdProp->Lock->AieDmaLock.LckRelEn_B.Mask) & 0xFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->Lock->AieDmaLock.LckRelVal_B.Lsb,
			BdProp->Lock->AieDmaLock.LckRelVal_B.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->LockDesc_2.LockRelVal = (s8)XAie_GetField(BdWord[1U],
				BdProp->Lock->AieDmaLock.LckRelVal_B.Lsb,
				BdProp->Lock->AieDmaLock.LckRelVal_B.Mask);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->Lock->AieDmaLock.LckRelUseVal_B.Lsb,
			BdProp->Lock->AieDmaLock.LckRelUseVal_B.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->LockDesc_2.LockRelValEn = (u8)(XAie_GetField(BdWord[1U],
				BdProp->Lock->AieDmaLock.LckRelUseVal_B.Lsb,
				BdProp->Lock->AieDmaLock.LckRelUseVal_B.Mask) & 0xFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->Lock->AieDmaLock.LckAcqEn_B.Lsb,
			BdProp->Lock->AieDmaLock.LckAcqEn_B.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->LockDesc_2.LockAcqEn = (u8)(XAie_GetField(BdWord[1U],
				BdProp->Lock->AieDmaLock.LckAcqEn_B.Lsb,
				BdProp->Lock->AieDmaLock.LckAcqEn_B.Mask) & 0xFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->Lock->AieDmaLock.LckAcqVal_B.Lsb,
			BdProp->Lock->AieDmaLock.LckAcqVal_B.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->LockDesc_2.LockAcqVal = (s8)XAie_GetField(BdWord[1U],
				BdProp->Lock->AieDmaLock.LckAcqVal_B.Lsb,
				BdProp->Lock->AieDmaLock.LckAcqVal_B.Mask);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->Lock->AieDmaLock.LckAcqUseVal_B.Lsb,
			BdProp->Lock->AieDmaLock.LckAcqUseVal_B.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->LockDesc_2.LockAcqValEn = (u8)(XAie_GetField(BdWord[1U],
				BdProp->Lock->AieDmaLock.LckAcqUseVal_B.Lsb,
				BdProp->Lock->AieDmaLock.LckAcqUseVal_B.Mask) & 0xFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->DoubleBuffer->BaseAddr_B.Lsb,
			BdProp->DoubleBuffer->BaseAddr_B.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->AddrDesc_2.Address = (u64)XAie_GetField(BdWord[1U],
				BdProp->DoubleBuffer->BaseAddr_B.Lsb,
				BdProp->DoubleBuffer->BaseAddr_B.Mask);


	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->AddrMode->AieMultiDimAddr.X_Incr.Lsb,
			BdProp->AddrMode->AieMultiDimAddr.X_Incr.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->MultiDimDesc.AieMultiDimDesc.X_Incr =
			(u8)(XAie_GetField(BdWord[2U],
				BdProp->AddrMode->AieMultiDimAddr.X_Incr.Lsb,
				BdProp->AddrMode->AieMultiDimAddr.X_Incr.Mask) & 0xFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->AddrMode->AieMultiDimAddr.X_Wrap.Lsb,
			BdProp->AddrMode->AieMultiDimAddr.X_Wrap.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->MultiDimDesc.AieMultiDimDesc.X_Wrap =
			(u8)(XAie_GetField(BdWord[2U],
				BdProp->AddrMode->AieMultiDimAddr.X_Wrap.Lsb,
				BdProp->AddrMode->AieMultiDimAddr.X_Wrap.Mask) & 0xFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->AddrMode->AieMultiDimAddr.X_Offset.Lsb,
			BdProp->AddrMode->AieMultiDimAddr.X_Offset.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->MultiDimDesc.AieMultiDimDesc.X_Offset =
			(u16)(XAie_GetField(BdWord[2U],
				BdProp->AddrMode->AieMultiDimAddr.X_Offset.Lsb,
				BdProp->AddrMode->AieMultiDimAddr.X_Offset.Mask) & 0xFFFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->AddrMode->AieMultiDimAddr.Y_Incr.Lsb,
			BdProp->AddrMode->AieMultiDimAddr.Y_Incr.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->MultiDimDesc.AieMultiDimDesc.Y_Incr =
			(u8)(XAie_GetField(BdWord[3U],
				BdProp->AddrMode->AieMultiDimAddr.Y_Incr.Lsb,
				BdProp->AddrMode->AieMultiDimAddr.Y_Incr.Mask) & 0xFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->AddrMode->AieMultiDimAddr.Y_Wrap.Lsb,
			BdProp->AddrMode->AieMultiDimAddr.Y_Wrap.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->MultiDimDesc.AieMultiDimDesc.X_Wrap =
			(u8)(XAie_GetField(BdWord[3U],
				BdProp->AddrMode->AieMultiDimAddr.Y_Wrap.Lsb,
				BdProp->AddrMode->AieMultiDimAddr.Y_Wrap.Mask) & 0xFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->AddrMode->AieMultiDimAddr.Y_Offset.Lsb,
			BdProp->AddrMode->AieMultiDimAddr.Y_Offset.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->MultiDimDesc.AieMultiDimDesc.Y_Offset =
			(u16)(XAie_GetField(BdWord[3U],
				BdProp->AddrMode->AieMultiDimAddr.Y_Offset.Lsb,
				BdProp->AddrMode->AieMultiDimAddr.Y_Offset.Mask) & 0xFFFFU);


	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->Pkt->PktId.Lsb,
			BdProp->Pkt->PktId.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->PktDesc.PktId = (u8)(XAie_GetField(BdWord[4U],
				BdProp->Pkt->PktId.Lsb,
				BdProp->Pkt->PktId.Mask) & 0xFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->Pkt->PktType.Lsb,
			BdProp->Pkt->PktType.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->PktDesc.PktType = (u8)(XAie_GetField(BdWord[4U],
				BdProp->Pkt->PktType.Lsb,
				BdProp->Pkt->PktType.Mask) & 0xFFU);


	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->DoubleBuffer->BuffSelect.Lsb,
			BdProp->DoubleBuffer->BuffSelect.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->MultiDimDesc.AieMultiDimDesc.IntrleaveBufSelect =
			(u8)(XAie_GetField(BdWord[5U],
				BdProp->DoubleBuffer->BuffSelect.Lsb,
				BdProp->DoubleBuffer->BuffSelect.Mask) & 0xFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->AddrMode->AieMultiDimAddr.CurrPtr.Lsb,
			BdProp->AddrMode->AieMultiDimAddr.CurrPtr.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->MultiDimDesc.AieMultiDimDesc.CurrPtr =
			(u16)(XAie_GetField(BdWord[5U],
				BdProp->AddrMode->AieMultiDimAddr.CurrPtr.Lsb,
				BdProp->AddrMode->AieMultiDimAddr.CurrPtr.Mask) & 0xFFFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->BdEn->ValidBd.Lsb,
			BdProp->BdEn->ValidBd.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->BdEnDesc.ValidBd = (u8)(XAie_GetField(BdWord[6U],
				BdProp->BdEn->ValidBd.Lsb,
				BdProp->BdEn->ValidBd.Mask) & 0xFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->BdEn->UseNxtBd.Lsb,
			BdProp->BdEn->UseNxtBd.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->BdEnDesc.UseNxtBd = (u8)(XAie_GetField(BdWord[6U],
				BdProp->BdEn->UseNxtBd.Lsb,
				BdProp->BdEn->UseNxtBd.Mask) & 0xFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->DoubleBuffer->EnDoubleBuff.Lsb,
			BdProp->DoubleBuffer->EnDoubleBuff.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->EnDoubleBuff = (u8)(XAie_GetField(BdWord[6U],
				BdProp->DoubleBuffer->EnDoubleBuff.Lsb,
				BdProp->DoubleBuffer->EnDoubleBuff.Mask) & 0xFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->DoubleBuffer->FifoMode.Lsb,
			BdProp->DoubleBuffer->FifoMode.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->FifoMode = (u8)(XAie_GetField(BdWord[6U],
				BdProp->DoubleBuffer->FifoMode.Lsb,
				BdProp->DoubleBuffer->FifoMode.Mask) & 0xFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->Pkt->EnPkt.Lsb,
			BdProp->Pkt->EnPkt.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->PktDesc.PktEn = (u8)(XAie_GetField(BdWord[6U],
				BdProp->Pkt->EnPkt.Lsb,
				BdProp->Pkt->EnPkt.Mask) & 0xFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->DoubleBuffer->EnIntrleaved.Lsb,
			BdProp->DoubleBuffer->EnIntrleaved.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->MultiDimDesc.AieMultiDimDesc.EnInterleaved =
			(u8)(XAie_GetField(BdWord[6U],
				BdProp->DoubleBuffer->EnIntrleaved.Lsb,
				BdProp->DoubleBuffer->EnIntrleaved.Mask) & 0xFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->DoubleBuffer->IntrleaveCnt.Lsb,
			BdProp->DoubleBuffer->IntrleaveCnt.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->MultiDimDesc.AieMultiDimDesc.IntrleaveCount = (u8)((1U +
			XAie_GetField(BdWord[6U],
				BdProp->DoubleBuffer->IntrleaveCnt.Lsb,
				BdProp->DoubleBuffer->IntrleaveCnt.Mask)) & 0xFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->BdEn->NxtBd.Lsb,
			BdProp->BdEn->NxtBd.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->BdEnDesc.NxtBd = (u8)(XAie_GetField(BdWord[6U],
				BdProp->BdEn->NxtBd.Lsb,
				BdProp->BdEn->NxtBd.Mask) & 0xFFU);

	if (_XAie_CheckPrecisionExceedsForRightShift(BdProp->BufferLen.Lsb,
			BdProp->BufferLen.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	DmaDesc->AddrDesc.Length = XAie_GetField(BdWord[6U],
				BdProp->BufferLen.Lsb,
				BdProp->BufferLen.Mask);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API is used to get the count of scheduled BDs in pending.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	DmaMod: Dma module pointer
* @param	ChNum: Channel number of the DMA.
* @param	Dir: Direction of the DMA Channel. (MM2S or S2MM)
* @param	PendingBd: Pointer to store the number of pending BDs.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only. For AIE Tiles only.
*
******************************************************************************/
AieRC _XAie_DmaGetPendingBdCount(XAie_DevInst *DevInst, XAie_LocType Loc,
		const XAie_DmaMod *DmaMod, u8 ChNum, XAie_DmaDirection Dir,
		u8 *PendingBd)
{
	AieRC RC;
	u64 Addr;
	u32 StatusReg, StartQSize, Stalled, Status;

	Addr = XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
		DmaMod->ChStatusBase +(u32)Dir * DmaMod->ChStatusOffset;

	RC = XAie_Read32(DevInst, Addr, &StatusReg);
	if(RC != XAIE_OK) {
		return RC;
	}

	if (_XAie_CheckPrecisionExceedsForRightShift(DmaMod->ChProp->DmaChStatus[ChNum].AieDmaChStatus.StartQSize.Lsb,
			DmaMod->ChProp->DmaChStatus[ChNum].AieDmaChStatus.StartQSize.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	StartQSize = XAie_GetField(StatusReg,
			DmaMod->ChProp->DmaChStatus[ChNum].AieDmaChStatus.StartQSize.Lsb,
			DmaMod->ChProp->DmaChStatus[ChNum].AieDmaChStatus.StartQSize.Mask);

	if(StartQSize > DmaMod->ChProp->StartQSizeMax) {
		XAIE_ERROR("Invalid start queue size from register\n");
		return XAIE_ERR;
	}

	if (_XAie_CheckPrecisionExceedsForRightShift(DmaMod->ChProp->DmaChStatus[ChNum].AieDmaChStatus.Status.Lsb,
			DmaMod->ChProp->DmaChStatus[ChNum].AieDmaChStatus.Status.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	Status = XAie_GetField(StatusReg,
			DmaMod->ChProp->DmaChStatus[ChNum].AieDmaChStatus.Status.Lsb,
			DmaMod->ChProp->DmaChStatus[ChNum].AieDmaChStatus.Status.Mask);

	if (_XAie_CheckPrecisionExceedsForRightShift(DmaMod->ChProp->DmaChStatus[ChNum].AieDmaChStatus.Stalled.Lsb,
			DmaMod->ChProp->DmaChStatus[ChNum].AieDmaChStatus.Stalled.Mask)) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	Stalled = XAie_GetField(StatusReg,
			DmaMod->ChProp->DmaChStatus[ChNum].AieDmaChStatus.Stalled.Lsb,
			DmaMod->ChProp->DmaChStatus[ChNum].AieDmaChStatus.Stalled.Mask);

	/* Check if BD is being used by a channel */
	if((Status != XAIE_DMA_STATUS_IDLE) || (Stalled) != 0U) {
		StartQSize++;
	}

	*PendingBd = (u8)StartQSize;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API is used to wait on Shim DMA channel to be completed.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	DmaMod: Dma module pointer
* @param	ChNum: Channel number of the DMA.
* @param	Dir: Direction of the DMA Channel. (MM2S or S2MM)
* @param        TimeOutUs - Minimum timeout value in micro seconds.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only. For AIE Tiles only.
*
******************************************************************************/
AieRC _XAie_DmaWaitForDone(XAie_DevInst *DevInst, XAie_LocType Loc,
		const XAie_DmaMod *DmaMod, u8 ChNum, XAie_DmaDirection Dir,
		u32 TimeOutUs, u8 BusyPoll)
{
	u64 Addr;
	u32 Mask, Value;
	AieRC Status = XAIE_OK;

	Addr = XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
		DmaMod->ChStatusBase + (u32)Dir * DmaMod->ChStatusOffset;
	Mask = DmaMod->ChProp->DmaChStatus[ChNum].AieDmaChStatus.Status.Mask |
		DmaMod->ChProp->DmaChStatus[ChNum].AieDmaChStatus.StartQSize.Mask |
		DmaMod->ChProp->DmaChStatus[ChNum].AieDmaChStatus.Stalled.Mask;

	if ((_XAie_CheckPrecisionExceeds(DmaMod->ChProp->DmaChStatus[ChNum].AieDmaChStatus.Status.Lsb,
			_XAie_MaxBitsNeeded((u32)XAIE_DMA_STATUS_IDLE),MAX_VALID_AIE_REG_BIT_INDEX))) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}

	/* This will check the stalled and start queue size bits to be zero */
	Value = (u32)(XAIE_DMA_STATUS_IDLE <<
		DmaMod->ChProp->DmaChStatus[ChNum].AieDmaChStatus.Status.Lsb);

	if (BusyPoll != XAIE_ENABLE){
		Status = XAie_MaskPoll(DevInst, Addr, Mask, Value, TimeOutUs);
	} else {
		Status = XAie_MaskPollBusy(DevInst, Addr, Mask, Value, TimeOutUs);
	}

	if (Status != XAIE_OK) {
		XAIE_DBG("Dma Wait Done Status poll time out\n");
		return XAIE_ERR;
	}

	return Status;
}

/*****************************************************************************/
/**
 *
 * This API is used to get Channel Status register value
 *
 * @param	DevInst: Device Instance
 * @param	Loc: Location of AIE Tile
 * @param	DmaMod: Dma module pointer
 * @param	ChNum: Channel number of the DMA.
 * @param	Dir: Direction of the DMA Channel. (MM2S or S2MM)
 * @param	Status - Channel Status Register value
 *
 * @return	XAIE_OK on success, Error code on failure.
 *
 * @note	Internal only. For AIE Tiles only.
 *
 ******************************************************************************/
AieRC _XAie_DmaGetChannelStatus(XAie_DevInst *DevInst, XAie_LocType Loc,
		const XAie_DmaMod *DmaMod, u8 ChNum, XAie_DmaDirection Dir,
		u32 *Status)
{
	u64 Addr;
	u32 Mask;
	AieRC RC;

	Addr = XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
		DmaMod->ChStatusBase + (u32)Dir * DmaMod->ChStatusOffset;

	Mask = DmaMod->ChProp->DmaChStatus[ChNum].AieDmaChStatus.Status.Mask |
		DmaMod->ChProp->DmaChStatus[ChNum].AieDmaChStatus.StartQSize.Mask |
		DmaMod->ChProp->DmaChStatus[ChNum].AieDmaChStatus.Stalled.Mask;

	RC = XAie_Read32(DevInst, Addr, Status);
	if (RC != XAIE_OK) {
		return RC;
	}

	*Status = (*Status & Mask);
	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API is used to check the validity of Bd number and Channel number
* combination.
*
* @param	BdNum: Buffer descriptor number.
* @param	ChNum: Channel number of the DMA.
* @param        TimeOutUs - Minimum timeout value in micro seconds.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only. For AIE Tiles only. This is a dummy function as
*		there is no restriction in AIE.
*
******************************************************************************/
AieRC _XAie_DmaCheckBdChValidity(u8 BdNum, u8 ChNum)
{
	(void)BdNum;
	(void)ChNum;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API updates the length of the buffer descriptor in the tile dma module.
*
* @param	DevInst: Device Instance.
* @param	DmaMod: Dma module pointer
* @param	Loc: Location of AIE Tile
* @param	Len: Length of BD in bytes.
* @param	BdNum: Hardware BD number to be written to.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only. This API accesses the hardware directly and does
*		not operate on software descriptor.
******************************************************************************/
AieRC _XAie_DmaUpdateBdLen(XAie_DevInst *DevInst, const XAie_DmaMod *DmaMod,
		XAie_LocType Loc, u32 Len, u16 BdNum)
{
	u64 RegAddr;
	u32 RegVal, Mask;

	RegAddr = (u64)(DmaMod->BaseAddr + BdNum * (u64)DmaMod->IdxOffset) +
		XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
	DmaMod->BdProp->BufferLen.Idx * 4U;

	Mask = DmaMod->BdProp->BufferLen.Mask;

	if ((_XAie_CheckPrecisionExceeds(DmaMod->BdProp->BufferLen.Lsb,
			_XAie_MaxBitsNeeded(Len),MAX_VALID_AIE_REG_BIT_INDEX))) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	RegVal = XAie_SetField(Len,
			DmaMod->BdProp->BufferLen.Lsb,
			Mask);

	return XAie_MaskWrite32(DevInst, RegAddr, Mask, RegVal);
}

/*****************************************************************************/
/**
*
* This API updates the length of the buffer descriptor in the shim dma module.
*
* @param	DevInst: Device Instance.
* @param	DmaMod: Dma module pointer
* @param	Loc: Location of AIE Tile
* @param	Len: Length of BD in bytes.
* @param	BdNum: Hardware BD number to be written to.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only. This API accesses the hardware directly and does
*		not operate on software descriptor.
******************************************************************************/
AieRC _XAie_ShimDmaUpdateBdLen(XAie_DevInst *DevInst, const XAie_DmaMod *DmaMod,
		XAie_LocType Loc, u32 Len, u16 BdNum)
{
	u64 RegAddr;
	u32 RegVal;

	RegAddr = (u64)(DmaMod->BaseAddr + BdNum * (u64)DmaMod->IdxOffset) +
		XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
		DmaMod->BdProp->BufferLen.Idx * 4U;

	if ((_XAie_CheckPrecisionExceeds(DmaMod->BdProp->BufferLen.Lsb,
			_XAie_MaxBitsNeeded(Len),MAX_VALID_AIE_REG_BIT_INDEX))) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}

	RegVal = XAie_SetField(Len,
			DmaMod->BdProp->BufferLen.Lsb,
			DmaMod->BdProp->BufferLen.Mask);

	/* BD length register does not have other parameters */
	return XAie_Write32(DevInst, RegAddr, RegVal);
}

/*****************************************************************************/
/**
*
* This API updates the address of the buffer descriptor in the dma module.
*
* @param	DevInst: Device Instance.
* @param	DmaMod: Dma module pointer
* @param	Loc: Location of AIE Tile
* @param	Addr: Buffer address
* @param	BdNum: Hardware BD number to be written to.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only. This API accesses the hardware directly and does
*		not operate on software descriptor.
******************************************************************************/
AieRC _XAie_DmaUpdateBdAddr(XAie_DevInst *DevInst, const XAie_DmaMod *DmaMod,
		XAie_LocType Loc, u64 Addr, u16 BdNum)
{
	u64 RegAddr;
	u32 RegVal, Mask;

	RegAddr = (u64)(DmaMod->BaseAddr + BdNum * (u64)DmaMod->IdxOffset) +
		XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
		DmaMod->BdProp->Buffer->TileDmaBuff.BaseAddr.Idx * 4U;

	Mask = DmaMod->BdProp->Buffer->TileDmaBuff.BaseAddr.Mask;

	if ((_XAie_CheckPrecisionExceeds(DmaMod->BdProp->Buffer->TileDmaBuff.BaseAddr.Lsb,
			_XAie_MaxBitsNeeded(Addr & 0xFFFFFFFFU),MAX_VALID_AIE_REG_BIT_INDEX))) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	RegVal = XAie_SetField(Addr,
			DmaMod->BdProp->Buffer->TileDmaBuff.BaseAddr.Lsb, Mask);

	return XAie_MaskWrite32(DevInst, RegAddr, Mask, RegVal);
}

/*****************************************************************************/
/**
*
* This API updates the address of the buffer descriptor in the dma module.
*
* @param	DevInst: Device Instance.
* @param	DmaMod: Dma module pointer
* @param	Loc: Location of AIE Tile
* @param	Addr: Buffer address
* @param	BdNum: Hardware BD number to be written to.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only. This API accesses the hardware directly and does
*		not operate on software descriptor.
******************************************************************************/
AieRC _XAie_ShimDmaUpdateBdAddr(XAie_DevInst *DevInst,
		const XAie_DmaMod *DmaMod, XAie_LocType Loc, u64 Addr, u16 BdNum)
{
	AieRC RC;
	u64 RegAddr;
	u32 RegVal, Mask;

	RegAddr = (u64)(DmaMod->BaseAddr + BdNum * (u64)DmaMod->IdxOffset) +
		XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
		DmaMod->BdProp->Buffer->ShimDmaBuff.AddrLow.Idx * 4U;

	Mask = DmaMod->BdProp->Buffer->ShimDmaBuff.AddrLow.Mask;

	if ((_XAie_CheckPrecisionExceeds(DmaMod->BdProp->Buffer->ShimDmaBuff.AddrLow.Lsb,
			_XAie_MaxBitsNeeded(Addr & 0xFFFFFFFFU),MAX_VALID_AIE_REG_BIT_INDEX))) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	RegVal = XAie_SetField(Addr,
			DmaMod->BdProp->Buffer->ShimDmaBuff.AddrLow.Lsb, Mask);

	/* Addrlow maps to a single register without other fields. */
	RC =  XAie_Write32(DevInst, RegAddr, RegVal);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Failed to update lower 32 bits of address\n");
		return RC;
	}

	RegAddr = (u64)(DmaMod->BaseAddr + BdNum * (u64)DmaMod->IdxOffset) +
		XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) +
		DmaMod->BdProp->Buffer->ShimDmaBuff.AddrHigh.Idx * 4U;

	Mask = DmaMod->BdProp->Buffer->ShimDmaBuff.AddrHigh.Mask;

	if ((_XAie_CheckPrecisionExceeds(DmaMod->BdProp->Buffer->ShimDmaBuff.AddrHigh.Lsb,
			_XAie_MaxBitsNeeded(Addr & 0xFFFFFFFFU),MAX_VALID_AIE_REG_BIT_INDEX))) {
		XAIE_ERROR("Check Precision Exceeds Failed\n");
		return XAIE_ERR;
	}
	RegVal = XAie_SetField(Addr,
			DmaMod->BdProp->Buffer->ShimDmaBuff.AddrHigh.Lsb, Mask);

	return XAie_MaskWrite32(DevInst, RegAddr, Mask, RegVal);
}

/*****************************************************************************/
/**
*
* This API setups the iteration parameters for a Buffer descriptor.
*
* @param	DmaDesc: Initialized Dma Descriptor.
* @param	StepSize: Offset applied at each execution of the BD.
* @param	Wrap: Iteration Wrap.
* @param	IterCurr: Current iteration step. This field is incremented by
*		the hardware after BD is loaded.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only.
*
******************************************************************************/
AieRC _XAie_DmaSetBdIteration(XAie_DmaDesc *DmaDesc, u32 StepSize, u16 Wrap,
		u8 IterCurr)
{
	(void)DmaDesc;
	(void)StepSize;
	(void)Wrap;
	(void)IterCurr;

	return XAIE_FEATURE_NOT_SUPPORTED;
}

AieRC _XAie_AxiBurstLenCheck(u8 BurstLen)
{
	switch (BurstLen) {
	case 4:
	case 8:
	case 16:
		return XAIE_OK;
	default:
		return XAIE_INVALID_BURST_LENGTH;
	}
}

#endif /* XAIE_FEATURE_DMA_ENABLE */

/** @} */
