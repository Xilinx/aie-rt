/******************************************************************************
 * Copyright (c) 2024 AMD, Inc.  All rights reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/


/*****************************************************************************/
/**
 * @file xaie_routing.c
 * @{
 *
 * This file contains the high level APIs for AI-Engines.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who     Date     Changes
 * ----- ------  -------- -----------------------------------------------------
 * 1.0   Arunkumar    12/1/2023  Initial creation
 * </pre>
 *
 ******************************************************************************/
/***************************** Include Files *********************************/
#include <string.h>
#include <stdlib.h>
#include "xaie_routing.h"
#include "xaie_routing_internal.h"
#include "xaie_helper.h"
#include "xaie_feature_config.h"
#include "xaie_ss.h"
#include "xaie_plif.h"

#ifdef XAIE_FEATURE_ROUTING_ENABLE
/***************************** Helper APIs *********************************/

void PrintBits(u8 value)
{
	XAIE_DBG("0b");
	for (int i = 7; i >= 0; i--) {
		XAIE_DBG("|%d", (value >> i) & 1);
	}
	XAIE_DBG("|");
}

void PrintBDBits(uint64_t num)
{
	XAIE_DBG("0b");
	/* Ensure we start from the highest bit of a 48-bit number */
	for (int i = 47; i >= 0; i--) {
		XAIE_DBG("|%lu", (num >> i) & 1);
		if (i % 8 == 0) {
			XAIE_DBG("|");
		}
	}
}

/* Queue Operations */
static Queue* createQueue(unsigned capacity)
{
	Queue* queue = (Queue*) aligned_alloc(16, sizeof(Queue));
	if (queue == NULL){
		XAIE_ERROR("Queue allocation returned NULL.\n");
		return NULL;
	}
	queue->capacity = capacity;
	queue->front = queue->size = 0;
	queue->rear = capacity - 1;
	queue->array = (XAie_LocType*) aligned_alloc(16, queue->capacity * sizeof(XAie_LocType));
	return queue;
}

static bool isFull(Queue* queue)
{
	return (queue->size == (int)queue->capacity);
}

static bool isEmpty(Queue* queue)
{
	return (queue->size == 0);
}

static void _XAie_enqueue(Queue* queue, XAie_LocType item)
{
	if (isFull(queue))
		return;
	queue->rear = (queue->rear + 1) % queue->capacity;
	queue->array[queue->rear] = item;
	queue->size = queue->size + 1;
}

static XAie_LocType _XAie_dequeue(Queue* queue)
{
	if (isEmpty(queue)) {
		/* Invalid tile */
		return (XAie_LocType){-1, -1};
	}
	XAie_LocType item = queue->array[queue->front];
	queue->front = (queue->front + 1) % queue->capacity;
	queue->size = queue->size - 1;
	return item;
}

static void reversePath(XAie_LocType *path, int pathLength)
{
	for (int i = 0; i < pathLength / 2; i++) {
		XAie_LocType temp = path[i];
		path[i] = path[pathLength - 1 - i];
		path[pathLength - 1 - i] = temp;
	}
}

static void freeQueue(Queue *queue)
{
	if (queue != NULL) {
		if (queue->array != NULL) {
			/* Free the dynamically allocated array in the queue */
			free(queue->array);
		}
		/* Free the queue structure itself */
		free(queue);
	}
}

static void _XAie_printBitfield(int value, int bits)
{
	for (int i = bits - 1; i >= 0; i--) {
		putchar(((value & (1U << (u32)i)) != 0U) ? '1' : '0');
	}
}

/************************** Function Definitions *****************************/
/*****************************************************************************/
/*
 *
 * This API prints the current state of the Routing Constraints DB. Specifically,
 * it prints all the fields of the data structure that forms the Routing Constraints DB
 *
 * @param	XAie_RoutingInstance: Routing Instance
 *
 * @return	XAIE_OK on success.
 *
 * @note		None.
 *
 ******************************************************************************/

AieRC XAie_dumpConstraintsToPrint(XAie_RoutingInstance* RoutingInstance)
{
	if (RoutingInstance == NULL) {
		XAIE_DBG("Invalid input to dumpConstraintsToPrint\n");
		return XAIE_ERR;
	}

	XAIE_DBG("{\n  \"tiles\": [\n");

	for (u8 row = 0; row < RoutingInstance->NumRows; row++) {
		for (u8 col = 0; col < RoutingInstance->NumCols; col++) {
			XAie_CoreConstraint *constraint = RoutingInstance->
				CoreConstraintPerCore[col][row];
			XAie_coreConstraintToPrint(RoutingInstance, constraint, row, col);

			if (row < RoutingInstance->NumRows - 1 ||
					col < RoutingInstance->NumCols - 1) {
				XAIE_DBG(",\n");
			}
		}
	}

	XAIE_DBG("\n  ],\n");
	XAIE_DBG("  \"initialConstraints\": ");
	XAIE_DBG("\n}\n");
	return XAIE_OK;
}

/*****************************************************************************/
/*
 *
 * This API prints the current state of a particular AIE Tile within the Routing Constraints DB.
 *
 * @param	XAie_RoutingInstance: Routing Instance
 *
 * @return	XAIE_OK on success.
 *
 * @note		None.
 *
 ******************************************************************************/
AieRC XAie_dumpSpecificConstraintToPrint(XAie_RoutingInstance* RoutingInstance, u8 row, u8 col)
{
	if (RoutingInstance == NULL || row >= RoutingInstance->NumRows ||
			col >= RoutingInstance->NumCols) {
		XAIE_ERROR("XAie_dumpSpecificConstraintToPrint backend failed!.Invalid input or "
				"out of bounds row/col to dumpSpecificConstraintToPrint\n");
		return XAIE_ERR;
	}

	XAie_CoreConstraint *constraint = RoutingInstance->CoreConstraintPerCore[col][row];
	XAIE_DBG("{\n  \"tile\":\n");
	XAie_coreConstraintToPrint(RoutingInstance, constraint, row, col);
	XAIE_DBG("\n}\n");
	return XAIE_OK;
}

/*****************************************************************************/
/*
 *
 * This API prints all the fields of the AIE Tile Constraint for particular AIE Tile.
 *
 * @param	XAie_RoutingInstance: Routing Instance
 * @param	XAie_CoreConstraint: Routing Constraint for a particular AIE Tile
 * @param	row: row number
 * @param	row: column number
 * @return	XAIE_OK on success.
 *
 * @note		Internal API.
 *
 ******************************************************************************/
AieRC XAie_coreConstraintToPrint(XAie_RoutingInstance* RoutingInstance,
					XAie_CoreConstraint* constraint, u8 row, u8 col)
{
	const char* tileTypeStr[] = {"XAIE_AIE_SHIM", "XAIE_AIE_MEM", "XAIE_AIE_CORE"};

	XAIE_DBG("    {\n");
	XAIE_DBG("      \"row\": %d,\n", row);
	XAIE_DBG("      \"col\": %d,\n", col);

	XAIE_DBG("      \"MM2S_State\": [");
	_XAie_printBitfield(constraint->MM2S_State, 2);
	XAIE_DBG("],\n");

	XAIE_DBG("      \"S2MM_State\": [");
	_XAie_printBitfield(constraint->S2MM_State, 2);
	XAIE_DBG("],\n");

	XAIE_DBG("      \"AllChannelsInUse\": %d,\n", constraint->AllChannelsInUse);

	XAIE_DBG("      \"BDState\": [");
	if (_XAie_isMemTile(RoutingInstance, XAie_TileLoc(col, row)))
		_XAie_printBitfield(constraint->BDState, 48);
	else
		_XAie_printBitfield(constraint->BDState, 16);
	XAIE_DBG("],\n");
	XAIE_DBG("      \"AllBDsareInUse\": %d,\n", constraint->AllBDsareInUse);
	XAIE_DBG("      \"tile_type\": \"%s\",\n", tileTypeStr[constraint->tile_type]);
	XAIE_DBG("      \"SlaveEast\": [");
	_XAie_printBitfield(constraint->SlaveEast, 8);
	XAIE_DBG("],\n");

	XAIE_DBG("      \"SlaveWest\": [");
	_XAie_printBitfield(constraint->SlaveWest, 8);
	XAIE_DBG("],\n");

	XAIE_DBG("      \"SlaveSouth\": [");
	_XAie_printBitfield(constraint->SlaveSouth, 8);
	XAIE_DBG("],\n");

	XAIE_DBG("      \"SlaveNorth\": [");
	_XAie_printBitfield(constraint->SlaveNorth, 8);
	XAIE_DBG("],\n");

	XAIE_DBG("      \"MasterEast\": [");
	_XAie_printBitfield(constraint->MasterEast, 8);
	XAIE_DBG("],\n");

	XAIE_DBG("      \"MasterWest\": [");
	_XAie_printBitfield(constraint->MasterWest, 8);
	XAIE_DBG("],\n");

	XAIE_DBG("      \"MasterNorth\": [");
	_XAie_printBitfield(constraint->MasterNorth, 8);
	XAIE_DBG("],\n");
	XAIE_DBG("    }");

	return XAIE_OK;
}

/*****************************************************************************/
/*
 *
 * This API maps StrmSwPortType enum to the appropriate string and returns it.
 *
 * @param	type: StreamSwPortType enum
 * @return	const char* string.
 *
 * @note		Internal API.
 *
 ******************************************************************************/
static const char* _XAie_StrmSwPortTypeToString(StrmSwPortType type)
{
	switch(type) {
		case CORE:
			return "CORE";
		case DMA:
			return "DMA";
		case CTRL:
			return "CTRL";
		case FIFO:
			return "FIFO";
		case SOUTH:
			return "SOUTH";
		case WEST:
			return "WEST";
		case NORTH:
			return "NORTH";
		case EAST:
			return "EAST";
		case TRACE:
			return "TRACE";
		case SS_PORT_TYPE_MAX:
			return "SS_PORT_TYPE_MAX";
		default:
			return "Unknown";
	}
}

/*****************************************************************************/
/*
 *
 * This API prints the programmed route within the AIE-Tile grid
 *
 * @param	XAie_RoutingInstance: Routing Instance
 * @param	path: Current programmed path
 * @param	pathLength: Length of the programmed path
 * @param	path: Current programmed path
 * @param	source: Source Tile
 * @param	destination: Destination Tile
 * @return	void.
 *
 * @note		Internal API.
 *
 ******************************************************************************/
static void _XAie_drawRoute(XAie_RoutingInstance *routingInstance, XAie_LocType *path,
		int pathLength, XAie_LocType source, XAie_LocType destination)
{
	char **Grid;

	Grid = calloc(routingInstance->NumRows, sizeof(char *));
	if (Grid == NULL) {
		XAIE_DBG("Grid alloc failed.\n");
		return;
	}
	for (int i = 0; i < routingInstance->NumRows; i++) {
		Grid[i] = calloc(routingInstance->NumCols, sizeof(char));
		if (Grid[i] == NULL) {
			XAIE_DBG("Grid alloc failed.\n");
			goto out;
		}
		memset(Grid[i], '.', routingInstance->NumCols * sizeof(char));
	}

	/* Mark the path on the grid */
	for (int i = 0; i < pathLength; i++) {
		int gridRow = routingInstance->NumRows - 1 - path[i].Row;
		int gridCol = path[i].Col;

		/* Default mark for the path */
		char mark = '*';
		if (i == 0) {
			/* Mark for source*/
			mark = 'S';
		} else if (i == pathLength - 1) {
			/* Mark for destination */
			mark = 'D';
		} else if (i > 0 && i < pathLength - 1) {
			/* Optional: Add arrows to indicate direction */
			int dRow = path[i].Row - path[i-1].Row;
			int dCol = path[i].Col - path[i-1].Col;
			/* Down */
			if (dRow == -1 && dCol == 0) mark = 'v';
			/* Up */
			else if (dRow == 1 && dCol == 0) mark = '^';
			/* Left */
			else if (dRow == 0 && dCol == -1) mark = '<';
			/* Right */
			else if (dRow == 0 && dCol == 1) mark = '>';
		}

		if (gridCol < routingInstance->NumCols && gridRow >= 0 &&
				gridRow < routingInstance->NumRows) {
			Grid[gridRow][gridCol] = mark;
		}
	}

	/* Print the grid with bounding box and title */
	XAIE_DBG(" Routing Path between S[%d %d] and D[%d %d]\n",
			source.Col, source.Row, destination.Col, destination.Row);
	/* Top left corner of the box */
	XAIE_DBG(" +");
	for (int col = 0; col < routingInstance->NumCols; col++) {
		/* Top border of the box */
		XAIE_DBG("--");
	}
	/* Top right corner of the box */
	XAIE_DBG("+\n");

	for (int row = 0; row < routingInstance->NumRows; row++) {
		/* Left border of the box */
		XAIE_DBG(" | ");
		for (int col = 0; col < routingInstance->NumCols; col++) {
			XAIE_DBG("%c ", Grid[row][col]);
		}
		/* Right border of the box */
		XAIE_DBG("|\n");
	}

	/* Bottom left corner of the box */
	XAIE_DBG(" +");
	for (int col = 0; col < routingInstance->NumCols; col++) {
		/* Bottom border of the box */
		XAIE_DBG("--");
	}
	/* Bottom right corner of the box */
	XAIE_DBG("+\n");
out:
	for (int i = 0; i < routingInstance->NumRows; i++) {
		free(Grid[i]);
	}
	free(Grid);
}
/*****************************************************************************/
/*
 *
 * This API prints the programmed route between a user-provided source and destination tile.
 * If there is no programmed route between source and destination, XAIE_ERR is returned.
 *
 * @param	XAie_RoutingInstance: Routing Instance
 * @param	source: Source Tile
 * @param	destination: Destination Tile
 * @return	XAIE_OK on success, Error code on failure.
 *
 * @note		None.
 *
 ******************************************************************************/
AieRC XAie_RoutesReveal(XAie_RoutingInstance *routingInstance, XAie_LocType source,
		XAie_LocType destination)
{
	XAie_CoreConstraint* constraint = routingInstance->
				CoreConstraintPerCore[source.Col][source.Row];
	AieRC Rc = XAIE_OK;
	char **Grid;

	Grid = calloc(routingInstance->NumRows, sizeof(char *));
	if (Grid == NULL) {
		XAIE_DBG("Grid alloc failed.\n");
		return XAIE_ERR;
	}
	for (int i = 0; i < routingInstance->NumRows; i++) {
		Grid[i] = calloc(routingInstance->NumCols, sizeof(char));
		if (Grid[i] == NULL) {
			XAIE_DBG("Grid alloc failed.\n");
			Rc = XAIE_ERR;
			goto out;
		}
		memset(Grid[i], '.', routingInstance->NumCols * sizeof(char));
	}

	if (constraint == NULL) {
		XAIE_ERROR("XAie_RoutesReveal backend failed!. Incorrect arguments!.\n");
		Rc = XAIE_ERR;
		goto out;
	}

	XAie_RoutingPath *routingPath = _XAie_findRouteInRouteDB(constraint->routesDB,
								source, destination);


	if (routingPath == NULL) {
		XAIE_ERROR("XAie_RoutesReveal backend failed!. "
				"No Routing Path found between [%d %d] and [%d %d].\n",
				source.Col, source.Row, destination.Col, destination.Row);
		Rc = XAIE_ERR;
		goto out;
	}

	XAie_RoutingStep *currentStep = routingPath->nextStep;
	while (currentStep != NULL) {
		int gridRowSource = routingInstance->NumRows - 1 - currentStep->sourceTile.Row;
		int gridColSource = currentStep->sourceTile.Col;

		if (gridColSource < routingInstance->NumCols && gridRowSource >= 0
				&& gridRowSource < routingInstance->NumRows) {
			Grid[gridRowSource][gridColSource] = '*';
		}

		currentStep = currentStep->next;
	}

	/* Print the grid */
	for (int row = 0; row < routingInstance->NumRows; row++) {
		for (int col = 0; col < routingInstance->NumCols; col++) {
			XAIE_DBG("%c ", Grid[row][col]);
		}
		XAIE_DBG("\n");
	}
out:
	for (int i = 0; i < routingInstance->NumRows; i++) {
		free(Grid[i]);
	}
	free(Grid);

	return Rc;
}

/*****************************************************************************/
/*
 *
 * This API check if the tile is a Shim Tile
 *
 * @param	XAie_RoutingInstance: Routing Instance
 * @param	tile: Tile to be checked if Shim Tile or Not
 * @return	true is shim tile or else false.
 *
 * @note		Internal API.
 *
 ******************************************************************************/
static bool _XAie_isShimTile(XAie_RoutingInstance* RoutingInstance, XAie_LocType tile)
{
	XAie_CoreConstraint* constraint = RoutingInstance->
				CoreConstraintPerCore[tile.Col][tile.Row];
	return ((constraint != NULL) && (constraint->tile_type == (u32)XAIE_AIE_SHIM));
}

/*****************************************************************************/
/*
 *
 * This API check if the tile is a Mem Tile
 *
 * @param	XAie_RoutingInstance: Routing Instance
 * @param	tile: Tile to be checked if Mem Tile or Not
 * @return	true is Mem tile or else false.
 *
 * @note		Internal API.
 *
 ******************************************************************************/
static bool _XAie_isMemTile(XAie_RoutingInstance* RoutingInstance, XAie_LocType tile)
{
	XAie_CoreConstraint* constraint = RoutingInstance->
					CoreConstraintPerCore[tile.Col][tile.Row];
	return ((constraint != NULL) && (constraint->tile_type == (u32)XAIE_AIE_MEM));
}

/*****************************************************************************/
/*
 *
 * This API is used to set the availability of a particular port
 * on a particular SHIM tile, in the direction of Shim To AIE.
 *
 * @param	XAie_RoutingInstance: Routing Instance
 * @param	tile: Tile on which port availability needs to be manipulated
 * @param	port: port whose availability needs to be manipulated
 * @param	SetOrUnset: availability of port. If True, marks port available or else unavailable/in-use.
 * @return	XAIE_OK if successful or XAIE_ERR.
 *
 * @note		Internal API.
 *
 ******************************************************************************/
static AieRC _XAie_updatePortAvailabilityForShimDmaToAie(XAie_RoutingInstance *routingInstance,
		XAie_LocType tile, int port, bool SetOrUnset)
{
	XAie_CoreConstraint* constraint = routingInstance->
					CoreConstraintPerCore[tile.Col][tile.Row];

	if (!_XAie_isShimTile(routingInstance, tile))
		return XAIE_ERR;

	for (int i = 0; i < constraint->Host2AIEPortCount; i++) {
		if (constraint->Host2AIEPorts[i].port == port) {
			constraint->Host2AIEPorts[i].availability = SetOrUnset;
		}
	}
	return XAIE_OK;
}

/*****************************************************************************/
/*
 *
 * This API is used to set the availability of a particular port
 * on a particular SHIM tile, in the direction of AIE To SHIM.
 *
 * @param	XAie_RoutingInstance: Routing Instance
 * @param	tile: Tile on which port availability needs to be manipulated
 * @param	port: port whose availability needs to be manipulated
 * @param	SetOrUnset: availability of port. If True, marks port available or else unavailable/in-use.
 * @return	XAIE_OK if successful or XAIE_ERR.
 *
 * @note		Internal API.
 *
 ******************************************************************************/
static AieRC _XAie_updatePortAvailabilityForAieToShimDma(XAie_RoutingInstance *routingInstance,
			XAie_LocType tile, int port, bool SetOrUnset)
{
	XAie_CoreConstraint* constraint = routingInstance->
					CoreConstraintPerCore[tile.Col][tile.Row];

	if (!_XAie_isShimTile(routingInstance, tile))
		return XAIE_ERR;

	for (int i = 0; i < constraint->AIE2HostPortCount; i++) {
		if (constraint->AIE2HostPorts[i].port == port) {
			constraint->AIE2HostPorts[i].availability = SetOrUnset;
		}
	}
	return XAIE_OK;
}

/*****************************************************************************/
/*
 *
 * This API is used to find the available channel for a particular port of a particular
 * Shim Tile
 *
 * @param	XAie_RoutingInstance: Routing Instance
 * @param	tile: Tile on which the channel mapping needs to be found for the port
 * @param	HostToAie: Direction. If true, its Host to AIE, or else AIE to Host.
 * @return	channel number.
 *
 * @note		Internal API.
 *
 ******************************************************************************/
int findAvailableChannelIDforShimTile(XAie_RoutingInstance *routingInstance, XAie_LocType tile,
		u8 port, bool HostToAie)
{
	XAie_CoreConstraint* constraint =
		routingInstance->CoreConstraintPerCore[tile.Col][tile.Row];
	if (HostToAie) {
		for (int i = 0; i < constraint->Host2AIEPortCount; i++) {
			if (constraint->Host2AIEPorts[i].port == port) {
				// Found a matching stream
				return constraint->Host2AIEPorts[i].channel;
			}
		}
	}
	else {
		for (int i = 0; i < constraint->AIE2HostPortCount; i++) {
			if (constraint->AIE2HostPorts[i].port == port) {
				// Found a matching stream
				return constraint->AIE2HostPorts[i].channel;
			}
		}
	}
	return 0;
}

/*****************************************************************************/
/*
 *
 * This API is used to update the port availability for a given tile. Specifically,
 * this API marks the provided port as "used" for the provided tile.
 *
 * @param	XAie_RoutingInstance: Routing Instance
 * @param	tile: Tile on which the port needs to be updated
 * @param	portSource: The direction of the Slave port.
 * @param	portDest: The direction of the Master port.
 * @param	sourceStream: The slave port number.
 * @param	destStream: The master port number.
 * @return	void.
 *
 * @note		Internal API.
 *
 ******************************************************************************/
static void _XAie_updatePortAvailabilityForStrmConn(XAie_RoutingInstance *routingInstance,
				XAie_LocType tile, StrmSwPortType portSource, int sourceStream,
					StrmSwPortType portDest, int destStream)
{
	XAie_CoreConstraint* constraint = routingInstance->
		CoreConstraintPerCore[tile.Col][tile.Row];
	if (constraint == NULL)
		return;

	/* Unset bit for the slave port in the source direction */
	switch (portSource) {
		case SOUTH:
			constraint->SlaveSouth &= ~((u8)1 << (u8)sourceStream);
			break;
		case NORTH:
			constraint->SlaveNorth &= ~((u8)1 << (u8)sourceStream);
			break;
		case EAST:
			constraint->SlaveEast &= ~((u8)1 << (u8)sourceStream);
			break;
		case WEST:
			constraint->SlaveWest &= ~((u8)1 << (u8)sourceStream);
			break;
		case DMA:
			constraint->MM2S_State &= ~((u8)1 << (u8)sourceStream);
			break;
		default:
			break;
	}

	/* Unset bit for the master port in the destination direction */
	switch (portDest) {
		case SOUTH:
			constraint->MasterSouth &= ~((u8)1 << (u8)destStream);
			break;
		case NORTH:
			constraint->MasterNorth &= ~((u8)1 << (u8)destStream);
			break;
		case EAST:
			constraint->MasterEast &= ~((u8)1 << (u8)destStream);
			break;
		case WEST:
			constraint->MasterWest &= ~((u8)1 << (u8)destStream);
			break;
		case DMA:
			constraint->S2MM_State &= ~((u8)1 << (u8)destStream);
			break;
		default:
			break;
	}
}

/*****************************************************************************/
/*
 *
 * This API is used to find the first set of available ports between the two tiles and given source(slave) direction.
 * Specifically, this API finds the port that is compatible between the source and destination tile (ports match).
 * This API assumes that slave port's direction is "sourceDirection" and automatically infers the
 * master port direction from this.
 *
 * @param	XAie_RoutingInstance: Routing Instance
 * @param	tile: Tile on which the port needs to be updated
 * @param	portSource: The direction of the Slave port.
 * @param	portDest: The direction of the Master port.
 * @param	sourceStream: The slave port number.
 * @param	destStream: The master port number.
 * @return	void.
 *
 * @note		Internal API.
 *
 ******************************************************************************/
static int findFirstMatchingStreamForDestination(XAie_RoutingInstance* RoutingInstance,
		XAie_LocType SourceTile,
		XAie_LocType DestinationTile,
		StrmSwPortType SourceDirection)
{
	/* Access the CoreConstraint for the specified tiles */
	XAie_CoreConstraint* constraintSource = RoutingInstance->
				CoreConstraintPerCore[SourceTile.Col][SourceTile.Row];
	XAie_CoreConstraint* constraintDestination = RoutingInstance->
				CoreConstraintPerCore[DestinationTile.Col][DestinationTile.Row];
	if ((constraintSource == NULL) || (constraintDestination == NULL)) {
		/* If either constraint is null, return -1 */
		return -1;
	}

	u8 portAvailabilitySourceTile = -1;
	u8 portAvailabilityDestinationTile = -1;

	/* Determine which ports to check based on the direction */
	switch (SourceDirection) {
		case NORTH:
			portAvailabilitySourceTile = constraintSource->MasterNorth;
			portAvailabilityDestinationTile = constraintDestination->SlaveSouth;
			break;
		case SOUTH:
			portAvailabilitySourceTile = constraintSource->MasterSouth;
			portAvailabilityDestinationTile = constraintDestination->SlaveNorth;
			break;
		case EAST:
			portAvailabilitySourceTile = constraintSource->MasterEast;
			portAvailabilityDestinationTile = constraintDestination->SlaveWest;
			break;
		case WEST:
			portAvailabilitySourceTile = constraintSource->MasterWest;
			portAvailabilityDestinationTile = constraintDestination->SlaveEast;
			break;
		case DMA:
			XAIE_DBG("[ERROR] This should never be called on end tiles");
			break;
		default:
			// Invalid direction
			return -1;
	}

	/* Now find the first empty port that is available on both
	 * portAvailabilitySourceTile and portAvailabilityDestinationTile
	 */
	for (int i = 0; i < 8; i++) {
		if (((u8)portAvailabilitySourceTile & (1U << (u8)i)) &&
			((u8)portAvailabilityDestinationTile & (1U << (u8)i))) {
			/* Found a matching stream */
			return i;
		}
	}

	XAIE_DBG("[ERROR] No available stream between tiles {%d %d} and "
		"{%d %d} in the direction %s\n",
		SourceTile.Col, SourceTile.Row,
		DestinationTile.Col, DestinationTile.Row,
		_XAie_StrmSwPortTypeToString(SourceDirection));
	/* No available or matching stream found */
	return -1;
}

/*****************************************************************************/
/*
 *
 * This API is used to find the first available port between the two tiles and given source(slave) direction.
 * Specifically, this API finds the port that is compatible between the source and destination tile (ports match).
 * This API assumes that slave port's direction is "sourceDirection" and automatically infers the
 * master port direction from this.
 *
 * @param	XAie_RoutingInstance: Routing Instance
 * @param	tile: Tile on which the port needs to be updated
 * @param	portSource: The direction of the Slave port.
 * @param	portDest: The direction of the Master port.
 * @param	sourceStream: The slave port number.
 * @param	destStream: The master port number.
 * @return	port number or -1 if port unavailable
 *
 * @note		Internal API.
 *
 ******************************************************************************/
int _XAie_findFirstMatchingStream(XAie_RoutingInstance* RoutingInstance, XAie_LocType tile,
		StrmSwPortType direction, bool isEndTile)
{
	u8 portAvailability;
	/* Access the CoreConstraint for the specified tile */
	XAie_CoreConstraint* constraint = RoutingInstance->
				CoreConstraintPerCore[tile.Col][tile.Row];
	if (constraint == NULL)
		return -1;

	/* ShimTile specific checks */
	if (_XAie_isShimTile(RoutingInstance, tile)) {
		/* For Shim to AIE based connections, communication
		 * can only be done on very specific ports
		 */
		if (isEndTile ){
			for (int i = 0; i < constraint->AIE2HostPortCount; i++) {
				if (constraint->AIE2HostPorts[i].availability) {
					// Found a available port
					return constraint->AIE2HostPorts[i].port;
				}
			}
		}
		else {
			for (int i = 0; i < constraint->Host2AIEPortCount; i++) {
				if (constraint->Host2AIEPorts[i].availability) {
					/* Found a available port */
					return constraint->Host2AIEPorts[i].port;
				}
			}
		}
	}

	/* Determine which port to check based on the direction */
	switch (direction) {
		case NORTH:
			portAvailability = constraint->SlaveNorth;
			break;
		case SOUTH:
			portAvailability = constraint->SlaveSouth;
			break;
		case EAST:
			portAvailability = constraint->SlaveEast;
			break;
		case WEST:
			portAvailability = constraint->SlaveWest;
			break;
		case DMA:
			if (isEndTile)
				portAvailability = constraint->S2MM_State;
			else
				portAvailability = constraint->MM2S_State;
			break;
		default:
			// Invalid direction
			return -1;
	}

	for (int i = 0; i < 8; i++) {
		if ((portAvailability & (1U << (u32)i)) != 0U) {
			// Return the port number
			return i;
		}
	}

	XAIE_DBG("[ERROR] No available stream found for {%d %d} in the direction %s\n",
		tile.Col, tile.Row,_XAie_StrmSwPortTypeToString(direction));
	/* No available or matching stream found */
	return -1;
}

/*****************************************************************************/
/*
 *
 * This API is used to reset the available of a Buffer Descriptor on a particular tile
 *
 * @param	XAie_RoutingInstance: Routing Instance
 * @param	tile: Tile on which the BD avaiability needs to be reset.
 * @param	bdID: Buffer Descriptor ID.
 * @return	void
 *
 * @note		Internal API.
 *
 ******************************************************************************/
static void _XAie_resetBDAvailability(XAie_RoutingInstance *routingInstance,
		XAie_LocType tile, u8 bdID)
{
	u8 col = tile.Col;
	u8 row = tile.Row;
	XAie_CoreConstraint*** coreConstraints = routingInstance->CoreConstraintPerCore;

	/* Assuming MAX_BUFFER_IDS is the maximum number of buffer IDs per tile */
	/* Adjust this value based on your system's specification */
	const int MAX_BUFFER_IDS = 16;

	if (bdID < MAX_BUFFER_IDS) {
		coreConstraints[col][row]->BDState |= (1U << (u32)bdID);
	}
}

/*****************************************************************************/
/*
 *
 * This API is used to find the first available Buffer descriptor on a particular tile
 *
 * @param	XAie_RoutingInstance: Routing Instance
 * @param	tile: Tile on which the BD avaiability needs to be reset.
 * @return	buffer descriptor ID
 *
 * @note		Internal API.
 *
 ******************************************************************************/
static int _XAie_findAvailableBufferID(XAie_RoutingInstance *routingInstance, XAie_LocType tile)
{
	/* 1 indicates available, 0 indicates in use */
	u8 col = tile.Col;
	u8 row = tile.Row;
	XAie_CoreConstraint*** coreConstraints = routingInstance->CoreConstraintPerCore;

	/* MemTile has a max of 48 available BDs */
	const int MAX_BUFFER_IDS = 48;

	for (int bufferID = 0; bufferID < MAX_BUFFER_IDS; bufferID++) {
		if (coreConstraints[col][row]->BDState & (1U << (u32)bufferID)) {
			/* Mark this buffer ID as used (not available) before returning */
			coreConstraints[col][row]->BDState &= ~(1U << (u32)bufferID);
			return bufferID;
		}
	}

	XAIE_DBG("[ERROR] No available BD for tile {%d %d}\n", tile.Col, tile.Row);
	return -1;
}

/*****************************************************************************/
/*
 *
 * This API is part of the BFS logic to find a path between a source and destination tile.
 * Specifically, this API checks if there is an available port in the appropriate direction,
 * so as to facilitate a connection between the two tiles.
 *
 *
 * @param	RoutingInstance: Routing Instance
 * @param	RouteConstraints: Route Constraints (blacklisted or whitelisted tiles)
 * @param	currentTile: Source Tile
 * @param	adjTile: Destination Tile
 * @param	adjTile: Destination Tile
 * @param	visited: 2D matrix to track visited tiles in the AIE-Tile grid.
 * @return	True if route possible between two tiles, else False.
 *
 * @note		Internal API.
 *
 ******************************************************************************/
static bool _XAie_isAdjTileValidForCurrTile(XAie_RoutingInstance* RoutingInstance,
		XAie_RouteConstraints* RouteConstraints,
		XAie_LocType currentTile, XAie_LocType adjTile,
		StrmSwPortType direction, bool** visited)
{
	XAie_CoreConstraint ***coreConstraints = RoutingInstance->CoreConstraintPerCore;

	if (!(adjTile.Col < RoutingInstance->NumCols &&
                            adjTile.Row < RoutingInstance->NumRows))
		return false;

	/* Check port availability in both current and adjacent tile */
	bool isPortAvailable = false;
	if ((coreConstraints[currentTile.Col][currentTile.Row] != NULL) &&
			(coreConstraints[adjTile.Col][adjTile.Row] != NULL)){
		XAie_CoreConstraint *currentConstraint = coreConstraints[currentTile.Col]
									[currentTile.Row];
		XAie_CoreConstraint *adjConstraint = coreConstraints[adjTile.Col][adjTile.Row];

		switch (direction) {
			/* North */
			case NORTH:
				isPortAvailable = ((currentConstraint->SlaveNorth != 0U) &&
						(adjConstraint->MasterSouth != 0U));
				break;
			/* South */
			case SOUTH:
				isPortAvailable = ((currentConstraint->SlaveSouth != 0U) &&
						(adjConstraint->MasterNorth != 0U));
				break;
			/* East */
			case EAST:
				isPortAvailable = ((currentConstraint->SlaveEast != 0U) &&
						(adjConstraint->MasterWest != 0U));
				break;
			/* West */
			case WEST:
				isPortAvailable = ((currentConstraint->SlaveWest != 0U) &&
						(adjConstraint->MasterEast != 0U));
				break;
			default:
				break;
		}
	}

	/* check if adjacent tile is blacklisted */
	return !_XAie_isTileBlackListed(adjTile, RouteConstraints)
		&& !visited[adjTile.Col][adjTile.Row] &&
		isPortAvailable;
}

/*****************************************************************************/
/*
 *
 * This API clears the stream switch settings on the listed tiles
 **
 * @param	routingInstance: Routing Instance
 * @param	source: Source Tile
 * @param	destination: Destination Tile
 * @param	ShouldModifyCoreConfig: if True, the tile previously marked to
 *           "execute" will be unmarked
 * @return	XAIE_OK if successful.
 *
 * @note		None.
 *
 ******************************************************************************/
AieRC XAie_RoutingSwitchReset(XAie_RoutingInstance *routingInstance,
		XAie_LocType* listOfTiles, uint32_t NoOfTiles)
{
	AieRC rc = XAIE_OK;

	/* Loop through the list of tiles */
	for (uint32_t i = 0; i < NoOfTiles; i++) {
		XAie_LocType currentTile = listOfTiles[i];
		XAie_CoreConstraint* tileConstraint = routingInstance->
				CoreConstraintPerCore[currentTile.Col][currentTile.Row];

		/* Check if the tile is auto-configured; if so, skip resetting */
		if (tileConstraint->isAutoConfigured) {
			XAIE_DBG("XAie_RoutingSwitchReset "
				"Warning: Skipping auto-configured tile at {%d, %d}.\n",
				currentTile.Col, currentTile.Row);
			/* Skip this tile */
			continue;
		}

		StrmSwPortType directions[] = {DMA, SOUTH, WEST, NORTH, EAST};
		/* Number of elements in the directions array */
		int dirCount = sizeof(directions) / sizeof(directions[0]);

		/* Iterate over all combinations of directions for slave and master ports */
		for (int slaveIdx = 0; slaveIdx < dirCount; ++slaveIdx) {
			StrmSwPortType slaveDir = directions[slaveIdx];
			u8 slavePortBitfield = 0;

			/* Determine slave port bitfield based on direction */
			switch (slaveDir) {
				case SOUTH:
					slavePortBitfield = tileConstraint->SlaveSouth;
					break;
				case NORTH:
					slavePortBitfield = tileConstraint->SlaveNorth;
					break;
				case EAST:
					slavePortBitfield = tileConstraint->SlaveEast;
					break;
				case WEST:
					slavePortBitfield = tileConstraint->SlaveWest;
					break;
				case DMA:
					/* MM2S for DMA */
					slavePortBitfield = tileConstraint->ShimMM2S_State;
					break;
				default:
					break;
			}

			for (int masterIdx = 0; masterIdx < dirCount; ++masterIdx) {
				StrmSwPortType masterDir = directions[masterIdx];
				u8 masterPortBitfield = 0;

				/* Determine master port bitfield based on direction */
				switch (masterDir) {
					case SOUTH:
						masterPortBitfield = tileConstraint->MasterSouth;
						break;
					case NORTH:
						masterPortBitfield = tileConstraint->MasterNorth;
						break;
					case EAST:
						masterPortBitfield = tileConstraint->MasterEast;
						break;
					case WEST:
						masterPortBitfield = tileConstraint->MasterWest;
						break;
					case DMA:
						/* S2MM for DMA */
						masterPortBitfield = tileConstraint->ShimS2MM_State;
						break;
					default:
						break;
				}

				/* Iterate through bitfields to find set bits and disable connection
				 */
				// Assuming 8 bits for simplicity; adjust as needed
				for (u8 bit = 0; bit < 8; bit++) {
					// Check if slave port bit is set
					if ((slavePortBitfield & (1U << (u32)bit)) != 0U){
						// Repeat for master port
						for (u8 bitM = 0; bitM < 8; bitM++) {
							// Check if master port bit is set
							if ((masterPortBitfield & (1U <<(u32) bitM)) != 0U) {
								/* since we are brute forcing our
								 * way, we may stumble upon
								 * invalid configurations
								 */
								/*The below API is a (silenced)
								 * version of
								 * "XAie_StrmConnCctDisable"
								 * that gracefully skips those
								 * configurations without
								 * erroring out.
								 */
								rc = XAie_StrmConnCctDisable(
								routingInstance->DeviceInstance,
								currentTile, slaveDir, bit,
								masterDir, bitM);
							}
						}
					}
				}
			}
		}
	}
	return rc;
}



/****************************************************************************
 *
 * This function provides a detailed dump of the routing and switch configuration
 * information for a specified list of tiles within an AIE array.
 * It iterates over each tile provided in the list and prints its routing constraints,
 * port states, and the execution status of its core. This includes detailed information
 * such as the state of memory-to-stream (MM2S) and stream-to-memory (S2MM) configurations,
 * the condition of slave and master ports for each cardinal direction, and the state of
 * block descriptors (BDState). Additionally, for tiles of type XAIE_AIE_SHIM, it prints
 * the state of SHIM MM2S and S2MM. The function also traverses and prints the programmed
 * routing paths, detailing the source and destination tiles along with the MM2S and S2MM
 * port numbers involved in the routing.
 *
 * @param	routingInstance: Pointer to the routing instance containing the
 *                           configuration and state information of the AIE array.
 * @param	listOfTiles: Array of XAie_LocType structures representing the tiles
 *                       for which the routing and switch information is to be dumped.
 * @param	NoOfTiles: Number of tiles in the listOfTiles array.
 *
 * @note		This function prints the information to the standard output and does
 *           not return any value.
 *
 *****************************************************************************/

void XAie_DumpRoutingSwitchInfo(XAie_RoutingInstance *routingInstance,
		XAie_LocType* listOfTiles, uint32_t NoOfTiles)
{
	/* Loop through the list of tiles */
	for (uint32_t i = 0; i < NoOfTiles; i++) {
		XAie_LocType CurrTile = listOfTiles[i];
		XAie_CoreConstraint* CurrentTileconstraint = routingInstance->
					CoreConstraintPerCore[CurrTile.Col][CurrTile.Row];

		/* Print Tile information */
		XAIE_DBG("*********************************************\n");
		XAIE_DBG("Tile[%d, %d]:\n", CurrTile.Col, CurrTile.Row);

		/* CoreConstraint fields */
		XAIE_DBG("\tisAutoConfigured: %s\n",
			CurrentTileconstraint->isAutoConfigured ? "true" : "false");
		XAIE_DBG("\tMM2S_State: ");
		PrintBits(CurrentTileconstraint->MM2S_State);
		XAIE_DBG("\n");
		XAIE_DBG("\tS2MM_State: ");
		PrintBits(CurrentTileconstraint->S2MM_State);
		XAIE_DBG("\n");
		if (CurrentTileconstraint->tile_type == XAIE_AIE_SHIM) {
			XAIE_DBG("\tShimMM2S_State: ");
			PrintBits(CurrentTileconstraint->ShimMM2S_State);
			XAIE_DBG("\n");
			XAIE_DBG("\tShimS2MM_State: ");
			PrintBits(CurrentTileconstraint->ShimS2MM_State);
			XAIE_DBG("\n");

		}
		XAIE_DBG("\tBDState: ");
		PrintBDBits(CurrentTileconstraint->BDState);
		XAIE_DBG("\n");

		/* Slave and Master ports */
		XAIE_DBG("\tSlaveEast: ");
		PrintBits(CurrentTileconstraint->SlaveEast);
		XAIE_DBG("\n");
		XAIE_DBG("\tSlaveWest: ");
		PrintBits(CurrentTileconstraint->SlaveWest);
		XAIE_DBG("\n");
		XAIE_DBG("\tSlaveSouth: ");
		PrintBits(CurrentTileconstraint->SlaveSouth);
		XAIE_DBG("\n");
		XAIE_DBG("\tSlaveNorth: ");
		PrintBits(CurrentTileconstraint->SlaveNorth);
		XAIE_DBG("\n");
		XAIE_DBG("\tMasterEast: ");
		PrintBits(CurrentTileconstraint->MasterEast);
		XAIE_DBG("\n");
		XAIE_DBG("\tMasterWest: ");
		PrintBits(CurrentTileconstraint->MasterWest);
		XAIE_DBG("\n");
		XAIE_DBG("\tMasterSouth: ");
		PrintBits(CurrentTileconstraint->MasterSouth);
		XAIE_DBG("\n");
		XAIE_DBG("\tMasterNorth: ");
		PrintBits(CurrentTileconstraint->MasterNorth);
		XAIE_DBG("\n");

		/* Core execution status */
		XAIE_DBG("\tisCoreExecuting: %s\n",
			CurrentTileconstraint->isCoreExecuting ? "true" : "false");

		/* Routing paths */
		XAie_ProgrammedRoutes *currentRoute = CurrentTileconstraint->routesDB;
		while (currentRoute != NULL) {
			XAIE_DBG("\tRouting Path:\n");
			XAie_RoutingPath* routePath = currentRoute->routePath;
			XAIE_DBG("\tSource[%d, %d] -> Destination[%d, %d], "
					"MM2S_portNo: %d, S2MM_portNo: %d\n",
					routePath->source.Col, routePath->source.Row,
					routePath->destination.Col, routePath->destination.Row,
					routePath->MM2S_portNo, routePath->S2MM_portNo);

			/* Beautify the routing steps */
			XAie_RoutingStep *step = routePath->nextStep;
			uint16_t stepCount = 0;
			while (step != NULL) {
				XAIE_DBG("\t------------------------------------------------\n");
				XAIE_DBG("\t| Step: %d\n", stepCount);
				XAIE_DBG("\t| Source Tile: [%d, %d] \n",
					step->sourceTile.Col, step->sourceTile.Row);
				XAIE_DBG("\t| Source Stream: %d, Direction: %s\n",
					step->sourceStream,
					_XAie_StrmSwPortTypeToString(step->source_direction));
				XAIE_DBG("\t| Dest Stream: %d, Direction: %s\n",
					step->destStream,
					_XAie_StrmSwPortTypeToString(step->dest_direction));
				XAIE_DBG("\t------------------------------------------------\n");
				step = step->next;
				stepCount++;
			}
			currentRoute = currentRoute->nextRoute;
		}
		XAIE_DBG("*********************************************\n");
	}
}

/*****************************************************************************/
/*
 *
 * This API is an inverse of XAie_Route. Basically, this API remove the configure route
 * between the source and destination tile.
 * Specifically, this API checks if there is an available port in the appropriate direction,
 * so as to facilitate a connection between the two tiles.
 *
 *
 * @param	routingInstance: Routing Instance
 * @param	source: Source Tile
 * @param	destination: Destination Tile
 * @param	ShouldModifyCoreConfig: if True, the tile previously marked to
 *           "execute" will be unmarked
 * @return	XAIE_OK if successful.
 *
 * @note		None.
 *
 ******************************************************************************/
AieRC XAie_DeRoute(XAie_RoutingInstance *routingInstance, XAie_LocType source,
		XAie_LocType destination, bool ShouldModifyCoreConfig)
{
	if (routingInstance == XAIE_NULL) {
		XAIE_ERROR("XAie_DeRoute backend failed!. Invalid arguments\n");
		return XAIE_INVALID_ARGS;
	}

	XAie_DevInst *DevInst = routingInstance->DeviceInstance;
	XAie_CoreConstraint* Sourceconstraint = routingInstance->
					CoreConstraintPerCore[source.Col][source.Row];
	XAie_CoreConstraint* Destconstraint = routingInstance->
					CoreConstraintPerCore[destination.Col][destination.Row];

	/* Ensure that the routing path exists for the given source and destination */
	if (Sourceconstraint == NULL || Sourceconstraint->routesDB == NULL) {
		XAIE_ERROR("XAie_DeRoute failed!. Routing Path does not exist "
				"between Source {%d, %d} and Destination {%d, %d}\n",
				source.Col, source.Row, destination.Col, destination.Row);
		/* Routing path does not exist */
		return XAIE_ERR;
	}

	if (ShouldModifyCoreConfig && !_XAie_isShimTile(routingInstance, source) &&
						!_XAie_isMemTile(routingInstance, source))
		Sourceconstraint->isCoreExecuting = false;

	if (ShouldModifyCoreConfig && !_XAie_isShimTile(routingInstance, destination) &&
						!_XAie_isMemTile(routingInstance, destination))
		Destconstraint->isCoreExecuting = false;

	XAie_RoutingPath *routingPath = _XAie_findRouteInRouteDB(Sourceconstraint->
						routesDB, source, destination);

	XAie_RoutingStep *currentStep = routingPath->nextStep;
	while (currentStep != NULL) {
		int RC = XAie_StrmConnCctDisable(DevInst, currentStep->sourceTile,
				currentStep->source_direction, currentStep->sourceStream,
				currentStep->dest_direction, currentStep->destStream);

		XAIE_DBG("\n [COMMAND] XAie_StrmConnCctDisable({%d,%d}, %s, %d, %s, %d)\n",
				currentStep->sourceTile.Col, currentStep->sourceTile.Row,
				_XAie_StrmSwPortTypeToString(currentStep->source_direction),
				currentStep->sourceStream,
				_XAie_StrmSwPortTypeToString(currentStep->dest_direction),
				currentStep->destStream);
		if (RC != XAIE_OK) {
			XAIE_ERROR("\n [COMMAND] XAie_StrmConnCctDisable( {%d,%d}, %s, %d, %s, %d)"
				       "FAILED!!\n",
					currentStep->sourceTile.Col,
					currentStep->sourceTile.Row,
					_XAie_StrmSwPortTypeToString(currentStep->source_direction),
					currentStep->sourceStream,
					_XAie_StrmSwPortTypeToString(currentStep->dest_direction),
					currentStep->destStream);
			return XAIE_ERR;
		}

		/* Set back the ports for the source and destination */
		_XAie_updatePortAvailabilityForStrmConnInverse(
				routingInstance, currentStep->sourceTile,
				currentStep->source_direction, currentStep->sourceStream,
				currentStep->dest_direction, currentStep->destStream);

		/* if the current tile is a shim tile, also disable the shim connection */
		if (_XAie_isShimTile(routingInstance, routingPath->source)) {
			/* MM2S */
			if (currentStep == routingPath->nextStep) {
				/* currently there is no API to disable ShimDMA to AIEStream Port */
				/* int RC = XAie_DisableShimDmaToAieStrmPort(DevInst,
				 * currentStep->sourceTile, currentStep->sourceStream);
				 */

				/* XAIE_DBG("\nXAie_DisableShimDmaToAieStrmPort({%d,%d}, %d)\n",
				 * currentStep->sourceTile.Col, currentStep->sourceTile.Row
				 *  currentStep->sourceStream);
				 */
				_XAie_updatePortAvailabilityForShimDmaToAie(routingInstance,
					currentStep->sourceTile, currentStep->sourceStream, true);
			}
			else {
				//currently there is no API to disable ShimDMA to AIEStream Port
				// int RC = XAie_DisableAieToShimDmaStrmPort(DevInst, currentStep->sourceTile,
				//     currentStep->sourceStream);

				// XAIE_DBG("\nXAie_DisableAieToShimDmaStrmPort({%d,%d}, %d)\n",
				// currentStep->sourceTile.Col, currentStep->sourceTile.Row
				// currentStep->sourceStream);
				_XAie_updatePortAvailabilityForAieToShimDma(routingInstance, currentStep->sourceTile, currentStep->sourceStream, true);
			}
		}
		currentStep = currentStep->next;
	}

	/* After disconnecting the routing steps */
	if (routingPath != NULL) {
		/* Free the routing steps associated with this path */
		if (routingPath->nextStep != NULL) {
			_XAie_FreeRoutingSteps(routingPath->nextStep);
			routingPath->nextStep = NULL;
		}

		/* Free this route in this routesDB */
		_XAie_FreeRouteFromRoutesDB(&(Sourceconstraint->routesDB), source, destination);
	}
	return XAIE_OK;
}

/*****************************************************************************/
/*
 *
 * This API is used to update the port availability for a given tile. Specifically,
 * this API marks the provided port as "available" for the provided tile.
 *
 * @param	XAie_RoutingInstance: Routing Instance
 * @param	tile: Tile on which the port needs to be updated
 * @param	portSource: The direction of the Slave port.
 * @param	portDest: The direction of the Master port.
 * @param	sourceStream: The slave port number.
 * @param	destStream: The master port number.
 * @return	void.
 *
 * @note		Internal API.
 *
 ******************************************************************************/
static void _XAie_updatePortAvailabilityForStrmConnInverse(XAie_RoutingInstance *routingInstance,
		XAie_LocType tile, StrmSwPortType portSource, int sourceStream,
		StrmSwPortType portDest, int destStream)
{
	XAie_CoreConstraint* constraint = routingInstance->
					CoreConstraintPerCore[tile.Col][tile.Row];
	if (constraint == NULL)
		return;

	/* Set bit for the slave port in the source direction */
	switch (portSource) {
		case SOUTH:
			constraint->SlaveSouth |= ((u8)1 << (u8)sourceStream);
			break;
		case NORTH:
			constraint->SlaveNorth |= ((u8)1 << (u8)sourceStream);
			break;
		case EAST:
			constraint->SlaveEast |= ((u8)1 << (u8)sourceStream);
			break;
		case WEST:
			constraint->SlaveWest |= ((u8)1 << (u8)sourceStream);
			break;
		case DMA:
			constraint->MM2S_State |= ((u8)1 << (u8)sourceStream);
			break;
		default:
			break;
	}

	/* Set bit for the master port in the destination direction */
	switch (portDest) {
		case SOUTH:
			constraint->MasterSouth |= ((u8)1 << (u8)destStream);
			break;
		case NORTH:
			constraint->MasterNorth |= ((u8)1 << (u8)destStream);
			break;
		case EAST:
			constraint->MasterEast |= ((u8)1 << (u8)destStream);
			break;
		case WEST:
			constraint->MasterWest |= ((u8)1 << (u8)destStream);
			break;
		case DMA:
			constraint->S2MM_State |= ((u8)1 << (u8)destStream);
			break;
		default:
			break;
	}
}

/*****************************************************************************/
/*
 *
 * This API performs the routing between the source and destination tiles by
 * issuing appropriate AIE-RT commands.
 *
 * @param	routingInstance: Routing Instance
 * @param	source: Source tile
 * @param	destination: Destination Tile
 * @param	path: Linked-List that represents the path between source and destination tile.
 * @param	pathLength: Path length.
 * @return	XAIE_OK is successful.
 *
 * @note		Internal API.
 *
 ******************************************************************************/
static AieRC _XAie_performRoutingOnPath(XAie_RoutingInstance *routingInstance, XAie_LocType source,
		XAie_LocType destination, XAie_LocType *path, int pathLength)
{
	XAie_DevInst *DevInst = routingInstance->DeviceInstance;
	/* Assuming XAIE_OK is a constant representing successful operation */
	int RC = XAIE_OK;
	int dirSource, dirDest, sourceStream;
	/* Source stream for the source tile */
	int actualdirSource;
	/* Used to store the last destination stream */
	StrmSwPortType lastDir = SOUTH;
	int lastStream = -1;
	XAie_CoreConstraint* Sourceconstraint = routingInstance->
					CoreConstraintPerCore[source.Col][source.Row];
	XAie_RoutingStep *lastStep = NULL;
	XAie_RoutingPath* CurrRoutingPath = NULL;

	CurrRoutingPath = (XAie_RoutingPath *)aligned_alloc(16, sizeof(XAie_RoutingPath));
	if (CurrRoutingPath == NULL) {
		XAIE_ERROR("Routing path allocation failed!\n");
		return XAIE_ERR;
	}
	CurrRoutingPath->source = source;
	CurrRoutingPath->destination = destination;
	CurrRoutingPath->MM2S_portNo = -1;
	CurrRoutingPath->S2MM_portNo = -1;
	CurrRoutingPath->nextStep = NULL;

	/* Initialize the routing path for the source tile */
	for (int i = 0; i < pathLength; i++) {
		if (i == pathLength - 1) {
			/* Special handling on the last tile */
			XAie_LocType LastTile = path[i];
			StrmSwPortType dirLast = (_XAie_isShimTile(routingInstance,
						LastTile)) ? SOUTH : DMA;
			int destStream = _XAie_findFirstMatchingStream(routingInstance, LastTile,
							dirLast, true/*is End Tile*/);

			if (destStream == -1) {
				XAIE_ERROR("Routing Failed. No available port on AIE-Tile {%d, %d} "
				"in the direction of %s. Please check constraints "
				"and consider re-route!\n",
				LastTile.Col,
				LastTile.Row,
				_XAie_StrmSwPortTypeToString(dirLast));
				return XAIE_ERR;
			}

			/* set current tile as autoconfigured */
			XAie_CoreConstraint* CurrentTileconstraint = routingInstance->
						CoreConstraintPerCore[LastTile.Col][LastTile.Row];
			CurrentTileconstraint->isAutoConfigured = true;

			/* Print the command and parameters before executing */
			XAIE_DBG("\n [COMMAND] XAie_StrmConnCctEnable({%d,%d}, %s, %d, %s, %d)\n",
					LastTile.Col, LastTile.Row,
					_XAie_StrmSwPortTypeToString(lastDir),
					lastStream,
					_XAie_StrmSwPortTypeToString(dirLast),
					destStream);

			CurrRoutingPath->S2MM_portNo = destStream;

			RC |= (u32)XAie_StrmConnCctEnable(DevInst, LastTile, lastDir,
					lastStream, dirLast, destStream);
			if (RC != XAIE_OK) {
				XAIE_ERROR("Routing Failed!. XAie_StrmConnCctEnable Failed!\n");
				return XAIE_ERR;
			}

			_XAie_updatePortAvailabilityForStrmConn(routingInstance, LastTile,
							lastDir, lastStream, dirLast, destStream);

			/* Create a new routing step */
			XAie_RoutingStep *newStep = (XAie_RoutingStep *)aligned_alloc(16, sizeof(XAie_RoutingStep));
			newStep->sourceTile = LastTile;
			newStep->sourceStream = lastStream;
			newStep->destStream = destStream;
			newStep->source_direction = lastDir;
			newStep->dest_direction = dirLast;
			newStep->next = NULL;

			/* Add the step to the routing path */
			if (lastStep == NULL) {
				CurrRoutingPath->nextStep = newStep;
			} else {
				lastStep->next = newStep;
			}
			lastStep = newStep;
			if (_XAie_isShimTile(routingInstance, LastTile)) {
				/* Print the command and parameters before executing */
				XAIE_DBG("\n [COMMAND] "
					"XAie_EnableAieToShimDmaStrmPort({%d,%d}, %d)\n",
					LastTile.Col, LastTile.Row, destStream);

				/* Call the function */
				RC |= (u32)XAie_EnableAieToShimDmaStrmPort(DevInst, LastTile,
						destStream);
				if (RC != XAIE_OK) {
					XAIE_ERROR("Routing Failed!. "
						"XAie_EnableAieToShimDmaStrmPort Failed!\n");
					return XAIE_ERR;
				}
				_XAie_updatePortAvailabilityForAieToShimDma(routingInstance,
								LastTile, destStream, false);
			}

			if (Sourceconstraint->routesDB == NULL) {
				XAIE_DBG("Adding first route to routes DB\n");
				Sourceconstraint->routesDB = (XAie_ProgrammedRoutes *)aligned_alloc(16,
								sizeof(XAie_ProgrammedRoutes));
				Sourceconstraint->routesDB->routePath = CurrRoutingPath;
			}
			else
				_XAie_addRouteInRoutesDB(&(Sourceconstraint->routesDB),
							CurrRoutingPath);
		}
		else {
			XAie_LocType SourceTile = path[i];
			XAie_LocType DestinationTile = path[i + 1];

			/* Determine direction of flow */
			if (DestinationTile.Col == SourceTile.Col) {
				dirSource = (DestinationTile.Row > SourceTile.Row) ? NORTH : SOUTH;
				dirDest = (DestinationTile.Row > SourceTile.Row) ? SOUTH : NORTH;
			} else {
				dirSource = (DestinationTile.Col > SourceTile.Col) ? EAST : WEST;
				dirDest = (DestinationTile.Col > SourceTile.Col) ? WEST : EAST;
			}

			if (_XAie_isShimTile(routingInstance, SourceTile))
				actualdirSource = lastDir;
			else
				actualdirSource = (i == 0) ? DMA : lastDir;

			/* Destination stream for the destination tile */
			int actualdirDest = (i == pathLength - 1) ? DMA : dirSource;
			if (_XAie_isShimTile(routingInstance, SourceTile)) {
				if (i == 0)
					sourceStream = _XAie_findFirstMatchingStream(
							routingInstance, SourceTile,
							actualdirSource, false);
				else
					sourceStream = lastStream;
			}
			else
				sourceStream = _XAie_findFirstMatchingStream(
						routingInstance, SourceTile,
						actualdirSource, false);

			int destStream = findFirstMatchingStreamForDestination(
					routingInstance, SourceTile, DestinationTile, dirSource);

			if (sourceStream == -1 || destStream == -1) {
				/* Adding XAIE_DBG statement to print the command with actual values
				 * */
				XAIE_ERROR("\n XAie_StrmConnCctEnable cannot be programmed between"
					" [%d %d] and [%d %d] due to unavailable stream ports. "
					"Please check constraints and consider re-route!\n",
					SourceTile.Col,
					SourceTile.Row,
					DestinationTile.Col,
					DestinationTile.Row);
				return XAIE_ERR;
			}

			/* Establish connection */
			StrmSwPortType portSource = actualdirSource;
			StrmSwPortType portDest = actualdirDest;

			/* set current tile as autoconfigured */
			XAie_CoreConstraint* CurrentTileconstraint = routingInstance->
					CoreConstraintPerCore[SourceTile.Col][SourceTile.Row];
			CurrentTileconstraint->isAutoConfigured = true;

			/* Adding XAIE_DBG statement to print the command with actual values */
			XAIE_DBG("\n [COMMAND] XAie_StrmConnCctEnable({%d,%d}, %s, %d, %s, %d)\n",
					SourceTile.Col, SourceTile.Row,
					_XAie_StrmSwPortTypeToString(portSource),
					sourceStream,
					_XAie_StrmSwPortTypeToString(portDest),
					destStream);

			RC |= (u32)XAie_StrmConnCctEnable(DevInst, SourceTile, portSource,
						sourceStream, portDest, destStream);
			if (RC != XAIE_OK) {
				XAIE_ERROR("Routing Failed!. "
					"XAie_EnableAieToShimDmaStrmPort Failed!\n");
				return XAIE_ERR;
			}
			/* Create a new routing step */
			XAie_RoutingStep *newStep = (XAie_RoutingStep *)aligned_alloc(16, sizeof(XAie_RoutingStep));
			newStep->sourceTile = SourceTile;
			newStep->sourceStream = sourceStream;
			newStep->destStream = destStream;
			newStep->source_direction = portSource;
			newStep->dest_direction = portDest;
			newStep->next = NULL;
			/* Add the step to the routing path */
			if (lastStep == NULL) {
				CurrRoutingPath->nextStep = newStep;
			} else {
				lastStep->next = newStep;
			}
			lastStep = newStep;

			if (i == 0) {
				CurrRoutingPath->MM2S_portNo = sourceStream;
			}

			_XAie_updatePortAvailabilityForStrmConn(routingInstance, SourceTile,
						portSource, sourceStream, portDest, destStream);

			/* Update lastStream for the next iteration */
			lastDir = dirDest;
			lastStream = destStream;

			if (_XAie_isShimTile(routingInstance, SourceTile) && (i == 0)) {
				/* Print the command and parameters before executing */
				XAIE_DBG("\n [COMMAND] "
					"XAie_EnableShimDmaToAieStrmPort({%d,%d}, %d)\n",
					SourceTile.Col, SourceTile.Row, sourceStream);

				/* Call the function */
				RC |= (u32)XAie_EnableShimDmaToAieStrmPort(DevInst,
						SourceTile, sourceStream);
				if (RC != XAIE_OK) {
					XAIE_ERROR("Routing Failed!. "
						"XAie_EnableShimDmaToAieStrmPort Failed!\n");
					return XAIE_ERR;
				}
				_XAie_updatePortAvailabilityForShimDmaToAie(routingInstance,
							SourceTile, sourceStream, false);
			}
		}
	}

	return XAIE_OK;
}

/*****************************************************************************/
/*
 *
 * This API programs the buffer descriptors by
 * issuing appropriate AIE-RT commands.
 *
 * @param	routingInstance: Routing Instance
 * @param	source: Source tile
 * @param	SourceObject: This can either be the source address or source XAie_MemInstance, based on backend
 * @param	data_size: Data size
 * @param	DestinationObject: This can either be the source address or destination XAie_MemInstance, based on backend
 * @param	destination: Destination Tile
 * @param	BDs: Data structure holding the appropriate BDs.

 * @return	XAIE_OK is successful.
 *
 * @note		Internal API.
 *
 ******************************************************************************/
static AieRC _XAie_programBufferDescriptors(XAie_RoutingInstance *routingInstance,
		XAie_LocType source, void* SourceObject, u32 data_size, void* DestinationObject,
		XAie_LocType destination, XAie_BDs* BDs)
{
	XAie_DevInst *DevInst = routingInstance->DeviceInstance;
	int RC = XAIE_OK;

	/* Initialize buffer descriptors for the source and destination tiles */
	XAie_DmaDesc SourceBufferDescriptor = {0};
	XAie_DmaDesc DestBufferDescriptor = {0};

	/* Program the buffer descriptor for the source tile */
	RC |= (u32)XAie_DmaDescInit(DevInst, &SourceBufferDescriptor, source);
	XAIE_DBG("\n [COMMAND] "
		"XAie_DmaDescInit(&SourceBufferDescriptor, {%d,%d})\n",
		source.Col, source.Row);
	if (_XAie_isShimTile(routingInstance, source)) {
		if ((DevInst->Backend->Type == XAIE_IO_BACKEND_BAREMETAL) ||
				(DevInst->Backend->Type == XAIE_IO_BACKEND_SOCKET)) {
			XAie_MemInst* srcMemInst = (XAie_MemInst*)SourceObject;
			u64 sourceObjectDevAddr = XAie_MemGetDevAddr(srcMemInst);
			RC |= (u32)XAie_DmaSetAddrLen(&SourceBufferDescriptor,
						sourceObjectDevAddr, data_size);
		} else {
			RC |= (u32)XAie_DmaSetAddrOffsetLen(&SourceBufferDescriptor,
						(XAie_MemInst*)SourceObject, 0x0, data_size);
		}
	} else {
		RC |= (u32)XAie_DmaSetAddrLen(&SourceBufferDescriptor,
						(u64)(uintptr_t)SourceObject, data_size);
	}

	RC |= (u32)XAie_DmaEnableBd(&SourceBufferDescriptor);
	int sourceBufferID = _XAie_findAvailableBufferID(routingInstance, source);
	RC |= XAie_DmaWriteBd(DevInst, &SourceBufferDescriptor, source, sourceBufferID);
	XAIE_DBG("\n [COMMAND] XAie_DmaWriteBd({%d,%d}, %d)\n",
			source.Col, source.Row, sourceBufferID);
	BDs->sourceBD = sourceBufferID;
	if (RC != XAIE_OK) {
		XAIE_ERROR("Buffer Descriptor programming failed!\n");
		return XAIE_ERR;
	}

	/* Program the buffer descriptor for the destination tile */
	RC |= (u32)XAie_DmaDescInit(DevInst, &DestBufferDescriptor, destination);
	XAIE_DBG("\n [COMMAND] "
		"XAie_DmaDescInit(&DestBufferDescriptor, {%d,%d})\n",
		destination.Col, destination.Row);
	if (_XAie_isShimTile(routingInstance, destination)) {
		if ((DevInst->Backend->Type == XAIE_IO_BACKEND_BAREMETAL) ||
				(DevInst->Backend->Type == XAIE_IO_BACKEND_SOCKET)) {
			XAie_MemInst* destMemInst = (XAie_MemInst*)DestinationObject;
			u64 destObjectDevAddr = XAie_MemGetDevAddr(destMemInst);
			RC |= (u32)XAie_DmaSetAddrLen(&DestBufferDescriptor,
						destObjectDevAddr, data_size);
		} else {
			RC |= (u32)XAie_DmaSetAddrOffsetLen(&DestBufferDescriptor,
						(XAie_MemInst*)DestinationObject, 0x0, data_size);
		}
	} else {
		RC |= (u32)XAie_DmaSetAddrLen(&DestBufferDescriptor,
						(u64)(uintptr_t)DestinationObject, data_size);
	}
	RC |= (u32)XAie_DmaEnableBd(&DestBufferDescriptor);
	int destBufferID = _XAie_findAvailableBufferID(routingInstance, destination);
	RC |= XAie_DmaWriteBd(DevInst, &DestBufferDescriptor, destination, destBufferID);
	XAIE_DBG("\n [COMMAND] XAie_DmaWriteBd({%d,%d}, %d)\n",
			destination.Col, destination.Row, destBufferID);
	BDs->destinationBD = destBufferID;
	/* Check for errors in the process */
	if (RC != XAIE_OK) {
		XAIE_ERROR("Buffer Descriptor programming failed!\n");
		return XAIE_ERR;
	}

	return XAIE_OK;
}

/*****************************************************************************/
/*
 *
 * This API programs moves data between the source and destination tile by
 * issuing appropriate AIE-RT commands.
 *
 * @param	routingInstance: Routing Instance
 * @param	source: Source tile
 * @param	SourceObject: This can either be the source address or source XAie_MemInstance, based on backend
 * @param	data_size: Data size
 * @param	DestinationObject: This can either be the source address or destination XAie_MemInstance, based on backend
 * @param	destination: Destination Tile

 * @return	XAIE_OK is successful.
 *
 * @note		None.
 *
 ******************************************************************************/
AieRC XAie_MoveData(XAie_RoutingInstance *routingInstance, XAie_LocType source, void* SourceObject,
			u32 data_size, void* DestinationObject, XAie_LocType destination)
{
	XAie_DevInst *DevInst = routingInstance->DeviceInstance;
	int RC = XAIE_OK;
	u8 sourceChannelID;
	u8 destChannelID;

	/* Program the buffer descriptors */
	XAie_BDs bufferDescriptors;
	if (_XAie_programBufferDescriptors(routingInstance, source, SourceObject, data_size,
				DestinationObject, destination, &bufferDescriptors) != XAIE_OK) {
		XAIE_DBG("\n[ERROR] FAILURE IN PROGRAMMING BUFFER DESCRIPTORS\n");
		return XAIE_ERR;
	}

	/* Loop through all programmed routes to find the matching source and destination */
	XAie_CoreConstraint* Sourceconstraint = routingInstance->
					CoreConstraintPerCore[source.Col][source.Row];

	XAie_RoutingPath *routingPath = _XAie_findRouteInRouteDB(Sourceconstraint->routesDB,
						source, destination);

	if (routingPath == NULL) {
		XAIE_ERROR("no programmed path found between "
			"Source: {%d %d} and Destination {%d %d}\n",
			source.Col,
			source.Row,
			destination.Col,
			destination.Row);
		return XAIE_ERR;
	}

	sourceChannelID = routingPath->MM2S_portNo;
	destChannelID = routingPath->S2MM_portNo;

	if (_XAie_isShimTile(routingInstance, source))
		sourceChannelID = findAvailableChannelIDforShimTile(routingInstance,
				source, sourceChannelID, true /*HostToAie*/);

	if (_XAie_isShimTile(routingInstance, destination))
		destChannelID = findAvailableChannelIDforShimTile(routingInstance,
				destination, destChannelID, false /*HostToAie*/);

	/* Push BD to DMA Channel Queue for source */
	XAIE_DBG("\n [COMMAND] XAie_DmaChannelPushBdToQueue({%d, %d}, %d, DMA_MM2S, %d)\n",
			source.Col, source.Row, sourceChannelID, bufferDescriptors.sourceBD);
	XAIE_DBG("\n [COMMAND] XAie_DmaChannelEnable({%d,%d}, %d, DMA_MM2S)\n",
			source.Col, source.Row, sourceChannelID);
	RC |= (u32)XAie_DmaChannelPushBdToQueue(DevInst, source, sourceChannelID,
			DMA_MM2S, bufferDescriptors.sourceBD);
	RC |= (u32)XAie_DmaChannelEnable(DevInst, source, sourceChannelID, DMA_MM2S);

	if (RC != XAIE_OK) {
		return XAIE_ERR;
	}

	/* now on the above tile (source tile) program the MM2S signatory */
	XAie_mm2sChannelsInUse MM2S = Sourceconstraint->mm2sChannelsInUse;
	u8 MM2S_Count = MM2S.MM2S_portCount;
	MM2S.MM2S_ports = realloc(MM2S.MM2S_ports, (MM2S_Count + 1) * sizeof(u8));
	MM2S.MM2S_ports[MM2S_Count] = sourceChannelID;

	/* Repeat the same process for destination */
	XAIE_DBG("\n [COMMAND] XAie_DmaChannelPushBdToQueue({%d, %d}, %d, DMA_S2MM, %d)\n",
			destination.Col, destination.Row,
			destChannelID, bufferDescriptors.destinationBD);
	XAIE_DBG("\n [COMMAND] XAie_DmaChannelEnable({%d,%d}, %d, DMA_S2MM)\n",
			destination.Col, destination.Row, destChannelID);
	RC |= (u32)XAie_DmaChannelPushBdToQueue(DevInst, destination, destChannelID,
			DMA_S2MM, bufferDescriptors.destinationBD);
	RC |= (u32)XAie_DmaChannelEnable(DevInst, destination, destChannelID, DMA_S2MM);
	if (RC != XAIE_OK) {
		return XAIE_ERR;
	}

	/* now on the above tile (source tile) program the MM2S signatory */
	XAie_s2mmChannelsInUse S2MM = Sourceconstraint->s2mmChannelsInUse;
	u8 S2MM_Count = S2MM.S2MM_portCount;
	S2MM.S2MM_ports = realloc(S2MM.S2MM_ports, (S2MM_Count + 1) * sizeof(u8));
	S2MM.S2MM_ports[S2MM_Count] = sourceChannelID;

	/* Wait for pending BD for destination */
	if(DevInst->DevProp.DevGen != XAIE_DEV_GEN_AIE2PS) {
		u8 destPendingBDCount = 5;
		while (destPendingBDCount) {
			RC |= (u32)XAie_DmaGetPendingBdCount(DevInst, destination, destChannelID,
					DMA_S2MM, &destPendingBDCount);
		}
		/* for AIE2PS architecture, the developer needs to manually
		 * call XAie_RouteDmaWait to ensure DMA is done.
		 */
	}

	/* Check for errors in the process */
	if (RC != XAIE_OK) {
		return XAIE_ERR;
	}

	/* Reset channel and BD availability for both source and destination */
	_XAie_resetBDAvailability(routingInstance, destination, bufferDescriptors.destinationBD);
	_XAie_resetBDAvailability(routingInstance, source, bufferDescriptors.sourceBD);
	XAIE_DBG("\n [COMMAND] XAie_DmaChannelEnable -> Data routed successfully\n");
	return XAIE_OK;
}

/*****************************************************************************/
/*
 * @brief	Moves data between the source and destination tile.
 *
 * @param	routingInstance: Routing Instance
 * @param 	source: Source Tile
 * @param 	SourceObject: Source XAie_MemInstance
 * @param 	data_size: Data size
 * @param 	DestinationObject: Destination Address
 * @param 	destination: Destination Tile
 * @return	XAIE_OK if successful.
 *
 * @note		None.
 *
 ******************************************************************************/
AieRC XAie_MoveDataExternal2Aie(XAie_RoutingInstance *routingInstance, XAie_LocType source, XAie_MemInst* SourceObject,
			u32 data_size, u32 DestinationObject, XAie_LocType destination) {
	return XAie_MoveData(routingInstance, source, (void*)SourceObject,
			data_size, (void*)(uintptr_t)DestinationObject, destination);
}

/*****************************************************************************/
/*
 * @brief	Moves data between the source and destination tile.
 *
 * @param	routingInstance: Routing Instance
 * @param	source: Source Tile
 * @param	SourceObject: Source Address
 * @param	data_size: Data size
 * @param	DestinationObject: Destination XAie_MemInstance
 * @param	destination: Destination Tile
 * @return	XAIE_OK if successful.
 *
 * @note		None.
 *
 ******************************************************************************/
AieRC XAie_MoveDataAie2External(XAie_RoutingInstance *routingInstance, XAie_LocType source, u32 SourceObject,
			u32 data_size, XAie_MemInst* DestinationObject, XAie_LocType destination) {
	return XAie_MoveData(routingInstance, source, (void*)(uintptr_t)SourceObject,
			data_size, (void*)DestinationObject, destination);
}


/*****************************************************************************/
/*
 *
 * This API finds the shortest legal path (constraint-based) between the source and destination tile
 * through BFS.
 *
 * @param	routingInstance: Routing Instance
 * @param	RouteConstraints: User defiend Route Constraints
 * @param	source: Source Tile
 * @param	destination: Destination Tile
 * @param	path: route path
 * @param	pathLength: path length
 * @return	true if successful.
 *
 * @note		Internal API.
 *
 ******************************************************************************/

static bool _XAie_findShortestPath(XAie_RoutingInstance *routingInstance,
		XAie_RouteConstraints* RouteConstraints, XAie_LocType source,
		XAie_LocType destination, XAie_LocType *path, int *pathLength)
{
	XAie_LocType **Pred;
	bool Ret = true;
	u32 MAX_ROWS = routingInstance->NumRows;
	u32 MAX_COLS = routingInstance->NumCols;
	bool **visited ;
	Queue* queue;
	bool found = false;


	if (source.Col == destination.Col && source.Row == destination.Row) {
		*pathLength = 0;
		// Source and destination are the same
		return true;
	}

	Pred = calloc(MAX_COLS, sizeof(XAie_LocType *));
	if (Pred == NULL)
		return false;

	for (u32 i = 0; i < MAX_COLS; i++) {
		Pred[i] = calloc(MAX_ROWS, sizeof(XAie_LocType));
		if (Pred[i] == NULL) {
			XAIE_ERROR("Pred alloc failed.\n");
			Ret = false;
			goto free_pred;
		}
	}

	visited = (bool**)aligned_alloc(16, MAX_COLS * sizeof(bool*));
	memset(visited, 0, MAX_COLS * sizeof(bool*));

	if (visited == NULL) {
		XAIE_ERROR("findShortestPath backend failed!. Failed to allocate memory!\n");
		Ret = false;
		goto free_pred;
	}

	for (u32 i = 0; i < MAX_COLS; i++) {
		visited[i] = (bool*)aligned_alloc(16, MAX_ROWS * sizeof(bool));

		if (visited[i] == NULL) {
			XAIE_ERROR("findShortestPath backend failed!. "
					"Failed to allocate memory.\n");
			Ret = false;
			goto free_visited;
		}
		memset(visited[i], 0, MAX_ROWS * sizeof(bool));
	}

	XAIE_DBG("Finding shortest path from (%d, %d) to (%d, %d)\n",
			source.Col, source.Row, destination.Col, destination.Row);

	queue = createQueue(MAX_ROWS * MAX_COLS);
	if (queue != NULL) {
		_XAie_enqueue(queue, source);
	} else {
		XAIE_ERROR("findShortestPath backend failed!. Failed to create Queue.\n");
		Ret = false;
		goto free_visited;;
	}

	while (!isEmpty(queue) && !found) {
		XAie_LocType current = _XAie_dequeue(queue);

		/* Directions: North, South, East, West, Self */
		int dRow[] = {1, -1, 0, 0};
		int dCol[] = {0, 0, 1, -1};
		int StrmSwPort[] ={6 /*NORTH*/, 4/*SOUTH*/, 7/*EAST*/, 5/*WEST*/ };
		for (int i = 0; i < 4; i++) {
			XAie_LocType adj = XAie_TileLoc(current.Col + dCol[i],
							current.Row + dRow[i]);

			if (_XAie_isAdjTileValidForCurrTile(routingInstance, RouteConstraints,
						current, adj, StrmSwPort[i], visited)) {
				visited[adj.Col][adj.Row] = true;
				_XAie_enqueue(queue, adj);
				Pred[adj.Col][adj.Row] = current;

				if (adj.Col == destination.Col && adj.Row == destination.Row) {
					/* Check if the path contains all of the whitelisted cores
					 */
					bool allWhitelisted = true;
					XAie_LocType temp = adj;
					while (!(temp.Col == source.Col &&
								temp.Row == source.Row)) {
						if ((RouteConstraints != NULL) &&
							(RouteConstraints->NoOfWhiteListedCores > 0U) &&
							(!_XAie_isTileWhitelisted(temp,
							RouteConstraints->WhiteListedCores,
							RouteConstraints->
							NoOfWhiteListedCores))) {
							allWhitelisted = false;
							break;
						}
						temp = Pred[temp.Col][temp.Row];
					}

					if (allWhitelisted) {
						found = true;
						break;
					}
				}
			}
		}
	}

	if (!found) {
		*pathLength = 0;
		XAIE_DBG("[ERROR] PATH NOT FOUND\n");
		// Path not found
		Ret = false;
		goto free_queue;
	}

	/* Construct and reverse the path */
	*pathLength = 0;
	XAie_LocType at = destination;
	while (!(at.Col == source.Col && at.Row == source.Row)) {
		path[(*pathLength)++] = at;
		at = Pred[at.Col][at.Row];
	}
	path[(*pathLength)++] = source;
	reversePath(path, *pathLength);

	_XAie_drawRoute(routingInstance, path, *pathLength, source, destination);
	XAIE_DBG("Shortest path found. Path length: %d\n", *pathLength);

free_queue:
	if (queue) {
		freeQueue(queue);
		queue = NULL;
	}

free_visited:
	if (visited) {
		for (u32 i = 0; i < MAX_COLS; i++) {
			if (visited[i]) {
				free(visited[i]);
				visited[i] = NULL;
			}
		}
		free(visited);
		visited = NULL;
	}

free_pred:
	if (Pred) {
		for (u32 i = 0; i < MAX_COLS; i++) {
			if (Pred[i]) {
				free(Pred[i]);
				Pred[i] = NULL;
			}
		}
		free(Pred);
		Pred = NULL;
	}
	return Ret;
}

/*****************************************************************************/
/*
 *
 * This API allows the developer to mark a specific core as "Executable" or Not.
 * Consequently when XAie_Run is invoked, all "Executable" cores will be executed
 *
 * @param	routingInstance: Routing Instance
 * @param	tile : Tile to be marked
 * @param	isExecute: to mark the core as executable or not
 * @return	XAIE_OK if successful.
 *
 * @note		None.
 *
 ******************************************************************************/
AieRC XAie_SetCoreExecute(XAie_RoutingInstance *routingInstance, XAie_LocType tile, bool isExecute)
{
	routingInstance->CoreConstraintPerCore[tile.Col][tile.Row]->isCoreExecuting = isExecute;
	return XAIE_OK;
}

AieRC XAie_Run(XAie_RoutingInstance* routingInstance, uint32_t count)
{
	if (routingInstance == NULL) {
		XAIE_ERROR("Invalid arguments!\n");
		return XAIE_INVALID_ARGS;
	}

	for (u32 i = 0; i < count; i++) {
		for (u8 col = 0; col < routingInstance->NumCols; col++) {
			for (u8 row = 0; row < routingInstance->NumRows; row++) {
				XAie_CoreConstraint* coreConstraint = routingInstance->
								CoreConstraintPerCore[col][row];
				if (coreConstraint != NULL && coreConstraint->isCoreExecuting) {
					XAie_CoreEnable(routingInstance->DeviceInstance,
								XAie_TileLoc(col, row));
					XAIE_DBG("\n [PROGRAM] XAie_CoreEnable({%d,%d})\n",
										col, row);
				}
			}
		}
	}

	return XAIE_OK;
}

/*****************************************************************************/
/*
 *
 * This API is to wait until all pending DMA accesses are finished.
 *
 * @param	routingInstance: Routing Instance
 * @param	tile : Tile whose DMA is to be checked
 * @param	isS2MM : check S2MM or MM2S DMA accesses
 * @return	XAIE_OK if successful.
 *
 * @note		None.
 *
 ******************************************************************************/
AieRC XAie_RouteDmaWait(XAie_RoutingInstance *routingInstance, XAie_LocType sourcetile,
			XAie_LocType tile, bool isS2MM)
{
	(void)isS2MM;
	XAie_CoreConstraint* coreConstraint =
		routingInstance->CoreConstraintPerCore[sourcetile.Col][sourcetile.Row];
	XAie_s2mmChannelsInUse S2MM_val __attribute__((unused)) = coreConstraint->s2mmChannelsInUse;
	XAie_mm2sChannelsInUse MM2S_val __attribute__((unused)) = coreConstraint->mm2sChannelsInUse;

	XAie_RoutingPath *routingPath = _XAie_findRouteInRouteDB(coreConstraint->routesDB,
					sourcetile, tile);

	if (routingPath == NULL) {
		XAIE_ERROR("no programmed path found between "
			"Source: {%d %d} and Destination {%d %d}\n",
			sourcetile.Col,
			sourcetile.Row,
			tile.Col,
			tile.Row);
		return XAIE_ERR;
	}

	u32 destChannelID = routingPath->S2MM_portNo;

	if (_XAie_isShimTile(routingInstance, tile))
		destChannelID = findAvailableChannelIDforShimTile(routingInstance,
				tile, destChannelID, false /*HostToAie*/);

	u8 destPendingBDCount = 5;
	while (destPendingBDCount) {
		XAIE_DBG("Pending BD (%d) on port %d. Source {%d, %d}, Destination {%d, %d}\n",
				destPendingBDCount, destChannelID, sourcetile.Col,
				sourcetile.Row, tile.Col, tile.Row);
		XAie_DmaGetPendingBdCount(routingInstance->DeviceInstance, tile,
				destChannelID, DMA_S2MM, &destPendingBDCount);
	}
	return XAIE_OK;
}

/*****************************************************************************/
/*
 *
 * This API is to wait until a particular AIE-Tile's Core has finished executing.
 *
 * @param	routingInstance: Routing Instance
 * @param	tile : Tile whose DMA is to be checked
 * @return	XAIE_OK if successful.
 *
 * @note		None.
 *
 ******************************************************************************/
AieRC XAie_CoreWait(XAie_RoutingInstance *routingInstance, XAie_LocType tile)
{
	AieRC RC = XAIE_OK;
	do {
		RC = XAie_CoreWaitForDone(routingInstance->DeviceInstance, tile, 0);
		XAIE_DBG("Waiting for core {%d, %d} to finish\n", tile.Col, tile.Row);
	} while (RC != XAIE_OK);

	return XAIE_OK;
}

/*****************************************************************************/
/*
 *
 * This API is to allow the developer to specify Host-Edge constraints.
 * Specifically, XAie_HostAieConstraint is a Data structure that contains a
 * mapping between the Map "XAie_ChannelPortMapping {port - channel}""
 * and "shim Tile (column no)".

 * @param	routingInstance: Routing Instance
 * @param	UserResCon : User defined HostAie constraint data structure
 * @return	XAIE_OK if successful.
 *
 * @note		None.
 *
 ******************************************************************************/
AieRC XAie_ConfigHostEdgeConstraints(XAie_RoutingInstance *routingInstance,
				XAie_HostAieConstraint UserResCon)
{
	/* iterate through all the user specified constraints and configure
	 * appropriate constraints on the appropriate SHIM Tiles
	 */
	for (int i = 0; i < UserResCon.ConstraintCount; i++) {
		XAie_RoutingResourceConstraint Constraint = UserResCon.RoutingResourceConstraint[i];
		XAie_LocType CurrTile = XAie_TileLoc(Constraint.column, 0);
		XAie_CoreConstraint* coreConstraint = routingInstance->
					CoreConstraintPerCore[CurrTile.Col][CurrTile.Row];
		if (Constraint.HostToAie) {
			coreConstraint->Host2AIEPorts = Constraint.channelPortMappings;
			coreConstraint->Host2AIEPortCount = Constraint.channelPortMappingCount;
		}
		else {
			coreConstraint->AIE2HostPorts = Constraint.channelPortMappings;
			coreConstraint->AIE2HostPortCount = Constraint.channelPortMappingCount;
		}
	}
	return XAIE_OK;
}

/*****************************************************************************/
/*
 *
 * This API is to allow the developer to reset all pre-configured Host-Edge constraints.

 * @param	routingInstance: Routing Instance
 * @return	XAIE_OK if successful.
 *
 * @note		None.
 *
 ******************************************************************************/
AieRC XAie_ResetHostEdgeConstraints(XAie_RoutingInstance *routingInstance)
{
	/* iterate through all the user specified constraints and configure
	 * appropriate constraints on the appropriate SHIM Tiles
	 */
	for (int i = 0; i < routingInstance->NumCols; i++) {
		XAie_LocType CurrTile = XAie_TileLoc(i, 0);
		XAie_CoreConstraint* coreConstraint = routingInstance->
					CoreConstraintPerCore[CurrTile.Col][CurrTile.Row];
		coreConstraint->Host2AIEPorts = NULL;
		coreConstraint->Host2AIEPortCount = 0;
		coreConstraint->AIE2HostPorts = NULL;
		coreConstraint->AIE2HostPortCount = 0;
	}
	return XAIE_OK;
}

/*****************************************************************************/
/*
 *
 * This API is to allow the developer to create a route between a source
 * and destination tile. If a route already exists between the two, the API fails!

 * @param	routingInstance: Routing Instance
 * @param	RouteConstraints: Route Constrains
 * @param	source: source tile
 * @param	destination: Destination tile

 * @return	XAIE_OK if successful.
 *
 * @note		None.
 *
 ******************************************************************************/
AieRC XAie_Route(XAie_RoutingInstance *routingInstance,  XAie_RouteConstraints* RouteConstraints,
		XAie_LocType source, XAie_LocType destination)
{
	int pathLength = 0;
	XAie_CoreConstraint* Sourceconstraint = routingInstance->
					CoreConstraintPerCore[source.Col][source.Row];
	if (Sourceconstraint == NULL) {
		XAIE_ERROR("XAie_Route backend failed!.\n");
		return XAIE_ERR;
	}

	if (NULL != _XAie_findRouteInRouteDB(Sourceconstraint->routesDB, source, destination)) {
		XAIE_ERROR("XAie_Route backend failed!. Route has already been "
				"programmed between source and destination\n");
		return XAIE_ERR;
	}

	XAie_LocType *path = (XAie_LocType *)aligned_alloc(16,
			routingInstance->NumRows * routingInstance->NumCols * sizeof(XAie_LocType));
	if (path == NULL) {
		XAIE_ERROR("XAie_Route backend Failed!. Memory allocation failed\n");
		return XAIE_ERR;
	}

	if (!_XAie_findShortestPath(routingInstance, RouteConstraints, source,
					destination, path, &pathLength)) {
		free(path);
		XAIE_ERROR("XAie_Route backend Failed!. "
			"Unable to find path between [%d %d] and [%d %d].\n",
			source.Col, source.Row, destination.Col, destination.Row);
		return XAIE_ERR;
	}

	/* Perform routing along the found path */
	if(_XAie_performRoutingOnPath(routingInstance, source, destination,
					path, pathLength) != XAIE_OK) {
		free(path);
		XAIE_ERROR("XAie_Route backend Failed!. "
			"Unable to program path between [%d %d] and [%d %d].\n",
			source.Col, source.Row, destination.Col, destination.Row);
		return XAIE_ERR;
	}

	if ((!_XAie_isShimTile(routingInstance, source)) &&
				(!_XAie_isMemTile(routingInstance, source)))
		XAie_SetCoreExecute(routingInstance, source, true);

	if ((!_XAie_isShimTile(routingInstance, destination)) &&
				(!_XAie_isMemTile(routingInstance, destination)))
		XAie_SetCoreExecute(routingInstance, destination, true);

	return XAIE_OK;
}

/*****************************************************************************/
/*
 *
 * This API is to check if this tile is whitelisted or not. This is called
 * from XAie_Route.
 *
 * If developer specifies whitelisted tiles through the Route Constraints,
 * XAie_Route will only pick a route between the source and destination that contains
 * all the whitelisted cores when exploring routes using BFS.

 * @param	tile: Current Tile
 * @param	whitelist: Array of whitelisted tiles
 * @param	whitelistCount: Number of whitelisted tiles

 * @return	true if tile is whitelisted, or else false.
 *
 * @note		Internal API.
 *
 ******************************************************************************/
static bool _XAie_isTileWhitelisted(XAie_LocType CurrTile,
			XAie_LocType *whitelist, u16 whitelistCount)
{
	for (u16 i = 0; i < whitelistCount; i++) {
		if (CurrTile.Col == whitelist[i].Col && CurrTile.Row == whitelist[i].Row) {
			return true;
		}
	}
	return false;
}

/*****************************************************************************/
/*
 *
 * This API is to check if the Current tile is Blacklisted or not. This is called
 * from XAie_Route. If CurrTile is blacklisted, XAie_Route avoid this path.
 *
 * @param	tile: Current Tile
 * @param	initialConstraints: Rotue Constraints

 * @return	true if tile is blacklisted, or else false.
 *
 * @note		Internal API.
 *
 ******************************************************************************/
static bool _XAie_isTileBlackListed(XAie_LocType CurrTile,
				XAie_RouteConstraints* initialConstraints)
{
	if (initialConstraints != NULL) {
		XAie_LocType* BlackListedCores = initialConstraints->BlackListedCores;
		u16 NoOfBlackListedCores =  initialConstraints->NoOfBlackListedCores;
		u8 CurrTileCol = CurrTile.Col;
		u8 CurrTileRow = CurrTile.Row;

		for (int i = 0 ; i < NoOfBlackListedCores; i++) {
			u8 col = BlackListedCores[i].Col;
			u8 row = BlackListedCores[i].Row;

			if ((CurrTileCol == col) && (CurrTileRow == row))
				return true;
		}
	}
	return false;
}

/*****************************************************************************/
/*
 *
 * This API is to initialize the Routing Instance Data structure.
 * This Data structure is basically the routing DB and holds its state.
 * This Data structure is populated from the information gathered from Device Instance.
 *
 * @param	DevInst: Device Instance

 * @return	Pointer to the Routing Instance Data structure.
 *
 * @note		None.
 *
 ******************************************************************************/
/* DeviceInstance[routing information] */
XAie_RoutingInstance* XAie_InitRoutingHandler(XAie_DevInst *DevInst)
{
	XAie_RoutingInstance* RoutingInstance;
	RoutingInstance = (XAie_RoutingInstance*)aligned_alloc(16, sizeof(XAie_RoutingInstance));
	if (RoutingInstance == NULL) {
		XAIE_ERROR("XAie_InitRoutingHandler backend failed!. Failed to allocate memory\n");
		return NULL;
	}

	RoutingInstance->DeviceInstance = DevInst;
	RoutingInstance->NumRows = DevInst->NumRows;
	RoutingInstance->NumCols = DevInst->NumCols;

	/* based on Device characteristics, setup the core constraints for each core. */
	u8 NumCols = DevInst->NumCols;
	u8 NumRows = DevInst->NumRows;
	u8 MemTileStart = DevInst->MemTileRowStart;
	u8 MemTileEnd = 0;
	if (MemTileStart > 0)
		MemTileEnd = MemTileStart +  DevInst->MemTileNumRows - 1;

	u8 AieTileRowStart = DevInst->AieTileRowStart;
	u8 AieTileRowEnd = AieTileRowStart + DevInst->AieTileNumRows;

	RoutingInstance->NumRows = NumRows;
	RoutingInstance->NumCols = NumCols;

	/* Allocate memory for each row of CoreConstraintPerCore */
	RoutingInstance->CoreConstraintPerCore = (XAie_CoreConstraint***)aligned_alloc(16,
						NumCols * sizeof(XAie_CoreConstraint**));

	if (RoutingInstance->CoreConstraintPerCore == NULL) {
		XAIE_ERROR("XAie_InitRoutingHandler backend failed!. "
			"Failed to allocate memory to CoreConstraintPerCore\n");
		return NULL;
	}

	for (u8 col = 0; col < NumCols; col++) {
		/* Allocate memory for each column in a col */
		RoutingInstance->CoreConstraintPerCore[col] = (XAie_CoreConstraint**)aligned_alloc
						(16, NumRows * sizeof(XAie_CoreConstraint*));
		if (RoutingInstance->CoreConstraintPerCore[col] == NULL) {
			XAIE_ERROR("XAie_InitRoutingHandler backend failed!. "
				"Failed to allocate memory to CoreConstraintPerCore[%d] \n", col);
			return NULL;
		}

		for (u8 row = 0; row < NumRows; row++) {
			/* Allocate memory for each core constraint */
			RoutingInstance->CoreConstraintPerCore[col][row] =
				(XAie_CoreConstraint*)aligned_alloc(16,sizeof(XAie_CoreConstraint));
		}
	}

	XAie_ChannelPortMapping* defaultHost2AIEPortChannelMapping = (XAie_ChannelPortMapping *)aligned_alloc
						(16, 2*sizeof(XAie_ChannelPortMapping));
	XAie_ChannelPortMapping* defaultAIE2HostPortChannelMapping = (XAie_ChannelPortMapping *)aligned_alloc
						(16, 2*sizeof(XAie_ChannelPortMapping));

	defaultHost2AIEPortChannelMapping[0].port = 3;
	defaultHost2AIEPortChannelMapping[0].channel = 0;
	defaultHost2AIEPortChannelMapping[0].availability = true;
	defaultHost2AIEPortChannelMapping[1].port = 7;
	defaultHost2AIEPortChannelMapping[1].channel = 1;
	defaultHost2AIEPortChannelMapping[1].availability = true;

	if(DevInst->DevProp.DevGen == XAIE_DEV_GEN_AIE2PS) {
		defaultAIE2HostPortChannelMapping[0].port = 1;
		defaultAIE2HostPortChannelMapping[0].channel = 0;
		defaultAIE2HostPortChannelMapping[0].availability = true;
		defaultAIE2HostPortChannelMapping[1].port = 3;
		defaultAIE2HostPortChannelMapping[1].channel = 1;
		defaultAIE2HostPortChannelMapping[1].availability = true;
	}
	else {
		defaultAIE2HostPortChannelMapping[0].port = 2;
		defaultAIE2HostPortChannelMapping[0].channel = 0;
		defaultAIE2HostPortChannelMapping[0].availability = true;
		defaultAIE2HostPortChannelMapping[1].port = 3;
		defaultAIE2HostPortChannelMapping[1].channel = 1;
		defaultAIE2HostPortChannelMapping[1].availability = true;
	}

	for (u8 row = 0; row < NumRows; row++)
		for (u8 col = 0; col < NumCols; col++) {
			XAie_LocType CurrTile __attribute__((unused)) = XAie_TileLoc(col, row);
			bool isShimTileRow = (row == DevInst->ShimRow) ? true : false;
			bool isMemTileRow = ((row >= MemTileStart) &&
						(row <= MemTileEnd)) ? true: false;
			bool isAieTile = ((row >= AieTileRowStart) &&
						(row <= AieTileRowEnd)) ? true: false;

			XAie_CoreConstraint *CurrCoreConstraint = RoutingInstance->
							CoreConstraintPerCore[col][row];

			/* there are 4 channels. 2-S2MM and 2-MM2S, initialize all of thes
			 * 11channels to available [1]. Rest unavailable [0]
			 */
			/* sometimes if no memtile, then memtile row by default would be 0
			 * --> This is not memtile then, its shim tile
			 */
			if (isMemTileRow && !isShimTileRow) {
				CurrCoreConstraint->isAutoConfigured = false;
				CurrCoreConstraint->tile_type = 1;
				/* set 6 bits and rest 0 */
				CurrCoreConstraint->S2MM_State = 0x3F;
				/* set 6 bits and rest 0 */
				CurrCoreConstraint->MM2S_State = 0x3F;
				CurrCoreConstraint->ShimS2MM_State = 0x0;
				CurrCoreConstraint->ShimMM2S_State = 0x0;
				/* 48 BDs are available */
				CurrCoreConstraint->BDState = 0xFFFFFFFFFFFF;
				/* only north and south streams accessible */
				CurrCoreConstraint->DirSupported = (XAie_StreamDirSupported)
						{.canMoveNorth = 1, .canMoveSouth = 1,
						.canMoveEast = 0, .canMoveWest = 0};
				/* no east streaming */
				CurrCoreConstraint->SlaveEast = 0x0;
				/* no west streaming */
				CurrCoreConstraint->SlaveWest = 0x0;
				/* 6 slave ports on south */
				CurrCoreConstraint->SlaveSouth = 0x3F;
				/* 4 slave ports on north */
				CurrCoreConstraint->SlaveNorth = 0xF;

				/* no east streaming */
				CurrCoreConstraint->MasterEast = 0x0;
				/* no west streaming */
				CurrCoreConstraint->MasterWest = 0x0;
				/* 4 slave ports on south */
				CurrCoreConstraint->MasterSouth = 0xF;
				CurrCoreConstraint->MasterNorth = 0x3F;

				/* Not applicable. Only applicable to SHIM TILES */
				CurrCoreConstraint->Host2AIEPorts = 0x0;
				CurrCoreConstraint->Host2AIEPortCount = 0;
				CurrCoreConstraint->AIE2HostPorts = 0x0;
				CurrCoreConstraint->AIE2HostPortCount = 0;
				CurrCoreConstraint->routesDB = NULL;
				CurrCoreConstraint->s2mmChannelsInUse.S2MM_ports = NULL;
				CurrCoreConstraint->s2mmChannelsInUse.S2MM_portCount = 0;
				CurrCoreConstraint->mm2sChannelsInUse.MM2S_ports = NULL;
				CurrCoreConstraint->mm2sChannelsInUse.MM2S_portCount = 0;
				CurrCoreConstraint->isCoreExecuting = false;
			}
			else if (isAieTile) {
				CurrCoreConstraint->isAutoConfigured = false;
				CurrCoreConstraint->tile_type = 2;
				/* set 2 bits and rest 0 */
				CurrCoreConstraint->S2MM_State = 0x3;
				/* set 2 bits and rest 0 */
				CurrCoreConstraint->MM2S_State = 0x3;
				CurrCoreConstraint->ShimS2MM_State = 0x0;
				CurrCoreConstraint->ShimMM2S_State = 0x0;
				/* 16 BDs are available */
				CurrCoreConstraint->BDState = 0xFFFF;
				/* north, south, west and east streams accessible */
				CurrCoreConstraint->DirSupported = (XAie_StreamDirSupported)
					{.canMoveNorth = 1, .canMoveSouth = 1,
					.canMoveEast = 1, .canMoveWest = 1};

				/* 4 ports on the EAST */
				CurrCoreConstraint->SlaveEast = 0xF;
				/* 4 ports on the WEST */
				CurrCoreConstraint->SlaveWest = 0xF;
				/* 6 slave ports on south */
				CurrCoreConstraint->SlaveSouth = 0x3F;
				/* 4 slave ports on north */
				CurrCoreConstraint->SlaveNorth = 0xF;

				/* 4 ports on the EAST */
				CurrCoreConstraint->MasterEast = 0xF;
				/* 4 ports on the WEST */
				CurrCoreConstraint->MasterWest = 0xF;
				/* 4 slave ports on south */
				CurrCoreConstraint->MasterSouth = 0xF;
				/* 6 slave ports on north */
				CurrCoreConstraint->MasterNorth = 0x3F;

				/* Not applicable. Only applicable to SHIM TILES */
				CurrCoreConstraint->Host2AIEPorts = 0x0;
				CurrCoreConstraint->Host2AIEPortCount = 0;
				CurrCoreConstraint->AIE2HostPorts = 0x0;
				CurrCoreConstraint->AIE2HostPortCount = 0;
				CurrCoreConstraint->routesDB = NULL;
				CurrCoreConstraint->s2mmChannelsInUse.S2MM_ports = NULL;
				CurrCoreConstraint->s2mmChannelsInUse.S2MM_portCount = 0;
				CurrCoreConstraint->mm2sChannelsInUse.MM2S_ports = NULL;
				CurrCoreConstraint->mm2sChannelsInUse.MM2S_portCount = 0;
				CurrCoreConstraint->isCoreExecuting = false;
			}
			else if (isShimTileRow) {
				CurrCoreConstraint->isAutoConfigured = false;
				CurrCoreConstraint->tile_type = 0;
				/* 2 S2MM channels? */
				CurrCoreConstraint->S2MM_State = 0x3;
				/* 2 MM2S channels? */
				CurrCoreConstraint->MM2S_State = 0x3;
				CurrCoreConstraint->ShimS2MM_State = 0x3;
				CurrCoreConstraint->ShimMM2S_State = 0x3;
				/* 16 BDs are available */
				CurrCoreConstraint->BDState = 0xFFFF;
				/* north, south, west and east streams accessible */
				CurrCoreConstraint->DirSupported = (XAie_StreamDirSupported)
					{.canMoveNorth = 1, .canMoveSouth = 1,
					.canMoveEast = 1, .canMoveWest = 1};

				/* 4 ports on the EAST */
				CurrCoreConstraint->SlaveEast = 0xF;
				/* 4 ports on the WEST */
				CurrCoreConstraint->SlaveWest = 0xF;
				/* SHIM tiles dont stream down */
				CurrCoreConstraint->SlaveSouth = 0x0;
				/* 4 slave ports on north */
				CurrCoreConstraint->SlaveNorth = 0xF;

				/* 4 ports on the EAST */
				CurrCoreConstraint->MasterEast = 0xF;
				/* 4 ports on the WEST */
				CurrCoreConstraint->MasterWest = 0xF;
				/* SHIM tiles dont stream down */
				CurrCoreConstraint->MasterSouth = 0x0;
				/* 6 slave ports on north */
				CurrCoreConstraint->MasterNorth = 0x3F;

				/* DEFAULT: ports 3 and 7 are available by default to
				 * move data from SHIM DMA to AIE Tile */
				CurrCoreConstraint->Host2AIEPorts =
							defaultHost2AIEPortChannelMapping;
				/* DEFAULT: 2 channels available at 0 and 1 */
				CurrCoreConstraint->Host2AIEPortCount = 2;
				CurrCoreConstraint->AIE2HostPorts =
							defaultAIE2HostPortChannelMapping;
				CurrCoreConstraint->AIE2HostPortCount = 2;
				CurrCoreConstraint->routesDB = NULL;
				CurrCoreConstraint->s2mmChannelsInUse.S2MM_ports = NULL;
				CurrCoreConstraint->s2mmChannelsInUse.S2MM_portCount = 0;
				CurrCoreConstraint->mm2sChannelsInUse.MM2S_ports = NULL;
				CurrCoreConstraint->mm2sChannelsInUse.MM2S_portCount = 0;
				CurrCoreConstraint->isCoreExecuting = false;
			}

			CurrCoreConstraint->AllChannelsInUse = false;
			CurrCoreConstraint->AllBDsareInUse = false;
		}
	return RoutingInstance;
}

/*****************************************************************************/
/*
 *
 * This API is to free the routing instance Data structure and all the other
 * relevant linked list structures it references internally as members.
 *
 * @param	RoutingInstance: Routing Instance DB

 * @return	void.
 *
 * @note		None.
 *
 ******************************************************************************/
void XAieRoutingInstance_free(XAie_RoutingInstance* RoutingInstance)
{
	if (RoutingInstance == NULL) {
		return;
	}

	u8 NumCols = RoutingInstance->NumCols;
	u8 NumRows = RoutingInstance->NumRows;

	/* Iterate over each column */
	for (u8 col = 0; col < NumCols; col++) {
		if (RoutingInstance->CoreConstraintPerCore[col] != NULL) {
			/* Iterate over each row in the current column */
			for (u8 row = 0; row < NumRows; row++) {
				XAie_CoreConstraint* coreConstraint = RoutingInstance->
								CoreConstraintPerCore[col][row];
				/* Free each XAie_CoreConstraint */
				if (coreConstraint != NULL) {
					/* Free the members of XAie_CoreConstraint */
					if (coreConstraint->routesDB != NULL) {
						_XAie_freeXAieProgrammedRoutes
								(coreConstraint->routesDB);
						coreConstraint->routesDB = NULL;
					}
					free(coreConstraint);
					RoutingInstance->CoreConstraintPerCore[col][row] = NULL;
				}
			}
			/* Free the row array */
			free(RoutingInstance->CoreConstraintPerCore[col]);
			RoutingInstance->CoreConstraintPerCore[col] = NULL;
		}
	}
	/* Free the column array */
	free(RoutingInstance->CoreConstraintPerCore);
	RoutingInstance->CoreConstraintPerCore = NULL;
	return;
}

/*****************************************************************************/
/*
 *
 * Given a Route Path, this API adds it to the programmed routes data structure
 *
 * @param	head: Reference to the head of Programmed Routes linked list
 * @param	routePath: Route Path

 * @return	void
 *
 * @note		Internal API.
 *
 ******************************************************************************/
static void _XAie_addRouteInRoutesDB(XAie_ProgrammedRoutes** head, XAie_RoutingPath* routePath)
{
	XAIE_DBG("Adding next route\n");
	XAie_ProgrammedRoutes* newNode = (XAie_ProgrammedRoutes*)aligned_alloc
						(16, sizeof(XAie_ProgrammedRoutes));
	if (newNode == NULL) {
		XAIE_DBG("Error creating a new Programmed Route\n");
		return;
	}
	newNode->routePath = routePath;
	newNode->nextRoute = *head;
	*head = newNode;
}

/*****************************************************************************/
/*
 *
 * Given a source and destination tile, this looks through the Programmed routes data structure
 * to find and return the appropriate Routing Path data structure
 *
 * @param	routesDB: Programmed routes
 * @param	source: Source Tile
 * @param	destination: Destination Tile
 * @return	RoutingPath data structure
 *
 * @note		Internal API.
 *
 ******************************************************************************/

static XAie_RoutingPath* _XAie_findRouteInRouteDB(XAie_ProgrammedRoutes* routesDB,
			XAie_LocType source, XAie_LocType destination)
{
	if (routesDB == NULL)
		return NULL;
	XAie_ProgrammedRoutes* CurrRoute = routesDB;
	while ((CurrRoute != NULL) &&  (CurrRoute->routePath != NULL)){
		if ((CurrRoute->routePath->source.Col == source.Col &&
				CurrRoute->routePath->source.Row == source.Row)
				&& (CurrRoute->routePath->destination.Col == destination.Col
				&& CurrRoute->routePath->destination.Row == destination.Row)){
			/* Route founds */
			return CurrRoute->routePath;
		}
		CurrRoute = CurrRoute->nextRoute;
	}
	/* Route not found */
	return NULL;
}

/*****************************************************************************/
/*
 *
 * Give the head to the routing step data structure, this API frees all the steps
 * in the routingStep linked-list.
 *
 * @param	step: Head to routing steps linkedlist
 * @return	void
 *
 * @note		Internal API.
 *
 ******************************************************************************/
static void _XAie_FreeRoutingSteps(XAie_RoutingStep* step) {
	while (step != NULL) {
		XAie_RoutingStep* temp = step;
		step = step->next;
		free(temp);
	}
}

/*****************************************************************************/
/*
 *
 * Give a source and destination tile, this routing API find the appropriate route
 * (if it exists) within the programmed routes data structure and deletes the
 * appropriate node in the linked list
 *
 * @param	head: Head to Programmed Routes Linked List
 * @param	source: Source Tile
 * @param	destination: Destination Tile
 * @return	void
 *
 * @note		Internal API.
 *
 ******************************************************************************/
static void _XAie_FreeRouteFromRoutesDB(XAie_ProgrammedRoutes** head,
		XAie_LocType source, XAie_LocType destination)
{
	XAie_ProgrammedRoutes *temp = *head, *prev = NULL;
	while (temp != NULL && !((temp->routePath->source.Col == source.Col &&
				temp->routePath->source.Row == source.Row) &&
				(temp->routePath->destination.Col == destination.Col &&
				 temp->routePath->destination.Row == destination.Row))) {
		prev = temp;
		temp = temp->nextRoute;
	}
	/* Node not found */
	if (temp == NULL)
		return;
	/* Node is head */
	if (prev == NULL) {
		*head = temp->nextRoute;
	} else {
		prev->nextRoute = temp->nextRoute;
	}
	free(temp);
}

/*****************************************************************************/
/*
 *
 * Given a head of the programmed routes Data structure, this API frees all the
 * appropriate references
 *
 * @param	headRoute: Programmed Route head reference
 * @return	void
 *
 * @note		Internal API.
 *
 ******************************************************************************/
static void _XAie_freeXAieProgrammedRoutes(XAie_ProgrammedRoutes* headRoute)
{
	while (headRoute != NULL) {
		XAie_ProgrammedRoutes* nextRoute = headRoute->nextRoute;
		/* Free the routing path associated with this route */
		XAie_RoutingPath* path = headRoute->routePath;
		if (path != NULL) {
			/* Free all steps in the path */
			XAie_RoutingStep* step = path->nextStep;
			while (step != NULL) {
				XAie_RoutingStep* nextStep = step->next;
				free(step);
				step = nextStep;
			}
			/* Then free the path itself */
			free(path);
		}
		/* Then free the route node itself */
		free(headRoute);
		headRoute = nextRoute;
	}
}

/* XAIE_FEATURE_ROUTING_ENABLE */
#endif
/** @} */
