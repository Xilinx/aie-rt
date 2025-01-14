/******************************************************************************
* Copyright (C) 2024 AMD, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_routing.h
* @{
*
* This file contains the high level APIs for AI-Engines.
*
******************************************************************************/
#ifndef XAIE_ROUTING_H
#define XAIE_ROUTING_H

/***************************** Include Files *********************************/
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "xaiegbl.h"
#include "xaie_io.h"
#include "xaie_feature_config.h"

/************************** Constant Definitions *****************************/
typedef struct {
    u16 sourceBD;
    u16 destinationBD;
} XAie_BDs;

typedef enum { XAIE_AIE_SHIM = 0, XAIE_AIE_MEM, XAIE_AIE_CORE } TileType;

typedef struct {
    u8 canMoveNorth : 1;
    u8 canMoveSouth : 1;
    u8 canMoveEast : 1;
    u8 canMoveWest : 1;
} XAie_StreamDirSupported;

typedef struct XAie_ChannelPortMapping {
    u8 channel; //available channels
    u8 port; //available ports
    bool availability; //is this channel used?
}XAie_ChannelPortMapping;

typedef struct XAie_RoutingResourceConstraint {
    u8 column; //specific column
    XAie_ChannelPortMapping* channelPortMappings;
    u8 channelPortMappingCount;
    bool HostToAie; //Host-to-aie or aie-to-host
} XAie_RoutingResourceConstraint;

typedef struct XAie_HostAieConstraint {
    XAie_RoutingResourceConstraint* RoutingResourceConstraint;
    u8 ConstraintCount;
} XAie_HostAieConstraint;

typedef struct XAie_RoutingStep {
    XAie_LocType sourceTile;
    int sourceStream;
    int destStream;
    StrmSwPortType source_direction;
    StrmSwPortType dest_direction;
    struct XAie_RoutingStep *next; // Pointer to the next step in the path
} XAie_RoutingStep;

typedef struct {
    XAie_LocType source;
    XAie_LocType destination;
    u8 MM2S_portNo;
    u8 S2MM_portNo;
    XAie_RoutingStep *nextStep; // Pointer to the first step in the path
} XAie_RoutingPath;

typedef struct XAie_ProgrammedRoutes {
    XAie_RoutingPath* routePath; // This can be changed to any type of data or a more complex structure
    struct XAie_ProgrammedRoutes* nextRoute;
} XAie_ProgrammedRoutes;

typedef struct {
    u8* S2MM_ports;
    u8 S2MM_portCount;
} XAie_s2mmChannelsInUse;

typedef struct {
    u8* MM2S_ports;
    u8 MM2S_portCount;
} XAie_mm2sChannelsInUse;

typedef struct {
    bool isAutoConfigured; //this tile is part of route auto configured by XAie_Route
    u8 MM2S_State; // each bit represents a channel state
    u8 S2MM_State; // each bit represents a channel state
    u8 ShimMM2S_State;
    u8 ShimS2MM_State;
    //DMA is composed of 4 independent channels, two MM2S and two S2MM for AIE tile , 6-S2MM and 6-MM2S for AIE-Mem tile
    bool AllChannelsInUse;
    u64 BDState; // each bit represents a BD state [totally 16 BDs available for aie-tile/ 48 for MemTile]
    bool AllBDsareInUse;
    TileType tile_type;
    XAie_StreamDirSupported DirSupported;
    u8 SlaveEast;
    u8 SlaveWest;
    u8 SlaveSouth;
    u8 SlaveNorth;
    u8 MasterEast;
    u8 MasterWest;
    u8 MasterSouth;
    u8 MasterNorth;
    XAie_s2mmChannelsInUse s2mmChannelsInUse;
    XAie_mm2sChannelsInUse mm2sChannelsInUse;
    XAie_ChannelPortMapping* Host2AIEPorts; //SHIM Tile's ports that are available for use to move data from Host to AIE Tile
    u8 Host2AIEPortCount;
    XAie_ChannelPortMapping* AIE2HostPorts; //SHIM Tile's ports that are available for use to move data from Host to AIE Tile
    u8 AIE2HostPortCount;
    XAie_ProgrammedRoutes *routesDB; // Pointer to the routing path data structure
    // More constraints can be added per core
    bool isCoreExecuting; // if this is true, this means this core
} XAie_CoreConstraint;


typedef struct {
    XAie_LocType* BlackListedCores;
    u16 NoOfBlackListedCores;
    XAie_LocType* WhiteListedCores;
    u16 NoOfWhiteListedCores;
}  XAie_RouteConstraints;

typedef struct XAie_RoutingInstance {
    XAie_DevInst* DeviceInstance;
    XAie_CoreConstraint*** CoreConstraintPerCore;//each core's constraint  [col][row]
    u8 NumRows;
    u8 NumCols;
} XAie_RoutingInstance;

typedef struct {
    int front, rear, size;
    unsigned capacity;
    XAie_LocType* array;
} Queue;

#ifdef XAIE_FEATURE_ROUTING_ENABLE
AieRC XAie_dumpConstraintsToPrint(XAie_RoutingInstance* RoutingInstance);
AieRC XAie_dumpSpecificConstraintToPrint(XAie_RoutingInstance* RoutingInstance, u8 row, u8 col);
AieRC XAie_coreConstraintToPrint(XAie_RoutingInstance* RoutingInstance, XAie_CoreConstraint* constraint, u8 row, u8 col) ;
AieRC XAie_MoveData(XAie_RoutingInstance *routingInstance, XAie_LocType source, void* SourceObject, u32 data_size, void* DestinationObject,
              XAie_LocType destination);

AieRC XAie_Route(XAie_RoutingInstance *routingInstance,  XAie_RouteConstraints* RouteConstraints, XAie_LocType source, XAie_LocType destination);
AieRC XAie_DeRoute(XAie_RoutingInstance *routingInstance, XAie_LocType source, XAie_LocType destination, bool ShouldModifyCoreConfig);
AieRC XAie_RoutesReveal(XAie_RoutingInstance *routingInstance, XAie_LocType source, XAie_LocType destination);
AieRC XAie_RouteDmaWait(XAie_RoutingInstance *routingInstance, XAie_LocType sourcetile, XAie_LocType tile, bool isS2MM);
AieRC XAie_CoreWait(XAie_RoutingInstance *routingInstance, XAie_LocType tile);
AieRC XAie_Run(XAie_RoutingInstance* routingInstance, uint32_t count);
AieRC XAie_SetCoreExecute(XAie_RoutingInstance *routingInstance, XAie_LocType tile, bool isExecute);
void XAieRoutingInstance_free(XAie_RoutingInstance* RoutingInstance);
AieRC XAie_ConfigHostEdgeConstraints(XAie_RoutingInstance *routingInstance, XAie_HostAieConstraint UserResCon);
AieRC XAie_ResetHostEdgeConstraints(XAie_RoutingInstance *routingInstance);
XAie_RoutingInstance* XAie_InitRoutingHandler(XAie_DevInst *DevInst);
AieRC XAie_RoutingSwitchReset(XAie_RoutingInstance *routingInstance, XAie_LocType* listOfTiles, uint32_t NoOfTiles);
void XAie_DumpRoutingSwitchInfo(XAie_RoutingInstance *routingInstance, XAie_LocType* listOfTiles, uint32_t NoOfTiles);

#else /* XAIE_FEATURE_ROUTING_ENABLE */

static inline AieRC XAie_dumpConstraintsToPrint(XAie_RoutingInstance* RoutingInstance)
{
	return XAIE_ERR;
}

static inline AieRC XAie_dumpSpecificConstraintToPrint(XAie_RoutingInstance* RoutingInstance, u8 row, u8 col)
{
	return XAIE_ERR;
}

static inline AieRC XAie_coreConstraintToPrint(XAie_RoutingInstance* RoutingInstance, XAie_CoreConstraint* constraint, u8 row, u8 col)
{
	return XAIE_ERR;
}

static inline AieRC XAie_MoveData(XAie_RoutingInstance *routingInstance, XAie_LocType source,
				  void* SourceObject, u32 data_size, void* DestinationObject,
				  XAie_LocType destination)
{
	return XAIE_ERR;
}

static inline AieRC XAie_Route(XAie_RoutingInstance *routingInstance,  XAie_RouteConstraints* RouteConstraints,
			       XAie_LocType source, XAie_LocType destination)
{
	return XAIE_ERR;
}

static inline AieRC XAie_DeRoute(XAie_RoutingInstance *routingInstance, XAie_LocType source,
				 XAie_LocType destination, bool ShouldModifyCoreConfig)
{
	return XAIE_ERR;
}

static inline AieRC XAie_RoutesReveal(XAie_RoutingInstance *routingInstance, XAie_LocType source,
				      XAie_LocType destination)
{
	return XAIE_ERR;
}

static inline AieRC XAie_RouteDmaWait(XAie_RoutingInstance *routingInstance, XAie_LocType tile, bool isS2MM)
{
	return XAIE_ERR;
}

static inline AieRC XAie_CoreWait(XAie_RoutingInstance *routingInstance, XAie_LocType tile)
{
	return XAIE_ERR;
}

static inline AieRC XAie_Run(XAie_RoutingInstance* routingInstance, uint32_t count)
{
	return XAIE_ERR;
}

static inline AieRC XAie_SetCoreExecute(XAie_RoutingInstance *routingInstance, XAie_LocType tile, bool isExecute)
{
	return XAIE_ERR;
}

static inline void XAieRoutingInstance_free(XAie_RoutingInstance* RoutingInstance)
{
	return;
}

static inline AieRC XAie_ConfigHostEdgeConstraints(XAie_RoutingInstance *routingInstance, XAie_HostAieConstraint UserResCon)
{
	return XAIE_ERR;
}

static inline AieRC XAie_ResetHostEdgeConstraints(XAie_RoutingInstance *routingInstance)
{
	return XAIE_ERR;
}

static inline XAie_RoutingInstance* XAie_InitRoutingHandler(XAie_DevInst *DevInst)
{
	return NULL;
}

static inline AieRC XAie_RoutingSwitchReset(XAie_RoutingInstance *routingInstance, XAie_LocType* listOfTiles, uint32_t NoOfTiles)
{
	return XAIE_ERR;
}

static inline void XAie_DumpRoutingSwitchInfo(XAie_RoutingInstance *routingInstance, XAie_LocType* listOfTiles, uint32_t NoOfTiles)
{
	return;
}

#endif /* XAIE_FEATURE_ROUTING_ENABLE */

#endif	/* End of protection macro */

/** @} */
