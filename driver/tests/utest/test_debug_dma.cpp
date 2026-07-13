#include "CppUTest/TestHarness.h"
#include <hw_config.h>

/*
 * The AIE branch driver (main-aie-next) only supports AIE (1), AIE-ML (2)
 * and AIE2PS (5). AIE2IPU (3) and AIE2P (4) have no XAie_CfgInitialize
 * support on this branch, so these tests are compiled out for those
 * generations, matching the #if AIE_GEN guards used by the other utests.
 */
#if (AIE_GEN == 1) || (AIE_GEN == 2) || (AIE_GEN == 5)

static XAie_SetupConfig(ConfigPtr, HW_GEN, XAIE_BASE_ADDR,
		XAIE_COL_SHIFT, XAIE_ROW_SHIFT,
		XAIE_NUM_COLS, XAIE_NUM_ROWS, XAIE_SHIM_ROW,
		XAIE_MEM_TILE_ROW_START, XAIE_MEM_TILE_NUM_ROWS,
		XAIE_AIE_TILE_ROW_START, XAIE_AIE_TILE_NUM_ROWS);

static XAie_InstDeclare(DevInst, &ConfigPtr);

TEST_GROUP(DmaTxnCountApis)
{
	void setup()
	{
		AieRC RC;

		RC = XAie_CfgInitialize(&(DevInst), &ConfigPtr);
		CHECK_EQUAL(XAIE_OK, RC);

		RC = XAie_PartitionInitialize(&(DevInst), NULL);
		CHECK_EQUAL(XAIE_OK, RC);
	}
	void teardown()
	{
		AieRC RC;

		RC = XAie_PartitionTeardown(&(DevInst));
		CHECK_EQUAL(XAIE_OK, RC);

		XAie_Finish(&DevInst);
	}
};

/* Helper macros to pick guaranteed tile coords regardless of generation. */
#define DMA_TXN_AIE_TILE_COL	1
#define DMA_TXN_AIE_TILE_ROW	XAIE_AIE_TILE_ROW_START
#define DMA_TXN_AIE_TILE	XAie_TileLoc(DMA_TXN_AIE_TILE_COL, \
					DMA_TXN_AIE_TILE_ROW)

/* Single-tile XAie_SubPartition literal for the standard AIE tile. */
#define DMA_TXN_AIE_SUBPART	{ {DMA_TXN_AIE_TILE_COL, 1}, \
				  {DMA_TXN_AIE_TILE_ROW, 1} }

TEST(DmaTxnCountApis, EnableDisableSingleTile)
{
	AieRC RC;
	XAie_SubPartition Sub = DMA_TXN_AIE_SUBPART;

	RC = XAie_DmaTxnCountEnable(&DevInst, &Sub, 1,
			XAIE_DMA_CH_S2MM_0);
	CHECK_EQUAL(XAIE_OK, RC);

	RC = XAie_DmaTxnCountDisable(&DevInst, &Sub, 1,
			XAIE_DMA_CH_S2MM_0);
	CHECK_EQUAL(XAIE_OK, RC);
}

TEST(DmaTxnCountApis, EnableDisableMultipleTiles)
{
	AieRC RC;
	/*
	 * Three single-tile sub-partitions across three columns at the
	 * AIE-tile row. Exercises the "list of rectangles" walk where
	 * each rectangle is 1x1.
	 */
	XAie_SubPartition SubParts[] = {
		{ {1, 1}, {XAIE_AIE_TILE_ROW_START, 1} },
		{ {2, 1}, {XAIE_AIE_TILE_ROW_START, 1} },
		{ {3, 1}, {XAIE_AIE_TILE_ROW_START, 1} },
	};

	/* AIE tile memory module has 2 perf counters: one channel fits. */
	RC = XAie_DmaTxnCountEnable(&DevInst, SubParts, 3,
			XAIE_DMA_CH_S2MM_0);
	CHECK_EQUAL(XAIE_OK, RC);

	RC = XAie_DmaTxnCountDisable(&DevInst, SubParts, 3,
			XAIE_DMA_CH_S2MM_0);
	CHECK_EQUAL(XAIE_OK, RC);
}

TEST(DmaTxnCountApis, EnableDisableRectangleStrip)
{
	AieRC RC;
	/*
	 * Single rectangular sub-partition: column 1, rows
	 * AIE_TILE_ROW_START..ROW_START+1 (or +0 if only one row exists).
	 * Proves the rectangle expansion in Enable/Disable: one entry in
	 * SubParts covers multiple tiles.
	 */
	u8 NumRows = (XAIE_AIE_TILE_NUM_ROWS >= 2U) ? 2U : 1U;
	XAie_SubPartition Strip = {
		{1, 1},
		{XAIE_AIE_TILE_ROW_START, NumRows},
	};

	RC = XAie_DmaTxnCountEnable(&DevInst, &Strip, 1,
			XAIE_DMA_CH_S2MM_0);
	CHECK_EQUAL(XAIE_OK, RC);

	RC = XAie_DmaTxnCountDisable(&DevInst, &Strip, 1,
			XAIE_DMA_CH_S2MM_0);
	CHECK_EQUAL(XAIE_OK, RC);
}

TEST(DmaTxnCountApis, GetTxnCount)
{
	AieRC RC;
	XAie_SubPartition Sub = DMA_TXN_AIE_SUBPART;
	XAie_LocType TileLoc = DMA_TXN_AIE_TILE;
	u32 EndCount = 0xDEADBEEF;
	u32 ChGroup = XAIE_DMA_CH_S2MM_0;

	RC = XAie_DmaTxnCountEnable(&DevInst, &Sub, 1, ChGroup);
	CHECK_EQUAL(XAIE_OK, RC);

	RC = XAie_DmaTxnCountGet(&DevInst, TileLoc, ChGroup,
			XAIE_DMA_CH_S2MM_0, &EndCount);
	CHECK_EQUAL(XAIE_OK, RC);
	/*
	 * Simulated IO backend returns a synthetic value, not what was
	 * written, so the actual count is verified by the system test.
	 * Just confirm Get touched the output buffer.
	 */
	CHECK(EndCount != 0xDEADBEEF);

	RC = XAie_DmaTxnCountDisable(&DevInst, &Sub, 1, ChGroup);
	CHECK_EQUAL(XAIE_OK, RC);
}

TEST(DmaTxnCountApis, GetSecondChannelInGroup)
{
	AieRC RC;
	/*
	 * Mem tile has 4 perf counters, one per channel, so all 4 channels
	 * fit in one group. Reading the last flag (MM2S_1) exercises
	 * counter slot 3 — the deterministic packing must resolve it
	 * correctly.
	 */
#if XAIE_MEM_TILE_NUM_ROWS > 0
	XAie_SubPartition Sub = { {1, 1}, {XAIE_MEM_TILE_ROW_START, 1} };
	XAie_LocType TileLoc = XAie_TileLoc(1, XAIE_MEM_TILE_ROW_START);
	u32 ChGroup = XAIE_DMA_CH_ALL;
	u32 EndCount;

	RC = XAie_DmaTxnCountEnable(&DevInst, &Sub, 1, ChGroup);
	CHECK_EQUAL(XAIE_OK, RC);

	RC = XAie_DmaTxnCountGet(&DevInst, TileLoc, ChGroup,
			XAIE_DMA_CH_MM2S_1, &EndCount);
	CHECK_EQUAL(XAIE_OK, RC);

	RC = XAie_DmaTxnCountDisable(&DevInst, &Sub, 1, ChGroup);
	CHECK_EQUAL(XAIE_OK, RC);
#endif
}

TEST(DmaTxnCountApis, GetRejectsBadArgs)
{
	AieRC RC;
	XAie_LocType TileLoc = DMA_TXN_AIE_TILE;
	u32 EndCount;
	u32 ChGroup = XAIE_DMA_CH_S2MM_0;

	RC = XAie_DmaTxnCountGet(NULL, TileLoc, ChGroup, XAIE_DMA_CH_S2MM_0,
			&EndCount);
	CHECK_EQUAL(XAIE_INVALID_ARGS, RC);

	RC = XAie_DmaTxnCountGet(&DevInst, TileLoc, ChGroup,
			XAIE_DMA_CH_S2MM_0, NULL);
	CHECK_EQUAL(XAIE_INVALID_ARGS, RC);

	/* Empty DmaChGroup */
	RC = XAie_DmaTxnCountGet(&DevInst, TileLoc, 0U,
			XAIE_DMA_CH_S2MM_0, &EndCount);
	CHECK_EQUAL(XAIE_INVALID_ARGS, RC);

	/* DmaChFlag must be exactly one channel bit. */
	RC = XAie_DmaTxnCountGet(&DevInst, TileLoc, ChGroup, 0U,
			&EndCount);
	CHECK_EQUAL(XAIE_INVALID_ARGS, RC);

	RC = XAie_DmaTxnCountGet(&DevInst, TileLoc, ChGroup,
			XAIE_DMA_CH_S2MM_0 | XAIE_DMA_CH_MM2S_0,
			&EndCount);
	CHECK_EQUAL(XAIE_INVALID_ARGS, RC);

	/* DmaChFlag not present in DmaChGroup */
	RC = XAie_DmaTxnCountGet(&DevInst, TileLoc, ChGroup,
			XAIE_DMA_CH_MM2S_1, &EndCount);
	CHECK_EQUAL(XAIE_INVALID_ARGS, RC);
}

TEST(DmaTxnCountApis, EmptyChannelGroupRejected)
{
	AieRC RC;
	XAie_SubPartition Sub = DMA_TXN_AIE_SUBPART;

	RC = XAie_DmaTxnCountEnable(&DevInst, &Sub, 1, 0U);
	CHECK_EQUAL(XAIE_INVALID_ARGS, RC);
}

TEST(DmaTxnCountApis, EmptyRangeRejected)
{
	AieRC RC;
	/* Zero-row range and zero-col range must both be rejected. */
	XAie_SubPartition ZeroRow = { {1, 1}, {XAIE_AIE_TILE_ROW_START, 0} };
	XAie_SubPartition ZeroCol = { {1, 0}, {XAIE_AIE_TILE_ROW_START, 1} };

	RC = XAie_DmaTxnCountEnable(&DevInst, &ZeroRow, 1,
			XAIE_DMA_CH_S2MM_0);
	CHECK_EQUAL(XAIE_INVALID_ARGS, RC);

	RC = XAie_DmaTxnCountEnable(&DevInst, &ZeroCol, 1,
			XAIE_DMA_CH_S2MM_0);
	CHECK_EQUAL(XAIE_INVALID_ARGS, RC);
}

TEST(DmaTxnCountApis, ExceedsCounterBudgetRejected)
{
	AieRC RC;
	XAie_SubPartition Sub = DMA_TXN_AIE_SUBPART;

	/*
	 * AIE tile memory module has 2 perf counters; each channel needs 1.
	 * Requesting 3 channels (3 counters) must be rejected.
	 */
	RC = XAie_DmaTxnCountEnable(&DevInst, &Sub, 1,
			XAIE_DMA_CH_S2MM_0 | XAIE_DMA_CH_S2MM_1 |
			XAIE_DMA_CH_MM2S_0);
	CHECK_EQUAL(XAIE_ERR, RC);
}

TEST(DmaTxnCountApis, NullDevInst)
{
	AieRC RC;
	XAie_SubPartition Sub = DMA_TXN_AIE_SUBPART;

	RC = XAie_DmaTxnCountEnable(NULL, &Sub, 1,
			XAIE_DMA_CH_S2MM_0);
	CHECK(XAIE_OK != RC);
}

TEST(DmaTxnCountApis, DisableRejectsBadArgs)
{
	AieRC RC;
	XAie_SubPartition Sub = DMA_TXN_AIE_SUBPART;

	RC = XAie_DmaTxnCountDisable(NULL, &Sub, 1,
			XAIE_DMA_CH_S2MM_0);
	CHECK_EQUAL(XAIE_INVALID_ARGS, RC);

	RC = XAie_DmaTxnCountDisable(&DevInst, NULL, 1,
			XAIE_DMA_CH_S2MM_0);
	CHECK_EQUAL(XAIE_INVALID_ARGS, RC);

	RC = XAie_DmaTxnCountDisable(&DevInst, &Sub, 0,
			XAIE_DMA_CH_S2MM_0);
	CHECK_EQUAL(XAIE_INVALID_ARGS, RC);

	RC = XAie_DmaTxnCountDisable(&DevInst, &Sub, 1, 0U);
	CHECK_EQUAL(XAIE_INVALID_ARGS, RC);
}

TEST(DmaTxnCountApis, EnableDisableEnableReusesCounters)
{
	AieRC RC;
	XAie_SubPartition Sub = DMA_TXN_AIE_SUBPART;

	/*
	 * After Disable, the counters must be free for the next Enable.
	 * Round-tripping the same channel twice catches a Disable that
	 * forgot to clear the perf-counter control register.
	 */
	RC = XAie_DmaTxnCountEnable(&DevInst, &Sub, 1,
			XAIE_DMA_CH_S2MM_0);
	CHECK_EQUAL(XAIE_OK, RC);

	RC = XAie_DmaTxnCountDisable(&DevInst, &Sub, 1,
			XAIE_DMA_CH_S2MM_0);
	CHECK_EQUAL(XAIE_OK, RC);

	RC = XAie_DmaTxnCountEnable(&DevInst, &Sub, 1,
			XAIE_DMA_CH_MM2S_0);
	CHECK_EQUAL(XAIE_OK, RC);

	RC = XAie_DmaTxnCountDisable(&DevInst, &Sub, 1,
			XAIE_DMA_CH_MM2S_0);
	CHECK_EQUAL(XAIE_OK, RC);
}

/*
 * Mem tile only exists on AIE-ML and later. Guard the Mem-tile-specific test
 * so it is not compiled for Gen1 builds where XAIE_MEM_TILE_NUM_ROWS == 0.
 */
#if XAIE_MEM_TILE_NUM_ROWS > 0

#define DMA_TXN_MEM_TILE	XAie_TileLoc(1, XAIE_MEM_TILE_ROW_START)
#define DMA_TXN_MEM_SUBPART	{ {1, 1}, {XAIE_MEM_TILE_ROW_START, 1} }

TEST(DmaTxnCountApis, MemTileAllFourChannels)
{
	AieRC RC;
	XAie_SubPartition Sub = DMA_TXN_MEM_SUBPART;
	u32 ChGroup = XAIE_DMA_CH_ALL;

	/*
	 * Mem tile has 4 perf counters, one per channel: all 4 channels fit
	 * concurrently now that each channel only needs 1 counter.
	 */
	RC = XAie_DmaTxnCountEnable(&DevInst, &Sub, 1, ChGroup);
	CHECK_EQUAL(XAIE_OK, RC);

	RC = XAie_DmaTxnCountDisable(&DevInst, &Sub, 1, ChGroup);
	CHECK_EQUAL(XAIE_OK, RC);
}

#endif /* XAIE_MEM_TILE_NUM_ROWS > 0 */

/*
 * Tests for the partition-level v2 wrapper. These manage their own partition
 * lifecycle so they live in a separate TEST_GROUP whose setup()/teardown()
 * only handle XAie_CfgInitialize / XAie_Finish.
 */
TEST_GROUP(PartitionInitV2Apis)
{
	void setup()
	{
		AieRC RC = XAie_CfgInitialize(&DevInst, &ConfigPtr);
		CHECK_EQUAL(XAIE_OK, RC);
	}
	void teardown()
	{
		XAie_Finish(&DevInst);
	}
};

TEST(PartitionInitV2Apis, NullOptsBehavesAsLegacyInit)
{
	AieRC RC = XAie_PartitionInitialize_v2(&DevInst, NULL);
	CHECK_EQUAL(XAIE_OK, RC);

	RC = XAie_PartitionTeardown_v2(&DevInst, NULL);
	CHECK_EQUAL(XAIE_OK, RC);
}

TEST(PartitionInitV2Apis, ZeroChGroupSkipsCounterSetup)
{
	XAie_SubPartition Sub = DMA_TXN_AIE_SUBPART;
	XAie_PartInitOpts_v2 Opts = {};
	Opts.SubParts = &Sub;
	Opts.NumSubParts = 1;
	Opts.InitOpts = XAIE_PART_INIT_OPT_DEFAULT;
	/* Opts.DmaTxnCountChGroup == 0 => feature off */

	AieRC RC = XAie_PartitionInitialize_v2(&DevInst, &Opts);
	CHECK_EQUAL(XAIE_OK, RC);

	RC = XAie_PartitionTeardown_v2(&DevInst, &Opts);
	CHECK_EQUAL(XAIE_OK, RC);
}

TEST(PartitionInitV2Apis, NonZeroChGroupArmsCounters)
{
	XAie_SubPartition Sub = DMA_TXN_AIE_SUBPART;
	XAie_LocType Tile = DMA_TXN_AIE_TILE;
	XAie_PartInitOpts_v2 Opts = {};
	Opts.SubParts = &Sub;
	Opts.NumSubParts = 1;
	Opts.InitOpts = XAIE_PART_INIT_OPT_DEFAULT;
	Opts.DmaTxnCountChGroup = XAIE_DMA_CH_S2MM_0;

	AieRC RC = XAie_PartitionInitialize_v2(&DevInst, &Opts);
	CHECK_EQUAL(XAIE_OK, RC);

	/*
	 * The counter is armed; verify Get sees it via the same opts.
	 * (Read-back values are not meaningful in the simulated IO backend;
	 * we just confirm Get does not error out.)
	 */
	u32 EndCount = 0xDEADBEEF;
	RC = XAie_DmaTxnCountGet(&DevInst, Tile, Opts.DmaTxnCountChGroup,
			XAIE_DMA_CH_S2MM_0, &EndCount);
	CHECK_EQUAL(XAIE_OK, RC);
	CHECK(EndCount != 0xDEADBEEF);

	RC = XAie_PartitionTeardown_v2(&DevInst, &Opts);
	CHECK_EQUAL(XAIE_OK, RC);
}

TEST(PartitionInitV2Apis, BudgetOverrunRollsBack)
{
	XAie_SubPartition Sub = DMA_TXN_AIE_SUBPART;
	XAie_PartInitOpts_v2 Opts = {};
	Opts.SubParts = &Sub;
	Opts.NumSubParts = 1;
	Opts.InitOpts = XAIE_PART_INIT_OPT_DEFAULT;
	/*
	 * AIE tile mem module has only 2 perf counters; asking for 3
	 * channels (3 counters) must fail at counter setup and the wrapper
	 * must roll back the partition init.
	 */
	Opts.DmaTxnCountChGroup = XAIE_DMA_CH_S2MM_0 | XAIE_DMA_CH_S2MM_1 |
			XAIE_DMA_CH_MM2S_0;

	AieRC RC = XAie_PartitionInitialize_v2(&DevInst, &Opts);
	CHECK(XAIE_OK != RC);

	/*
	 * Rollback proof: a fresh PartitionInitialize must succeed. If the
	 * wrapper had left the partition in a half-initialized state, this
	 * second init would fail or misbehave.
	 */
	RC = XAie_PartitionInitialize(&DevInst, NULL);
	CHECK_EQUAL(XAIE_OK, RC);

	RC = XAie_PartitionTeardown(&DevInst);
	CHECK_EQUAL(XAIE_OK, RC);
}

TEST(PartitionInitV2Apis, NullDevInstRejected)
{
	AieRC RC = XAie_PartitionInitialize_v2(NULL, NULL);
	CHECK_EQUAL(XAIE_INVALID_ARGS, RC);

	RC = XAie_PartitionTeardown_v2(NULL, NULL);
	CHECK_EQUAL(XAIE_INVALID_ARGS, RC);
}

#endif /* AIE_GEN == 1 || 2 || 5 */
