/******************************************************************************
* Copyright (C) 2023 AMD.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_device_aie2p.h
* @{
*
* This file contains the apis for device specific operations of aie.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Keerthanna   01/25/2023  Initial creation
* </pre>
*
******************************************************************************/
#ifndef XAIE_DEVICE_AIE2PS_H
#define XAIE_DEVICE_AIE2PS_H

/***************************** Include Files *********************************/
/************************** Function Prototypes  *****************************/
u8 _XAie2PS_GetTTypefromLoc(XAie_DevInst *DevInst, XAie_LocType Loc);
AieRC _XAie2PS_SetAxiMMIsolation(XAie_DevInst* DevInst, u8 IsolationFlags);
AieRC _XAie2PS_SetColumnClk(XAie_DevInst *DevInst, XAie_BackendColumnReq *Args);

#endif /* XAIE_DEVICE_AIE2P */
/** @} */
