/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_trace.c
* @{
*
* This file contains routines for AIE trace module.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Nishad   06/16/2020  Initial creation
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xaie_helper.h"
#include "xaie_trace.h"

/************************** Constant Definitions *****************************/
/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This API is used to configure trace event register with event number at the
* given slot index.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	Module: Module of tile.
*			For AIE Tile - XAIE_MEM_MOD or XAIE_CORE_MOD,
*			For Shim tile - XAIE_PL_MOD,
*			For Mem tile - XAIE_MEM_MOD.
* @param	Event: Event to be traced
* @param	SlotId: Trace slot index for corresponding event.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_TraceEvent(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, XAie_Events Event, u8 SlotId)
{
	AieRC RC;
	u64 RegAddr;
	u32 RegOffset, FldVal, FldMask;
	u8 TileType, MappedEvent, EventRegOffId;
	const XAie_TraceMod *TraceMod;
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

	if(Module == XAIE_PL_MOD) {
		TraceMod = &DevInst->DevProp.DevMod[TileType].TraceMod[0U];
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[0U];
	} else {
		TraceMod = &DevInst->DevProp.DevMod[TileType].TraceMod[Module];
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[Module];
	}

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

	if(SlotId >= TraceMod->NumTraceSlotIds) {
		XAieLib_print("Error: Invalid trace slot index\n");
		return XAIE_INVALID_ARGS;
	}

	EventRegOffId = SlotId / TraceMod->NumEventsPerSlot;
	RegOffset = TraceMod->EventRegOffs[EventRegOffId];
	FldMask = TraceMod->Event[SlotId].Mask;
	FldVal = XAie_SetField(MappedEvent, TraceMod->Event[SlotId].Lsb, FldMask);
	RegAddr = DevInst->BaseAddr +
			_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) + RegOffset;

	XAieGbl_MaskWrite32(RegAddr, FldMask, FldVal);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API is used to configure trace control register with event number to
* start tracing.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	Module: Module of tile.
*			For AIE Tile - XAIE_MEM_MOD or XAIE_CORE_MOD,
*			For Shim tile - XAIE_PL_MOD,
*			For Mem tile - XAIE_MEM_MOD.
* @param	StartEvent: Event number to start tracing.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_TraceStartEvent(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, XAie_Events StartEvent)
{
	AieRC RC;
	u64 RegAddr;
	u32 RegOffset, FldVal, FldMask;
	u8 TileType, MappedEvent;
	const XAie_TraceMod *TraceMod;
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

	if(Module == XAIE_PL_MOD) {
		TraceMod = &DevInst->DevProp.DevMod[TileType].TraceMod[0U];
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[0U];
	} else {
		TraceMod = &DevInst->DevProp.DevMod[TileType].TraceMod[Module];
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[Module];
	}

	if(StartEvent < EvntMod->EventMin || StartEvent > EvntMod->EventMax) {
		XAieLib_print("Error: Invalid event ID\n");
		return XAIE_INVALID_ARGS;
	}

	StartEvent -= EvntMod->EventMin;

	MappedEvent = EvntMod->XAie_EventNumber[StartEvent];
	if(MappedEvent == XAIE_EVENT_INVALID) {
		XAieLib_print("Error: Invalid event ID\n");
		return XAIE_INVALID_ARGS;
	}

	RegOffset = TraceMod->CtrlRegOff;
	FldMask = TraceMod->StartEvent.Mask;
	FldVal = XAie_SetField(MappedEvent, TraceMod->StartEvent.Lsb, FldMask);
	RegAddr = DevInst->BaseAddr +
			_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) + RegOffset;

	XAieGbl_MaskWrite32(RegAddr, FldMask, FldVal);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API is used to configure trace control register with event number to stop
* tracing.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	Module: Module of tile.
*			For AIE Tile - XAIE_MEM_MOD or XAIE_CORE_MOD,
*			For Shim tile - XAIE_PL_MOD,
*			For Mem tile - XAIE_MEM_MOD.
* @param	StopEvent: Event number to stop tracing.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_TraceStopEvent(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, XAie_Events StopEvent)
{
	AieRC RC;
	u64 RegAddr;
	u32 RegOffset, FldVal, FldMask;
	u8 TileType, MappedEvent;
	const XAie_TraceMod *TraceMod;
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

	if(Module == XAIE_PL_MOD) {
		TraceMod = &DevInst->DevProp.DevMod[TileType].TraceMod[0U];
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[0U];
	} else {
		TraceMod = &DevInst->DevProp.DevMod[TileType].TraceMod[Module];
		EvntMod = &DevInst->DevProp.DevMod[TileType].EvntMod[Module];
	}

	if(StopEvent < EvntMod->EventMin || StopEvent > EvntMod->EventMax) {
		XAieLib_print("Error: Invalid event ID\n");
		return XAIE_INVALID_ARGS;
	}

	StopEvent -= EvntMod->EventMin;

	MappedEvent = EvntMod->XAie_EventNumber[StopEvent];
	if(MappedEvent == XAIE_EVENT_INVALID) {
		XAieLib_print("Error: Invalid event ID\n");
		return XAIE_INVALID_ARGS;
	}

	RegOffset = TraceMod->CtrlRegOff;
	FldMask = TraceMod->StopEvent.Mask;
	FldVal = XAie_SetField(MappedEvent, TraceMod->StopEvent.Lsb, FldMask);
	RegAddr = DevInst->BaseAddr +
			_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) + RegOffset;

	XAieGbl_MaskWrite32(RegAddr, FldMask, FldVal);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API is used to configure trace packet register with packet ID and packet
* type defined as part of XAie_Packet structure.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	Module: Module of tile.
*			For AIE Tile - XAIE_MEM_MOD or XAIE_CORE_MOD,
*			For Shim tile - XAIE_PL_MOD,
*			For Mem tile - XAIE_MEM_MOD.
* @param	Pkt: Packet with initialized packet id and packet type.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_TracePktConfig(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, XAie_Packet Pkt)
{
	AieRC RC;
	u64 RegAddr;
	u32 RegOffset, FldVal, FldMask;
	u8 TileType;
	const XAie_TraceMod *TraceMod;

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
		TraceMod = &DevInst->DevProp.DevMod[TileType].TraceMod[0U];
	else
		TraceMod = &DevInst->DevProp.DevMod[TileType].TraceMod[Module];

	if(Pkt.PktId > XAIE_PACKET_ID_MAX || Pkt.PktId > XAIE_PACKET_TYPE_MAX) {
		XAieLib_print("Error: Invalid packet type or ID\n");
		return XAIE_INVALID_ARGS;
	}

	RegOffset = TraceMod->PktConfigRegOff;
	FldMask = TraceMod->PktId.Mask | TraceMod->PktType.Mask;
	FldVal = XAie_SetField(Pkt.PktId, TraceMod->PktId.Lsb,
				TraceMod->PktId.Mask) |
		 XAie_SetField(Pkt.PktType, TraceMod->PktType.Lsb,
				 TraceMod->PktType.Mask);
	RegAddr = DevInst->BaseAddr +
			_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) + RegOffset;

	XAieGbl_MaskWrite32(RegAddr, FldMask, FldVal);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API is used to configure trace event register with event number at a
* particular slot index.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	Module: Module of tile.
*			For AIE Tile - XAIE_MEM_MOD or XAIE_CORE_MOD,
*			For Shim tile - XAIE_PL_MOD,
*			For Mem tile - XAIE_MEM_MOD.
* @param	Mode: Trace module can operate in the following modes,
*			EVENT_TIME: Valid for all module types
*			EVENT_PC, INST_EXEC: Only valid for core module.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_TraceModeConfig(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, XAie_TraceMode Mode)
{
	AieRC RC;
	u64 RegAddr;
	u32 RegOffset, FldVal, FldMask;
	u8 TileType;
	const XAie_TraceMod *TraceMod;

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
		TraceMod = &DevInst->DevProp.DevMod[TileType].TraceMod[0U];
	else
		TraceMod = &DevInst->DevProp.DevMod[TileType].TraceMod[Module];

	if(Mode > XAIE_TRACE_INST_EXEC ||
			TraceMod->ModeConfig.Mask == XAIE_FEATURE_UNAVAILABLE) {
		XAieLib_print("Error: Invalid trace mode\n");
		return XAIE_INVALID_ARGS;
	}

	RegOffset = TraceMod->CtrlRegOff;
	FldMask = TraceMod->ModeConfig.Mask;
	FldVal = XAie_SetField(Mode, TraceMod->ModeConfig.Lsb, FldMask);
	RegAddr = DevInst->BaseAddr +
			_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) + RegOffset;

	XAieGbl_MaskWrite32(RegAddr, FldMask, FldVal);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API is used to read the current state of trace module.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	Module: Module of tile.
*			For AIE Tile - XAIE_MEM_MOD or XAIE_CORE_MOD,
*			For Shim tile - XAIE_PL_MOD,
*			For Mem tile - XAIE_MEM_MOD.
* @param	State: Pointer to return the state trace module. Valid trace
* 		       are, IDLE, RUNNING and OVERFLOW.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_TraceGetState(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, XAie_TraceState *State)
{
	AieRC RC;
	u64 RegAddr;
	u32 RegOffset, RegValue;
	u8 TileType;
	const XAie_TraceMod *TraceMod;

	if((DevInst == XAIE_NULL) || (State == NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAieLib_print("Error: Invalid device instance or XAie_TraceState pointer\n");
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
		TraceMod = &DevInst->DevProp.DevMod[TileType].TraceMod[0U];
	else
		TraceMod = &DevInst->DevProp.DevMod[TileType].TraceMod[Module];

	RegOffset = TraceMod->StatusRegOff;
	RegAddr = DevInst->BaseAddr +
			_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) + RegOffset;
	RegValue = XAieGbl_Read32(RegAddr);

	*State = XAie_GetField(RegValue, TraceMod->State.Lsb,
			TraceMod->State.Mask);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API is used to read current operational mode of trace module.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of AIE Tile
* @param	Module: Module of tile.
*			For AIE Tile - XAIE_MEM_MOD or XAIE_CORE_MOD,
*			For Shim tile - XAIE_PL_MOD,
*			For Mem tile - XAIE_MEM_MOD.
* @param	Mode: Pointer to return the current operational mode of trace
* 		      module. Valid modes are,
*			EVENT_TIME: Valid for all module types
*			EVENT_PC, INST_EXEC: Only valid for core module.
*
* @return	XAIE_OK on success, error code on failure.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_TraceGetMode(XAie_DevInst *DevInst, XAie_LocType Loc,
		XAie_ModuleType Module, XAie_TraceMode *Mode)
{
	AieRC RC;
	u64 RegAddr;
	u32 RegOffset, RegValue;
	u8 TileType;
	const XAie_TraceMod *TraceMod;

	if((DevInst == XAIE_NULL) || (Mode == NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAieLib_print("Error: Invalid device instance or XAie_TraceMode pointer\n");
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
		TraceMod = &DevInst->DevProp.DevMod[TileType].TraceMod[0U];
	else
		TraceMod = &DevInst->DevProp.DevMod[TileType].TraceMod[Module];

	RegOffset = TraceMod->StatusRegOff;
	RegAddr = DevInst->BaseAddr +
			_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) + RegOffset;
	RegValue = XAieGbl_Read32(RegAddr);

	*Mode = XAie_GetField(RegValue, TraceMod->ModeSts.Lsb,
			TraceMod->ModeSts.Mask);

	return XAIE_OK;
}

/** @} */
