/******************************************************************************
* Copyright (C) 2024 AMD, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file txn_buffer.h
* @{
*
* This file contains implementation of transaction buffer.
******************************************************************************/

#ifndef XAIE_TXN_BUFFER_H
#define XAIE_TXN_BUFFER_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <limits.h>

#define XAIE_DEFAULT_TXN_BUFFER_SIZE (1024 * 4)

#define XAIE_TXN_INSTANCE_EXPORTED	0b10U
#define XAIE_TXN_INST_EXPORTED_MASK XAIE_TXN_INSTANCE_EXPORTED
#ifdef ENABLE_UART
#   define TRACE(...)   xil_printf(__VA_ARGS__)
#else
#   define TRACE(...)
#endif

//#define WITH_ASSERT
#ifdef WITH_ASSERT
#   define assert(x)        if (UNLIKELY(!(x))) { xil_printf("Assertion failed @ line %u!\n", __LINE__); uint32_t result = 0xFF; cputfsl(result, 0); mb_sleep(); }
#else
#   define assert(x)
#endif
// Padding is different when structures are padded by gcc(submitter->host) vs mb_gcc(receiver->uC)
// To show the difference, commented structures define host side structures and uncommented
// structure definitions are for uC side.

// typedef struct {
// 	uint8_t major;
// 	uint8_t minor;
// 	uint8_t dev_gen;
// 	uint8_t num_rows;
// 	uint8_t num_cols;
// 	uint8_t num_mem_tile_rows;
// 	uint32_t num_ops;
// 	uint32_t txn_size;
// } txn_header; ---> host definition

typedef struct {
	uint32_t header1;
	uint32_t header2;
	uint32_t num_ops;
	uint32_t txn_size;
} txn_header;

typedef struct {
	uint8_t op;
	uint8_t col;
	uint8_t row;
} op_header;

// typedef struct {
// 	op_header op_hdr;
// 	uint64_t reg_off;
// 	uint32_t value;
// 	uint32_t size;
// } write32_header; ---> host definition

typedef struct {
	uint64_t op_hdr;
	uint64_t reg_off;
	uint32_t value;
	uint32_t size;
} write32_header;

// typedef struct {
// 	op_header op_hdr;
// 	uint64_t reg_off;
// 	uint32_t value;
// 	uint32_t mask;
// 	uint32_t size;
// } mask_write32_header;--->host definition

typedef struct {
	uint64_t op_hdr;
	uint64_t reg_off;
	uint32_t value;
	uint32_t mask;
	uint32_t size;
	uint32_t pad;
} mask_write32_header;

// typedef struct {
// 	op_header op_hdr;
// 	uint64_t reg_off;
// 	uint32_t value;
// 	uint32_t mask;
// 	uint32_t size;
// } mask_poll32_header;---> host definition

typedef struct {
	uint64_t op_hdr;
	uint64_t reg_off;
	uint32_t value;
	uint32_t mask;
	uint32_t size;
	uint32_t pad;
} mask_poll32_header;

// typedef struct {
// 	op_header op_hdr;
// 	uint8_t col;
// 	uint8_t row;
// 	uint32_t reg_off;
// 	uint32_t size;
// } block_write32_header;---> host definition

typedef struct {
	uint64_t header;
	uint32_t reg_off;
	uint32_t size;
} block_write32_header;

// typedef struct {
// 	op_header op_hdr;
// 	uint32_t size;
// } custom_op_header;---> No support in UC yet

typedef struct list {
	struct list *next;
} list;

typedef enum {
	XAIE_IO_WRITE,
	XAIE_IO_BLOCKWRITE,
	XAIE_IO_BLOCKSET,
	XAIE_IO_MASKWRITE,
	XAIE_IO_MASKPOLL,
	XAIE_CONFIG_SHIMDMA_BD,
	XAIE_CONFIG_SHIMDMA_DMABUF_BD,
	XAIE_IO_CUSTOM_OP_BEGIN = 1U<<7U,
	XAIE_IO_CUSTOM_OP_TCT = XAIE_IO_CUSTOM_OP_BEGIN,
	XAIE_IO_CUSTOM_OP_DDR_PATCH,
	XAIE_IO_CUSTOM_OP_NEXT,
	XAIE_IO_CUSTOM_OP_MAX = UCHAR_MAX,
} txn_opcode;

typedef struct  {
	txn_opcode opcode;
	uint32_t mask;
	uint64_t reg_offset;
	uint32_t value;
	uint64_t data_ptr;
	uint32_t size;
} txn_cmd;

// typedef struct {
// 	uint64_t tid;
// 	uint32_t flags;
// 	uint32_t num_cmds;
// 	uint32_t max_cmds;
// 	uint8_t  next_custom_op;
// 	txn_cmd *cmd_buf;
// 	list node;
// } txn_inst;

void uc_read32(uint64_t reg_offset, uint32_t *data);
void uc_write32(uint64_t reg_offset, uint32_t data);
void uc_mask_write32(uint64_t reg_offset, uint32_t mask, uint32_t data);
int uc_mask_poll(uint64_t reg_offset, uint32_t mask, uint32_t value, uint32_t timeout_us);
void  uc_block_write32(uint64_t reg_offset, const uint32_t *data, uint32_t size);
void uc_block_set32(uint64_t reg_offset, uint32_t data, uint32_t size);
int  uc_execute_txn_buffer_cmds(txn_cmd *cmd, uint32_t Flags, uint32_t *cmd_size);
int uc_load_txn_buffer(uint32_t* buf);

#endif

/** @} */