/******************************************************************************
* Copyright (C) 2024 AMD, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_auto_routing.c
* @{
*
* This file contains the test application for demonstrating the use of high-level
* automatic routing APIs.
*
* This application calls contains several submodules. Each submodule first
* initializes a route between a source and a destination tile using XAie_Route.
* Then random data is put onto the source tile, following which this data is
* moved to the destination tile using XAie_Move API. For correctness, the data
* received at the destination tile is compared against the data at the source
* tile.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     			Date     Changes
* ----- ------------------  -------- -----------------------------------------------------
* 1.0   Arunkumar Bhattar   02/20/2024  Initial changes
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include <stdlib.h>
#include <xaiengine.h>
#include "hw_config.h"

/************************** Constant Definitions *****************************/
#define DATA_SIZE 32

/*****************************************************************************/
/**
*
* This is to test routing between two AIE tiles.
*
* @param	None.
*
* @return	0 on success and error code on failure.
*
* @note		None.
*
*******************************************************************************/
int test_aie_auto_routing_aie2aie_1(XAie_DevInst *DevInst)
{
	AieRC RC = XAIE_OK;
	XAie_RoutingInstance* routingInstance;
	uint32_t input[DATA_SIZE];
	uint32_t output[DATA_SIZE];

	routingInstance = XAie_InitRoutingHandler(DevInst);
	XAie_Route(routingInstance, NULL, XAie_TileLoc(2,3) /* Source*/,
			XAie_TileLoc(4,3) /* destination*/);

	// step 1: generate random data
	for(uint8_t i = 0U; i < DATA_SIZE; i++) {
		input[i] = rand() % 127;
	}
	// step 2: write data to source tile
	RC = XAie_DataMemBlockWrite(DevInst, XAie_TileLoc(2,3), 0x2000,
			(void *)input, sizeof(uint32_t) * DATA_SIZE);


	// step 3: move data to destination tile
	XAie_MoveData(routingInstance,  XAie_TileLoc(2,3) /* Source*/,
			(void*)(0x2000), DATA_SIZE*sizeof(uint32_t),
			(void*)(0x2000), XAie_TileLoc(4,3) /* destination*/);

#if AIE_GEN == XAIE_DEV_GEN_AIE2PS
	//Wait until the data transfer completes.
	XAie_RouteDmaWait(routingInstance, XAie_TileLoc(2,3), XAie_TileLoc(4,3));
#endif
	// step 4: read data from destination tile
	RC |= XAie_DataMemBlockRead(DevInst, XAie_TileLoc(4,3),  0x2000,
			 (void*)output, DATA_SIZE * sizeof(uint32_t));

	// step 5 validate data
	for(uint8_t i = 0U; i < DATA_SIZE; i++) {
		if (input[i] != output[i]) {
			return -1;
        }
	}

	// step 6 cleanup
	XAie_DeRoute(routingInstance, XAie_TileLoc(2,3) /* Source*/,
			XAie_TileLoc(4,3) /* destination*/, true);

	printf("AIE Auto Routing [Test-1 AIE2AIE] PASSED!.\n");

	return 0;
}

/*****************************************************************************/
/**
*
* This is to test hop routing between two AIE tiles.
*
* @param	None.
*
* @return	0 on success and error code on failure.
*
* @note		None.
*
*******************************************************************************/
int test_aie_auto_routing_aie2aie_2(XAie_DevInst *DevInst)
{
	AieRC RC = XAIE_OK;
	XAie_RoutingInstance* routingInstance;
	uint32_t input[DATA_SIZE];
	uint32_t output[DATA_SIZE];

	routingInstance = XAie_InitRoutingHandler(DevInst);
	XAie_Route(routingInstance, NULL, XAie_TileLoc(2,3) /* Source*/,
			XAie_TileLoc(3,3) /* destination*/);

	XAie_Route(routingInstance, NULL, XAie_TileLoc(3,3) /* Source*/,
			XAie_TileLoc(4,3) /* destination*/);

	// step 1: generate random data
	for(uint8_t i = 0U; i < DATA_SIZE; i++) {
		input[i] = rand() % 127;
	}
	// step 2: write data to source tile
	RC = XAie_DataMemBlockWrite(DevInst, XAie_TileLoc(2,3), 0x2000,
			(void *)input, sizeof(uint32_t) * DATA_SIZE);


	// step 3: move data to destination tile
	XAie_MoveData(routingInstance,  XAie_TileLoc(2,3) /* Source*/,
			(void*)(0x2000), DATA_SIZE*sizeof(uint32_t),
			(void*)(0x2000), XAie_TileLoc(3,3) /* destination*/);

#if AIE_GEN == XAIE_DEV_GEN_AIE2PS
	//Wait until the data transfer completes.
	XAie_RouteDmaWait(routingInstance, XAie_TileLoc(2,3), XAie_TileLoc(3,3));
#endif
	XAie_MoveData(routingInstance,  XAie_TileLoc(3,3) /* Source*/,
			(void*)(0x2000), DATA_SIZE*sizeof(uint32_t),
			(void*)(0x2000), XAie_TileLoc(4,3) /* destination*/);

#if AIE_GEN == XAIE_DEV_GEN_AIE2PS
	//Wait until the data transfer completes.
	XAie_RouteDmaWait(routingInstance, XAie_TileLoc(3,3), XAie_TileLoc(4,3));
#endif
	// step 4: read data from destination tile
	RC |= XAie_DataMemBlockRead(DevInst, XAie_TileLoc(4,3),  0x2000,
			 (void*)output, DATA_SIZE * sizeof(uint32_t));

	// step 5 validate data
	for(uint8_t i = 0U; i < DATA_SIZE; i++) {
		if (input[i] != output[i]) {
			return -1;
        }
	}

	// step 6 cleanup
	XAie_DeRoute(routingInstance, XAie_TileLoc(2,3) /* Source*/,
				XAie_TileLoc(3,3) /* destination*/, true);

	XAie_DeRoute(routingInstance, XAie_TileLoc(3,3) /* Source*/,
				XAie_TileLoc(4,3) /* destination*/, true);

	printf("AIE Auto Routing [Test-2 AIE2AIE with Hop] PASSED!.\n");

	return 0;
}

/*****************************************************************************/
/**
*
* This is to test loopback routing between two AIE tiles.
*
* @param	None.
*
* @return	0 on success and error code on failure.
*
* @note		None.
*
*******************************************************************************/
int test_aie_auto_routing_aie2aie_3(XAie_DevInst *DevInst)
{
	AieRC RC = XAIE_OK;
	XAie_RoutingInstance* routingInstance;
	uint32_t input[DATA_SIZE];
	uint32_t output[DATA_SIZE];

	routingInstance = XAie_InitRoutingHandler(DevInst);
	XAie_Route(routingInstance, NULL, XAie_TileLoc(2,3) /* Source*/,
			XAie_TileLoc(3,3) /* destination*/);

	XAie_Route(routingInstance, NULL, XAie_TileLoc(3,3) /* Source*/,
			XAie_TileLoc(2,3) /* destination*/);

	// step 1: generate random data
	for(uint8_t i = 0U; i < DATA_SIZE; i++) {
		input[i] = rand() % 127;
	}
	// step 2: write data to source tile
	RC = XAie_DataMemBlockWrite(DevInst, XAie_TileLoc(2,3), 0x2000,
			(void *)input, sizeof(uint32_t) * DATA_SIZE);


	// step 3: move data to destination tile
	XAie_MoveData(routingInstance,  XAie_TileLoc(2,3) /* Source*/,
			(void*)(0x2000), DATA_SIZE*sizeof(uint32_t),
			(void*)(0x2000), XAie_TileLoc(3,3) /* destination*/);

#if AIE_GEN == XAIE_DEV_GEN_AIE2PS
	//Wait until the data transfer completes.
	XAie_RouteDmaWait(routingInstance, XAie_TileLoc(2,3), XAie_TileLoc(3,3));
#endif
	XAie_MoveData(routingInstance,  XAie_TileLoc(3,3) /* Source*/,
			(void*)(0x2000), DATA_SIZE*sizeof(uint32_t),
			(void*)(0x1000), XAie_TileLoc(2,3) /* destination*/);

#if AIE_GEN == XAIE_DEV_GEN_AIE2PS
	//Wait until the data transfer completes.
	XAie_RouteDmaWait(routingInstance, XAie_TileLoc(3,3), XAie_TileLoc(2,3));
#endif
	// step 4: read data from destination tile
	RC |= XAie_DataMemBlockRead(DevInst, XAie_TileLoc(2,3),  0x1000,
			 (void*)output, DATA_SIZE * sizeof(uint32_t));

	// step 5 validate data
	for(uint8_t i = 0U; i < DATA_SIZE; i++) {
		if (input[i] != output[i]) {
			return -1;
        }
	}

	// step 6 cleanup
	XAie_DeRoute(routingInstance, XAie_TileLoc(2,3) /* Source*/,
				XAie_TileLoc(3,3) /* destination*/, true);

	XAie_DeRoute(routingInstance, XAie_TileLoc(3,3) /* Source*/,
				XAie_TileLoc(2,3) /* destination*/, true);

	printf("AIE Auto Routing [Test-3 AIE2AIE loopback] PASSED!.\n");

	return 0;
}

#if DEVICE != 0
/*****************************************************************************/
/**
*
* This is to test routing between SHIM and AIE tiles.
*
* @param	None.
*
* @return	0 on success and error code on failure.
*
* @note		None.
*
*******************************************************************************/
int test_aie_auto_routing_shim2aie(XAie_DevInst *DevInst)
{
	AieRC RC = XAIE_OK;
	XAie_RoutingInstance* routingInstance;
	uint32_t output[DATA_SIZE];


	routingInstance = XAie_InitRoutingHandler(DevInst);
	XAie_Route(routingInstance, NULL, XAie_TileLoc(2,0) /* Source*/,
				 XAie_TileLoc(4,4) /* destination*/	);


	XAie_MemInst* InputData;
	uint32_t* input;
	uint32_t* dma_buf_input;
	uint32_t* DDR_MEM_INPUT;
	InputData = XAie_MemAllocate(DevInst, DATA_SIZE * sizeof(uint32_t), XAIE_MEM_CACHEABLE);
	input = (uint_TYPE *)XAie_MemGetDevAddr(InputData);
	XAie_MemSyncForCPU(InputData);

	// step 1: generate random data
	for(uint8_t i = 0U; i < DATA_SIZE; i++) {
		input[i] = rand() % 127;
	}

	//step 2: attach data to a MemInst
	XAie_MemSyncForDev(InputData);

	// step 3: move data to destination tile
	XAie_MoveData(routingInstance,  XAie_TileLoc(2,0) /* Source*/,
		((void*)(InputData)), DATA_SIZE*sizeof(uint32_t),
		(void*)(0x2000), XAie_TileLoc(4,4) /* destination*/);

	// step 4: read data from destination tile
	RC |= XAie_DataMemBlockRead(DevInst, XAie_TileLoc(4,4),  0x2000,
			 (void*)output, DATA_SIZE * sizeof(uint32_t));

	// step 5 validate data
	for(uint8_t i = 0U; i < DATA_SIZE; i++) {
		if (input[i] != output[i]) {
			return -1;
        }
	}

	//step 6 cleanup
	XAie_DeRoute(routingInstance, XAie_TileLoc(2,0) /* Source*/,
				XAie_TileLoc(4,4) /* destination*/, true);

	printf("AIE Auto Routing [Test-4 SHIM2AIE] PASSED!.\n");

	return 0;
}


/*****************************************************************************/
/**
*
* This is to test routing between AIE and SHIM tile.
*
* @param	None.
*
* @return	0 on success and error code on failure.
*
* @note		None.
*
*******************************************************************************/
int test_aie_auto_routing_aie2shim(XAie_DevInst *DevInst)
{
	AieRC RC = XAIE_OK;
	XAie_RoutingInstance* routingInstance;
	uint32_t input[DATA_SIZE];

	routingInstance = XAie_InitRoutingHandler(DevInst);
	XAie_Route(routingInstance, NULL, XAie_TileLoc(4,4) /* Source*/,
				XAie_TileLoc(35,0) /* destination*/	);

	// step 1: generate random data
	for(uint8_t i = 0U; i < DATA_SIZE; i++) {
		input[i] = rand() % 127;
	}

	// step 2: write data to source tile
	RC = XAie_DataMemBlockWrite(DevInst, XAie_TileLoc(4,4), 0x2000,
			(void *)input, sizeof(uint32_t) * DATA_SIZE);

	XAie_MemInst* OutputData;
	uint32_t* output;
	uint32_t* dma_buf_output;
	uint32_t* DDR_MEM_OUTPUT;
	OutputData = XAie_MemAllocate(DevInst, DATA_SIZE * sizeof(uint32_t), XAIE_MEM_CACHEABLE);
	output = (uint_TYPE *)XAie_MemGetDevAddr(OutputData);
	XAie_MemSyncForCPU(OutputData);

	//step 2: attach data to a MemInst
	XAie_MemSyncForDev(OutputData);

	// step 3: move data to destination tile
	XAie_MoveData(routingInstance,  XAie_TileLoc(4,4) /* Source*/,
		(void*)(0x2000), DATA_SIZE*sizeof(uint32_t),
		((void*)(OutputData)), XAie_TileLoc(35,0) /* destination*/);

	// step 5 validate data
	for(uint8_t i = 0U; i < DATA_SIZE; i++) {
		if (input[i] != output[i]) {
			return -1;
		}
	}

	// step 6 cleanup
	XAie_DeRoute(routingInstance, XAie_TileLoc(4,4) /* Source*/,
				XAie_TileLoc(35,0) /* destination*/, true);

	printf("AIE Auto Routing [Test-5 AIE2SHIM] PASSED!.\n");

	return 0;
}

/*****************************************************************************/
/**
*
* This is to test routing between SHIM and SHIM tile.
*
* @param	None.
*
* @return	0 on success and error code on failure.
*
* @note		None.
*
*******************************************************************************/
int test_aie_auto_routing_shim2shim(XAie_DevInst *DevInst)
{
	AieRC RC = XAIE_OK;
	XAie_RoutingInstance* routingInstance;
	int rc;

	routingInstance = XAie_InitRoutingHandler(DevInst);
	XAie_Route(routingInstance, NULL, XAie_TileLoc(2,0) /* Source*/,
				XAie_TileLoc(35,0) /* destination*/);

	XAie_MemInst* InputData;
	uint32_t* input;
	uint32_t* dma_buf_input;
	uint32_t* DDR_MEM_INPUT;
	InputData = XAie_MemAllocate(DevInst, DATA_SIZE * sizeof(uint32_t), XAIE_MEM_CACHEABLE);
	input = (uint_TYPE *)XAie_MemGetDevAddr(InputData);
	XAie_MemSyncForCPU(InputData);

	// step 1: generate random data
	for(uint8_t i = 0U; i < DATA_SIZE; i++) {
		input[i] = rand() % 127;
	}

	//step 2: attach data to a MemInst
	XAie_MemSyncForDev(InputData);

	XAie_MemInst* OutputData;
	uint32_t* output;
	uint32_t* dma_buf_output;
	uint32_t* DDR_MEM_OUTPUT;
	OutputData = XAie_MemAllocate(DevInst, DATA_SIZE * sizeof(uint32_t), XAIE_MEM_CACHEABLE);
	output = (uint_TYPE *)XAie_MemGetDevAddr(OutputData);
	XAie_MemSyncForCPU(OutputData);

	//step 2: attach data to a MemInst
	XAie_MemSyncForDev(OutputData);

	// step 3: move data to destination tile
	XAie_MoveData(routingInstance,  XAie_TileLoc(2,0) /* Source*/,
		(void*)(InputData), DATA_SIZE*sizeof(uint32_t),
		((void*)(OutputData)), XAie_TileLoc(35,0) /* destination*/);

	// step 5 validate data
	for(uint8_t i = 0U; i < DATA_SIZE; i++) {
		if (input[i] != output[i]) {
			return -1;
		}
	}

	// step 6 cleanup
	XAie_DeRoute(routingInstance, XAie_TileLoc(2,0) /* Source*/,
		 XAie_TileLoc(35,0) /* destination*/, true);

	printf("AIE Auto Routing [Test-6 SHIM2SHIM] PASSED!.\n");

	return 0;
}

#endif

/*****************************************************************************/
/**
*
* This is the main entry point for the AIE driver Auto Routing example.
*
* @param	None.
*
* @return	0 on success and error code on failure.
*
* @note		None.
*
*******************************************************************************/
int test_aie_auto_routing(XAie_DevInst *DevInst)
{
	AieRC RC = XAIE_OK;
	//the below is specific initialization for linux XRT HW backend.

	if (test_aie_auto_routing_aie2aie_1(DevInst) != 0)
	{
		printf("AIE Auto Routing [Test-1 AIE2AIE] FAILED!.\n");
		return XAIE_ERR;
	}

	if (test_aie_auto_routing_aie2aie_2(DevInst) != 0)
	{
		printf("AIE Auto Routing [Test-2 AIE2AIE with Hop] FAILED!.\n");
		return XAIE_ERR;
	}

	if (test_aie_auto_routing_aie2aie_3(DevInst) != 0)
	{
		printf("AIE Auto Routing [Test-3 AIE2AIE loopback] FAILED!.\n");
		return XAIE_ERR;
	}

#if DEVICE != 0
	if (test_aie_auto_routing_shim2aie(DevInst) != 0)
	{
		printf("AIE Auto Routing [Test-4 SHIM2AIE] FAILED!.\n");
		return XAIE_ERR;
	}


	if (test_aie_auto_routing_aie2shim(DevInst) != 0)
	{
		printf("AIE Auto Routing [Test-5 AIE2SHIM] FAILED!.\n");
		return XAIE_ERR;
	}

	if (test_aie_auto_routing_shim2shim(DevInst) != 0)
	{
		printf("AIE Auto Routing [Test-6 SHIM2SHIM] FAILED!.\n");
		return XAIE_ERR;
	}
#endif

	printf("AIE Auto Routing test suite FINISHED.\n");

	return 0;
}
/** @} */
