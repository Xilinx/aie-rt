/******************************************************************************
* Copyright (C) 2024 AMD, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file txn_buffer.c
* @{
*
* This file contains implementation of transaction buffer processing in uC.
******************************************************************************/
#include <unistd.h>

#include "xaie_txn_buffer.h"
#include "xaie_regdb_uc_module_core.h"

#include <unistd.h>

void uc_read32(uint64_t reg_offset, uint32_t *data)
{
	*data = *((uint32_t *)(AIE2PS_UC_MODULE_CORE__ARRAY_AXI_MM + reg_offset));
	return;
}

void uc_write32(uint64_t reg_offset, uint32_t data)
{

	*((uint32_t *)(AIE2PS_UC_MODULE_CORE__ARRAY_AXI_MM + reg_offset)) = data;
	return;
}

void uc_mask_write32(uint64_t reg_offset, uint32_t mask, uint32_t data)
{
	uint32_t reg_val;

	uc_read32(reg_offset, &reg_val);

	reg_val &= ~mask;
	reg_val |= data;
	uc_write32(reg_offset, reg_val);

	return;
}

int uc_mask_poll(uint64_t reg_offset, uint32_t mask, uint32_t value, uint32_t timeout_us)
{
    int ret = -1;
	uint32_t count, min_timeout_us, reg_val;

	/*
	 * Any value less than 200 us becomes noticable overhead. This is based
	 * on some profiling, and it may vary between platforms.
	 */
	min_timeout_us = 200;
	count = ((uint64_t)timeout_us + min_timeout_us - 1) / min_timeout_us;

	while (count > 0U) {
		uc_read32(reg_offset, &reg_val);
		if((reg_val & mask) == value) {
			return 0;
		}
		usleep(min_timeout_us);
		count--;
	}

	/* Check for the break from timed-out loop */
	uc_read32(reg_offset, &reg_val);
	if((reg_val & mask) == value) {
		ret = 0;
	}

	return ret;
}

void  uc_block_write32(uint64_t reg_offset, const uint32_t *data, uint32_t size)
{
	for(uint32_t i = 0; i < size; i++) {
		uc_write32(reg_offset + i * 4U, *data);
		data++;
	}

	return;
}

void uc_block_set32(uint64_t reg_offset, uint32_t data, uint32_t size)
{
	for(uint32_t i = 0; i < size; i++) {
		uc_write32(reg_offset + i * 4U, data);
	}

	return ;
}

// Execute a command passed in struct txn_cmd. cmd_size is not being used currently but kept for future usage.
int  uc_execute_txn_buffer_cmds(txn_cmd *cmd, uint32_t Flags, uint32_t *cmd_size)
{
	(void)Flags;
	(void)cmd_size;
	assert(cmd->opcode < XAIE_IO_CUSTOM_OP_BEGIN);
	int ret = 0;

	switch(cmd->opcode)
	{
		case XAIE_IO_WRITE:
			if(!cmd->mask) {
				uc_write32(cmd->reg_offset, cmd->value);

			} else {
                uc_mask_write32(cmd->reg_offset, cmd->mask, cmd->value);
			}
			//*cmd_size = sizeof(txn_cmd);
			break;
		case XAIE_IO_BLOCKWRITE:
			uc_block_write32(cmd->reg_offset,
					(uint32_t *)(uintptr_t)cmd->data_ptr,
					cmd->size);

			// if(!(Flags & XAIE_TXN_INST_EXPORTED_MASK)) {
			// 	free((void *)cmd->data_ptr);
			// }
			// *cmd_size =  sizeof(txn_cmd) + cmd->size;
			break;
		case XAIE_IO_BLOCKSET:
			uc_block_set32(cmd->reg_offset, cmd->value,
					cmd->size);
			//*cmd_size =  sizeof(txn_cmd) + cmd->size;
			break;
		case XAIE_IO_MASKPOLL:
			/* Force timeout to default value */
			uc_mask_poll(cmd->reg_offset, cmd->mask,
							cmd->value, 0U);

			//*cmd_size = sizeof(txn_cmd);
			break;

		default:
			TRACE("Invalid transaction opcode\n");
			return -1;
	}

	return ret;
}

// Load a txn buffer pointed by buf, parse the buffer and run each command till end of buffer is reached.
int uc_load_txn_buffer(uint32_t* buf)
{
	int ret = 0;
	if(buf == NULL) return -1;
	uint32_t* curr_buffer = buf;
	//uint32_t buf_size = 0;
	//uint32_t curr_buffer_size = 0;
	uint32_t num_ops = 0;
	//buf_size = ((txn_header*)curr_buffer)->txn_size;
	num_ops = ((txn_header*)curr_buffer)->num_ops;
	curr_buffer+= (uint32_t)sizeof(txn_header)/sizeof(uint32_t);
	//curr_buffer_size += (uint32_t)sizeof(txn_header);
	uint32_t cmd_size = 0;
	txn_cmd t_cmd;
	txn_cmd *cmd = &t_cmd;
	//printf("From load_txn_buffer, num_ops:%d:\n", num_ops);

	for(uint32_t i = 0U;i < num_ops;i++){

		if (*(uint8_t*)curr_buffer == XAIE_IO_WRITE){
			write32_header *hdr = (write32_header*)curr_buffer;
			cmd->opcode = XAIE_IO_WRITE;
			cmd->mask = 0U;
			cmd->reg_offset = hdr->reg_off;
			cmd->value = hdr->value;

			cmd->data_ptr = 0U;
			cmd->size = 0U;
			ret =  uc_execute_txn_buffer_cmds(cmd, 0 , &cmd_size);
			curr_buffer += sizeof(write32_header)/sizeof(uint32_t);
			//curr_buffer_size = (curr_buffer - buf) * sizeof(uint32_t);

			continue;


		}
		if (*(uint8_t*)curr_buffer == XAIE_IO_MASKWRITE){
			mask_write32_header *hdr = (mask_write32_header*)curr_buffer;
			cmd->opcode = XAIE_IO_WRITE;
			cmd->mask = hdr->mask;
			cmd->reg_offset = hdr->reg_off;
			cmd->value = hdr->value;
			cmd->data_ptr = 0U;
			cmd->size = 0U;
			ret = uc_execute_txn_buffer_cmds(cmd, 0 , &cmd_size);
			curr_buffer += sizeof(mask_write32_header)/sizeof(uint32_t);
			//curr_buffer_size = (curr_buffer - buf) * sizeof(uint32_t);

			continue;
		}

		if (*(uint8_t*)curr_buffer == XAIE_IO_MASKPOLL) {


			// mask_poll32_header *hdr = (mask_poll32_header*)curr_buffer;
			// cmd->opcode = XAIE_IO_MASKPOLL;
			// cmd->mask = hdr->mask;
			// cmd->reg_offset = hdr->reg_off;
			// cmd->value = hdr->value;
			// cmd->data_ptr = 0U;
			// cmd->size = 0U;
			// ret = execute_txn_buffer_cmds(cmd, 0 , &cmd_size);
			// curr_buffer += sizeof(mask_poll32_header);
			// curr_buffer_size += cmd_size;
			// ret++;
			continue;
		}
		if (*(uint8_t*)curr_buffer == XAIE_IO_BLOCKWRITE) {

			block_write32_header *hdr = (block_write32_header*)curr_buffer;

			cmd->opcode = XAIE_IO_BLOCKWRITE;
			cmd->mask = 0;
			cmd->reg_offset = hdr->reg_off;
			cmd->value = 0;
			cmd->data_ptr = (uint64_t) (curr_buffer + sizeof(block_write32_header)/sizeof(uint32_t));
			cmd->size = (hdr->size -  sizeof(block_write32_header))/sizeof(uint32_t);
			ret = uc_execute_txn_buffer_cmds(cmd, 0 , &cmd_size);
			curr_buffer += (hdr->size)/sizeof(uint32_t);
			//curr_buffer_size = (curr_buffer - buf) * sizeof(uint32_t);

			continue;
		}
		if(*(uint8_t*)curr_buffer== XAIE_IO_BLOCKSET) {
			// Block set gets converted to Block write by driver. This is placeholder in case behavior changes.
			continue;

		}
		if(*(uint8_t*)curr_buffer >= XAIE_IO_CUSTOM_OP_BEGIN) {
			// Do nothing
			continue;
		}
	}

    //return curr_buffer_size;
	return ret;
}

/** @} */
