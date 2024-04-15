// Copyright(C) 2020 - 2021 by Xilinx, Inc. All rights reserved.
// SPDX-License-Identifier: MIT

#include <fstream>
#include <functional>
#include <string.h>
#include <vector>
#include <xaiengine.h>

#include <xaiefal/rsc/xaiefal-rsc-base.hpp>

#pragma once

namespace xaiefal {
	/**
	 * @class XAieBroadcast
	 * @brief class for broadcast channel resource.
	 * A broadcast channel resource represents a broadcast channel between
	 * specified tiles.
	 *
	 * There are three different ways tiles can be broadcasted:
	 *
	 * - Broadcast within a tile if a single tile is specified.
	 *
	 * - Broadcast to the whole partition if no tiles are specified.
	 *
	 * - Broadcast to the tiles on the contiguous path of tiles specified by user.
	 */
	class XAieBroadcast: public XAieRsc {
	public:
		XAieBroadcast() = delete;
		XAieBroadcast(std::shared_ptr<XAieDevHandle> DevHd,
			const std::vector<XAie_LocType> &vInLocs,
			XAie_ModuleType StartM, XAie_ModuleType EndM):
			XAieRsc(DevHd, XAIE_BROADCAST) {
			StartMod = StartM;
			EndMod = EndM;
			vLocs = vInLocs;

			State.Initialized = 1;
			State.Configured = 1;
		}
		XAieBroadcast(XAieDev &Dev,
			const std::vector<XAie_LocType> &vInLocs,
			XAie_ModuleType StartM, XAie_ModuleType EndM):
			XAieBroadcast(Dev.getDevHandle(), vInLocs, StartM, EndM) {}
		~XAieBroadcast() {}
		/**
		 * This function returns the broadcast channel ID
		 *
		 * @return broadcast channel ID if resource has been reserved, -1 otherwise.
		 */
		int getBc() {
			int BC = XAIE_RSC_ID_ANY;

			if (State.Reserved == 1) {
				BC = vRscs[0].RscId;
			}
			return BC;
		}
		/**
		 * This function gets the broadcast event of the specified tile
		 * module on the broadcast channel.
		 *
		 * @param Loc tile location
		 * @param Mod module type
		 * @param Event for returning broadcast event
		 * @return XAIE_OK for success, error code for failure
		 */
		AieRC getEvent(XAie_LocType Loc, XAie_ModuleType Mod,
			       XAie_Events &Event) {
			AieRC RC = XAIE_INVALID_ARGS;

			if (State.Reserved == 0) {
				Logger::log(LogLevel::ERROR) << "broadcast object " << __func__ << " (" <<
					static_cast<uint32_t>(Loc.Col) << "," << static_cast<uint32_t>(Loc.Row) << ")" <<
					" Mod= " << Mod <<
					" resource not reserved." << std::endl;
				RC = XAIE_ERR;
			} else {
				uint8_t TileType = _XAie_GetTileTypefromLoc(AieHd->dev(), Loc);
				for (int i = 0; i < (int)vLocs.size(); i++) {
					if (Loc.Col == vLocs[i].Col && Loc.Row == vLocs[i].Row) {
						RC = XAIE_OK;
						if (TileType == XAIEGBL_TILE_TYPE_AIETILE) {
							if (Mod == XAIE_MEM_MOD) {
								Event = XAIE_EVENT_BROADCAST_0_MEM;
							} else {
								Event = XAIE_EVENT_BROADCAST_0_CORE;
							}
						} else if (TileType == XAIEGBL_TILE_TYPE_MEMTILE) {
							Event = XAIE_EVENT_BROADCAST_0_MEM_TILE;
						} else {
							Event = XAIE_EVENT_BROADCAST_A_0_PL;
						}
						Event = static_cast<XAie_Events>(static_cast<uint32_t>(Event) +
										 vRscs[0].RscId);
						break;
					}
				}
			}
			return RC;
		}
		/**
		 * This function returns broadcast channel information
		 *
		 * @param vL returns vector of tiles of the channel
		 * @param StartM returns start module
		 * @param EndM returns end module
		 * @return XAIE_OK
		 */
		AieRC getChannel(std::vector<XAie_LocType> &vL,
				 XAie_ModuleType &StartM,
				 XAie_ModuleType &EndM) {
			vL = vLocs;
			StartM = StartMod;
			EndM = EndMod;

			return XAIE_OK;
		}
		XAieRscStat getRscStat(const std::string &GName) const {
			XAieRscStat RscStat(GName);
			(void) GName;

			if (preferredId == XAIE_RSC_ID_ANY) {
				return AieHd->getRscGroup(GName).getRscStat(vLocs,
						getRscType());
			} else {
				return AieHd->getRscGroup(GName).getRscStat(vLocs,
						getRscType(), preferredId);
			}
		}
	private:
		AieRC _reserve() {
			AieRC RC = XAIE_OK;

			if (vLocs.size() != 0)  {
				RC = setRscs(AieHd, vLocs, StartMod, EndMod, vRscs);
			}

			if (RC == XAIE_OK) {
				RC = AieHd->rscMgr()->request(*this);
			}

			if (RC != XAIE_OK) {
				Logger::log(LogLevel::WARN) << "broadcast " <<
					__func__ << " resource not available.\n";
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
			AieRC RC = XAIE_OK;
			int i = 0;
			uint32_t TType;

			for(auto r = vRscs.begin(); r != vRscs.end();) {
				uint8_t un_block = 0;
				uint8_t un_block_n = 0;

				TType = _XAie_GetTileTypefromLoc(dev(), (*r).Loc);
				if (TType == XAIEGBL_TILE_TYPE_AIETILE) {
					if (r != vRscs.begin()) {
						if  ((*(r - 1)).Loc.Col < ((*r).Loc.Col ))  {
							un_block |= XAIE_EVENT_BROADCAST_WEST;
						} else if  ((*(r - 1)).Loc.Col > ((*r).Loc.Col))  {
							un_block |= XAIE_EVENT_BROADCAST_EAST;
						} else if ((*(r - 1)).Loc.Row > ((*r).Loc.Row))  {
							un_block |= XAIE_EVENT_BROADCAST_NORTH;
						} else {
							un_block |= XAIE_EVENT_BROADCAST_SOUTH;
						}
					}

					if ((r + 2) != vRscs.end()) {
						if  ((*(r + 2)).Loc.Col < ((*r).Loc.Col ))  {
							un_block |= XAIE_EVENT_BROADCAST_WEST;
						} else if  ((*(r + 2)).Loc.Col > ((*r).Loc.Col))  {
							un_block |= XAIE_EVENT_BROADCAST_EAST;
						} else if ((*(r + 2)).Loc.Row > ((*r).Loc.Row))  {
							un_block |= XAIE_EVENT_BROADCAST_NORTH;
						} else {
							un_block |= XAIE_EVENT_BROADCAST_SOUTH;
						}
					}
					un_block_n = un_block;
					u8 IsCheckerBoard;
					XAie_IsDeviceCheckerboard(dev(), &IsCheckerBoard);
					if (IsCheckerBoard) {
						/* All broadcast core tiles begin with reserving core module first */
						if ((*r).Loc.Row % 2) {
							un_block |= XAIE_EVENT_BROADCAST_EAST;
							un_block_n |= XAIE_EVENT_BROADCAST_WEST;
						} else {
							un_block |= XAIE_EVENT_BROADCAST_WEST;
							un_block_n |= XAIE_EVENT_BROADCAST_EAST;
						}

					} else {
						un_block |= XAIE_EVENT_BROADCAST_EAST;
						un_block_n |= XAIE_EVENT_BROADCAST_WEST;
					}

					RC = XAie_EventBroadcastUnblockDir(dev(),
									(*r).Loc, (*r).Mod,
									XAIE_EVENT_SWITCH_A,
									(*r).RscId,
									XAIE_EVENT_BROADCAST_ALL);
					RC = XAie_EventBroadcastUnblockDir(dev(),
									(*(r+1)).Loc, (*(r + 1)).Mod,
									XAIE_EVENT_SWITCH_A,
									(*(r + 1)).RscId,
									XAIE_EVENT_BROADCAST_ALL);
					RC = XAie_EventBroadcastBlockDir(dev(),
									(*r).Loc, (*r).Mod,
									XAIE_EVENT_SWITCH_A,
									(*r).RscId,
									XAIE_EVENT_BROADCAST_ALL & (~un_block));
					RC = XAie_EventBroadcastBlockDir(dev(),
									(*(r + 1)).Loc, (*(r+1)).Mod,
									XAIE_EVENT_SWITCH_A,
									(*(r + 1)).RscId,
									XAIE_EVENT_BROADCAST_ALL & (~un_block_n));

					r += 2;
				} else {
					if (r != vRscs.begin()) {
						if  ((*(r - 1)).Loc.Col < ((*r).Loc.Col ))  {
							un_block = XAIE_EVENT_BROADCAST_WEST;
						} else if  ((*(r - 1)).Loc.Col > ((*r).Loc.Col))  {
							un_block = XAIE_EVENT_BROADCAST_EAST;
						} else {
							un_block = XAIE_EVENT_BROADCAST_NORTH;
						}
					}
					if ((r + 1) != vRscs.end()) {
						if  ((*(r + 1)).Loc.Col < ((*r).Loc.Col ))  {
							un_block |= XAIE_EVENT_BROADCAST_WEST;
						} else if  ((*(r + 1)).Loc.Col > ((*r).Loc.Col))  {
							un_block |= XAIE_EVENT_BROADCAST_EAST;
						} else {
							un_block |= XAIE_EVENT_BROADCAST_NORTH;
						}
					}
					RC = XAie_EventBroadcastUnblockDir(dev(),
						(*r).Loc, (*r).Mod,
						XAIE_EVENT_SWITCH_A,
						(*r).RscId,
						XAIE_EVENT_BROADCAST_ALL);
					RC = XAie_EventBroadcastUnblockDir(dev(),
						(*r).Loc, (*r).Mod,
						XAIE_EVENT_SWITCH_B,
						(*r).RscId,
						XAIE_EVENT_BROADCAST_ALL);
					RC = XAie_EventBroadcastBlockDir(dev(),
						(*r).Loc, (*r).Mod,
						XAIE_EVENT_SWITCH_A,
						(*r).RscId,
						XAIE_EVENT_BROADCAST_ALL & (~(un_block | XAIE_EVENT_BROADCAST_EAST)));

					RC = XAie_EventBroadcastBlockDir(dev(),
						(*r).Loc, (*r).Mod,
						XAIE_EVENT_SWITCH_B,
						(*r).RscId,
						XAIE_EVENT_BROADCAST_ALL & (~(un_block | XAIE_EVENT_BROADCAST_WEST)));

					r++;
				}
				if (RC != XAIE_OK) {
					break;
				}
				i++;
			}
			if (RC != XAIE_OK) {
				Logger::log(LogLevel::ERROR) << "BC: " <<
					" failed to stop." << std::endl;
			}
			return RC;
		}

		AieRC _stop() {
			AieRC RC;
			int iRC = XAIE_OK;

			if(vLocs.size() == 0) {
				return XAIE_OK;
			}

			for (auto r: vRscs) {
				if (r.Loc.Row == 0) {
					// Unblock SHIM tile switch A and B
					iRC |= XAie_EventBroadcastUnblockDir(dev(),
						r.Loc, r.Mod,
						XAIE_EVENT_SWITCH_A, r.RscId,
						XAIE_EVENT_BROADCAST_ALL);
					iRC |= XAie_EventBroadcastUnblockDir(dev(),
						r.Loc, r.Mod,
						XAIE_EVENT_SWITCH_B, r.RscId,
						XAIE_EVENT_BROADCAST_ALL);
				} else {
					iRC |= XAie_EventBroadcastUnblockDir(dev(),
						r.Loc, r.Mod,
						XAIE_EVENT_SWITCH_A, r.RscId,
						XAIE_EVENT_BROADCAST_ALL);
				}
			}
			if (iRC != (int)XAIE_OK) {
				Logger::log(LogLevel::ERROR) << "BC: " <<
					" failed to stop." << std::endl;
				RC = XAIE_ERR;
			} else {
				RC = XAIE_OK;
			}
			return RC;
		}

		void _getReservedRscs(std::vector<XAieUserRsc> &vOutRscs) const {
			vOutRscs.insert(vOutRscs.end(), vRscs.begin(), vRscs.end());
		}
	private:
		XAie_ModuleType StartMod; /**< module type of the starting module on the channel */
		XAie_ModuleType EndMod; /**< module type of the ending modile on the channel */
		std::vector<XAie_LocType> vLocs; /**< tiles on the channel */
	private:
		static AieRC setRscs(std::shared_ptr<XAieDevHandle> Dev,
				const std::vector<XAie_LocType> &vL,
				XAie_ModuleType startM, XAie_ModuleType endM,
				std::vector<XAieUserRsc> &vR) {

			if ((vL[0].Row == 0 && startM != XAIE_PL_MOD) ||
			    (vL.back().Row == 0 && endM != XAIE_PL_MOD) ||
			    (vL[0].Row != 0 && startM == XAIE_PL_MOD) ||
			    (vL.back().Row != 0 && endM == XAIE_PL_MOD)) {
				Logger::log(LogLevel::ERROR) << __func__ <<
					"BC: invalid tiles and modules combination." << std::endl;
				return XAIE_INVALID_ARGS;
			}

			for (size_t i = 0; i < vL.size(); i++) {
				XAieUserRsc R;
				uint32_t TType;

				R.RscType = XAIE_BROADCAST;

				TType = _XAie_GetTileTypefromLoc(Dev->dev(),
						vL[i]);
				if (TType == XAIEGBL_TILE_TYPE_MAX) {
					Logger::log(LogLevel::ERROR) << __func__ <<
						"BC: invalid tile (" << vL[i].Col <<
						"," << vL[i].Row <<")" <<
						std::endl;
					return XAIE_INVALID_ARGS;
				}

				R.Loc.Col = vL[i].Col;
				R.Loc.Row = vL[i].Row;

				if (TType == XAIEGBL_TILE_TYPE_AIETILE) {
					R.Mod = XAIE_CORE_MOD;
					vR.push_back(R);
					R.Mod = XAIE_MEM_MOD;
					vR.push_back(R);
				} else if (TType == XAIEGBL_TILE_TYPE_MEMTILE) {
					R.Mod = XAIE_MEM_MOD;
					vR.push_back(R);
				} else {
					R.Mod = XAIE_PL_MOD;
					vR.push_back(R);
				}

				if (i != vL.size() - 1) {
					if (vL[i+1].Row == vL[i].Row &&
						vL[i+1].Col == vL[i].Col) {
						Logger::log(LogLevel::ERROR) << __func__ <<
							"BC: duplicated tiles in the vector." <<
							std::endl;
						return XAIE_INVALID_ARGS;
					}
					if (vL[i+1].Row != vL[i].Row) {
						if ((vL[i+1].Col != vL[i].Col) ||
							(vL[i+1].Row > vL[i].Row &&
							 (vL[i+1].Row - vL[i].Row) > 1) ||
							(vL[i+1].Row < vL[i].Row &&
							 (vL[i].Row - vL[i+1].Row) > 1)) {
							Logger::log(LogLevel::ERROR) << __func__ <<
								"BC: discontinuous input tiles." <<
								std::endl;
							return XAIE_INVALID_ARGS;
						}
					} else {
						if ((vL[i+1].Col > vL[i].Col &&
						     (vL[i+1].Col - vL[i].Col) > 1) ||
						    (vL[i+1].Col < vL[i].Col &&
						     (vL[i].Col - vL[i+1].Col) > 1)) {
							Logger::log(LogLevel::ERROR) << __func__ <<
								"BC: discontinuous input tiles." <<
								std::endl;
							return XAIE_INVALID_ARGS;
						}
					}
				}
			}
			return XAIE_OK;
		}
	};
}
