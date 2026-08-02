/* SPDX-License-Identifier: MIT */

#ifndef CDO_RTS_H
#define CDO_RTS_H

#include <stdint.h>

void cdo_Write32(uint64_t Addr, uint32_t Data);
void cdo_MaskWrite32(uint64_t Addr, uint32_t Mask, uint32_t Data);
void cdo_MaskPoll(uint64_t Addr, uint32_t Mask, uint32_t ExpectedValue,
		uint32_t TimeoutInMs);
void cdo_BlockWrite32(uint64_t Addr, uint32_t *Data, uint32_t Size);
void cdo_BlockSet32(uint64_t Addr, uint32_t Data, uint32_t Size);

#endif /* CDO_RTS_H */
