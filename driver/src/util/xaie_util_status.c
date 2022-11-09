/******************************************************************************
* Copyright (C) 2022 AMD.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_util_status.c
* @{
*
* This file contains function implementations for AIE status utilities
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   dsteger 07/25/2022  Initial creation
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xaie_util_status.h"

#ifdef XAIE_FEATURE_UTIL_STATUS_ENABLE

/**************************** Variable Definitions *******************************/

static const char* XAie_CoreStatus_Strings[] = {
    [XAIE_CORE_STATUS_ENABLE_BIT]               "Enable",
    [XAIE_CORE_STATUS_RESET_BIT]                "Reset",
    [XAIE_CORE_STATUS_MEM_STALL_S_BIT]          "Memory_Stall_S",
    [XAIE_CORE_STATUS_MEM_STALL_W_BIT]          "Memory_Stall_W",
    [XAIE_CORE_STATUS_MEM_STALL_N_BIT]          "Memory_Stall_N",
    [XAIE_CORE_STATUS_MEM_STALL_E_BIT]          "Memory_Stall_E",
    [XAIE_CORE_STATUS_LOCK_STALL_S_BIT]         "Lock_Stall_S",
    [XAIE_CORE_STATUS_LOCK_STALL_W_BIT]         "Lock_Stall_W",
    [XAIE_CORE_STATUS_LOCK_STALL_N_BIT]         "Lock_Stall_N",
    [XAIE_CORE_STATUS_LOCK_STALL_E_BIT]         "Lock_Stall_E",
    [XAIE_CORE_STATUS_STREAM_STALL_SS0_BIT]     "Stream_Stall_SSO",
    [XAIE_CORE_STATUS_STREAM_STALL_SS1_BIT]     "Stream_Stall_SS1",
    [XAIE_CORE_STATUS_STREAM_STALL_MS0_BIT]     "Stream_Stall_MSO",
    [XAIE_CORE_STATUS_STREAM_STALL_MS1_BIT]     "Stream_Stall_MS1",
    [XAIE_CORE_STATUS_CASCADE_STALL_SCD_BIT]    "Cascade_Stall_SCD",
    [XAIE_CORE_STATUS_CASCADE_STALL_MCD_BIT]    "Cascade_Stall_MCD",
    [XAIE_CORE_STATUS_DEBUG_HALT_BIT]           "Debug_Halt",
    [XAIE_CORE_STATUS_ECC_ERROR_STALL_BIT]      "ECC_Error_Stall",
    [XAIE_CORE_STATUS_ECC_SCRUBBING_STALL_BIT]  "ECC_Scrubbing_Stall",
    [XAIE_CORE_STATUS_ERROR_HALT_BIT]           "Error_Halt",
    [XAIE_CORE_STATUS_DONE_BIT]                 "Core_Done",
    [XAIE_CORE_STATUS_PROCESSOR_BUS_STALL_BIT]  "Core_Proc_Bus_Stall",
};

static const char* XAie_DmaS2MMStatus_Strings[] = {
    [XAIE_DMA_STATUS_S2MM_STATUS]                         "Status",
    [XAIE_DMA_STATUS_S2MM_STALLED_LOCK_ACK]               "Stalled_Lock_Acq",
    [XAIE_DMA_STATUS_S2MM_STALLED_LOCK_REL]               "Stalled_Lock_Rel",
    [XAIE_DMA_STATUS_S2MM_STALLED_STREAM_STARVATION]      "Stalled_Stream_Starvation",
    [XAIE_DMA_STATUS_S2MM_STALLED_TCT_OR_COUNT_FIFO_FULL] "Stalled_TCT_Or_Count_FIFO_Full",
    [XAIE_DMA_STATUS_S2MM_ERROR_BD_UNAVAIL]               "Error_BD_Unavail",
    [XAIE_DMA_STATUS_S2MM_ERROR_BD_INVALID]               "Error_BD_Invalid",
    [XAIE_DMA_STATUS_S2MM_ERROR_FOT_LENGTH]               "Error_FoT_Length",
    [XAIE_DMA_STATUS_S2MM_ERROR_FOT_BDS_PER_TASK]         "Error_Fot_BDs",
    [XAIE_DMA_STATUS_S2MM_TASK_QUEUE_OVERFLOW]            "Task_Queue_Overflow",
    [XAIE_DMA_STATUS_S2MM_CHANNEL_RUNNING]                "Channel_Running",
    [XAIE_DMA_STATUS_S2MM_TASK_QUEUE_SIZE]                "Task_Queue_Size",
    [XAIE_DMA_STATUS_S2MM_CURRENT_BD]                     "Cur_BD",
};

static const char* XAie_DmaMM2SStatus_Strings[] = {
    [XAIE_DMA_STATUS_MM2S_STATUS]                       "Status",
    [XAIE_DMA_STATUS_MM2S_STALLED_LOCK_ACK]             "Stalled_Lock_Acq",
    [XAIE_DMA_STATUS_MM2S_STALLED_LOCK_REL]             "Stalled_Lock_Rel",
    [XAIE_DMA_STATUS_MM2S_STALLED_STREAM_BACKPRESSURE]  "Stalled_Stream_Back_Pressure",
    [XAIE_DMA_STATUS_MM2S_STALLED_TCT]                  "Stalled_TCT",
    [XAIE_DMA_STATUS_MM2S_ERROR_BD_INVALID]             "Error_BD_Invalid",
    [XAIE_DMA_STATUS_MM2S_TASK_QUEUE_OVERFLOW]          "Task_Queue_Overflow",
    [XAIE_DMA_STATUS_MM2S_CHANNEL_RUNNING]              "Channel_Running",
    [XAIE_DMA_STATUS_MM2S_TASK_QUEUE_SIZE]              "Task_Queue_Size",
    [XAIE_DMA_STATUS_MM2S_CURRENT_BD]                   "Cur_BD",
};

/**************************** Function Definitions *******************************/
#define XAIE_ERROR_MSG(...)						\
	"[AIE ERROR] %s():%d: %s", __func__, __LINE__, __VA_ARGS__

#ifdef _ENABLE_IPU_LX6_
#include <printf.h>
#endif
#define XAIE_ERROR_RETURN(...) {	\
		printf(__VA_ARGS__);		\
}

/*****************************************************************************/
/**
*
* This is a helper function to implement string copy.
*
* @param	destination: Destination char pointer.
* @param	source: Source string.
*
* @return	XAIE_OK if the string was successfully copied. XAIE_ERROR if
*           a failure occured.
*
* @note		Internal only.
*
******************************************************************************/
AieRC _XAie_strcpy(char* Destination, const char* Source)
{
    if (Destination == NULL || Source == NULL) {
        return XAIE_ERR;
    }

    char *ptr = Destination;
    while (*Source != '\0')
    {
        *Destination = *Source;
        Destination++;
        Source++;
    }

    *Destination = '\0';
    return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This is a helper function to implement integer to string conversion.
*
* @param	num: Integer value to be converted to String.
* @param	str: Char pointer to copy the integer to.
*
* @return	XAIE_OK if the string was successfully copied. XAIE_ERROR if
*           a failure occured.
*
* @note		Internal only.
*
******************************************************************************/
AieRC _XAie_int2str(u32 Num, char* Str){

    u8 Len = (int)(Num / 10) + 2;

    if (Str == NULL) {
        return XAIE_ERR;
    }

    u8 i = 1;
    if (Num == 0)
    {
        Str[i-1] = '0';
        Str[i] = '\0';
        return XAIE_OK;
    }

    while(Num > 0){
        u8 rem = Num % 10;
        Str[Len-1-i] = rem + '0';
        Num /= 10;
        i++;
    }
    Str[Len-1] = '\0';
    return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This API maps the core status bits to its coresponding string.
*
* @param	Reg: Core status raw register value.
* @param	Flag: Status flag to translate to string.
* @param	Buf: Pointer to the buffer which the string will be written to.
* @param	BufSize: Size of the buffer.
*
* @return	XAIE_OK if the buffer was successfully writting to. XAIE_ERROR if
*           a failure occured.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_CoreStatus_ToString(u32 Reg, u8 Flag, char *Buf, u32 BufSize) {

    AieRC RC = XAIE_OK;
    u32 val = (Reg >> Flag) & 0x1;

    if (val){
        RC = _XAie_strcpy(Buf, XAie_CoreStatus_Strings[Flag]);
	    return RC;
    } else {
        RC = _XAie_strcpy(Buf, "\0");
        return RC;
    }
}

/*****************************************************************************/
/**
*
* This API maps the DMA S2MM status bits to its coresponding string.
*
* @param	Reg: DMA S2MM status raw register value.
* @param	Flag: Status flag to translate to string.
* @param	Buf: Pointer to the buffer which the string will be written to.
* @param	BufSize: Size of the buffer.
*
* @return	String corresponding to the Register and flag
*
* @note		None.
*
******************************************************************************/
AieRC XAie_DmaS2MMStatus_ToString(u32 Reg, u8 Flag, char *Buf, u32 BufSize) {
    AieRC RC;
    u32 val = (Reg >> Flag);
    char temp[3];

    switch (Flag) {
        case XAIE_DMA_STATUS_S2MM_STATUS:
            val &= 0x3;

            if (val == 0) {
                RC = _XAie_strcpy(Buf, "Idle");
            }
            else if (val == 1) {
                RC = _XAie_strcpy(Buf, "Starting");
            }
            else if (val == 2) {
                RC = _XAie_strcpy(Buf, "Running");
            }
            else {
                RC = _XAie_strcpy(Buf, "Invalid State");
            }
            break;
        case XAIE_DMA_STATUS_S2MM_CHANNEL_RUNNING:
            val &= 0x1;

            if (val == 0)
                RC = _XAie_strcpy(Buf, "Channel data path empty and queue empty");
            else
                RC = _XAie_strcpy(Buf, "Channel operating on a transfer or queue not empty");
            break;
        case XAIE_DMA_STATUS_S2MM_TASK_QUEUE_SIZE:
            val &= 0x7;

            _XAie_int2str(val, temp);
            RC = _XAie_strcpy(Buf, temp);
            break;
        case XAIE_DMA_STATUS_S2MM_CURRENT_BD:
            val &= 0xF;

            _XAie_int2str(val, temp);
            RC = _XAie_strcpy(Buf, temp);
            break;
        default:
            val &= 0x1;

            if (val) {
                RC = _XAie_strcpy(Buf, XAie_DmaS2MMStatus_Strings[Flag]);
            }
            else {
                RC = _XAie_strcpy(Buf, "\0");
            }
            break;
    };

    return RC;
}

/*****************************************************************************/
/**
*
* This API maps the DMA MM2S status bits to its coresponding string.
*
* @param	Reg: DMA MM2S status raw register value.
* @param	Flag: Status flag to translate to string.
* @param	Buf: Pointer to the buffer which the string will be written to.
* @param	BufSize: Size of the buffer.
*
* @return	XAIE_OK if the buffer was successfully writting to. XAIE_ERROR if
*           a failure occured.
*
* @note		None.
*
******************************************************************************/
AieRC XAie_DmaMM2SStatus_ToString(u32 Reg, u8 Flag, char *Buf, u32 BufSize) {
    int RC;
    u32 val = (Reg >> Flag);
    char temp[3];

    switch (Flag) {
        case XAIE_DMA_STATUS_MM2S_STATUS:
            val &= 0x3;

            if (val == 0) {
                RC = _XAie_strcpy(Buf, "Idle");
            }
            else if (val == 1) {
                RC = _XAie_strcpy(Buf, "Starting");
            }
            else if (val == 2) {
                RC = _XAie_strcpy(Buf, "Running");
            }
            else {
                RC = _XAie_strcpy(Buf, "Invalid state");
            }
            break;
        case XAIE_DMA_STATUS_MM2S_CHANNEL_RUNNING:
            val &= 0x1;

            if (val == 0) {
                RC = _XAie_strcpy(Buf, "Channel data path empty and queue empty");
            }
            else {
                RC = _XAie_strcpy(Buf, "Channel operating on a transfer or queue not empty");
            }
            break;
        case XAIE_DMA_STATUS_MM2S_TASK_QUEUE_SIZE:
            val &= 0x7;

            _XAie_int2str(val, temp);
            RC = _XAie_strcpy(Buf, temp);
            break;
        case XAIE_DMA_STATUS_MM2S_CURRENT_BD:
            val &= 0xF;

            _XAie_int2str(val, temp);
            RC = _XAie_strcpy(Buf, temp);
            break;
        default:
            val &= 0x1;

            if (val)
                RC = _XAie_strcpy(Buf, XAie_DmaMM2SStatus_Strings[Flag]);
            else
                RC = _XAie_strcpy(Buf, "\0");
            break;
    };

    return RC;
}

#endif /* XAIE_FEATURE_UTIL_STATUS_ENABLE */
/** @} */
