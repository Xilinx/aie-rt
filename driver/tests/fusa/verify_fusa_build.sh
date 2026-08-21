#!/usr/bin/env bash
###############################################################################
# Copyright (C) 2026 AMD, Inc.  All rights reserved.
# SPDX-License-Identifier: MIT
###############################################################################
#
# Verifies the aie-rt driver's FuSa-libc opt-out paths (XAIE_FEATURE_NO_*,
# see driver/src/global/xaie_feature_config.h) actually compile:
#
#   1. Native logic test: builds and runs the standalone allocator test
#      against the real xaie_mem_alloc.c with the host compiler. The only
#      step that executes anything.
#   2. Cross-compile check: compiles the FuSa-touched driver files with
#      armclang for Cortex-R52, with and without the FuSa opt-outs.
#      Compile-only -- no R52 target/simulator here to link or run.
#
# xaie_baremetal.c is excluded from step 2: it needs real Xilinx BSP
# headers, a BSP/toolchain concern unrelated to FuSa libc.
#
# Step 2's FuSa half compiles against the real Arm Certified FuSa C
# library headers when FUSA_LIBC_INCLUDE_DIR exists, else falls back to
# shim/. The default half always uses shim/ (or armclang's own bundled
# headers) since it exercises hosted code paths needing full stdio/errno
# that FuSa deliberately doesn't provide.
#
# Usage:
#   ./verify_fusa_build.sh              # run both steps
#   ./verify_fusa_build.sh --native-only
#   ./verify_fusa_build.sh --cross-only
#
# Env vars (all optional):
#   CC                     host C compiler for step 1 (default: cc)
#   ARMCLANG                armclang binary name (default: armclang)
#   ARMCLANG_BIN_DIR         dir to prepend to PATH to find armclang
#   ARMLMD_LICENSE_FILE      Arm license server, e.g. 8224@licsrv07.amd.com
#   ARM_TOOL_VARIANT         Arm toolchain variant, e.g. ult
#   REQUIRE_ARMCLANG         default 1: fail step 2 if armclang is missing,
#                            instead of skipping it. Set 0 to skip instead.
#   FUSA_LIBC_INCLUDE_DIR    real FuSa C library's include/ dir (defaults
#                            to the AMD-internal path). Falls back to
#                            shim/ if missing.
#
# Every build/run log gets its own file under a "Logs: ..." temp dir
# printed on exit; nothing is auto-deleted (clean up with
# `rm -rf /tmp/fusa_verify.*`).
###############################################################################
set -u

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DRIVER_SRC="$SCRIPT_DIR/../../src"
UTEST_DIR="$SCRIPT_DIR/../utest"
SHIM_DIR="$SCRIPT_DIR/shim"
WORK="$(mktemp -d /tmp/fusa_verify.XXXXXX)"
# Not cleaned up: logs below stay here for later inspection. Path is
# printed on every exit path; old runs are yours to clean up.
trap 'echo; echo "Logs: $WORK"' EXIT

CC="${CC:-cc}"
ARMCLANG="${ARMCLANG:-armclang}"
: "${ARMCLANG_BIN_DIR:=}"
: "${ARMLMD_LICENSE_FILE:=}"
: "${ARM_TOOL_VARIANT:=}"
: "${REQUIRE_ARMCLANG:=1}"
: "${FUSA_LIBC_INCLUDE_DIR:=/proj/xsjsswstaff/mvanukur/FuSa_Libc/fusa_libc/include}"

RUN_NATIVE=1
RUN_CROSS=1
case "${1:-}" in
	--native-only) RUN_CROSS=0 ;;
	--cross-only)  RUN_NATIVE=0 ;;
esac

FAILED=0

# ---------------------------------------------------------------------------
# Step 1: native logic test (host compiler, actually runs)
# ---------------------------------------------------------------------------
if [ "$RUN_NATIVE" -eq 1 ]; then
	echo "== Step 1: native allocator logic test ($CC) =="
	for HEAP_DEF in "-DXAIE_FUSA_HEAP_SIZE=8192" ""; do
		SLUG="${HEAP_DEF:+8192}"
		SLUG="${SLUG:-default}"
		BIN="$WORK/native_test_$SLUG"
		BUILD_LOG="$WORK/native_build_$SLUG.log"
		RUN_LOG="$WORK/native_run_$SLUG.log"
		if $CC -std=c11 -Wall -Wextra -Werror -fsanitize=address,undefined \
			-DXAIE_FEATURE_NO_LIBC_MALLOC $HEAP_DEF \
			-I"$DRIVER_SRC/common" -I"$DRIVER_SRC/global" \
			"$UTEST_DIR/test_xaie_mem_alloc.c" \
			"$DRIVER_SRC/common/xaie_mem_alloc.c" \
			-o "$BIN" 2>"$BUILD_LOG"
		then
			if "$BIN" >"$RUN_LOG" 2>&1; then
				echo "  PASS  ${HEAP_DEF:-<default heap size>}" \
					"  ($(tail -n1 "$RUN_LOG"))"
			else
				echo "  FAIL (runtime)  ${HEAP_DEF:-<default heap size>}"
				cat "$RUN_LOG"
				FAILED=1
			fi
		else
			echo "  FAIL (compile)  ${HEAP_DEF:-<default heap size>}"
			cat "$BUILD_LOG"
			FAILED=1
		fi
	done
	echo
fi

# ---------------------------------------------------------------------------
# Step 2: armclang cross-compile check (Cortex-R52, compile-only)
# ---------------------------------------------------------------------------
if [ "$RUN_CROSS" -eq 1 ]; then
	echo "== Step 2: armclang cross-compile check (Cortex-R52) =="

	[ -n "$ARMCLANG_BIN_DIR" ] && export PATH="$ARMCLANG_BIN_DIR:$PATH"
	[ -n "$ARMLMD_LICENSE_FILE" ] && export ARMLMD_LICENSE_FILE
	[ -n "$ARM_TOOL_VARIANT" ] && export ARM_TOOL_VARIANT

	if ! command -v "$ARMCLANG" >/dev/null 2>&1; then
		if [ "$REQUIRE_ARMCLANG" -eq 1 ]; then
			echo "  FAIL: '$ARMCLANG' not found on PATH."
			echo "        Set ARMCLANG_BIN_DIR (and, if needed,"
			echo "        ARMLMD_LICENSE_FILE / ARM_TOOL_VARIANT) and re-run,"
			echo "        or set REQUIRE_ARMCLANG=0 / pass --native-only" \
				"to skip this instead of failing."
			exit 1
		fi
		echo "  SKIP: '$ARMCLANG' not found on PATH."
		echo "        Set ARMCLANG_BIN_DIR (and, if needed,"
		echo "        ARMLMD_LICENSE_FILE / ARM_TOOL_VARIANT) and re-run,"
		echo "        or pass --native-only to silence this."
	else
		# Files touched by the FuSa libc work (Groups A/B/C); see the
		# header comment for why xaie_baremetal.c is excluded.
		FILES="
			common/xaie_mem_alloc.c
			common/xaie_helper.c
			util/btree4.c
			core/xaie_elfloader.c
			core/xaie_uc.c
			global/xaiegbl.c
			routing/xaie_routing.c
			timer/xaie_timer.c
		"
		# No blanket -Werror: this checks that the FuSa opt-outs
		# compile, not that every pre-existing file is warning-clean
		# under armclang's stricter set (e.g. btree4.c's redundant
		# parens, unrelated to FuSa).
		#
		# -Werror=implicit-function-declaration IS enforced: since
		# this step never links, calling a function the headers don't
		# declare (e.g. free()/realloc(), missing from the real FuSa
		# <stdlib.h>) would otherwise only warn, not FAIL.
		COMMON_FLAGS="--target=arm-arm-none-eabi -mcpu=cortex-r52 \
			-mfloat-abi=hard -ffp-mode=full -Wall -Wextra \
			-Werror=implicit-function-declaration \
			-D__AIEBAREMETAL__ \
			-I$DRIVER_SRC/common -I$DRIVER_SRC/global \
			-I$DRIVER_SRC/core -I$DRIVER_SRC/util \
			-I$DRIVER_SRC/routing -I$DRIVER_SRC/timer \
			-I$DRIVER_SRC/memory -I$DRIVER_SRC/pm \
			-I$DRIVER_SRC/io_backend -I$DRIVER_SRC/stream_switch \
			-I$DRIVER_SRC/pl -I$DRIVER_SRC/events -I$DRIVER_SRC/dma \
			-I$DRIVER_SRC/lite -I$DRIVER_SRC/locks -I$DRIVER_SRC/debug"

		# FuSa-mode prefers the real FuSa headers (-I order makes them
		# win over shim/ for anything both provide, e.g. errno.h),
		# falling back to shim/ if unavailable; shim/ is always added
		# after for elf.h, which no C library provides. Default-mode
		# always uses shim/ only -- see header comment for why.
		if [ -d "$FUSA_LIBC_INCLUDE_DIR" ]; then
			FUSA_INC_FLAGS="-I$FUSA_LIBC_INCLUDE_DIR -I$SHIM_DIR"
			echo "  Using real FuSa libc headers: $FUSA_LIBC_INCLUDE_DIR"
		else
			FUSA_INC_FLAGS="-I$SHIM_DIR"
			echo "  FUSA_LIBC_INCLUDE_DIR not found ($FUSA_LIBC_INCLUDE_DIR)," \
				"falling back to shim/ for the FuSa-mode compile too."
		fi
		DEFAULT_INC_FLAGS="-I$SHIM_DIR"

		for f in $FILES; do
			BASE="$(basename "$f" .c)"
			OBJ="$WORK/$BASE.o"
			for MODE in fusa default; do
				if [ "$MODE" = "fusa" ]; then
					MODE_FLAGS="-DXAIE_FEATURE_FUSA_LIBC"
					INC_FLAGS="$FUSA_INC_FLAGS"
				else
					MODE_FLAGS=""
					INC_FLAGS="$DEFAULT_INC_FLAGS"
				fi
				BUILD_LOG="$WORK/cross_${BASE}_$MODE.log"
				if $ARMCLANG $COMMON_FLAGS $INC_FLAGS $MODE_FLAGS \
					-c "$DRIVER_SRC/$f" -o "$OBJ" \
					2>"$BUILD_LOG"
				then
					printf "  PASS  %-8s  %s\n" "$MODE" "$f"
				else
					printf "  FAIL  %-8s  %s\n" "$MODE" "$f"
					cat "$BUILD_LOG"
					FAILED=1
				fi
			done
		done
	fi
	echo
fi

if [ "$FAILED" -eq 0 ]; then
	echo "All checks passed."
else
	echo "One or more checks FAILED."
fi
exit "$FAILED"
