/******************************************************************************
*
* Copyright (C) 2005 - 2019 Xilinx, Inc.  All rights reserved.
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
* @file xaiegbl_reginit.c
* @{
*
* This file contains the instances of the register bit field definitions for the
* Core, Memory, NoC and PL module registers.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   10/22/2019  Initial creation
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xaiegbl.h"
#include "xaiegbl_params.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/**************************** Macro Definitions ******************************/

/************************** Variable Definitions *****************************/
/*
 * Global instance for Core module Core_Control register.
 */
const static XAie_RegCoreCtrl AieCoreCtrlReg =
{
	XAIEGBL_CORE_CORECTRL,
	{XAIEGBL_CORE_CORECTRL_ENA_LSB, XAIEGBL_CORE_CORECTRL_ENA_MASK},
	{XAIEGBL_CORE_CORECTRL_RST_LSB, XAIEGBL_CORE_CORECTRL_RST_MASK}
};

/*
 * Global instance for Core module Core_Status register.
 */
const static XAie_RegCoreSts AieCoreStsReg =
{
	XAIEGBL_CORE_CORESTA,
	{XAIEGBL_CORE_CORESTA_COREDON_LSB, XAIEGBL_CORE_CORESTA_COREDON_MASK},
	{XAIEGBL_CORE_CORESTA_RST_LSB, XAIEGBL_CORE_CORESTA_RST_MASK},
	{XAIEGBL_CORE_CORESTA_ENA_LSB, XAIEGBL_CORE_CORESTA_ENA_MASK}
};

/*
 * Array of all Tile Stream Switch Master Config registers
 * The data structure contains number of ports and the register offsets
 */
const static XAie_StrmPort AieTileStrmMstr[SS_PORT_TYPE_MAX] =
{
	{	/* Core */
		2,
		XAIEGBL_CORE_STRSWIMSTRCFGMECORE0,
	},
	{	/* DMA */
		2,
		XAIEGBL_CORE_STRSWIMSTRCFGDMA0,
	},
	{	/* Ctrl */
		1,
		XAIEGBL_CORE_STRSWIMSTRCFGTILCTR,
	},
	{	/* Fifo */
		2,
		XAIEGBL_CORE_STRSWIMSTRCFGFIF0,
	},
	{	/* South */
		4,
		XAIEGBL_CORE_STRSWIMSTRCFGSOU0,
	},
	{	/* West */
		4,
		XAIEGBL_CORE_STRSWIMSTRCFGWES0,
	},
	{	/* North */
		6,
		XAIEGBL_CORE_STRSWIMSTRCFGNOR0,
	},
	{	/* East */
		4,
		XAIEGBL_CORE_STRSWIMSTRCFGEAS0,
	},
	{	/* Trace */
		0,
		0
	}
};

/*
 * Array of all Tile Stream Switch Slave Config registers
 * The data structure contains number of ports and the register offsets
 */
const static XAie_StrmPort AieTileStrmSlv[SS_PORT_TYPE_MAX] =
{
	{	/* Core */
		2,
		XAIEGBL_CORE_STRSWISLVMECORE0CFG,
	},
	{	/* DMA */
		2,
		XAIEGBL_CORE_STRSWISLVDMA0CFG,
	},
	{	/* Ctrl */
		1,
		XAIEGBL_CORE_STRSWISLVTILCTRCFG,
	},
	{	/* Fifo */
		2,
		XAIEGBL_CORE_STRSWISLVFIF0CFG,
	},
	{	/* South */
		6,
		XAIEGBL_CORE_STRSWISLVSOU0CFG,
	},
	{	/* West */
		4,
		XAIEGBL_CORE_STRSWISLVWES0CFG,
	},
	{	/* North */
		4,
		XAIEGBL_CORE_STRSWISLVNOR0CFG,
	},
	{	/* East */
		4,
		XAIEGBL_CORE_STRSWISLVEAS0CFG,
	},
	{	/* Trace */
		1,
		XAIEGBL_CORE_STRSWISLVMETRACFG
	}
};

/*
 * Array of all Shim NOC/PL Stream Switch Master Config registers
 * The data structure contains number of ports and the register offsets
 */
const static XAie_StrmPort AieShimStrmMstr[SS_PORT_TYPE_MAX] =
{
	{	/* Core */
		0,
		0,
	},
	{	/* DMA */
		0,
		0,
	},
	{	/* Ctrl */
		1,
		XAIEGBL_PL_STRSWIMSTRCFGTILCTR,
	},
	{	/* Fifo */
		2,
		XAIEGBL_PL_STRSWIMSTRCFGFIF0,
	},
	{	/* South */
		6,
		XAIEGBL_PL_STRSWIMSTRCFGSOU0,
	},
	{	/* West */
		4,
		XAIEGBL_PL_STRSWIMSTRCFGWES0,
	},
	{	/* North */
		6,
		XAIEGBL_PL_STRSWIMSTRCFGNOR0,
	},
	{	/* East */
		4,
		XAIEGBL_PL_STRSWIMSTRCFGEAS0,
	},
	{	/* Trace */
		0,
		0
	}
};

/*
 * Array of all Shim NOC/PL Stream Switch Slave Config registers
 * The data structure contains number of ports and the register offsets
 */
const static XAie_StrmPort AieShimStrmSlv[SS_PORT_TYPE_MAX] =
{
	{	/* Core */
		0,
		0,
	},
	{	/* DMA */
		0,
		0,
	},
	{	/* Ctrl */
		1,
		XAIEGBL_PL_STRSWISLVTILCTRCFG,
	},
	{	/* Fifo */
		2,
		XAIEGBL_PL_STRSWISLVFIF0CFG,
	},
	{	/* South */
		8,
		XAIEGBL_PL_STRSWISLVSOU0CFG,
	},
	{	/* West */
		4,
		XAIEGBL_PL_STRSWISLVWES0CFG,
	},
	{	/* North */
		4,
		XAIEGBL_PL_STRSWISLVNOR0CFG,
	},
	{	/* East */
		4,
		XAIEGBL_PL_STRSWISLVEAS0CFG,
	},
	{	/* Trace */
		1,
		XAIEGBL_PL_STRSWISLVTRACFG,
	}
};

/*
 * Data structure to capture all stream configs for XAIEGBL_TILE_TYPE_AIETILE
 */
const static XAie_StrmMod AieTileStrmSw =
{
	XAIEGBL_CORE_STRSWISLVMECORE0CFG,
	0x4,
	{XAIEGBL_CORE_STRSWIMSTRCFGMECORE0_MSTRENA_LSB, XAIEGBL_CORE_STRSWIMSTRCFGMECORE0_MSTRENA_MASK},
	{XAIEGBL_CORE_STRSWIMSTRCFGMECORE0_PKTENA_LSB, XAIEGBL_CORE_STRSWIMSTRCFGMECORE0_PKTENA_MASK},
	{XAIEGBL_CORE_STRSWIMSTRCFGMECORE0_DROHEA_LSB, XAIEGBL_CORE_STRSWIMSTRCFGMECORE0_DROHEA_MASK},
	{XAIEGBL_CORE_STRSWIMSTRCFGMECORE0_CON_LSB, XAIEGBL_CORE_STRSWIMSTRCFGMECORE0_CON_MASK},
	{XAIEGBL_CORE_STRSWISLVMECORE0CFG_SLVENA_LSB, XAIEGBL_CORE_STRSWISLVMECORE0CFG_SLVENA_MASK},
	{XAIEGBL_CORE_STRSWISLVMECORE0CFG_PKTENA_LSB, XAIEGBL_CORE_STRSWISLVMECORE0CFG_PKTENA_MASK},
	AieTileStrmMstr,
	AieTileStrmSlv
};

/*
 * Data structure to capture all stream configs for XAIEGBL_TILE_TYPE_SHIMNOC/PL
 */
const static XAie_StrmMod AieShimStrmSw =
{
	XAIEGBL_PL_STRSWISLVTILCTRCFG,
	0x4,
	{XAIEGBL_CORE_STRSWIMSTRCFGMECORE0_MSTRENA_LSB, XAIEGBL_CORE_STRSWIMSTRCFGMECORE0_MSTRENA_MASK},
	{XAIEGBL_CORE_STRSWIMSTRCFGMECORE0_PKTENA_LSB, XAIEGBL_CORE_STRSWIMSTRCFGMECORE0_PKTENA_MASK},
	{XAIEGBL_CORE_STRSWIMSTRCFGMECORE0_DROHEA_LSB, XAIEGBL_CORE_STRSWIMSTRCFGMECORE0_DROHEA_MASK},
	{XAIEGBL_CORE_STRSWIMSTRCFGMECORE0_CON_LSB, XAIEGBL_CORE_STRSWIMSTRCFGMECORE0_CON_MASK},
	{XAIEGBL_CORE_STRSWISLVMECORE0CFG_SLVENA_LSB, XAIEGBL_CORE_STRSWISLVMECORE0CFG_SLVENA_MASK},
	{XAIEGBL_CORE_STRSWISLVMECORE0CFG_PKTENA_LSB, XAIEGBL_CORE_STRSWISLVMECORE0CFG_PKTENA_MASK},
	AieShimStrmMstr,
	AieShimStrmSlv
};

/* Core Module */
const static XAie_CoreMod AieCoreMod =
{
	.ProgMemAddr = XAIEGBL_CORE_PRGMEM,
	.CoreCtrl = &AieCoreCtrlReg,
	.CoreSts = &AieCoreStsReg,
	.CoreMemSize = 32 * 1024,
	.CoreEastAddrStart = 0x38000,
	.CoreEastAddrEnd = 0x3FFFF,
	.CoreWestAddrStart = 0x28000,
	.CoreWestAddrEnd = 0x2FFFF,
	.CoreSouthAddrStart = 0x20000,
	.CoreSouthAddrEnd = 0x27FFF,
	.CoreNorthAddrStart = 0x30000,
	.CoreNorthAddrEnd = 0x37FFF,
};

/* Data Memory Module for Tile data memory*/
const static XAie_MemMod AieTileMemMod =
{
	.Size = 32 * 1024,
	.MemAddr = XAIEGBL_MEM_DATMEM
};

/*
 * AIE Module
 * This data structure captures all the modules for each tile type.
 * Depending on the tile type, this data strcuture can be used to access all
 * hardware properties of individual modules.
 */
XAie_TileMod AieMod[] =
{
	{
		/*
		 * AIE Tile Module indexed using XAIEGBL_TILE_TYPE_AIETILE
		 */
		.CoreMod = &AieCoreMod,
		.StrmSw  = &AieTileStrmSw,
		.DmaMod  = NULL,
		.MemMod  = &AieTileMemMod,
	},
	{
		/*
		 * AIE Shim Noc Module indexed using XAIEGBL_TILE_TYPE_SHIMNOC
		 */
		.CoreMod = NULL,
		.StrmSw  = &AieShimStrmSw,
		.DmaMod  = NULL,
		.MemMod  = NULL,
	},
	{
		/*
		 * AIE Shim PL Module indexed using XAIEGBL_TILE_TYPE_SHIMPL
		 */
		.CoreMod = NULL,
		.StrmSw  = &AieShimStrmSw,
		.DmaMod  = NULL,
		.MemMod  = NULL,
	},
	{
		/*
		 * AIE MemTile Module indexed using XAIEGBL_TILE_TYPE_MEMTILE
		 */
		.CoreMod = NULL,
		.StrmSw  = NULL,
		.DmaMod  = NULL,
		.MemMod  = NULL,
	}
};

/** @} */
