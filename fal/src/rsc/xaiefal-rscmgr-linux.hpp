#pragma once

#include <memory>
#include <vector>
#include <xaiengine.h>
#include <xaiefal/common/xaiefal-common.hpp>
#include <xaiefal/common/xaiefal-log.hpp>
#include <xaiefal/rsc/xaiefal-rscmgr-backend-base.hpp>

#ifdef FAL_LINUX
#include <xaiengine/xlnx-ai-engine.h>
#include <sys/ioctl.h>
namespace xaiefal {
	/**
	 * @class XAieRscMgrLinux
	 * @brief Resource Manager Backend Linux Class
	 */
	class XAieRscMgrLinux : public XAieRscMgrBackend {
	public:
		XAieRscMgrLinux() = delete;
		XAieRscMgrLinux(XAieDevHandle *DevHd):
			XAieRscMgrBackend(DevHd) {
			if (DevHd->dev()->Backend->Type != XAIE_IO_BACKEND_LINUX)
				throw std::invalid_argument("aie rscmgr: Driver not compiled with Linux Backend");
			PartitionFd = DevHd->dev()->Backend->Ops.GetPartFd((void *)DevHd->dev()->IOInst);
		}

		/**
		 * This function checks for resource availibility and will
		 * reserve the resources if available
		 *
		 * @param reference to a resource
		 *
		 * @return XAIE_OK for success, error code for failure
		 */
		AieRC request(XAieUserRsc& RscReq) {
			struct aie_rsc_req_rsp Req;
			struct aie_rsc RscRet;
			int Ret;

			Req.req.num_rscs = 1;
			Req.req.flag = 0U;
			Req.req.loc.col = RscReq.Loc.Col;
			Req.req.loc.row = RscReq.Loc.Row;
			Req.req.mod = static_cast<uint32_t>(RscReq.Mod);
			Req.req.type = static_cast<uint32_t>(RscReq.RscType);
			Req.rscs = (__u64)(uintptr_t)&RscRet;

			Ret = ioctl(PartitionFd, AIE_RSC_REQ_IOCTL, &Req);
			if (Ret != 0) {
				return XAIE_ERR;
			}
			RscReq.RscId = RscRet.id;
			return XAIE_OK;
		}

		/**
		 * This function checks for resource availibility of a specific
		 * resource and will reserve the resources if available.
		 *
		 * @param reference to a specific resource
		 *
		 * @return XAIE_OK for success, error code for failure
		 */
		AieRC requestAllocated(XAieUserRsc& RscReq) {
			int Ret;

			struct aie_rsc Rsc = {
				.loc = {
					.row = static_cast<uint8_t>(RscReq.Loc.Row),
					.col = static_cast<uint8_t>(RscReq.Loc.Col),
				},
				.mod = static_cast<uint32_t>(RscReq.Mod),
				.type = static_cast<uint32_t>(RscReq.RscType),
				.id = RscReq.RscId,
			};
			Ret = ioctl(PartitionFd, AIE_RSC_REQ_SPECIFIC_IOCTL, &Rsc);
			if (Ret != 0) {
				return XAIE_ERR;
			}
			return XAIE_OK;
		}

		/**
		 * This function checks for resource availibility and will
		 * reserve the amount of resources needed contiguously
		 *
		 * @param reference to list of resource
		 *
		 * @return XAIE_OK for success, error code for failure
		 */
		AieRC requestContiguous(std::vector<XAieUserRsc>& RscReq) {
			struct aie_rsc_req_rsp Req;
			struct aie_rsc RscRet[RscReq.size()];
			int Ret;

			Req.req.num_rscs = static_cast<uint32_t>(RscReq.size());
			Req.req.loc.col = RscReq[0].Loc.Col;
			Req.req.loc.row = RscReq[0].Loc.Row;
			Req.req.mod = static_cast<uint32_t>(RscReq[0].Mod);
			Req.req.type = static_cast<uint32_t>(RscReq[0].RscType);
			Req.req.flag = XAIE_RSC_MGR_CONTIG_FLAG;
			Req.rscs = (__u64)(uintptr_t)&RscRet;

			Ret = ioctl(PartitionFd, AIE_RSC_REQ_IOCTL, &Req);
			if (Ret != 0) {
				return XAIE_ERR;
			}
			for (unsigned int i = 0; i < RscReq.size(); i++)
				RscReq[i].RscId = RscRet[i].id;
			return XAIE_OK;
		}

		/**
		 * This function checks for resource availibility of a
		 * broadcast resource and will reserve the resources if
		 * available.
		 *
		 * @param reference to list of resource
		 *
		 * @return XAIE_OK for success, error code for failure
		 */
		AieRC requestBc(std::vector<XAieUserRsc>& RscReq, bool isBcAll) {
			struct aie_rsc RscRet[RscReq.size()];
			struct aie_rsc_bc_req Req;
			int Ret;

			Req.num_rscs = static_cast<uint32_t>(RscReq.size());
			Req.flag = 0;
			Req.id = RscReq[0].RscId;

			if (isBcAll) {
				Req.flag = XAIE_BROADCAST_ALL;
			} else {
				for (unsigned int i = 0; i < RscReq.size(); i++) {
					RscRet[i].loc.col = static_cast<uint8_t>(RscReq[i].Loc.Col);
					RscRet[i].loc.row = static_cast<uint8_t>(RscReq[i].Loc.Row);
					RscRet[i].mod = static_cast<uint32_t>(RscReq[i].Mod);
				}
			}
			Req.rscs = (__u64)(uintptr_t)&RscRet;

			if (RscReq[0].RscId == XAIE_RSC_ID_ANY) {
				Req.id = XAIE_BROADCAST_ID_ANY;
			}

			Ret = ioctl(PartitionFd, AIE_RSC_GET_COMMON_BROADCAST_IOCTL, &Req);
			if (Ret != 0) {
				return XAIE_ERR;
			}
			RscReq.resize(Req.num_rscs);
			for (unsigned int i = 0; i < RscReq.size(); i++) {
				RscReq[i].Loc.Col = RscRet[i].loc.col;
				RscReq[i].Loc.Row = RscRet[i].loc.row;
				RscReq[i].Mod = static_cast<XAie_ModuleType>(RscRet[i].mod);
				RscReq[i].RscId = RscRet[i].id;
				RscReq[i].RscType = XAIE_BROADCAST;
			}
			return XAIE_OK;
		}

		/**
		 * This function releases the resource from runtime and static
		 * bitmaps.
		 *
		 * @param reference to resource
		 *
		 * @return XAIE_OK for success, error code for failure
		 */
		AieRC release(XAieUserRsc& RscRel) {
			struct aie_rsc Rsc = {
				.loc = {
					.row = static_cast<uint8_t>(RscRel.Loc.Row),
					.col = static_cast<uint8_t>(RscRel.Loc.Col),
				},
				.mod = static_cast<uint32_t>(RscRel.Mod),
				.type = static_cast<uint32_t>(RscRel.RscType),
				.id = RscRel.RscId,
			};

			ioctl(PartitionFd, AIE_RSC_RELEASE_IOCTL, &Rsc);
			return XAIE_OK;
		}

		/**
		 * This function frees a resource from just the
		 * runtime bitmap.
		 *
		 * @param reference to resource
		 *
		 * @return XAIE_OK for success, error code for failure
		 */
		AieRC free(XAieUserRsc& RscFree) {
			struct aie_rsc Rsc = {
				.loc = {
					.row = static_cast<uint8_t>(RscFree.Loc.Row),
					.col = static_cast<uint8_t>(RscFree.Loc.Col),
				},
				.mod = static_cast<uint32_t>(RscFree.Mod),
				.type = static_cast<uint32_t>(RscFree.RscType),
				.id = RscFree.RscId,
			};

			ioctl(PartitionFd, AIE_RSC_FREE_IOCTL, &Rsc);
			return XAIE_OK;
		}

		/**
		 * This function requests resource statistics for
		 * static or available resources
		 *
		 * @return XAIE_OK for success, error code for failure
		 */
		AieRC getRscStats(std::vector<XAieUserRscStat> &vStats,
				XAieRscBitmapType BType) {
			struct aie_rsc_user_stat RStats[vStats.size()];
			struct aie_rsc_user_stat_array RStatArray;
			int Ret;

			RStatArray.num_stats = vStats.size();
			RStatArray.stats = (__u64)(uintptr_t)&RStats;
			if (BType == XAIE_STATIC_RSC)
				RStatArray.stats_type = AIE_RSC_STAT_TYPE_STATIC;
			else if (BType ==  XAIE_AVAIL_RSC)
				RStatArray.stats_type = AIE_RSC_STAT_TYPE_AVAIL;

			for (uint32_t i = 0; i < vStats.size(); i++) {
				RStats[i].loc.col = vStats[i].Loc.Col;
				RStats[i].loc.row = vStats[i].Loc.Row;
				RStats[i].mod = static_cast<uint8_t>(vStats[i].Mod);
				RStats[i].type = static_cast<uint8_t>(vStats[i].RscType);
			}

			Ret = ioctl(PartitionFd, AIE_RSC_GET_STAT_IOCTL, &RStatArray);
			if (Ret != 0)
				return XAIE_ERR;

			for (uint32_t i = 0; i < vStats.size(); i++)
				vStats[i].NumRscs = RStats[i].num_rscs;

			return XAIE_OK;
		}

	private:
		int PartitionFd;
	}; /* class XAieRscMgrLinux */
#endif /* FAL_LINUX */
} /* namespace xaiefal */
