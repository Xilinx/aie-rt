/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_events.c
* @{
*
* This file contains routines for AIE events module.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Nishad  07/01/2020  Initial creation
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xaie_events.h"
#include "xaie_helper.h"

/************************** Constant Definitions *****************************/
/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This API is used to trigger an event the given module
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	Module: Module of tile.
*			for AIE Tile - XAIE_MEM_MOD or XAIE_CORE_MOD,
*			for Shim tile - XAIE_PL_MOD,
*			for Mem tile - XAIE_MEM_MOD.
* @param	Event: Event to be triggered
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_EventGenerate(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, XAie_Events Event)
{
	AieRC RC;
	u64 RegAddr;
	u32 RegOffset, FldVal, FldMask;
	u8 TileType, MappedEvent;
	const XAie_EvntMod *EvntMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAieLib_print("Error: Invalid device instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAieLib_print("Error: Invalid tile type\n");
		return XAIE_INVALID_TILE;
	}

	RC = _XAie_CheckModule(DevInst, Loc, Module);
	if(RC != XAIE_OK) {
		XAieLib_print("Error: Invalid module\n");
		return XAIE_INVALID_ARGS;
	}

	if(Module == XAIE_PL_MOD)
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[0U];
	else
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[Module];

	if(Event < EvntMod->EventMin || Event > EvntMod->EventMax) {
		XAieLib_print("Error: Invalid event ID\n");
		return XAIE_INVALID_ARGS;
	}

	Event -= EvntMod->EventMin;
	MappedEvent = EvntMod->XAie_EventNumber[Event];
	if(MappedEvent == XAIE_EVENT_INVALID) {
		XAieLib_print("Error: Invalid event ID\n");
		return XAIE_INVALID_ARGS;
	}

	RegOffset = EvntMod->GenEventRegOff;
	FldMask = EvntMod->GenEvent.Mask;
	FldVal = XAie_SetField(MappedEvent, EvntMod->GenEvent.Lsb, FldMask);
	RegAddr = _XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) + RegOffset;

	XAie_MaskWrite32(DevInst, RegAddr, FldMask, FldVal);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API is used to configure combo events for a given module.
*
* @param	DevInst: Device Instance.
* @param	Loc: Location of AIE Tile.
* @param	Module: Module of tile.
*			for AIE Tile - XAIE_MEM_MOD or XAIE_CORE_MOD,
*			for Shim tile - XAIE_PL_MOD,
*			for Mem tile - XAIE_MEM_MOD.
* @param	ComboId: Combo index.
* @param	Op: Logical operation between Event1 and Event2 to trigger combo
*		    event.
* @param	Event1: When, ComboId == XAIE_EVENT_COMBO0 Event1 coressponds to
*			Event A, ComboId == XAIE_EVENT_COMBO1 Event1 coressponds
*			to Event C, ComboId == XAIE_EVENT_COMBO2 Event1
*			coressponds to XAIE_EVENT_COMBO0.
* @param	Event2: When, ComboId == XAIE_EVENT_COMBO0 Event2 coressponds to
*			Event B, ComboId == XAIE_EVENT_COMBO1 Event2 coressponds
*			to Event D, ComboId == XAIE_EVENT_COMBO2 Event2
*			coressponds to XAIE_EVENT_COMBO1.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_EventComboConfig(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, XAie_EventComboId ComboId,
		XAie_EventComboOps Op, XAie_Events Event1, XAie_Events Event2)
{
	AieRC RC;
	u64 RegAddr;
	u32 RegOffset, FldVal, FldMask, Event1Mask, Event2Mask;
	u8 TileType, Event1Lsb, Event2Lsb, MappedEvent1, MappedEvent2;
	const XAie_EvntMod *EvntMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAieLib_print("Error: Invalid device instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAieLib_print("Error: Invalid tile type\n");
		return XAIE_INVALID_TILE;
	}

	RC = _XAie_CheckModule(DevInst, Loc, Module);
	if(RC != XAIE_OK) {
		XAieLib_print("Error: Invalid module\n");
		return XAIE_INVALID_ARGS;
	}

	if(Module == XAIE_PL_MOD)
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[0U];
	else
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[Module];

	RegOffset = EvntMod->ComboCtrlRegOff;
	FldMask = EvntMod->ComboConfigMask << (ComboId * EvntMod->ComboConfigOff);
	FldVal = XAie_SetField(Op, ComboId * EvntMod->ComboConfigOff, FldMask);
	RegAddr = _XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) + RegOffset;

	XAie_MaskWrite32(DevInst, RegAddr, FldMask, FldVal);

	/* Skip combo input event register config for XAIE_COMBO2 combo ID */
	if(ComboId == XAIE_EVENT_COMBO2)
		return XAIE_OK;

	if(Event1 < EvntMod->EventMin || Event1 > EvntMod->EventMax ||
		Event2 < EvntMod->EventMin || Event2 > EvntMod->EventMax)
	{
		XAieLib_print("Error: Invalid event ID\n");
		return XAIE_INVALID_ARGS;
	}

	Event1 -= EvntMod->EventMin;
	Event2 -= EvntMod->EventMin;
	MappedEvent1 = EvntMod->XAie_EventNumber[Event1];
	MappedEvent2 = EvntMod->XAie_EventNumber[Event2];
	if(MappedEvent1 == XAIE_EVENT_INVALID ||
			MappedEvent2 == XAIE_EVENT_INVALID)
	{
		XAieLib_print("Error: Invalid event ID\n");
		return XAIE_INVALID_ARGS;
	}

	RegOffset = EvntMod->ComboInputRegOff;
	Event1Lsb = ComboId * EvntMod->ComboEventOff;
	Event2Lsb = (ComboId + 1) * EvntMod->ComboEventOff;
	Event1Mask = EvntMod->ComboEventMask << Event1Lsb;
	Event2Mask = EvntMod->ComboEventMask << Event2Lsb;
	FldVal = XAie_SetField(MappedEvent1, Event1Lsb, Event1Mask) |
		 XAie_SetField(MappedEvent2, Event2Lsb, Event2Mask);
	RegAddr = _XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) + RegOffset;

	XAie_MaskWrite32(DevInst, RegAddr, Event1Mask | Event2Mask, FldVal);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API is used to configure the stream switch event selection register for
* any given tile type. Any of the Master or Slave stream switch ports can be
* programmed at given selection index. Events corresponding to the port could be
* monitored at the given selection ID through event status registers.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	Module: Module of tile.
*			for AIE Tile - XAIE_MEM_MOD or XAIE_CORE_MOD,
*			for Shim tile - XAIE_PL_MOD,
*			for Mem tile - XAIE_MEM_MOD.
* @param	SelectId: Selection index at which given port's event are
*			  captured
* @param	PortIntf: Stream switch port interface.
*			  for Slave port - XAIE_STRMSW_SLAVE,
*			  for Mater port - XAIE_STRMSW_MASTER.
* @param	Port: Stream switch port type.
* @param	PortNum: Stream switch port number.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_EventSelectStrmPort(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, u8 SelectId, XAie_StrmPortIntf PortIntf,
		StrmSwPortType Port, u8 PortNum)
{
	AieRC RC;
	u64 RegAddr;
	u32 RegOffset, FldVal, PortIdMask, PortMstrSlvMask;
	u8 TileType, SelectRegOffId, PortIdx, PortIdLsb, PortMstrSlvLsb;
	const XAie_StrmMod *StrmMod;
	const XAie_EvntMod *EvntMod;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAieLib_print("Error: Invalid device instance\n");
		return XAIE_INVALID_ARGS;
	}

	if(PortIntf > XAIE_STRMSW_MASTER) {
		XAieLib_print("Error: Invalid stream switch interface\n");
		return XAIE_INVALID_ARGS;
	}

	if(Port >= SS_PORT_TYPE_MAX) {
		XAieLib_print("Error: Invalid stream switch ports\n");
		return XAIE_ERR_STREAM_PORT;
	}

	TileType = _XAie_GetTileTypefromLoc(DevInst, Loc);
	if(TileType == XAIEGBL_TILE_TYPE_MAX) {
		XAieLib_print("Error: Invalid tile type\n");
		return XAIE_INVALID_TILE;
	}

	RC = _XAie_CheckModule(DevInst, Loc, Module);
	if(RC != XAIE_OK) {
		XAieLib_print("Error: Invalid module\n");
		return XAIE_INVALID_ARGS;
	}

	if(Module == XAIE_PL_MOD)
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[0U];
	else
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[Module];

	if(SelectId >= EvntMod->NumStrmPortSelectIds) {
		XAieLib_print("Error: Invalid selection ID\n");
		return XAIE_INVALID_ARGS;
	}

	/* Get stream switch module pointer from device instance */
	StrmMod = DevInst->DevProp.DevMod[TileType].StrmSw;

	if(PortIntf == XAIE_STRMSW_SLAVE)
		RC = _XAie_GetSlaveIdx(StrmMod, Port, PortNum, &PortIdx);
	else
		RC = _XAie_GetMstrIdx(StrmMod, Port, PortNum, &PortIdx);
	if(RC != XAIE_OK) {
		XAieLib_print("Error: Unable to compute port index\n");
		return RC;
	}

	SelectRegOffId = SelectId / EvntMod->StrmPortSelectIdsPerReg;
	RegOffset = EvntMod->BaseStrmPortSelectRegOff + SelectRegOffId * 4U;
	PortIdLsb = SelectId * EvntMod->PortIdOff;
	PortIdMask = EvntMod->PortIdMask << PortIdLsb;
	PortMstrSlvLsb = SelectId * EvntMod->PortMstrSlvOff;
	PortMstrSlvMask = EvntMod->PortMstrSlvMask << PortMstrSlvLsb;
	FldVal = XAie_SetField(PortIdx, PortIdLsb, PortIdMask) |
		 XAie_SetField(PortIntf, PortMstrSlvLsb, PortMstrSlvMask);
	RegAddr = _XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) + RegOffset;

	XAie_MaskWrite32(DevInst, RegAddr, PortIdMask | PortMstrSlvMask, FldVal);

	return XAIE_OK;
}

/** @} */
