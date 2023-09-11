/******************************************************************************
* Copyright (C) 2022 AMD.  All rights reserved.
* Copyright (C) 2022-2023, Advanced Micro Devices, Inc. All Rights Reserved.  *
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
    [XAIE_CORE_STATUS_STREAM_STALL_MS0_BIT]     "Stream_Stall_MSO",
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
    [XAIE_DMA_STATUS_S2MM_ERROR_LOCK_ACCESS_TO_UNAVAIL]   "Error_Lock_Access_Unavail",
    [XAIE_DMA_STATUS_S2MM_ERROR_DM_ACCESS_TO_UNAVAIL]     "Error_DM_Access_Unavail",
    [XAIE_DMA_STATUS_S2MM_ERROR_BD_UNAVAIL]               "Error_BD_Unavail",
    [XAIE_DMA_STATUS_S2MM_ERROR_BD_INVALID]               "Error_BD_Invalid",
    [XAIE_DMA_STATUS_S2MM_ERROR_FOT_LENGTH]               "Error_FoT_Length",
    [XAIE_DMA_STATUS_S2MM_ERROR_FOT_BDS_PER_TASK]         "Error_Fot_BDs",
    [XAIE_DMA_STATUS_S2MM_AXI_MM_DECODE_ERROR]            "AXI-MM_decode_error",
    [XAIE_DMA_STATUS_S2MM_AXI_MM_SLAVE_ERROR]             "AXI-MM_slave_error",
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
    [XAIE_DMA_STATUS_MM2S_ERROR_LOCK_ACCESS_TO_UNAVAIL] "Error_Lock_Access_Unavail",
    [XAIE_DMA_STATUS_MM2S_ERROR_DM_ACCESS_TO_UNAVAIL]   "Error_DM_Access_Unavail",
    [XAIE_DMA_STATUS_MM2S_ERROR_BD_UNAVAIL]             "Error_BD_Unavail",
    [XAIE_DMA_STATUS_MM2S_ERROR_BD_INVALID]             "Error_BD_Invalid",
    [XAIE_DMA_STATUS_MM2S_AXI_MM_DECODE_ERROR]          "AXI-MM_decode_error",
    [XAIE_DMA_STATUS_MM2S_AXI_MM_SLAVE_ERROR]           "AXI-MM_slave_error",
    [XAIE_DMA_STATUS_MM2S_TASK_QUEUE_OVERFLOW]          "Task_Queue_Overflow",
    [XAIE_DMA_STATUS_MM2S_CHANNEL_RUNNING]              "Channel_Running",
    [XAIE_DMA_STATUS_MM2S_TASK_QUEUE_SIZE]              "Task_Queue_Size",
    [XAIE_DMA_STATUS_MM2S_CURRENT_BD]                   "Cur_BD",
};


/**************************** Function Definitions *******************************/
/*****************************************************************************/
/**
*
* This is a helper function to implement string copy.
*
*
* @param	Destination: Destination char pointer.
* @param	Source: Source string.
* @param        CommaNeeded: u8; if true a comma will be appended at the
*               end after copying.
*
* @return	The number of characters copied successfully from source to
*               destination. XAIE_ERROR if a failure occured.
*
* @note		Internal only.
*
*******************************************************************************/
static int _XAie_strcpy(char* Destination, const char* Source, u8 CommaNeeded)
{
    int len = 0;

    if (Destination == NULL || Source == NULL) {
        return XAIE_ERR;
    }

    char *ptr = Destination;
    while (*Source != '\0')
    {
        *Destination = *Source;
        Destination++;
        Source++;
	len++;
    }

    if (CommaNeeded)
    {
        *Destination = ',';
        len++;
    }
    return len;
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
*               a failure occured.
*
* @note		Internal only.
*
******************************************************************************/
static void _XAie_ToString(char str[], int num)
{
    int i, rem, len = 0, n;

    n = num;

    if(num == 0)
    {
	str[num] = '0';
        str[1]   = '\0';
	return;
    }
    while (n != 0)
    {
        len++;
        n /= 10;
    }
    for (i = 0; i < len; i++)
    {
        rem = num % 10;
        num = num / 10;
        str[len - (i + 1)] = rem + '0';
    }
    str[len] = '\0';
}

/*****************************************************************************/
/**
*
* This API maps the Core status bits to its corresponding string.  If more
* than one bit is set, all the corresponding strings are separated by a comma
* and concatenated.
*
* @param        Reg: Core Status raw register value.
* @param        Buf: Pointer to the buffer which the string will be written to.
*
* @return       The total number of characters filled up in the Buffer
*               argument parameter.
*
* @note         None.
*
******************************************************************************/
int XAie_CoreStatus_CSV(u32 Reg, char *Buf) {

    int CharsWritten = 0;        // characters written
    u8  Shift 	= 0;
    u32 Val 	= 0;
    u32 TempReg = Reg;

    if((TempReg & 0x01U)) // if bit 0 is set then Enabled state.
        CharsWritten += _XAie_strcpy(&Buf[CharsWritten], XAie_CoreStatus_Strings[Shift], TRUE);

    Shift++;
    TempReg     = TempReg >> 1;
    Val = TempReg & 0x01U;
    if(Val) // if bit 1 is set then Reset state.
        CharsWritten += _XAie_strcpy(&Buf[CharsWritten], XAie_CoreStatus_Strings[Shift], TRUE);

    if((Reg&0x01U) || (Reg&0x02U) == 0U) // if neither bit 0 nor bit  1 is set, Disabled is output.
        CharsWritten += _XAie_strcpy(&Buf[CharsWritten], "Disabled", TRUE);

    TempReg     = TempReg >> 1;

    while(TempReg!=0){
	Shift++;
	Val = TempReg & 0x1U;
	if (Val){
            CharsWritten += _XAie_strcpy(&Buf[CharsWritten], XAie_CoreStatus_Strings[Shift], TRUE);
	}
	TempReg >>= 1;
    }
    CharsWritten--;   // The last call added a comma , which is not needed at the end.
    Buf[CharsWritten]='\0';
    return CharsWritten;
}

/*****************************************************************************/
/**
*
* This API maps the DMA S2MM status bits to its coresponding string.  If more
* than one bit is set, all the corresponding strings are separated by a comma
* and concatenated.
*
* @param        Reg: DMA S2MM status raw register value.
* @param        Buf: Pointer to the buffer which the string will be written to.
* @param        TType: Tile type used to distinguish the tile type, Core,
*               Memory or Shim for which this function is called.
*
* @return       The total number of characters filled up in the Buffer
*               argument parameter.
*
* @note         None.
*
******************************************************************************/
int XAie_DmaS2MMStatus_CSV(u32 Reg, char *Buf, u8 TType) {

    int CharsWritten = 0;
    enum DmaStatus_S2MM_enum Flag;
    u32 FlagVal;
    FlagVal = (u32)Flag;

    for(FlagVal = (u32)XAIE_DMA_STATUS_S2MM_STATUS; FlagVal <= XAIE_DMA_STATUS_S2MM_MAX; FlagVal++) {

        // Below is for bits  8, 9 in DMAS2MM for mem tile
        if( (TType != XAIEGBL_TILE_TYPE_MEMTILE) && \
			((FlagVal == XAIE_DMA_STATUS_S2MM_ERROR_LOCK_ACCESS_TO_UNAVAIL) || \
			 (FlagVal == XAIE_DMA_STATUS_S2MM_ERROR_DM_ACCESS_TO_UNAVAIL))
	  ) {
	   continue;
	}

        // Below is for bits 16,17 in DMAS2MM for shim tile
        if( (TType != XAIEGBL_TILE_TYPE_SHIMNOC) && ((FlagVal == XAIE_DMA_STATUS_S2MM_AXI_MM_DECODE_ERROR) || \
				   (FlagVal == XAIE_DMA_STATUS_S2MM_AXI_MM_SLAVE_ERROR)) ) {
	   continue;
	}

	if(XAie_DmaS2MMStatus_Strings[FlagVal] != NULL)
        {
            u32 Val = (Reg >> FlagVal);
            char TempString[4];
            switch (FlagVal) {
		case (u32)XAIE_DMA_STATUS_S2MM_STATUS:
		    CharsWritten += _XAie_strcpy(&Buf[CharsWritten], "Channel_status:", FALSE);
		    Val &= 0x3U;
		    if (Val == 0U) {
		        CharsWritten += _XAie_strcpy(&Buf[CharsWritten], "Idle", TRUE);
		    }
		    else if (Val == 1U) {
		        CharsWritten += _XAie_strcpy(&Buf[CharsWritten], "Starting", TRUE);
		    }
		    else if (Val == 2U) {
		        CharsWritten += _XAie_strcpy(&Buf[CharsWritten], "Running", TRUE);
		    }
		    else {
		        CharsWritten += _XAie_strcpy(&Buf[CharsWritten], "Invalid_State", TRUE);
		    }
		    break;
		case (u32)XAIE_DMA_STATUS_S2MM_TASK_QUEUE_OVERFLOW:
		    Val &= 0x01U;
		    CharsWritten--; // to overwrite the comma in the previous write
		    if (Val == 0U) {
		        CharsWritten += _XAie_strcpy(&Buf[CharsWritten], ";Queue_status:okay", TRUE);
		    }
		    else {
		        CharsWritten += _XAie_strcpy(&Buf[CharsWritten], ";Queue_status:channel_overflow", TRUE);
		    }
		    break;
		case (u32)XAIE_DMA_STATUS_S2MM_CHANNEL_RUNNING:
		    Val &= 0x1U;
		    CharsWritten--; // to overwrite the comma in the previous write
		    if (Val == 0U) {
		        CharsWritten += _XAie_strcpy(&Buf[CharsWritten], ";Queue_empty", TRUE);
			}
		    else {
		        CharsWritten += _XAie_strcpy(&Buf[CharsWritten], ";Queue_not_empty", TRUE);
			}
		    break;
		case (u32)XAIE_DMA_STATUS_S2MM_TASK_QUEUE_SIZE:
		    Val &= 0x7U;
		    CharsWritten--; // to overwrite the comma in the previous write
		    _XAie_ToString(TempString, Val);
		    CharsWritten += _XAie_strcpy(&Buf[CharsWritten], ";Tasks_in_queue:", FALSE);
		    CharsWritten += _XAie_strcpy(&Buf[CharsWritten], TempString, TRUE);
		    break;
		case (u32)XAIE_DMA_STATUS_S2MM_CURRENT_BD:
		    Val &= 0x3FU;
		    CharsWritten--; // to overwrite the comma in the previous write
		    _XAie_ToString(TempString,Val);
		    CharsWritten += _XAie_strcpy(&Buf[CharsWritten], ";Current_bd:", FALSE);
		    CharsWritten += _XAie_strcpy(&Buf[CharsWritten], TempString, TRUE);
		    break;
		default:
		    Val &= 0x1U;
		    if (Val) {
			CharsWritten += _XAie_strcpy(&Buf[CharsWritten], XAie_DmaS2MMStatus_Strings[Flag], TRUE);
		    }
		    break;
		};
        }
    }

    CharsWritten--;   // The last call added a comma , which is not needed at the end.
    Buf[CharsWritten]='\0';
    return CharsWritten;
}   // end of XAie_DmaS2MMStatus_CSV

/*****************************************************************************/
/**
*
* This API maps the DMA MM2S status bits to its corresponding string. If more
* than one bit is set, all the corresponding strings are separated by a comma
* and concatenated.
*
* @param        Reg: DMA MM2S status raw register value.
* @param        Buf: Pointer to the buffer to where the string will be written.
* @param        TType: Tile type used to distinguish the tile type, Core,
*               Memory or Shim for which this function is called.
*
* @return       The total number of characters filled up in the Buffer.
*
* @note         None.
*
******************************************************************************/
int XAie_DmaMM2SStatus_CSV(u32 Reg, char *Buf, u8 TType) {

    int CharsWritten = 0;
    enum DmaStatus_MM2S_enum Flag;
    u32 FlagVal;
    FlagVal = (u32)Flag;

    for(FlagVal = XAIE_DMA_STATUS_MM2S_STATUS; FlagVal <= XAIE_DMA_STATUS_MM2S_MAX; FlagVal++) {

        // Below is for bits  8, 9, 10 in DMA_MM2S for mem tile
        if( (TType != XAIEGBL_TILE_TYPE_MEMTILE) && \
			((FlagVal == XAIE_DMA_STATUS_MM2S_ERROR_LOCK_ACCESS_TO_UNAVAIL) || \
                         (FlagVal == XAIE_DMA_STATUS_MM2S_ERROR_DM_ACCESS_TO_UNAVAIL)   || \
			 (FlagVal == XAIE_DMA_STATUS_MM2S_ERROR_BD_UNAVAIL))
	  ) {
           continue;
	}

	// Below is for bits 16, 17 in DMA_MM2S for shim tile
	if( (TType != XAIEGBL_TILE_TYPE_SHIMNOC) && \
			((FlagVal == XAIE_DMA_STATUS_MM2S_AXI_MM_DECODE_ERROR) || \
			 (FlagVal == XAIE_DMA_STATUS_MM2S_AXI_MM_SLAVE_ERROR))
	  ) {
	   continue;
	}

	if(XAie_DmaMM2SStatus_Strings[FlagVal] != NULL)
        {
            u32 Val = (Reg >> FlagVal);
            char TempString[4];
            switch (FlagVal) {
                case (u32)XAIE_DMA_STATUS_MM2S_STATUS:
		    CharsWritten += _XAie_strcpy(&Buf[CharsWritten], "Channel_status:", FALSE);
                    Val &= 0x3U;
                    if (Val == 0U) {
                        CharsWritten += _XAie_strcpy(&Buf[CharsWritten], "Idle", TRUE);
                    }
                    else if (Val == 1U) {
                        CharsWritten += _XAie_strcpy(&Buf[CharsWritten], "Starting", TRUE);
                    }
                    else if (Val == 2U) {
                        CharsWritten += _XAie_strcpy(&Buf[CharsWritten], "Running", TRUE);
                    }
                    else {
                        CharsWritten += _XAie_strcpy(&Buf[CharsWritten], "Invalid_State", TRUE);
                    }
                    break;
		case (u32)XAIE_DMA_STATUS_MM2S_TASK_QUEUE_OVERFLOW:
		    CharsWritten--; // to overwrite the comma in the previous write
		    Val &= 0x01U;
		    if (Val == 0U) {
		        CharsWritten += _XAie_strcpy(&Buf[CharsWritten], ";Queue_status:okay", TRUE);
		    }
		    else {
		        CharsWritten += _XAie_strcpy(&Buf[CharsWritten], ";Queue_status:channel_overflow", TRUE);
		    }
		    break;
                case (u32)XAIE_DMA_STATUS_MM2S_CHANNEL_RUNNING:
		    CharsWritten--; // to overwrite the comma in the previous write
                    Val &= 0x1U;
                    if (Val == 0U) {
                        CharsWritten += _XAie_strcpy(&Buf[CharsWritten], ";Queue_empty", TRUE);
					}
                    else {
                        CharsWritten += _XAie_strcpy(&Buf[CharsWritten], ";Queue_not_empty", TRUE);
					}
                    break;
                case (u32)XAIE_DMA_STATUS_MM2S_TASK_QUEUE_SIZE:
		    CharsWritten--; // to overwrite the comma in the previous write
                    Val &= 0x7U;
                    _XAie_ToString(TempString, Val);
		    CharsWritten += _XAie_strcpy(&Buf[CharsWritten], ";Tasks_in_queue:", FALSE);
                    CharsWritten += _XAie_strcpy(&Buf[CharsWritten], TempString, TRUE);
                    break;
                case (u32)XAIE_DMA_STATUS_MM2S_CURRENT_BD:
		    CharsWritten--; // to overwrite the comma in the previous write
                    Val &= 0x3FU;
		    _XAie_ToString(TempString, Val);
                    CharsWritten += _XAie_strcpy(&Buf[CharsWritten], ";Current_bd:", FALSE);
                    CharsWritten += _XAie_strcpy(&Buf[CharsWritten], TempString, TRUE);
                    break;
                default:
                    Val &= 0x1U;
                    if (Val) {
                        CharsWritten += _XAie_strcpy(&Buf[CharsWritten], XAie_DmaMM2SStatus_Strings[Flag], TRUE);
                    }
                    break;
                };
        }
    }
    CharsWritten--;   // The last call added a comma , which is not needed at the end.
    Buf[CharsWritten]='\0';
    return CharsWritten;
}   // end of XAie_DmaMM2SStatus_CSV

#endif /* XAIE_FEATURE_UTIL_STATUS_ENABLE */
/** @} */
