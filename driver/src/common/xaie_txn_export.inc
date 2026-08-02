/* SPDX-License-Identifier: MIT */

#ifndef XAIE_TXN_EXPORT_NAME
#error "XAIE_TXN_EXPORT_NAME must be defined"
#endif

u8* XAIE_TXN_EXPORT_NAME(XAie_DevInst *DevInst, u8 NumConsumers,
		u32 Flags XAIE_TXN_EXPORT_MAP_PARAMETER)
{
	const XAie_Backend *Backend = DevInst->Backend;
	XAie_TxnInst *TmpInst = NULL;
	u8 *TxnPtr = NULL;
	u8 *TxnBasePtr = NULL;
	u32 BuffSize = 0U, NumOps = 0, PatchCmdCount = 0,BWBuffSize = 0;
	u32 LoadSeqCount = 0;
	u64 RegOffLastBlockWrite = 0;
	u8 FirstBlockwriteProcessed = 0;
	u32* BlockwriteBuffer = NULL;
	u8* LoadSeqCountPtr = NULL;
	u32 LoadSeqCountOffset = 0;
	XAIE_TXN_EXPORT_MAP_STATE;

	u32 AllocatedBuffSize = XAIE_DEFAULT_TXN_BUFFER_SIZE;
	u32 BWBuffAllocatedSize = XAIE_DEFAULT_TXN_BUFFER_SIZE;
	(void)NumConsumers;
	(void)Flags;

	TmpInst = _XAie_GetTxnInst(DevInst, Backend->Ops.GetTid());
	if(TmpInst == NULL) {
		XAIE_ERROR("Failed to get the correct transaction instance "
				"from internal list\n");
		return NULL;
	}

	BlockwriteBuffer = calloc(1,BWBuffAllocatedSize);
	if(BlockwriteBuffer == NULL) {
		XAIE_ERROR("BlockWrite Buffer Calloc failed\n");
		return NULL;
	}

	TxnPtr = calloc(1,AllocatedBuffSize);
	if(TxnPtr == NULL) {
		XAIE_ERROR("TxnPtr Calloc failed\n");
		free(BlockwriteBuffer);
		return NULL;
	}

	// Saved base pointer to avoid backward pointer arithmetic
	TxnBasePtr = TxnPtr;
	_XAie_CreateTxnHeader(DevInst, (XAie_TxnHeader *)(uintptr_t)TxnPtr);
	BuffSize += (u32)sizeof(XAie_TxnHeader);
	TxnPtr += sizeof(XAie_TxnHeader);
	XAIE_DBG("number of cmd %d\n", TmpInst->NumCmds);

	for(u32 i = 0U; i < TmpInst->NumCmds; i++) {
		NumOps++;
		XAie_TxnCmd *Cmd = &TmpInst->CmdBuf[i];

		if( (DevInst->PmLoadingActive == 1) && (Cmd->Opcode != XAIE_IO_LOAD_PM_START) && (Cmd->Opcode != XAIE_IO_LOAD_PM_END_INTERNAL))
		{
			LoadSeqCount++;
		}

		if( (Cmd->Opcode != XAIE_IO_BLOCKWRITE) &&
		    (Cmd->Opcode != XAIE_IO_CUSTOM_OP_DDR_PATCH) &&
		    (FirstBlockwriteProcessed != 0) )
		{
			XAIE_TXN_EXPORT_MAP_FLUSH_BLOCKWRITES(TmpInst, i);
			XAie_BlockWrite32Hdr *Hdr = (XAie_BlockWrite32Hdr*)(uintptr_t)BlockwriteBuffer;
			while((BuffSize + Hdr->Size) > AllocatedBuffSize) {
				u8 *TmpPtr = _XAie_ReallocTxnBuf_MemInit(TxnBasePtr,
				(AllocatedBuffSize) * 2U, BuffSize);
				if(TmpPtr == NULL) {
					XAIE_ERROR("TxnPtr realloc failed\n");
					free(BlockwriteBuffer);
					free(TxnBasePtr);
					return NULL;
				}
				TxnBasePtr = TmpPtr;
				AllocatedBuffSize *= 2U;
				TxnPtr = TxnBasePtr + BuffSize;
			}
			BuffSize += Hdr->Size;
			TxnPtr += _XAie_AppendBWToTxnBuff(BlockwriteBuffer,TxnPtr,PatchCmdCount);
			PatchCmdCount = 0;
			FirstBlockwriteProcessed = 0;
		}
		if ((Cmd->Opcode == XAIE_IO_WRITE) && (Cmd->Mask == 0U)) {
			while((BuffSize + sizeof(XAie_Write32Hdr)) >
					AllocatedBuffSize) {
				u8 *TmpPtr = _XAie_ReallocTxnBuf_MemInit(TxnBasePtr,
						AllocatedBuffSize * 2U, BuffSize);
				if(TmpPtr == NULL){
					XAIE_ERROR("TxnPtr realloc failed\n");
					free(BlockwriteBuffer);
					free(TxnBasePtr);
					return NULL;
				}
				TxnBasePtr = TmpPtr;
				AllocatedBuffSize *= 2U;
				TxnPtr = TxnBasePtr + BuffSize;
			}
			_XAie_AppendWrite32(DevInst, Cmd, TxnPtr);
			XAIE_TXN_EXPORT_MAP_COMMAND(i);
			TxnPtr += sizeof(XAie_Write32Hdr);
			BuffSize += (u32)sizeof(XAie_Write32Hdr);
			continue;
		}
		else if ((Cmd->Opcode == XAIE_IO_WRITE) && ((Cmd->Mask)!=0U)) {
			while((BuffSize + sizeof(XAie_MaskWrite32Hdr)) >
					AllocatedBuffSize) {
				u8 *TmpPtr = _XAie_ReallocTxnBuf_MemInit(TxnBasePtr,
						AllocatedBuffSize * 2U, BuffSize);
				if(TmpPtr == NULL) {
					XAIE_ERROR("TxnPtr realloc failed\n");
					free(BlockwriteBuffer);
					free(TxnBasePtr);
					return NULL;
				}
				TxnBasePtr = TmpPtr;
				AllocatedBuffSize *= 2U;
				TxnPtr = TxnBasePtr + BuffSize;
			}
			_XAie_AppendMaskWrite32(DevInst, Cmd, TxnPtr);
			XAIE_TXN_EXPORT_MAP_COMMAND(i);
			TxnPtr += sizeof(XAie_MaskWrite32Hdr);
			BuffSize += (u32)sizeof(XAie_MaskWrite32Hdr);
			continue;
		}
		else if (Cmd->Opcode == XAIE_IO_MASKPOLL) {
			while((BuffSize + sizeof(XAie_MaskPoll32Hdr)) >
					AllocatedBuffSize) {
				u8 *TmpPtr = _XAie_ReallocTxnBuf_MemInit(TxnBasePtr,
						AllocatedBuffSize * 2U, BuffSize);
				if(TmpPtr == NULL) {
					XAIE_ERROR("TxnPtr realloc failed\n");
					free(BlockwriteBuffer);
					free(TxnBasePtr);
					return NULL;
				}
				TxnBasePtr = TmpPtr;
				AllocatedBuffSize *= 2U;
				TxnPtr = TxnBasePtr + BuffSize;
			}
			_XAie_AppendMaskPoll32(DevInst, Cmd, TxnPtr);
			XAIE_TXN_EXPORT_MAP_COMMAND(i);
			TxnPtr += sizeof(XAie_MaskPoll32Hdr);
			BuffSize += (u32)sizeof(XAie_MaskPoll32Hdr);
			continue;
		}
		else if (Cmd->Opcode == XAIE_IO_MASKPOLL_BUSY) {
			while((BuffSize + sizeof(XAie_MaskPoll32Hdr)) >
					AllocatedBuffSize) {
				u8 *TmpPtr = _XAie_ReallocTxnBuf_MemInit(TxnBasePtr,
						AllocatedBuffSize * 2U, BuffSize);
				if(TmpPtr == NULL) {
					XAIE_ERROR("TxnPtr realloc failed\n");
					free(BlockwriteBuffer);
					free(TxnBasePtr);
					return NULL;
				}
				TxnBasePtr = TmpPtr;
				AllocatedBuffSize *= 2U;
				TxnPtr = TxnBasePtr + BuffSize;
			}
			_XAie_AppendMaskPollBusy32(DevInst, Cmd, TxnPtr);
			XAIE_TXN_EXPORT_MAP_COMMAND(i);
			TxnPtr += sizeof(XAie_MaskPoll32Hdr);
			BuffSize += (u32)sizeof(XAie_MaskPoll32Hdr);
			continue;
		}
		else if (Cmd->Opcode == XAIE_IO_BLOCKWRITE) {
			XAIE_TXN_EXPORT_MAP_BLOCKWRITE(TmpInst, Cmd, i, RegOffLastBlockWrite);
			/**
			 * In case of Block Write and Block Set, it is possible
			 * that the new allocated buffer size may not be sufficient.
			 * In that case we should keep reallocating till the new
			 * buffer size if big enough to hold existing + current opcode.
			 */
			BWBuffSize = ((XAie_BlockWrite32Hdr*)(uintptr_t)BlockwriteBuffer)->Size;
			if(FirstBlockwriteProcessed != 0)
			{
				if ( Cmd->RegOff != RegOffLastBlockWrite)
				{
					while((BuffSize + BWBuffSize) > AllocatedBuffSize) {
						u8 *TmpPtr = _XAie_ReallocTxnBuf_MemInit(TxnBasePtr,
						(AllocatedBuffSize) * 2U, BuffSize);
						if(TmpPtr == NULL) {
							XAIE_ERROR("TxnPtr realloc failed\n");
							free(BlockwriteBuffer);
							free(TxnBasePtr);
							return NULL;
						}
						TxnBasePtr = TmpPtr;
						AllocatedBuffSize *= 2U;
						TxnPtr = TxnBasePtr + BuffSize;
					}
					BuffSize += BWBuffSize;
					TxnPtr += _XAie_AppendBWToTxnBuff(BlockwriteBuffer,TxnPtr,PatchCmdCount);
					PatchCmdCount = 0;
					FirstBlockwriteProcessed = 0;
				}
				else
				{
					NumOps--;

					/**
					 * Load Sequence Count should be reduced only if there is an active
					 * PM Loading else do nothing.
					 **/
					if (DevInst->PmLoadingActive == 1) {
						LoadSeqCount--;
					}
				}
			} else {
				/**
				 *  When BW Buffer is empty which is represented by FirstBlockwriteProcessed == 0
				 *  BWBuffSize should be initialized to size of XAie_BlockWrite32Hdr struct in bytes.
				 *  Since the Cmd->Size field of Cmd from TmpInst->CmdBuf only considers payload size.
				 */
				BWBuffSize = sizeof(XAie_BlockWrite32Hdr);
			}

			while( (Cmd->Size * 4) + BWBuffSize > BWBuffAllocatedSize)
			{
				u32 *TmpBWPtr = (u32*) (uintptr_t) ( _XAie_ReallocTxnBuf_MemInit((u8 *)BlockwriteBuffer,
									( BWBuffAllocatedSize + BWBuffAllocatedSize), BWBuffSize) );
				if(TmpBWPtr == NULL) {
					XAIE_ERROR("BlockWrite Buffer Realloc Failed\n");
					free(BlockwriteBuffer);
					// To free Txn Buffer successfully we need to use the start pointer not current pointer
					free(TxnBasePtr);
					return NULL;
				}
				BlockwriteBuffer = TmpBWPtr;
				BWBuffAllocatedSize *= 2U;
			}
			RegOffLastBlockWrite = (u64) ( Cmd->RegOff + (Cmd->Size*4) );
			_XAie_AppendBWToBlockwriteBuff(DevInst, Cmd, FirstBlockwriteProcessed, BlockwriteBuffer);
			FirstBlockwriteProcessed = 1;
		}
		else if (Cmd->Opcode == XAIE_IO_BLOCKSET) {
			/**
			 * In case of Block Write and Block Set, it is possible
			 * that the new allocated buffer size may not be sufficient
			 * In that case we should keep reallocating till the new
			 * buffer size if big enough to hold existing + current opcode.
			 *
			 * Blockset gets converted to blockwrite. so check for
			 * blockwrite size
			 */
			while((BuffSize + sizeof(XAie_BlockWrite32Hdr) +
						Cmd->Size * sizeof(u32)) >
					AllocatedBuffSize) {
				u8 *TmpPtr = _XAie_ReallocTxnBuf_MemInit(TxnBasePtr,
						AllocatedBuffSize * 2U, BuffSize);
				if(TmpPtr == NULL) {
					XAIE_ERROR("TxnPtr realloc failed\n");
					free(BlockwriteBuffer);
					free(TxnBasePtr);
					return NULL;
				}
				TxnBasePtr = TmpPtr;
				AllocatedBuffSize *= 2U;
				TxnPtr = TxnBasePtr + BuffSize;
			}
			_XAie_AppendBlockSet32(DevInst, Cmd, TxnPtr);
			XAIE_TXN_EXPORT_MAP_COMMAND(i);
			TxnPtr += sizeof(XAie_BlockWrite32Hdr) +
				Cmd->Size * sizeof(u32);
			BuffSize += (u32)sizeof(XAie_BlockWrite32Hdr) +
				Cmd->Size * (u32)sizeof(u32);
			continue;
		}
		else if(Cmd->Opcode == XAIE_IO_NOOP)
		{
			while( (BuffSize + sizeof(XAie_NoOpHdr)) >
					AllocatedBuffSize ) {
				u8 *TmpPtr = _XAie_ReallocTxnBuf_MemInit(TxnBasePtr,
						AllocatedBuffSize * 2U, BuffSize);
				if(TmpPtr == NULL) {
					XAIE_ERROR("TxnPtr realloc failed\n");
					free(BlockwriteBuffer);
					free(TxnBasePtr);
					return NULL;
				}
				TxnBasePtr = TmpPtr;
				AllocatedBuffSize *= 2U;
				TxnPtr = TxnBasePtr + BuffSize;
			}
			_XAie_AppendNoOp(Cmd, TxnPtr);
			XAIE_TXN_EXPORT_MAP_COMMAND(i);
			TxnPtr += sizeof(XAie_NoOpHdr);
			BuffSize += (u32)sizeof(XAie_NoOpHdr);
			continue;
		}
		else if(Cmd->Opcode == XAIE_IO_PREEMPT)
		{
			while( (BuffSize + sizeof(XAie_PreemptHdr)) >
					AllocatedBuffSize ) {
				u8 *TmpPtr = _XAie_ReallocTxnBuf_MemInit(TxnBasePtr,
						AllocatedBuffSize * 2U, BuffSize);
				if(TmpPtr == NULL) {
					XAIE_ERROR("TxnPtr realloc failed\n");
					free(BlockwriteBuffer);
					free(TxnBasePtr);
					return NULL;
				}
				TxnBasePtr = TmpPtr;
				AllocatedBuffSize *= 2U;
				TxnPtr = TxnBasePtr + BuffSize;
			}
			_XAie_AppendPreempt(Cmd, TxnPtr);
			XAIE_TXN_EXPORT_MAP_COMMAND(i);
			TxnPtr += sizeof(XAie_PreemptHdr);
			BuffSize += (u32)sizeof(XAie_PreemptHdr);
			continue;
		}
		else if(Cmd->Opcode == XAIE_IO_LOADPDI)
		{
			while( (BuffSize + sizeof(XAie_LoadPdiHdr)) >
					AllocatedBuffSize ) {
				u8 *TmpPtr = _XAie_ReallocTxnBuf_MemInit(TxnBasePtr,
						AllocatedBuffSize * 2U, BuffSize);
				if(TmpPtr == NULL) {
					XAIE_ERROR("TxnPtr realloc failed\n");
					free(BlockwriteBuffer);
					free(TxnBasePtr);
					return NULL;
				}
				TxnBasePtr = TmpPtr;
				AllocatedBuffSize *= 2U;
				TxnPtr = TxnBasePtr + BuffSize;
			}
			_XAie_AppendLoadPdi(Cmd, TxnPtr);
			XAIE_TXN_EXPORT_MAP_COMMAND(i);
			TxnPtr += sizeof(XAie_LoadPdiHdr);
			BuffSize += (u32)sizeof(XAie_LoadPdiHdr);
			continue;
		}
		else if(Cmd->Opcode == XAIE_IO_LOAD_PM_START)
		{
			while( (BuffSize + sizeof(XAie_PmLoadHdr)) >
					AllocatedBuffSize ) {
				u8 *TmpPtr = _XAie_ReallocTxnBuf_MemInit(TxnBasePtr,
						AllocatedBuffSize * 2U, BuffSize);
				if(TmpPtr == NULL) {
					XAIE_ERROR("TxnPtr realloc failed\n");
					free(BlockwriteBuffer);
					free(TxnBasePtr);
					return NULL;
				}
				TxnBasePtr = TmpPtr;
				AllocatedBuffSize *= 2U;
				TxnPtr = TxnBasePtr + BuffSize;
			}
			LoadSeqCountPtr = _XAie_AppendPmLoad(Cmd, TxnPtr, &LoadSeqCountOffset);
			XAIE_TXN_EXPORT_MAP_COMMAND(i);
			LoadSeqCountOffset += BuffSize;
			TxnPtr += sizeof(XAie_PmLoadHdr);
			BuffSize += (u32)sizeof(XAie_PmLoadHdr);
			DevInst->PmLoadingActive = 1;
			continue;
		}
		else if (Cmd->Opcode == XAIE_IO_LOAD_PM_END_INTERNAL)
		{
			// Check if LoadSeqCountPtr needs to be updated due to realloc of TXN Ptr.
			if ((LoadSeqCountOffset != 0) && (LoadSeqCountPtr != (TxnBasePtr + LoadSeqCountOffset))) {
				LoadSeqCountPtr = TxnBasePtr + LoadSeqCountOffset;
			}

			if((LoadSeqCount != 0) && (LoadSeqCountPtr != NULL))
			{
				*LoadSeqCountPtr = LoadSeqCount & 0xFF;
				*(LoadSeqCountPtr + 1) = (LoadSeqCount & 0xFF00) >> 8;
				*(LoadSeqCountPtr + 2) = (LoadSeqCount & 0xFF0000) >> 16;
			}
			else if (LoadSeqCountPtr == NULL)
			{
				XAIE_ERROR("LoadSeqCountPtr is equal to NULL\n");
				free(BlockwriteBuffer);
				// To free Txn Buffer successfully we need to use the start pointer not current pointer
				free(TxnBasePtr);
				return NULL;
			}
			LoadSeqCount = 0;
			NumOps--;
			DevInst->PmLoadingActive = 0;
			XAIE_TXN_EXPORT_MAP_NO_OPERATION(i);
			continue;
		}
		else if(Cmd->Opcode == XAIE_IO_CREATE_SCRATCHPAD)
		{
			while( (BuffSize + sizeof(XAie_CreateScratchpadHdr)) >
					AllocatedBuffSize ) {
				u8 *TmpPtr = _XAie_ReallocTxnBuf_MemInit(TxnBasePtr,
						AllocatedBuffSize * 2U, BuffSize);
				if(TmpPtr == NULL) {
					XAIE_ERROR("TxnPtr realloc failed\n");
					free(BlockwriteBuffer);
					free(TxnBasePtr);
					return NULL;
				}
				TxnBasePtr = TmpPtr;
				AllocatedBuffSize *= 2U;
				TxnPtr = TxnBasePtr + BuffSize;
			}
			_XAie_AppendCreateScratchpad(Cmd, TxnPtr);
			XAIE_TXN_EXPORT_MAP_COMMAND(i);
			TxnPtr += sizeof(XAie_CreateScratchpadHdr);
			BuffSize += (u32)sizeof(XAie_CreateScratchpadHdr);
			continue;
		}
		else if(Cmd->Opcode == XAIE_IO_UPDATE_STATE_TABLE)
		{
			while((BuffSize + sizeof(XAie_UpdateStateHdr)) >
					AllocatedBuffSize ) {
				u8 *TmpPtr = _XAie_ReallocTxnBuf_MemInit(TxnBasePtr,
						AllocatedBuffSize * 2U, BuffSize);
				if(TmpPtr == NULL) {
					XAIE_ERROR("TxnPtr realloc failed\n");
					free(BlockwriteBuffer);
					free(TxnBasePtr);
					return NULL;
				}
				TxnBasePtr = TmpPtr;
				AllocatedBuffSize *= 2U;
				TxnPtr = TxnBasePtr + BuffSize;
			}
			_XAie_AppendUpdateState(Cmd, TxnPtr);
			XAIE_TXN_EXPORT_MAP_COMMAND(i);
			TxnPtr += sizeof(XAie_UpdateStateHdr);
			BuffSize += (u32)sizeof(XAie_UpdateStateHdr);
			continue;
		}
		else if(Cmd->Opcode == XAIE_IO_UPDATE_REG)
		{
			XAIE_TXN_EXPORT_MAP_BEFORE_UPDATE_REG(NumOps);
			/* Check for 16KB boundary crossing and add padding if needed */
			TxnPtr = _XAie_HandleTxnCmd16KBBoundary(TxnPtr, &BuffSize,
								&AllocatedBuffSize, (u32)sizeof(XAie_UpdateRegHdr),
								BlockwriteBuffer, &NumOps);
			if (TxnPtr == NULL) {
				return NULL;
			}
			while((BuffSize + sizeof(XAie_UpdateRegHdr)) >
					AllocatedBuffSize ) {
				u8 *TmpPtr = _XAie_ReallocTxnBuf_MemInit(TxnBasePtr,
						AllocatedBuffSize * 2U, BuffSize);
				if(TmpPtr == NULL) {
					XAIE_ERROR("TxnPtr realloc failed\n");
					free(BlockwriteBuffer);
					free(TxnBasePtr);
					return NULL;
				}
				TxnBasePtr = TmpPtr;
				AllocatedBuffSize *= 2U;
				TxnPtr = TxnBasePtr + BuffSize;
			}
			_XAie_AppendUpdateReg(Cmd, TxnPtr);
			XAIE_TXN_EXPORT_MAP_UPDATE_REG(i, NumOps);
			TxnPtr += sizeof(XAie_UpdateRegHdr);
			BuffSize += (u32)sizeof(XAie_UpdateRegHdr);
			continue;
		}
		else if(Cmd->Opcode == XAIE_IO_UPDATE_SCRATCH)
		{
			while( (BuffSize + sizeof(XAie_UpdateScratchHdr)) >
					AllocatedBuffSize ) {
				u8 *TmpPtr = _XAie_ReallocTxnBuf_MemInit(TxnBasePtr,
						AllocatedBuffSize * 2U, BuffSize);
				if(TmpPtr == NULL) {
					XAIE_ERROR("TxnPtr realloc failed\n");
					free(BlockwriteBuffer);
					free(TxnBasePtr);
					return NULL;
				}
				TxnBasePtr = TmpPtr;
				AllocatedBuffSize *= 2U;
				TxnPtr = TxnBasePtr + BuffSize;
			}
			_XAie_AppendUpdateScratch(Cmd, TxnPtr);
			XAIE_TXN_EXPORT_MAP_COMMAND(i);
			TxnPtr += sizeof(XAie_UpdateScratchHdr);
			BuffSize += (u32)sizeof(XAie_UpdateScratchHdr);
			continue;
		}
		else if (Cmd->Opcode >= XAIE_IO_CUSTOM_OP_TCT) {

			if(Cmd->Opcode == XAIE_IO_CUSTOM_OP_DDR_PATCH)
			{
				++PatchCmdCount;
			}
			if (TX_DUMP_ENABLE) {
				TxnCmdDump(Cmd);
			}

			XAIE_DBG("Size of the CustomOp Hdr being exported: %u bytes\n", sizeof(XAie_CustomOpHdr));

			while((BuffSize + sizeof(XAie_CustomOpHdr) +
						Cmd->Size) > AllocatedBuffSize) {
				u8 *TmpPtr = _XAie_ReallocTxnBuf_MemInit(TxnBasePtr,
						AllocatedBuffSize * 2U, BuffSize);
				if(TmpPtr == NULL) {
					XAIE_ERROR("TxnPtr realloc failed\n");
					free(BlockwriteBuffer);
					free(TxnBasePtr);
					return NULL;
				}
				TxnBasePtr = TmpPtr;
				AllocatedBuffSize *= 2U;
				TxnPtr = TxnBasePtr + BuffSize;
			}
			_XAie_AppendCustomOp(Cmd, TxnPtr);
			XAIE_TXN_EXPORT_MAP_CUSTOM_COMMAND(Cmd, i);
			TxnPtr += sizeof(XAie_CustomOpHdr) +
				Cmd->Size * sizeof(u8);
			BuffSize += (u32)sizeof(XAie_CustomOpHdr) +
				Cmd->Size * (u32)sizeof(u8);
			continue;
		}
		XAIE_TXN_EXPORT_MAP_UNHANDLED_COMMAND(Cmd, i);
	}
	if(FirstBlockwriteProcessed != 0)
	{
		XAIE_TXN_EXPORT_MAP_FLUSH_BLOCKWRITES(TmpInst, TmpInst->NumCmds);
		XAie_BlockWrite32Hdr *Hdr = (XAie_BlockWrite32Hdr*)(uintptr_t)BlockwriteBuffer;
		while((BuffSize + Hdr->Size) > AllocatedBuffSize) {
			u8 *TmpPtr = _XAie_ReallocTxnBuf_MemInit(TxnBasePtr,
						(AllocatedBuffSize) * 2U, BuffSize);
			if(TmpPtr == NULL) {
				XAIE_ERROR("TxnPtr realloc failed\n");
				free(BlockwriteBuffer);
				free(TxnBasePtr);
				return NULL;
			}
			TxnBasePtr = TmpPtr;
			AllocatedBuffSize *= 2U;
			TxnPtr = TxnBasePtr + BuffSize;
		}
		BuffSize += Hdr->Size;
		TxnPtr += _XAie_AppendBWToTxnBuff(BlockwriteBuffer,TxnPtr,PatchCmdCount);
	}

	XAIE_TXN_EXPORT_MAP_FINALIZE(TmpInst, NumOps, BlockwriteBuffer, TxnBasePtr);

	// Free the BlockwriteBuffer
	free(BlockwriteBuffer);

	u32 four_byte_aligned_BuffSize = ((BuffSize % 4U) != 0U) ? ((BuffSize / 4U + 1U)*4) : BuffSize;
	XAIE_DBG("Size of the Txn Hdr being exported: %u bytes\n",
			sizeof(XAie_TxnHeader));
	XAIE_DBG("Size of the transaction buffer being exported: %u bytes\n",
			four_byte_aligned_BuffSize);
	XAIE_DBG("Num of Operations in the transaction buffer: %u\n",
			TmpInst->NumCmds);

	/* Adjust pointer and reallocate to the right size */
	u8 *NewPtr = _XAie_ReallocTxnBuf(TxnBasePtr, four_byte_aligned_BuffSize);
	if(NewPtr == NULL) {
		XAIE_ERROR("TxnPtr realloc failed\n");
		free(TxnBasePtr);  /* Free original buffer on failure */
		return NULL;
	}
	TxnBasePtr = NewPtr;
	((XAie_TxnHeader *)(uintptr_t)TxnBasePtr)->NumOps =  NumOps;
	((XAie_TxnHeader *)(uintptr_t)TxnBasePtr)->TxnSize =  four_byte_aligned_BuffSize;

	return (u8 *)TxnBasePtr;
}
