/******************************************************************************
* Copyright (C) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file utest.c
* @{
*
* This file contains unit test driver for .
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who           Date     Changes
* ----- ----------   -------- -----------------------------------------------------
* 1.0   siyuan sun   09/24/2019  Initial creation

* </pre>
*
******************************************************************************/


#define XAIE_DEBUG

#include "aie_hw_config.h"



#define XAIE_RES_TILE_ROW_START 0x20
#define XAIE_RES_TILE_NUM_ROWS  3
/*
 * Example op to send ifm, ofm.
 */
typedef struct {
    u64 ifm[6];
    u64 ofm[6];
    u32 flags;
} __attribute__((packed, aligned(4))) dpu_op_1;

/*
 * Example op for register poll;
 */
typedef struct {
    u32 addr;
    u32 timeout;
}  __attribute__((packed, aligned(4))) dpu_op_2;


int main() {

    /* Initialize aie driver. */
    XAie_SetupConfig(ConfigPtr, XAIE_DEV_GEN_AIE, XAIE_BASE_ADDR,
            XAIE_COL_SHIFT, XAIE_ROW_SHIFT,
            XAIE_NUM_COLS, XAIE_NUM_ROWS, XAIE_SHIM_ROW,
            XAIE_RES_TILE_ROW_START, XAIE_RES_TILE_NUM_ROWS,
            XAIE_AIE_TILE_ROW_START, XAIE_AIE_TILE_NUM_ROWS);

    XAie_InstDeclare(DevInst, &ConfigPtr);

    XAie_CfgInitialize(&DevInst, &ConfigPtr);

    XAie_StartTransaction(&DevInst,XAIE_TRANSACTION_DISABLE_AUTO_FLUSH);

    int Op1 = XAie_RequestCustomTxnOp(&DevInst);
    int Op2 = XAie_RequestCustomTxnOp(&DevInst);

    if( -1 != Op1) {

        dpu_op_1 dpu_op1;
        dpu_op1.ifm[0] = 0xDEADBEEF;
        dpu_op1.ofm[0] = 0xBEEFDEAD;
        dpu_op1.flags = 1 << 5;

         /*test hexdump with the same function*/
        printf("UNIT TEST LOG BEGIN>>>");
        BuffHexDump(&dpu_op1,sizeof(dpu_op_1));
        printf("\n<<<UNIT TEST LOG END");

        /* test case for run custom op */
        XAie_AddCustomTxnOp(&DevInst, Op1, &dpu_op1, sizeof(dpu_op_1));

    }

    if( -1 != Op2) {
        dpu_op_2 dpu_op2;
        dpu_op2.addr = 0xAAAABBBB;
        dpu_op2.timeout = 5;

        /*test hexdump with the same function*/
        printf("UNIT TEST LOG BEGIN>>>");
        BuffHexDump(&dpu_op2,sizeof(dpu_op_2));
        printf("\n<<<UNIT TEST LOG END");

        /* test case for run custom op */
        XAie_AddCustomTxnOp(&DevInst, Op2, &dpu_op2, sizeof(dpu_op_2));

        /* test case for invalid op code */
        XAie_AddCustomTxnOp(&DevInst, 254, &dpu_op2, sizeof(dpu_op_2));
    }
    /* test case for serialization */
    _XAie_TxnExportSerialized(&DevInst,0,0);
    _XAie_ClearTransaction(&DevInst);
    _XAie_TxnExportSerialized(&DevInst,0,0);

}