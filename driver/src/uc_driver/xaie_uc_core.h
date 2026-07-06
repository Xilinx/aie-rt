/******************************************************************************
* Copyright (C) 2024 AMD, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file uc.h
* @{
*
* This file contains implementation of core control functionality from uC.
******************************************************************************/

#ifndef XAIE_UC_CORE_H
#define XAIE_UC_CORE_H

#include <stdlib.h>
#include <string.h>
#include "xaie_helper.h"

void uc_core_enable(uint8_t row, uint8_t col);
void uc_get_core_status(uint8_t row, uint8_t col, uint32_t* core_status);
#endif