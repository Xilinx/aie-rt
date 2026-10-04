/******************************************************************************
* Copyright (C) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xaie_helper.h
* @{
*
* This file contains inline helper functions for AIE drivers.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- -----------------------------------------------------
* 1.0   Tejus   09/24/2019  Initial creation
* 1.1   Tejus   12/09/2019  Include correct header file to avoid cyclic
*			    dependancy
* 1.2   Tejus   03/22/2020  Remove helper functions used by initial dma
*			    implementations
* 1.3   Tejus   04/13/2020  Add api to get tile type from Loc
* 1.4   Tejus   04/13/2020  Remove helper functions for range apis
* 1.5   Tejus   06/10/2020  Add helper functions for IO backend.
* 1.6   Nishad  07/06/2020  Add helper functions for stream switch module.
* 1.7   Nishad  07/24/2020  Add _XAie_GetFatalGroupErrors() helper function.
* </pre>
*
******************************************************************************/
#ifndef XAIEHELPER_H
#define XAIEHELPER_H

/***************************** Include Files *********************************/
#include <limits.h>
#include <stddef.h>
#include "xaie_feature_config.h"
#include "xaie_mem_alloc.h"
#include "xaie_io.h"
#include "xaiegbl_regdef.h"
#include "xaie_core.h"
#include "xaie_dma.h"
#include "xaie_locks.h"
/************************** Variable Definitions *****************************/
/**
 * Defines the available log levels.
 * The levels are ordered by severity. A higher value means more verbose logging.
 */
typedef enum {
    XAIE_LOG_LEVEL_FATAL = 0,
    XAIE_LOG_LEVEL_ERROR,
    XAIE_LOG_LEVEL_WARN,
    XAIE_LOG_LEVEL_INFO,
    XAIE_LOG_LEVEL_DEBUG,
    XAIE_LOG_LEVEL_TRACE
} XAieLogLevel;

/*
 * Compile-time default log level, used to initialize AieLogLevel.
 * When XAIE_FEATURE_STDIO_LOGGING_ENABLE is set, this default can still be
 * overridden at runtime via the "XAIE_LOG_LEVEL" environment variable or an
 * explicit call to XAie_LoggerInit(). When it is not set (FuSa builds with
 * no hosted getenv()), this macro is the only way to change the default
 * level, e.g. by defining -DXAIE_LOG_LEVEL_DEFAULT=XAIE_LOG_LEVEL_WARN on
 * the compiler command line.
 */
#ifndef XAIE_LOG_LEVEL_DEFAULT
#define XAIE_LOG_LEVEL_DEFAULT XAIE_LOG_LEVEL_ERROR
#endif

/* Bound on the size of a single formatted log/print message. */
#ifndef XAIE_LOG_BUF_SIZE
#define XAIE_LOG_BUF_SIZE 256U
#endif

extern XAieLogLevel AieLogLevel;

/***************************** Macro Definitions *****************************/
#define container_of(ptr, type, member)	({					\
		void *__ptr = ptr;						\
		__ptr == NULL ? NULL : (type *)(__ptr - offsetof(type, member));	\
	})
#define CheckBit(bitmap, pos)   ((bitmap)[(u64)(pos) / (sizeof((bitmap)[0]) * 8U)] & \
				(u32)((u32)1U << ((u64)(pos) % (sizeof((bitmap)[0]) * 8U))))

#ifdef XAIE_FEATURE_STDIO_LOGGING_ENABLE
/*
 * Hosted path: XAie_Log() takes a real FILE* and formats with
 * fprintf()/vfprintf(), so it needs a real <stdio.h> (and the va_list
 * variant of printf) from a full hosted C library.
 */
#include <stdio.h>
#define XAIE_LOG_STDOUT stdout
#define XAIE_LOG_STDERR stderr

#define XAIE_FATAL(...) \
    do { \
        if (AieLogLevel >= XAIE_LOG_LEVEL_FATAL) { \
            XAie_Log(XAIE_LOG_STDERR, "[AIE FATAL]", __func__, __LINE__, __VA_ARGS__); \
        } \
    } while(0)

#define XAIE_ERROR(...) \
    do { \
        if (AieLogLevel >= XAIE_LOG_LEVEL_ERROR) { \
            XAie_Log(XAIE_LOG_STDERR, "[AIE ERROR]", __func__, __LINE__, __VA_ARGS__); \
        } \
    } while(0)

#define XAIE_WARN(...) \
    do { \
        if (AieLogLevel >= XAIE_LOG_LEVEL_WARN) { \
            XAie_Log(XAIE_LOG_STDOUT, "[AIE WARNING]", __func__, __LINE__, __VA_ARGS__); \
        } \
    } while(0)

#define XAIE_INFO(...) \
    do { \
        if (AieLogLevel >= XAIE_LOG_LEVEL_INFO) { \
            XAie_Log(XAIE_LOG_STDOUT, "[AIE INFO]", __func__, __LINE__, __VA_ARGS__); \
        } \
    } while(0)

#define XAIE_DBG(...) \
    do { \
        if (AieLogLevel >= XAIE_LOG_LEVEL_DEBUG) { \
            XAie_Log(XAIE_LOG_STDOUT, "[AIE DEBUG]", __func__, __LINE__, __VA_ARGS__); \
        } \
    } while(0)

#define XAIE_TRACE(...) \
    do { \
        if (AieLogLevel >= XAIE_LOG_LEVEL_TRACE) { \
            XAie_Log(XAIE_LOG_STDOUT, "[AIE TRACE]", __func__, __LINE__, __VA_ARGS__); \
        } \
    } while(0)
#else /* !XAIE_FEATURE_STDIO_LOGGING_ENABLE */
/*
 * FuSa path: the Arm Certified FuSa C library has no FILE type/streams and
 * no vsnprintf()/vfprintf() (those are C++-extension-only, not part of the
 * plain-C interface) -- only snprintf(const char *, ...) with a literal
 * format is available. So these macros must build the final message with
 * a single direct snprintf() call at the call site (where the format
 * string and argument types are known at compile time) instead of going
 * through a variadic wrapper function that would need a va_list-based
 * formatter to re-expand "..." into text.
 */
#include <stdio.h>
#define XAIE_LOG_IMPL(Level, Tag, Format, ...) \
    do { \
        if (AieLogLevel >= (Level)) { \
            char XAieLogBuf_[XAIE_LOG_BUF_SIZE]; \
            (void)snprintf(XAieLogBuf_, sizeof(XAieLogBuf_), \
                    Tag " %s():" U32_FORMAT ": " Format, \
                    __func__, __LINE__, ##__VA_ARGS__); \
            _XAie_LogSink(XAieLogBuf_); \
        } \
    } while(0)

#define XAIE_FATAL(Format, ...) \
        XAIE_LOG_IMPL(XAIE_LOG_LEVEL_FATAL, "[AIE FATAL]", Format, ##__VA_ARGS__)
#define XAIE_ERROR(Format, ...) \
        XAIE_LOG_IMPL(XAIE_LOG_LEVEL_ERROR, "[AIE ERROR]", Format, ##__VA_ARGS__)
#define XAIE_WARN(Format, ...) \
        XAIE_LOG_IMPL(XAIE_LOG_LEVEL_WARN, "[AIE WARNING]", Format, ##__VA_ARGS__)
#define XAIE_INFO(Format, ...) \
        XAIE_LOG_IMPL(XAIE_LOG_LEVEL_INFO, "[AIE INFO]", Format, ##__VA_ARGS__)
#define XAIE_DBG(Format, ...) \
        XAIE_LOG_IMPL(XAIE_LOG_LEVEL_DEBUG, "[AIE DEBUG]", Format, ##__VA_ARGS__)
#define XAIE_TRACE(Format, ...) \
        XAIE_LOG_IMPL(XAIE_LOG_LEVEL_TRACE, "[AIE TRACE]", Format, ##__VA_ARGS__)
#endif /* XAIE_FEATURE_STDIO_LOGGING_ENABLE */

#if  defined(__microblaze__)
#define PRINT(...) do { (void)xil_printf(__VA_ARGS__); } while(0)
#define UINTPTR_T (uintptr_t)
#define U32_FORMAT "%ld"
#elif !defined(XAIE_FEATURE_STDIO_LOGGING_ENABLE)
/*
 * Same reasoning as XAIE_LOG_IMPL() above: no vsnprintf() in the FuSa
 * C library, so PRINT() must resolve to a single direct snprintf() call
 * at the call site rather than a variadic wrapper function.
 */
#define PRINT(Format, ...) \
    do { \
        char XAiePrintBuf_[XAIE_LOG_BUF_SIZE]; \
        (void)snprintf(XAiePrintBuf_, sizeof(XAiePrintBuf_), Format, ##__VA_ARGS__); \
        _XAie_LogSink(XAiePrintBuf_); \
    } while(0)
#define UINTPTR_T
#define U32_FORMAT "%d"
#else
#define PRINT(...) do { (void)printf(__VA_ARGS__); } while(0)
#define UINTPTR_T
#define U32_FORMAT "%d"
#endif

#ifdef XAIE_FEATURE_STDIO_LOGGING_ENABLE
#define XAIE_PUTCHAR(c) putchar(c)
#else
#define XAIE_PUTCHAR(c) _XAie_SafePutChar(c)
/*
 * FuSa-safe putchar, used in place of hosted putchar() when
 * XAIE_FEATURE_STDIO_LOGGING_ENABLE is not set. Hands a single-char string
 * to _XAie_LogSink(). (PRINT()'s formatting is handled by a macro above,
 * not a function -- see the comment there for why.)
 */
void _XAie_SafePutChar(char c);
/*
 * FuSa-safe log sink. The formatted log/print message is handed to this
 * function instead of being written via hosted stdio. The default
 * implementation (in xaie_helper.c) is a weak no-op, so a FuSa build is
 * silent unless a platform integrator supplies a real sink.
 *
 * To enable output, provide a non-weak (ordinary) definition of this
 * function in BSP/application code and link it in; the linker will use it
 * in place of the weak default automatically -- no registration call is
 * needed. For example, in a BSP source file:
 *
 *   void _XAie_LogSink(const char *Str)
 *   {
 *       MyPlatform_UartWriteString(Str);
 *   }
 */
void _XAie_LogSink(const char *Str);
#endif /* XAIE_FEATURE_STDIO_LOGGING_ENABLE */

/* Compute offset of field within a structure */
#define XAIE_OFFSET_OF(structure, member) \
	((uintptr_t)&(((structure *)0)->member))

/* Compute a pointer to a structure given a pointer to one of its fields */
#define XAIE_CONTAINER_OF(ptr, structure, member) \
	(void*)((uintptr_t)(ptr) - XAIE_OFFSET_OF(structure, member))

/* Loop through the set bits in Value */
#define for_each_set_bit(Index, Value, Len)				      \
	for((Index) = first_set_bit((Value)) - 1U;			      \
	    (Index) < (Len);						      \
	    (Value) &= (Value) - 1U, (Index) = first_set_bit((Value)) - 1U)

/* Generate value with a set bit at given Index */
#define BIT(Index)		((u32)1U << (Index))

/*
 * __attribute is not supported for windows. remove it conditionally.
 */
#ifdef _MSC_VER
#define XAIE_PACK_ATTRIBUTE
#else
#define XAIE_PACK_ATTRIBUTE  __attribute__((packed, aligned(4)))
#endif

/* Data structure to capture the dma status */
typedef struct {
        u32 S2MMStatus;
        u32 MM2SStatus;
} XAie_DmaStatus;

/* Data structure to capture the core tile status */
typedef struct {
        XAie_DmaStatus *Dma;
        u32 *EventCoreModStatus;
        u32 *EventMemModStatus;
        u32 CoreStatus;
        u32 ProgramCounter;
        u32 StackPtr;
        u32 LinkReg;
        u8  *LockValue;
} XAie_CoreTileStatus;
/* Data structure to capture the mem tile status */
typedef struct {
        XAie_DmaStatus *Dma;
        u32 *EventStatus;
        u8 *LockValue;
} XAie_MemTileStatus;

/* Data structure to capture the shim tile status */
typedef struct {
        XAie_DmaStatus *Dma;
        u32 *EventStatus;
        u8 *LockValue;
} XAie_ShimTileStatus;

/* Data structure to capture column status */
typedef struct {
        XAie_CoreTileStatus *CoreTile;
        XAie_MemTileStatus *MemTile;
        XAie_ShimTileStatus *ShimTile;
} XAie_ColStatus;

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
*
* Calculates the Tile Address from Row, Col of the AIE array/partition
*
* @param	DevInst: Device Instance
* @param	R: Row
* @param	C: Column
* @return	TileAddr
*
******************************************************************************/
static inline u64 XAie_GetTileAddr(XAie_DevInst *DevInst, u8 R, u8 C)
{
	return (((u64)R & 0xFFU) << DevInst->DevProp.RowShift) |
		(((u64)C & 0xFFU) << DevInst->DevProp.ColShift);
}

/*****************************************************************************/
/**
*
* Calculates the index value of first set bit. Indexing starts with a value of
* 1.
*
* @param	Value: Value
* @return	Index of first set bit.
*
* @note		Internal API only.
*
******************************************************************************/
static inline u32 first_set_bit(u64 Value)
{
	u32 Index = 1;

	if (Value == 0U) {
		return 0;
	}

	while ((Value & 1U) == 0U) {
		Value >>= 1;
		Index++;
	}

	return Index;
}

#ifdef XAIE_FEATURE_STDIO_LOGGING_ENABLE
void XAie_Log(FILE *Fd, const char *prefix, const char *func, u32 line,
		const char *Format, ...);
#endif
u8 XAie_GetTileTypefromLoc(XAie_DevInst *DevInst, XAie_LocType Loc);
AieRC XAie_CheckModule(XAie_DevInst *DevInst, XAie_LocType Loc,
                XAie_ModuleType Module);
AieRC XAie_GetUngatedLocsInPartition(XAie_DevInst *DevInst, u32 *NumTiles,
                XAie_LocType *Locs);
AieRC XAie_Write32(XAie_DevInst *DevInst, u64 RegOff, u32 Value);
s32 XAie_Write32Async(XAie_DevInst *DevInst, u64 RegOff, u32 Value, XAie_AsyncRes *AsyncRes);
AieRC XAie_AsyncWait(XAie_DevInst *DevInst);
AieRC XAie_AsyncWaitNr(XAie_DevInst *DevInst, u32 Nr);
AieRC XAie_Read32(XAie_DevInst *DevInst, u64 RegOff, u32 *Data);
AieRC XAie_MaskWrite32(XAie_DevInst *DevInst, u64 RegOff, u32 Mask, u32 Value);
s32 XAie_MaskWrite32Async(XAie_DevInst *DevInst, u64 RegOff, u32 Mask, u32 Value,
			  XAie_AsyncRes *AsyncRes);
AieRC XAie_MaskPoll(XAie_DevInst *DevInst, u64 RegOff, u32 Mask, u32 Value,
		u32 TimeOutUs);
AieRC XAie_BlockWrite32(XAie_DevInst *DevInst, u64 RegOff, const u32 *Data,
			u32 Size);
s32 XAie_BlockWrite32Async(XAie_DevInst *DevInst, u64 RegOff, const u32 *Data,
			   u32 Size, XAie_AsyncRes *AsyncRes);
s32 XAie_BlockWrite64Async(XAie_DevInst *DevInst, u64 RegOff, const u32 *Data,
			   u32 Size, XAie_AsyncRes *AsyncRes);
AieRC XAie_BlockSet32(XAie_DevInst *DevInst, u64 RegOff, u32 Data, u32 Size);
s32 XAie_BlockSet32Async(XAie_DevInst *DevInst, u64 RegOff, u32 Data,
			 u32 Size, XAie_AsyncRes *AsyncRes);
AieRC XAie_CmdWrite(XAie_DevInst *DevInst, u8 Col, u8 Row, u8 Command,
		u32 CmdWd0, u32 CmdWd1, const char *CmdStr);
void BuffHexDump(char* buff,u32 size);
s32 XAie_RequestCustomTxnOp(XAie_DevInst *DevInst);
AieRC XAie_AddCustomTxnOp(XAie_DevInst *DevInst, u8 OpNumber, void* Args, size_t size);
AieRC XAie_AddressPatching(XAie_DevInst *DevInst, u32 Arg_Offset, u8 Num_BDs);
AieRC XAie_WaitUCDMA(XAie_DevInst *DevInst);
AieRC XAie_ModeConfig(XAie_DevInst *DevInst, XAie_ModeSelect Mode);
XAie_ModeSelect XAie_GetModeConfig(XAie_DevInst *DevInst);
AieRC XAie_RunOp(XAie_DevInst *DevInst, XAie_BackendOpCode Op, void *Arg);
AieRC XAie_SetPadInteger(XAie_DevInst *DevInst, char* BuffName, u32 BuffSize);
AieRC XAie_SetPadString(XAie_DevInst *DevInst, char* BuffName, char* BuffBlobPath);
u32 XAie_GetNumRows(XAie_DevInst *DevInst, u8 TileType);
u32 XAie_GetStartRow(XAie_DevInst *DevInst, u8 TileType);
AieRC XAie_PrintDmaStatus(XAie_DevInst *DevInst, XAie_LocType Loc, const XAie_DmaMod *DmaMod);
AieRC XAie_PrintBdStatus(XAie_DevInst *DevInst, XAie_LocType Loc, const XAie_DmaMod *DmaMod, u8 BdWordCount);
AieRC XAie_DmaStatusDump(XAie_DevInst *DevInst, u8 StartCol, u8 NumCols);
AieRC XAie_StatusDump(XAie_DevInst *DevInst, XAie_ColStatus *Status);
AieRC XAie_GetPartitionList(XAie_DevInst *DevInst);
u8 XAie_IsUcModulePresent(XAie_DevInst* DevInst, u8 TileType);
void XAie_LoggerInit(const char *AieLevelEnv);
#endif		/* end of protection macro */
/** @} */
