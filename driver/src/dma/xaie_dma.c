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
* This file contains routines for AIE tile control.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   09/24/2019  Initial creation
* 1.1   Tejus	01/04/2020  Cleanup error messages
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xaie_dma.h"

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This API is used to  get the len of the data transfer from the dma tensor.
*
* @param	Tensor: Tensor descriptor of the data
*
* @return	Len: Number of 32-bit words to be transferred for the tensor.
*
* @note		None.
*
*******************************************************************************/
static u32 _XAie_GetBufferLenFromTensor(XAie_TensorDesc Tensor)
{
	u32 Len;
	/* Return 1D value from Tensor descriptor until more dimensions are
	 * supported.
	 */
	Len = Tensor.Shape[0];
	return Len;
}

/*****************************************************************************/
/**
*
* This API is used to setup a buffer descriptor for  a range of tiles.
*
* @param	DevInst: Device Instance
* @param	Range: Range of AIE Tiles
* @param	BdNum: Buffer descriptor number.
* @param	BaseAddr: Address of the 32-bit values.
* @param	Tensor: Tensor descriptor of the data
* @param	Acq: Lock descriptor for acquiring the lock
* @param	Rel: Lock descriptor for releasing the lock
* @param	Attr: Dma buffer descriptor attributes
*
* @return	XAIE_OK if successful or error code if failure.
*
* @note		Supports only 1D transfers. The API may change in future.
*
*******************************************************************************/
AieRC XAie_DmaBdConfig(XAie_DevInst *DevInst, XAie_LocRange Range, u8 BdNum,
		u32 BaseAddr, XAie_TensorDesc Tensor, XAie_LockDesc Acq,
		XAie_LockDesc Rel, XAie_DmaBdAttr Attr)
{

	u32 BdBuf[8] = {0};
	u64 Addr;
	u64 BdBaseAddr;
	u32 BufferLen;
	u8 TileType;
	const XAie_DmaMod *DmaMod;
	const XAie_DmaBdProp *BdProp;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAieLib_print("Error: Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	/* NOTE: Temporary check until AIE1 dma operations are supported */
	if(DevInst->DevProp.DevGen == XAIE_DEV_GEN_AIE) {
		XAieLib_print("Error: Operation not supported for device\
				version %d\n", XAIE_DEV_GEN_AIE);
		return XAIE_ERR;
	}

	if(_XAie_CheckLocRange(DevInst, Range) != XAIE_OK) {
		XAieLib_print("Error: Invalid Device Range\n");
		return XAIE_INVALID_RANGE;
	}

	TileType = _XAie_GetTileType(DevInst, Range);

	/* Supports only Tile Dma and Mem Tile Dma for now */
	if((TileType != XAIEGBL_TILE_TYPE_AIETILE) &&
			(TileType != XAIEGBL_TILE_TYPE_MEMTILE)){
		XAieLib_print("Error: Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	if(_XAie_CheckRangeTileType(DevInst, Range) != XAIE_OK) {
		XAieLib_print("Error: Range has different Tile Types\n");
		return XAIE_INVALID_RANGE;
	}

	DmaMod = DevInst->DevProp.DevMod[TileType].DmaMod;
	if(BdNum >= DmaMod->NumBds) {
		XAieLib_print("Error: Invalid BD\n");
		return XAIE_INVALID_BD_NUM;
	}

	if(_XAie_IsValidLock(Rel, DmaMod->NumLocks)) {
		XAieLib_print("Error: Invalid Release lock\n");
		return XAIE_INVALID_LOCK;
	}

	/* Calculate the Base address of the Bd */
	BdBaseAddr = DmaMod->BaseAddr + BdNum * DmaMod->IdxOffset;
	BdProp = DmaMod->BdProp;
	BufferLen = _XAie_GetBufferLenFromTensor(Tensor);

	/* Base address is always 32-bit aligned */
	BdBuf[BdProp->BaseAddr.Idx] |= XAie_SetField(BaseAddr >> 2,
			BdProp->BaseAddr.Lsb, BdProp->BaseAddr.Mask);
	BdBuf[BdProp->BufferLen.Idx] |= XAie_SetField(BufferLen,
			BdProp->BufferLen.Lsb, BdProp->BufferLen.Mask);
	BdBuf[BdProp->UseNxtBd.Idx] |= XAie_SetField(Attr.UseNxtBd,
			BdProp->UseNxtBd.Lsb, BdProp->UseNxtBd.Mask);
	BdBuf[BdProp->NxtBd.Idx] |= XAie_SetField(Attr.NxtBd,
			BdProp->NxtBd.Lsb, BdProp->NxtBd.Mask);
	BdBuf[BdProp->ValidBd.Idx] |= XAie_SetField(Attr.ValidBd,
			BdProp->ValidBd.Lsb, BdProp->ValidBd.Mask);
	BdBuf[BdProp->LckRelVal.Idx] |= XAie_SetField(Rel.Val,
			BdProp->LckRelVal.Lsb, BdProp->LckRelVal.Mask);
	BdBuf[BdProp->LckRelId.Idx] |= XAie_SetField(Rel.Id,
			BdProp->LckRelId.Lsb, BdProp->LckRelId.Mask);
	BdBuf[BdProp->LckAcqEn.Idx] |= XAie_SetField(Acq.En,
			BdProp->LckAcqEn.Lsb, BdProp->LckAcqEn.Mask);
	BdBuf[BdProp->LckAcqVal.Idx] |= XAie_SetField(Acq.Val,
			BdProp->LckAcqVal.Lsb, BdProp->LckAcqVal.Mask);
	BdBuf[BdProp->LckAcqId.Idx] |= XAie_SetField(Acq.Id,
			BdProp->LckAcqId.Lsb, BdProp->LckAcqId.Mask);
	BdBuf[BdProp->EnCompression.Idx] |= XAie_SetField(Attr.EnCompression,
			BdProp->EnCompression.Lsb, BdProp->EnCompression.Mask);
	BdBuf[BdProp->EnPkt.Idx] |= XAie_SetField(Attr.EnPkt,
			BdProp->EnPkt.Lsb, BdProp->EnPkt.Mask);
	BdBuf[BdProp->OutofOrderBdId.Idx] |= XAie_SetField(Attr.OutofOrderBdId,
			BdProp->OutofOrderBdId.Lsb, BdProp->OutofOrderBdId.Mask);
	BdBuf[BdProp->PktId.Idx] |= XAie_SetField(Attr.PktId,
			BdProp->PktId.Lsb, BdProp->PktId.Mask);
	BdBuf[BdProp->PktType.Idx] |= XAie_SetField(Attr.PktType,
			BdProp->PktType.Lsb, BdProp->PktType.Mask);

	/* TODO: Deduce Stepsize/Wraps for all dimensions from TensorDesc.
	 * For now, default all values to zero for 1D.
	 */
	BdBuf[BdProp->D1_StepSize.Idx] |= XAie_SetField(0,
			BdProp->D1_StepSize.Lsb, BdProp->D1_StepSize.Mask);
	BdBuf[BdProp->D0_StepSize.Idx] |= XAie_SetField(0,
			BdProp->D0_StepSize.Lsb, BdProp->D0_StepSize.Mask);
	BdBuf[BdProp->D1_Wrap.Idx] |= XAie_SetField(0,
			BdProp->D1_Wrap.Lsb, BdProp->D1_Wrap.Mask);
	BdBuf[BdProp->D0_Wrap.Idx] |= XAie_SetField(0,
			BdProp->D0_Wrap.Lsb, BdProp->D0_Wrap.Mask);
	BdBuf[BdProp->D2_StepSize.Idx] |= XAie_SetField(0,
			BdProp->D2_StepSize.Lsb, BdProp->D2_StepSize.Mask);
	BdBuf[BdProp->IterCurr.Idx] |= XAie_SetField(0,
			BdProp->IterCurr.Lsb, BdProp->IterCurr.Mask);
	BdBuf[BdProp->IterWrap.Idx] |= XAie_SetField(0,
			BdProp->IterWrap.Lsb, BdProp->IterWrap.Mask);
	BdBuf[BdProp->IterStepSize.Idx] |= XAie_SetField(0,
			BdProp->IterStepSize.Lsb, BdProp->IterStepSize.Mask);

	/* Write to the BD registers */
	for(u8 R = Range.Start.Row; R <= Range.End.Row; R += Range.Stride.Row) {
		for(u8 C = Range.Start.Col; C <= Range.End.Col; C += Range.Stride.Col) {

			Addr = DevInst->BaseAddr +
				_XAie_GetTileAddr(DevInst, R, C) + BdBaseAddr;

			for(u8 i = 0; i < DmaMod->NumBdReg; i++) {
				XAieGbl_Write32((Addr + i * 4), BdBuf[i]);
			}
		}
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API is used to reset/enable the selected DMA channel for a range of tile.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tiles
* @param	ChNum: Channel number.
* @param	StartBd: Buffer descriptor number to start the channel with.
* @param	Dir: Direction of transaction(DMA_MM2S or DMA_S2MM)
* @param	RptCnt: Repeat Count
* @param	EnToke: Enable Token
* @param	CtrlId: Controller ID for tokens generated
* @param	Attr: Dma buffer descriptor attributes
*
* @return	XAIE_SUCCESS if successful, error code for failure
*
* @note		API may change in future.
*
*******************************************************************************/
AieRC XAie_DmaEnChannel(XAie_DevInst *DevInst, XAie_LocType Loc, u8 ChNum,
		u8 StartBd, XAie_DmaDirection Dir, u8 RptCnt, u8 EnToken, u8 CtrlId,
		XAie_DmaBdAttr Attr)
{
	XAie_LocRange Range = {Loc, Loc, { 1, 1 }};
	return XAie_DmaEnChannelRange(DevInst, Range, ChNum, StartBd, Dir,
			RptCnt, EnToken, CtrlId, Attr);
}

/*****************************************************************************/
/**
*
* This API is used to setup a buffer descriptor in 1D for  a range of tiles.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tiles
* @param	BdNum: Buffer descriptor number.
* @param	BaseAddr: Address of the 32-bit values.
* @param	Tensor: Tensor descriptor of the data
* @param	Acq: Lock descriptor for acquiring the lock
* @param	Rel: Lock descriptor for releasing the lock
* @param	Attr: Dma buffer descriptor attributes
*
* @return	XAIE_OK if successful or error code if failure.
*
* @note		API may change in future.
*
*******************************************************************************/
AieRC XAie_DmaBdConfig_1D(XAie_DevInst *DevInst, XAie_LocType Loc, u8 BdNum,
		u32 BaseAddr, u32 Len, XAie_LockDesc Acq, XAie_LockDesc Rel,
		XAie_DmaBdAttr Attr)
{
	XAie_LocRange Range = {Loc, Loc, {1, 1}};
	return XAie_DmaBdConfig_1DRange(DevInst, Range, BdNum, BaseAddr, Len, 
			Acq, Rel, Attr);
}

/*****************************************************************************/
/**
*
* This API is used to setup a buffer descriptor in 1D for  a range of tiles.
*
* @param	DevInst: Device Instance
* @param	Range: Range of AIE Tiles
* @param	BdNum: Buffer descriptor number.
* @param	BaseAddr: Address of the 32-bit values.
* @param	Tensor: Tensor descriptor of the data
* @param	Acq: Lock descriptor for acquiring the lock
* @param	Rel: Lock descriptor for releasing the lock
* @param	Attr: Dma buffer descriptor attributes
*
* @return	XAIE_OK if successful or error code if failure.
*
* @note		API may change in future.
*
*******************************************************************************/
AieRC XAie_DmaBdConfig_1DRange(XAie_DevInst *DevInst, XAie_LocRange Range,
		u8 BdNum, u32 BaseAddr, u32 Len, XAie_LockDesc Acq,
		XAie_LockDesc Rel, XAie_DmaBdAttr Attr)
{
	XAie_TensorDesc Tensor;
	Tensor.Shape[0] = Len;
	return XAie_DmaBdConfig(DevInst, Range, BdNum, BaseAddr, Tensor, Acq,
			Rel, Attr);
}

/*****************************************************************************/
/**
*
* This API is used to reset/enable the selected DMA channel for a range of tile.
*
* @param	DevInst: Device Instance
* @param	Range: Range of AIE Tiles
* @param	ChNum: Channel number.
* @param	StartBd: Buffer descriptor number to start the channel with.
* @param	Dir: Direction of transaction(DMA_MM2S or DMA_S2MM)
* @param	RptCnt: Repeat Count
* @param	EnToke: Enable Token
* @param	CtrlId: Controller ID for tokens generated
* @param	Attr: Dma buffer descriptor attributes
*
* @return	XAIE_SUCCESS if successful, error code on failure.
*
* @note		API may change in future.
*
*******************************************************************************/
AieRC XAie_DmaEnChannelRange(XAie_DevInst *DevInst, XAie_LocRange Range,
		u8 ChNum, u8 StartBd, XAie_DmaDirection Dir, u8 RptCnt,
		u8 EnToken, u8 CtrlId, XAie_DmaBdAttr Attr)
{
	/*
	 * TODO: Check with architecture team if Reset bit has to be handled
	 * The Arm runtime simulator does not set/reset the reset field
	 */

	u32 ChData[2] = {0};
	u8 TileType;
	const XAie_DmaMod *DmaMod;
	const XAie_DmaChProp *ChProp;
	u64 BaseAddr;
	u64 Addr;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAieLib_print("Error: Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	/* NOTE: Temporary check until AIE1 dma operations are supported */
	if(DevInst->DevProp.DevGen == XAIE_DEV_GEN_AIE) {
		XAieLib_print("Error: Operation not supported for device\
				version %d\n", XAIE_DEV_GEN_AIE);
		return XAIE_ERR;
	}

	if(_XAie_CheckLocRange(DevInst, Range) != XAIE_OK) {
		XAieLib_print("Error: Invalid Device Range\n");
		return XAIE_INVALID_RANGE;
	}

	TileType = _XAie_GetTileType(DevInst, Range);
	/* Supports only Tile Dma and Mem Tile Dma for now */
	if((TileType != XAIEGBL_TILE_TYPE_AIETILE) &&
			(TileType != XAIEGBL_TILE_TYPE_MEMTILE)){
		XAieLib_print("Error: Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	if(_XAie_CheckRangeTileType(DevInst, Range) != XAIE_OK) {
		XAieLib_print("Error: Range has different Tile Types\n");
		return XAIE_INVALID_RANGE;
	}

	if(Dir >= DMA_MAX) {
		XAieLib_print("Error: DMA supports only MM2S/S2MM\n");
		return XAIE_INVALID_DMA_DIRECTION;
	}

	/* Get dma module pointer from device instance */
	DmaMod = DevInst->DevProp.DevMod[TileType].DmaMod;
	if(StartBd >= DmaMod->NumBds) {
		XAieLib_print("Error: Invalid BD\n");
		return XAIE_INVALID_BD_NUM;
	}

	if(ChNum >= DmaMod->NumChannels) {
		XAieLib_print("Error: Invalid Channel\n");
		return XAIE_INVALID_CHANNEL_NUM;
	}

	/* Get channel propert pointer from dma module */
	ChProp = DmaMod->ChProp;
	BaseAddr = DmaMod->ChCtrlBase + ChNum * DmaMod->ChIdxOffset +
		Dir * DmaMod->ChIdxOffset * DmaMod->NumChannels;

	ChData[ChProp->CtrlId.Idx] |= XAie_SetField(CtrlId, ChProp->CtrlId.Lsb,
			ChProp->CtrlId.Mask);

	ChData[ChProp->EnToken.Idx] |= XAie_SetField(EnToken, ChProp->EnToken.Lsb,
			ChProp->EnToken.Mask);

	ChData[ChProp->RptCount.Idx] |= XAie_SetField(RptCnt, ChProp->RptCount.Lsb,
			ChProp->RptCount.Mask);

	ChData[ChProp->EnCompression.Idx] |= XAie_SetField(Attr.EnCompression,
			ChProp->EnCompression.Lsb, ChProp->EnCompression.Mask);

	ChData[ChProp->StartBd.Idx] |= XAie_SetField(StartBd, ChProp->StartBd.Lsb,
			ChProp->StartBd.Mask);

	ChData[ChProp->EnOutofOrder.Idx] |= XAie_SetField(!!Attr.OutofOrderBdId,
			ChProp->EnOutofOrder.Lsb, ChProp->EnOutofOrder.Mask);

	for(u8 R = Range.Start.Row; R <= Range.End.Row; R += Range.Stride.Row) {
		for(u8 C = Range.Start.Col; C <= Range.End.Col; C += Range.Stride.Col) {

			Addr = DevInst->BaseAddr +
				_XAie_GetTileAddr(DevInst, R, C) + BaseAddr;

			for(u8 i = 0; i < 2; i++) {
				XAieGbl_Write32((Addr + i * 4), ChData[i]);
			}
		}
	}

	return XAIE_OK;
}

/** @} */
