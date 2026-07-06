/******************************************************************************
* Copyright (C) 2024 AMD.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file pm_init.h
* @{
*
* This file contains api to initialize the IPI and PM.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Keerthanna 05/28/2024 Initial creation.
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include <stdlib.h>
#include "xaie_helper.h"

#ifdef __AIEBAREMETAL__

#include "pm_api_sys.h"
#include "pm_client.h"
#include "xpm_defs.h"
#include <xipipsu_hw.h>
#include <xipipsu.h>
#include <xstatus.h>
#include "xil_types.h"
#include "xil_io.h"

/************************** Function Definitions *****************************/

#define PM_DEV_NODE_ID 0x18800000U
/*****************************************************************************/
/**
*
* This api clears the ps interrupt status.
*
* @param	IpiInst: IPI instance.
*
* @return	XST_SUCCESS on success, error on failure.
*
* @note		None.
*
*******************************************************************************/
static XStatus XAie_IpiConfigure(XIpiPsu *IpiInst)
{
	int Status = XST_FAILURE;
	XIpiPsu_Config *IpiCfgPtr;

	if (!IpiInst) {
		goto done;
	}
	/* Look Up the config data */
	IpiCfgPtr = XIpiPsu_LookupConfig(XPAR_XIPIPSU_0_BASEADDR);
	if (!IpiCfgPtr) {
		Status = XST_FAILURE;
		XAIE_ERROR("Failed to find config for %d\n", XPAR_XIPIPSU_0_BASEADDR);
		goto done;
	}

	/* Init with the Cfg Data */
	Status = XIpiPsu_CfgInitialize(IpiInst, IpiCfgPtr, IpiCfgPtr->BaseAddress);
	if (XST_SUCCESS != Status) {
		XAIE_ERROR("IPI config initialization failed!\n");
		goto done;
	}

	/* Clear Any existing Interrupts */
	XIpiPsu_ClearInterruptStatus(IpiInst, XIPIPSU_ALL_MASK);

done:
	return Status;
}

/*****************************************************************************/
/**
*
* This api enables the interrupt.
*
* @param	IpiInst: IPI instance.
*
* @return	XST_SUCCESS on success, error on failure.
*
* @note		None.
*
*******************************************************************************/
XStatus XAie_IpiInit(XIpiPsu *InstancePtr)
{
	int Status;

	Status = XAie_IpiConfigure(InstancePtr);
	if (Status != XST_SUCCESS) {
		XAIE_ERROR("IPI configuration failed!\n");
		goto done;
	}

	XIpiPsu_InterruptEnable(InstancePtr, XIPIPSU_ALL_MASK);

done:

	return Status;
}

/*****************************************************************************/
/**
*
* This api comfigures IPI and requests node.
*
* @param	IpiInst: IPI instance.
*
* @return	XST_SUCCESS on success, error on failure.
*
* @note		None.
*
*******************************************************************************/
XStatus XAie_PmInit(XIpiPsu *IpiInst)
{
	int Status;

	/* IPI Initialize */
	Status = XAie_IpiInit(IpiInst);
	if (Status != XST_SUCCESS) {
		XAIE_ERROR("Ipi initialization failed: %d\n", Status);
		goto done;
	}

	Status = XPm_InitXilpm(IpiInst);
	if (Status != XST_SUCCESS) {
		XAIE_ERROR("Xilpm initialization failed: %d\n", Status);
		goto done;
	}

	/* Request AIE device node */
	Status = XPm_RequestNode(PM_DEV_NODE_ID, PM_CAP_ACCESS, 100, 0);
	if (Status != XST_SUCCESS) {
			XAIE_ERROR("Requesting of AIE device node failed!\n");
			goto done;
	}

	Status = XPm_InitFinalize();
	if (Status != XST_SUCCESS) {
		XAIE_ERROR("PM initialization failed!\n");
		goto done;
	}

done:
	return Status;
}

#endif
