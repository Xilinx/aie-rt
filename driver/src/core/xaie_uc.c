/******************************************************************************
* Copyright (C) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_uc.c
* @{
*
* The file has implementations of routines for uC loading.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Kishan  12/23/2022  Initial creation
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include <errno.h>

#include "xaie_uc.h"
#include "xaie_elfloader.h"
#include "xaie_core_aie.h"
#include "xaie_feature_config.h"
#include "xaie_ecc.h"
#include "xaie_mem.h"

#ifdef XAIE_FEATURE_UC_ENABLE
/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This routine is used to write the loadable sections of the elf belonging to
* the program memory of ai engines.
*
* @param	DevInst: Device Instance.
* @param	Loc: Starting location of the section.
* @param	SectionPtr: Pointer to the program section entry in the ELF
*		buffer.
* @param	Phdr: Pointer to the program header.
*
* @return	XAIE_OK on success and error code for failure.
*
* @note		Internal API only.
*
*******************************************************************************/
static AieRC _XAie_LoadProgMemSection(XAie_DevInst *DevInst, XAie_LocType Loc,
		const unsigned char *SectionPtr, const Elf32_Phdr *Phdr)
{
	u64 Addr;
	const XAie_UcMod *UcMod;

	UcMod = DevInst->DevProp.DevMod[XAIEGBL_TILE_TYPE_SHIMNOC].UcMod;

	/* Write to Program Memory */
	if((Phdr->p_paddr + Phdr->p_memsz) > UcMod->ProgMemSize) {
		XAIE_ERROR("Overflow of program memory\n");
		return XAIE_INVALID_ELF;
	}

	Addr = (u64)(UcMod->ProgMemHostOffset + Phdr->p_paddr) +
		XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	/*
	 * The program memory sections in the elf can end at 32bit
	 * unaligned addresses. To factor this, we round up the number
	 * of 32-bit words that has to be written to the program
	 * memory. Since the elf has footers at the end, accessing
	 * memory out of Progsec will not result in a segmentation
	 * fault.
	 */
	return XAie_BlockWrite32(DevInst, Addr, (u32 *)SectionPtr,
			(Phdr->p_memsz + 4U - 1U) / 4U);
}

/*****************************************************************************/
/**
*
* This routine is used to write the loadable sections of the elf belonging to
* the data memory of ai engines.
*
* @param	DevInst: Device Instance.
* @param	Loc: Starting location of the section.
* @param	SectionPtr: Pointer to the program section entry in the ELF
*		buffer.
* @param	Phdr: Pointer to the program header.
*
* @return	XAIE_OK on success and error code for failure.
*
* @note		Internal API only.
*
*******************************************************************************/
static AieRC _XAie_LoadDataMemSection(XAie_DevInst *DevInst, XAie_LocType Loc,
		const unsigned char *SectionPtr, const Elf32_Phdr *Phdr)
{
	AieRC RC;
	u64 Addr;
	u32 SectionAddr;
	u32 SectionSize;
	u32 BytesToWrite;
	u32 OverFlowBytes;
	const XAie_UcMod *UcMod;
	unsigned char *Tmp = XAIE_NULL;
	u32 AddrMask, MemSize;
	u8 Inv = 1U, MemType;
	const unsigned char *Buffer = SectionPtr;
	AieRC (*BlockWrite)(XAie_DevInst *DevInst, XAie_LocType Loc,
		u32 Addr, const void *Src, u32 Size);

	UcMod = DevInst->DevProp.DevMod[XAIEGBL_TILE_TYPE_SHIMNOC].UcMod;

	/*
	 * Check if section can access out of bound memory location on device
	 * and to identify the type of data memory from the address in the elf.
	 * Since the elf is created from the uc core view, mapping is needed to
	 * convert UcMod->PrivDataMemAddr from host view to uc core view.
	 */
	if(Phdr->p_paddr >= UcMod->ProgMemSize) {
		if((Phdr->p_paddr >= (UcMod->PrivDataMemAddr -
		   UcMod->ProgMemHostOffset)) && ((Phdr->p_paddr + Phdr->p_memsz)
		   < (UcMod->PrivDataMemAddr - UcMod->ProgMemHostOffset
		   + UcMod->PrivDataMemSize))) {
			Inv = 0;
			MemType = XAIE_PRIVATE_DATA_MEMORY;
		} else if ((Phdr->p_paddr >=
			 UcMod->DataMemUcOffset) && ((Phdr->p_paddr + Phdr->p_memsz) <
			 (UcMod->DataMemUcOffset + UcMod->DataMemSize))) {
			Inv = 0;
			MemType = XAIE_MODULE_DATA_MEMORY;
		}
	}

	if(Inv) {
		XAIE_ERROR("Invalid section starting at 0x%x\n", Phdr->p_paddr);
		return XAIE_INVALID_ELF;
	}

	/* Write initialized section to data memory */
	SectionSize = Phdr->p_memsz;
	SectionAddr = Phdr->p_paddr;

	/* Check if file size is 0. If yes, allocate memory and init to 0 */
	if(Phdr->p_filesz == 0U) {
		Buffer = (const unsigned char *)calloc(Phdr->p_memsz,
				sizeof(char));
		if(Buffer == XAIE_NULL) {
			XAIE_ERROR("Memory allocation failed for buffer\n");
			return XAIE_ERR;
		}
		/* Copy pointer to free allocated memory in case of error. */
		Tmp = (unsigned char *)Buffer;
	}

	if(MemType == XAIE_PRIVATE_DATA_MEMORY) {
		AddrMask = UcMod->PrivDataMemSize - 1;
		MemSize = UcMod->PrivDataMemSize;
		BlockWrite = XAie_DataMemBlockWrite;

	} else if(MemType == XAIE_MODULE_DATA_MEMORY) {
		AddrMask = UcMod->DataMemSize - 1;
		MemSize = UcMod->DataMemSize;
		BlockWrite = XAie_SharedDataMemBlockWrite;
	}

	while(SectionSize > 0U) {
		/*Bytes to write in this section */
		OverFlowBytes = 0U;
		if((SectionAddr & AddrMask) + SectionSize > MemSize) {
			OverFlowBytes = (SectionAddr & AddrMask) + SectionSize -
					 MemSize;
		}

		BytesToWrite = SectionSize - OverFlowBytes;
		Addr = (u64)(SectionAddr & AddrMask);

		/* Since depending on the type of data memory written the API
		 * used varies a function pointer BlockWrite is used to vary
		 * the API at runtime.
		 */
		RC = (*BlockWrite)(DevInst, Loc, (u32)Addr,
				(const void*)Buffer, BytesToWrite);
		if(RC != XAIE_OK) {
			XAIE_ERROR("Write to data memory failed. Col: %u Row: %u, BytesToWrite: %lu\n"\
					,Loc.Col, Loc.Row, BytesToWrite);
			if(Phdr->p_filesz == 0U) {
				free(Tmp);
			}
			return RC;
		}

		SectionSize -= BytesToWrite;
		SectionAddr += BytesToWrite;
		Buffer += BytesToWrite;
	}

	if(Phdr->p_filesz == 0U) {
		free(Tmp);
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This routine is used to write to the specified program section by reading the
* corresponding data from the ELF buffer.
*
* @param	DevInst: Device Instance.
* @param	Loc: Starting location of the section.
* @param	ProgSec: Pointer to the program section entry in the ELF buffer.
* @param	Phdr: Pointer to the program header.
*
* @return	XAIE_OK on success and error code for failure.
*
* @note		Internal API only.
*
*******************************************************************************/
static AieRC _XAie_WriteProgramSection(XAie_DevInst *DevInst, XAie_LocType Loc,
		const unsigned char *ProgSec, const Elf32_Phdr *Phdr)
{
	const XAie_UcMod *UcMod;

	UcMod = DevInst->DevProp.DevMod[XAIEGBL_TILE_TYPE_SHIMNOC].UcMod;

	/* Write to Program Memory */
	if(Phdr->p_paddr < UcMod->ProgMemSize) {
		return _XAie_LoadProgMemSection(DevInst, Loc, ProgSec, Phdr);
	}

	if((Phdr->p_filesz == 0U) || (Phdr->p_filesz != 0U)) {
		return _XAie_LoadDataMemSection(DevInst, Loc, ProgSec, Phdr);
	} else  {
		XAIE_WARN("Mismatch in program header to data memory loadable section. Skipping this program section.\n");
		return XAIE_OK;
	}
}

/*****************************************************************************/
/**
*
* This helper function loads the elf from memory to the uC.
*
* @param	DevInst: Device Instance.
* @param	Loc: Location of AIE Tile.
* @param	ElfMem: Pointer to the Elf contents in memory.
*
* @return	XAIE_OK on success and error code for failure.
*
* @note		Internal Only.
*
*******************************************************************************/
static AieRC _XAie_LoadElfFromMem(XAie_DevInst *DevInst, XAie_LocType Loc,
		const unsigned char* ElfMem)
{
	AieRC RC;
	const Elf32_Ehdr *Ehdr;
	const Elf32_Phdr *Phdr;
	const unsigned char *SectionPtr;

	Ehdr = (const Elf32_Ehdr *) ElfMem;
	_XAie_PrintElfHdr(Ehdr);

	for(u32 phnum = 0U; phnum < Ehdr->e_phnum; phnum++) {
		Phdr = (Elf32_Phdr*) (ElfMem + sizeof(*Ehdr) +
				phnum * sizeof(*Phdr));
		_XAie_PrintProgSectHdr(Phdr);
		if(Phdr->p_type == (u32)PT_LOAD) {
			SectionPtr = ElfMem + Phdr->p_offset;
			RC = _XAie_WriteProgramSection(DevInst, Loc, SectionPtr, Phdr);
			if(RC != XAIE_OK) {
				return RC;
			}
		}
	}

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This function loads the elf from memory to the uC. The function writes
* 0 for the uninitialized data section.
*
* @param	DevInst: Device Instance.
* @param	Loc: Location of AIE Tile.
* @param	ElfMem: Pointer to the Elf contents in memory.
*
* @return	XAIE_OK on success and error code for failure.
*
* @note		None.
*
*******************************************************************************/
AieRC XAie_LoadUcMem(XAie_DevInst *DevInst, XAie_LocType Loc,
		const unsigned char* ElfMem)
{
	u8 TileType;

	if((DevInst == XAIE_NULL) || (ElfMem == XAIE_NULL) ||
		(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid arguments\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType != XAIEGBL_TILE_TYPE_SHIMNOC) {
		XAIE_ERROR("Invalid Tile Type, Col:%u Row:%u TileType:%u\n", Loc.Col, Loc.Row, TileType);
		return XAIE_INVALID_TILE;
	}

	if (ElfMem == XAIE_NULL) {
		XAIE_ERROR("Invalid ElfMem\n");
		return XAIE_INVALID_ARGS;
	}

	return _XAie_LoadElfFromMem(DevInst, Loc, ElfMem);
}

/*****************************************************************************/
/**
*
* This function loads the elf from file to the uC. The function writes
* 0 for the unitialized data section.
*
* @param	DevInst: Device Instance.
* @param	Loc: Location of AIE Tile.
* @param	ElfPtr: Path to the elf file.
*
* @return	XAIE_OK on success and error code for failure.
*
* @note		None.
*
*******************************************************************************/
AieRC XAie_LoadUc(XAie_DevInst *DevInst, XAie_LocType Loc, const char *ElfPtr)
{
	u8 TileType;
	FILE *Fd;
	int Ret;
	unsigned char *ElfMem;
	u64 ElfSz;
	AieRC RC;


	if((DevInst == XAIE_NULL) ||
		(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAIE_ERROR("Invalid device instance\n");
		return XAIE_INVALID_ARGS;
	}

	TileType = DevInst->DevOps->GetTTypefromLoc(DevInst, Loc);
	if(TileType != XAIEGBL_TILE_TYPE_SHIMNOC) {
		XAIE_ERROR("Invalid Tile Type, Col:%u Row:%u TileType:%u\n", Loc.Col, Loc.Row, TileType);
		return XAIE_INVALID_TILE;
	}

	if (ElfPtr == XAIE_NULL) {
		XAIE_ERROR("Invalid ElfPtr\n");
		return XAIE_INVALID_ARGS;
	}

	Fd = fopen(ElfPtr, "r");
	if(Fd == XAIE_NULL) {
		XAIE_ERROR("Unable to open elf file, %d: %s\n",
			errno, strerror(errno));
		return XAIE_INVALID_ELF;
	}

	/* Get the file size of the elf */
	Ret = fseek(Fd, 0L, SEEK_END);
	if(Ret != 0) {
		XAIE_ERROR("Failed to get end of file, %d: %s\n",
			errno, strerror(errno));
		fclose(Fd);
		return XAIE_INVALID_ELF;
	}

	ElfSz = (u64)ftell(Fd);
	rewind(Fd);
	XAIE_DBG("Elf size is %ld bytes\n", ElfSz);

	/* Read entire elf file into memory */
	ElfMem = (unsigned char*) malloc(ElfSz);
	if(ElfMem == NULL) {
		fclose(Fd);
		XAIE_ERROR("Memory allocation failed\n");
		return XAIE_ERR;
	}

	Ret = (int)fread((void*)ElfMem, ElfSz, 1U, Fd);
	if(Ret == 0) {
		fclose(Fd);
		free(ElfMem);
		XAIE_ERROR("Failed to read Elf into memory\n");
		return XAIE_ERR;
	}

	fclose(Fd);

	RC = _XAie_LoadElfFromMem(DevInst, Loc, ElfMem);
	free(ElfMem);

	return RC;
}

/*****************************************************************************/
/*
*
* This API writes to the Core control register of a uC to wakeup the core.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of the Shim tile.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only.
*
******************************************************************************/
AieRC _XAie_UcCoreWakeup(XAie_DevInst *DevInst, XAie_LocType Loc,
		const struct XAie_UcMod *UcMod)
{
	u32 Mask, Value;
	u64 RegAddr;

	Mask = UcMod->CoreCtrl->CtrlWakeup.Mask;
	Value = XAie_SetField(1U, UcMod->CoreCtrl->CtrlWakeup.Lsb, Mask);
	RegAddr = UcMod->CoreCtrl->RegOff +
		XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);
	return XAie_Write32(DevInst, RegAddr, Value);
}

/*****************************************************************************/
/*
*
* This API writes to the Core control register of a uC to sleep the core.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of the Shim tile.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only.
*
******************************************************************************/
AieRC _XAie_UcCoreSleep(XAie_DevInst *DevInst, XAie_LocType Loc,
		const struct XAie_UcMod *UcMod)
{
	u32 Mask, Value;
	u64 RegAddr;

	Mask = UcMod->CoreCtrl->CtrlSleep.Mask;
	Value = XAie_SetField(1U, UcMod->CoreCtrl->CtrlSleep.Lsb, Mask);
	RegAddr = UcMod->CoreCtrl->RegOff +
		XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);

	return XAie_Write32(DevInst, RegAddr, Value);
}

/*****************************************************************************/
/*
*
* This API reads the uC core status register value.
*
* @param	DevInst: Device Instance
* @param	Loc: Location of the AIE tile.
* @param	CoreStatus: Pointer to store the core status register value.
* @param	UcMod: Pointer to the uC module data structure.
*
* @return	XAIE_OK on success, Error code on failure.
*
* @note		Internal only.
*
******************************************************************************/
AieRC _XAie_UcCoreGetStatus(XAie_DevInst *DevInst, XAie_LocType Loc,
		u32 *CoreStatus, const struct XAie_UcMod *UcMod)
{
	AieRC RC;
	u64 RegAddr;
	u32 RegVal;

	/* Read core status register */
	RegAddr = UcMod->CoreSts->RegOff +
		XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col);
	RC = XAie_Read32(DevInst, RegAddr, &RegVal);
	if(RC != XAIE_OK) {
		return RC;
	}

	*CoreStatus = XAie_GetField(RegVal, 0U, UcMod->CoreSts->Mask);

	return XAIE_OK;
}

#endif /* XAIE_FEATURE_UC_ENABLE */
/** @} */
