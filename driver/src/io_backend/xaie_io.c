/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_io.c
* @{
*
* This file contains the data structures and routines for low level IO
* operations.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   06/09/2020 Initial creation.
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xaie_io.h"

/************************** Constant Definitions *****************************/
#if defined (__AIEMETAL__)
	#define XAIE_DEFAULT_BACKEND XAIE_IO_BACKEND_METAL
#else
	#define XAIE_DEFAULT_BACKEND 0U
#endif

/************************** Variable Definitions *****************************/
const extern XAie_Backend MetalBackend;

const static XAie_Backend *IOBackend[XAIE_IO_BACKEND_MAX] =
{
	&MetalBackend,
};

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This is the api initialize global IO instance. The default IO backend is
* libmetal.
*
* @param	DevInst - Device instance pointer.
*
* @return	XAIE_OK on success and error code on failure.
*
* @note		Internal Only.
*
******************************************************************************/
AieRC XAie_IOInit(XAie_DevInst *DevInst)
{
	AieRC RC;
	const XAie_Backend *Backend = IOBackend[XAIE_DEFAULT_BACKEND];

	RC = Backend->Ops.Init(DevInst);
	if(RC != XAIE_OK) {
		return RC;
	}

	DevInst->Backend = Backend;

	XAieLib_print("LOG: Initialized with backend %d\n", Backend->Type);

	return XAIE_OK;
}

/** @} */
