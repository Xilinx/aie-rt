###############################################################################
# Copyright (C) 2024 AMD, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
###############################################################################


###############################################################################
# @file Makefile.uc
# @{
#
# This file contains Makefile to build uC driver APIs.
###############################################################################

include $(ME_ROOT)/microblaze/microblaze.mk
CP = cp
LIBSOURCES = $(filter-out ./liburing/%, $(wildcard ./*/*.c))  $(filter-out ./liburing/%, $(wildcard ./*/*/*.c))
CFLAGS += -Wall -Wextra --std=c11
EXTRA_CFLAGS += -D_POSIX_C_SOURCE=200809 -D_DEFAULT_SOURCE

OUTS = $(LIBSOURCES:.c=.o)
INCLUDEFILES =./*/xaie_aie2ps_uc_module_core.h  ./*/xaie_aie2ps_uc_module.h  ./*/xaie_handshake.h  ./*/xaie_queue.h  ./*/xaie_regdb_uc_module_core.h  ./*/xaie_regdb_uc_module.h  ./*/xaie_uc_dma.h   ./*/xaie_utils.h  ./*/xaie_wait_handle.h ./*/xaie_uc_core.h ./*/xaie_clock.h ./*/xaie_reset.h ./*/xaie_core.h ./*/xaie_dma.h ./*/xaie_elfloader.h ./*/xaie_events.h ./*/xaie_events_aie.h ./*/xaie_events_aieml.h ./*/xaie_interrupt.h ./*/xaie_lite.h ./*/xaie_lite_util.h ./*/xaie_locks.h ./*/xaie_mem.h ./*/xaie_perfcnt.h ./*/xaie_plif.h ./*/xaie_ss.h ./*/xaie_timer.h ./*/xaie_trace.h ./*/xaie_util_events.h ./*/xaiegbl.h ./*/xaiegbl_defs.h ./*/xaiegbl_regdef.h ./*/xaie_io.h ./*/xaie_feature_config.h ./*/xaie_helper.h ./*/xaie_lite_hwcfg.h ./*/xaiegbl_params.h ./xaiengine.h ./*/*/xlnx-ai-engine.h ./*/xaie_txn.h
INTERNALFILES = $(filter-out ./liburing/%, $(wildcard ./*/*.h ./*/*/*.h))
INCLUDEDIR = ../include
INTERNALDIR = ../internal
NAME := xaiengine
MAJOR = 3
MINOR = 5
VERSION = $(MAJOR).$(MINOR)

all: include lib$(NAME).so

include:
	mkdir $(INCLUDEDIR)/xaiengine -p
	mkdir $(INTERNALDIR)/xaiengine -p
	$(CP) xaiengine.h $(INCLUDEDIR)
	$(CP) $(INCLUDEFILES) $(INCLUDEDIR)/xaiengine
	$(CP) xaiengine.h $(INTERNALDIR)
	$(CP) $(INTERNALFILES) $(INTERNALDIR)/xaiengine

NAME_UC := xaiengineuc
CFLAGS += $(ADD_CFLAGS) -D_UCFW_
LIBSOURCES_UC = $(wildcard ./uc_driver/*.c)
OUTS_UC = $(LIBSOURCES_UC:.c=.o)

lib$(NAME_UC).a: $(OUTS_UC)
	ar rcs lib$(NAME_UC).a $(OUTS_UC)

xaieuc: include lib$(NAME_UC).a

ifeq ($(MAKECMDGOALS) ,xaieuc)
%.o: %.c include
	$(CC) -Wl,-T -Wl,lscript.ld $(CFLAGS) -I$(INTERNALDIR) -I$(INTERNALDIR)/xaiengine -c -fPIC $< -o $@
endif
