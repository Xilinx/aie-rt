// Copyright(C) 2020 - 2021 by Xilinx, Inc. All rights reserved.
// SPDX-License-Identifier: MIT

#include <fstream>
#include <functional>
#include <map>
#include <string.h>
#include <vector>
#include <xaiengine.h>

#include <xaiefal/rsc/xaiefal-bc.hpp>
#include <xaiefal/rsc/xaiefal-rsc-base.hpp>
#include <xaiefal/rsc/xaiefal-rscmgr.hpp>

#pragma once

namespace xaiefal {
	/**
	 * @class XAiePerfCounter
	 * @brief class for Perfcounter resource.
	 * Each perfcounter resources contains the resource ID of in a module of
	 * a tile, start, stop, and reset events, and counter value for the
	 * counter event.
	 * If there is not enough perfcounter of the specified module, it will
	 * check if there is counter available from the adjacent module if it is
	 * core tile. If there is available counter in the adjacent module of
	 * the same tile, broadcast channels within the tile of the
	 * start/stop/reset events will be reserved.
	 */
	class XAiePerfCounter : public XAieSingleTileRsc {
	public:
		XAiePerfCounter() = delete;
		XAiePerfCounter(std::shared_ptr<XAieDevHandle> DevHd,
			XAie_LocType Loc, XAie_ModuleType Mod,
			bool CrossM = false, uint32_t Threshold = 0):
			XAieSingleTileRsc(DevHd, Loc, Mod, XAIE_PERFCOUNT),
			CrossMod(CrossM) {
			StartMod = Mod;
			StopMod = Mod;
			RstMod = Mod;
			RstEvent = XAIE_EVENT_NONE_CORE;
			State.Initialized = 1;
			EventVal = Threshold;
		}
		XAiePerfCounter(XAieDev &Dev,
			XAie_LocType Loc, XAie_ModuleType Mod,
			bool CrossM = false):
			XAiePerfCounter(Dev.getDevHandle(), Loc, Mod, CrossM) {}
		~XAiePerfCounter() {
			if (State.Reserved == 1) {
				if (StartMod != vRscs[0].Mod) {
					delete StartBC;
				}
				if (StopMod != vRscs[0].Mod) {
					delete StopBC;
				}
				if (RstEvent != XAIE_EVENT_NONE_CORE &&
					RstMod != vRscs[0].Mod) {
					delete RstBC;
				}
			}
		}
		/**
		 * This function sets the perfcounter start, stop, reset events.
		 *
		 * @param StartM start event module type
		 * @param StartE start event
		 * @param StopM stop event module type
		 * @param StopE stop event
		 * @param RstM reset event module type
		 * @param RstE reset event, default is XAIE_EVENT_NONE_CORE,
		 *	       that is no reset event.
		 * @return XAIE_OK for success, error code for failure
		 */
		AieRC initialize(XAie_ModuleType StartM, XAie_Events StartE,
				 XAie_ModuleType StopM, XAie_Events StopE,
				 XAie_ModuleType RstM = XAIE_CORE_MOD, XAie_Events RstE = XAIE_EVENT_NONE_CORE) {
			AieRC RC;

			if (State.Reserved == 1) {
				Logger::log(LogLevel::ERROR) << "perfcount " << __func__ << " (" <<
					(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row << ")" <<
					" Expect Mod= " << Mod << " resource reserved." << std::endl;
				return XAIE_ERR;
			} else {
				uint8_t HwEvent;

				RC = XAie_EventLogicalToPhysicalConv(dev(), Loc,
					StartM, StartE, &HwEvent);
				if (RC == XAIE_OK) {
					RC = XAie_EventLogicalToPhysicalConv(dev(), Loc,
						StopM, StopE, &HwEvent);
				}
				if (RC == XAIE_OK && RstE != XAIE_EVENT_NONE_CORE) {
					RC = XAie_EventLogicalToPhysicalConv(dev(), Loc,
						RstM, RstE, &HwEvent);
				}
				if (RC == XAIE_OK) {
					StartEvent = StartE;
					StopEvent = StopE;
					RstEvent = RstE;
					StartMod = StartM;
					StopMod = StopM;
					if (RstE != XAIE_EVENT_NONE_CORE) {
						RstMod = RstM;
					}
					State.Initialized = 1;
					State.Configured = 1;
				} else {
					Logger::log(LogLevel::ERROR) << "perfcount " << __func__ << " (" <<
						(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row << ")" <<
						" Expect Mod= " << Mod <<
						" StartEvent=" <<StartM << "," << StartE << " " <<
						" StopEvent=" <<StopM << "," << StopE << " " <<
						" RstEvent=" <<RstM << "," << RstE << " " <<
						std::endl;
				}
			}
			return RC;
		}
		/**
		 * This function gets if allow to allocate perfcounter from a
		 * different module than the expected one in the same tile.
		 *
		 * @return true if cross module is enabled, false otherwise.
		 */
		bool getCrossMod()
		{
			return CrossMod;
		}
		/**
		 * This function change the reset event.
		 * It needs to be called before counter is configured in hardware.
		 * That is it needs to be called before start().
		 *
		 * @param Module reset event module
		 * @param Event reset event
		 * @return XAIE_OK for success, error code for failure
		 */
		AieRC changeRstEvent(XAie_ModuleType Module, XAie_Events Event) {
			AieRC RC;

			if (State.Running == 1) {
				Logger::log(LogLevel::ERROR) << "perfcount " << __func__ << " (" <<
					(int)Loc.Col << "," << (int)Loc.Row << ")" <<
					" Expect Mod= " << Mod << " Actual Mod=" << (int)vRscs[0].Mod <<
					" resource is in use" << std::endl;
				RC = XAIE_ERR;
			} else if (State.Reserved == 1 && Module != RstMod) {
				Logger::log(LogLevel::ERROR) << "perfcount " << __func__ << " (" <<
					(int)Loc.Col << "," << (int)Loc.Row << ")" <<
					" Expect Mod= " << Mod << " Actual Mod=" << (int)vRscs[0].Mod <<
					" resource is reserved, event module type cannot change." <<
					std::endl;
				RC = XAIE_INVALID_ARGS;
			} else {
				uint8_t HwEvent;

				RC = XAie_EventLogicalToPhysicalConv(dev(),
						Loc, Module, Event, &HwEvent);
				if (RC == XAIE_OK) {
					RstEvent = Event;
					RstMod = Module;
				}
			}
			return RC;
		}
		/**
		 * This function change threshold to generate counter event.
		 * It needs to be called before counter is configured in hardware.
		 * That is it needs to be called before start().
		 *
		 * @param Val counter value to generate counter event
		 * @return XAIE_OK for success, error code for failure
		 */
		AieRC changeThreshold(uint32_t Val) {
			AieRC RC;

			if (State.Running == 1) {
				Logger::log(LogLevel::ERROR) << "perfcount " << __func__ << " (" <<
					(int)Loc.Col << "," << (int)Loc.Row << ")" <<
					" Expect Mod= " << Mod << " Actual Mod=" << (int)vRscs[0].Mod <<
					" resource is in use." << std::endl;
				RC = XAIE_ERR;
			} else {
				EventVal = Val;
				RC = XAIE_OK;
			}
			return RC;
		}
		/**
		 * This function change the start event.
		 * It needs to be called before counter is configured in hardware.
		 * That is it needs to be called before start().
		 *
		 * @param Module reset event module
		 * @param Event start event
		 *
		 * @return XAIE_OK for success, error code for failure
		 */
		AieRC changeStartEvent(XAie_ModuleType Module, XAie_Events Event) {
			AieRC RC;

			if (State.Running == 1) {
				Logger::log(LogLevel::ERROR) << "perfcount " << __func__ << " (" <<
					(int)Loc.Col << "," << (int)Loc.Row << ")" <<
					" Expect Mod= " << Mod << " Actual Mod=" << (int)vRscs[0].Mod <<
					" resource is in use." << std::endl;
				RC = XAIE_ERR;
			} else if (State.Reserved == 1 && Module != StartMod) {
				Logger::log(LogLevel::ERROR) << "perfcount " << __func__ << " (" <<
					(int)Loc.Col << "," << (int)Loc.Row << ")" <<
					" Expect Mod= " << Mod << " Actual Mod=" << (int)vRscs[0].Mod <<
					" resource is reserved, event module type cannot change." <<
					std::endl;
				RC = XAIE_INVALID_ARGS;
			} else {
				uint8_t HwEvent;

				RC = XAie_EventLogicalToPhysicalConv(dev(), Loc,
						Module, Event, &HwEvent);
				if (RC == XAIE_OK) {
					StartEvent = Event;
					StartMod = Module;
				}
			}
			return RC;
		}
		/**
		 * This function change the stop event.
		 * It needs to be called before counter is configured in hardware.
		 * That is it needs to be called before start().
		 *
		 * @param Module module type of the event
		 * @param Event stop event
		 * @return XAIE_OK for success, error code for failure
		 */
		AieRC changeStopEvent(XAie_ModuleType Module, XAie_Events Event) {
			AieRC RC;

			if (State.Running == 1) {
				Logger::log(LogLevel::ERROR) << "perfcount " << __func__ << " (" <<
					(int)Loc.Col << "," << (int)Loc.Row << ")" <<
					" Expect Mod= " << Mod << " Actual Mod=" << (int)vRscs[0].Mod <<
					" resource is in use." << std::endl;
				RC = XAIE_ERR;
			} else if (State.Reserved == 1 && Module != StopMod) {
				Logger::log(LogLevel::ERROR) << "perfcount " << __func__ << " (" <<
					(int)Loc.Col << "," << (int)Loc.Row << ")" <<
					" Expect Mod= " << Mod << " Actual Counter Mod=" << (int)vRscs[0].Mod <<
					" resource is reserved, event module type cannot change." <<
					std::endl;
				RC = XAIE_INVALID_ARGS;
			} else {
				uint8_t HwEvent;

				RC = XAie_EventLogicalToPhysicalConv(dev(), Loc,
						Module, Event, &HwEvent);
				if (RC == XAIE_OK) {
					StopEvent = Event;
					StopMod = Module;
				}
			}
			return RC;
		}
		/**
		 * This function reads the perfcounter value if counter is
		 * in use.
		 *
		 * @param Result counter value if counter is in use.
		 * @return XAIE_OK for success, error code for failure
		 */
		AieRC readResult(uint32_t &Result) {
			AieRC RC;

			if (State.Running == 0) {
				Logger::log(LogLevel::ERROR) << "perfcount " << __func__ << " (" <<
					(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row << ")" <<
					" Expect Mod= " << Mod <<
					" resource not in use." << std::endl;
				RC = XAIE_ERR;
			} else {
				RC = XAie_PerfCounterGet(dev(), Loc, vRscs[0].Mod,
						vRscs[0].RscId, &Result);
			}
			return RC;
		}
		/**
		 * This function returns the counter event and the event module.
		 *
		 * @param Module module of the counter
		 * @param Event counter event
		 * @return XAIE_OK for success, error code for failure
		 */
		AieRC getCounterEvent(XAie_ModuleType &Module, XAie_Events &Event) const {
			AieRC RC;

			if (State.Reserved == 0) {
				Logger::log(LogLevel::ERROR) << "perfcount " << __func__ << " (" <<
					(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row << ")" <<
					" Expect Mod= " << Mod <<
					" resource not allocated." << std::endl;
				RC = XAIE_ERR;
			} else {
				Module = vRscs[0].Mod;
				XAie_PerfCounterGetEventBase(AieHd->dev(), Loc, Module, &Event);
				Event = static_cast<XAie_Events>(static_cast<uint32_t>(Event) + vRscs[0].RscId);
				RC = XAIE_OK;
			}

			return RC;
		}
		XAieRscStat getRscStat(const std::string &GName) const {
			XAieRscStat RscStat(GName);
			(void) GName;

			if (preferredId == XAIE_RSC_ID_ANY) {
				if (CrossMod) {
					return AieHd->getRscGroup(GName).getRscStat(Loc,
						getRscType());
				} else {
					return AieHd->getRscGroup(GName).getRscStat(Loc,
						Mod, getRscType());
				}
			} else {
				return AieHd->getRscGroup(GName).getRscStat(Loc,
						Mod, getRscType(), preferredId);
			}
		}
	protected:
		XAie_Events StartEvent; /**< start event */
		XAie_Events StopEvent; /**< stop event */
		XAie_Events RstEvent; /**< reset event */
		XAie_ModuleType StartMod; /**< start event module */
		XAie_ModuleType StopMod; /**< stop event module */
		XAie_ModuleType RstMod; /**< Reset event module */
		uint32_t EventVal; /**< counter event value */
		bool CrossMod; /**< true to indicate can try allocating counter
				    from different module in the same tile */
		XAieBroadcast *StartBC; /**< start Event braodcast resource */
		XAieBroadcast *StopBC; /**< stop Event braodcast resource */
		XAieBroadcast *RstBC; /**< reset Event braodcast resource */
	private:
		AieRC _reserve() {
			AieRC RC;
			uint32_t TType = _XAie_GetTileTypefromLoc(dev(), Loc);
			XAieUserRsc Rsc;

			Rsc.Loc = Loc;
			Rsc.Mod = Mod;
			Rsc.RscType = Type;
			Rsc.RscId = preferredId;
			vRscs.push_back(Rsc);
			RC = AieHd->rscMgr()->request(*this);
			if (RC != XAIE_OK && preferredId == XAIE_RSC_ID_ANY) {
				if (TType == XAIEGBL_TILE_TYPE_AIETILE && CrossMod) {
					XAie_ModuleType lM;

					if (Mod == XAIE_CORE_MOD) {
						lM = XAIE_MEM_MOD;
					} else {
						lM = XAIE_CORE_MOD;
					}
					vRscs[0].Mod = lM;
					RC = AieHd->rscMgr()->request(*this);
				}
			}
			if (RC == XAIE_OK && TType == XAIEGBL_TILE_TYPE_AIETILE) {
				std::vector<XAie_LocType> vL;

				vL.push_back(Loc);
				if (StartMod != vRscs[0].Mod) {
					StartBC = new XAieBroadcast(AieHd, vL,
						StartMod, vRscs[0].Mod);
					RC = StartBC->reserve();
					if (RC != XAIE_OK) {
						delete StartBC;
					}
				}
				if (RC == XAIE_OK && StopMod != vRscs[0].Mod) {
					StopBC = new XAieBroadcast(AieHd, vL,
							StartMod, vRscs[0].Mod);
					RC = StopBC->reserve();
					if (RC != XAIE_OK) {
						delete StopBC;
						if (StartMod != vRscs[0].Mod) {
							StartBC->release();
							delete StartBC;
						}
					}
				}
				if (RC == XAIE_OK && RstEvent != XAIE_EVENT_NONE_CORE &&
					RstMod != vRscs[0].Mod) {
					RstBC = new XAieBroadcast(AieHd, vL,
							StartMod, vRscs[0].Mod);
					RC = RstBC->reserve();
					if (RC != XAIE_OK) {
						delete RstBC;
						if (StartMod != vRscs[0].Mod) {
							StartBC->release();
							delete StartBC;
						}
						if (StopMod != vRscs[0].Mod) {
							StopBC->release();
							delete StopBC;
						}
					}
				}
				if (RC != XAIE_OK) {
					AieHd->rscMgr()->release(*this);
					vRscs.clear();
				}
			}
			if (RC != XAIE_OK) {
				Logger::log(LogLevel::WARN) << "perfcount " << __func__ << " (" <<
					(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row << ")" <<
					" Expect Mod= " << Mod << " resource not available.\n";
				vRscs.clear();
			} else {
				RC = _reserveAppend();
			}
			return RC;
		}
		AieRC _release() {
			AieRC RC;

			if (StartMod != vRscs[0].Mod) {
				StartBC->release();
				delete StartBC;
			}
			if (StopMod != vRscs[0].Mod) {
				StopBC->release();
				delete StopBC;
			}
			if (RstEvent != XAIE_EVENT_NONE_CORE && RstMod != vRscs[0].Mod) {
				RstBC->release();
				delete RstBC;
			}
			_releaseAppend();

			RC = AieHd->rscMgr()->release(*this);
			vRscs.clear();
			return RC;
		}
		AieRC _start() {
			AieRC RC;

			RC = _startPrepend();
			if (RC == XAIE_OK) {
				XAie_Events lStartE = StartEvent;
				XAie_Events lStopE = StopEvent;
				XAie_Events lRstE = RstEvent;

				if(EventVal != 0) {
					RC = XAie_PerfCounterEventValueSet(dev(), Loc, vRscs[0].Mod,
						vRscs[0].RscId, EventVal);
				}

				if (RC == XAIE_OK && StartMod != vRscs[0].Mod) {
					StartBC->getEvent(Loc, vRscs[0].Mod, lStartE);
					RC = XAie_EventBroadcast(dev(), Loc, StartMod, StartBC->getBc(), StartEvent);
					if (RC == XAIE_OK) {
						RC = StartBC->start();
					}
				}
				if (RC == XAIE_OK && StopMod != vRscs[0].Mod) {
					StopBC->getEvent(Loc, vRscs[0].Mod, lStopE);
					XAie_EventBroadcast(dev(), Loc, StopMod, StopBC->getBc(), StopEvent);
					if (RC == XAIE_OK) {
						RC = StopBC->start();
					}
				}
				if (RC == XAIE_OK && RstEvent != XAIE_EVENT_NONE_CORE && RstMod != vRscs[0].Mod) {
					RstBC->getEvent(Loc, vRscs[0].Mod, lRstE);
					RC = XAie_EventBroadcast(dev(), Loc, RstMod, RstBC->getBc(), RstEvent);
					if (RC == XAIE_OK) {
						RC = RstBC->start();
					}
				}
				if (RC == XAIE_OK) {
					RC = XAie_PerfCounterControlSet(dev(), Loc, vRscs[0].Mod,
						vRscs[0].RscId, lStartE, lStopE);
				}
				if (RC == XAIE_OK && RstEvent != XAIE_EVENT_NONE_CORE) {
					RC = XAie_PerfCounterResetControlSet(dev(), Loc,
						vRscs[0].Mod, vRscs[0].RscId, lRstE);
				}
			}

			if (RC != XAIE_OK) {
				Logger::log(LogLevel::ERROR) << "perfcount " << __func__ << " (" <<
					(int)Loc.Col << "," << (int)Loc.Row << ")" <<
					" Expect Mod= " << Mod << " Actual Mod=" << (int)vRscs[0].Mod <<
					" failed to start." << std::endl;
			}
			return RC;
		}
		AieRC _stop() {
			AieRC RC;
			int iRC;

			iRC = (int)XAie_PerfCounterControlReset(dev(), Loc, vRscs[0].Mod, vRscs[0].RscId);
			iRC |= (int)XAie_PerfCounterResetControlReset(dev(), Loc, vRscs[0].Mod, vRscs[0].RscId);
			iRC |= (int)XAie_PerfCounterReset(dev(), Loc, vRscs[0].Mod, vRscs[0].RscId);
			iRC |= (int)XAie_PerfCounterEventValueReset(dev(), Loc, vRscs[0].Mod, vRscs[0].RscId);

			if (StartMod != vRscs[0].Mod) {
				StartBC->getEvent(Loc, vRscs[0].Mod, StartEvent);
				iRC |= XAie_EventBroadcastReset(dev(), Loc,
						StartMod, StartBC->getBc());
				iRC |= StartBC->stop();
			}
			if (StopMod != vRscs[0].Mod) {
				StopBC->getEvent(Loc, vRscs[0].Mod, StopEvent);
				iRC |= XAie_EventBroadcastReset(dev(), Loc,
						StopMod, StopBC->getBc());
				iRC |= StopBC->stop();
			}
			if (RstEvent != XAIE_EVENT_NONE_CORE && RstMod != vRscs[0].Mod) {
				RstBC->getEvent(Loc, vRscs[0].Mod, RstEvent);
				iRC |= XAie_EventBroadcastReset(dev(), Loc,
						RstMod, RstBC->getBc());
				iRC |= RstBC->stop();
			}
			if (iRC != (int)XAIE_OK) {
				Logger::log(LogLevel::ERROR) << "perfcount " << __func__ << " (" <<
					(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row << ")" <<
					" Expect Mod= " << (int)Mod << " Actual Mod=" << (int)vRscs[0].Mod <<
					" failed to stop." << std::endl;
				RC = XAIE_ERR;
			} else {
				RC = _stopAppend();
			}
			return RC;
		}

		void _getReservedRscs(std::vector<XAieUserRsc> &vOutRscs) const {
			vOutRscs.push_back(vRscs[0]);
			if (StartMod != vRscs[0].Mod) {
				StartBC->getReservedRscs(vOutRscs);
			}
			if (StopMod != vRscs[0].Mod) {
				StopBC->getReservedRscs(vOutRscs);
			}
			if (RstEvent != XAIE_EVENT_NONE_CORE && StopMod != vRscs[0].Mod) {
				RstBC->getReservedRscs(vOutRscs);
			}
			_getRscsAppend(vOutRscs);
		}
		virtual AieRC _startPrepend() {
			return XAIE_OK;
		}
		virtual AieRC _reserveAppend() {
			return XAIE_OK;
		}
		virtual AieRC _stopAppend() {
			return XAIE_OK;
		}
		virtual AieRC _releaseAppend() {
			return XAIE_OK;
		}
		virtual void _getRscsAppend(std::vector<XAieUserRsc> &vOutRscs) const {
			(void)vOutRscs;
		}
	};
}
