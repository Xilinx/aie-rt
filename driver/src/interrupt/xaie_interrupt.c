/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_interrupt.c
* @{
*
* This file contains routines for AIE interrupt module.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Nishad   07/21/2020  Initial creation
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xaie_helper.h"
#include "xaie_interrupt.h"

/************************** Constant Definitions *****************************/
/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This API enables/disables interrupts to first level interrupt controller.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	Switch: Switch in the given module. For shim tile value could be
*			XAIE_EVENT_SWITCH_A or XAIE_EVENT_SWITCH_B.
* @param	IntrId: Interrupt index to configure.
* @param	Enable: XAIE_ENABLE or XAIE_DISABLE to enable or disable.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		Internal Only.
*
******************************************************************************/
static AieRC _XAie_IntrCtrlL1Config(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_BroadcastSw Switch, u8 IntrId, u8 Enable)
{
	u64 RegAddr;
	u32 RegOffset;
	u8 TileType;
	const XAie_L1IntrMod *L1IntrMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid device instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid tile type\n");
		return XAIE_INVALID_TILE;
	}

	L1IntrMod = DevInst->DevProp.DevMod[TileType].L1IntrMod;

	if(L1IntrMod == NULL || IntrId >= L1IntrMod->NumIntrIds) {
		XAIE_ERROR("Invalid module type or interrupt ID\n");
		return XAIE_INVALID_ARGS;
	}

	if(Enable == XAIE_ENABLE)
		RegOffset = L1IntrMod->BaseEnableRegOff;
	else
		RegOffset = L1IntrMod->BaseDisableRegOff;

	RegAddr = _XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) + RegOffset +
					Switch * L1IntrMod->SwOff;

	XAie_Write32(DevInst, RegAddr, XAIE_ENABLE << IntrId);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API enables interrupts to first level interrupt controller.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	Switch: Switch in the given module. For shim tile value could be
*			XAIE_EVENT_SWITCH_A or XAIE_EVENT_SWITCH_B.
* @param	IntrId: Interrupt index to configure.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_IntrCtrlL1Enable(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_BroadcastSw Switch, u8 IntrId)
{
	return _XAie_IntrCtrlL1Config(DevInst, Loc, Switch, IntrId, XAIE_ENABLE);
}

/*****************************************************************************/
/**
*
* This API disables interrupts to first level interrupt controller.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	Switch: Switch in the given module. For shim tile value could be
*			XAIE_EVENT_SWITCH_A or XAIE_EVENT_SWITCH_B.
* @param	IntrId: Interrupt index to configure.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_IntrCtrlL1Disable(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_BroadcastSw Switch, u8 IntrId)
{
	return _XAie_IntrCtrlL1Config(DevInst, Loc, Switch, IntrId, XAIE_DISABLE);
}

/*****************************************************************************/
/**
*
* This API sets broadcast ID on which the interrupt from first level interrupt
* controller shall be driven to.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	Switch: Switch in the given module. For shim tile value could be
*			XAIE_EVENT_SWITCH_A or XAIE_EVENT_SWITCH_B.
* @param	BroadcastId: Broadcast index on which the interrupt shall be
*			     driven.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_IntrCtrlL1IrqSet(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_BroadcastSw Switch, u8 BroadcastId)
{
	u64 RegAddr;
	u32 RegOffset;
	u8 TileType;
	const XAie_L1IntrMod *L1IntrMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid device instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid tile type\n");
		return XAIE_INVALID_TILE;
	}

	L1IntrMod = DevInst->DevProp.DevMod[TileType].L1IntrMod;

	if(L1IntrMod == NULL || BroadcastId >= L1IntrMod->NumBroadcastIds) {
		XAIE_ERROR("Invalid module type or broadcast ID\n");
		return XAIE_INVALID_ARGS;
	}

	RegOffset = L1IntrMod->BaseIrqRegOff + Switch * L1IntrMod->SwOff;
	RegAddr = _XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) + RegOffset;

	XAie_Write32(DevInst, RegAddr, BroadcastId);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API maps an event which interrupts the first level interrupt controller
* at the given IRQ event index.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	Switch: Switch in the given module. For shim tile value could be
*			XAIE_EVENT_SWITCH_A or XAIE_EVENT_SWITCH_B.
* @param	IrqEventId: IRQ event index. Value 0 causes IRQ16,
*			    value 1 Causes IRQ17, and so on.
* @param	Event: Event ID to interrupt first level interrupt controller.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_IntrCtrlL1Event(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_BroadcastSw Switch, u8 IrqEventId, XAie_Events Event)
{
	u64 RegAddr;
	u32 RegOffset, EventMask, FldVal;
	u8 TileType, EventLsb, MappedEvent;
	const XAie_L1IntrMod *L1IntrMod;
	const XAie_EvntMod *EvntMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid device instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid tile type\n");
		return XAIE_INVALID_TILE;
	}

	L1IntrMod = DevInst->DevProp.DevMod[TileType].L1IntrMod;
	EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[0U];

	if(L1IntrMod == NULL || IrqEventId >= L1IntrMod->NumIrqEvents) {
		XAIE_ERROR("Invalid module type or IRQ event ID\n");
		return XAIE_INVALID_ARGS;
	}

	if(Event < EvntMod->EventMin || Event > EvntMod->EventMax) {
		XAIE_ERROR("Invalid event ID\n");
		return XAIE_INVALID_ARGS;
	}

	Event -= EvntMod->EventMin;
	MappedEvent = EvntMod->XAie_EventNumber[Event];
	if(MappedEvent == XAIE_EVENT_INVALID) {
		XAIE_ERROR("Invalid event ID\n");
		return XAIE_INVALID_ARGS;
	}

	RegOffset = L1IntrMod->BaseIrqEventRegOff + Switch * L1IntrMod->SwOff;
	EventLsb = IrqEventId * L1IntrMod->IrqEventOff;
	EventMask = L1IntrMod->BaseIrqEventMask << EventLsb;
	FldVal = XAie_SetField(MappedEvent, EventLsb, EventMask);
	RegAddr = _XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) + RegOffset;
	XAie_MaskWrite32(DevInst, RegAddr, EventMask, FldVal);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API blocks broadcast signals from AIE array at the first level interrupt
* controller. Unlike the tile switch, switches in the PL module have the ability
* to mask incoming signals from the AIE Tile after they have been passed to the
* first level interrupt handler. This prevents pollution of the broadcast
* network in case of interrupt usage.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	Switch: Switch in the given module. For shim tile value could be
*			XAIE_EVENT_SWITCH_A or XAIE_EVENT_SWITCH_B.
* @param	ChannelBitMap: Bitmap to block broadcast channels.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_IntrCtrlL1BroadcastBlock(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_BroadcastSw Switch, u32 ChannelBitMap)
{
	u64 RegAddr;
	u32 RegOffset;
	u8 TileType;
	const XAie_L1IntrMod *L1IntrMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid device instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid tile type\n");
		return XAIE_INVALID_TILE;
	}

	L1IntrMod = DevInst->DevProp.DevMod[TileType].L1IntrMod;
	if(L1IntrMod == NULL) {
		XAIE_ERROR("Invalid tile type\n");
		return XAIE_INVALID_ARGS;
	}

	if(ChannelBitMap >= (XAIE_ENABLE << L1IntrMod->NumBroadcastIds)) {
		XAIE_ERROR("Invalid channel bitmap\n");
		return XAIE_INVALID_ARGS;
	}

	RegOffset = L1IntrMod->BaseBroadcastBlockRegOff +
						Switch * L1IntrMod->SwOff;
	RegAddr = _XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) + RegOffset;

	XAie_Write32(DevInst, RegAddr, ChannelBitMap);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API unblocks broadcast signals from AIE array at the first level
* interrupt controller. Unlike the tile switch, switches in the PL module have
* the ability to mask incoming signals from the AIE Tile after they have been
* passed to the first level interrupt handler. This prevents pollution of the
* broadcast network in case of interrupt usage.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	Switch: Switch in the given module. For shim tile value could be
*			XAIE_EVENT_SWITCH_A or XAIE_EVENT_SWITCH_B.
* @param	ChannelBitMap: Bitmap to unblock broadcast channels.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		Each broadcast line is allocated and utilized for channeling
*		particular types of events. Hence this API enforces unblocking
*		broadcast line per ID basis rather than a bitmap.

******************************************************************************/
AieRC XAie_IntrCtrlL1BroadcastUnblock(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_BroadcastSw Switch, u32 ChannelBitMap)
{
	u64 RegAddr;
	u32 RegOffset;
	u8 TileType;
	const XAie_L1IntrMod *L1IntrMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid device instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid tile type\n");
		return XAIE_INVALID_TILE;
	}

	L1IntrMod = DevInst->DevProp.DevMod[TileType].L1IntrMod;
	if(L1IntrMod == NULL) {
		XAIE_ERROR("Invalid tile type\n");
		return XAIE_INVALID_ARGS;
	}

	if(ChannelBitMap >= (XAIE_ENABLE << L1IntrMod->NumBroadcastIds)) {
		XAIE_ERROR("Invalid channel bitmap\n");
		return XAIE_INVALID_ARGS;
	}

	RegOffset = L1IntrMod->BaseBroadcastUnblockRegOff +
						Switch * L1IntrMod->SwOff;
	RegAddr = _XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) + RegOffset;

	XAie_Write32(DevInst, RegAddr, ChannelBitMap);

	return XAIE_OK;
}

/** @} */
