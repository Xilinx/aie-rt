/******************************************************************************
* Copyright (C) 2023 AMD.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_cdo.c
* @{
*
* This file contains the data structures and routines for low level IO
* operations for controlcode backend.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who        Date     Changes
* ----- ------     -------- -----------------------------------------------------
* 1.0   Sankarji   03/08/2023 Initial creation.
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xaie_helper.h"
#include "xaie_io.h"
#include "xaie_io_common.h"
#include "xaie_io_privilege.h"
#include "xaie_npi.h"
#include "isa_stubs.h"

#ifdef __AIECONTROLCODE__

#define TEMP_ASM_FILE1    ".temp_data1.txt"
#define TEMP_ASM_FILE2    ".temp_data2.txt"
#define TEMP_ASM_FILE3    ".temp_data3.txt"

//#define UC_DMA_DATASZ					4
//#define DATA_SECTION_ALIGNMENT        16

/*#define START_JOB_OPSZ				8
#define END_JOB_OPSZ					4
#define EOF_OPSZ						4
#define UC_DMA_WRITE_DES_SYNC_OPSZ		4
#define MASK_WRITE32_OPSZ				16
#define WRITE_32_DATA_OPSZ				12
#define UC_DMA_BD_OPSZ					16
#define UC_DMA_DATASZ					4
#define DATA_SECTION_ALIGNMENT          16
#define HEADER_SIZE						16*/

/************************** Constant Definitions *****************************/
static u8 is_shim_bd;

/****************************** Type Definitions *****************************/
typedef struct {
	XAie_DevInst *DevInst;
	u64 BaseAddr;
	u64 NpiBaseAddr;
	FILE *ControlCodefp;
	FILE *ControlCodedatafp;
	FILE *ControlCodedata2fp;
	FILE *ControlCodedata3fp;
	u32  UcbdLabelNum;
	u32  UcbdDataNum;
	u32  UcDmaDataNum;
	u32  UcJobNum;
	u32  UcPageSize;
	u32  UcPageTextSize;
	u32  PageSizeMax;
	u8   CombineCommands;
	u8   IsJobOpen;
	u8   IsPageOpen;
	char *ScrachpadName;
} XAie_ControlCodeIO;

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This is the memory IO function to free the global IO instance
*
* @param	IOInst: IO Instance pointer.
*
* @return	None.
*
* @note		The global IO instance is a singleton and freed when
* the reference count reaches a zero. Internal only.
*
*******************************************************************************/
static AieRC XAie_ControlCodeIO_Finish(void *IOInst)
{
	XAie_ControlCodeIO  *ControlCodeInst = (XAie_ControlCodeIO *)IOInst;
	XAie_DevInst *DevInst = ControlCodeInst->DevInst;
	XAie_CloseControlCodeFile(DevInst);

	if(IOInst) {
		free(IOInst);
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This is the memory IO function to initialize the global IO instance
*
* @param	DevInst: Device instance pointer.
*
* @return	XAIE_OK on success. Error code on failure.
*
* @note		Internal only.
*
*******************************************************************************/
static AieRC XAie_ControlCodeIO_Init(XAie_DevInst *DevInst)
{
	XAie_ControlCodeIO *IOInst;

	IOInst = (XAie_ControlCodeIO *)malloc(sizeof(*IOInst));
	if(IOInst == NULL) {
		XAIE_ERROR("Memory allocation failed\n");
		return XAIE_ERR;
	}

	IOInst->BaseAddr = DevInst->BaseAddr;
	IOInst->NpiBaseAddr = XAIE_NPI_BASEADDR;
	IOInst->ScrachpadName = NULL;
	DevInst->IOInst = IOInst;
	IOInst->DevInst = DevInst;


	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This function ends a Control code Job
*
* @param	ControlCodeInst: ControlCodeInst instance pointer.
*
* @return	void.
*
* @note		Internal only.
*
*******************************************************************************/
static void _XAie_EndJob(XAie_ControlCodeIO  *ControlCodeInst) {

	if(ControlCodeInst->IsPageOpen && ControlCodeInst->IsJobOpen) {
		fprintf(ControlCodeInst->ControlCodefp, "END_JOB\n\n");
		ControlCodeInst->IsJobOpen 		= 0;
		ControlCodeInst->CombineCommands 	= 0;
	}
}

/*****************************************************************************/
/**
*
* This function ends a Control code Page
*
* @param	ControlCodeInst: ControlCodeInst instance pointer.
*
* @return	void.
*
* @note		Internal only.
*
*******************************************************************************/
static void _XAie_EndPage(XAie_ControlCodeIO  *ControlCodeInst) {

	if(ControlCodeInst->IsPageOpen) {
		_XAie_EndJob(ControlCodeInst);
		fprintf(ControlCodeInst->ControlCodefp, ".eop\n\n");
		ControlCodeInst->UcPageSize 	= 0;
		ControlCodeInst->UcPageTextSize = 0;
		ControlCodeInst->IsPageOpen 	= 0;
	}
}

/*****************************************************************************/
/**
*
* This function starts a new Control code Page
*
* @param	ControlCodeInst: ControlCodeInst instance pointer.
*
* @return	void.
*
* @note		Internal only.
*
*******************************************************************************/
static void _XAie_StartNewPage(XAie_ControlCodeIO  *ControlCodeInst) {

	if(ControlCodeInst->IsPageOpen) {
		_XAie_EndPage(ControlCodeInst);
	}

	ControlCodeInst->UcPageTextSize	 = ISA_OPSIZE_EOF;
	ControlCodeInst->UcPageSize 	 = PAGE_HEADER_SIZE + ISA_OPSIZE_EOF;
	ControlCodeInst->CombineCommands = 0;
	ControlCodeInst->IsPageOpen 	 = 1;
}

/*****************************************************************************/
/**
*
* This function starts a new Control code Job
*
* @param	ControlCodeInst: ControlCodeInst instance pointer.
*
* @return	void.
*
* @note		Internal only.
*
*******************************************************************************/
static void _XAie_StartNewJob(XAie_ControlCodeIO  *ControlCodeInst) {
	// no open page, create one
	if (!ControlCodeInst->IsPageOpen) {
		_XAie_StartNewPage(ControlCodeInst);
	}

	u32 DataAligner = (DATA_SECTION_ALIGNMENT -
		((ControlCodeInst->UcPageTextSize + ISA_OPSIZE_START_JOB + ISA_OPSIZE_END_JOB) % DATA_SECTION_ALIGNMENT));
        if (DataAligner == DATA_SECTION_ALIGNMENT) {
                DataAligner = 0U;
        }

	// the existing page cannot fit in a new job
	// >= to prevent an empty job
	if (ControlCodeInst->UcPageSize + ISA_OPSIZE_START_JOB + ISA_OPSIZE_END_JOB + DataAligner >= ControlCodeInst->PageSizeMax) {
		_XAie_StartNewPage(ControlCodeInst);
	}

	// close current job on current page
	if (ControlCodeInst->IsJobOpen) {
		_XAie_EndJob(ControlCodeInst);
	}

	fprintf(ControlCodeInst->ControlCodefp, "START_JOB %d\n",
		ControlCodeInst->UcJobNum);
	ControlCodeInst->UcPageTextSize += ISA_OPSIZE_START_JOB + ISA_OPSIZE_END_JOB;
	ControlCodeInst->UcPageSize += ISA_OPSIZE_START_JOB + ISA_OPSIZE_END_JOB;
	ControlCodeInst->UcJobNum++;
	ControlCodeInst->IsJobOpen = 1;
	ControlCodeInst->CombineCommands = 0;
}

/*****************************************************************************/
/**
*
* This is the memory IO function to write 32bit data to the specified address.
*
* @param	IOInst: IO instance pointer
* @param	RegOff: Register offset to read from.
* @param	Data: 32-bit data to be written.
*
* @return	None.
*
* @note		None.
* @note		Internal only.
*
*******************************************************************************/
static AieRC XAie_ControlCodeIO_Write32(void *IOInst, u64 RegOff, u32 Value)
{
	XAie_ControlCodeIO  *ControlCodeInst = (XAie_ControlCodeIO *)IOInst;
	u32 DataAligner = (DATA_SECTION_ALIGNMENT -
		((ControlCodeInst->UcPageTextSize + ISA_OPSIZE_UC_DMA_WRITE_DES_SYNC) % DATA_SECTION_ALIGNMENT));
	if (DataAligner == DATA_SECTION_ALIGNMENT) {
		DataAligner = 0U;
	}

	if (ControlCodeInst->ControlCodefp != NULL) {

		if (!ControlCodeInst->IsJobOpen) {
			_XAie_StartNewJob(ControlCodeInst);
		}

		if((ControlCodeInst->UcPageSize + ISA_OPSIZE_UC_DMA_WRITE_DES_SYNC +
			UC_DMA_BD_SIZE + UC_DMA_WORD_LEN + DataAligner) > ControlCodeInst->PageSizeMax) {
			_XAie_StartNewPage(ControlCodeInst);
			_XAie_StartNewJob(ControlCodeInst);
		}

		if(ControlCodeInst->CombineCommands) {
			fseek(ControlCodeInst->ControlCodedatafp, -3, SEEK_CUR);
			fprintf(ControlCodeInst->ControlCodedatafp, " 1\n");
		}
		else {
			fprintf(ControlCodeInst->ControlCodefp,
					"UC_DMA_WRITE_DES_SYNC\t @UCBD_label_%d\n",
					ControlCodeInst->UcbdLabelNum);
			fprintf(ControlCodeInst->ControlCodedatafp, "UCBD_label_%d:\n",
					ControlCodeInst->UcbdLabelNum);
			ControlCodeInst->CombineCommands = 1;
			ControlCodeInst->UcbdLabelNum++;
			ControlCodeInst->UcPageTextSize += ISA_OPSIZE_UC_DMA_WRITE_DES_SYNC;
			ControlCodeInst->UcPageSize += ISA_OPSIZE_UC_DMA_WRITE_DES_SYNC;
		}

		fprintf(ControlCodeInst->ControlCodedatafp,
                                "\t UC_DMA_BD\t 0, 0x%lx, @WRITE_data_%d, 1, 0, 0\n",
                                 RegOff,  ControlCodeInst->UcbdDataNum);

		ControlCodeInst->UcPageSize += UC_DMA_BD_SIZE;
		fprintf(ControlCodeInst->ControlCodedata2fp, "WRITE_data_%d:\n",
				ControlCodeInst->UcbdDataNum);
		fprintf(ControlCodeInst->ControlCodedata2fp, "\t.long 0x%08x\n", Value);
		ControlCodeInst->UcbdDataNum++;
		ControlCodeInst->UcPageSize += UC_DMA_WORD_LEN;
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This is the memory IO function to read 32bit data from the specified address.
*
* @param	IOInst: IO instance pointer
* @param	RegOff: Register offset to read from.
* @param	Data: Pointer to store the 32 bit value
*
* @return	XAIE_OK on success.
*
* @note		None.
* @note		Internal only.
*
*******************************************************************************/
static AieRC XAie_ControlCodeIO_Read32(void *IOInst, u64 RegOff, u32 *Data)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	*Data = 0U;
	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This is the memory IO function to write masked 32bit data to the specified
* address.
*
* @param	IOInst: IO instance pointer
* @param	RegOff: Register offset to read from.
* @param	Mask: Mask to be applied to Data.
* @param	Value: 32-bit data to be written.
*
* @return	None.
*
* @note		None.
* @note		Internal only.
*
*******************************************************************************/
static AieRC XAie_ControlCodeIO_MaskWrite32(void *IOInst, u64 RegOff, u32 Mask,
		u32 Value)
{
	XAie_ControlCodeIO  *ControlCodeInst = (XAie_ControlCodeIO *)IOInst;
	u32 DataAligner = (DATA_SECTION_ALIGNMENT -
		((ControlCodeInst->UcPageTextSize + ISA_OPSIZE_MASK_WRITE_32) % DATA_SECTION_ALIGNMENT));
	if (DataAligner == DATA_SECTION_ALIGNMENT) {
		DataAligner = 0U;
	}

	if (ControlCodeInst->ControlCodefp != NULL) {
		if (!ControlCodeInst->IsJobOpen) {
			_XAie_StartNewJob(ControlCodeInst);
		}

		if((ControlCodeInst->UcPageSize + ISA_OPSIZE_MASK_WRITE_32 +
			DataAligner) > ControlCodeInst->PageSizeMax) {
			_XAie_StartNewPage(ControlCodeInst);
			_XAie_StartNewJob(ControlCodeInst);
		}

		fprintf(ControlCodeInst->ControlCodefp, "MASK_WRITE_32\t 0x%lx, 0x%x, 0x%x\n",
				RegOff, Mask, Value );
		ControlCodeInst->CombineCommands = 0;
		ControlCodeInst->UcPageSize += ISA_OPSIZE_MASK_WRITE_32;
		ControlCodeInst->UcPageTextSize += ISA_OPSIZE_MASK_WRITE_32;
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This is the memory IO function to mask poll an address for a value.
*
* @param	IOInst: IO instance pointer
* @param	RegOff: Register offset to read from.
* @param	Mask: Mask to be applied to Data.
* @param	Value: 32-bit value to poll for
* @param	TimeOutUs: Timeout in micro seconds.
*
* @return	XAIE_OK or XAIE_ERR.
*
* @note		None.
* @note		Internal only.
*
*******************************************************************************/
static AieRC XAie_ControlCodeIO_MaskPoll(void *IOInst, u64 RegOff, u32 Mask, u32 Value,
		u32 TimeOutUs)
{

	XAie_ControlCodeIO  *ControlCodeInst = (XAie_ControlCodeIO *)IOInst;
	u32 DataAligner = (DATA_SECTION_ALIGNMENT -
		((ControlCodeInst->UcPageTextSize + ISA_OPSIZE_MASK_POLL_32) % DATA_SECTION_ALIGNMENT));
	if (DataAligner == DATA_SECTION_ALIGNMENT) {
		DataAligner = 0U;
	}

	(void) TimeOutUs;
	if (ControlCodeInst->ControlCodefp != NULL) {
		if (!ControlCodeInst->IsJobOpen) {
			_XAie_StartNewJob(ControlCodeInst);
		}

		if((ControlCodeInst->UcPageSize + ISA_OPSIZE_MASK_POLL_32 +
			DataAligner) > ControlCodeInst->PageSizeMax) {
			_XAie_StartNewPage(ControlCodeInst);
			_XAie_StartNewJob(ControlCodeInst);
		}

		fprintf(ControlCodeInst->ControlCodefp, "MASK_POLL_32\t 0x%lx, 0x%x, 0x%x\n",
				RegOff, Mask, Value );
		ControlCodeInst->CombineCommands = 0;
		ControlCodeInst->UcPageSize += ISA_OPSIZE_MASK_POLL_32;
		ControlCodeInst->UcPageTextSize += ISA_OPSIZE_MASK_POLL_32;
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This is the memory IO function to write a block of data to aie.
*
* @param	IOInst: IO instance pointer
* @param	RegOff: Register offset to read from.
* @param	Data: Pointer to the data buffer.
* @param	Size: Number of 32-bit words.
*
* @return	None.
*
* @note		None.
* @note		Internal only.
*
*******************************************************************************/
static AieRC XAie_ControlCodeIO_BlockWrite32(void *IOInst, u64 RegOff, const u32 *Data,
		u32 Size)
{
	u32 CompletedSize = 0;
	u32 IterationSize;
	u64 AdjustedOff = 0;
	u32 TempItrSize = 0;

	XAie_ControlCodeIO  *ControlCodeInst = (XAie_ControlCodeIO *)IOInst;
	u32 DataAligner;

	while (Size > CompletedSize) {
		if (ControlCodeInst->ControlCodefp != NULL) {
			if (!ControlCodeInst->IsJobOpen) {
				_XAie_StartNewJob(ControlCodeInst);
			}

			DataAligner = (DATA_SECTION_ALIGNMENT -
				((ControlCodeInst->UcPageTextSize + ISA_OPSIZE_UC_DMA_WRITE_DES_SYNC) % DATA_SECTION_ALIGNMENT));
			if (DataAligner == DATA_SECTION_ALIGNMENT) {
				DataAligner = 0U;
			}

			if((ControlCodeInst->UcPageSize + ISA_OPSIZE_UC_DMA_WRITE_DES_SYNC +
				UC_DMA_BD_SIZE + UC_DMA_WORD_LEN + DataAligner) > ControlCodeInst->PageSizeMax) {
				_XAie_StartNewPage(ControlCodeInst);
				_XAie_StartNewJob(ControlCodeInst);
			}

			if(is_shim_bd){
				ControlCodeInst->CombineCommands = 0;
			}

			if(ControlCodeInst->CombineCommands) {
				fseek(ControlCodeInst->ControlCodedatafp, -3, SEEK_CUR);
				fprintf(ControlCodeInst->ControlCodedatafp, " 1\n");
			}
			else {
				fprintf(ControlCodeInst->ControlCodefp,
						"UC_DMA_WRITE_DES_SYNC\t @UCBD_label_%d\n",
						ControlCodeInst->UcbdLabelNum);
				fprintf(ControlCodeInst->ControlCodedatafp, "UCBD_label_%d:\n",
						ControlCodeInst->UcbdLabelNum);
				ControlCodeInst->CombineCommands = 1;
				ControlCodeInst->UcbdLabelNum++;
				ControlCodeInst->UcPageSize += ISA_OPSIZE_UC_DMA_WRITE_DES_SYNC;
				ControlCodeInst->UcPageTextSize += ISA_OPSIZE_UC_DMA_WRITE_DES_SYNC;
			}

			DataAligner = (DATA_SECTION_ALIGNMENT -
			(ControlCodeInst->UcPageTextSize % DATA_SECTION_ALIGNMENT));
			if (DataAligner == DATA_SECTION_ALIGNMENT) {
				DataAligner = 0U;
			}

			fprintf(ControlCodeInst->ControlCodedata3fp, "DMAWRITE_data_%d:\n",
					ControlCodeInst->UcDmaDataNum);
			ControlCodeInst->UcPageSize += UC_DMA_BD_SIZE;
			for (IterationSize = TempItrSize; IterationSize < Size; IterationSize++) {
				 if( (ControlCodeInst->UcPageSize + UC_DMA_WORD_LEN + DataAligner) > ControlCodeInst->PageSizeMax )
				 {
					break;
				 }
				fprintf(ControlCodeInst->ControlCodedata3fp, "\t.long 0x%08x\n", *(Data+IterationSize));
				ControlCodeInst->UcPageSize += UC_DMA_WORD_LEN;
			}

                        fprintf(ControlCodeInst->ControlCodedatafp,
                                        "\t UC_DMA_BD\t 0, 0x%lx, @DMAWRITE_data_%d, 0x%x, 0, 0\n",
                                        (RegOff + AdjustedOff),  ControlCodeInst->UcDmaDataNum,
					(IterationSize - TempItrSize));

			AdjustedOff += ((IterationSize - TempItrSize) * UC_DMA_WORD_LEN);
			CompletedSize += (IterationSize - TempItrSize);
			TempItrSize = IterationSize;
			ControlCodeInst->UcDmaDataNum++;
		}
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This is the memory IO function to initialize a chunk of aie address space with
* a specified value.
*
* @param	IOInst: IO instance pointer
* @param	RegOff: Register offset to read from.
* @param	Data: Data to initialize a chunk of aie address space..
* @param	Size: Number of 32-bit words.
*
* @return	None.
*
* @note		None.
* @note		Internal only.
*
*******************************************************************************/
static AieRC XAie_ControlCodeIO_BlockSet32(void *IOInst, u64 RegOff, u32 Data, u32 Size)
{
	u32 CompletedSize = 0;
	u32 IterationSize;
	u64 AdjustedOff = 0;

	XAie_ControlCodeIO  *ControlCodeInst = (XAie_ControlCodeIO *)IOInst;
	u32 DataAligner;

	CompletedSize = 0;
	while (Size > CompletedSize) {
		if (ControlCodeInst->ControlCodefp != NULL) {
			if (!ControlCodeInst->IsJobOpen) {
				_XAie_StartNewJob(ControlCodeInst);
			}

			DataAligner = (DATA_SECTION_ALIGNMENT -
				((ControlCodeInst->UcPageTextSize + ISA_OPSIZE_UC_DMA_WRITE_DES_SYNC) % DATA_SECTION_ALIGNMENT));
			if (DataAligner == DATA_SECTION_ALIGNMENT) {
				DataAligner = 0U;
			}

			if((ControlCodeInst->UcPageSize + ISA_OPSIZE_UC_DMA_WRITE_DES_SYNC +
				UC_DMA_BD_SIZE + UC_DMA_WORD_LEN + DataAligner) > ControlCodeInst->PageSizeMax) {
				_XAie_StartNewPage(ControlCodeInst);
				_XAie_StartNewJob(ControlCodeInst);
			}

			if(ControlCodeInst->CombineCommands) {
				fseek(ControlCodeInst->ControlCodedatafp, -3, SEEK_CUR);
				fprintf(ControlCodeInst->ControlCodedatafp, " 1\n");
			}
			else {
				fprintf(ControlCodeInst->ControlCodefp,
						"UC_DMA_WRITE_DES_SYNC\t @UCBD_label_%d\n",
						ControlCodeInst->UcbdLabelNum);
				fprintf(ControlCodeInst->ControlCodedatafp, "UCBD_label_%d:\n",
						ControlCodeInst->UcbdLabelNum);
				ControlCodeInst->CombineCommands = 1;
				ControlCodeInst->UcbdLabelNum++;
				ControlCodeInst->UcPageSize += ISA_OPSIZE_UC_DMA_WRITE_DES_SYNC;
				ControlCodeInst->UcPageTextSize += ISA_OPSIZE_UC_DMA_WRITE_DES_SYNC;
			}

			DataAligner = (DATA_SECTION_ALIGNMENT -
				(ControlCodeInst->UcPageTextSize % DATA_SECTION_ALIGNMENT));
			if (DataAligner == DATA_SECTION_ALIGNMENT) {
				DataAligner = 0U;
			}

			fprintf(ControlCodeInst->ControlCodedata3fp, "DMAWRITE_data_%d:\n",
					ControlCodeInst->UcDmaDataNum);
			ControlCodeInst->UcPageSize += UC_DMA_BD_SIZE;
			for(IterationSize = 0; (IterationSize + CompletedSize) < Size &&
				(ControlCodeInst->UcPageSize + UC_DMA_WORD_LEN + DataAligner)
				<= ControlCodeInst->PageSizeMax; IterationSize++)
			{
				fprintf(ControlCodeInst->ControlCodedata3fp, "\t.long 0x%08x\n", Data);
				ControlCodeInst->UcPageSize += UC_DMA_WORD_LEN;
			}

                        fprintf(ControlCodeInst->ControlCodedatafp,
                                     "\t UC_DMA_BD\t 0, 0x%lx, @DMAWRITE_data_%d, %d, 0, 0\n\n",
                                     (RegOff + AdjustedOff),
                                     ControlCodeInst->UcDmaDataNum, IterationSize);

			AdjustedOff += (IterationSize * UC_DMA_WORD_LEN);
			CompletedSize += IterationSize;
			ControlCodeInst->UcDmaDataNum++;
		}
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This is the memory IO function to perform Address Patching by CERT
* @param	IOInst:    IO instance pointer
* @param	Arg_Index: Index of the argument to be patched coresponding to its index in Kernel Signature.
* @param	Num_BDs:   Represents Number of BDs to be patched.
*
* @return	None.
*
* @note		None.
* @note		Internal only.
*
*******************************************************************************/
static AieRC XAie_ControlCodeIO_AddressPatching(void *IOInst, u32 Arg_Index, u8 Num_BDs)
{
	XAie_ControlCodeIO  *ControlCodeInst = (XAie_ControlCodeIO *)IOInst;
	u32 DataAligner = (DATA_SECTION_ALIGNMENT -
		((ControlCodeInst->UcPageTextSize + ISA_OPSIZE_APPLY_OFFSET_57 + ISA_OPSIZE_UC_DMA_WRITE_DES_SYNC) % DATA_SECTION_ALIGNMENT));

	if (DataAligner == DATA_SECTION_ALIGNMENT) {
		DataAligner = 0U;
	}

	if (ControlCodeInst->ControlCodefp != NULL) {

		if (!ControlCodeInst->IsJobOpen) {
			_XAie_StartNewJob(ControlCodeInst);
		}

		if((ControlCodeInst->UcPageSize + ISA_OPSIZE_APPLY_OFFSET_57 + ISA_OPSIZE_UC_DMA_WRITE_DES_SYNC +
			UC_DMA_BD_SIZE + (UC_DMA_WORD_LEN * Num_BDs * 9) + DataAligner) > ControlCodeInst->PageSizeMax) {
			_XAie_StartNewPage(ControlCodeInst);
			_XAie_StartNewJob(ControlCodeInst);
		}

		if(ControlCodeInst->ScrachpadName == NULL)
			fprintf(ControlCodeInst->ControlCodefp,
					"APPLY_OFFSET_57\t @DMAWRITE_data_%d, %d, %d\n",
					ControlCodeInst->UcDmaDataNum,
					Num_BDs, Arg_Index);
		else
			fprintf(ControlCodeInst->ControlCodefp,
                                        "APPLY_OFFSET_57\t @DMAWRITE_data_%d, %d, %d, @%s\n",
                                        ControlCodeInst->UcDmaDataNum,
                                        Num_BDs, Arg_Index, ControlCodeInst->ScrachpadName);
		ControlCodeInst->UcPageTextSize += ISA_OPSIZE_APPLY_OFFSET_57;
		ControlCodeInst->UcPageSize += ISA_OPSIZE_APPLY_OFFSET_57;
	}

        if(ControlCodeInst->ScrachpadName)
                free(ControlCodeInst->ScrachpadName);
        ControlCodeInst->ScrachpadName = NULL;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This function is used to save the time stamps.
*
* @param        DevInst: Device instance pointer
* @param        Timestamp: Unique id.
*
* @return       XAIE_OK or XAIE_ERR.
*
*******************************************************************************/
AieRC XAie_ControlCodeSaveTimestamp(XAie_DevInst *DevInst, u32 Timestamp)
{
        if(DevInst->Backend->Type != XAIE_IO_BACKEND_CONTROLCODE) {
                XAIE_ERROR("This is supported only in Controlcode Backend %d \n", DevInst->Backend->Type);
                return XAIE_INVALID_BACKEND;
        }

        XAie_ControlCodeIO  *ControlCodeInst = (XAie_ControlCodeIO *)DevInst->IOInst;

        u32 DataAligner = (DATA_SECTION_ALIGNMENT -
                ((ControlCodeInst->UcPageTextSize + ISA_OPSIZE_SAVE_TIMESTAMPS)% DATA_SECTION_ALIGNMENT));
        if (DataAligner == DATA_SECTION_ALIGNMENT) {
                DataAligner = 0U;
        }

        if (ControlCodeInst->ControlCodefp != NULL) {

                if (!ControlCodeInst->IsJobOpen) {
                        _XAie_StartNewJob(ControlCodeInst);
                }

                if((ControlCodeInst->UcPageSize + ISA_OPSIZE_SAVE_TIMESTAMPS +
                        DataAligner) > ControlCodeInst->PageSizeMax) {
                        _XAie_StartNewPage(ControlCodeInst);
                        _XAie_StartNewJob(ControlCodeInst);
                }

                fprintf(ControlCodeInst->ControlCodefp, "SAVE_TIMESTAMPS\t %d\n",
                                Timestamp );
                ControlCodeInst->CombineCommands = 0;
                ControlCodeInst->UcPageSize += ISA_OPSIZE_SAVE_TIMESTAMPS;
                ControlCodeInst->UcPageTextSize += ISA_OPSIZE_SAVE_TIMESTAMPS;

                return XAIE_OK;
        }
        else
                return XAIE_ERR;
}

/*****************************************************************************/
/**
*
* This is the function to write 32 bit value to NPI register address.
*
* @param	IOInst: IO instance pointer
* @param	RegOff: NPI register offset
* @param	RegVal: Value to write to register
*
* @return	None.
*
* @note		None.
* @note		Internal only.
*
*******************************************************************************/
static void _XAie_ControlCodeIO_NpiWrite32(void *IOInst, u32 RegOff, u32 RegVal)
{
	(void)IOInst;
	(void)RegOff;
	(void)RegVal;

	return;
}

/*****************************************************************************/
/**
*
* This is the memory IO function to mask poll a NPI address for a value.
*
* @param	IOInst: IO instance pointer
* @param	RegOff: Register offset to read from.
* @param	Mask: Mask to be applied to Data.
* @param	Value: 32-bit value to poll for
* @param	TimeOutUs: Timeout in micro seconds.
*
* @return	XAIE_OK or XAIE_ERR.
*
* @note		None.
* @note		Internal only.
*
*******************************************************************************/
static AieRC _XAie_ControlCodeIO_NpiMaskPoll(void *IOInst, u64 RegOff, u32 Mask,
		u32 Value, u32 TimeOutUs)
{
	(void)IOInst;
	(void)RegOff;
	(void)Mask;
	(void)Value;
	(void)TimeOutUs;

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This is the function to run backend operations
*
* @param	IOInst: IO instance pointer
* @param	DevInst: AI engine partition device instance
* @param	Op: Backend operation code
* @param	Arg: Backend operation argument
*
* @return	XAIE_OK for success and error code for failure.
*
* @note		Internal only.
*
*******************************************************************************/
static AieRC XAie_ControlCodeIO_RunOp(void *IOInst, XAie_DevInst *DevInst,
		     XAie_BackendOpCode Op, void *Arg)
{
	AieRC RC = XAIE_OK;
	XAie_ControlCodeIO  *ControlCodeInst = (XAie_ControlCodeIO *)IOInst;

	switch(Op) {
		case XAIE_BACKEND_OP_NPIWR32:
		{
			XAie_BackendNpiWrReq *Req = Arg;

			_XAie_ControlCodeIO_NpiWrite32(IOInst, Req->NpiRegOff,
					Req->Val);
			break;
		}
		case XAIE_BACKEND_OP_NPIMASKPOLL32:
		{
			XAie_BackendNpiMaskPollReq *Req = Arg;

			return _XAie_ControlCodeIO_NpiMaskPoll(IOInst, Req->NpiRegOff,
					Req->Mask, Req->Val, Req->TimeOutUs);
		}
		case XAIE_BACKEND_OP_ASSERT_SHIMRST:
		{
			u8 RstEnable = (u8)((uintptr_t)Arg & 0xFF);

			_XAie_NpiSetShimReset(DevInst, RstEnable);
			break;
		}
		case XAIE_BACKEND_OP_SET_PROTREG:
		{
			RC = _XAie_NpiSetProtectedRegEnable(DevInst, Arg);
			break;
		}
		case XAIE_BACKEND_OP_CONFIG_SHIMDMABD:
		{
			is_shim_bd = 1;
			XAie_ShimDmaBdArgs *BdArgs =
				(XAie_ShimDmaBdArgs *)Arg;

			XAie_ControlCodeIO_BlockWrite32(IOInst, BdArgs->Addr,
				BdArgs->BdWords, BdArgs->NumBdWords);
			is_shim_bd = 0;
			break;
		}
		case XAIE_BACKEND_OP_REQUEST_TILES:
			return _XAie_PrivilegeRequestTiles(DevInst,
					(XAie_BackendTilesArray *)Arg);
		case XAIE_BACKEND_OP_PARTITION_INITIALIZE:
			return _XAie_PrivilegeInitPart(DevInst,
					(XAie_PartInitOpts *)Arg);
		case XAIE_BACKEND_OP_PARTITION_TEARDOWN:
			return _XAie_PrivilegeTeardownPart(DevInst);
		case XAIE_BACKEND_OP_UPDATE_NPI_ADDR:
		{
			XAie_ControlCodeIO *ControlCodeIOInst = (XAie_ControlCodeIO *)IOInst;
			ControlCodeIOInst->NpiBaseAddr = *((u64 *)Arg);
			break;
		}
		default:
			XAIE_ERROR("CDO backend doesn't support operation"
					" %u.\n", Op);
			RC = XAIE_FEATURE_NOT_SUPPORTED;
			break;
	}

	return RC;
}

/*****************************************************************************/
/**
*
* This is the function used to start capturing the control code
* This is usefull only in control code backend. In other backend calling this
* makes no sense.
*
*  @return    1 on failure.
*
*
******************************************************************************/
AieRC XAie_OpenControlCodeFile(XAie_DevInst *DevInst, const char *FileName, u32 PageSize)
{
	XAie_ControlCodeIO  *ControlCodeInst = (XAie_ControlCodeIO *)DevInst->IOInst;

	if(DevInst->Backend->Type != XAIE_IO_BACKEND_CONTROLCODE) {
		XAIE_ERROR("This is supported only in Controlcode Backend %d \n", DevInst->Backend->Type);
		return XAIE_INVALID_BACKEND;
	}

	ControlCodeInst->UcbdLabelNum 	= 0;
	ControlCodeInst->UcbdDataNum 	= 0;
	ControlCodeInst->UcDmaDataNum 	= 0;
	ControlCodeInst->UcJobNum 	= 0;
	ControlCodeInst->UcPageSize 	= 0;
	ControlCodeInst->UcPageTextSize	= 0;
	ControlCodeInst->IsJobOpen  	= 0;
	ControlCodeInst->IsPageOpen  	= 0;
	ControlCodeInst->ControlCodefp      = fopen(FileName, "w");
	ControlCodeInst->ControlCodedatafp  = fopen(TEMP_ASM_FILE1, "w+");
	ControlCodeInst->ControlCodedata2fp = fopen(TEMP_ASM_FILE2, "w+");
	ControlCodeInst->ControlCodedata3fp = fopen(TEMP_ASM_FILE3, "w+");

	ControlCodeInst->PageSizeMax = PageSize;
	ControlCodeInst->PageSizeMax = 8192; //TBD this line to be removed
	ControlCodeInst->CombineCommands = 0;

	if (ControlCodeInst->ControlCodefp == NULL ||
		ControlCodeInst->ControlCodedatafp == NULL ||
		ControlCodeInst->ControlCodedata2fp == NULL ||
		ControlCodeInst->ControlCodedata3fp == NULL) {

		if(ControlCodeInst->ControlCodefp) {
			fclose(ControlCodeInst->ControlCodefp);
		}
		if (ControlCodeInst->ControlCodedatafp) {
			fclose(ControlCodeInst->ControlCodedatafp);
		}
		if (ControlCodeInst->ControlCodedata2fp) {
			fclose(ControlCodeInst->ControlCodedata2fp);
		}
		if (ControlCodeInst->ControlCodedata3fp) {
			fclose(ControlCodeInst->ControlCodedata3fp);
		}
		//printf("File could not be opened, fopen Error: %s\n", strerror(errno));
		return XAIE_ERR;
	}
	printf("Generating: %s\n", FileName);
	fprintf(ControlCodeInst->ControlCodefp, ";\n");
	fprintf(ControlCodeInst->ControlCodefp, ";text\n");
	fprintf(ControlCodeInst->ControlCodefp, ";\n");
	ControlCodeInst->UcPageTextSize += ISA_OPSIZE_EOF;
	ControlCodeInst->UcPageSize += PAGE_HEADER_SIZE + ISA_OPSIZE_EOF;
	ControlCodeInst->IsPageOpen = 1;

	fprintf(ControlCodeInst->ControlCodedatafp, ";\n");
	fprintf(ControlCodeInst->ControlCodedatafp, ";data\n");
	fprintf(ControlCodeInst->ControlCodedatafp, ";\n");
	fprintf(ControlCodeInst->ControlCodedatafp, ".align    16\n");
	fprintf(ControlCodeInst->ControlCodedata2fp, ".align    4\n");

	return XAIE_OK;
}

/*****************************************************************************/
/**
* This function ends the current job and starts the new job
* @param DevInst AI engine device instance pointer
*
* @return  0 on success.
*
******************************************************************************/
AieRC XAie_StartNewJob(XAie_DevInst *DevInst)
{
	XAie_ControlCodeIO  *ControlCodeInst = (XAie_ControlCodeIO *)DevInst->IOInst;

	if (ControlCodeInst->ControlCodefp != NULL) {
		_XAie_StartNewJob(ControlCodeInst);
		return XAIE_OK;
	}

	return XAIE_ERR;
}

/*****************************************************************************/
/**
* This function ends the current job
* @param DevInst AI engine device instance pointer
*
* @return  0 on success.
*
******************************************************************************/
AieRC XAie_EndJob(XAie_DevInst *DevInst) {
	XAie_ControlCodeIO  *ControlCodeInst = (XAie_ControlCodeIO *)DevInst->IOInst;

	if (ControlCodeInst->ControlCodefp != NULL) {
		_XAie_EndJob(ControlCodeInst);
		return XAIE_OK;
	}

	return XAIE_ERR;
}

/*****************************************************************************/

/**
* This function ends the current page
* @param DevInst AI engine device instance pointer
*
* @return  0 on success.
*
******************************************************************************/
AieRC XAie_EndPage(XAie_DevInst *DevInst) {

	XAie_ControlCodeIO  *ControlCodeInst = (XAie_ControlCodeIO *)DevInst->IOInst;

	if (ControlCodeInst->ControlCodefp != NULL) {
	        _XAie_EndPage(ControlCodeInst);
		return XAIE_OK;
	}

	return XAIE_ERR;
}

/*****************************************************************************/
/**
* Merges the given text file.
* @param	SrcFp: file pointer of Source File.
* @param	DesFile: file pointer of Destination File.
*
* @note		Internal API only.
*
******************************************************************************/
static void _XAie_MegreFiles(FILE *SrcFp, FILE *DesFp) {
	char TempBuf;

	if (!SrcFp || !DesFp) {
		printf("Files not opened\n");
		return;
	}

	fseek(SrcFp, 0, SEEK_SET);
	fseek(DesFp, 0, SEEK_END);

	while ((TempBuf = fgetc(SrcFp)) != EOF) {
		fputc(TempBuf, DesFp);
	}

}

/*****************************************************************************/
/**
* This function used to stop the control code capture.
* This also merges the temp files and updates the control code file.
*
******************************************************************************/
void XAie_CloseControlCodeFile(XAie_DevInst *DevInst) {
	XAie_ControlCodeIO  *ControlCodeInst = (XAie_ControlCodeIO *)DevInst->IOInst;

	if (ControlCodeInst->ControlCodefp != NULL) {
		_XAie_EndPage(ControlCodeInst);
		fprintf(ControlCodeInst->ControlCodefp, "EOF\n\n");

		_XAie_MegreFiles(ControlCodeInst->ControlCodedatafp, ControlCodeInst->ControlCodefp);
		_XAie_MegreFiles(ControlCodeInst->ControlCodedata2fp, ControlCodeInst->ControlCodefp);
		_XAie_MegreFiles(ControlCodeInst->ControlCodedata3fp, ControlCodeInst->ControlCodefp);

		fclose(ControlCodeInst->ControlCodefp);
		fclose(ControlCodeInst->ControlCodedatafp);
		fclose(ControlCodeInst->ControlCodedata2fp);
		fclose(ControlCodeInst->ControlCodedata3fp);

		remove(TEMP_ASM_FILE1);
		remove(TEMP_ASM_FILE2);
		remove(TEMP_ASM_FILE3);

		ControlCodeInst->ControlCodefp		= NULL;
		ControlCodeInst->ControlCodedatafp	= NULL;
		ControlCodeInst->ControlCodedata2fp	= NULL;
		ControlCodeInst->ControlCodedata3fp	= NULL;
	}
}

/*****************************************************************************/
/**
*
* This is the function to generate code for wait for task complete token.
*
* @param	DevInst: Device instance pointer
* @param	Column: Column number.
* @param	Row: Row number.Mask to be applied to Data.
* @param	Channel: Channel number.
* @param	NumTokens: Number of tokens to wait for completion.

* @return	0 on success.
*
*
*******************************************************************************/
AieRC XAie_WaitTaskCompleteToken(XAie_DevInst *DevInst,
	uint16_t Column, uint16_t Row, uint32_t Channel, uint8_t NumTokens)
{
	if (DevInst->Backend->Type != XAIE_IO_BACKEND_CONTROLCODE) {
		XAIE_ERROR("This is supported only in Controlcode Backend %d \n", DevInst->Backend->Type);
		return XAIE_INVALID_BACKEND;
	}

	XAie_ControlCodeIO  *ControlCodeInst = (XAie_ControlCodeIO *)DevInst->IOInst;
	uint32_t TileId;
	u32 DataAligner = (DATA_SECTION_ALIGNMENT -
		((ControlCodeInst->UcPageTextSize + ISA_OPSIZE_WAIT_TCTS) % DATA_SECTION_ALIGNMENT));
	if (DataAligner == DATA_SECTION_ALIGNMENT) {
		DataAligner = 0U;
	}

	if (ControlCodeInst->ControlCodefp != NULL) {

		if (!ControlCodeInst->IsJobOpen) {
			_XAie_StartNewJob(ControlCodeInst);
		}

		if((ControlCodeInst->UcPageSize + ISA_OPSIZE_WAIT_TCTS +
			DataAligner) > ControlCodeInst->PageSizeMax) {
			_XAie_StartNewPage(ControlCodeInst);
			_XAie_StartNewJob(ControlCodeInst);
		}

		TileId = ((Column << 5) | Row);
		fprintf(ControlCodeInst->ControlCodefp, "WAIT_TCTS\t 0x%x, 0x%x, 0x%x\n",
				TileId, Channel, NumTokens );
		ControlCodeInst->CombineCommands = 0;
		ControlCodeInst->UcPageSize += ISA_OPSIZE_WAIT_TCTS;
		ControlCodeInst->UcPageTextSize += ISA_OPSIZE_WAIT_TCTS;
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This is the function to add annotation in asm code.
*
* @param        DevInst: Device instance pointer
* @param        Id: Serves as a key to a dynamic tracing probe.
* @param        Name: Serves as a key to look up interested annotations.
* @param        Description: Serves as comments.

* @return       XAIE_OK or XAIE_ERR.
*
*
*******************************************************************************/
AieRC XAie_ControlCodeAddAnnotation(XAie_DevInst *DevInst,
               u32 Id, const char *Name, const char *Description)
{
        if(DevInst->Backend->Type != XAIE_IO_BACKEND_CONTROLCODE) {
                XAIE_ERROR("This is supported only in Controlcode Backend %d \n", DevInst->Backend->Type);
                return XAIE_INVALID_BACKEND;
        }

        XAie_ControlCodeIO  *ControlCodeInst = (XAie_ControlCodeIO *)DevInst->IOInst;

        if (ControlCodeInst->ControlCodefp != NULL) {
                fprintf(ControlCodeInst->ControlCodefp,".section annotation\n");
                fprintf(ControlCodeInst->ControlCodefp,"id: %d\n", Id);
                fprintf(ControlCodeInst->ControlCodefp,"name: %s\n", Name);
                fprintf(ControlCodeInst->ControlCodefp,"description: %s\n", Description);
                return XAIE_OK;
        }
        else
                return XAIE_ERR;
}

/*****************************************************************************/
/**
*
* This function is used to set scrachpad string.
*
* @param        DevInst: Device instance pointer.
* @param        Scrachpad: It is the name of a declared scratchpad buffer
* @return       XAIE_OK or XAIE_ERR.
*
*
*******************************************************************************/
AieRC XAie_ControlCodeSetScrachPad(XAie_DevInst *DevInst, const char *Scrachpad)
{
        XAie_ControlCodeIO  *ControlCodeInst = (XAie_ControlCodeIO *)DevInst->IOInst;

        ControlCodeInst->ScrachpadName = (char *)calloc(strlen(Scrachpad) + 1, sizeof(char));
        if(ControlCodeInst->ScrachpadName == NULL)
                return XAIE_ERR;
        else {
                strncpy(ControlCodeInst->ScrachpadName, Scrachpad, strlen(Scrachpad));
                return XAIE_OK;
        }
}

#else

AieRC XAie_OpenControlCodeFile(XAie_DevInst *DevInst, const char *FileName, u32 PageSize)
{
	/* no-op */
	(void)DevInst;
	(void)FileName;
	(void)PageSize;
	XAIE_ERROR("Driver is not compiled with ControlCode generation "
			"backend (__AIECONTROLCODE__)\n");
	return XAIE_INVALID_BACKEND;
}

AieRC XAie_ControlCodeAddAnnotation(XAie_DevInst *DevInst,
               u32 Id, const char *Name, const char *Description)
{
        (void)DevInst;
        (void)Id;
        (void)Name;
        (void)Description;

        XAIE_ERROR("Driver is not compiled with ControlCode generation "
                        "backend (__AIECONTROLCODE__)\n");
       return XAIE_INVALID_BACKEND;
}

AieRC XAie_StartNewJob(XAie_DevInst *DevInst)
{
	/* no-op */
	(void)DevInst;
	XAIE_ERROR("Driver is not compiled with ControlCode generation "
			"backend (__AIECONTROLCODE__)\n");
	return XAIE_INVALID_BACKEND;
}

AieRC XAie_EndJob(XAie_DevInst *DevInst)
{
	/* no-op */
	(void)DevInst;
	XAIE_ERROR("Driver is not compiled with ControlCode generation "
			"backend (__AIECONTROLCODE__)\n");
	return XAIE_INVALID_BACKEND;
}

AieRC XAie_EndPage(XAie_DevInst *DevInst)
{
	/* no-op */
	(void)DevInst;
	XAIE_ERROR("Driver is not compiled with ControlCode generation "
			"backend (__AIECONTROLCODE__)\n");
	return XAIE_INVALID_BACKEND;
}

void XAie_CloseControlCodeFile(XAie_DevInst *DevInst)
{
	/* no-op */
	(void)DevInst;
	XAIE_ERROR("Driver is not compiled with ControlCode generation "
			"backend (__AIECONTROLCODE__)\n");
	return;
}

AieRC XAie_WaitTaskCompleteToken(XAie_DevInst *DevInst, uint16_t Column, uint16_t Row, uint32_t Channel, uint8_t NumTokens)
{
	/* no-op */
	(void)DevInst;
	(void)Column;
	(void)Row;
	(void)Channel;
	(void)NumTokens;
	XAIE_ERROR("Driver is not compiled with ControlCode generation "
			"backend (__AIECONTROLCODE__)\n");
	return XAIE_INVALID_BACKEND;
}

static AieRC XAie_ControlCodeIO_Finish(void *IOInst)
{
	/* no-op */
	(void)IOInst;
	return XAIE_OK;
}

static AieRC XAie_ControlCodeIO_Init(XAie_DevInst *DevInst)
{
	/* no-op */
	(void)DevInst;
	XAIE_ERROR("Driver is not compiled with ControlCode generation "
			"backend (__AIECONTROLCODE__)\n");
	return XAIE_INVALID_BACKEND;
}

static AieRC XAie_ControlCodeIO_Write32(void *IOInst, u64 RegOff, u32 Value)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Value;

	return XAIE_ERR;
}

static AieRC XAie_ControlCodeIO_Read32(void *IOInst, u64 RegOff, u32 *Data)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Data;
	return 0;
}

static AieRC XAie_ControlCodeIO_MaskWrite32(void *IOInst, u64 RegOff, u32 Mask,
		u32 Value)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Mask;
	(void)Value;

	return XAIE_ERR;
}

static AieRC XAie_ControlCodeIO_MaskPoll(void *IOInst, u64 RegOff, u32 Mask, u32 Value,
		u32 TimeOutUs)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Mask;
	(void)Value;
	(void)TimeOutUs;

	return XAIE_ERR;
}

static AieRC XAie_ControlCodeIO_BlockWrite32(void *IOInst, u64 RegOff, const u32 *Data,
		u32 Size)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Data;
	(void)Size;

	return XAIE_ERR;
}

static AieRC XAie_ControlCodeIO_BlockSet32(void *IOInst, u64 RegOff, u32 Data, u32 Size)
{
	/* no-op */
	(void)IOInst;
	(void)RegOff;
	(void)Data;
	(void)Size;

	return XAIE_ERR;
}

static AieRC XAie_ControlCodeIO_RunOp(void *IOInst, XAie_DevInst *DevInst,
		     XAie_BackendOpCode Op, void *Arg)
{
	(void)IOInst;
	(void)DevInst;
	(void)Op;
	(void)Arg;
	return XAIE_FEATURE_NOT_SUPPORTED;
}

static AieRC XAie_ControlCodeIO_AddressPatching(void *IOInst, u32 Arg_Offset, u8 Num_BDs)
{
	/* no-op */
	(void)IOInst;
	(void)Arg_Offset;
	(void)Num_BDs;
	XAIE_ERROR("Driver is not compiled with ControlCode generation "
			"backend (__AIECONTROLCODE__)\n");
	return XAIE_INVALID_BACKEND;
}

AieRC XAie_ControlCodeSaveTimestamp(XAie_DevInst *DevInst, u32 Timestamp)
{
        (void)DevInst;
        (void)Timestamp;
        XAIE_ERROR("Driver is not compiled with ControlCode generation "
                        "backend (__AIECONTROLCODE__)\n");
        return XAIE_INVALID_BACKEND;
}

AieRC XAie_ControlCodeSetScrachPad(XAie_DevInst *DevInst, const char *Scrachpad)
{
        (void)DevInst;
        (void)Scrachpad;
        XAIE_ERROR("Driver is not compiled with ControlCode generation "
                        "backend (__AIECONTROLCODE__)\n");
        return XAIE_INVALID_BACKEND;
}

#endif /* __AIECONTROLCODE__ */

static AieRC XAie_ControlCodeIO_CmdWrite(void *IOInst, u8 Col, u8 Row, u8 Command,
		u32 CmdWd0, u32 CmdWd1, const char *CmdStr)
{
	/* no-op */
	(void)IOInst;
	(void)Col;
	(void)Row;
	(void)Command;
	(void)CmdWd0;
	(void)CmdWd1;
	(void)CmdStr;

	return XAIE_OK;
}

static XAie_MemInst* XAie_ControlCodeMemAllocate(XAie_DevInst *DevInst, u64 Size,
		XAie_MemCacheProp Cache)
{
	(void)DevInst;
	(void)Size;
	(void)Cache;
	return NULL;
}

static AieRC XAie_ControlCodeMemFree(XAie_MemInst *MemInst)
{
	(void)MemInst;
	return XAIE_ERR;
}

static AieRC XAie_ControlCodeMemSyncForCPU(XAie_MemInst *MemInst)
{
	(void)MemInst;
	return XAIE_ERR;
}

static AieRC XAie_ControlCodeMemSyncForDev(XAie_MemInst *MemInst)
{
	(void)MemInst;
	return XAIE_ERR;
}

static AieRC XAie_ControlCodeMemAttach(XAie_MemInst *MemInst, u64 MemHandle)
{
	(void)MemInst;
	(void)MemHandle;
	return XAIE_ERR;
}

static AieRC XAie_ControlCodeMemDetach(XAie_MemInst *MemInst)
{
	(void)MemInst;
	return XAIE_ERR;
}

const XAie_Backend ControlCodeBackend =
{
	.Type = XAIE_IO_BACKEND_CONTROLCODE,
	.Ops.Init = XAie_ControlCodeIO_Init,
	.Ops.Finish = XAie_ControlCodeIO_Finish,
	.Ops.Write32 = XAie_ControlCodeIO_Write32,
	.Ops.Read32 = XAie_ControlCodeIO_Read32,
	.Ops.MaskWrite32 = XAie_ControlCodeIO_MaskWrite32,
	.Ops.MaskPoll = XAie_ControlCodeIO_MaskPoll,
	.Ops.BlockWrite32 = XAie_ControlCodeIO_BlockWrite32,
	.Ops.BlockSet32 = XAie_ControlCodeIO_BlockSet32,
	.Ops.CmdWrite = XAie_ControlCodeIO_CmdWrite,
	.Ops.RunOp = XAie_ControlCodeIO_RunOp,
	.Ops.AddressPatching = XAie_ControlCodeIO_AddressPatching,
	.Ops.MemAllocate = XAie_ControlCodeMemAllocate,
	.Ops.MemFree = XAie_ControlCodeMemFree,
	.Ops.MemSyncForCPU = XAie_ControlCodeMemSyncForCPU,
	.Ops.MemSyncForDev = XAie_ControlCodeMemSyncForDev,
	.Ops.MemAttach = XAie_ControlCodeMemAttach,
	.Ops.MemDetach = XAie_ControlCodeMemDetach,
	.Ops.GetTid = XAie_IODummyGetTid,
	.Ops.SubmitTxn = NULL,
};

/** @} */
