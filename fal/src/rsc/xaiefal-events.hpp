// Copyright(C) 2020 - 2021 by Xilinx, Inc. All rights reserved.
// SPDX-License-Identifier: MIT

#include <fstream>
#include <functional>
#include <string.h>
#include <vector>
#include <xaiengine.h>

#include <xaiefal/rsc/xaiefal-rsc-base.hpp>
#include <xaiefal/rsc/xaiefal-rscmgr.hpp>

#pragma once

namespace xaiefal {
	/**
	 * @class XAieComboEvent
	 * @brief AI engine combo event resource class
	 */
	class XAieComboEvent: public XAieSingleTileRsc {
	public:
		XAieComboEvent() = delete;
		XAieComboEvent(std::shared_ptr<XAieDevHandle> DevHd,
			XAie_LocType Loc, XAie_ModuleType Mod, uint32_t ENum = 2):
			XAieSingleTileRsc(DevHd, Loc, Mod, XAIE_COMBOEVENT) {
			if (ENum > 4 || ENum < 2) {
				throw std::invalid_argument("Combo event failed, invalid input events number");
			}
			vEvents.resize(ENum);
			State.Initialized = 1;
		}
		XAieComboEvent(XAieDev &Dev,
			XAie_LocType Loc, XAie_ModuleType Mod, uint32_t ENum = 2):
			XAieComboEvent(Dev.getDevHandle(), Loc, Mod, ENum) {}
		/**
		 * This function sets input events, and combo operations.
		 *
		 * @param vInEvents vector of input events.Minum 2 events, maximum 4 events
		 *	vInEvents[0] for Event0, vInEvents[1] for Event1,
		 *	vInEvents[2] for Event2, vInEvents[3] for Event3
		 * @param vInOps vector of combo operations
		 *	vInOps[0] for combo operation for Event0 and Event1
		 *	vInOps[1] for combo operation for Event2 and Event3
		 *	vInOps[2] for combo operation for (Event0,Event1) and
		 *		(Event2,Event3)
		 * @return XAIE_OK for success, error code for failure.
		 */
		AieRC setEvents(const std::vector<XAie_Events> &vInEvents,
				const std::vector<XAie_EventComboOps> &vInOps) {
			AieRC RC;
			if ((vInEvents.size() != vEvents.size()) || (vInOps.size() > 3) ||
				(vInEvents.size() <= 2 && vInOps.size() > 1) ||
				(vInEvents.size() > 2 && vInOps.size() < 2)) {
				Logger::log(LogLevel::ERROR) << "combo event " << __func__ << " (" <<
					(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row <<
					" Mod=" << Mod <<  " invalid number of input events and ops." << std::endl;
				RC = XAIE_INVALID_ARGS;
			} else {
				for (int i = 0; i < (int)vInEvents.size(); i++) {
					uint8_t HwEvent;

					RC = XAie_EventLogicalToPhysicalConv(dev(), Loc,
							Mod, vInEvents[i], &HwEvent);
					if (RC != XAIE_OK) {
						Logger::log(LogLevel::ERROR) << "combo event " << __func__ << " (" <<
							(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row <<
							" Mod=" << Mod <<  " invalid E=" << vInEvents[i] << std::endl;
						break;
					} else {
						vEvents[i] = vInEvents[i];
					}
				}
				if (RC == XAIE_OK) {
					vOps.clear();
					for (int i = 0; i < (int)vInOps.size(); i++) {
						vOps.push_back(vInOps[i]);
					}
					State.Configured = 1;
				}
			}
			return RC;
		}
		/**
		 * This function returns combo events for the input combination.
		 *
		 * @param vOutEvents combo events vector
		 *	vOutEvents[0] for combination of input events Event0, Event1
		 *	vOutEvents[1] for combination of input events Event2, Event3
		 *	vOutEvents[2] for combination of input events (Event0,Event1)
		 *		and (Event2,Event3)
		 * @return XAIE_OK for success, error code for failure.
		 */
		AieRC getEvents(std::vector<XAie_Events> &vOutEvents) {
			AieRC RC;

			(void)vOutEvents;
			if (State.Reserved == 0) {
				Logger::log(LogLevel::ERROR) << "combo event " << __func__ << " (" <<
					(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row <<
					" Mod=" << Mod <<  " resource is not reserved." << std::endl;
				RC = XAIE_ERR;
			} else {
				XAie_Events BaseEvent;
				XAie_EventGetComboEventBase(dev(), Loc, Mod, &BaseEvent);
				vOutEvents.clear();
				if (vOps.size() == 1) {
					vOutEvents.push_back((XAie_Events)((uint32_t)BaseEvent + vRscs[0].RscId));
				} else {
					for (uint32_t i = 0; i < (uint32_t)vOps.size(); i++) {
						vOutEvents.push_back((XAie_Events)((uint32_t)BaseEvent + i));
					}
				}
				RC = XAIE_OK;
			}
			return RC;
		}

		AieRC getInputEvents(std::vector<XAie_Events> &vOutEvents,
				std::vector<XAie_EventComboOps> &vOutOps) {
			AieRC RC;

			if (State.Configured == 1) {
				vOutEvents.clear();
				for (int i = 0; i < (int)vEvents.size(); i++) {
					vOutEvents.push_back(vEvents[i]);
				}
				vOutOps.clear();
				for (int i = 0; i < (int)vOps.size(); i++) {
					vOutOps.push_back(vOps[i]);
				}
				RC = XAIE_OK;
			} else {
				Logger::log(LogLevel::ERROR) << "combo event " << __func__ << " (" <<
					(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row <<
					" Mod=" << Mod <<  " no input events specified." << std::endl;
				RC = XAIE_ERR;
			}
			return RC;
		}
	protected:
		std::vector<XAie_Events> vEvents; /**< input events */
		std::vector<XAie_EventComboOps> vOps; /**< combo operations */
	private:
		AieRC _reserve() {
			AieRC RC;
			XAieUserRsc Rsc;

			Rsc.Loc = Loc;
			Rsc.Mod = Mod;
			Rsc.RscType = Type;
			Rsc.RscId = 0;
			for (uint32_t i = 0; i < vEvents.size(); i++) {
				vRscs.push_back(Rsc);
			}

			RC = AieHd->rscMgr()->request(*this);
			if (RC != XAIE_OK) {
				Logger::log(LogLevel::WARN) << "comboevent " << __func__ << " (" <<
					(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row << ")" <<
					" Expect Mod= " << Mod << " resource not available.\n";
				vRscs.clear();
			} else {
				if (vRscs.size() <= 2) {
					/*
					 * Only two input events, it can be combo0 or
					 * combo1
					 */
					if (vRscs[0].RscId < 2) {
						reservedId = 0;
					} else {
						reservedId = 1;
					}
				} else {
					reservedId = 2;
				}
			}
			return RC;
		}
		AieRC _release() {
			AieRC RC;

			RC = AieHd->rscMgr()->release(*this);
			vRscs.clear();
			return RC;
		}
		AieRC _start() {
			AieRC RC;
			XAie_EventComboId StartCId;

			for (uint32_t i = 0 ; i < vEvents.size(); i += 2) {
				XAie_EventComboId ComboId;

				if (vRscs[i].RscId == 0) {
					ComboId = XAIE_EVENT_COMBO0;
				} else {
					ComboId = XAIE_EVENT_COMBO1;
				}
				if (i == 0) {
					StartCId = ComboId;
				}
				RC = XAie_EventComboConfig(dev(), Loc, Mod,
						ComboId, vOps[i/2], vEvents[i], vEvents[i+1]);
				if (RC != XAIE_OK) {
					Logger::log(LogLevel::ERROR) << "combo event " << __func__ << " (" <<
						(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row <<
						" Mod=" << Mod <<  " failed to config combo " << ComboId << std::endl;
					for (XAie_EventComboId tId = StartCId;
						tId < ComboId;
						tId = (XAie_EventComboId)((int)tId + i)) {
						XAie_EventComboReset(dev(), Loc, Mod,
								tId);
					}
					break;
				}
			}
			if (RC == XAIE_OK && vOps.size() == 3) {
				RC = XAie_EventComboConfig(dev(), Loc, Mod,
					XAIE_EVENT_COMBO2, vOps[2], vEvents[0], vEvents[0]);
				if (RC != XAIE_OK) {
					Logger::log(LogLevel::ERROR) << "combo event " << __func__ << " (" <<
						(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row <<
						" Mod=" << Mod <<  " failed to config combo " << XAIE_EVENT_COMBO2 << std::endl;
				}
			}
			return RC;
		}
		AieRC _stop() {
			XAie_EventComboId ComboId;

			for (int i = 0; i < (int)vOps.size(); i++) {
				if (i == 0) {
					if (vRscs[i].RscId == 0) {
						ComboId = XAIE_EVENT_COMBO0;
					} else {
						ComboId = XAIE_EVENT_COMBO1;
					}
					XAie_EventComboReset(dev(), Loc, Mod, ComboId);
					ComboId = (XAie_EventComboId)((uint32_t)ComboId + 1);
				}
			}
			return XAIE_OK;
		}
		void _getReservedRscs(std::vector<XAieUserRsc> &vOutRscs) const {
			vOutRscs.insert(vOutRscs.end(), vRscs.begin(), vRscs.end());
		}
	};

	/**
	 * @class XAieUserEvent
	 * @brief AI engine user event resource class
	 */
	class XAieUserEvent: public XAieSingleTileRsc {
	public:
		XAieUserEvent() = delete;
		XAieUserEvent(std::shared_ptr<XAieDevHandle> DevHd,
			XAie_LocType Loc, XAie_ModuleType Mod):
			XAieSingleTileRsc(DevHd, Loc, Mod, XAIE_USEREVENT) {
			State.Initialized = 1;
			State.Configured = 1;
		}
		XAieUserEvent(XAieDev &Dev,
			XAie_LocType Loc, XAie_ModuleType Mod):
			XAieUserEvent(Dev.getDevHandle(), Loc, Mod) {}

		/**
		 * This function returns user event.
		 * It needs to be called after reserve() succeeds.
		 *
		 * @param Event returns user event
		 * @return XAIE_OK for success, error code for failure
		 */
		AieRC getEvent(XAie_Events &Event) const {
			AieRC RC = XAIE_OK;

			if (State.Reserved == 0) {
				Logger::log(LogLevel::ERROR) << "User Event " << __func__ << " (" <<
					static_cast<uint32_t>(Loc.Col) << "," << static_cast<uint32_t>(Loc.Row) << ")" <<
					" resource not resesrved." << std::endl;
				RC = XAIE_INVALID_ARGS;
			} else {
				Event = _getEventFromId(vRscs[0].RscId);
			}
			return RC;
		}
	private:
		AieRC _reserve() {
			AieRC RC;
			XAieUserRsc Rsc;
			Rsc.Loc = Loc;
			Rsc.Mod = Mod;
			Rsc.RscType = Type;
			Rsc.RscId = preferredId;

			vRscs.push_back(Rsc);
			RC = AieHd->rscMgr()->request(*this);
			if (RC != XAIE_OK) {
				Logger::log(LogLevel::WARN) << "userevent " << __func__ << " (" <<
					(uint32_t)Loc.Col << "," << (uint32_t)Loc.Row << ")" <<
					" Expect Mod= " << Mod << " resource not available.\n";
				vRscs.clear();
			} else {
				reservedId = vRscs[0].RscId;
			}
			return RC;
		}
		AieRC _release() {
			AieRC RC;

			RC = AieHd->rscMgr()->release(*this);
			vRscs.clear();
			return RC;
		}
		AieRC _start() {
			// As no hardware config is required for user event
			// always succeeds.
			return XAIE_OK;
		}
		AieRC _stop() {
			// As no hardware config is required for user event
			// always succeeds.
			return XAIE_OK;
		}

		/**
		 * This function returns user event resource id from user event.
		 *
		 * @param Event user event
		 * @return user event resource id
		 */
		uint32_t _getIdFromEvent(XAie_Events Event) const {
			XAie_Events UserEventStart;
			XAie_EventGetUserEventBase(AieHd->dev(), Loc, Mod, &UserEventStart);

			return static_cast<uint32_t>(Event - UserEventStart);
		}

		/**
		 * This function returns user event from resource id.
		 *
		 * @param RscId resource ID
		 * @return user event
		 */
		XAie_Events _getEventFromId(uint32_t RscId) const {
			XAie_Events UserEventStart;
			XAie_EventGetUserEventBase(AieHd->dev(), Loc, Mod, &UserEventStart);

			return static_cast<XAie_Events>(static_cast<uint32_t>(UserEventStart) + RscId);
		}
	};
}
