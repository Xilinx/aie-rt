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
* 1.1   Tejus   06/10/2020 Add ess simulation backend.
* 1.2   Tejus   06/10/2020 Add cdo backend.
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xaie_io.h"

/************************** Constant Definitions *****************************/
#if defined (__AIEMETAL__)
	#define XAIE_DEFAULT_BACKEND XAIE_IO_BACKEND_METAL
#elif defined (__AIESIM__)
	#define XAIE_DEFAULT_BACKEND XAIE_IO_BACKEND_SIM
#elif defined (__AIECDO__)
	#define XAIE_DEFAULT_BACKEND XAIE_IO_BACKEND_CDO
#else
	#define XAIE_DEFAULT_BACKEND XAIE_IO_BACKEND_DEBUG
#endif

/************************** Variable Definitions *****************************/
const extern XAie_Backend MetalBackend;
const extern XAie_Backend SimBackend;
const extern XAie_Backend CdoBackend;
const extern XAie_Backend DebugBackend;

const static XAie_Backend *IOBackend[XAIE_IO_BACKEND_MAX] =
{
	&MetalBackend,
	&SimBackend,
	&CdoBackend,
	&DebugBackend,
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
