// Copyright(C) 2020 - 2021 by Xilinx, Inc. All rights reserved.
// SPDX-License-Identifier: MIT

//#pragma warning (disable : 4099 4101 4146 /*unary minus*/ 4244 /*int to u8*/ 4267 4389 4457 4458)
#ifdef _WIN32
#pragma warning (disable : 4146 4244 )
#endif

#include <fstream>
#include <functional>
#include <vector>
#include <xaiengine.h>
#include <xaiefal/common/xaiefal-base.hpp>
#include <xaiefal/common/xaiefal-common.hpp>
#include <xaiefal/common/xaiefal-log.hpp>
#include <xaiefal/profile/xaiefal-profile.hpp>
#include <xaiefal/rsc/xaiefal-bc.hpp>
#include <xaiefal/rsc/xaiefal-events.hpp>
#include <xaiefal/rsc/xaiefal-groupevent.hpp>
#include <xaiefal/rsc/xaiefal-pc.hpp>
#include <xaiefal/rsc/xaiefal-perf.hpp>
#include <xaiefal/rsc/xaiefal-rsc-group.hpp>
#include <xaiefal/rsc/xaiefal-rsc-group-impl.hpp>
#include <xaiefal/rsc/xaiefal-ss.hpp>
#include <xaiefal/rsc/xaiefal-trace.hpp>

