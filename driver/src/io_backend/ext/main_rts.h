/******************************************************************************
 * * Copyright (C) 2024 AMD, Inc.  All rights reserved.
 * * SPDX-License-Identifier: MIT
 * ******************************************************************************/

// 67d7842dbbe25473c3c32b93c0da8047785f30d78e8a024de1b57352245f9689
#ifndef MAIN_RTS_IO_H
#define MAIN_RTS_IO_H

#include <stdint.h>
typedef unsigned int uint;

void ess_Write32(uint64_t Addr, uint Data);
uint ess_Read32(uint64_t Addr);

void ess_Write128(uint64_t Addr, uint32_t* Data);
void ess_Read128(uint64_t Addr, uint32_t* Data);

void ess_WriteCmd(unsigned char Command, unsigned char ColId, unsigned char RowId, unsigned int CmdWd0, unsigned int CmdWd1, unsigned char *CmdStr);

void ess_NpiWrite32(uint64_t Addr, uint Data);
uint ess_NpiRead32(uint64_t Addr);

void ess_WriteGM(uint64_t addr, const void* data, uint64_t size);
void ess_ReadGM(uint64_t addr, void* data, uint64_t size);

void start_plios();

#endif


