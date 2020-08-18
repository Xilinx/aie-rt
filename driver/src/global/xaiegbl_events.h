/******************************************************************************
*
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
******************************************************************************/
/*****************************************************************************/
/**
* @file xaiegbl_events.h
* @{
*
* Header file containing all events macros for AIE.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who      Date     Changes
* ----- ------   -------- -----------------------------------------------------
* 1.0   Dishita  11/21/2019  Initial creation
* </pre>
*
******************************************************************************/

/*
 * Macro of all the Events of AIE for all modules: Core, Memory of AIE tile
 * MEM tile and PL tile
 */

/* Declaration of events for core module of aie tile */
#define XAIEGBL_CORE_EVENT_NONE					0
#define XAIEGBL_CORE_EVENT_TRUE					1
#define XAIEGBL_CORE_EVENT_GROUP_0				2
#define XAIEGBL_CORE_EVENT_TIMER_SYNC				3
#define XAIEGBL_CORE_EVENT_TIMER_VALUE_REACHED			4
#define XAIEGBL_CORE_EVENT_PERF_CNT_0				5
#define XAIEGBL_CORE_EVENT_PERF_CNT_1				6
#define XAIEGBL_CORE_EVENT_PERF_CNT_2				7
#define XAIEGBL_CORE_EVENT_PERF_CNT_3				8
#define XAIEGBL_CORE_EVENT_COMBO_EVENT_0			9
#define XAIEGBL_CORE_EVENT_COMBO_EVENT_1			10
#define XAIEGBL_CORE_EVENT_COMBO_EVENT_2			11
#define XAIEGBL_CORE_EVENT_COMBO_EVENT_3			12
#define XAIEGBL_CORE_EVENT_GROUP_PC_EVENT			15
#define XAIEGBL_CORE_EVENT_PC_0					16
#define XAIEGBL_CORE_EVENT_PC_1					17
#define XAIEGBL_CORE_EVENT_PC_2					18
#define XAIEGBL_CORE_EVENT_PC_3					19
#define XAIEGBL_CORE_EVENT_PC_RANGE_0_1				20
#define XAIEGBL_CORE_EVENT_PC_RANGE_2_3				21
#define XAIEGBL_CORE_EVENT_GROUP_STALL				22
#define XAIEGBL_CORE_EVENT_MEMORY_STALL				23
#define XAIEGBL_CORE_EVENT_STREAM_STALL				24
#define XAIEGBL_CORE_EVENT_CASCADE_STALL			25
#define XAIEGBL_CORE_EVENT_LOCK_STALL				26
#define XAIEGBL_CORE_EVENT_DEBUG_HALTED				27
#define XAIEGBL_CORE_EVENT_ACTIVE				28
#define XAIEGBL_CORE_EVENT_DISABLED				29
#define XAIEGBL_CORE_EVENT_ECC_ERROR_STALL			30
#define XAIEGBL_CORE_EVENT_ECC_SCRUBBING_STALL			31
#define XAIEGBL_CORE_EVENT_GROUP_PROGRAM_FLOW			32
#define XAIEGBL_CORE_EVENT_INSTR_EVENT_0			33
#define XAIEGBL_CORE_EVENT_INSTR_EVENT_1			34
#define XAIEGBL_CORE_EVENT_INSTR_CALL				35
#define XAIEGBL_CORE_EVENT_INSTR_RETURN				36
#define XAIEGBL_CORE_EVENT_INSTR_VECTOR				37
#define XAIEGBL_CORE_EVENT_INSTR_LOAD				38
#define XAIEGBL_CORE_EVENT_INSTR_STORE				39
#define XAIEGBL_CORE_EVENT_INSTR_STREAM_GET			40
#define XAIEGBL_CORE_EVENT_INSTR_STREAM_PUT			41
#define XAIEGBL_CORE_EVENT_INSTR_CASCADE_GET			42
#define XAIEGBL_CORE_EVENT_INSTR_CASCADE_PUT			43
#define XAIEGBL_CORE_EVENT_INSTR_LOCK_ACQUIRE_REQ		44
#define XAIEGBL_CORE_EVENT_INSTR_LOCK_RELEASE_REQ		45
#define XAIEGBL_CORE_EVENT_GROUP_ERRORS_0			46
#define XAIEGBL_CORE_EVENT_GROUP_ERRORS_1			47
#define XAIEGBL_CORE_EVENT_SRS_SATURATE				48
#define XAIEGBL_CORE_EVENT_UPS_SATURATE				49
#define XAIEGBL_CORE_EVENT_FP_OVERFLOW				50
#define XAIEGBL_CORE_EVENT_FP_UNDERFLOW				51
#define XAIEGBL_CORE_EVENT_FP_INVALID				52
#define XAIEGBL_CORE_EVENT_FP_DIV_BY_ZERO			53
#define XAIEGBL_CORE_EVENT_TLAST_IN_WSS_WORDS_0_2		54
#define XAIEGBL_CORE_EVENT_PM_REG_ACCESS_FAILURE		55
#define XAIEGBL_CORE_EVENT_STREAM_PKT_PARITY_ERROR		56
#define XAIEGBL_CORE_EVENT_CONTROL_PKT_ERROR			57
#define XAIEGBL_CORE_EVENT_AXI_MM_SLAVE_ERROR			58
#define XAIEGBL_CORE_EVENT_INSTR_DECOMPRSN_ERROR		59
#define XAIEGBL_CORE_EVENT_DM_ADDRESS_OUT_OF_RANGE		60
#define XAIEGBL_CORE_EVENT_PM_ECC_ERROR_SCRUB_CORRECTED		61
#define XAIEGBL_CORE_EVENT_PM_ECC_ERROR_SCRUB_2BIT		62
#define XAIEGBL_CORE_EVENT_PM_ECC_ERROR_1BIT			63
#define XAIEGBL_CORE_EVENT_PM_ECC_ERROR_2BIT			64
#define XAIEGBL_CORE_EVENT_PM_ADDRESS_OUT_OF_RANGE		65
#define XAIEGBL_CORE_EVENT_DM_ACCESS_TO_UNAVAILABLE		66
#define XAIEGBL_CORE_EVENT_LOCK_ACCESS_TO_UNAVAILABLE		67
#define XAIEGBL_CORE_EVENT_INSTR_EVENT_2			68
#define XAIEGBL_CORE_EVENT_INSTR_EVENT_3			69
#define XAIEGBL_CORE_EVENT_GROUP_STREAM_SWITCH			73
#define XAIEGBL_CORE_EVENT_PORT_IDLE_0				74
#define XAIEGBL_CORE_EVENT_PORT_RUNNING_0			75
#define XAIEGBL_CORE_EVENT_PORT_STALLED_0			76
#define XAIEGBL_CORE_EVENT_PORT_TLAST_0				77
#define XAIEGBL_CORE_EVENT_PORT_IDLE_1				78
#define XAIEGBL_CORE_EVENT_PORT_RUNNING_1			79
#define XAIEGBL_CORE_EVENT_PORT_STALLED_1			80
#define XAIEGBL_CORE_EVENT_PORT_TLAST_1				81
#define XAIEGBL_CORE_EVENT_PORT_IDLE_2				82
#define XAIEGBL_CORE_EVENT_PORT_RUNNING_2			83
#define XAIEGBL_CORE_EVENT_PORT_STALLED_2			84
#define XAIEGBL_CORE_EVENT_PORT_TLAST_2				85
#define XAIEGBL_CORE_EVENT_PORT_IDLE_3				86
#define XAIEGBL_CORE_EVENT_PORT_RUNNING_3			87
#define XAIEGBL_CORE_EVENT_PORT_STALLED_3			88
#define XAIEGBL_CORE_EVENT_PORT_TLAST_3				89
#define XAIEGBL_CORE_EVENT_PORT_IDLE_4				90
#define XAIEGBL_CORE_EVENT_PORT_RUNNING_4			91
#define XAIEGBL_CORE_EVENT_PORT_STALLED_4			92
#define XAIEGBL_CORE_EVENT_PORT_TLAST_4				93
#define XAIEGBL_CORE_EVENT_PORT_IDLE_5				94
#define XAIEGBL_CORE_EVENT_PORT_RUNNING_5			95
#define XAIEGBL_CORE_EVENT_PORT_STALLED_5			96
#define XAIEGBL_CORE_EVENT_PORT_TLAST_5				97
#define XAIEGBL_CORE_EVENT_PORT_IDLE_6				98
#define XAIEGBL_CORE_EVENT_PORT_RUNNING_6			99
#define XAIEGBL_CORE_EVENT_PORT_STALLED_6			100
#define XAIEGBL_CORE_EVENT_PORT_TLAST_6				101
#define XAIEGBL_CORE_EVENT_PORT_IDLE_7				102
#define XAIEGBL_CORE_EVENT_PORT_RUNNING_7			103
#define XAIEGBL_CORE_EVENT_PORT_STALLED_7			104
#define XAIEGBL_CORE_EVENT_PORT_TLAST_7				105
#define XAIEGBL_CORE_EVENT_GROUP_BROADCAST			106
#define XAIEGBL_CORE_EVENT_BROADCAST_0				107
#define XAIEGBL_CORE_EVENT_BROADCAST_1				108
#define XAIEGBL_CORE_EVENT_BROADCAST_2				109
#define XAIEGBL_CORE_EVENT_BROADCAST_3				110
#define XAIEGBL_CORE_EVENT_BROADCAST_4				111
#define XAIEGBL_CORE_EVENT_BROADCAST_5				112
#define XAIEGBL_CORE_EVENT_BROADCAST_6				113
#define XAIEGBL_CORE_EVENT_BROADCAST_7				114
#define XAIEGBL_CORE_EVENT_BROADCAST_8				115
#define XAIEGBL_CORE_EVENT_BROADCAST_9				116
#define XAIEGBL_CORE_EVENT_BROADCAST_10				117
#define XAIEGBL_CORE_EVENT_BROADCAST_11				118
#define XAIEGBL_CORE_EVENT_BROADCAST_12				119
#define XAIEGBL_CORE_EVENT_BROADCAST_13				120
#define XAIEGBL_CORE_EVENT_BROADCAST_14				121
#define XAIEGBL_CORE_EVENT_BROADCAST_15				122
#define XAIEGBL_CORE_EVENT_GROUP_USER_EVENT			123
#define XAIEGBL_CORE_EVENT_USER_EVENT_0				124
#define XAIEGBL_CORE_EVENT_USER_EVENT_1				125
#define XAIEGBL_CORE_EVENT_USER_EVENT_2				126
#define XAIEGBL_CORE_EVENT_USER_EVENT_3				127

/* Declaration of events for Mem module of aie tile */
#define XAIEGBL_MEM_EVENT_NONE					0
#define XAIEGBL_MEM_EVENT_TRUE					1
#define XAIEGBL_MEM_EVENT_GROUP_0				2
#define XAIEGBL_MEM_EVENT_TIMER_SYNC				3
#define XAIEGBL_MEM_EVENT_TIMER_VALUE_REACHED			4
#define XAIEGBL_MEM_EVENT_PERF_CNT_0				5
#define XAIEGBL_MEM_EVENT_PERF_CNT_1				6
#define XAIEGBL_MEM_EVENT_COMBO_EVENT_0				7
#define XAIEGBL_MEM_EVENT_COMBO_EVENT_1				8
#define XAIEGBL_MEM_EVENT_COMBO_EVENT_2				9
#define XAIEGBL_MEM_EVENT_COMBO_EVENT_3				10
#define XAIEGBL_MEM_EVENT_GROUP_WATCHPOINT			15
#define XAIEGBL_MEM_EVENT_WATCHPOINT_0				16
#define XAIEGBL_MEM_EVENT_WATCHPOINT_1				17
#define XAIEGBL_MEM_EVENT_GROUP_DMA_ACTIVITY			20
#define XAIEGBL_MEM_EVENT_DMA_S2MM_0_START_BD			21
#define XAIEGBL_MEM_EVENT_DMA_S2MM_1_START_BD			22
#define XAIEGBL_MEM_EVENT_DMA_MM2S_0_START_BD			23
#define XAIEGBL_MEM_EVENT_DMA_MM2S_1_START_BD			24
#define XAIEGBL_MEM_EVENT_DMA_S2MM_0_FINISHED_BD		25
#define XAIEGBL_MEM_EVENT_DMA_S2MM_1_FINISHED_BD		26
#define XAIEGBL_MEM_EVENT_DMA_MM2S_0_FINISHED_BD		27
#define XAIEGBL_MEM_EVENT_DMA_MM2S_1_FINISHED_BD		28
#define XAIEGBL_MEM_EVENT_DMA_S2MM_0_GO_TO_IDLE			29
#define XAIEGBL_MEM_EVENT_DMA_S2MM_1_GO_TO_IDLE			30
#define XAIEGBL_MEM_EVENT_DMA_MM2S_0_GO_TO_IDLE			31
#define XAIEGBL_MEM_EVENT_DMA_MM2S_1_GO_TO_IDLE			32
#define XAIEGBL_MEM_EVENT_DMA_S2MM_0_STALLED_LOCK_ACQUIRE	33
#define XAIEGBL_MEM_EVENT_DMA_S2MM_1_STALLED_LOCK_ACQUIRE	34
#define XAIEGBL_MEM_EVENT_DMA_MM2S_0_STALLED_LOCK_ACQUIRE	35
#define XAIEGBL_MEM_EVENT_DMA_MM2S_1_STALLED_LOCK_ACQUIRE	36
#define XAIEGBL_MEM_EVENT_DMA_S2MM_0ORY_CONFLICT		37
#define XAIEGBL_MEM_EVENT_DMA_S2MM_1ORY_CONFLICT		38
#define XAIEGBL_MEM_EVENT_DMA_MM2S_0ORY_CONFLICT		39
#define XAIEGBL_MEM_EVENT_DMA_MM2S_1ORY_CONFLICT		40
#define XAIEGBL_MEM_EVENT_GROUP_LOCK				43
#define XAIEGBL_MEM_EVENT_LOCK_0_ACQ				44
#define XAIEGBL_MEM_EVENT_LOCK_0_REL				45
#define XAIEGBL_MEM_EVENT_LOCK_1_ACQ				46
#define XAIEGBL_MEM_EVENT_LOCK_1_REL				47
#define XAIEGBL_MEM_EVENT_LOCK_2_ACQ				48
#define XAIEGBL_MEM_EVENT_LOCK_2_REL				49
#define XAIEGBL_MEM_EVENT_LOCK_3_ACQ				50
#define XAIEGBL_MEM_EVENT_LOCK_3_REL				51
#define XAIEGBL_MEM_EVENT_LOCK_4_ACQ				52
#define XAIEGBL_MEM_EVENT_LOCK_4_REL				53
#define XAIEGBL_MEM_EVENT_LOCK_5_ACQ				54
#define XAIEGBL_MEM_EVENT_LOCK_5_REL				55
#define XAIEGBL_MEM_EVENT_LOCK_6_ACQ				56
#define XAIEGBL_MEM_EVENT_LOCK_6_REL				57
#define XAIEGBL_MEM_EVENT_LOCK_7_ACQ				58
#define XAIEGBL_MEM_EVENT_LOCK_7_REL				59
#define XAIEGBL_MEM_EVENT_LOCK_8_ACQ				60
#define XAIEGBL_MEM_EVENT_LOCK_8_REL				61
#define XAIEGBL_MEM_EVENT_LOCK_9_ACQ				62
#define XAIEGBL_MEM_EVENT_LOCK_9_REL				63
#define XAIEGBL_MEM_EVENT_LOCK_10_ACQ				64
#define XAIEGBL_MEM_EVENT_LOCK_10_REL				65
#define XAIEGBL_MEM_EVENT_LOCK_11_ACQ				66
#define XAIEGBL_MEM_EVENT_LOCK_11_REL				67
#define XAIEGBL_MEM_EVENT_LOCK_12_ACQ				68
#define XAIEGBL_MEM_EVENT_LOCK_12_REL				69
#define XAIEGBL_MEM_EVENT_LOCK_13_ACQ				70
#define XAIEGBL_MEM_EVENT_LOCK_13_REL				71
#define XAIEGBL_MEM_EVENT_LOCK_14_ACQ				72
#define XAIEGBL_MEM_EVENT_LOCK_14_REL				73
#define XAIEGBL_MEM_EVENT_LOCK_15_ACQ				74
#define XAIEGBL_MEM_EVENT_LOCK_15_REL				75
#define XAIEGBL_MEM_EVENT_GROUPORY_CONFLICT			76
#define XAIEGBL_MEM_EVENT_CONFLICT_DM_BANK_0			77
#define XAIEGBL_MEM_EVENT_CONFLICT_DM_BANK_1			78
#define XAIEGBL_MEM_EVENT_CONFLICT_DM_BANK_2			79
#define XAIEGBL_MEM_EVENT_CONFLICT_DM_BANK_3			80
#define XAIEGBL_MEM_EVENT_CONFLICT_DM_BANK_4			81
#define XAIEGBL_MEM_EVENT_CONFLICT_DM_BANK_5			82
#define XAIEGBL_MEM_EVENT_CONFLICT_DM_BANK_6			83
#define XAIEGBL_MEM_EVENT_CONFLICT_DM_BANK_7			84
#define XAIEGBL_MEM_EVENT_GROUP_ERRORS				86
#define XAIEGBL_MEM_EVENT_DM_ECC_ERROR_SCRUB_CORRECTED		87
#define XAIEGBL_MEM_EVENT_DM_ECC_ERROR_SCRUB_2BIT		88
#define XAIEGBL_MEM_EVENT_DM_ECC_ERROR_1BIT			89
#define XAIEGBL_MEM_EVENT_DM_ECC_ERROR_2BIT			90
#define XAIEGBL_MEM_EVENT_DM_PARITY_ERROR_BANK_2		91
#define XAIEGBL_MEM_EVENT_DM_PARITY_ERROR_BANK_3		92
#define XAIEGBL_MEM_EVENT_DM_PARITY_ERROR_BANK_4		93
#define XAIEGBL_MEM_EVENT_DM_PARITY_ERROR_BANK_5		94
#define XAIEGBL_MEM_EVENT_DM_PARITY_ERROR_BANK_6		95
#define XAIEGBL_MEM_EVENT_DM_PARITY_ERROR_BANK_7		96
#define XAIEGBL_MEM_EVENT_DMA_S2MM_0_ERROR			97
#define XAIEGBL_MEM_EVENT_DMA_S2MM_1_ERROR			98
#define XAIEGBL_MEM_EVENT_DMA_MM2S_0_ERROR			99
#define XAIEGBL_MEM_EVENT_DMA_MM2S_1_ERROR			100
#define XAIEGBL_MEM_EVENT_GROUP_BROADCAST			106
#define XAIEGBL_MEM_EVENT_BROADCAST_0				107
#define XAIEGBL_MEM_EVENT_BROADCAST_1				108
#define XAIEGBL_MEM_EVENT_BROADCAST_2				109
#define XAIEGBL_MEM_EVENT_BROADCAST_3				110
#define XAIEGBL_MEM_EVENT_BROADCAST_4				111
#define XAIEGBL_MEM_EVENT_BROADCAST_5				112
#define XAIEGBL_MEM_EVENT_BROADCAST_6				113
#define XAIEGBL_MEM_EVENT_BROADCAST_7				114
#define XAIEGBL_MEM_EVENT_BROADCAST_8				115
#define XAIEGBL_MEM_EVENT_BROADCAST_9				116
#define XAIEGBL_MEM_EVENT_BROADCAST_10				117
#define XAIEGBL_MEM_EVENT_BROADCAST_11				118
#define XAIEGBL_MEM_EVENT_BROADCAST_12				119
#define XAIEGBL_MEM_EVENT_BROADCAST_13				120
#define XAIEGBL_MEM_EVENT_BROADCAST_14				121
#define XAIEGBL_MEM_EVENT_BROADCAST_15				122
#define XAIEGBL_MEM_EVENT_GROUP_USER_EVENT			123
#define XAIEGBL_MEM_EVENT_USER_EVENT_0				124
#define XAIEGBL_MEM_EVENT_USER_EVENT_1				125
#define XAIEGBL_MEM_EVENT_USER_EVENT_2				126
#define XAIEGBL_MEM_EVENT_USER_EVENT_3				127

/* Declaration of events for PL/ Shim module */
#define XAIEGBL_PL_EVENT_NONE					0
#define XAIEGBL_PL_EVENT_TRUE					1
#define XAIEGBL_PL_EVENT_GROUP_0				2
#define XAIEGBL_PL_EVENT_TIMER_SYNC				3
#define XAIEGBL_PL_EVENT_TIMER_VALUE_REACHED			4
#define XAIEGBL_PL_EVENT_PERF_CNT_0				5
#define XAIEGBL_PL_EVENT_PERF_CNT_1				6
#define XAIEGBL_PL_EVENT_COMBO_EVENT_0				7
#define XAIEGBL_PL_EVENT_COMBO_EVENT_1				8
#define XAIEGBL_PL_EVENT_COMBO_EVENT_2				9
#define XAIEGBL_PL_EVENT_COMBO_EVENT_3				10
#define XAIEGBL_PL_EVENT_GROUP_DMA_ACTIVITY			11
#define XAIEGBL_PL_EVENT_DMA_S2MM_0_START_BD			12
#define XAIEGBL_PL_EVENT_DMA_S2MM_1_START_BD			13
#define XAIEGBL_PL_EVENT_DMA_MM2S_0_START_BD			14
#define XAIEGBL_PL_EVENT_DMA_MM2S_1_START_BD			15
#define XAIEGBL_PL_EVENT_DMA_S2MM_0_FINISHED_BD			16
#define XAIEGBL_PL_EVENT_DMA_S2MM_1_FINISHED_BD			17
#define XAIEGBL_PL_EVENT_DMA_MM2S_0_FINISHED_BD			18
#define XAIEGBL_PL_EVENT_DMA_MM2S_1_FINISHED_BD			19
#define XAIEGBL_PL_EVENT_DMA_S2MM_0_GO_TO_IDLE			20
#define XAIEGBL_PL_EVENT_DMA_S2MM_1_GO_TO_IDLE			21
#define XAIEGBL_PL_EVENT_DMA_MM2S_0_GO_TO_IDLE			22
#define XAIEGBL_PL_EVENT_DMA_MM2S_1_GO_TO_IDLE			23
#define XAIEGBL_PL_EVENT_DMA_S2MM_0_STALLED_LOCK_ACQUIRE	24
#define XAIEGBL_PL_EVENT_DMA_S2MM_1_STALLED_LOCK_ACQUIRE	25
#define XAIEGBL_PL_EVENT_DMA_MM2S_0_STALLED_LOCK_ACQUIRE	26
#define XAIEGBL_PL_EVENT_DMA_MM2S_1_STALLED_LOCK_ACQUIRE	27
#define XAIEGBL_PL_EVENT_GROUP_LOCK				28
#define XAIEGBL_PL_EVENT_LOCK_0_ACQUIRED			29
#define XAIEGBL_PL_EVENT_LOCK_0_RELEASED			30
#define XAIEGBL_PL_EVENT_LOCK_1_ACQUIRED			31
#define XAIEGBL_PL_EVENT_LOCK_1_RELEASED			32
#define XAIEGBL_PL_EVENT_LOCK_2_ACQUIRED			33
#define XAIEGBL_PL_EVENT_LOCK_2_RELEASED			34
#define XAIEGBL_PL_EVENT_LOCK_3_ACQUIRED			35
#define XAIEGBL_PL_EVENT_LOCK_3_RELEASED			36
#define XAIEGBL_PL_EVENT_LOCK_4_ACQUIRED			37
#define XAIEGBL_PL_EVENT_LOCK_4_RELEASED			38
#define XAIEGBL_PL_EVENT_LOCK_5_ACQUIRED			39
#define XAIEGBL_PL_EVENT_LOCK_5_RELEASED			40
#define XAIEGBL_PL_EVENT_LOCK_6_ACQUIRED			41
#define XAIEGBL_PL_EVENT_LOCK_6_RELEASED			42
#define XAIEGBL_PL_EVENT_LOCK_7_ACQUIRED			43
#define XAIEGBL_PL_EVENT_LOCK_7_RELEASED			44
#define XAIEGBL_PL_EVENT_LOCK_8_ACQUIRED			45
#define XAIEGBL_PL_EVENT_LOCK_8_RELEASED			46
#define XAIEGBL_PL_EVENT_LOCK_9_ACQUIRED			47
#define XAIEGBL_PL_EVENT_LOCK_9_RELEASED			48
#define XAIEGBL_PL_EVENT_LOCK_10_ACQUIRED			49
#define XAIEGBL_PL_EVENT_LOCK_10_RELEASED			50
#define XAIEGBL_PL_EVENT_LOCK_11_ACQUIRED			51
#define XAIEGBL_PL_EVENT_LOCK_11_RELEASED			52
#define XAIEGBL_PL_EVENT_LOCK_12_ACQUIRED			53
#define XAIEGBL_PL_EVENT_LOCK_12_RELEASED			54
#define XAIEGBL_PL_EVENT_LOCK_13_ACQUIRED			55
#define XAIEGBL_PL_EVENT_LOCK_13_RELEASED			56
#define XAIEGBL_PL_EVENT_LOCK_14_ACQUIRED			57
#define XAIEGBL_PL_EVENT_LOCK_14_RELEASED			58
#define XAIEGBL_PL_EVENT_LOCK_15_ACQUIRED			59
#define XAIEGBL_PL_EVENT_LOCK_15_RELEASED			60
#define XAIEGBL_PL_EVENT_GROUP_ERRORS				61
#define XAIEGBL_PL_EVENT_AXI_MM_SLAVE_TILE_ERROR		62
#define XAIEGBL_PL_EVENT_CONTROL_PKT_ERROR			63
#define XAIEGBL_PL_EVENT_AXI_MM_DECODE_NSU_ERROR		64
#define XAIEGBL_PL_EVENT_AXI_MM_SLAVE_NSU_ERROR			65
#define XAIEGBL_PL_EVENT_AXI_MM_UNSUPPORTED_TRAFFIC		66
#define XAIEGBL_PL_EVENT_AXI_MM_UNSECURE_ACCESS_IN_SECURE_MODE	67
#define XAIEGBL_PL_EVENT_AXI_MM_BYTE_STROBE_ERROR		68
#define XAIEGBL_PL_EVENT_DMA_S2MM_0_ERROR			69
#define XAIEGBL_PL_EVENT_DMA_S2MM_1_ERROR			70
#define XAIEGBL_PL_EVENT_DMA_MM2S_0_ERROR			71
#define XAIEGBL_PL_EVENT_DMA_MM2S_1_ERROR			72
#define XAIEGBL_PL_EVENT_GROUP_STREAM_SWITCH			73
#define XAIEGBL_PL_EVENT_PORT_IDLE_0				74
#define XAIEGBL_PL_EVENT_PORT_RUNNING_0				75
#define XAIEGBL_PL_EVENT_PORT_STALLED_0				76
#define XAIEGBL_PL_EVENT_PORT_TLAST_0				77
#define XAIEGBL_PL_EVENT_PORT_IDLE_1				78
#define XAIEGBL_PL_EVENT_PORT_RUNNING_1				79
#define XAIEGBL_PL_EVENT_PORT_STALLED_1				80
#define XAIEGBL_PL_EVENT_PORT_TLAST_1				81
#define XAIEGBL_PL_EVENT_PORT_IDLE_2				82
#define XAIEGBL_PL_EVENT_PORT_RUNNING_2				83
#define XAIEGBL_PL_EVENT_PORT_STALLED_2				84
#define XAIEGBL_PL_EVENT_PORT_TLAST_2				85
#define XAIEGBL_PL_EVENT_PORT_IDLE_3				86
#define XAIEGBL_PL_EVENT_PORT_RUNNING_3				87
#define XAIEGBL_PL_EVENT_PORT_STALLED_3				88
#define XAIEGBL_PL_EVENT_PORT_TLAST_3				89
#define XAIEGBL_PL_EVENT_PORT_IDLE_4				90
#define XAIEGBL_PL_EVENT_PORT_RUNNING_4				91
#define XAIEGBL_PL_EVENT_PORT_STALLED_4				92
#define XAIEGBL_PL_EVENT_PORT_TLAST_4				93
#define XAIEGBL_PL_EVENT_PORT_IDLE_5				94
#define XAIEGBL_PL_EVENT_PORT_RUNNING_5				95
#define XAIEGBL_PL_EVENT_PORT_STALLED_5				96
#define XAIEGBL_PL_EVENT_PORT_TLAST_5				97
#define XAIEGBL_PL_EVENT_PORT_IDLE_6				98
#define XAIEGBL_PL_EVENT_PORT_RUNNING_6				99
#define XAIEGBL_PL_EVENT_PORT_STALLED_6				100
#define XAIEGBL_PL_EVENT_PORT_TLAST_6				101
#define XAIEGBL_PL_EVENT_PORT_IDLE_7				102
#define XAIEGBL_PL_EVENT_PORT_RUNNING_7				103
#define XAIEGBL_PL_EVENT_PORT_STALLED_7				104
#define XAIEGBL_PL_EVENT_PORT_TLAST_7				105
#define XAIEGBL_PL_EVENT_GROUP_BROADCAST_A			106
#define XAIEGBL_PL_EVENT_BROADCAST_0				107
#define XAIEGBL_PL_EVENT_BROADCAST_A_1				108
#define XAIEGBL_PL_EVENT_BROADCAST_A_2				109
#define XAIEGBL_PL_EVENT_BROADCAST_A_3				110
#define XAIEGBL_PL_EVENT_BROADCAST_A_4				111
#define XAIEGBL_PL_EVENT_BROADCAST_A_5				112
#define XAIEGBL_PL_EVENT_BROADCAST_A_6				113
#define XAIEGBL_PL_EVENT_BROADCAST_A_7				114
#define XAIEGBL_PL_EVENT_BROADCAST_A_8				115
#define XAIEGBL_PL_EVENT_BROADCAST_A_9				116
#define XAIEGBL_PL_EVENT_BROADCAST_A_10				117
#define XAIEGBL_PL_EVENT_BROADCAST_A_11				118
#define XAIEGBL_PL_EVENT_BROADCAST_A_12				119
#define XAIEGBL_PL_EVENT_BROADCAST_A_13				120
#define XAIEGBL_PL_EVENT_BROADCAST_A_14				121
#define XAIEGBL_PL_EVENT_BROADCAST_A_15				122
#define XAIEGBL_PL_EVENT_GROUP_USER_EVENT			123
#define XAIEGBL_PL_EVENT_USER_EVENT_0				124
#define XAIEGBL_PL_EVENT_USER_EVENT_1				125
#define XAIEGBL_PL_EVENT_USER_EVENT_2				126
#define XAIEGBL_PL_EVENT_USER_EVENT_3				127
/** @} */
