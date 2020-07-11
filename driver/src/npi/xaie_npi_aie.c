/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_npi_aie.c
* @{
*
* This file contains routines for operations for accessing AI engine
* specific NPI.
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xaie_npi.h"
#include "xaiegbl.h"

/************************** Constant Definitions *****************************/
#define XAIE_NPI_PCSR_MASK				0x00000000U
#define XAIE_NPI_PCSR_MASK_SHIM_RESET_MSK		0x08000000U
#define XAIE_NPI_PCSR_MASK_SHIM_RESET_LSB		27U

#define XAIE_NPI_PCSR_CONTROL				0X00000004U
#define XAIE_NPI_PCSR_CONTROL_SHIM_RESET_MSK		0x08000000U
#define XAIE_NPI_PCSR_CONTROL_SHIM_RESET_LSB		27U

#define XAIE_NPI_PCSR_LOCK				0X0000000CU
#define XAIE_NPI_PCSR_UNLOCK_CODE			0xF9E8D7C6U

#define XAIE_NPI_SPARE_REG				0x00000200U
#define XAIE_NPI_PROT_REG_EN_MSK			0x00000001U
#define XAIE_NPI_PROT_REG_EN_LSB			0U

/****************************** Type Definitions *****************************/

/************************** Variable Definitions *****************************/
static u32 _XAie_NpiSetProtectedRegField(XAie_DevInst *DevInst, u8 Enable);

XAie_NpiMod _XAieNpiMod =
{
	.PcsrMaskOff = XAIE_NPI_PCSR_MASK,
	.PcsrCntrOff = XAIE_NPI_PCSR_CONTROL,
	.PcsrLockOff = XAIE_NPI_PCSR_LOCK,
	.ProtRegOff = XAIE_NPI_SPARE_REG,
	.PcsrUnlockCode = XAIE_NPI_PCSR_UNLOCK_CODE,
	.ShimReset = {XAIE_NPI_PCSR_CONTROL_SHIM_RESET_LSB, XAIE_NPI_PCSR_CONTROL_SHIM_RESET_MSK},
	.ProtRegEnable = {XAIE_NPI_PROT_REG_EN_LSB, XAIE_NPI_PROT_REG_EN_MSK},
	.ProtRegFirstCol = {0, 0},
	.ProtRegLastCol = {0, 0},
	.SetProtectedRegField = _XAie_NpiSetProtectedRegField,
};

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This is function to setup the protected register configuration value.
*
* @param	DevInst : AI engine partition device pointer
* @param	Enable : XAIE_ENABLE to enable access to protected register,
*			 XAIE_DISABLE to disable access to protected register
*
* @return	32bit Value used by caller function to set to the register
*
* @note		None
*
*******************************************************************************/
static u32 _XAie_NpiSetProtectedRegField(XAie_DevInst *DevInst, u8 Enable)
{
	u32 RegVal;

	(void)DevInst;
	RegVal = XAie_SetField(Enable, _XAieNpiMod.ProtRegEnable.Lsb,
			       _XAieNpiMod.ProtRegEnable.Mask);
	return RegVal;
}

/** @} */
