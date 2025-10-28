/******************************************************************************
* Copyright (C) 2022 Xilinx, Inc. All rights reserved.
* Copyright (C) 2022-2025 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_util_events.c
* @{
*
* This file contains function implementations for AIE utilities
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Gregory 03/31/2022  Initial creation
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xaie_feature_config.h"
#include "xaie_util_events.h"

#ifdef XAIE_FEATURE_UTIL_ENABLE

/**************************** Variable Definitions *******************************/
/* Event string lookup table indexed by event number */

static const char *XAie_EventStrings[] = {
	XAIE_EVENT_GENERATE_CORE_MOD(ENUM_STRING)
	XAIE_EVENT_GENERATE_MEM_MOD(ENUM_STRING)
	XAIE_EVENT_GENERATE_PL_MOD(ENUM_STRING)
	XAIE_EVENT_GENERATE_MEM_TILE(ENUM_STRING)
};

/**************************** Function Definitions *******************************/
/*****************************************************************************/
/**
*
* This API takes input of an Event id and returns a string describing the event
*
* @param	Event - Event ID
*
* @return	String corresponding to Event ID
*
* @note		None
*
******************************************************************************/
const char* XAie_EventGetString(XAie_Events Event) {
	return XAie_EventStrings[Event];
}

#endif /* XAIE_FEATURE_UTIL_ENABLE */
/** @} */
