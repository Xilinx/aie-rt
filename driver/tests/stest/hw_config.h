#ifndef HW_CONFIG_H
#define HW_CONFIG_H

#if AIE_GEN == 1

#define HW_GEN XAIE_DEV_GEN_AIE
#define XAIE_BASE_ADDR 0x20000000000
#define XAIE_COL_SHIFT 23
#define XAIE_ROW_SHIFT 18

#if DEVICE == 80 /* s80 Config */
	#define XAIE_NUM_ROWS 9
	#define XAIE_NUM_COLS 50
	#define XAIE_SHIM_ROW 0
	#define XAIE_MEM_TILE_ROW_START 0
	#define XAIE_MEM_TILE_NUM_ROWS 0
	#define XAIE_AIE_TILE_ROW_START 1
	#define XAIE_AIE_TILE_NUM_ROWS 8

#elif DEVICE == 0 /* SystemC Config */
        #define XAIE_NUM_ROWS 4
        #define XAIE_NUM_COLS 5
        #define XAIE_SHIM_ROW 0
        #define XAIE_MEM_TILE_ROW_START 0
        #define XAIE_MEM_TILE_NUM_ROWS 0
        #define XAIE_AIE_TILE_ROW_START 1
        #define XAIE_AIE_TILE_NUM_ROWS 3
#endif /* DEVICE */

/* AIE_GEN = 2 by default for now. */
#elif AIE_GEN == 2

#define HW_GEN XAIE_DEV_GEN_AIEML
#define XAIE_BASE_ADDR 0x20000000000
#define XAIE_COL_SHIFT 25
#define XAIE_ROW_SHIFT 20

#if DEVICE == 60 /* sv60 Config */
        #define XAIE_NUM_ROWS 11
        #define XAIE_NUM_COLS 38
        #define XAIE_SHIM_ROW 0
        #define XAIE_MEM_TILE_ROW_START 1
        #define XAIE_MEM_TILE_NUM_ROWS 2
        #define XAIE_AIE_TILE_ROW_START 3
        #define XAIE_AIE_TILE_NUM_ROWS 8

#elif DEVICE == 0 /* SystemC Config */
        #define XAIE_NUM_ROWS 6
        #define XAIE_NUM_COLS 5
        #define XAIE_SHIM_ROW 0
        #define XAIE_MEM_TILE_ROW_START 1
        #define XAIE_MEM_TILE_NUM_ROWS 1
        #define XAIE_AIE_TILE_ROW_START 2
        #define XAIE_AIE_TILE_NUM_ROWS 4
#endif /* DEVICE */

#elif AIE_GEN == 3

#define HW_GEN XAIE_DEV_GEN_AIE2IPU
#define XAIE_BASE_ADDR 0x20000000000
#define XAIE_COL_SHIFT 25
#define XAIE_ROW_SHIFT 20

/* SystemC Config */
#define XAIE_NUM_ROWS 6
#define XAIE_NUM_COLS 5
#define XAIE_SHIM_ROW 0
#define XAIE_MEM_TILE_ROW_START 1
#define XAIE_MEM_TILE_NUM_ROWS 1
#define XAIE_AIE_TILE_ROW_START 2
#define XAIE_AIE_TILE_NUM_ROWS 4

#elif AIE_GEN == 4

#define HW_GEN XAIE_DEV_GEN_AIE2P
#define XAIE_BASE_ADDR 0x20000000000
#define XAIE_COL_SHIFT 25
#define XAIE_ROW_SHIFT 20

/* SystemC Config */
#define XAIE_NUM_ROWS 6
#define XAIE_NUM_COLS 5
#define XAIE_SHIM_ROW 0
#define XAIE_MEM_TILE_ROW_START 1
#define XAIE_MEM_TILE_NUM_ROWS 1
#define XAIE_AIE_TILE_ROW_START 2
#define XAIE_AIE_TILE_NUM_ROWS 4

#elif AIE_GEN == 5

#define HW_GEN XAIE_DEV_GEN_AIE2PS
#define XAIE_COL_SHIFT 25
#define XAIE_ROW_SHIFT 20

#if DEVICE == 80 /* IPP Config */
	#define XAIE_BASE_ADDR 0x22000000000
        #define XAIE_NUM_ROWS 7
        #define XAIE_NUM_COLS 4
        #define XAIE_SHIM_ROW 0
        #define XAIE_MEM_TILE_ROW_START 1
        #define XAIE_MEM_TILE_NUM_ROWS 2
        #define XAIE_AIE_TILE_ROW_START 3
        #define XAIE_AIE_TILE_NUM_ROWS 4

#elif DEVICE == 0 /* systemc Config */
	#define XAIE_BASE_ADDR 0x20000000000
	#define XAIE_NUM_ROWS 4
	#define XAIE_NUM_COLS 5
	#define XAIE_SHIM_ROW 0
	#define XAIE_MEM_TILE_ROW_START 1
	#define XAIE_MEM_TILE_NUM_ROWS 1
	#define XAIE_AIE_TILE_ROW_START 2
	#define XAIE_AIE_TILE_NUM_ROWS 2

#endif /* DEVICE */
#endif /* AIE_GEN */
#endif /* HW_CONFIG_H */
