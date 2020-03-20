/******************************************************************************
*
* Copyright (C) 2019 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/

/*****************************************************************************/
/**
* @file xaie_elfloader.c
* @{
*
* The file has implementations of routines for elf loading.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   09/24/2019  Initial creation
* 1.1   Tejus	01/04/2020  Cleanup error messages
* 1.2   Tejus   03/20/2020  Make internal functions static
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xaie_elfloader.h"

/************************** Constant Definitions *****************************/
#define XAIE_ELF_SECTION_NUMMAX		100
#define XAIE_ELF_SECTION_NAMEMAXLEN		100
/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* This is the routine to derive the stack start and end addresses from the
* specified map file. This function basically looks for the line
* <b><init_address>..<final_address> ( <num> items) : Stack</b> in the
* map file to derive the stack address range.
*
* @param	MapPtr: Path to the Map file.
* @param	StackSzPtr: Pointer to the stack range structure.
*
* @return	XAIESIM_SUCCESS on success, else XAIESIM_FAILURE.
*
* @note		None.
*
*******************************************************************************/
static u32 XAieSim_GetStackRange(u8 *MapPtr, XAieSim_StackSz *StackSzPtr)
{
#ifdef __AIESIM__
	FILE *Fd;
	u8 buffer[200U];

	/*
	 * Read map file and look for line:
	 * <init_address>..<final_address> ( <num> items) : Stack
	 */
	StackSzPtr->start = 0xFFFFFFFFU;
	StackSzPtr->end = 0U;

	Fd = fopen(MapPtr, "r");
	if(Fd == NULL) {
		XAieSim_print("ERROR: Invalid Map file\n");
		return XAIESIM_FAILURE;
	}

	while(fgets(buffer, 200U, Fd) != NULL) {
		if(strstr(buffer, "items) : Stack") != NULL) {
			sscanf(buffer, "    0x%8x..0x%8x (%*s",
					&StackSzPtr->start, &StackSzPtr->end);
			break;
		}
	}

	if(StackSzPtr->start == 0xFFFFFFFFU) {
		return XAIESIM_FAILURE;
	} else {
		return XAIESIM_SUCCESS;
	}
	fclose(Fd);
#endif
}

/*****************************************************************************/
/**
*
* This routine is used to get the actual tile data memory address based on the
* section's loadable address. The actual tile address is derived from the
* cardinal direction the secton's loadable address points to.
*
* @param	DevInst- Pointer to the Device instance structure.
* @param	Loc: Location of Aie Tile.
* @param	ShAddr: Section's loadable address.
*
* @return	64-bit target tile address.
*
* @note		None.
*
*******************************************************************************/
static uint64_t XAie_GetTargetTileAddr(XAie_DevInst *DevInst, XAie_LocType Loc,
		u32 ShAddr)
{
	uint64_t TgtTileAddr;
	u8 CardDir;
	u8 TgtRow;
	u8 TgtCol;
        u8 RowParity;
	u32 AddrMask;

	const XAie_CoreMod *CoreMod;

	CoreMod = DevInst->DevProp.DevMod[XAIEGBL_TILE_TYPE_AIETILE].CoreMod;

	/* Find the cardinal direction and get the tile addr */
	AddrMask = CoreMod->CoreEastAddrStart - CoreMod->CoreSouthAddrStart;
	CardDir = (ShAddr & AddrMask) >> __builtin_ctz(AddrMask);

	TgtRow = Loc.Row;
	TgtCol = Loc.Col;
        RowParity = TgtRow % 2U;
	if(DevInst->DevProp.DevGen == XAIE_DEV_GEN_AIE2) {
		RowParity = 1U;
	}

	switch(CardDir) {
	case 0U:
                /* South */
		if(TgtRow > 0U) {
			TgtRow -= 1U;
		}
		break;
	case 1U:
                /*
                 * West - West I/F could be same tile or adjacent tile based on
                 * the row number
                 */
		if(RowParity == 1U) {
                        /* Adjacent tile */
                        if(TgtCol > 0U) {
			        TgtCol -= 1U;
                        }
		}
		break;
	case 2U:
                /* North */
		TgtRow += 1U;
		break;
	case 3U:
		/*
		 * East - East I/F could be same tile or adjacent tile based on
		 * the row number
		 */
		if(RowParity == 0U) {
                        /* Adjacent tile */
                        TgtCol += 1U;
                }
                break;
	}

        /* Restore orig values if we have exceeded the array boundary limits */
	if(TgtRow > DevInst->NumRows) {
		TgtRow = Loc.Row;
	}
	if(TgtCol >= DevInst->NumCols) {
		TgtCol = Loc.Col;
	}

	TgtTileAddr = DevInst->BaseAddr + _XAie_GetTileAddr(DevInst, TgtRow, TgtCol); 

	return TgtTileAddr;
}

/*****************************************************************************/
/**
*
* This routine is used to write to the specified section by reading the
* corresponding data from the ELF file.
*
* @param	DevInst- Pointer to the Device instance structure.
* @param	Loc: Location of Aie Tile.
* @param        SectName: Name of the section.
* @param        SectPtr: Poiner to the section entry in the ELF file.
* @param	Fd: Pointer to the ELF file.
*
* @return	XAIE_OK on success, else error code.
*
* @note		None.
*
*******************************************************************************/
void XAie_WriteElfSection(XAie_DevInst *DevInst, XAie_LocType Loc, u8 *SectName,
						Elf32_Shdr *SectPtr, FILE *Fd)
{

	u32 *DataPtr;
	u32 *CurrPtr;
	u32 NumBytesRead;
	u32 NumBytesRem;
	u32 PrevPos;
	u32 CurrPos;
	u32 Idx;
        u32 DmbOff;
        u32 SectAddr;
        u32 RemSize;
        u32 DoneSize;
	uint64_t DmbAddr;
	uint64_t TgtTileAddr;
	const XAie_CoreMod *CoreMod;

	CoreMod = DevInst->DevProp.DevMod[XAIEGBL_TILE_TYPE_AIETILE].CoreMod;

	/* Point to the section in the ELF file */
	fseek(Fd, SectPtr->sh_offset, SEEK_SET);

	/* Allocate a memory of the size of this section */
	DataPtr = (u32 *)malloc(SectPtr->sh_size);
	memset((u8 *)DataPtr, 0U, SectPtr->sh_size);

	NumBytesRem = SectPtr->sh_size;
	NumBytesRead = 0U;
	CurrPtr = DataPtr;
	PrevPos = ftell(Fd);

	/*
	 * Read complete data of this section until all
	 * bytes of the section are read
	 */
	while(NumBytesRem > 0U) {
		if(NumBytesRem < 4U) {
			*(u8 *)CurrPtr = fgetc(Fd);
		} else {
			fgets((u8 *)CurrPtr, NumBytesRem, Fd);
		}
		CurrPos = ftell(Fd);
		NumBytesRead = (CurrPos - PrevPos);
		NumBytesRem -= NumBytesRead;
		PrevPos = CurrPos;
		CurrPtr = (u8 *)CurrPtr + NumBytesRead;

		XAieLib_print("NumBytesRead : %d, NumBytesRem : %d\n",
				NumBytesRead, NumBytesRem);
	}

	CurrPtr = DataPtr;

	if(strstr(SectName, "data.DMb") != NULL) {
                TgtTileAddr = XAie_GetTargetTileAddr(DevInst, Loc,
                					SectPtr->sh_addr);
                SectAddr = SectPtr->sh_addr;
                RemSize = SectPtr->sh_size;
                DoneSize = 0U;

		/* Use 32 bit loads to match sim output */
		for(Idx = 0U; Idx < RemSize; Idx += 4U){
			/* Mask address as per the Data memory offset*/
			DmbOff = (SectAddr &
                                       (CoreMod->CoreMemSize - 1)) + Idx;

                        if(DmbOff & CoreMod->CoreMemSize) {
                                /*
                                  * 32 KB boundary cross. Data section 
                                  * moving to the memory bank of next
                                  * cardinal direction. Change the target
                                  * tile address
                                  */
                                SectPtr->sh_addr += CoreMod->CoreMemSize;
                                TgtTileAddr =
                                        XAie_GetTargetTileAddr(DevInst, Loc,
						(SectPtr->sh_addr &
                                                (CoreMod->CoreEastAddrEnd - CoreMod->CoreSouthAddrEnd)));
                                SectAddr = 0x0U;
                                Idx = 0U;
                                DmbOff = 0U;
                                RemSize -= DoneSize;
                        }

                        DmbAddr = TgtTileAddr +
                                        DevInst->DevProp.DevMod[XAIEGBL_TILE_TYPE_AIETILE]
					.MemMod->MemAddr
					+ DmbOff;
			XAieGbl_Write32(DmbAddr, *CurrPtr++);
                        DoneSize += 4U;
		}
        } else {
                TgtTileAddr = DevInst->BaseAddr + 
			_XAie_GetTileAddr(DevInst, Loc.Row, Loc.Col) + 
			CoreMod->ProgMemAddr + SectPtr->sh_addr;

                for(Idx = 0U; Idx < SectPtr->sh_size; Idx += 4U) {
			XAieGbl_Write32((TgtTileAddr + Idx), *CurrPtr++);
        	}
        }

	/* Free the allocated memory */
	free(DataPtr);
}

/*****************************************************************************/
/**
*
* This is the API to load the specified ELF to the target AIE Tile program
* memory followed by clearing of the BSS sections.
*
* @param	DevInst- Pointer to the Device instance structure.
* @param	Range: Range of Aie Tiles.
* @param	ElfPtr: Path to the ELF file to be loaded into memory.
*
* @return	XAIE_OK on success, else error code.
*
* @note		Internal only.
*
*******************************************************************************/
static AieRC XAie_LoadElfRange(XAie_DevInst *DevInst, XAie_LocRange Range,
		u8 *ElfPtr, u8 LoadSym)
{
	FILE *Fd;
	Elf32_Ehdr ElfHdr;
	Elf32_Shdr SectHdr[XAIE_ELF_SECTION_NUMMAX];
	u8 TileType;

	u8 MapPath[256U];
	u8 Count = 0U;
	u8 ShNameIdx = 0U;

	u8 ShName[XAIE_ELF_SECTION_NUMMAX][XAIE_ELF_SECTION_NAMEMAXLEN];

	u32 Idx = 0U;
	u32 Status;
	u32 *DataPtr;
	u32 *CurrPtr;
	u32 NumBytesRead;
	u32 NumBytesRem;
	u32 PrevPos;
	u32 CurrPos;
        u32 DmbOff;
	u32 SectAddr;
	u32 RemSize;
	u32 DoneSize;
	uint64_t DmbAddr;
	uint64_t TgtTileAddr;

	const XAie_CoreMod *CoreMod;
	XAie_LocType Loc;

	if((DevInst == XAIE_NULL) ||
			(DevInst->IsReady != XAIE_COMPONENT_IS_READY)) {
		XAieLib_print("Error: Invalid Device Instance\n");
		return XAIE_INVALID_ARGS;
	}

	if(_XAie_CheckLocRange(DevInst, Range) != XAIE_OK) {
		XAieLib_print("Error: Invalid Device Range\n");
		return XAIE_INVALID_RANGE;
	}

	TileType = _XAie_GetTileType(DevInst, Range);
	if(TileType != XAIEGBL_TILE_TYPE_AIETILE) {
		XAieLib_print("Error: Invalid Tile Type\n");
		return XAIE_INVALID_TILE;
	}

	if(_XAie_CheckRangeTileType(DevInst, Range) != XAIE_OK) {
		XAieLib_print("Error: Range has different Tile Types\n");
		return XAIE_INVALID_RANGE;
	}

	CoreMod = DevInst->DevProp.DevMod[TileType].CoreMod;

#ifdef __AIESIM__

	XAieSim_StackSz StackSz;
	/* Get the stack range */
	strcpy(MapPath, ElfPtr);
	strcat(MapPath, ".map");
	Status = XAieSim_GetStackRange(MapPath, &StackSz);
	XAieLib_print("Stack start:%08x, end:%08x\n",
					StackSz.start, StackSz.end);
	if(Status != XAIESIM_SUCCESS) {
		XAieSim_print("ERROR: Stack range definition failed\n");
		return Status;
	}

	for(int R = Range.Start.Row; R <= Range.End.Row; R += Range.Stride.Row) {
		for(int C = Range.Start.Col; C <= Range.End.Col; C += Range.Stride.Col) {
			/* Send the stack range set command */
			XAieSim_WriteCmd(XAIESIM_CMDIO_CMD_SETSTACK, C, R,
					StackSz.start, StackSz.end, XAIE_NULL);
			/* Load symbols if enabled */
			if(LoadSym == XAIE_ENABLE) {
				XAieSim_WriteCmd(XAIESIM_CMDIO_CMD_LOADSYM, C,
						R, 0, 0, ElfPtr);
			}
		}
	}

#endif

	/* Open the ELF file for reading */
	Fd = fopen(ElfPtr, "r");
	if(Fd == NULL) {
		XAieLib_print("Error: Invalid ELF file\n");
		return XAIE_INVALID_ELF;
	}

	/* Read the ELF header */
        NumBytesRem = sizeof(ElfHdr);
	NumBytesRead = 0U;
	PrevPos = ftell(Fd);
	CurrPtr = (u8 *)&ElfHdr;

	/* Read complete data of ELF headers until all bytes  are read */
	while(NumBytesRem > 0U) {
		if(NumBytesRem < 4U) {
			*(u8 *)CurrPtr = fgetc(Fd);
		} else {
			fgets((u8 *)CurrPtr, NumBytesRem, Fd);
		}

		CurrPos = ftell(Fd);
		NumBytesRead = (CurrPos - PrevPos);
		NumBytesRem -= NumBytesRead;
		PrevPos = CurrPos;
		CurrPtr = (u8 *)CurrPtr + NumBytesRead;

		XAieLib_print("NumBytesRead : %d, NumBytesRem : %d\n",
						NumBytesRead, NumBytesRem);
	}

	XAieLib_print("**** ELF HEADER ****\n");
	XAieLib_print("e_type\t\t : %08x\ne_machine\t : %08x\ne_version\t : "
		"%08x\ne_entry\t\t : %08x\ne_phoff\t\t : %08x\n",
		ElfHdr.e_type, ElfHdr.e_machine, ElfHdr.e_version,
		ElfHdr.e_entry, ElfHdr.e_phoff);
	XAieLib_print("e_shoff\t\t : ""%08x\ne_flags\t\t : %08x\ne_ehsize\t"
		" : %08x\ne_phentsize\t : %08x\ne_phnum\t\t : %08x\n"
		"e_shentsize\t : %08x\ne_shnum\t\t : %08x\ne_shstrndx\t : "
		"%08x\n\n", ElfHdr.e_shoff, ElfHdr.e_flags,
		ElfHdr.e_ehsize, ElfHdr.e_phentsize, ElfHdr.e_phnum,
		ElfHdr.e_shentsize, ElfHdr.e_shnum, ElfHdr.e_shstrndx);

	/* Point to the section headers in the file */
	fseek(Fd, ElfHdr.e_shoff, SEEK_SET);
	/* Read the section header entries */

	NumBytesRem = (sizeof(SectHdr[0]) * ElfHdr.e_shnum);
	NumBytesRead = 0U;
	PrevPos = ftell(Fd);
	CurrPtr = (u8 *)&SectHdr[0];

	/* Read complete data of section headers until all bytes  are read */
	while(NumBytesRem > 0U) {
		if(NumBytesRem < 4U) {
			*(u8 *)CurrPtr = fgetc(Fd);
		} else {
			fgets((u8 *)CurrPtr, NumBytesRem, Fd);
		}

		CurrPos = ftell(Fd);
		NumBytesRead = (CurrPos - PrevPos);
		NumBytesRem -= NumBytesRead;
		PrevPos = CurrPos;
		CurrPtr = (u8 *)CurrPtr + NumBytesRead;

		XAieLib_print("NumBytesRead : %d, NumBytesRem : %d\n",
						NumBytesRead, NumBytesRem);
	}

	/*
	 * Get the section names from the .shstrtab section. First find the
	 * index of this section among the section header entries
	 */
	while(Count < ElfHdr.e_shnum) {
		if(SectHdr[Count].sh_type == SHT_STRTAB) {
			ShNameIdx = Count;
			break;
		}
		Count++;
	}

	/* Now point to the .shstrtab section in the ELF file */
	fseek(Fd, SectHdr[ShNameIdx].sh_offset, SEEK_SET);
	Count = 0U;
	while(Count < ElfHdr.e_shnum) {
		/*
		 * Read each section's name string from respective offsets
		 * from the .shstrtab section
		 */
		fseek(Fd, SectHdr[Count].sh_name, SEEK_CUR);
		fgets(ShName[Count], XAIE_ELF_SECTION_NAMEMAXLEN, Fd);

		/*
		 * Again point to the start of the section .shstrtab for the
		 * next section to get its name from its corresponding offset
		 */
		fseek(Fd, SectHdr[ShNameIdx].sh_offset, SEEK_SET);
		Count++;
	}

	XAieLib_print("**** SECTION HEADERS ****\n");
	XAieLib_print("Idx\tsh_name\t\tsh_type\t\tsh_flags\tsh_addr\t\tsh_offset"
		"\tsh_size\t\tsh_link\t\tsh_addralign\tsection_name\n");
	Count = 0U;
	while(Count < ElfHdr.e_shnum) {
		XAieLib_print("%d\t%08x\t%08x\t%08x\t%08x\t%08x\t%08x\t%08x\t"
			"%08x\t%s\n", Count, SectHdr[Count].sh_name,
			SectHdr[Count].sh_type, SectHdr[Count].sh_flags,
			SectHdr[Count].sh_addr, SectHdr[Count].sh_offset,
			SectHdr[Count].sh_size, SectHdr[Count].sh_link,
			SectHdr[Count].sh_addralign, ShName[Count]);
		Count++;
	}

	Count = 0U;
	for(u8 R = Range.Start.Row; R <= Range.End.Row; R += Range.Stride.Row) {
		for(u8 C = Range.Start.Col; C <= Range.End.Col; C += Range.Stride.Col) {
			Loc.Row = R;
			Loc.Col = C;
			while(Count < ElfHdr.e_shnum) {
				/* Copy the program data sections to memory */
				if((SectHdr[Count].sh_type == SHT_PROGBITS) &&
						((SectHdr[Count].sh_flags &
						  (SHF_ALLOC | SHF_EXECINSTR)) 
						 || (SectHdr[Count].sh_flags &
							 (SHF_ALLOC | SHF_WRITE)))) {
					XAie_WriteElfSection(DevInst, Loc,
							ShName[Count], 
							&SectHdr[Count], Fd);
				}
				/* Zero out the bss sections */
				if((SectHdr[Count].sh_type == SHT_NOBITS) &&
						(strstr(ShName[Count], "bss.DMb")
						 != NULL)) {
					XAieLib_print("Zeroing out the bss sections\n");
					TgtTileAddr = XAie_GetTargetTileAddr(
							DevInst, Loc,
							SectHdr[Count].sh_addr);
					SectAddr = SectHdr[Count].sh_addr;
					RemSize = SectHdr[Count].sh_size;
					DoneSize = 0U;

					/* Use 32 bit loads to match sim output */
					for(Idx = 0U; Idx < RemSize; Idx += 4U){
						/* Mask address as per the Data 
						   memory offset*/
						DmbOff = (SectAddr &
								(CoreMod->CoreMemSize - 1)) + Idx;

						if(DmbOff & CoreMod->CoreMemSize) {
                                        /*
                                         * 32/64 KB boundary cross. Data section
                                         * moving to the memory bank of next
                                         * cardinal direction. Change the target
                                         * tile address
                                         */
							SectHdr[Count].sh_addr
								+= CoreMod->CoreMemSize;
							TgtTileAddr =
								XAie_GetTargetTileAddr(DevInst, Loc,
										(SectHdr[Count].sh_addr &
										 (CoreMod->CoreEastAddrEnd - CoreMod->CoreSouthAddrEnd)));
							SectAddr = 0x0U;
							Idx = 0U;
							DmbOff = 0U;
							RemSize -= DoneSize;
							DoneSize = 0U;
						}

						DmbAddr = TgtTileAddr +
							DevInst->DevProp.DevMod[TileType].MemMod->MemAddr
							+ DmbOff;
						XAieGbl_Write32(DmbAddr, 0U);
						DoneSize += 4U;
					}
				}
				Count++;
			}
		}
	}
	fclose(Fd);

	return XAIE_OK;
}

/*****************************************************************************/
/**
*
* This is the API to load the specified ELF to the target AIE Tile program
* memory followed by clearing of the BSS sections.
*
* @param	DevInst- Pointer to the Device instance structure.
* @param	Loc: Coordinate of Aie Tile.
* @param	ElfPtr: Path to the ELF file to be loaded into memory.
*
* @return	XAIE_OK on success, else error code.
*
* @note		None.
*
*******************************************************************************/
AieRC XAie_LoadElf(XAie_DevInst *DevInst, XAie_LocType Loc, u8 *ElfPtr,
		u8 LoadSym)
{
	XAie_LocRange Range = { Loc, Loc, { 1, 1 } };
	return XAie_LoadElfRange(DevInst, Range, ElfPtr, LoadSym);
}

/** @} */
