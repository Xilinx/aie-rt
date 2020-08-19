/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
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
* 1.1   Tejus   10/28/2019  Add pl interface register properties
* 1.2   Tejus   12/09/2019  Include correct header file to avoid cyclic
*			    dependancy
* 1.3   Tejus   03/16/2020  Seperate PlIf Module for SHIMPL and SHIMNOC Tiles
* 1.4   Tejus   03/16/2020  Add register properties for Mux/Demux registers
* 1.5   Tejus   03/17/2020  Add lock module properties
* 1.6   Tejus   03/21/2020  Add structure fields to stream switch module
*			    definition
* 1.7   Tejus   03/21/2020  Add register properties for stream switch slot
*			    registers
* 1.8   Tejus   03/23/2020  Organize header files in alphabetical order
* 1.9   Tejus   03/23/2020  Add register properties for dmas
* 2.0   Dishita 03/24/2020  Add performance counter properties
* 2.1   Dishita 04/16/2020  Fix compiler warnings
* 2.2   Dishita 04/20/2020  Add timer properties
* 2.3   Tejus   05/26/2020  Restructure and optimize core module.
* 2.4   Tejus   06/01/2020  Add data structure for core debug register.
* 2.5   Nishad  06/02/2020  Rename included header file xaiegbl_events to
*			    xaie_events_aie
* 2.6   Nishad  06/03/2020  Rename XAIEGBL_<MODULE>_EVENT_* macros to
*			    XAIE_EVENTS_<MODULE>_*
* 2.7   Nishad  06/09/2020  Fix typo in *_MEMORY_* event macros
* 2.8   Tejus   06/05/2020  Populate fifo mode availability in data structure.
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xaie_dma_aie.h"
#include "xaie_events.h"
#include "xaie_events_aie.h"
#include "xaie_locks_aie.h"
#include "xaiegbl_regdef.h"
#include "xaiegbl_params.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/**************************** Macro Definitions ******************************/

/************************** Variable Definitions *****************************/
/*
 * Global instance for Core module Core_Control register.
 */
static const XAie_RegCoreCtrl AieCoreCtrlReg =
{
	XAIEGBL_CORE_CORECTRL,
	{XAIEGBL_CORE_CORECTRL_ENA_LSB, XAIEGBL_CORE_CORECTRL_ENA_MASK},
	{XAIEGBL_CORE_CORECTRL_RST_LSB, XAIEGBL_CORE_CORECTRL_RST_MASK}
};

/*
 * Global instance for Core module Core_Status register.
 */
static const XAie_RegCoreSts AieCoreStsReg =
{
	XAIEGBL_CORE_CORESTA,
	{XAIEGBL_CORE_CORESTA_COREDON_LSB, XAIEGBL_CORE_CORESTA_COREDON_MASK},
	{XAIEGBL_CORE_CORESTA_RST_LSB, XAIEGBL_CORE_CORESTA_RST_MASK},
	{XAIEGBL_CORE_CORESTA_ENA_LSB, XAIEGBL_CORE_CORESTA_ENA_MASK}
};

/*
 * Global instance for Core module for core debug registers.
 */
static const XAie_RegCoreDebug AieCoreDebugReg =
{
	.RegOff = XAIEGBL_CORE_DBGCTRL0,
	.DebugHalt.Lsb = XAIEGBL_CORE_DBGCTRL0_DBGHLTBIT_LSB,
	.DebugHalt.Mask = XAIEGBL_CORE_DBGCTRL0_DBGHLTBIT_MASK
};

/*
 * Array of all Tile Stream Switch Master Config registers
 * The data structure contains number of ports and the register offsets
 */
static const XAie_StrmPort AieTileStrmMstr[SS_PORT_TYPE_MAX] =
{
	{	/* Core */
		.NumPorts = 2,
		.PortBaseAddr = XAIEGBL_CORE_STRSWIMSTRCFGMECORE0,
	},
	{	/* DMA */
		.NumPorts = 2,
		.PortBaseAddr = XAIEGBL_CORE_STRSWIMSTRCFGDMA0,
	},
	{	/* Ctrl */
		.NumPorts = 1,
		.PortBaseAddr = XAIEGBL_CORE_STRSWIMSTRCFGTILCTR,
	},
	{	/* Fifo */
		.NumPorts = 2,
		.PortBaseAddr = XAIEGBL_CORE_STRSWIMSTRCFGFIF0,
	},
	{	/* South */
		.NumPorts = 4,
		.PortBaseAddr = XAIEGBL_CORE_STRSWIMSTRCFGSOU0,
	},
	{	/* West */
		.NumPorts = 4,
		.PortBaseAddr = XAIEGBL_CORE_STRSWIMSTRCFGWES0,
	},
	{	/* North */
		.NumPorts = 6,
		.PortBaseAddr = XAIEGBL_CORE_STRSWIMSTRCFGNOR0,
	},
	{	/* East */
		.NumPorts = 4,
		.PortBaseAddr = XAIEGBL_CORE_STRSWIMSTRCFGEAS0,
	},
	{	/* Trace */
		.NumPorts = 0,
		.PortBaseAddr = 0
	}
};

/*
 * Array of all Tile Stream Switch Slave Config registers
 * The data structure contains number of ports and the register offsets
 */
static const XAie_StrmPort AieTileStrmSlv[SS_PORT_TYPE_MAX] =
{
	{	/* Core */
		.NumPorts = 2,
		.PortBaseAddr = XAIEGBL_CORE_STRSWISLVMECORE0CFG,
	},
	{	/* DMA */
		.NumPorts = 2,
		.PortBaseAddr = XAIEGBL_CORE_STRSWISLVDMA0CFG,
	},
	{	/* Ctrl */
		.NumPorts = 1,
		.PortBaseAddr = XAIEGBL_CORE_STRSWISLVTILCTRCFG,
	},
	{	/* Fifo */
		.NumPorts = 2,
		.PortBaseAddr = XAIEGBL_CORE_STRSWISLVFIF0CFG,
	},
	{	/* South */
		.NumPorts = 6,
		.PortBaseAddr = XAIEGBL_CORE_STRSWISLVSOU0CFG,
	},
	{	/* West */
		.NumPorts = 4,
		.PortBaseAddr = XAIEGBL_CORE_STRSWISLVWES0CFG,
	},
	{	/* North */
		.NumPorts = 4,
		.PortBaseAddr = XAIEGBL_CORE_STRSWISLVNOR0CFG,
	},
	{	/* East */
		.NumPorts = 4,
		.PortBaseAddr = XAIEGBL_CORE_STRSWISLVEAS0CFG,
	},
	{	/* Trace */
		.NumPorts = 1,
		.PortBaseAddr = XAIEGBL_CORE_STRSWISLVMETRACFG
	}
};

/*
 * Array of all Shim NOC/PL Stream Switch Master Config registers
 * The data structure contains number of ports and the register offsets
 */
static const XAie_StrmPort AieShimStrmMstr[SS_PORT_TYPE_MAX] =
{
	{	/* Core */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* DMA */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* Ctrl */
		.NumPorts = 1,
		.PortBaseAddr = XAIEGBL_PL_STRSWIMSTRCFGTILCTR,
	},
	{	/* Fifo */
		.NumPorts = 2,
		.PortBaseAddr = XAIEGBL_PL_STRSWIMSTRCFGFIF0,
	},
	{	/* South */
		.NumPorts = 6,
		.PortBaseAddr = XAIEGBL_PL_STRSWIMSTRCFGSOU0,
	},
	{	/* West */
		.NumPorts = 4,
		.PortBaseAddr = XAIEGBL_PL_STRSWIMSTRCFGWES0,
	},
	{	/* North */
		.NumPorts = 6,
		.PortBaseAddr = XAIEGBL_PL_STRSWIMSTRCFGNOR0,
	},
	{	/* East */
		.NumPorts = 4,
		.PortBaseAddr = XAIEGBL_PL_STRSWIMSTRCFGEAS0,
	},
	{	/* Trace */
		.NumPorts = 0,
		.PortBaseAddr = 0
	}
};

/*
 * Array of all Shim NOC/PL Stream Switch Slave Config registers
 * The data structure contains number of ports and the register offsets
 */
static const XAie_StrmPort AieShimStrmSlv[SS_PORT_TYPE_MAX] =
{
	{	/* Core */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* DMA */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* Ctrl */
		.NumPorts = 1,
		.PortBaseAddr = XAIEGBL_PL_STRSWISLVTILCTRCFG,
	},
	{	/* Fifo */
		.NumPorts = 2,
		.PortBaseAddr = XAIEGBL_PL_STRSWISLVFIF0CFG,
	},
	{	/* South */
		.NumPorts = 8,
		.PortBaseAddr = XAIEGBL_PL_STRSWISLVSOU0CFG,
	},
	{	/* West */
		.NumPorts = 4,
		.PortBaseAddr = XAIEGBL_PL_STRSWISLVWES0CFG,
	},
	{	/* North */
		.NumPorts = 4,
		.PortBaseAddr = XAIEGBL_PL_STRSWISLVNOR0CFG,
	},
	{	/* East */
		.NumPorts = 4,
		.PortBaseAddr = XAIEGBL_PL_STRSWISLVEAS0CFG,
	},
	{	/* Trace */
		.NumPorts = 1,
		.PortBaseAddr = XAIEGBL_PL_STRSWISLVTRACFG,
	}
};

/*
 * Array of all Shim NOC/PL Stream Switch Slave Slot Config registers
 * The data structure contains number of ports and the register base address.
 */
static const XAie_StrmPort AieShimStrmSlaveSlot[SS_PORT_TYPE_MAX] =
{
	{	/* Core */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* DMA */
		.NumPorts = 0,
		.PortBaseAddr = 0,
	},
	{	/* Ctrl */
		.NumPorts = 1,
		.PortBaseAddr = XAIEGBL_PL_STRSWISLVTILCTRSLO0,
	},
	{	/* Fifo */
		.NumPorts = 2,
		.PortBaseAddr = XAIEGBL_PL_STRSWISLVFIF0SLO0,
	},
	{	/* South */
		.NumPorts = 8,
		.PortBaseAddr = XAIEGBL_PL_STRSWISLVSOU0SLO0,
	},
	{	/* West */
		.NumPorts = 4,
		.PortBaseAddr = XAIEGBL_PL_STRSWISLVWES0SLO0,
	},
	{	/* North */
		.NumPorts = 4,
		.PortBaseAddr = XAIEGBL_PL_STRSWISLVNOR0SLO0,
	},
	{	/* East */
		.NumPorts = 4,
		.PortBaseAddr = XAIEGBL_PL_STRSWISLVEAS0SLO0,
	},
	{	/* Trace */
		.NumPorts = 1,
		.PortBaseAddr = XAIEGBL_PL_STRSWISLVTRASLO0
	}
};

/*
 * Data structure to capture stream switch slave slot register base address for
 * AIE Tiles
 */
static const XAie_StrmPort AieTileStrmSlaveSlot[SS_PORT_TYPE_MAX] =
{
	{	/* Core */
		.NumPorts = 2,
		.PortBaseAddr = XAIEGBL_CORE_STRSWISLVMECORE0SLO0,
	},
	{	/* DMA */
		.NumPorts = 2,
		.PortBaseAddr = XAIEGBL_CORE_STRSWISLVDMA0SLO0,
	},
	{	/* Ctrl */
		.NumPorts = 1,
		.PortBaseAddr = XAIEGBL_CORE_STRSWISLVTILCTRSLO0,
	},
	{	/* Fifo */
		.NumPorts = 2,
		.PortBaseAddr = XAIEGBL_CORE_STRSWISLVFIF0SLO0,
	},
	{	/* South */
		.NumPorts = 6,
		.PortBaseAddr = XAIEGBL_CORE_STRSWISLVSOU0SLO0,
	},
	{	/* West */
		.NumPorts = 4,
		.PortBaseAddr = XAIEGBL_CORE_STRSWISLVWES0SLO0,
	},
	{	/* North */
		.NumPorts = 4,
		.PortBaseAddr = XAIEGBL_CORE_STRSWISLVNOR0SLO0,
	},
	{	/* East */
		.NumPorts = 4,
		.PortBaseAddr = XAIEGBL_CORE_STRSWISLVEAS0SLO0,
	},
	{	/* Trace */
		.NumPorts = 1,
		.PortBaseAddr = XAIEGBL_CORE_STRSWISLVMETRASLO0
	}
};

/*
 * Data structure to capture all stream configs for XAIEGBL_TILE_TYPE_AIETILE
 */
static const XAie_StrmMod AieTileStrmSw =
{
	.SlvConfigBaseAddr = XAIEGBL_CORE_STRSWISLVMECORE0CFG,
	.PortOffset = 0x4,
	.NumSlaveSlots = 4U,
	.SlotOffsetPerPort = 0x10,
	.SlotOffset = 0x4,
	.MstrEn = {XAIEGBL_CORE_STRSWIMSTRCFGMECORE0_MSTRENA_LSB, XAIEGBL_CORE_STRSWIMSTRCFGMECORE0_MSTRENA_MASK},
	.MstrPktEn = {XAIEGBL_CORE_STRSWIMSTRCFGMECORE0_PKTENA_LSB, XAIEGBL_CORE_STRSWIMSTRCFGMECORE0_PKTENA_MASK},
	.DrpHdr = {XAIEGBL_CORE_STRSWIMSTRCFGMECORE0_DROHEA_LSB, XAIEGBL_CORE_STRSWIMSTRCFGMECORE0_DROHEA_MASK},
	.Config = {XAIEGBL_CORE_STRSWIMSTRCFGMECORE0_CON_LSB, XAIEGBL_CORE_STRSWIMSTRCFGMECORE0_CON_MASK},
	.SlvEn = {XAIEGBL_CORE_STRSWISLVMECORE0CFG_SLVENA_LSB, XAIEGBL_CORE_STRSWISLVMECORE0CFG_SLVENA_MASK},
	.SlvPktEn = {XAIEGBL_CORE_STRSWISLVMECORE0CFG_PKTENA_LSB, XAIEGBL_CORE_STRSWISLVMECORE0CFG_PKTENA_MASK},
	.SlotPktId = {XAIEGBL_CORE_STRSWISLVMECORE0SLO0_ID_LSB, XAIEGBL_CORE_STRSWISLVMECORE0SLO0_ID_MASK},
	.SlotMask = {XAIEGBL_CORE_STRSWISLVMECORE0SLO0_MSK_LSB, XAIEGBL_CORE_STRSWISLVMECORE0SLO0_MSK_MASK},
	.SlotEn = {XAIEGBL_CORE_STRSWISLVMECORE0SLO0_ENA_LSB, XAIEGBL_CORE_STRSWISLVMECORE0SLO0_ENA_MASK},
	.SlotMsel = {XAIEGBL_CORE_STRSWISLVMECORE0SLO0_MSE_LSB, XAIEGBL_CORE_STRSWISLVMECORE0SLO0_MSE_MASK},
	.SlotArbitor = {XAIEGBL_CORE_STRSWISLVMECORE0SLO0_ARB_LSB, XAIEGBL_CORE_STRSWISLVMECORE0SLO0_ARB_MASK},
	.MstrConfig = AieTileStrmMstr,
	.SlvConfig = AieTileStrmSlv,
	.SlvSlotConfig = AieTileStrmSlaveSlot
};

/*
 * Data structure to capture all stream configs for XAIEGBL_TILE_TYPE_SHIMNOC/PL
 */
static const XAie_StrmMod AieShimStrmSw =
{
	.SlvConfigBaseAddr = XAIEGBL_PL_STRSWISLVTILCTRCFG,
	.PortOffset = 0x4,
	.NumSlaveSlots = 4U,
	.SlotOffsetPerPort = 0x10,
	.SlotOffset = 0x4,
	.MstrEn = {XAIEGBL_CORE_STRSWIMSTRCFGMECORE0_MSTRENA_LSB, XAIEGBL_CORE_STRSWIMSTRCFGMECORE0_MSTRENA_MASK},
	.MstrPktEn = {XAIEGBL_CORE_STRSWIMSTRCFGMECORE0_PKTENA_LSB, XAIEGBL_CORE_STRSWIMSTRCFGMECORE0_PKTENA_MASK},
	.DrpHdr = {XAIEGBL_CORE_STRSWIMSTRCFGMECORE0_DROHEA_LSB, XAIEGBL_CORE_STRSWIMSTRCFGMECORE0_DROHEA_MASK},
	.Config = {XAIEGBL_CORE_STRSWIMSTRCFGMECORE0_CON_LSB, XAIEGBL_CORE_STRSWIMSTRCFGMECORE0_CON_MASK},
	.SlvEn = {XAIEGBL_CORE_STRSWISLVMECORE0CFG_SLVENA_LSB, XAIEGBL_CORE_STRSWISLVMECORE0CFG_SLVENA_MASK},
	.SlvPktEn = {XAIEGBL_CORE_STRSWISLVMECORE0CFG_PKTENA_LSB, XAIEGBL_CORE_STRSWISLVMECORE0CFG_PKTENA_MASK},
	.SlotPktId = {XAIEGBL_CORE_STRSWISLVMECORE0SLO0_ID_LSB, XAIEGBL_CORE_STRSWISLVMECORE0SLO0_ID_MASK},
	.SlotMask = {XAIEGBL_CORE_STRSWISLVMECORE0SLO0_MSK_LSB, XAIEGBL_CORE_STRSWISLVMECORE0SLO0_MSK_MASK},
	.SlotEn = {XAIEGBL_CORE_STRSWISLVMECORE0SLO0_ENA_LSB, XAIEGBL_CORE_STRSWISLVMECORE0SLO0_ENA_MASK},
	.SlotMsel = {XAIEGBL_CORE_STRSWISLVMECORE0SLO0_MSE_LSB, XAIEGBL_CORE_STRSWISLVMECORE0SLO0_MSE_MASK},
	.SlotArbitor = {XAIEGBL_CORE_STRSWISLVMECORE0SLO0_ARB_LSB, XAIEGBL_CORE_STRSWISLVMECORE0SLO0_ARB_MASK},
	.MstrConfig = AieShimStrmMstr,
	.SlvConfig = AieShimStrmSlv,
	.SlvSlotConfig = AieShimStrmSlaveSlot
};

/* Register field attributes for PL interface down sizer for 32 and 64 bits */
static const XAie_RegFldAttr AieDownSzr32_64Bit[] =
{
	{XAIEGBL_PL_PLINTDOWCFG_SOU0_LSB, XAIEGBL_PL_PLINTDOWCFG_SOU0_MASK},
	{XAIEGBL_PL_PLINTDOWCFG_SOU1_LSB, XAIEGBL_PL_PLINTDOWCFG_SOU1_MASK},
	{XAIEGBL_PL_PLINTDOWCFG_SOU2_LSB, XAIEGBL_PL_PLINTDOWCFG_SOU2_MASK},
	{XAIEGBL_PL_PLINTDOWCFG_SOU3_LSB, XAIEGBL_PL_PLINTDOWCFG_SOU3_MASK},
	{XAIEGBL_PL_PLINTDOWCFG_SOU4_LSB, XAIEGBL_PL_PLINTDOWCFG_SOU4_MASK},
	{XAIEGBL_PL_PLINTDOWCFG_SOU5_LSB, XAIEGBL_PL_PLINTDOWCFG_SOU5_MASK},
	{XAIEGBL_PL_PLINTDOWCFG_SOU6_LSB, XAIEGBL_PL_PLINTDOWCFG_SOU6_MASK},
	{XAIEGBL_PL_PLINTDOWCFG_SOU7_LSB, XAIEGBL_PL_PLINTDOWCFG_SOU7_MASK}
};

/* Register field attributes for PL interface down sizer for 128 bits */
static const XAie_RegFldAttr AieDownSzr128Bit[] =
{
	{XAIEGBL_PL_PLINTDOWCFG_SOU0SOU1128COM_LSB, XAIEGBL_PL_PLINTDOWCFG_SOU0SOU1128COM_MASK},
	{XAIEGBL_PL_PLINTDOWCFG_SOU2SOU3128COM_LSB, XAIEGBL_PL_PLINTDOWCFG_SOU2SOU3128COM_MASK},
	{XAIEGBL_PL_PLINTDOWCFG_SOU4SOU5128COM_LSB, XAIEGBL_PL_PLINTDOWCFG_SOU4SOU5128COM_MASK},
	{XAIEGBL_PL_PLINTDOWCFG_SOU6SOU7128COM_LSB, XAIEGBL_PL_PLINTDOWCFG_SOU6SOU7128COM_MASK}
};

/* Register field attributes for PL interface up sizer */
static const XAie_RegFldAttr AieUpSzr32_64Bit[] =
{
	{XAIEGBL_PL_PLINTUPSCFG_SOU0_LSB, XAIEGBL_PL_PLINTUPSCFG_SOU0_MASK},
	{XAIEGBL_PL_PLINTUPSCFG_SOU1_LSB, XAIEGBL_PL_PLINTUPSCFG_SOU1_MASK},
	{XAIEGBL_PL_PLINTUPSCFG_SOU2_LSB, XAIEGBL_PL_PLINTUPSCFG_SOU2_MASK},
	{XAIEGBL_PL_PLINTUPSCFG_SOU3_LSB, XAIEGBL_PL_PLINTUPSCFG_SOU3_MASK},
	{XAIEGBL_PL_PLINTUPSCFG_SOU4_LSB, XAIEGBL_PL_PLINTUPSCFG_SOU4_MASK},
	{XAIEGBL_PL_PLINTUPSCFG_SOU5_LSB, XAIEGBL_PL_PLINTUPSCFG_SOU5_MASK},
};

/* Register field attributes for PL interface up sizer for 128 bits */
static const XAie_RegFldAttr AieUpSzr128Bit[] =
{
	{XAIEGBL_PL_PLINTUPSCFG_SOU0SOU1128COM_LSB, XAIEGBL_PL_PLINTUPSCFG_SOU0SOU1128COM_MASK},
	{XAIEGBL_PL_PLINTUPSCFG_SOU2SOU3128COM_LSB, XAIEGBL_PL_PLINTUPSCFG_SOU2SOU3128COM_MASK},
	{XAIEGBL_PL_PLINTUPSCFG_SOU4SOU5128COM_LSB, XAIEGBL_PL_PLINTUPSCFG_SOU4SOU5128COM_MASK},
};

/* Register field attributes for PL interface down sizer bypass */
static const XAie_RegFldAttr AieDownSzrByPass[] =
{
	{XAIEGBL_PL_PLINTDOWBYPASS_SOU0_LSB, XAIEGBL_PL_PLINTDOWBYPASS_SOU0_MASK},
	{XAIEGBL_PL_PLINTDOWBYPASS_SOU1_LSB, XAIEGBL_PL_PLINTDOWBYPASS_SOU1_MASK},
	{XAIEGBL_PL_PLINTDOWBYPASS_SOU2_LSB, XAIEGBL_PL_PLINTDOWBYPASS_SOU2_MASK},
	{XAIEGBL_PL_PLINTDOWBYPASS_SOU4_LSB, XAIEGBL_PL_PLINTDOWBYPASS_SOU4_MASK},
	{XAIEGBL_PL_PLINTDOWBYPASS_SOU5_LSB, XAIEGBL_PL_PLINTDOWBYPASS_SOU5_MASK},
	{XAIEGBL_PL_PLINTDOWBYPASS_SOU6_LSB, XAIEGBL_PL_PLINTDOWBYPASS_SOU6_MASK},
};

/* Register field attributes for PL interface down sizer enable */
static const XAie_RegFldAttr AieDownSzrEnable[] =
{
	{XAIEGBL_PL_PLINTDOWENA_SOU0_LSB, XAIEGBL_PL_PLINTDOWENA_SOU0_MASK},
	{XAIEGBL_PL_PLINTDOWENA_SOU1_LSB, XAIEGBL_PL_PLINTDOWENA_SOU1_MASK},
	{XAIEGBL_PL_PLINTDOWENA_SOU2_LSB, XAIEGBL_PL_PLINTDOWENA_SOU2_MASK},
	{XAIEGBL_PL_PLINTDOWENA_SOU3_LSB, XAIEGBL_PL_PLINTDOWENA_SOU3_MASK},
	{XAIEGBL_PL_PLINTDOWENA_SOU4_LSB, XAIEGBL_PL_PLINTDOWENA_SOU4_MASK},
	{XAIEGBL_PL_PLINTDOWENA_SOU5_LSB, XAIEGBL_PL_PLINTDOWENA_SOU5_MASK},
	{XAIEGBL_PL_PLINTDOWENA_SOU6_LSB, XAIEGBL_PL_PLINTDOWENA_SOU6_MASK},
	{XAIEGBL_PL_PLINTDOWENA_SOU7_LSB, XAIEGBL_PL_PLINTDOWENA_SOU7_MASK}
};

/* Register field attributes for SHIMNOC Mux configuration */
static const XAie_RegFldAttr AieShimMuxConfig[] =
{
	{XAIEGBL_NOC_MUXCFG_SOU2_LSB, XAIEGBL_NOC_MUXCFG_SOU2_MASK},
	{XAIEGBL_NOC_MUXCFG_SOU3_LSB, XAIEGBL_NOC_MUXCFG_SOU3_MASK},
	{XAIEGBL_NOC_MUXCFG_SOU6_LSB, XAIEGBL_NOC_MUXCFG_SOU6_MASK},
	{XAIEGBL_NOC_MUXCFG_SOU7_LSB, XAIEGBL_NOC_MUXCFG_SOU7_MASK},
};

/* Register field attributes for SHIMNOC DeMux configuration */
static const XAie_RegFldAttr AieShimDeMuxConfig[] =
{
	{XAIEGBL_NOC_DEMCFG_SOU2_LSB, XAIEGBL_NOC_DEMCFG_SOU2_MASK},
	{XAIEGBL_NOC_DEMCFG_SOU3_LSB, XAIEGBL_NOC_DEMCFG_SOU3_MASK},
	{XAIEGBL_NOC_DEMCFG_SOU4_LSB, XAIEGBL_NOC_DEMCFG_SOU4_MASK},
	{XAIEGBL_NOC_DEMCFG_SOU5_LSB, XAIEGBL_NOC_DEMCFG_SOU5_MASK},
};

/* Core Module */
static const XAie_CoreMod AieCoreMod =
{
	.IsCheckerBoard = 1U,
	.ProgMemAddr = 0x0,
	.ProgMemSize = 16 * 1024,
	.DataMemAddr = 0x20000,
	.ProgMemHostOffset = XAIEGBL_CORE_PRGMEM,
	.DataMemSize = 32 * 1024,
	.DataMemShift = 15U,
	.CoreCtrl = &AieCoreCtrlReg,
	.CoreSts = &AieCoreStsReg,
	.CoreDebug = &AieCoreDebugReg
};

/* Data Memory Module for Tile data memory*/
static const XAie_MemMod AieTileMemMod =
{
	.Size = 32 * 1024,
	.MemAddr = XAIEGBL_MEM_DATMEM
};

/* PL Interface module for SHIMPL Tiles */
static const XAie_PlIfMod AiePlIfMod =
{
	.UpSzrOff = XAIEGBL_PL_PLINTUPSCFG,
	.DownSzrOff = XAIEGBL_PL_PLINTDOWCFG,
	.DownSzrEnOff = XAIEGBL_PL_PLINTDOWENA,
	.DownSzrByPassOff = XAIEGBL_PL_PLINTDOWBYPASS,
	.NumUpSzrPorts = 0x6,
	.MaxByPassPortNum = 0x6,
	.NumDownSzrPorts = 0x8,
	.UpSzr32_64Bit = AieUpSzr32_64Bit,
	.UpSzr128Bit = AieUpSzr128Bit,
	.DownSzr32_64Bit = AieDownSzr32_64Bit,
	.DownSzr128Bit = AieDownSzr128Bit,
	.DownSzrEn = AieDownSzrEnable,
	.DownSzrByPass = AieDownSzrByPass,
	.ShimNocMuxOff = 0x0,
	.ShimNocDeMuxOff = 0x0,
	.ShimNocMux = NULL,
	.ShimNocDeMux = NULL
};

/* PL Interface module for SHIMNOC Tiles */
static const XAie_PlIfMod AieShimTilePlIfMod =
{
	.UpSzrOff = XAIEGBL_PL_PLINTUPSCFG,
	.DownSzrOff = XAIEGBL_PL_PLINTDOWCFG,
	.DownSzrEnOff = XAIEGBL_PL_PLINTDOWENA,
	.DownSzrByPassOff = XAIEGBL_PL_PLINTDOWBYPASS,
	.NumUpSzrPorts = 0x6,
	.MaxByPassPortNum = 0x6,
	.NumDownSzrPorts = 0x8,
	.UpSzr32_64Bit = AieUpSzr32_64Bit,
	.UpSzr128Bit = AieUpSzr128Bit,
	.DownSzr32_64Bit = AieDownSzr32_64Bit,
	.DownSzr128Bit = AieDownSzr128Bit,
	.DownSzrEn = AieDownSzrEnable,
	.DownSzrByPass = AieDownSzrByPass,
	.ShimNocMuxOff = XAIEGBL_NOC_MUXCFG,
	.ShimNocDeMuxOff = XAIEGBL_NOC_DEMCFG,
	.ShimNocMux = AieShimMuxConfig,
	.ShimNocDeMux = AieShimDeMuxConfig
};

/* Lock Module for AIE Tiles  */
static const XAie_LockMod AieTileLockMod =
{
	.BaseAddr = XAIEGBL_MEM_LOCK0RELNV,
	.NumLocks = 16U,
	.LockIdOff = 0x80,
	.RelAcqOff = 0x40,
	.LockValOff = 0x10,
	.LockValUpperBound = 1,
	.LockValLowerBound = -1,
	.Acquire = &(_XAie_LockAcquire),
	.Release = &(_XAie_LockRelease)
};

/* Lock Module for SHIM NOC Tiles  */
static const XAie_LockMod AieShimNocLockMod =
{
	.BaseAddr = XAIEGBL_NOC_LOCK0RELNV,
	.NumLocks = 16U,
	.LockIdOff = 0x80,
	.RelAcqOff = 0x40,
	.LockValOff = 0x10,
	.LockValUpperBound = 1,
	.LockValLowerBound = -1,
	.Acquire = &(_XAie_LockAcquire),
	.Release = &(_XAie_LockRelease)
};

static const XAie_DmaBdEnProp AieTileDmaBdEnProp =
{
	.NxtBd.Idx = 6U,
	.NxtBd.Lsb = XAIEGBL_MEM_DMABD0CTRL_NEXBD_LSB,
	.NxtBd.Mask = XAIEGBL_MEM_DMABD0CTRL_NEXBD_MASK,
	.UseNxtBd.Idx = 6U,
	.UseNxtBd.Lsb = XAIEGBL_MEM_DMABD0CTRL_USENEXBD_LSB,
	.UseNxtBd.Mask = XAIEGBL_MEM_DMABD0CTRL_USENEXBD_MASK,
	.ValidBd.Idx = 6U,
	.ValidBd.Lsb = XAIEGBL_MEM_DMABD0CTRL_VALBD_LSB,
	.ValidBd.Mask = XAIEGBL_MEM_DMABD0CTRL_VALBD_MASK,
	.OutofOrderBdId = {0U}
};

static const XAie_DmaBdPkt AieTileDmaBdPktProp =
{
	.EnPkt.Idx = 4U,
	.EnPkt.Lsb = XAIEGBL_MEM_DMABD0CTRL_ENAPKT_LSB,
	.EnPkt.Mask = XAIEGBL_MEM_DMABD0CTRL_ENAPKT_MASK,
	.PktType.Idx = 4U,
	.PktType.Lsb = XAIEGBL_MEM_DMABD0PKT_PKTTYP_LSB,
	.PktType.Mask = XAIEGBL_MEM_DMABD0PKT_PKTTYP_MASK,
	.PktId.Idx = 4U,
	.PktId.Lsb = XAIEGBL_MEM_DMABD0PKT_ID_LSB,
	.PktId.Mask = XAIEGBL_MEM_DMABD0PKT_ID_MASK
};

static const XAie_DmaBdLock AieTileDmaLockProp =
{
	.AieDmaLock.LckId_A.Idx = 0U,
	.AieDmaLock.LckId_A.Lsb = XAIEGBL_MEM_DMABD0ADDA_LOCKIDA_LSB,
	.AieDmaLock.LckId_A.Mask = XAIEGBL_MEM_DMABD0ADDA_LOCKIDA_MASK,
	.AieDmaLock.LckId_B.Idx = 1U,
	.AieDmaLock.LckId_B.Lsb = XAIEGBL_MEM_DMABD0ADDB_LOCKIDB_LSB,
	.AieDmaLock.LckId_B.Mask = XAIEGBL_MEM_DMABD0ADDB_LOCKIDB_MASK,
	.AieDmaLock.LckRelEn_A.Idx = 0U,
	.AieDmaLock.LckRelEn_A.Lsb = XAIEGBL_MEM_DMABD0ADDA_ENAREL_LSB,
	.AieDmaLock.LckRelEn_A.Mask = XAIEGBL_MEM_DMABD0ADDA_ENAREL_MASK,
	.AieDmaLock.LckRelVal_A.Idx = 0U,
	.AieDmaLock.LckRelVal_A.Lsb = XAIEGBL_MEM_DMABD0ADDA_RELVALA_LSB,
	.AieDmaLock.LckRelVal_A.Mask = XAIEGBL_MEM_DMABD0ADDA_RELVALA_MASK,
	.AieDmaLock.LckRelUseVal_A.Idx = 0U,
	.AieDmaLock.LckRelUseVal_A.Lsb = XAIEGBL_MEM_DMABD0ADDA_USERELVALA_LSB,
	.AieDmaLock.LckRelUseVal_A.Mask = XAIEGBL_MEM_DMABD0ADDA_USERELVALA_MASK,
	.AieDmaLock.LckAcqEn_A.Idx = 0U,
	.AieDmaLock.LckAcqEn_A.Lsb = XAIEGBL_MEM_DMABD0ADDA_ENAACQ_LSB,
	.AieDmaLock.LckAcqEn_A.Mask = XAIEGBL_MEM_DMABD0ADDA_ENAACQ_MASK,
	.AieDmaLock.LckAcqVal_A.Idx = 0U,
	.AieDmaLock.LckAcqVal_A.Lsb = XAIEGBL_MEM_DMABD0ADDA_ACQVALA_LSB,
	.AieDmaLock.LckAcqVal_A.Mask = XAIEGBL_MEM_DMABD0ADDA_ACQVALA_MASK,
	.AieDmaLock.LckAcqUseVal_A.Idx = 0U,
	.AieDmaLock.LckAcqUseVal_A.Lsb = XAIEGBL_MEM_DMABD0ADDA_USEACQVALA_LSB,
	.AieDmaLock.LckAcqUseVal_A.Mask = XAIEGBL_MEM_DMABD0ADDA_USEACQVALA_MASK,
	.AieDmaLock.LckRelEn_B.Idx = 1U,
	.AieDmaLock.LckRelEn_B.Lsb = XAIEGBL_MEM_DMABD0ADDB_ENAREL_LSB,
	.AieDmaLock.LckRelEn_B.Mask = XAIEGBL_MEM_DMABD0ADDB_ENAREL_MASK,
	.AieDmaLock.LckRelVal_B.Idx = 1U,
	.AieDmaLock.LckRelVal_B.Lsb = XAIEGBL_MEM_DMABD0ADDB_RELVALB_LSB,
	.AieDmaLock.LckRelVal_B.Mask = XAIEGBL_MEM_DMABD0ADDB_RELVALB_MASK,
	.AieDmaLock.LckRelUseVal_B.Idx = 1U,
	.AieDmaLock.LckRelUseVal_B.Lsb = XAIEGBL_MEM_DMABD0ADDB_USERELVALB_MASK,
	.AieDmaLock.LckRelUseVal_B.Mask = XAIEGBL_MEM_DMABD0ADDB_USERELVALB_MASK,
	.AieDmaLock.LckAcqEn_B.Idx = 1U,
	.AieDmaLock.LckAcqEn_B.Lsb = XAIEGBL_MEM_DMABD0ADDB_ENAACQ_LSB,
	.AieDmaLock.LckAcqEn_B.Mask = XAIEGBL_MEM_DMABD0ADDB_ENAACQ_MASK,
	.AieDmaLock.LckAcqVal_B.Idx = 1U,
	.AieDmaLock.LckAcqVal_B.Lsb = XAIEGBL_MEM_DMABD0ADDB_ACQVALB_LSB,
	.AieDmaLock.LckAcqVal_B.Mask = XAIEGBL_MEM_DMABD0ADDB_ACQVALB_MASK,
	.AieDmaLock.LckAcqUseVal_B.Idx = 1U,
	.AieDmaLock.LckAcqUseVal_B.Lsb = XAIEGBL_MEM_DMABD0ADDB_USEACQVALB_LSB,
	.AieDmaLock.LckAcqUseVal_B.Mask = XAIEGBL_MEM_DMABD0ADDB_USEACQVALB_MASK,
};

static const XAie_DmaBdBuffer AieTileDmaBufferProp =
{
	.TileDmaBuff.BaseAddr.Idx = 0U,
	.TileDmaBuff.BaseAddr.Lsb = XAIEGBL_MEM_DMABD0ADDA_BASADDA_LSB,
	.TileDmaBuff.BaseAddr.Mask = XAIEGBL_MEM_DMABD0ADDA_BASADDA_MASK,
	.TileDmaBuff.BufferLen.Idx = 6U,
	.TileDmaBuff.BufferLen.Lsb = XAIEGBL_MEM_DMABD0CTRL_LEN_LSB,
	.TileDmaBuff.BufferLen.Mask = XAIEGBL_MEM_DMABD0CTRL_LEN_MASK,
};

static const XAie_DmaBdDoubleBuffer AieTileDmaDoubleBufferProp =
{
	.EnDoubleBuff.Idx = 6U,
	.EnDoubleBuff.Lsb = XAIEGBL_MEM_DMABD0CTRL_ENAABMOD_LSB,
	.EnDoubleBuff.Mask = XAIEGBL_MEM_DMABD0CTRL_ENAABMOD_MASK,
	.BaseAddr_B.Idx = 1U,
	.BaseAddr_B.Lsb = XAIEGBL_MEM_DMABD0ADDB_BASADDB_LSB,
	.BaseAddr_B.Mask = XAIEGBL_MEM_DMABD0ADDB_BASADDB_MASK,
	.FifoMode.Idx = 6U,
	.FifoMode.Lsb = XAIEGBL_MEM_DMABD0CTRL_ENAFIFMOD_LSB,
	.FifoMode.Mask = XAIEGBL_MEM_DMABD0CTRL_ENAFIFMOD_MASK,
	.EnIntrleaved.Idx = 6U,
	.EnIntrleaved.Lsb = XAIEGBL_MEM_DMABD0CTRL_ENAINT_LSB,
	.EnIntrleaved.Mask = XAIEGBL_MEM_DMABD0CTRL_ENAINT_MASK,
	.IntrleaveCnt.Idx = 6U,
	.IntrleaveCnt.Lsb = XAIEGBL_MEM_DMABD0CTRL_INTCNT_LSB,
	.IntrleaveCnt.Mask = XAIEGBL_MEM_DMABD0CTRL_INTCNT_MASK,
	.BuffSelect.Idx = 5U,
	.BuffSelect.Lsb = XAIEGBL_MEM_DMABD0INTSTA_AB_LSB,
	.BuffSelect.Mask = XAIEGBL_MEM_DMABD0INTSTA_AB_MASK
};

static const XAie_DmaBdMultiDimAddr AieTileDmaMultiDimProp =
{
	.AieMultiDimAddr.X_Incr.Idx = 2U,
	.AieMultiDimAddr.X_Incr.Lsb = XAIEGBL_MEM_DMABD02DX_XINC_LSB,
	.AieMultiDimAddr.X_Incr.Mask = XAIEGBL_MEM_DMABD02DX_XINC_MASK,
	.AieMultiDimAddr.X_Wrap.Idx = 2U,
	.AieMultiDimAddr.X_Wrap.Lsb = XAIEGBL_MEM_DMABD02DX_XWRA_LSB,
	.AieMultiDimAddr.X_Wrap.Mask = XAIEGBL_MEM_DMABD02DX_XWRA_MASK,
	.AieMultiDimAddr.X_Offset.Idx = 2U,
	.AieMultiDimAddr.X_Offset.Lsb = XAIEGBL_MEM_DMABD02DX_XOFF_LSB,
	.AieMultiDimAddr.X_Offset.Mask = XAIEGBL_MEM_DMABD02DX_XOFF_MASK,
	.AieMultiDimAddr.Y_Incr.Idx = 3U,
	.AieMultiDimAddr.Y_Incr.Lsb = XAIEGBL_MEM_DMABD02DY_YINC_LSB,
	.AieMultiDimAddr.Y_Incr.Mask = XAIEGBL_MEM_DMABD02DY_YINC_MASK,
	.AieMultiDimAddr.Y_Wrap.Idx = 3U,
	.AieMultiDimAddr.Y_Wrap.Lsb = XAIEGBL_MEM_DMABD02DY_YWRA_LSB,
	.AieMultiDimAddr.Y_Wrap.Mask = XAIEGBL_MEM_DMABD02DY_YWRA_MASK,
	.AieMultiDimAddr.Y_Offset.Idx = 3U,
	.AieMultiDimAddr.Y_Offset.Lsb = XAIEGBL_MEM_DMABD02DY_YOFF_LSB,
	.AieMultiDimAddr.Y_Offset.Mask = XAIEGBL_MEM_DMABD02DY_YOFF_MASK,
	.AieMultiDimAddr.CurrPtr.Idx = 5U,
	.AieMultiDimAddr.CurrPtr.Lsb = XAIEGBL_MEM_DMABD0INTSTA_CURPTR_LSB,
	.AieMultiDimAddr.CurrPtr.Mask = XAIEGBL_MEM_DMABD0INTSTA_CURPTR_MASK
};

static const XAie_DmaBdProp AieTileDmaProp =
{
	.AddrAlignMask = 0x3,
	.AddrAlignShift = 0x2,
	.AddrMask = (1U << 16U) - 1U,
	.LenActualOffset = 1U,
	.Buffer = &AieTileDmaBufferProp,
	.DoubleBuffer = &AieTileDmaDoubleBufferProp,
	.Lock = &AieTileDmaLockProp,
	.Pkt = &AieTileDmaBdPktProp,
	.BdEn = &AieTileDmaBdEnProp,
	.AddrMode = &AieTileDmaMultiDimProp,
	.ZeroPad = NULL,
	.Compression = NULL,
	.SysProp = NULL
};

static const XAie_DmaChProp AieTileDmaChProp =
{
	.CtrlId = {0U},
	.EnCompression = {0U},
	.EnOutofOrder = {0U},
	.EnToken = {0U},
	.RptCount = {0U},
	.PauseStream = {0U},
	.PauseMem = {0U},
	.Reset.Idx = 0U,
	.Reset.Lsb = XAIEGBL_MEM_DMAS2MM0CTR_RST_LSB,
	.Reset.Mask = XAIEGBL_MEM_DMAS2MM0CTR_RST_MASK,
	.Enable.Idx = 0U,
	.Enable.Lsb = XAIEGBL_MEM_DMAS2MM0CTR_ENA_LSB,
	.Enable.Mask = XAIEGBL_MEM_DMAS2MM0CTR_ENA_MASK,
	.StartBd.Idx = 1U,
	.StartBd.Lsb = XAIEGBL_MEM_DMAS2MM0STAQUE_STABD_LSB,
	.StartBd.Mask = XAIEGBL_MEM_DMAS2MM0STAQUE_STABD_MASK,
};

/* Tile Dma Module */
static const XAie_DmaMod AieTileDmaMod =
{
	.BaseAddr = XAIEGBL_MEM_DMABD0ADDA,
	.IdxOffset = 0x20,  	/* This is the offset between each BD */
	.NumBds = 16U,	   	/* Number of BDs for AIE Tile DMA */
	.NumLocks = 16U,
	.NumAddrDim = 2U,
	.DoubleBuffering = XAIE_FEATURE_AVAILABLE,
	.Compression = XAIE_FEATURE_UNAVAILABLE,
	.ZeroPadding = XAIE_FEATURE_UNAVAILABLE,
	.OutofOrderBdId = XAIE_FEATURE_UNAVAILABLE,
	.InterleaveMode = XAIE_FEATURE_AVAILABLE,
	.FifoMode = XAIE_FEATURE_AVAILABLE,
	.ChCtrlBase = XAIEGBL_MEM_DMAS2MM0CTR,
	.NumChannels = 2U,  /* Number of s2mm/mm2s channels */
	.ChIdxOffset = 0x8,  /* This is the offset between each channel */
	.BdProp = &AieTileDmaProp,
	.ChProp = &AieTileDmaChProp,
	.DmaBdInit = &_XAie_TileDmaInit,
	.SetLock = &_XAie_DmaSetLock,
	.SetIntrleave = &_XAie_DmaSetInterleaveEnable,
	.SetMultiDim = &_XAie_DmaSetMultiDim,
	.WriteBd = &_XAie_TileDmaWriteBd
};

/* shim dma structures */
static const XAie_DmaBdEnProp AieShimDmaBdEnProp =
{
	.NxtBd.Idx = 2U,
	.NxtBd.Lsb = XAIEGBL_NOC_DMABD1BUFCTRL_NEXBD_LSB,
	.NxtBd.Mask = XAIEGBL_NOC_DMABD1BUFCTRL_NEXBD_MASK,
	.UseNxtBd.Idx = 2U,
	.UseNxtBd.Lsb = XAIEGBL_NOC_DMABD1BUFCTRL_USENEXBD_LSB,
	.UseNxtBd.Mask = XAIEGBL_NOC_DMABD1BUFCTRL_USENEXBD_MASK,
	.ValidBd.Idx = 2U,
	.ValidBd.Lsb = XAIEGBL_NOC_DMABD1BUFCTRL_VALBD_LSB,
	.ValidBd.Mask = XAIEGBL_NOC_DMABD1BUFCTRL_VALBD_MASK,
	.OutofOrderBdId = {0U}
};

static const XAie_DmaBdPkt AieShimDmaBdPktProp =
{
	.EnPkt.Idx = 4U,
	.EnPkt.Lsb = XAIEGBL_NOC_DMABD0PKT_ENAPKT_LSB,
	.EnPkt.Mask = XAIEGBL_NOC_DMABD0PKT_ENAPKT_MASK,
	.PktType.Idx = 4U,
	.PktType.Lsb = XAIEGBL_NOC_DMABD0PKT_PKTTYP_LSB,
	.PktType.Mask = XAIEGBL_NOC_DMABD0PKT_PKTTYP_MASK,
	.PktId.Idx = 4U,
	.PktId.Lsb = XAIEGBL_NOC_DMABD0PKT_ID_LSB,
	.PktId.Mask = XAIEGBL_NOC_DMABD0PKT_ID_MASK
};

static const XAie_DmaBdLock AieShimDmaLockProp =
{
	.AieDmaLock.LckId_A.Idx = 2U,
	.AieDmaLock.LckId_A.Lsb = XAIEGBL_NOC_DMABD0CTRL_LOCKID_LSB,
	.AieDmaLock.LckId_A.Mask = XAIEGBL_NOC_DMABD0CTRL_LOCKID_MASK,
	.AieDmaLock.LckRelEn_A.Idx = 2U,
	.AieDmaLock.LckRelEn_A.Lsb = XAIEGBL_NOC_DMABD0CTRL_ENAREL_LSB,
	.AieDmaLock.LckRelEn_A.Mask = XAIEGBL_NOC_DMABD0CTRL_ENAREL_MASK,
	.AieDmaLock.LckRelVal_A.Idx = 2U,
	.AieDmaLock.LckRelVal_A.Lsb = XAIEGBL_NOC_DMABD0CTRL_RELVAL_LSB,
	.AieDmaLock.LckRelVal_A.Mask = XAIEGBL_NOC_DMABD0CTRL_RELVAL_MASK,
	.AieDmaLock.LckRelUseVal_A.Idx = 2U,
	.AieDmaLock.LckRelUseVal_A.Lsb = XAIEGBL_NOC_DMABD0CTRL_USERELVAL_LSB,
	.AieDmaLock.LckRelUseVal_A.Mask = XAIEGBL_NOC_DMABD0CTRL_USERELVAL_MASK,
	.AieDmaLock.LckAcqEn_A.Idx = 2U,
	.AieDmaLock.LckAcqEn_A.Lsb = XAIEGBL_NOC_DMABD0CTRL_ENAACQ_LSB,
	.AieDmaLock.LckAcqEn_A.Mask = XAIEGBL_NOC_DMABD0CTRL_ENAACQ_MASK,
	.AieDmaLock.LckAcqVal_A.Idx = 2U,
	.AieDmaLock.LckAcqVal_A.Lsb = XAIEGBL_NOC_DMABD0CTRL_ACQVAL_LSB,
	.AieDmaLock.LckAcqVal_A.Mask = XAIEGBL_NOC_DMABD0CTRL_ACQVAL_MASK,
	.AieDmaLock.LckAcqUseVal_A.Idx = 2U,
	.AieDmaLock.LckAcqUseVal_A.Lsb = XAIEGBL_NOC_DMABD0CTRL_USEACQVAL_LSB,
	.AieDmaLock.LckAcqUseVal_A.Mask = XAIEGBL_NOC_DMABD0CTRL_USEACQVAL_MASK,
	.AieDmaLock.LckId_B = {0U},
	.AieDmaLock.LckRelEn_B = {0U},
	.AieDmaLock.LckRelVal_B = {0U},
	.AieDmaLock.LckRelUseVal_B = {0U},
	.AieDmaLock.LckAcqEn_B = {0U},
	.AieDmaLock.LckAcqVal_B = {0U},
	.AieDmaLock.LckAcqUseVal_B = {0U}
};

static const XAie_DmaBdBuffer AieShimDmaBufferProp =
{
	.ShimDmaBuff.AddrLow.Idx = 0U,
	.ShimDmaBuff.AddrLow.Lsb = XAIEGBL_NOC_DMABD0ADDLOW_ADDLOW_LSB,
	.ShimDmaBuff.AddrLow.Mask = XAIEGBL_NOC_DMABD0ADDLOW_ADDLOW_MASK,
	.ShimDmaBuff.AddrHigh.Idx = 2U,
	.ShimDmaBuff.AddrHigh.Lsb = XAIEGBL_NOC_DMABD0CTRL_ADDHIG_LSB,
	.ShimDmaBuff.AddrHigh.Mask = XAIEGBL_NOC_DMABD0CTRL_ADDHIG_MASK,
	.ShimDmaBuff.BufferLen.Idx = 1U,
	.ShimDmaBuff.BufferLen.Lsb = XAIEGBL_NOC_DMABD0BUFLEN_BUFLEN_LSB,
	.ShimDmaBuff.BufferLen.Mask = XAIEGBL_NOC_DMABD0BUFLEN_BUFLEN_MASK,
};

static const XAie_DmaSysProp AieShimDmaSysProp =
{
	.SMID.Idx = 3U,
	.SMID.Lsb = XAIEGBL_NOC_DMABD0AXICFG_SMI_LSB,
	.SMID.Mask = XAIEGBL_NOC_DMABD0AXICFG_SMI_MASK,
	.BurstLen.Idx = 3U,
	.BurstLen.Lsb = XAIEGBL_NOC_DMABD0AXICFG_BURLEN_LSB,
	.BurstLen.Mask = XAIEGBL_NOC_DMABD0AXICFG_BURLEN_MASK,
	.AxQos.Idx = 3U,
	.AxQos.Lsb = XAIEGBL_NOC_DMABD0AXICFG_AXQ_LSB,
	.AxQos.Mask = XAIEGBL_NOC_DMABD0AXICFG_AXQ_MASK,
	.SecureAccess.Idx = 3U,
	.SecureAccess.Lsb = XAIEGBL_NOC_DMABD0AXICFG_SECACC_LSB,
	.SecureAccess.Mask = XAIEGBL_NOC_DMABD0AXICFG_SECACC_MASK,
	.AxCache.Idx = 3U,
	.AxCache.Lsb = XAIEGBL_NOC_DMABD0AXICFG_AXC_LSB,
	.AxCache.Mask = XAIEGBL_NOC_DMABD0AXICFG_AXC_MASK,
};

static const XAie_DmaBdProp AieShimDmaProp =
{
	.AddrAlignMask = 0xF,
	.AddrAlignShift = 0x0,
	.AddrMask = (1UL << 48U) - 1U,
	.LenActualOffset = 0U,
	.Buffer = &AieShimDmaBufferProp,
	.DoubleBuffer = NULL,
	.Lock = &AieShimDmaLockProp,
	.Pkt = &AieShimDmaBdPktProp,
	.BdEn = &AieShimDmaBdEnProp,
	.AddrMode = NULL,
	.ZeroPad = NULL,
	.Compression = NULL,
	.SysProp = &AieShimDmaSysProp
};

static const XAie_DmaChProp AieShimDmaChProp =
{
	.CtrlId = {0U},
	.EnCompression = {0U},
	.EnOutofOrder = {0U},
	.EnToken = {0U},
	.RptCount = {0U},
	.Reset = {0U},
	.Enable.Idx = 0U,
	.Enable.Lsb = XAIEGBL_NOC_DMAS2MM0CTR_ENA_LSB,
	.Enable.Mask = XAIEGBL_NOC_DMAS2MM0CTR_ENA_MASK,
	.PauseStream.Idx = 0U,
	.PauseStream.Lsb = XAIEGBL_NOC_DMAS2MM0CTR_PAUSTR_LSB,
	.PauseStream.Mask = XAIEGBL_NOC_DMAS2MM0CTR_PAUSTR_MASK,
	.PauseMem.Idx = 0U,
	.PauseMem.Lsb = XAIEGBL_NOC_DMAS2MM0CTR_PAUMEM_LSB,
	.PauseMem.Mask = XAIEGBL_NOC_DMAS2MM0CTR_PAUMEM_MASK,
	.StartBd.Idx = 1U,
	.StartBd.Lsb = XAIEGBL_NOC_DMAS2MM0STAQUE_STABD_LSB,
	.StartBd.Mask = XAIEGBL_NOC_DMAS2MM0STAQUE_STABD_MASK,
};

/* Shim Dma Module */
static const XAie_DmaMod AieShimDmaMod =
{
	.BaseAddr = XAIEGBL_NOC_DMABD0ADDLOW,
	.IdxOffset = 0x14,  	/* This is the offset between each BD */
	.NumBds = 16U,	   	/* Number of BDs for AIE Tile DMA */
	.NumLocks = 16U,
	.NumAddrDim = 0U,
	.DoubleBuffering = XAIE_FEATURE_UNAVAILABLE,
	.Compression = XAIE_FEATURE_UNAVAILABLE,
	.ZeroPadding = XAIE_FEATURE_UNAVAILABLE,
	.OutofOrderBdId = XAIE_FEATURE_UNAVAILABLE,
	.InterleaveMode = XAIE_FEATURE_UNAVAILABLE,
	.FifoMode = XAIE_FEATURE_UNAVAILABLE,
	.ChCtrlBase = XAIEGBL_NOC_DMAS2MM0CTR,
	.NumChannels = 2U,  /* Number of s2mm/mm2s channels */
	.ChIdxOffset = 0x8,  /* This is the offset between each channel */
	.BdProp = &AieShimDmaProp,
	.ChProp = &AieShimDmaChProp,
	.DmaBdInit = &_XAie_ShimDmaInit,
	.SetLock = &_XAie_DmaSetLock,
	.SetIntrleave = NULL,
	.SetMultiDim = NULL,
	.WriteBd = &_XAie_ShimDmaWriteBd
};

/* Enum to Event Number mapping of all events of AIE Core module */
static const u8 AieTileCoreModEventMapping[] =
{
	XAIE_EVENTS_CORE_NONE,
	XAIE_EVENTS_CORE_TRUE,
	XAIE_EVENTS_CORE_GROUP_0,
	XAIE_EVENTS_CORE_TIMER_SYNC,
	XAIE_EVENTS_CORE_TIMER_VALUE_REACHED,
	XAIE_EVENTS_CORE_PERF_CNT_0,
	XAIE_EVENTS_CORE_PERF_CNT_1,
	XAIE_EVENTS_CORE_PERF_CNT_2,
	XAIE_EVENTS_CORE_PERF_CNT_3,
	XAIE_EVENTS_CORE_COMBO_EVENT_0,
	XAIE_EVENTS_CORE_COMBO_EVENT_1,
	XAIE_EVENTS_CORE_COMBO_EVENT_2,
	XAIE_EVENTS_CORE_COMBO_EVENT_3,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENTS_CORE_GROUP_PC_EVENT,
	XAIE_EVENTS_CORE_PC_0,
	XAIE_EVENTS_CORE_PC_1,
	XAIE_EVENTS_CORE_PC_2,
	XAIE_EVENTS_CORE_PC_3,
	XAIE_EVENTS_CORE_PC_RANGE_0_1,
	XAIE_EVENTS_CORE_PC_RANGE_2_3,
	XAIE_EVENTS_CORE_GROUP_STALL,
	XAIE_EVENTS_CORE_MEMORY_STALL,
	XAIE_EVENTS_CORE_STREAM_STALL,
	XAIE_EVENTS_CORE_CASCADE_STALL,
	XAIE_EVENTS_CORE_LOCK_STALL,
	XAIE_EVENTS_CORE_DEBUG_HALTED,
	XAIE_EVENTS_CORE_ACTIVE,
	XAIE_EVENTS_CORE_DISABLED,
	XAIE_EVENTS_CORE_ECC_ERROR_STALL,
	XAIE_EVENTS_CORE_ECC_SCRUBBING_STALL,
	XAIE_EVENTS_CORE_GROUP_PROGRAM_FLOW,
	XAIE_EVENTS_CORE_INSTR_EVENT_0,
	XAIE_EVENTS_CORE_INSTR_EVENT_1,
	XAIE_EVENTS_CORE_INSTR_CALL,
	XAIE_EVENTS_CORE_INSTR_RETURN,
	XAIE_EVENTS_CORE_INSTR_VECTOR,
	XAIE_EVENTS_CORE_INSTR_LOAD,
	XAIE_EVENTS_CORE_INSTR_STORE,
	XAIE_EVENTS_CORE_INSTR_STREAM_GET,
	XAIE_EVENTS_CORE_INSTR_STREAM_PUT,
	XAIE_EVENTS_CORE_INSTR_CASCADE_GET,
	XAIE_EVENTS_CORE_INSTR_CASCADE_PUT,
	XAIE_EVENTS_CORE_INSTR_LOCK_ACQUIRE_REQ,
	XAIE_EVENTS_CORE_INSTR_LOCK_RELEASE_REQ,
	XAIE_EVENTS_CORE_GROUP_ERRORS_0,
	XAIE_EVENTS_CORE_GROUP_ERRORS_1,
	XAIE_EVENTS_CORE_SRS_SATURATE,
	XAIE_EVENTS_CORE_UPS_SATURATE,
	XAIE_EVENTS_CORE_FP_OVERFLOW,
	XAIE_EVENTS_CORE_FP_UNDERFLOW,
	XAIE_EVENTS_CORE_FP_INVALID,
	XAIE_EVENTS_CORE_FP_DIV_BY_ZERO,
	XAIE_EVENTS_CORE_TLAST_IN_WSS_WORDS_0_2,
	XAIE_EVENTS_CORE_PM_REG_ACCESS_FAILURE,
	XAIE_EVENTS_CORE_STREAM_PKT_PARITY_ERROR,
	XAIE_EVENTS_CORE_CONTROL_PKT_ERROR,
	XAIE_EVENTS_CORE_AXI_MM_SLAVE_ERROR,
	XAIE_EVENTS_CORE_INSTR_DECOMPRSN_ERROR,
	XAIE_EVENTS_CORE_DM_ADDRESS_OUT_OF_RANGE,
	XAIE_EVENTS_CORE_PM_ECC_ERROR_SCRUB_CORRECTED,
	XAIE_EVENTS_CORE_PM_ECC_ERROR_SCRUB_2BIT,
	XAIE_EVENTS_CORE_PM_ECC_ERROR_1BIT,
	XAIE_EVENTS_CORE_PM_ECC_ERROR_2BIT,
	XAIE_EVENTS_CORE_PM_ADDRESS_OUT_OF_RANGE,
	XAIE_EVENTS_CORE_DM_ACCESS_TO_UNAVAILABLE,
	XAIE_EVENTS_CORE_LOCK_ACCESS_TO_UNAVAILABLE,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENTS_CORE_GROUP_STREAM_SWITCH,
	XAIE_EVENTS_CORE_PORT_IDLE_0,
	XAIE_EVENTS_CORE_PORT_RUNNING_0,
	XAIE_EVENTS_CORE_PORT_STALLED_0,
	XAIE_EVENTS_CORE_PORT_TLAST_0,
	XAIE_EVENTS_CORE_PORT_IDLE_1,
	XAIE_EVENTS_CORE_PORT_RUNNING_1,
	XAIE_EVENTS_CORE_PORT_STALLED_1,
	XAIE_EVENTS_CORE_PORT_TLAST_1,
	XAIE_EVENTS_CORE_PORT_IDLE_2,
	XAIE_EVENTS_CORE_PORT_RUNNING_2,
	XAIE_EVENTS_CORE_PORT_STALLED_2,
	XAIE_EVENTS_CORE_PORT_TLAST_2,
	XAIE_EVENTS_CORE_PORT_IDLE_3,
	XAIE_EVENTS_CORE_PORT_RUNNING_3,
	XAIE_EVENTS_CORE_PORT_STALLED_3,
	XAIE_EVENTS_CORE_PORT_TLAST_3,
	XAIE_EVENTS_CORE_PORT_IDLE_4,
	XAIE_EVENTS_CORE_PORT_RUNNING_4,
	XAIE_EVENTS_CORE_PORT_STALLED_4,
	XAIE_EVENTS_CORE_PORT_TLAST_4,
	XAIE_EVENTS_CORE_PORT_IDLE_5,
	XAIE_EVENTS_CORE_PORT_RUNNING_5,
	XAIE_EVENTS_CORE_PORT_STALLED_5,
	XAIE_EVENTS_CORE_PORT_TLAST_5,
	XAIE_EVENTS_CORE_PORT_IDLE_6,
	XAIE_EVENTS_CORE_PORT_RUNNING_6,
	XAIE_EVENTS_CORE_PORT_STALLED_6,
	XAIE_EVENTS_CORE_PORT_TLAST_6,
	XAIE_EVENTS_CORE_PORT_IDLE_7,
	XAIE_EVENTS_CORE_PORT_RUNNING_7,
	XAIE_EVENTS_CORE_PORT_STALLED_7,
	XAIE_EVENTS_CORE_PORT_TLAST_7,
	XAIE_EVENTS_CORE_GROUP_BROADCAST,
	XAIE_EVENTS_CORE_BROADCAST_0,
	XAIE_EVENTS_CORE_BROADCAST_1,
	XAIE_EVENTS_CORE_BROADCAST_2,
	XAIE_EVENTS_CORE_BROADCAST_3,
	XAIE_EVENTS_CORE_BROADCAST_4,
	XAIE_EVENTS_CORE_BROADCAST_5,
	XAIE_EVENTS_CORE_BROADCAST_6,
	XAIE_EVENTS_CORE_BROADCAST_7,
	XAIE_EVENTS_CORE_BROADCAST_8,
	XAIE_EVENTS_CORE_BROADCAST_9,
	XAIE_EVENTS_CORE_BROADCAST_10,
	XAIE_EVENTS_CORE_BROADCAST_11,
	XAIE_EVENTS_CORE_BROADCAST_12,
	XAIE_EVENTS_CORE_BROADCAST_13,
	XAIE_EVENTS_CORE_BROADCAST_14,
	XAIE_EVENTS_CORE_BROADCAST_15,
	XAIE_EVENTS_CORE_GROUP_USER_EVENT,
	XAIE_EVENTS_CORE_USER_EVENT_0,
	XAIE_EVENTS_CORE_USER_EVENT_1,
	XAIE_EVENTS_CORE_USER_EVENT_2,
	XAIE_EVENTS_CORE_USER_EVENT_3,
	XAIE_EVENTS_CORE_INSTR_EVENT_2,
	XAIE_EVENTS_CORE_INSTR_EVENT_2,
};
/* Enum to Event Number mapping of all events of AIE Mem module */
static const u8 AieTileMemModEventMapping[] =
{
	XAIE_EVENTS_MEM_NONE,
	XAIE_EVENTS_MEM_TRUE,
	XAIE_EVENTS_MEM_GROUP_0,
	XAIE_EVENTS_MEM_TIMER_SYNC,
	XAIE_EVENTS_MEM_TIMER_VALUE_REACHED,
	XAIE_EVENTS_MEM_PERF_CNT_0,
	XAIE_EVENTS_MEM_PERF_CNT_1,
	XAIE_EVENTS_MEM_COMBO_EVENT_0,
	XAIE_EVENTS_MEM_COMBO_EVENT_1,
	XAIE_EVENTS_MEM_COMBO_EVENT_2,
	XAIE_EVENTS_MEM_COMBO_EVENT_3,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENTS_MEM_GROUP_WATCHPOINT,
	XAIE_EVENTS_MEM_WATCHPOINT_0,
	XAIE_EVENTS_MEM_WATCHPOINT_1,
	XAIE_EVENTS_MEM_GROUP_DMA_ACTIVITY,
	XAIE_EVENTS_MEM_DMA_S2MM_0_START_BD,
	XAIE_EVENTS_MEM_DMA_S2MM_1_START_BD,
	XAIE_EVENTS_MEM_DMA_MM2S_0_START_BD,
	XAIE_EVENTS_MEM_DMA_MM2S_1_START_BD,
	XAIE_EVENTS_MEM_DMA_S2MM_0_FINISHED_BD,
	XAIE_EVENTS_MEM_DMA_S2MM_1_FINISHED_BD,
	XAIE_EVENTS_MEM_DMA_MM2S_0_FINISHED_BD,
	XAIE_EVENTS_MEM_DMA_MM2S_1_FINISHED_BD,
	XAIE_EVENTS_MEM_DMA_S2MM_0_GO_TO_IDLE,
	XAIE_EVENTS_MEM_DMA_S2MM_1_GO_TO_IDLE,
	XAIE_EVENTS_MEM_DMA_MM2S_0_GO_TO_IDLE,
	XAIE_EVENTS_MEM_DMA_MM2S_1_GO_TO_IDLE,
	XAIE_EVENTS_MEM_DMA_S2MM_0_STALLED_LOCK_ACQUIRE,
	XAIE_EVENTS_MEM_DMA_S2MM_1_STALLED_LOCK_ACQUIRE,
	XAIE_EVENTS_MEM_DMA_MM2S_0_STALLED_LOCK_ACQUIRE,
	XAIE_EVENTS_MEM_DMA_MM2S_1_STALLED_LOCK_ACQUIRE,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENTS_MEM_DMA_S2MM_0_MEMORY_CONFLICT,
	XAIE_EVENTS_MEM_DMA_S2MM_1_MEMORY_CONFLICT,
	XAIE_EVENTS_MEM_DMA_MM2S_0_MEMORY_CONFLICT,
	XAIE_EVENTS_MEM_DMA_MM2S_1_MEMORY_CONFLICT,
	XAIE_EVENTS_MEM_GROUP_LOCK,
	XAIE_EVENTS_MEM_LOCK_0_ACQ,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENTS_MEM_LOCK_0_REL,
	XAIE_EVENT_INVALID,
	XAIE_EVENTS_MEM_LOCK_1_ACQ,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENTS_MEM_LOCK_1_REL,
	XAIE_EVENT_INVALID,
	XAIE_EVENTS_MEM_LOCK_2_ACQ,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENTS_MEM_LOCK_2_REL,
	XAIE_EVENT_INVALID,
	XAIE_EVENTS_MEM_LOCK_3_ACQ,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENTS_MEM_LOCK_3_REL,
	XAIE_EVENT_INVALID,
	XAIE_EVENTS_MEM_LOCK_4_ACQ,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENTS_MEM_LOCK_4_REL,
	XAIE_EVENT_INVALID,
	XAIE_EVENTS_MEM_LOCK_5_ACQ,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENTS_MEM_LOCK_5_REL,
	XAIE_EVENT_INVALID,
	XAIE_EVENTS_MEM_LOCK_6_ACQ,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENTS_MEM_LOCK_6_REL,
	XAIE_EVENT_INVALID,
	XAIE_EVENTS_MEM_LOCK_7_ACQ,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENTS_MEM_LOCK_7_REL,
	XAIE_EVENT_INVALID,
	XAIE_EVENTS_MEM_LOCK_8_ACQ,
	XAIE_EVENTS_MEM_LOCK_8_REL,
	XAIE_EVENTS_MEM_LOCK_9_ACQ,
	XAIE_EVENTS_MEM_LOCK_9_REL,
	XAIE_EVENTS_MEM_LOCK_10_ACQ,
	XAIE_EVENTS_MEM_LOCK_10_REL,
	XAIE_EVENTS_MEM_LOCK_11_ACQ,
	XAIE_EVENTS_MEM_LOCK_11_REL,
	XAIE_EVENTS_MEM_LOCK_12_ACQ,
	XAIE_EVENTS_MEM_LOCK_12_REL,
	XAIE_EVENTS_MEM_LOCK_13_ACQ,
	XAIE_EVENTS_MEM_LOCK_13_REL,
	XAIE_EVENTS_MEM_LOCK_14_ACQ,
	XAIE_EVENTS_MEM_LOCK_14_REL,
	XAIE_EVENTS_MEM_LOCK_15_ACQ,
	XAIE_EVENTS_MEM_LOCK_15_REL,
	XAIE_EVENTS_MEM_GROUP_MEMORY_CONFLICT,
	XAIE_EVENTS_MEM_CONFLICT_DM_BANK_0,
	XAIE_EVENTS_MEM_CONFLICT_DM_BANK_1,
	XAIE_EVENTS_MEM_CONFLICT_DM_BANK_2,
	XAIE_EVENTS_MEM_CONFLICT_DM_BANK_3,
	XAIE_EVENTS_MEM_CONFLICT_DM_BANK_4,
	XAIE_EVENTS_MEM_CONFLICT_DM_BANK_5,
	XAIE_EVENTS_MEM_CONFLICT_DM_BANK_6,
	XAIE_EVENTS_MEM_CONFLICT_DM_BANK_7,
	XAIE_EVENTS_MEM_GROUP_ERRORS,
	XAIE_EVENTS_MEM_DM_ECC_ERROR_SCRUB_CORRECTED,
	XAIE_EVENTS_MEM_DM_ECC_ERROR_SCRUB_2BIT,
	XAIE_EVENTS_MEM_DM_ECC_ERROR_1BIT,
	XAIE_EVENTS_MEM_DM_ECC_ERROR_2BIT,
	XAIE_EVENTS_MEM_DM_PARITY_ERROR_BANK_2,
	XAIE_EVENTS_MEM_DM_PARITY_ERROR_BANK_3,
	XAIE_EVENTS_MEM_DM_PARITY_ERROR_BANK_4,
	XAIE_EVENTS_MEM_DM_PARITY_ERROR_BANK_5,
	XAIE_EVENTS_MEM_DM_PARITY_ERROR_BANK_6,
	XAIE_EVENTS_MEM_DM_PARITY_ERROR_BANK_7,
	XAIE_EVENTS_MEM_DMA_S2MM_0_ERROR,
	XAIE_EVENTS_MEM_DMA_S2MM_1_ERROR,
	XAIE_EVENTS_MEM_DMA_MM2S_0_ERROR,
	XAIE_EVENTS_MEM_DMA_MM2S_1_ERROR,
	XAIE_EVENT_INVALID,
	XAIE_EVENT_INVALID,
	XAIE_EVENTS_MEM_GROUP_BROADCAST,
	XAIE_EVENTS_MEM_BROADCAST_0,
	XAIE_EVENTS_MEM_BROADCAST_1,
	XAIE_EVENTS_MEM_BROADCAST_2,
	XAIE_EVENTS_MEM_BROADCAST_3,
	XAIE_EVENTS_MEM_BROADCAST_4,
	XAIE_EVENTS_MEM_BROADCAST_5,
	XAIE_EVENTS_MEM_BROADCAST_6,
	XAIE_EVENTS_MEM_BROADCAST_7,
	XAIE_EVENTS_MEM_BROADCAST_8,
	XAIE_EVENTS_MEM_BROADCAST_9,
	XAIE_EVENTS_MEM_BROADCAST_10,
	XAIE_EVENTS_MEM_BROADCAST_11,
	XAIE_EVENTS_MEM_BROADCAST_12,
	XAIE_EVENTS_MEM_BROADCAST_13,
	XAIE_EVENTS_MEM_BROADCAST_14,
	XAIE_EVENTS_MEM_BROADCAST_15,
	XAIE_EVENTS_MEM_GROUP_USER_EVENT,
	XAIE_EVENTS_MEM_USER_EVENT_0,
	XAIE_EVENTS_MEM_USER_EVENT_1,
	XAIE_EVENTS_MEM_USER_EVENT_2,
	XAIE_EVENTS_MEM_USER_EVENT_3,
};

/* Enum to Event Number mapping of all events of AIE PL module */
static const u8 AieTilePlModEventMapping[] =
{
	XAIE_EVENTS_PL_NONE,
	XAIE_EVENTS_PL_TRUE,
	XAIE_EVENTS_PL_GROUP_0,
	XAIE_EVENTS_PL_TIMER_SYNC,
	XAIE_EVENTS_PL_TIMER_VALUE_REACHED,
	XAIE_EVENTS_PL_PERF_CNT_0,
	XAIE_EVENTS_PL_PERF_CNT_1,
	XAIE_EVENTS_PL_COMBO_EVENT_0,
	XAIE_EVENTS_PL_COMBO_EVENT_1,
	XAIE_EVENTS_PL_COMBO_EVENT_2,
	XAIE_EVENTS_PL_COMBO_EVENT_3,
	XAIE_EVENTS_PL_GROUP_DMA_ACTIVITY,
	XAIE_EVENTS_PL_DMA_S2MM_0_START_BD,
	XAIE_EVENTS_PL_DMA_S2MM_1_START_BD,
	XAIE_EVENTS_PL_DMA_MM2S_0_START_BD,
	XAIE_EVENTS_PL_DMA_MM2S_1_START_BD,
	XAIE_EVENTS_PL_DMA_S2MM_0_FINISHED_BD,
	XAIE_EVENTS_PL_DMA_S2MM_1_FINISHED_BD,
	XAIE_EVENTS_PL_DMA_MM2S_0_FINISHED_BD,
	XAIE_EVENTS_PL_DMA_MM2S_1_FINISHED_BD,
	XAIE_EVENTS_PL_DMA_S2MM_0_GO_TO_IDLE,
	XAIE_EVENTS_PL_DMA_S2MM_1_GO_TO_IDLE,
	XAIE_EVENTS_PL_DMA_MM2S_0_GO_TO_IDLE,
	XAIE_EVENTS_PL_DMA_MM2S_1_GO_TO_IDLE,
	XAIE_EVENTS_PL_DMA_S2MM_0_STALLED_LOCK_ACQUIRE,
	XAIE_EVENTS_PL_DMA_S2MM_1_STALLED_LOCK_ACQUIRE,
	XAIE_EVENTS_PL_DMA_MM2S_0_STALLED_LOCK_ACQUIRE,
	XAIE_EVENTS_PL_DMA_MM2S_1_STALLED_LOCK_ACQUIRE,
	XAIE_EVENTS_PL_GROUP_LOCK,
	XAIE_EVENTS_PL_LOCK_0_ACQUIRED,
	XAIE_EVENTS_PL_LOCK_0_RELEASED,
	XAIE_EVENTS_PL_LOCK_1_ACQUIRED,
	XAIE_EVENTS_PL_LOCK_1_RELEASED,
	XAIE_EVENTS_PL_LOCK_2_ACQUIRED,
	XAIE_EVENTS_PL_LOCK_2_RELEASED,
	XAIE_EVENTS_PL_LOCK_3_ACQUIRED,
	XAIE_EVENTS_PL_LOCK_3_RELEASED,
	XAIE_EVENTS_PL_LOCK_4_ACQUIRED,
	XAIE_EVENTS_PL_LOCK_4_RELEASED,
	XAIE_EVENTS_PL_LOCK_5_ACQUIRED,
	XAIE_EVENTS_PL_LOCK_5_RELEASED,
	XAIE_EVENTS_PL_LOCK_6_ACQUIRED,
	XAIE_EVENTS_PL_LOCK_6_RELEASED,
	XAIE_EVENTS_PL_LOCK_7_ACQUIRED,
	XAIE_EVENTS_PL_LOCK_7_RELEASED,
	XAIE_EVENTS_PL_LOCK_8_ACQUIRED,
	XAIE_EVENTS_PL_LOCK_8_RELEASED,
	XAIE_EVENTS_PL_LOCK_9_ACQUIRED,
	XAIE_EVENTS_PL_LOCK_9_RELEASED,
	XAIE_EVENTS_PL_LOCK_10_ACQUIRED,
	XAIE_EVENTS_PL_LOCK_10_RELEASED,
	XAIE_EVENTS_PL_LOCK_11_ACQUIRED,
	XAIE_EVENTS_PL_LOCK_11_RELEASED,
	XAIE_EVENTS_PL_LOCK_12_ACQUIRED,
	XAIE_EVENTS_PL_LOCK_12_RELEASED,
	XAIE_EVENTS_PL_LOCK_13_ACQUIRED,
	XAIE_EVENTS_PL_LOCK_13_RELEASED,
	XAIE_EVENTS_PL_LOCK_14_ACQUIRED,
	XAIE_EVENTS_PL_LOCK_14_RELEASED,
	XAIE_EVENTS_PL_LOCK_15_ACQUIRED,
	XAIE_EVENTS_PL_LOCK_15_RELEASED,
	XAIE_EVENTS_PL_GROUP_ERRORS,
	XAIE_EVENTS_PL_AXI_MM_SLAVE_TILE_ERROR,
	XAIE_EVENTS_PL_CONTROL_PKT_ERROR,
	XAIE_EVENTS_PL_AXI_MM_DECODE_NSU_ERROR,
	XAIE_EVENTS_PL_AXI_MM_SLAVE_NSU_ERROR,
	XAIE_EVENTS_PL_AXI_MM_UNSUPPORTED_TRAFFIC,
	XAIE_EVENTS_PL_AXI_MM_UNSECURE_ACCESS_IN_SECURE_MODE,
	XAIE_EVENTS_PL_AXI_MM_BYTE_STROBE_ERROR,
	XAIE_EVENTS_PL_DMA_S2MM_0_ERROR,
	XAIE_EVENTS_PL_DMA_S2MM_1_ERROR,
	XAIE_EVENTS_PL_DMA_MM2S_0_ERROR,
	XAIE_EVENTS_PL_DMA_MM2S_1_ERROR,
	XAIE_EVENTS_PL_GROUP_STREAM_SWITCH,
	XAIE_EVENTS_PL_PORT_IDLE_0,
	XAIE_EVENTS_PL_PORT_RUNNING_0,
	XAIE_EVENTS_PL_PORT_STALLED_0,
	XAIE_EVENTS_PL_PORT_TLAST_0,
	XAIE_EVENTS_PL_PORT_IDLE_1,
	XAIE_EVENTS_PL_PORT_RUNNING_1,
	XAIE_EVENTS_PL_PORT_STALLED_1,
	XAIE_EVENTS_PL_PORT_TLAST_1,
	XAIE_EVENTS_PL_PORT_IDLE_2,
	XAIE_EVENTS_PL_PORT_RUNNING_2,
	XAIE_EVENTS_PL_PORT_STALLED_2,
	XAIE_EVENTS_PL_PORT_TLAST_2,
	XAIE_EVENTS_PL_PORT_IDLE_3,
	XAIE_EVENTS_PL_PORT_RUNNING_3,
	XAIE_EVENTS_PL_PORT_STALLED_3,
	XAIE_EVENTS_PL_PORT_TLAST_3,
	XAIE_EVENTS_PL_PORT_IDLE_4,
	XAIE_EVENTS_PL_PORT_RUNNING_4,
	XAIE_EVENTS_PL_PORT_STALLED_4,
	XAIE_EVENTS_PL_PORT_TLAST_4,
	XAIE_EVENTS_PL_PORT_IDLE_5,
	XAIE_EVENTS_PL_PORT_RUNNING_5,
	XAIE_EVENTS_PL_PORT_STALLED_5,
	XAIE_EVENTS_PL_PORT_TLAST_5,
	XAIE_EVENTS_PL_PORT_IDLE_6,
	XAIE_EVENTS_PL_PORT_RUNNING_6,
	XAIE_EVENTS_PL_PORT_STALLED_6,
	XAIE_EVENTS_PL_PORT_TLAST_6,
	XAIE_EVENTS_PL_PORT_IDLE_7,
	XAIE_EVENTS_PL_PORT_RUNNING_7,
	XAIE_EVENTS_PL_PORT_STALLED_7,
	XAIE_EVENTS_PL_PORT_TLAST_7,
	XAIE_EVENTS_PL_GROUP_BROADCAST_A,
	XAIE_EVENTS_PL_BROADCAST_0,
	XAIE_EVENTS_PL_BROADCAST_A_1,
	XAIE_EVENTS_PL_BROADCAST_A_2,
	XAIE_EVENTS_PL_BROADCAST_A_3,
	XAIE_EVENTS_PL_BROADCAST_A_4,
	XAIE_EVENTS_PL_BROADCAST_A_5,
	XAIE_EVENTS_PL_BROADCAST_A_6,
	XAIE_EVENTS_PL_BROADCAST_A_7,
	XAIE_EVENTS_PL_BROADCAST_A_8,
	XAIE_EVENTS_PL_BROADCAST_A_9,
	XAIE_EVENTS_PL_BROADCAST_A_10,
	XAIE_EVENTS_PL_BROADCAST_A_11,
	XAIE_EVENTS_PL_BROADCAST_A_12,
	XAIE_EVENTS_PL_BROADCAST_A_13,
	XAIE_EVENTS_PL_BROADCAST_A_14,
	XAIE_EVENTS_PL_BROADCAST_A_15,
	XAIE_EVENTS_PL_GROUP_USER_EVENT,
	XAIE_EVENTS_PL_USER_EVENT_0,
	XAIE_EVENTS_PL_USER_EVENT_1,
	XAIE_EVENTS_PL_USER_EVENT_2,
	XAIE_EVENTS_PL_USER_EVENT_3,
};

/*
 * Data Structure contains all registers and offset info for perf counter
 * of AIE Core and Memory Module.
 */
static const XAie_PerfMod AieTilePerfCnt[] =
{
	{
		.MaxCounterVal = 2U,
		.StartStopShift = 16U,
		.ResetShift = 8U,
		.PerfCounterOffsetAdd = 0x4,
		.PerfCtrlBaseAddr = XAIEGBL_MEM_PERCTRL0,
		.PerfCtrlOffsetAdd = 0x0,
		.PerfCtrlResetBaseAddr = XAIEGBL_MEM_PERCTRL1,
		.PerfCounterBaseAddr = XAIEGBL_MEM_PERCOU0,
		.PerfCounterEvtValBaseAddr = XAIEGBL_MEM_PERCOU0EVTVAL,
		{XAIEGBL_MEM_PERCTRL0_CNT0STAEVT_LSB, XAIEGBL_MEM_PERCTRL0_CNT0STAEVT_MASK},
		{XAIEGBL_MEM_PERCTRL0_CNT0STOPEVT_LSB, XAIEGBL_MEM_PERCTRL0_CNT0STOPEVT_MASK},
		{XAIEGBL_MEM_PERCTRL1_CNT0RSTEVT_LSB, XAIEGBL_MEM_PERCTRL1_CNT0RSTEVT_MASK},
	},
	{
		.MaxCounterVal = 4U,
		.StartStopShift = 16U,
		.ResetShift = 8U,
		.PerfCounterOffsetAdd = 0x4,
		.PerfCtrlBaseAddr = XAIEGBL_CORE_PERCTR0,
		.PerfCtrlOffsetAdd = 0x4,
		.PerfCtrlResetBaseAddr = XAIEGBL_CORE_PERCTR2,
		.PerfCounterBaseAddr = XAIEGBL_CORE_PERCOU0,
		.PerfCounterEvtValBaseAddr = XAIEGBL_CORE_PERCOU0EVTVAL,
		{XAIEGBL_CORE_PERCTR0_CNT0STAEVT_LSB, XAIEGBL_CORE_PERCTR0_CNT0STAEVT_MASK},
		{XAIEGBL_CORE_PERCTR0_CNT0STOPEVT_LSB, XAIEGBL_CORE_PERCTR0_CNT0STOPEVT_MASK},
		{XAIEGBL_CORE_PERCTR2_CNT0RSTEVT_LSB, XAIEGBL_CORE_PERCTR2_CNT0RSTEVT_MASK},
	}
};

/*
 * Data Structure contains all registers and offset info for perf counter
 * of AIE PL Module.
 */
static const XAie_PerfMod AiePlPerfCnt =
{
	.MaxCounterVal = 2U,
	.StartStopShift = 16U,
	.ResetShift = 8U,
	.PerfCounterOffsetAdd = 0x4,
	.PerfCtrlBaseAddr = XAIEGBL_PL_PERCTR0,
	.PerfCtrlOffsetAdd = 0x0,
	.PerfCtrlResetBaseAddr = XAIEGBL_PL_PERCTR1,
	.PerfCounterBaseAddr = XAIEGBL_PL_PERCOU0,
	.PerfCounterEvtValBaseAddr = XAIEGBL_PL_PERCOU0EVTVAL,
	{XAIEGBL_PL_PERCTR0_CNT0STAEVT_LSB, XAIEGBL_PL_PERCTR0_CNT0STAEVT_MASK},
	{XAIEGBL_PL_PERCTR0_CNT0STOPEVT_LSB, XAIEGBL_PL_PERCTR0_CNT0STOPEVT_MASK},
	{XAIEGBL_PL_PERCTR1_CNT0RSTEVT_LSB, XAIEGBL_PL_PERCTR1_CNT0RSTEVT_MASK},
};

/* Data structure to capture core and memory module events properties */
static const XAie_EvntMod AieTileEvntMod[] =
{
	{
		.XAie_EventNumber = AieTileMemModEventMapping,
		.EventMin = 1000U,
		.EventMax = 1149U,
	},
	{
		.XAie_EventNumber = AieTileCoreModEventMapping,
		.EventMin = 0U,
		.EventMax = 128U,
	},
};

/* Data structure to capture PL module events properties */
static const XAie_EvntMod AiePlEvntMod =
{
	.XAie_EventNumber = AieTilePlModEventMapping,
	.EventMin = 2000U,
	.EventMax = 2160U,
};

/* Data structure to capture core and mem module timer properties */
static XAie_TimerMod AieTileTimerMod[] =
{
	{
		.TrigEventLowValOff = XAIEGBL_MEM_TIMTRIEVTLOWVAL,
		.TrigEventHighValOff = XAIEGBL_MEM_TIMTRIEVTHIGVAL,
		.LowOff = XAIEGBL_MEM_TIMLOW,
		.HighOff = XAIEGBL_MEM_TIMHIG,
		.CtrlOff = XAIEGBL_MEM_TIMCTRL,
		{XAIEGBL_MEM_TIMCTRL_RST_LSB, XAIEGBL_MEM_TIMCTRL_RST_MASK},
		{XAIEGBL_MEM_TIMCTRL_RSTEVT_LSB, XAIEGBL_MEM_TIMCTRL_RSTEVT_MASK},
	},
	{
		.TrigEventLowValOff = XAIEGBL_CORE_TIMTRIEVTLOWVAL,
		.TrigEventHighValOff = XAIEGBL_CORE_TIMTRIEVTHIGVAL,
		.LowOff = XAIEGBL_CORE_TIMLOW,
		.HighOff = XAIEGBL_CORE_TIMHIG,
		.CtrlOff = XAIEGBL_CORE_TIMCTRL,
		{XAIEGBL_CORE_TIMCTRL_RST_LSB, XAIEGBL_CORE_TIMCTRL_RST_MASK},
		{XAIEGBL_CORE_TIMCTRL_RSTEVT_LSB, XAIEGBL_CORE_TIMCTRL_RSTEVT_MASK},
	},
};

/* Data structure to capture PL module timer properties */
static XAie_TimerMod AiePlTimerMod =
{
	.TrigEventLowValOff = XAIEGBL_PL_TIMTRIEVTLOWVAL,
	.TrigEventHighValOff = XAIEGBL_PL_TIMTRIEVTHIGVAL,
	.LowOff = XAIEGBL_PL_TIMLOW,
	.HighOff = XAIEGBL_PL_TIMHIG,
	.CtrlOff = XAIEGBL_PL_TIMCTRL,
	{XAIEGBL_PL_TIMCTRL_RST_LSB, XAIEGBL_PL_TIMCTRL_RST_MASK},
	{XAIEGBL_PL_TIMCTRL_RSTEVT_LSB, XAIEGBL_PL_TIMCTRL_RSTEVT_MASK},
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
		.DmaMod  = &AieTileDmaMod,
		.MemMod  = &AieTileMemMod,
		.PlIfMod = NULL,
		.LockMod = &AieTileLockMod,
		.PerfMod = AieTilePerfCnt,
		.EvntMod = AieTileEvntMod,
		.TimerMod = AieTileTimerMod,
	},
	{
		/*
		 * AIE Shim Noc Module indexed using XAIEGBL_TILE_TYPE_SHIMNOC
		 */
		.CoreMod = NULL,
		.StrmSw  = &AieShimStrmSw,
		.DmaMod  = &AieShimDmaMod,
		.MemMod  = NULL,
		.PlIfMod = &AieShimTilePlIfMod,
		.LockMod = &AieShimNocLockMod,
		.PerfMod = &AiePlPerfCnt,
		.EvntMod = &AiePlEvntMod,
		.TimerMod = &AiePlTimerMod,
	},
	{
		/*
		 * AIE Shim PL Module indexed using XAIEGBL_TILE_TYPE_SHIMPL
		 */
		.CoreMod = NULL,
		.StrmSw  = &AieShimStrmSw,
		.DmaMod  = NULL,
		.MemMod  = NULL,
		.PlIfMod = &AiePlIfMod,
		.LockMod = NULL,
		.PerfMod = &AiePlPerfCnt,
		.EvntMod = &AiePlEvntMod,
		.TimerMod = &AiePlTimerMod,
	},
	{
		/*
		 * AIE MemTile Module indexed using XAIEGBL_TILE_TYPE_MEMTILE
		 */
		.CoreMod = NULL,
		.StrmSw  = NULL,
		.DmaMod  = NULL,
		.MemMod  = NULL,
		.PlIfMod = NULL,
		.LockMod = NULL,
		.PerfMod = NULL,
		.EvntMod = NULL,
		.TimerMod = NULL,
	}
};

/** @} */
