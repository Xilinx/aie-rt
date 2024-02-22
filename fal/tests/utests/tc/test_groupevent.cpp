// Copyright(C) 2020 - 2021 by Xilinx, Inc. All rights reserved.
// SPDX-License-Identifier: MIT

#include "xaiefal/xaiefal.hpp"

#include "CppUTest/CommandLineTestRunner.h"
#include "CppUTest/TestHarness.h"
#include "CppUTest/TestRegistry.h"

#include "common/tc_config.h"

using namespace xaiefal;

TEST_GROUP(GroupEvent)
{
};

TEST(GroupEvent, GroupEventBasic)
{
	AieRC RC;
	uint32_t RscType;

	XAie_SetupConfig(ConfigPtr, HW_GEN, XAIE_BASE_ADDR,
			XAIE_COL_SHIFT, XAIE_ROW_SHIFT,
			XAIE_NUM_COLS, XAIE_NUM_ROWS, XAIE_SHIM_ROW,
			XAIE_MEM_TILE_ROW_START, XAIE_MEM_TILE_NUM_ROWS,
			XAIE_AIE_TILE_ROW_START, XAIE_AIE_TILE_NUM_ROWS);

	XAie_InstDeclare(DevInst, &ConfigPtr);

	RC = XAie_CfgInitialize(&(DevInst), &ConfigPtr);
	CHECK_EQUAL(RC, XAIE_OK);

	XAieDev Aie(&DevInst, true);

	CHECK_THROWS(std::invalid_argument, Aie.tile(1,3).core().groupEvent(XAIE_EVENT_TRUE_PL));

	auto GroupEventHd = Aie.tile(1,3).core().groupEvent(XAIE_EVENT_GROUP_CORE_PROGRAM_FLOW_CORE);

	//RscType = GroupEventHd->getRscType();

	auto gEvent = GroupEventHd->getEvent();
	CHECK_EQUAL(gEvent, XAIE_EVENT_GROUP_CORE_PROGRAM_FLOW_CORE);

	RC = GroupEventHd->reserve();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = GroupEventHd->start();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = GroupEventHd->stop();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = GroupEventHd->release();
	CHECK_EQUAL(RC, XAIE_OK);

	auto GroupEventHd0 = Aie.tile(1,3).core().groupEvent(XAIE_EVENT_GROUP_CORE_PROGRAM_FLOW_CORE);
	GroupEventHd0->setGroupEvents(0x80);
	CHECK_EQUAL(GroupEventHd0->getGroupEvents(), 0x80);
	RC = GroupEventHd0->reserve();
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(GroupEventHd0->getEvent(), XAIE_EVENT_GROUP_CORE_PROGRAM_FLOW_CORE);
	auto GroupEventHd1 = Aie.tile(1,3).core().groupEvent(XAIE_EVENT_GROUP_CORE_PROGRAM_FLOW_CORE);
	GroupEventHd1->setGroupEvents(0x81);
	RC = GroupEventHd1->reserve();
	CHECK_EQUAL(RC, XAIE_INVALID_ARGS);
	auto GroupEventHd2 = Aie.tile(1,3).core().groupEvent(XAIE_EVENT_GROUP_CORE_PROGRAM_FLOW_CORE);
	GroupEventHd2->setGroupEvents(0x81);
	RC = GroupEventHd2->reserve();
	CHECK_EQUAL(RC, XAIE_INVALID_ARGS);

	GroupEventHd1->setGroupEvents(0x80);
	RC = GroupEventHd1->reserve();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = GroupEventHd1->start();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = GroupEventHd0->start();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = GroupEventHd2->start();
	CHECK_EQUAL(RC, XAIE_ERR);
	RC = GroupEventHd2->stop();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = GroupEventHd0->start();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = GroupEventHd1->start();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = GroupEventHd0->stop();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = GroupEventHd1->stop();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = GroupEventHd2->release();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = GroupEventHd1->release();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = GroupEventHd0->release();
	CHECK_EQUAL(RC, XAIE_OK);

	/* test Mem mod */
	auto GroupEventMemHd = Aie.tile(1,1).mem().groupEvent(XAIE_EVENT_GROUP_DMA_ACTIVITY_MEM);
	auto gEventMem = GroupEventMemHd->getEvent();
	CHECK_EQUAL(gEventMem, XAIE_EVENT_GROUP_DMA_ACTIVITY_MEM);
	RC = GroupEventMemHd->reserve();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = GroupEventMemHd->start();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = GroupEventMemHd->stop();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = GroupEventMemHd->release();
	CHECK_EQUAL(RC, XAIE_OK);

	/* test Pl mod */
	auto GroupEventPlHd = Aie.tile(1,0).pl().groupEvent(XAIE_EVENT_GROUP_STREAM_SWITCH_PL);
	auto gEventPl = GroupEventPlHd->getEvent();
	CHECK_EQUAL(gEventPl, XAIE_EVENT_GROUP_STREAM_SWITCH_PL);
	RC = GroupEventPlHd->reserve();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = GroupEventPlHd->start();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = GroupEventPlHd->stop();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = GroupEventPlHd->release();
	CHECK_EQUAL(RC, XAIE_OK);
}

TEST(GroupEvent, Invalid)
{
	AieRC RC;
	XAie_SetupConfig(ConfigPtr, HW_GEN, XAIE_BASE_ADDR,
			XAIE_COL_SHIFT, XAIE_ROW_SHIFT,
			XAIE_NUM_COLS, XAIE_NUM_ROWS, XAIE_SHIM_ROW,
			XAIE_MEM_TILE_ROW_START, XAIE_MEM_TILE_NUM_ROWS,
			XAIE_AIE_TILE_ROW_START, XAIE_AIE_TILE_NUM_ROWS);

	XAie_InstDeclare(DevInst, &ConfigPtr);

	RC = XAie_CfgInitialize(&(DevInst), &ConfigPtr);
	CHECK_EQUAL(RC, XAIE_OK);

	XAieDev Aie(&DevInst, true);
	auto GroupEvent = std::make_shared<XAieGroupEvent>(Aie.getDevHandle(),
			XAie_TileLoc(1,3), XAIE_CORE_MOD,
			XAIE_EVENT_GROUP_CORE_PROGRAM_FLOW_CORE);

	XAieGroupEventHandle GroupEventHd(Aie.getDevHandle(), GroupEvent);
	RC = GroupEvent->attachHandle(nullptr, 0xFF);
	CHECK_EQUAL(RC, XAIE_INVALID_ARGS);
	RC = GroupEventHd.reserve();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = GroupEvent->removeHandle(nullptr);
	CHECK_EQUAL(RC, XAIE_INVALID_ARGS);
	RC = GroupEvent->startHandle(nullptr);
	CHECK_EQUAL(RC, XAIE_INVALID_ARGS);
	RC = GroupEventHd.start();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = GroupEvent->startHandle(nullptr);
	CHECK_EQUAL(RC, XAIE_INVALID_ARGS);
}

#if AIE_GEN != 1
TEST(GroupEvent, GroupEventBasicMemTile)
{
	AieRC RC;
	uint32_t RscType;

	XAie_SetupConfig(ConfigPtr, HW_GEN, XAIE_BASE_ADDR,
			XAIE_COL_SHIFT, XAIE_ROW_SHIFT,
			XAIE_NUM_COLS, XAIE_NUM_ROWS, XAIE_SHIM_ROW,
			XAIE_MEM_TILE_ROW_START, XAIE_MEM_TILE_NUM_ROWS,
			XAIE_AIE_TILE_ROW_START, XAIE_AIE_TILE_NUM_ROWS);

	XAie_InstDeclare(DevInst, &ConfigPtr);

	RC = XAie_CfgInitialize(&(DevInst), &ConfigPtr);
	CHECK_EQUAL(RC, XAIE_OK);

	XAieDev Aie(&DevInst, true);

	CHECK_THROWS(std::invalid_argument, Aie.tile(1,1).mem().groupEvent(XAIE_EVENT_TRUE_MEM_TILE));

	auto GroupEventHd = Aie.tile(1,1).mem().groupEvent(XAIE_EVENT_GROUP_DMA_ACTIVITY_MEM_TILE);


	auto gEvent = GroupEventHd->getEvent();
	CHECK_EQUAL(gEvent, XAIE_EVENT_GROUP_DMA_ACTIVITY_MEM_TILE);

	DevInst.IsReady = 0;
	RC = GroupEventHd->reserve();
	CHECK_EQUAL(RC, XAIE_INVALID_ARGS);

	DevInst.IsReady = 1;
	RC = GroupEventHd->reserve();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = GroupEventHd->start();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = GroupEventHd->stop();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = GroupEventHd->release();
	CHECK_EQUAL(RC, XAIE_OK);

	auto GroupEventHd0 = Aie.tile(1,1).mem().groupEvent(XAIE_EVENT_GROUP_DMA_ACTIVITY_MEM_TILE);
	GroupEventHd0->setGroupEvents(0x80);
	CHECK_EQUAL(GroupEventHd0->getGroupEvents(), 0x80);
	RC = GroupEventHd0->reserve();
	CHECK_EQUAL(RC, XAIE_OK);
	CHECK_EQUAL(GroupEventHd0->getEvent(), XAIE_EVENT_GROUP_DMA_ACTIVITY_MEM_TILE);
	auto GroupEventHd1 = Aie.tile(1,1).mem().groupEvent(XAIE_EVENT_GROUP_DMA_ACTIVITY_MEM_TILE);
	GroupEventHd1->setGroupEvents(0x81);
	RC = GroupEventHd1->reserve();
	CHECK_EQUAL(RC, XAIE_INVALID_ARGS);
	auto GroupEventHd2 = Aie.tile(1,1).mem().groupEvent(XAIE_EVENT_GROUP_DMA_ACTIVITY_MEM_TILE);
	GroupEventHd2->setGroupEvents(0x81);
	RC = GroupEventHd2->reserve();
	CHECK_EQUAL(RC, XAIE_INVALID_ARGS);

	GroupEventHd1->setGroupEvents(0x80);
	RC = GroupEventHd1->reserve();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = GroupEventHd1->start();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = GroupEventHd0->start();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = GroupEventHd2->start();
	CHECK_EQUAL(RC, XAIE_ERR);
	RC = GroupEventHd2->stop();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = GroupEventHd0->start();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = GroupEventHd1->start();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = GroupEventHd0->stop();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = GroupEventHd1->stop();
	CHECK_EQUAL(RC, XAIE_OK);

	RC = GroupEventHd2->release();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = GroupEventHd1->release();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = GroupEventHd0->release();
	CHECK_EQUAL(RC, XAIE_OK);
}

TEST(GroupEvent, InvalidMemTile)
{
	AieRC RC;
	XAie_SetupConfig(ConfigPtr, HW_GEN, XAIE_BASE_ADDR,
			XAIE_COL_SHIFT, XAIE_ROW_SHIFT,
			XAIE_NUM_COLS, XAIE_NUM_ROWS, XAIE_SHIM_ROW,
			XAIE_MEM_TILE_ROW_START, XAIE_MEM_TILE_NUM_ROWS,
			XAIE_AIE_TILE_ROW_START, XAIE_AIE_TILE_NUM_ROWS);

	XAie_InstDeclare(DevInst, &ConfigPtr);

	RC = XAie_CfgInitialize(&(DevInst), &ConfigPtr);
	CHECK_EQUAL(RC, XAIE_OK);

	XAieDev Aie(&DevInst, true);
	auto GroupEvent = std::make_shared<XAieGroupEvent>(Aie.getDevHandle(),
			XAie_TileLoc(1,1), XAIE_MEM_MOD,
			XAIE_EVENT_GROUP_DMA_ACTIVITY_MEM_TILE);

	XAieGroupEventHandle GroupEventHd(Aie.getDevHandle(), GroupEvent);
	RC = GroupEvent->attachHandle(nullptr, 0xFF);
	CHECK_EQUAL(RC, XAIE_INVALID_ARGS);
	RC = GroupEventHd.reserve();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = GroupEvent->removeHandle(nullptr);
	CHECK_EQUAL(RC, XAIE_INVALID_ARGS);
	RC = GroupEvent->startHandle(nullptr);
	CHECK_EQUAL(RC, XAIE_INVALID_ARGS);
	RC = GroupEventHd.start();
	CHECK_EQUAL(RC, XAIE_OK);
	RC = GroupEvent->startHandle(nullptr);
	CHECK_EQUAL(RC, XAIE_INVALID_ARGS);
}
#endif
