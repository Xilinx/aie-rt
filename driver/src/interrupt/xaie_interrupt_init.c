/******************************************************************************
* Copyright (C) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_interrupt_init.c
* @{
*
* This file implements routines for initializing AIE interrupt network.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Nishad   07/21/2020  Initial creation
* 1.1   Nishad   07/23/2020  Add APIs to configure second level interrupt
*			     controller.
* 1.2   Nishad   07/23/2020  Add API to initialize error broadcast network.
* 1.3   Nishad   08/13/2020  Block error broadcasts from AIE array to shim
			     while setting up error network.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include <stdlib.h>

#include "xaie_clock.h"
#include "xaie_feature_config.h"
#include "xaie_reset_aie.h"
#include "xaie_helper.h"
#include "xaie_helper_internal.h"
#include "xaie_interrupt.h"
#include "xaie_npi.h"

#ifdef XAIE_FEATURE_INTR_INIT_ENABLE

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

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid tile type\n");
		return XAIE_INVALID_TILE;
	}

	L1IntrMod = DevInst->DevProp.DevMod[TileType].L1IntrMod;

	if(L1IntrMod == NULL || IntrId >= L1IntrMod->NumIntrIds) {
		XAIE_ERROR("Invalid module type or interrupt ID\n");
		return XAIE_INVALID_ARGS;
	}

	if(Enable == XAIE_ENABLE) {
		RegOffset = L1IntrMod->BaseEnableRegOff;
	} else {
		RegOffset = L1IntrMod->BaseDisableRegOff;
	}

	RegAddr = XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) + RegOffset +
					(u8)Switch * L1IntrMod->SwOff;

	return XAie_Write32(DevInst, RegAddr,(u32)(XAIE_ENABLE << IntrId));
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

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid tile type\n");
		return XAIE_INVALID_TILE;
	}

	L1IntrMod = DevInst->DevProp.DevMod[TileType].L1IntrMod;

	if(L1IntrMod == NULL || BroadcastId >= L1IntrMod->NumBroadcastIds) {
		XAIE_ERROR("Invalid module type or broadcast ID\n");
		return XAIE_INVALID_ARGS;
	}

	RegOffset = L1IntrMod->BaseIrqRegOff +(u32)((u8)Switch * L1IntrMod->SwOff);
	RegAddr = XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) + RegOffset;

	return XAie_Write32(DevInst, RegAddr, BroadcastId);
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
	u32 EventVal;
	u8 TileType, EventLsb, MappedEvent;
	const XAie_L1IntrMod *L1IntrMod;
	const XAie_EvntMod *EvntMod;

	EventVal = (u32)Event;
	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid device instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
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

	if(EventVal < EvntMod->EventMin || EventVal > EvntMod->EventMax) {
		XAIE_ERROR("Invalid event ID\n");
		return XAIE_INVALID_ARGS;
	}

	EventVal -= EvntMod->EventMin;
	MappedEvent = EvntMod->XAie_EventNumber[EventVal];
	if(MappedEvent == XAIE_EVENT_INVALID) {
		XAIE_ERROR("Invalid event ID\n");
		return XAIE_INVALID_ARGS;
	}

	RegOffset = L1IntrMod->BaseIrqEventRegOff + (u32)((u8)Switch * L1IntrMod->SwOff);
	EventLsb = IrqEventId * L1IntrMod->IrqEventOff;
	EventMask = L1IntrMod->BaseIrqEventMask << EventLsb;
	FldVal = XAie_SetField(MappedEvent, EventLsb, EventMask);
	RegAddr = XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) + RegOffset;

	return XAie_MaskWrite32(DevInst, RegAddr, EventMask, FldVal);
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

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid tile type\n");
		return XAIE_INVALID_TILE;
	}

	L1IntrMod = DevInst->DevProp.DevMod[TileType].L1IntrMod;
	if(L1IntrMod == NULL) {
		XAIE_ERROR("Invalid tile type\n");
		return XAIE_INVALID_ARGS;
	}

	if(ChannelBitMap >= (u32)(XAIE_ENABLE << L1IntrMod->NumBroadcastIds)) {
		XAIE_ERROR("Invalid channel bitmap\n");
		return XAIE_INVALID_ARGS;
	}

	RegOffset = L1IntrMod->BaseBroadcastBlockRegOff +
						(u32)((u8)Switch * L1IntrMod->SwOff);
	RegAddr = XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) + RegOffset;

	return XAie_Write32(DevInst, RegAddr, ChannelBitMap);
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

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid tile type\n");
		return XAIE_INVALID_TILE;
	}

	L1IntrMod = DevInst->DevProp.DevMod[TileType].L1IntrMod;
	if(L1IntrMod == NULL) {
		XAIE_ERROR("Invalid tile type\n");
		return XAIE_INVALID_ARGS;
	}

	if(ChannelBitMap >= (u32)(XAIE_ENABLE << L1IntrMod->NumBroadcastIds)) {
		XAIE_ERROR("Invalid channel bitmap\n");
		return XAIE_INVALID_ARGS;
	}

	RegOffset = L1IntrMod->BaseBroadcastUnblockRegOff +
						(u32)((u8)Switch * L1IntrMod->SwOff);
	RegAddr = XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) + RegOffset;

	return XAie_Write32(DevInst, RegAddr, ChannelBitMap);
}

/*****************************************************************************/
/**
*
* This API returns the status an event.
*
* @param	DevInst: Device Instance.
* @param	Loc: Location of AIE tile.
* @param	Module: Module type.
* @param	Event: Physical event ID.
*
* @return	True is event was asserted, otherwise false.
*
* @note		Internal only.
*
******************************************************************************/
u8 XAie_EventReadStatusHw(XAie_DevInst *DevInst,
		XAie_LocType Loc, XAie_ModuleType Module, u8 Event)
{
	AieRC RC;
	u64 RegAddr;
	u32 RegOff, RegVal = 0;
	u8 TileType;
	const XAie_EvntMod *EvntMod;

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid tile type\n");
		return 0;
	}

	RC = XAie_CheckModule(DevInst, Loc, Module);
	if(RC != XAIE_OK) {
		return 0;
	}

	if (Module == XAIE_PL_MOD) {
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[0U];
	} else {
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[Module];
	}

	RegOff = EvntMod->BaseStatusRegOff + (((u32)Event / (u32)32U) * (u32)4U);
	RegAddr = XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) + RegOff;
	RC = XAie_Read32(DevInst, RegAddr, &RegVal);
	if(RC != XAIE_OK) {
		return 0;
	}

	RegVal = (u8)((RegVal >> ((u32)Event % (u32)32U)) & (u32)1U);

	return (u8)RegVal;
}

/*****************************************************************************/
/**
*
* This API clears the status of first-level interrupt controller.
*
* @param	DevInst: Device Instance.
* @param	Loc: Location of AIE tile.
* @param	Switch: Broadcast switch.
* @param	ChannelBitMap: Bitmap of channel statues to be cleared.
*
* @return	None.
*
* @note		Internal only.
*
******************************************************************************/
void XAie_IntrCtrlL1Ack(XAie_DevInst *DevInst,
			XAie_LocType Loc, XAie_BroadcastSw Switch,
			u32 ChannelBitMap)
{
	const XAie_L1IntrMod *L1Mod;
	u8 TileType;
	u64 RegAddr;

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if ((TileType != XAIEGBL_TILE_TYPE_SHIMPL) &&
	    (TileType != XAIEGBL_TILE_TYPE_SHIMNOC)) {
		return;
	}

	L1Mod = DevInst->DevProp.DevMod[XAIEGBL_TILE_TYPE_SHIMPL].L1IntrMod;
	if (L1Mod == NULL) {
		return;
	}
	RegAddr = (u64)L1Mod->BaseStatusRegOff + ((u64)L1Mod->SwOff * (u64)Switch) +
		  XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);
	(void)XAie_Write32(DevInst, RegAddr, ChannelBitMap);
}

/*****************************************************************************/
/**
*
* This API return the bitmap value of second level interrupts channels enabled.
*
* @param	DevInst: Device Instance.
* @param	Loc: Location of AIE Tile.
*
* @return	Channel bitmap.
*
* @note		Internal Only.
*
******************************************************************************/
u32 XAie_IntrCtrlL2Mask(XAie_DevInst *DevInst, XAie_LocType Loc)
{
	const XAie_L2IntrMod *L2Mod;
	u8 TileType;
	u64 RegAddr;
	u32 MaskVal = 0;
	AieRC RC;

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if (TileType != XAIEGBL_TILE_TYPE_SHIMNOC) {
		return 0;
	}

	L2Mod = DevInst->DevProp.DevMod[XAIEGBL_TILE_TYPE_SHIMNOC].L2IntrMod;
	if (L2Mod == NULL) {
		return 0;
	}
	RegAddr = L2Mod->MaskRegOff + XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);
	RC = XAie_Read32(DevInst, RegAddr, &MaskVal);
	if (RC != XAIE_OK) {
		MaskVal = 0;
	}

	return MaskVal;
}

/*****************************************************************************/
/**
*
* This API clears the event status.
*
* @param	DevInst: Device Instance.
* @param	Loc: Location of AIE tile.
* @param	Module: Module type.
* @param	Event: Physical event ID.
*
* @return	None.
*
* @note		Internal only.
*
******************************************************************************/
void XAie_EventClearStatus(XAie_DevInst *DevInst,
		XAie_LocType Loc, XAie_ModuleType Module, u8 Event)
{
	AieRC RC;
	u64 RegAddr;
	u32 RegOff, RegVal;
	u8 TileType;
	const XAie_EvntMod *EvntMod;

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAIE_ERROR("Invalid tile type\n");
		return;
	}

	RC = XAie_CheckModule(DevInst, Loc, Module);
	if(RC != XAIE_OK) {
		return;
	}

	if (Module == XAIE_PL_MOD) {
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[0U];
	} else {
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[Module];
	}

	RegOff = EvntMod->BaseStatusRegOff + (u32)(((u32)Event / (u32)32U) * (u32)4U);
	RegAddr = XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) + RegOff;

	RegVal = (u32)((u32)1UL << ((u32)Event % (u32)32U));
	(void)XAie_Write32(DevInst, RegAddr, RegVal);

	return;
}

/*****************************************************************************/
/**
*
* This API returns the status of first-level interrupt controller.
*
* @param	DevInst: Device Instance.
* @param	Loc: Location of AIE tile.
* @param	Switch: Broadcast switch.
*
* @return	Status first-level interrupt controller.
*
* @note		Internal only.
*
******************************************************************************/
u32 XAie_IntrCtrlL1Status(XAie_DevInst *DevInst,
			XAie_LocType Loc, XAie_BroadcastSw Switch)
{
	const XAie_L1IntrMod *L1Mod;
	u8 TileType;
	u64 RegAddr;
	u32 StatusVal = 0;
	AieRC RC;

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	switch (TileType) {
	case XAIEGBL_TILE_TYPE_SHIMPL:
	case XAIEGBL_TILE_TYPE_SHIMNOC:
	      L1Mod = DevInst->DevProp.DevMod[XAIEGBL_TILE_TYPE_SHIMPL].L1IntrMod;
	      break;
	default:
	      XAIE_ERROR("No L1Ctrl. Not a NOC or PL tile\n");
	      return 0;
	}

	if (L1Mod == NULL) {
		return 0;
	}
	RegAddr = (u64)L1Mod->BaseStatusRegOff + ((u64)L1Mod->SwOff * (u64)Switch) +
		  XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);
	RC = XAie_Read32(DevInst, RegAddr, &StatusVal);
	if (RC != XAIE_OK) {
		StatusVal = 0;
	}

	return StatusVal;
}

/*****************************************************************************/
/**
*
* This API enables/disables interrupts to second level interrupt controller.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	ChannelBitMap: Interrupt Bitmap.
* @param	Enable: XAIE_ENABLE or XAIE_DISABLE to enable or disable.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		Internal Only.
*
******************************************************************************/
static AieRC _XAie_IntrCtrlL2Config(XAie_DevInst *DevInst, XAie_LocType Loc,
		u32 ChannelBitMap, u8 Enable)
{
	u64 RegAddr;
	u32 RegOffset;
	u8 TileType;
	const XAie_L2IntrMod *L2IntrMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid device instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType != XAIEGBL_TILE_TYPE_SHIMNOC) {
		XAIE_ERROR("Invalid tile type\n");
		return XAIE_INVALID_TILE;
	}

	L2IntrMod = DevInst->DevProp.DevMod[TileType].L2IntrMod;

	if(ChannelBitMap >= (u32)(XAIE_ENABLE << L2IntrMod->NumBroadcastIds)) {
		XAIE_ERROR("Invalid interrupt bitmap\n");
		return XAIE_INVALID_ARGS;
	}

	if(Enable == XAIE_ENABLE) {
		RegOffset = L2IntrMod->EnableRegOff;
	} else {
		RegOffset = L2IntrMod->DisableRegOff;
	}

	RegAddr = XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) + RegOffset;

	return XAie_Write32(DevInst, RegAddr, ChannelBitMap);
}

/*****************************************************************************/
/**
*
* This API enables interrupts to second level interrupt controller.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	ChannelBitMap: Interrupt bit map.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_IntrCtrlL2Enable(XAie_DevInst *DevInst, XAie_LocType Loc,
		u32 ChannelBitMap)
{
	return _XAie_IntrCtrlL2Config(DevInst, Loc, ChannelBitMap, XAIE_ENABLE);
}

/*****************************************************************************/
/**
*
* This API returns L2status.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
*
* @return	L2Status or 0 incase of error.
*
* @note		None.
*
******************************************************************************/
u32 XAie_IntrCtrlL2Status(XAie_DevInst *DevInst, XAie_LocType Loc)
{
	const XAie_L2IntrMod *L2Mod;
	u8 TileType;
	u64 RegAddr;
	u32 Status = 0;
	AieRC RC;

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if (TileType != XAIEGBL_TILE_TYPE_SHIMNOC) {
		return 0;
	}

	L2Mod = DevInst->DevProp.DevMod[XAIEGBL_TILE_TYPE_SHIMNOC].L2IntrMod;
	if (L2Mod == NULL) {
		return 0;
	}
	RegAddr = L2Mod->StatusRegOff + XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);
	RC = XAie_Read32(DevInst, RegAddr, &Status);
	if (RC != XAIE_OK) {
		Status = 0;
	}

	return Status;

}

/*****************************************************************************/
/**
*
* This API ACKs the L2 controller. i.e Write to L2 Status register with a bitmap.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	Status: Bitmap of status to ack.
*
* @return	L2Status or 0 incase of error.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_IntrCtrlL2Ack(XAie_DevInst *DevInst, XAie_LocType Loc, u32 Status)
{
	const XAie_L2IntrMod *L2Mod;
	u8 TileType;
	u64 RegAddr;
	AieRC RC;

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if (TileType != XAIEGBL_TILE_TYPE_SHIMNOC) {
		return XAIE_FEATURE_NOT_SUPPORTED;
	}

	L2Mod = DevInst->DevProp.DevMod[XAIEGBL_TILE_TYPE_SHIMNOC].L2IntrMod;
	if (L2Mod == NULL) {
		return XAIE_FEATURE_NOT_SUPPORTED;
	}
	RegAddr = L2Mod->StatusRegOff + XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);
	RC = XAie_Write32(DevInst, RegAddr, Status);
	return RC;
}



/*****************************************************************************/
/**
*
* This API disables interrupts to second level interrupt controller.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	ChannelBitMap: Interrupt bit map.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_IntrCtrlL2Disable(XAie_DevInst *DevInst, XAie_LocType Loc,
		u32 ChannelBitMap)
{
	return _XAie_IntrCtrlL2Config(DevInst, Loc, ChannelBitMap, XAIE_DISABLE);
}

/*****************************************************************************/
/**
* This API enables default group error events which are marked as fatal errors.
* It also channels them on broadcast line #0.
*
* @param	DevInst: Device Instance
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		None.
*
******************************************************************************/
static AieRC _XAie_GroupErrorInit(XAie_DevInst *DevInst)
{
	AieRC RC;
	u32 GroupErrorEnableMask;
	u8 MemTileStart, MemTileEnd, AieRowStart, AieRowEnd;
	XAie_LocType Loc;

	MemTileStart = DevInst->MemTileRowStart;
	MemTileEnd = DevInst->MemTileRowStart + DevInst->MemTileNumRows;
	AieRowStart = DevInst->AieTileRowStart;
	AieRowEnd = DevInst->AieTileRowStart + DevInst->AieTileNumRows;

	for(u8 Col = 0; Col < DevInst->NumCols; Col++) {
		for(u8 Row = AieRowStart; Row < AieRowEnd; Row++) {
			Loc = XAie_TileLoc(Col, Row);

			if (_XAie_PmIsTileRequested(DevInst, Loc) == XAIE_DISABLE) {
				continue;
			}

			GroupErrorEnableMask = _XAie_GetFatalGroupErrors(DevInst,
							Loc, XAIE_MEM_MOD);
			RC = XAie_EventGroupControl(DevInst, Loc, XAIE_MEM_MOD,
					XAIE_EVENT_GROUP_ERRORS_MEM,
					GroupErrorEnableMask);
			if(RC != XAIE_OK) {
				XAIE_ERROR("Failed to configure group errors in memory module\n");
				return RC;
			}

			RC = XAie_EventBroadcast(DevInst, Loc, XAIE_MEM_MOD,
					XAIE_ERROR_BROADCAST_ID,
					XAIE_EVENT_GROUP_ERRORS_MEM);
			if(RC != XAIE_OK) {
				XAIE_ERROR("Failed to setup error broadcast for memory module\n");
				return RC;
			}

			GroupErrorEnableMask = _XAie_GetFatalGroupErrors(DevInst,
							Loc, XAIE_CORE_MOD);
			RC = XAie_EventGroupControl(DevInst, Loc, XAIE_CORE_MOD,
					XAIE_EVENT_GROUP_ERRORS_0_CORE,
					GroupErrorEnableMask);
			if(RC != XAIE_OK) {
				XAIE_ERROR("Failed to configure group error in core module\n");
				return RC;
			}

			RC = XAie_EventBroadcast(DevInst, Loc, XAIE_CORE_MOD,
					XAIE_ERROR_BROADCAST_ID,
					XAIE_EVENT_GROUP_ERRORS_0_CORE);
			if(RC != XAIE_OK) {
				XAIE_ERROR("Failed to setup error broadcast for core module\n");
				return RC;
			}
		}

		for(u8 MemRow = MemTileStart; MemRow < MemTileEnd; MemRow++) {
			Loc = XAie_TileLoc(Col, MemRow);

			if (_XAie_PmIsTileRequested(DevInst, Loc) == XAIE_DISABLE) {
				continue;
			}

			GroupErrorEnableMask = _XAie_GetFatalGroupErrors(DevInst,
							Loc, XAIE_MEM_MOD);
			RC = XAie_EventGroupControl(DevInst, Loc, XAIE_MEM_MOD,
					XAIE_EVENT_GROUP_ERRORS_MEM_TILE,
					GroupErrorEnableMask);
			if(RC != XAIE_OK) {
				XAIE_ERROR("Failed to configure group error in mem tile\n");
				return RC;
			}

			RC = XAie_EventBroadcast(DevInst, Loc, XAIE_MEM_MOD,
					XAIE_ERROR_BROADCAST_ID,
					XAIE_EVENT_GROUP_ERRORS_MEM_TILE);
			if(RC != XAIE_OK) {
				XAIE_ERROR("Failed to setup error broadcast for mem tile\n");
				return RC;
			}
		}

		/*
		 * Shim tile only needs to setup error notification with first
		 * level interrupt controller.
		 */
		Loc = XAie_TileLoc(Col, DevInst->ShimRow);
		GroupErrorEnableMask = _XAie_GetFatalGroupErrors(DevInst, Loc,
								XAIE_PL_MOD);
		RC = XAie_EventGroupControl(DevInst, Loc, XAIE_PL_MOD,
					XAIE_EVENT_GROUP_ERRORS_PL,
					GroupErrorEnableMask);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Failed to configure group error in shim tile\n");
			return RC;
		}

		RC = XAie_IntrCtrlL1Event(DevInst, Loc, XAIE_EVENT_SWITCH_A,
				XAIE_ERROR_BROADCAST_ID,
				XAIE_EVENT_GROUP_ERRORS_PL);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Failed to setup L1 internal error interrupt in shim tile\n");
			return RC;
		}
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
 * This API calls API to Configure Error halt register with group Error0. This
 * will put core in halt state if any group error0 occurs.
 *
 * @param	DevInst: Device Instance
 *
 * @return       XAIE_OK on success, error code on failure.
 *
 * @note	 This function is used internally only.
 *
 ******************************************************************************/
static AieRC _XAie_ErrorHandlingEventHaltCore(XAie_DevInst *DevInst)
{
	AieRC RC;
	u8 AieRowStart, AieRowEnd;

	AieRowStart = DevInst->AieTileRowStart;
	AieRowEnd = DevInst->AieTileRowStart + DevInst->AieTileNumRows;

	for(u8 Col = DevInst->StartCol; Col < DevInst->NumCols; Col++) {
		for(u8 Row = AieRowStart; Row < AieRowEnd; Row++) {
			XAie_LocType Loc;
			Loc = XAie_TileLoc(Col, Row);

			if (_XAie_PmIsTileRequested(DevInst, Loc) == XAIE_DISABLE) {
				continue;
			}

			RC = XAie_CoreConfigureErrorHaltEvent(DevInst, Loc,
					XAIE_EVENT_GROUP_ERRORS_0_CORE);
			if(RC != XAIE_OK) {
				return RC;
			}
		}
	}
	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API finds the location on next NoC tile with respect to the current
* shim tile.
*
* @param	DevInst: Device Instance
* @param	Loc: Current shim tile
* @param	NextLoc: Pointer to return location of NoC tile
*
* @return	XAIE_OK on success, error code if no NoC tile is found.
*
* @note		This function is used internally only.
******************************************************************************/
static AieRC _XAie_FindNextNoCTile(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_LocType *NextLoc)
{
	while (++Loc.Col < DevInst->NumCols) {
		u8 TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
		if (TileType == XAIEGBL_TILE_TYPE_SHIMNOC) {
			NextLoc->Col = Loc.Col;
			NextLoc->Row = Loc.Row;
			return XAIE_OK;
		}
	}

	return XAIE_ERR;
}

/*****************************************************************************/
/**
*
* This API configures broadcast network for AIE tile in AIE2PS.
*
* @param	DevInst: Device Instance
*
* @Loc		Location of SHIM tile.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		internal only
*
******************************************************************************/
static AieRC _XAie_ErrorHandlingInitAie2psAieTile(XAie_DevInst *DevInst, XAie_LocType Loc)
{
	u8 TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	u32 BroadcastBitMap, BroadcastBlockDir;
	AieRC RC;

	if (TileType != XAIEGBL_TILE_TYPE_AIETILE) {
		XAIE_ERROR("Not a aietile.\n");
		return XAIE_INVALID_TILE;
	}

	/* Block broadcast 0 from propagating to north, east and west
	 */
	BroadcastBlockDir = XAIE_EVENT_BROADCAST_NORTH |
			    XAIE_EVENT_BROADCAST_EAST |
			    XAIE_EVENT_BROADCAST_WEST;
	RC = XAie_EventBroadcastBlockDir(DevInst, Loc,
		   XAIE_CORE_MOD, XAIE_EVENT_SWITCH_A,
		   XAIE_ERROR_BROADCAST_ID, BroadcastBlockDir);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Failed to block broadcasts in core module\n");
		return RC;
	}

	RC = XAie_EventBroadcastBlockDir(DevInst, Loc,
		   XAIE_MEM_MOD, XAIE_EVENT_SWITCH_A,
		   XAIE_ERROR_BROADCAST_ID, BroadcastBlockDir);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Failed to block broadcasts in memory module\n");
		return RC;
	}
	/* Block broadcast 1 and 2 from propagating to north, west, south, east
	 */
	BroadcastBitMap = BIT(XAIE_ERROR_BROADCAST_ID_UC_EVENT) |
			  BIT(XAIE_ERROR_BROADCAST_ID_USER_EVENT1);
	RC = XAie_EventBroadcastBlockMapDir(DevInst, Loc,
		   XAIE_CORE_MOD, XAIE_EVENT_SWITCH_A,
		   BroadcastBitMap, XAIE_EVENT_BROADCAST_ALL);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Failed to block broadcasts 1, 2  in core module\n");
		return RC;
	}
	RC = XAie_EventBroadcastBlockMapDir(DevInst, Loc,
		   XAIE_MEM_MOD, XAIE_EVENT_SWITCH_A,
		   BroadcastBitMap, XAIE_EVENT_BROADCAST_ALL);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Failed to block broadcasts 1, 2  in mem module\n");
		return RC;
	}

	return 0;
}

/*****************************************************************************/
/**
*
* This API configures broadcast network for Mem tile in AIE2PS.
*
* @param	DevInst: Device Instance
*
* @Loc		Location of SHIM tile.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		internal only
*
******************************************************************************/
static AieRC _XAie_ErrorHandlingInitAie2psMemTile(XAie_DevInst *DevInst, XAie_LocType Loc)
{
	u8 TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	u32 BroadcastBitMap, BroadcastBlockDir;
	AieRC RC;

	if (TileType != XAIEGBL_TILE_TYPE_MEMTILE) {
		XAIE_ERROR("Not a memtile.\n");
		return XAIE_INVALID_TILE;
	}
	/* Block broadcast 0 from propagating to north, east and west
	 */
	BroadcastBlockDir = XAIE_EVENT_BROADCAST_NORTH |
			    XAIE_EVENT_BROADCAST_EAST |
			    XAIE_EVENT_BROADCAST_WEST;
	RC = XAie_EventBroadcastBlockDir(DevInst, Loc,
		   XAIE_MEM_MOD, XAIE_EVENT_SWITCH_A,
		   XAIE_ERROR_BROADCAST_ID, BroadcastBlockDir);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Failed to block broadcasts in mem tile switch A\n");
		return RC;
	}

	RC = XAie_EventBroadcastBlockDir(DevInst, Loc,
		   XAIE_MEM_MOD, XAIE_EVENT_SWITCH_B,
		   XAIE_ERROR_BROADCAST_ID, BroadcastBlockDir);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Failed to block broadcasts in mem tile switch B\n");
		return RC;
	}
	/* Block broadcast 1 and 2 from propagating to north, west, south, east
	 */
	BroadcastBitMap = XAIE_ERROR_BROADCAST_ID_UC_EVENT |
			  XAIE_ERROR_BROADCAST_ID_USER_EVENT1;
	RC = XAie_EventBroadcastBlockMapDir(DevInst, Loc,
		   XAIE_MEM_MOD, XAIE_EVENT_SWITCH_A,
		   BroadcastBitMap, XAIE_EVENT_BROADCAST_ALL);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Failed to block broadcasts 1, 2  in core module\n");
		return RC;
	}
	RC = XAie_EventBroadcastBlockMapDir(DevInst, Loc,
		   XAIE_MEM_MOD, XAIE_EVENT_SWITCH_B,
		   BroadcastBitMap, XAIE_EVENT_BROADCAST_ALL);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Failed to block broadcasts 1, 2  in mem module\n");
		return RC;
	}

	return 0;
}

/*****************************************************************************/
/**
*
* This API configures broadcast network for SHIM tile for coli 0.
*
* @param	DevInst: Device Instance
*
* @Loc		Location of SHIM tile.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		internal only
*
******************************************************************************/
static AieRC _XAie_ErrorHandlingInitAie2psShimTileCol0(XAie_DevInst *DevInst, XAie_LocType Loc)
{
	u8 TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	u32 BroadcastBitMap, BroadcastBlockDir;
	AieRC RC;

	if ((TileType != XAIEGBL_TILE_TYPE_SHIMNOC) &&
	    (TileType != XAIEGBL_TILE_TYPE_SHIMPL)) {
		XAIE_ERROR("Not a shimtile\n");
		return XAIE_INVALID_TILE;
	}
	if (Loc.Col != 0) {
		XAIE_ERROR("Col shoule be 0.\n");
		return XAIE_INVALID_TILE;
	}
	/* For Col 0, propagate broadcast 2 east only
	 */
	BroadcastBlockDir = XAIE_EVENT_BROADCAST_NORTH |
			    XAIE_EVENT_BROADCAST_WEST |
			    XAIE_EVENT_BROADCAST_SOUTH;
	RC = XAie_EventBroadcastBlockDir(DevInst, Loc,
			XAIE_PL_MOD, XAIE_EVENT_SWITCH_A,
			XAIE_ERROR_BROADCAST_ID_USER_EVENT1,
			BroadcastBlockDir);
	if (RC != XAIE_OK) {
		XAIE_ERROR("Failed to block broadcasts Loc: [%d, %d]: %d\n", Loc.Col, Loc.Row, RC);
		return RC;
	}
	RC = XAie_EventBroadcastBlockDir(DevInst, Loc,
			XAIE_PL_MOD, XAIE_EVENT_SWITCH_B,
			XAIE_ERROR_BROADCAST_ID_USER_EVENT1,
			BroadcastBlockDir);
	if (RC != XAIE_OK) {
		XAIE_ERROR("Failed to block broadcasts Loc: [%d, %d]: %d\n", Loc.Col, Loc.Row, RC);
		return RC;
	}
	/* For Col 0, block broadcast 0,1 in all directions
	 */
	BroadcastBitMap = BIT(XAIE_ERROR_BROADCAST_ID) |
			  BIT(XAIE_ERROR_BROADCAST_ID_UC_EVENT);
	BroadcastBlockDir = XAIE_EVENT_BROADCAST_NORTH |
			    XAIE_EVENT_BROADCAST_WEST |
			    XAIE_EVENT_BROADCAST_SOUTH;
	RC = XAie_EventBroadcastBlockMapDir(DevInst, Loc,
			XAIE_PL_MOD, XAIE_EVENT_SWITCH_A,
			BroadcastBitMap,
			BroadcastBlockDir);
	if (RC != XAIE_OK) {
		XAIE_ERROR("Failed to block broadcasts Loc: [%d, %d]: %d\n", Loc.Col, Loc.Row, RC);
		return RC;
	}
	RC = XAie_EventBroadcastBlockMapDir(DevInst, Loc,
			XAIE_PL_MOD, XAIE_EVENT_SWITCH_B,
			BroadcastBitMap,
			XAIE_EVENT_BROADCAST_ALL);
	if (RC != XAIE_OK) {
		XAIE_ERROR("Failed to block broadcasts Loc: [%d, %d]: %d\n", Loc.Col, Loc.Row, RC);
		return RC;
	}

	return 0;
}

/*****************************************************************************/
/**
*
* This API configures broadcast network for SHIM tile for lead col.
* Lead column for aie2ps is assumed to be Col 1.
*
* @param	DevInst: Device Instance
*
* @Loc		Location of SHIM tile.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		internal only
*
******************************************************************************/
static AieRC _XAie_ErrorHandlingInitAie2psShimTileLeadCol(XAie_DevInst *DevInst, XAie_LocType Loc)
{
	u8 TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	u32 BroadcastBitMap, BroadcastBlockDir;
	AieRC RC;

	if ((TileType != XAIEGBL_TILE_TYPE_SHIMNOC) &&
	    (TileType != XAIEGBL_TILE_TYPE_SHIMPL)) {
		XAIE_ERROR("Not a shimtile\n");
		return XAIE_INVALID_TILE;
	}
	if (Loc.Col != 1) {
		XAIE_ERROR("Lead col should be 1.\n");
		return XAIE_INVALID_TILE;
	}
	/* For column 1, propagate broadcast for 0,1 to west.
	 */
	BroadcastBitMap = BIT(XAIE_ERROR_BROADCAST_ID) |
			  BIT(XAIE_ERROR_BROADCAST_ID_UC_EVENT);
	BroadcastBlockDir = XAIE_EVENT_BROADCAST_NORTH |
			    XAIE_EVENT_BROADCAST_EAST |
			    XAIE_EVENT_BROADCAST_SOUTH;

	RC = XAie_EventBroadcastBlockMapDir(DevInst, Loc,
			XAIE_PL_MOD, XAIE_EVENT_SWITCH_A,
			BroadcastBitMap, BroadcastBlockDir);
	if (RC != XAIE_OK) {
		XAIE_ERROR("Failed to block broadcasts in switch A Loc: [%d, %d]: %d\n", Loc.Col, Loc.Row, RC);
		return RC;
	}
	RC = XAie_EventBroadcastBlockMapDir(DevInst, Loc,
			XAIE_PL_MOD, XAIE_EVENT_SWITCH_B,
			BroadcastBitMap, BroadcastBlockDir);

	if (RC != XAIE_OK) {
		XAIE_ERROR("Failed to block broadcasts in switch B Loc: [%d, %d]: %d\n", Loc.Col, Loc.Row, RC);
		return RC;
	}
	/* For column 1, block broadcast for id 2 in all directions
	 */
	BroadcastBlockDir = XAIE_EVENT_BROADCAST_NORTH |
			    XAIE_EVENT_BROADCAST_WEST |
			    XAIE_EVENT_BROADCAST_SOUTH;
	RC = XAie_EventBroadcastBlockDir(DevInst, Loc,
			XAIE_PL_MOD, XAIE_EVENT_SWITCH_A,
			XAIE_ERROR_BROADCAST_ID_USER_EVENT1,
			BroadcastBlockDir);
	if (RC != XAIE_OK) {
		XAIE_ERROR("Failed to block broadcasts Loc: [%d, %d]: %d\n", Loc.Col, Loc.Row, RC);
		return RC;
	}
	RC = XAie_EventBroadcastBlockDir(DevInst, Loc,
			XAIE_PL_MOD, XAIE_EVENT_SWITCH_B,
			XAIE_ERROR_BROADCAST_ID_USER_EVENT1,
			XAIE_EVENT_BROADCAST_ALL);
	if (RC != XAIE_OK) {
		XAIE_ERROR("Failed to block broadcasts Loc: [%d, %d]: %d\n", Loc.Col, Loc.Row, RC);
		return RC;
	}

	return 0;
}

/*****************************************************************************/
/**
*
* This API configures broadcast network for SHIM tile for Non-lead, non 0 Col.
* Lead column for aie2ps is assumed to be Col 1.
*
* @param	DevInst: Device Instance
*
* @Loc		Location of SHIM tile.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		internal only
*
******************************************************************************/
static AieRC _XAie_ErrorHandlingInitAie2psShimTile(XAie_DevInst *DevInst, XAie_LocType Loc)
{
	u8 TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	u32 BroadcastBitMap, BroadcastBlockDir;
	AieRC RC;

	if ((TileType != XAIEGBL_TILE_TYPE_SHIMNOC) &&
	    (TileType != XAIEGBL_TILE_TYPE_SHIMPL)) {
		XAIE_ERROR("Not a shimtile\n");
		return XAIE_INVALID_TILE;
	}
	if (Loc.Col < 2) {
		XAIE_ERROR("Col should be > 1.\n");
		return XAIE_INVALID_TILE;
	}
	/* Block broadcast 0,1 in all directions
	 */
	BroadcastBitMap = BIT(XAIE_ERROR_BROADCAST_ID) |
			  BIT(XAIE_ERROR_BROADCAST_ID_UC_EVENT);
	BroadcastBlockDir = XAIE_EVENT_BROADCAST_NORTH |
			    XAIE_EVENT_BROADCAST_WEST |
			    XAIE_EVENT_BROADCAST_SOUTH;
	RC = XAie_EventBroadcastBlockMapDir(DevInst, Loc,
			XAIE_PL_MOD, XAIE_EVENT_SWITCH_A,
			BroadcastBitMap,
			BroadcastBlockDir);
	if (RC != XAIE_OK) {
		XAIE_ERROR("Failed to block broadcasts Loc: [%d, %d]: %d\n", Loc.Col, Loc.Row, RC);
		return RC;
	}
	RC = XAie_EventBroadcastBlockMapDir(DevInst, Loc,
			XAIE_PL_MOD, XAIE_EVENT_SWITCH_B,
			BroadcastBitMap,
			XAIE_EVENT_BROADCAST_ALL);
	if (RC != XAIE_OK) {
		XAIE_ERROR("Failed to block broadcasts Loc: [%d, %d]: %d\n", Loc.Col, Loc.Row, RC);
		return RC;
	}
	/* Propagate broadcast 2 west only
	 */
	BroadcastBlockDir = XAIE_EVENT_BROADCAST_NORTH |
			    XAIE_EVENT_BROADCAST_EAST |
			    XAIE_EVENT_BROADCAST_SOUTH;
	RC = XAie_EventBroadcastBlockDir(DevInst, Loc,
			XAIE_PL_MOD, XAIE_EVENT_SWITCH_A,
			XAIE_ERROR_BROADCAST_ID_USER_EVENT1,
			BroadcastBlockDir);
	if (RC != XAIE_OK) {
		XAIE_ERROR("Failed to block broadcasts Loc: [%d, %d]: %d\n", Loc.Col, Loc.Row, RC);
		return RC;
	}
	RC = XAie_EventBroadcastBlockDir(DevInst, Loc,
			XAIE_PL_MOD, XAIE_EVENT_SWITCH_B,
			XAIE_ERROR_BROADCAST_ID_USER_EVENT1,
			BroadcastBlockDir);
	if (RC != XAIE_OK) {
		XAIE_ERROR("Failed to block broadcasts Loc: [%d, %d]: %d\n", Loc.Col, Loc.Row, RC);
		return RC;
	}

	return 0;
}

/*****************************************************************************/
/**
*
* This API configures L1 ctrl for AIE2PS.
*
* @param	DevInst: Device Instance
*
* @Loc		Location of L1 SHIM tile.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		internal only
*
******************************************************************************/
static AieRC _XAie_ErrorHandlingInitAie2psL1Ctrl(XAie_DevInst *DevInst, XAie_LocType Loc)
{
	AieRC RC;
	u8 TileType;
	u8 L1IrqA, L1IrqB;
	u32 BroadcastBitMap;
	const XAie_L1IntrMod *L1IntrMod;

	/*
	 * Block direct broadcast from AIE array to the
	 * broadcast network in shim tiles.
	 */
	BroadcastBitMap = BIT(XAIE_ERROR_BROADCAST_ID) |
			  BIT(XAIE_ERROR_BROADCAST_ID_UC_EVENT) |
			  BIT(XAIE_ERROR_BROADCAST_ID_USER_EVENT1);
	RC = XAie_IntrCtrlL1BroadcastBlock(DevInst, Loc,
			XAIE_EVENT_SWITCH_A, BroadcastBitMap);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Failed to block direct broadcasts from AIE array\n");
		return RC;
	}
	RC = XAie_IntrCtrlL1BroadcastBlock(DevInst, Loc,
			XAIE_EVENT_SWITCH_B, BroadcastBitMap);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Failed to block direct broadcasts from AIE array\n");
		return RC;
	}

	if (Loc.Col == 1) {
		/* Enable l1 ctrl for broadcast 2
		 */
		RC = XAie_IntrCtrlL1Enable(DevInst, Loc, XAIE_EVENT_SWITCH_A,
			XAIE_ERROR_BROADCAST_ID_USER_EVENT1);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Failed to enable interrupts to L1\n");
			return RC;
		}
		RC = XAie_IntrCtrlL1Enable(DevInst, Loc, XAIE_EVENT_SWITCH_B,
			XAIE_ERROR_BROADCAST_ID_USER_EVENT1);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Failed to enable interrupts to L1\n");
			return RC;
		}
	} else {
		/* Enable l1 ctrl for broadcast 0 and 1
		 */
		RC = XAie_IntrCtrlL1Enable(DevInst, Loc, XAIE_EVENT_SWITCH_A,
			XAIE_ERROR_BROADCAST_ID);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Failed to enable interrupts to L1\n");
			return RC;
		}
		RC = XAie_IntrCtrlL1Enable(DevInst, Loc, XAIE_EVENT_SWITCH_B,
			XAIE_ERROR_BROADCAST_ID);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Failed to enable interrupts to L1\n");
			return RC;
		}
		RC = XAie_IntrCtrlL1Enable(DevInst, Loc, XAIE_EVENT_SWITCH_A,
			XAIE_ERROR_BROADCAST_ID_UC_EVENT);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Failed to enable interrupts to L1\n");
			return RC;
		}
		RC = XAie_IntrCtrlL1Enable(DevInst, Loc, XAIE_EVENT_SWITCH_B,
			XAIE_ERROR_BROADCAST_ID_UC_EVENT);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Failed to enable interrupts to L1\n");
			return RC;
		}

	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	L1IntrMod = DevInst->DevProp.DevMod[TileType].L1IntrMod;
	if (L1IntrMod == NULL) {
		XAIE_ERROR("Invalid module type\n");
		return XAIE_INVALID_ARGS;
	}

	L1IrqA = L1IntrMod->IntrCtrlL1IrqId(DevInst, Loc, XAIE_EVENT_SWITCH_A);
	RC = XAie_IntrCtrlL1IrqSet(DevInst, Loc, XAIE_EVENT_SWITCH_A, L1IrqA);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Failed to configure L1 IRQ line\n");
		return RC;
	}

	L1IrqB = L1IntrMod->IntrCtrlL1IrqId(DevInst, Loc, XAIE_EVENT_SWITCH_B);
	RC = XAie_IntrCtrlL1IrqSet(DevInst, Loc, XAIE_EVENT_SWITCH_B, L1IrqB);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Failed to configure L1 IRQ line\n");
		return RC;
	}


	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API configures broadcast network to deliver error events as interrupts in
* NPI. When error occurs, interrupt is raised on NPI interrupt line #5, 6, 7, 8.
*
* @param	DevInst: Device Instance
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		This API assumes the whole AIE2PS as a single partition and the
*		following broadcast channels to be available. To avoid conflicts,
*		it is the user's responsibility to make sure none of the below
*		channels are being used.
*			* Broadcast channel #0, 1, 2 in AIE array tiles.
*			* NPI interrupt line #5, 6, 7, 8.
*		Currently, this API only supports Linux UIO, CDO, and debug
*		backends.
*
*		This function is internal only
******************************************************************************/
static AieRC _XAie_ErrorHandlingInitAie2ps(XAie_DevInst *DevInst)
{
	AieRC RC;
	u8 TileType;
	XAie_LocType Loc;

	for (Loc.Col = 0; Loc.Col < DevInst->NumCols; Loc.Col++) {
		for (Loc.Row = 0; Loc.Row < DevInst->NumRows; Loc.Row++) {

			if (_XAie_PmIsTileRequested(DevInst, Loc) == XAIE_DISABLE) {
				continue;
			}

			TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);

			switch (TileType) {
			case XAIEGBL_TILE_TYPE_SHIMNOC:
			case XAIEGBL_TILE_TYPE_SHIMPL:
				RC = _XAie_ErrorHandlingInitAie2psL1Ctrl(DevInst, Loc);
				if (RC != XAIE_OK) {
					XAIE_ERROR("Failed to config L1 ctrl [%d, %d]: %d\n", Loc.Col, Loc.Row, RC);
					return RC;
				}
				RC = XAie_IntrCtrlL2Enable(DevInst, Loc,
						XAIE_ERROR_L2_ENABLE);
				if(RC != XAIE_OK) {
					XAIE_ERROR("Failed to enable interrupts to L2\n");
					return RC;
				}


				switch (Loc.Col) {
				case 1:
					RC = _XAie_ErrorHandlingInitAie2psShimTileLeadCol(DevInst, Loc);
					if (RC != XAIE_OK)
						return RC;
					break;
				case 0:
					RC = _XAie_ErrorHandlingInitAie2psShimTileCol0(DevInst, Loc);
					if (RC != XAIE_OK)
						return RC;

					break;
				default:
					RC = _XAie_ErrorHandlingInitAie2psShimTile(DevInst, Loc);
					if (RC != XAIE_OK)
						return RC;
					break;
				}
				break;
			case XAIEGBL_TILE_TYPE_AIETILE:
				RC = _XAie_ErrorHandlingInitAie2psAieTile(DevInst, Loc);
				if (RC != XAIE_OK) {
					return RC;
				}
				break;
			case XAIEGBL_TILE_TYPE_MEMTILE:
				RC = _XAie_ErrorHandlingInitAie2psMemTile(DevInst, Loc);
				if (RC != XAIE_OK)
					return RC;
				break;
			}
		}
	}

	return XAIE_OK;
}
/*****************************************************************************/
/**
*
* This API configures broadcast network to deliver error events as interrupts in
* NPI. When error occurs, interrupt is raised on NPI interrupt line #5. Also it
* configure error halt register, to put core in halt if any group error0 occurs
*
* @param	DevInst: Device Instance
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		This API assumes the whole AIE as a single partition and the
*		following broadcast channels to be available. To avoid conflicts,
*		it is the user's responsibility to make sure none of the below
*		channels are being used.
*			* Broadcast channel #0 in AIE array tiles.
*			* Switch A L1 IRQ 16.
*			* NPI interrupt line #5.
*		Currently, this API only supports Linux UIO, CDO, and debug
*		backends.
*
*		This function is internal only.
******************************************************************************/
static AieRC _XAie_ErrorHandlingInitAie(XAie_DevInst *DevInst)
{
	AieRC RC;
	u8 TileType, L1BroadcastIdSwA, L1BroadcastIdSwB, MemTileStart,
	   MemTileEnd, AieRowStart, AieRowEnd, BroadcastDirSwA, BroadcastDirSwB;
	XAie_LocType Loc;
	const XAie_L1IntrMod *L1IntrMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid device instance\n");
		return XAIE_INVALID_ARGS;
	}

	MemTileStart = DevInst->MemTileRowStart;
	MemTileEnd = DevInst->MemTileRowStart + DevInst->MemTileNumRows;
	AieRowStart = DevInst->AieTileRowStart;
	AieRowEnd = DevInst->AieTileRowStart + DevInst->AieTileNumRows;

	for(Loc.Col = 0; Loc.Col < DevInst->NumCols; Loc.Col++) {
		/* Setup error broadcasts to SOUTH from memory and core module */
		BroadcastDirSwA = (u8)XAIE_EVENT_BROADCAST_NORTH |
				  (u8)XAIE_EVENT_BROADCAST_EAST |
				  (u8)XAIE_EVENT_BROADCAST_WEST;
		BroadcastDirSwB = BroadcastDirSwA;

		for(Loc.Row = AieRowStart; Loc.Row < AieRowEnd; Loc.Row++) {
			if (_XAie_PmIsTileRequested(DevInst, Loc) == XAIE_DISABLE) {
				continue;
			}

			RC = XAie_EventBroadcastBlockDir(DevInst, Loc,
				   XAIE_CORE_MOD, XAIE_EVENT_SWITCH_A,
				   XAIE_ERROR_BROADCAST_ID, BroadcastDirSwA);
			if(RC != XAIE_OK) {
				XAIE_ERROR("Failed to block broadcasts in core module\n");
				return RC;
			}

			RC = XAie_EventBroadcastBlockDir(DevInst, Loc,
				   XAIE_MEM_MOD, XAIE_EVENT_SWITCH_A,
				   XAIE_ERROR_BROADCAST_ID, BroadcastDirSwB);
			if(RC != XAIE_OK) {
				XAIE_ERROR("Failed to block broadcasts in memory module\n");
				return RC;
			}
		}

		/* Setup error broadcasts to SOUTH from mem tile */
		for(Loc.Row = MemTileStart; Loc.Row < MemTileEnd; Loc.Row++) {
			if (_XAie_PmIsTileRequested(DevInst, Loc) == XAIE_DISABLE) {
				continue;
			}

			RC = XAie_EventBroadcastBlockDir(DevInst, Loc,
				   XAIE_MEM_MOD, XAIE_EVENT_SWITCH_A,
				   XAIE_ERROR_BROADCAST_ID, BroadcastDirSwA);
			if(RC != XAIE_OK) {
				XAIE_ERROR("Failed to block broadcasts in mem tile switch A\n");
				return RC;
			}

			RC = XAie_EventBroadcastBlockDir(DevInst, Loc,
				   XAIE_MEM_MOD, XAIE_EVENT_SWITCH_B,
				   XAIE_ERROR_BROADCAST_ID, BroadcastDirSwB);
			if(RC != XAIE_OK) {
				XAIE_ERROR("Failed to block broadcasts in mem tile switch B\n");
				return RC;
			}
		}

		/*
		 * Setup broadcast from array and PL module to the closest
		 * available L2 interrupt controller.
		 */
		Loc.Row = DevInst->ShimRow;

		/*
		 * Block direct broadcast from AIE array to the
		 * broadcast network in shim tiles.
		 */
		RC = XAie_IntrCtrlL1BroadcastBlock(DevInst, Loc,
				XAIE_EVENT_SWITCH_A, XAIE_ERROR_BROADCAST_MASK);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Failed to block direct broadcasts from AIE array\n");
			return RC;
		}

		RC = XAie_IntrCtrlL1BroadcastBlock(DevInst, Loc,
				XAIE_EVENT_SWITCH_B, XAIE_ERROR_BROADCAST_MASK);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Failed to block direct broadcasts from AIE array\n");
			return RC;
		}

		/*
		 * Enable broadcast network from AIE array to generate
		 * interrupts in L1 interrupt controller.
		 */
		RC = XAie_IntrCtrlL1Enable(DevInst, Loc, XAIE_EVENT_SWITCH_A,
				XAIE_ERROR_BROADCAST_ID);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Failed to enable interrupts to L1\n");
			return RC;
		}

		RC = XAie_IntrCtrlL1Enable(DevInst, Loc, XAIE_EVENT_SWITCH_B,
				XAIE_ERROR_BROADCAST_ID);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Failed to enable interrupts to L1\n");
			return RC;
		}

		/*
		 * Enable shim tile's internal error interrupts to L1
		 * interrupt controller in switch A.
		 */
		RC = XAie_IntrCtrlL1Enable(DevInst, Loc, XAIE_EVENT_SWITCH_A,
				XAIE_ERROR_SHIM_INTR_ID);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Failed to enable interrupts to L1\n");
			return RC;
		}

		/*
		 * Compute the broadcast line number on which L1 interrupt
		 * controller must generate error interrupts.
		 */
		TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
		L1IntrMod = DevInst->DevProp.DevMod[TileType].L1IntrMod;
		if (L1IntrMod == NULL) {
			XAIE_ERROR("Invalid module type\n");
			return XAIE_INVALID_ARGS;
		}

		L1BroadcastIdSwA = L1IntrMod->IntrCtrlL1IrqId(DevInst, Loc,
					XAIE_EVENT_SWITCH_A);

		RC = XAie_IntrCtrlL1IrqSet(DevInst, Loc, XAIE_EVENT_SWITCH_A,
				L1BroadcastIdSwA);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Failed to configure L1 IRQ line\n");
			return RC;
		}

		L1BroadcastIdSwB = L1IntrMod->IntrCtrlL1IrqId(DevInst, Loc,
					XAIE_EVENT_SWITCH_B);
		RC = XAie_IntrCtrlL1IrqSet(DevInst, Loc, XAIE_EVENT_SWITCH_B,
				L1BroadcastIdSwB);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Failed to configure L1 IRQ line\n");
			return RC;
		}

		/*
		 * Interrupts within the shim tile's broadcast network must be
		 * routed to the closest L2 interrupt controller. While doing
		 * so, such interrupts need to be blocked from broadcasting
		 * beyond the L2 interrupt controller tile.
		 */
		if(TileType == XAIEGBL_TILE_TYPE_SHIMNOC) {
			XAie_LocType NextLoc;

			BroadcastDirSwA = (u8)XAIE_EVENT_BROADCAST_NORTH |
					  (u8)XAIE_EVENT_BROADCAST_SOUTH |
					  (u8)XAIE_EVENT_BROADCAST_WEST;
			BroadcastDirSwB = (u8)XAIE_EVENT_BROADCAST_ALL;

			/*
			 * Create bitmask to block broadcast from previous
			 * columns based on the location NoC tile.
			 */
			RC = _XAie_FindNextNoCTile(DevInst, Loc, &NextLoc);
			if (RC != XAIE_OK) {
				L1BroadcastIdSwB = XAIE_ERROR_L2_ENABLE;
			} else {
				L1BroadcastIdSwB = (1U <<
						(L1BroadcastIdSwB + 1U)) - 1U;
			}

			RC = XAie_EventBroadcastBlockMapDir(DevInst, Loc,
					XAIE_PL_MOD, XAIE_EVENT_SWITCH_B,
					L1BroadcastIdSwB, BroadcastDirSwB);
			if(RC != XAIE_OK) {
				XAIE_ERROR("Failed to block broadcasts in shim tile switch B\n");
				return RC;
			}

			RC = XAie_IntrCtrlL2Enable(DevInst, Loc,
					XAIE_ERROR_L2_ENABLE);
			if(RC != XAIE_OK) {
				XAIE_ERROR("Failed to enable interrupts to L2\n");
				return RC;
			}
		} else {
			XAie_LocType NextLoc;

			RC = _XAie_FindNextNoCTile(DevInst, Loc, &NextLoc);
			if (RC != XAIE_OK) {
				BroadcastDirSwA = (u8)XAIE_EVENT_BROADCAST_NORTH |
						  (u8)XAIE_EVENT_BROADCAST_SOUTH |
						  (u8)XAIE_EVENT_BROADCAST_EAST;
				BroadcastDirSwB = BroadcastDirSwA;
			} else {
				BroadcastDirSwA = (u8)XAIE_EVENT_BROADCAST_NORTH |
						  (u8)XAIE_EVENT_BROADCAST_SOUTH |
						  (u8)XAIE_EVENT_BROADCAST_WEST;
				BroadcastDirSwB = BroadcastDirSwA;
			}

			RC = XAie_EventBroadcastBlockDir(DevInst, Loc,
					XAIE_PL_MOD, XAIE_EVENT_SWITCH_B,
					L1BroadcastIdSwB, BroadcastDirSwB);
			if(RC != XAIE_OK) {
				XAIE_ERROR("Failed to block broadcasts in shim tile switch B\n");
				return RC;
			}
		}

		RC = XAie_EventBroadcastBlockDir(DevInst, Loc, XAIE_PL_MOD,
				XAIE_EVENT_SWITCH_A, L1BroadcastIdSwA,
				BroadcastDirSwA);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Failed to block broadcasts in shim tile switch A\n");
			return RC;
		}
	}

	return XAIE_OK;
}
/*****************************************************************************/
/**
*
* This API configures broadcast network to deliver error events as interrupts in
* NPI. When error occurs, interrupt is raised on NPI interrupt line #5-7. Also it
* configure error halt register, to put core in halt if any group error0 occurs
*
* @param	DevInst: Device Instance
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		This API assumes the whole AIE as a single partition and the
*		following broadcast channels to be available. To avoid conflicts,
*		it is the user's responsibility to make sure none of the below
*		channels are being used.
*			* Broadcast channel #0 in AIE1&2 array tiles.
*			* Broadcast channel #0,1,2 in AIE2PS.
*			* Switch A L1 IRQ 16. For AIE1&2
*			* NPI interrupt line #5 for AIE1 and 2
*			* NPI interrupt line #5, 6, 7, 8. for AIE2PS.
*		Currently, this API only supports Linux UIO, CDO, and debug
*		backends.
******************************************************************************/
AieRC XAie_ErrorHandlingInit(XAie_DevInst *DevInst)
{
	AieRC RC;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid device instance\n");
		return XAIE_INVALID_ARGS;
	}

	switch (DevInst->DevProp.DevGen) {
	case XAIE_DEV_GEN_AIE2PS:
		RC = _XAie_ErrorHandlingInitAie2ps(DevInst);
		break;
	default:
		RC = _XAie_ErrorHandlingInitAie(DevInst);
		break;
	}
	if (RC != XAIE_OK) {
		XAIE_ERROR("Failed to set Broadcast network: %d\n", RC);
		return RC;
	}

	RC =  _XAie_GroupErrorInit(DevInst);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Failed to initialize group errors\n");
		return RC;
	}

	RC = _XAie_ErrorHandlingEventHaltCore(DevInst);
	if(RC != XAIE_OK) {
		XAIE_ERROR("Failed to initialize Error Halt Event Register\n");
		return RC;
	}

	return XAIE_OK;
}
#endif /* XAIE_FEATURE_INTR_INIT_ENABLE */
/** @} */
