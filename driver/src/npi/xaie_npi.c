/******************************************************************************
* Copyright (C) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_npi.c
* @{
*
* This file contains routines for operations for accessing AI engine NPI.
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xaie_feature_config.h"
#include "xaie_io_internal.h"
#include "xaie_helper.h"
#include "xaie_npi.h"
#include "xaiegbl.h"

#ifdef XAIE_FEATURE_PRIVILEGED_ENABLE

/************************** Constant Definitions *****************************/
/****************************** Type Definitions *****************************/

/************************** Variable Definitions *****************************/
extern XAie_NpiMod _XAieNpiMod;
extern XAie_NpiMod _XAieMlNpiMod;
/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This is the function to get the NPI module based on the device version
*
* @param	DevInst : Device Instance pointer
*
* @return	NPI module pointer for success, NULL for failure.
*
* @note		None.
*
*******************************************************************************/
static XAie_NpiMod *_XAie_NpiGetMod(XAie_DevInst *DevInst)
{
	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("failed to get NPI module, invalid dev instance.\n");
		return NULL;
	}

	if (DevInst->DevProp.DevGen == XAIE_DEV_GEN_AIE) {
		return &_XAieNpiMod;
	} else if (DevInst->DevProp.DevGen == XAIE_DEV_GEN_AIEML) {
		return &_XAieMlNpiMod;
	}

	XAIE_ERROR("failed to get NPI module, invalid dev version.\n");
	return NULL;
}

/*****************************************************************************/
/**
*
* This is function to set NPI lock
*
* @param	DevInst : AI engine device pointer
* @param	Lock : XAIE_ENABLE to lock, XAIE_DISABLE to unlock
*
* @return	XAIE_OK for success, and error value for failure
*
* @note		This function will not check if NPI module is available as
*		it expects the caller function will do the checking.
*******************************************************************************/
static AieRC _XAie_NpiSetLock(XAie_DevInst *DevInst, u8 Lock)
{
	XAie_NpiMod *NpiMod;
	XAie_BackendNpiWrReq Req;
	XAie_BackendNpiMaskPollReq MPReq;
	u32 LockVal;

	NpiMod = _XAie_NpiGetMod(DevInst);

	if (Lock == XAIE_DISABLE) {
		LockVal = NpiMod->PcsrUnlockCode;
	} else {
		LockVal = 0;
	}

	Req = _XAie_SetBackendNpiWrReq(NpiMod->PcsrLockOff, LockVal);
	XAie_RunOp(DevInst, XAIE_BACKEND_OP_NPIWR32, &Req);

	MPReq = _XAie_SetBackendNpiMaskPollReq(NpiMod->PcsrLockOff, 0U,
			0U, XAIE_NPI_TIMEOUT_US);

	return XAie_RunOp(DevInst, XAIE_BACKEND_OP_NPIMASKPOLL32, &MPReq);
}

/*****************************************************************************/
/**
*
* This is function to mask write to PCSR register
*
* @param	DevInst : AI engine device pointer
* @param	RegVal : Value to write to PCSR register
* @param	Mask : Mask to write to PCSR register
*
* @return	XAIE_OK for success, and error value for failure
*
* @note		Sequence to write PCSR control register is as follows:
*		* unlock the PCSR register
*		* enable PCSR mask from mask register
*		* set the value to PCSR control register
*		* disable PCSR mask from mask register
*		* lock the PCSR register
*******************************************************************************/
static AieRC _XAie_NpiWritePcsr(XAie_DevInst *DevInst, u32 RegVal, u32 Mask)
{
	XAie_NpiMod *NpiMod;
	XAie_BackendNpiWrReq Req;
	XAie_BackendNpiMaskPollReq MPReq;
	AieRC RC;

	NpiMod = _XAie_NpiGetMod(DevInst);
	if (NpiMod == NULL) {
		return XAIE_ERR;
	}

	RC = _XAie_NpiSetLock(DevInst, XAIE_DISABLE);
	if (RC != XAIE_OK) {
		return RC;
	}

	Req = _XAie_SetBackendNpiWrReq(NpiMod->PcsrMaskOff, Mask);
	XAie_RunOp(DevInst, XAIE_BACKEND_OP_NPIWR32, &Req);

	Req = _XAie_SetBackendNpiWrReq(NpiMod->PcsrCntrOff, RegVal);
	XAie_RunOp(DevInst, XAIE_BACKEND_OP_NPIWR32, &Req);

	Req = _XAie_SetBackendNpiWrReq(NpiMod->PcsrMaskOff, 0);
	XAie_RunOp(DevInst, XAIE_BACKEND_OP_NPIWR32, &Req);

	MPReq = _XAie_SetBackendNpiMaskPollReq(NpiMod->PcsrCntrOff, 0U,
			0U, XAIE_NPI_TIMEOUT_US);

	RC = XAie_RunOp(DevInst, XAIE_BACKEND_OP_NPIMASKPOLL32, &MPReq);
	_XAie_NpiSetLock(DevInst, XAIE_ENABLE);

	return RC;
}

/*****************************************************************************/
/**
*
* This is the NPI function to set the SHIM set assert
*
* @param	DevInst : AI engine device pointer
* @param	RstEnable : XAIE_ENABLE to assert reset, and XAIE_DISABLE to
*			    deassert reset.
*
* @return	XAIE_OK for success, and error value for failure
*
* @note		None.
*
*******************************************************************************/
AieRC _XAie_NpiSetShimReset(XAie_DevInst *DevInst, u8 RstEnable)
{
	u32 RegVal, Mask;
	XAie_NpiMod *NpiMod;

	NpiMod = _XAie_NpiGetMod(DevInst);
	if (NpiMod == NULL) {
		return XAIE_ERR;
	}

	Mask = NpiMod->ShimReset.Mask;
	RegVal = XAie_SetField(RstEnable, NpiMod->ShimReset.Lsb, Mask);

	return _XAie_NpiWritePcsr(DevInst, RegVal, Mask);
}

/*****************************************************************************/
/**
*
* This is the NPI function to set the AI engine protect register configuration
*
* @param	DevInst : AI engine partition device pointer
* @param	Req : Request to set the protected registers
*
* @return	XAIE_OK for success, and error value for failure
*
* @note		None.
*
*******************************************************************************/
AieRC _XAie_NpiSetProtectedRegEnable(XAie_DevInst *DevInst,
				    XAie_NpiProtRegReq *Req)
{
	u32 RegVal;
	XAie_NpiMod *NpiMod;
	XAie_BackendNpiWrReq IOReq;
	XAie_BackendNpiMaskPollReq MPReq;
	AieRC RC;

	NpiMod = _XAie_NpiGetMod(DevInst);
	if (NpiMod == NULL) {
		return XAIE_INVALID_ARGS;
	}

	RC = NpiMod->SetProtectedRegField(DevInst, Req, &RegVal);
	if (RC != XAIE_OK) {
		return RC;
	}

	IOReq = _XAie_SetBackendNpiWrReq(NpiMod->ProtRegOff, RegVal);

	RC = _XAie_NpiSetLock(DevInst, XAIE_DISABLE);
	if (RC != XAIE_OK) {
		return RC;
	}

	RC = XAie_RunOp(DevInst, XAIE_BACKEND_OP_NPIWR32, &IOReq);
	if (RC != XAIE_OK) {
		_XAie_NpiSetLock(DevInst, XAIE_ENABLE);
		return RC;
	}

	MPReq = _XAie_SetBackendNpiMaskPollReq(NpiMod->ProtRegOff, 0U,
			0U, XAIE_NPI_TIMEOUT_US);

	RC = XAie_RunOp(DevInst, XAIE_BACKEND_OP_NPIMASKPOLL32, &MPReq);
	_XAie_NpiSetLock(DevInst, XAIE_ENABLE);

	return RC;
}
#endif /* XAIE_FEATURE_PRIVILEGED_ENABLE */
/** @} */
