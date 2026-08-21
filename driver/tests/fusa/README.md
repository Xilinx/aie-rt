# FuSa libc build verification

`verify_fusa_build.sh` checks the driver's FuSa opt-out paths
(`XAIE_FEATURE_NO_LIBC_MALLOC`, `XAIE_FEATURE_NO_FILESYSTEM`,
`XAIE_FEATURE_NO_STDIO_LOGGING`, or the `XAIE_FEATURE_FUSA_LIBC` umbrella
flag -- see `driver/src/global/xaie_feature_config.h`) actually compile,
and that the custom allocator's logic is correct.

## What it does

1. **Native logic test** (always runs, no special toolchain needed): builds
   and runs `driver/tests/utest/test_xaie_mem_alloc.c` against the real
   `xaie_mem_alloc.c` with the host C compiler, at two heap sizes. This is
   the only step that actually executes anything.
2. **armclang cross-compile check** (Cortex-R52, compile-only): compiles
   the driver files touched by the FuSa work with `armclang`, once with
   the FuSa opt-outs enabled and once without, and confirms both compile
   clean. No linking or execution -- there's no R52 target/simulator here.
   Requires access to an Arm Compiler for Embedded install; by default the
   step **fails** (not skips) if `armclang` can't be found, so a
   misconfigured environment can't silently report success without ever
   compiling anything -- see `REQUIRE_ARMCLANG` below.
   The FuSa-enabled half of this compiles against the **real** Arm
   Certified FuSa C library headers when available (see
   `FUSA_LIBC_INCLUDE_DIR` below), not just a hand-written stand-in.

`io_backend/ext/xaie_baremetal.c` is intentionally excluded from step 2:
it needs real Xilinx BSP headers (`xstatus.h`, `xpm_defs.h`, `xil_io.h`,
...) to compile at all, which is a BSP/toolchain concern unrelated to FuSa
libc. Verify it as part of a real BSP build instead.

## Usage

```bash
./verify_fusa_build.sh                # both steps
./verify_fusa_build.sh --native-only   # skip armclang entirely
./verify_fusa_build.sh --cross-only    # skip the native test
```

To run the cross-compile step, point it at an Arm Compiler for Embedded
install, e.g.:

```bash
export ARMCLANG_BIN_DIR=/tools/packages/arm/safety/armcc/6.16/bin
export ARMLMD_LICENSE_FILE=8224@licsrv07.amd.com   # your license server
export ARM_TOOL_VARIANT=ult
./verify_fusa_build.sh
```

`REQUIRE_ARMCLANG` defaults to `1`: the cross-compile step fails if
`armclang` isn't found on `PATH` after the `export`s above. Set
`REQUIRE_ARMCLANG=0` if you want it to just skip with a warning instead
(e.g. on a host with no Arm toolchain at all) -- or pass `--native-only`,
which skips step 2 entirely regardless of `REQUIRE_ARMCLANG`.

Every run prints a `Logs: /tmp/fusa_verify.XXXXXX` line on exit. That
directory has one build/run log per native heap-size and per
cross-compile file/mode (e.g. `cross_xaie_helper_fusa.log`), and is kept
around afterward -- even on a `PASS` it's worth checking, since step 2
doesn't use `-Werror` and so can pass with warnings still logged. Nothing
is auto-deleted; clean up old runs yourself with `rm -rf
/tmp/fusa_verify.*`.

## Real FuSa library headers vs. `shim/`

The FuSa-mode half of the cross-compile check looks for a real Arm
Certified FuSa C library install and, if found, compiles against its
actual headers instead of a hand-written stand-in:

```bash
export FUSA_LIBC_INCLUDE_DIR=/path/to/fusa_libc/include
./verify_fusa_build.sh
```

It defaults to the AMD-internal install path
(`/proj/xsjsswstaff/mvanukur/FuSa_Libc/fusa_libc/include`); if that
doesn't exist (e.g. you're not on the same filesystem) it silently falls
back to `shim/errno.h` so the script still runs. `shim/errno.h` mirrors
the real library's `<errno.h>` (`EDOM`/`ERANGE` only, values `1`/`2` as
Arm defines them) for parity with that fallback case.

`shim/elf.h` is used unconditionally, real headers or not: no C
library -- FuSa or otherwise -- ships an ELF header, since ELF loading
is unrelated to the C library itself. In a real integration this is a
BSP/toolchain header (e.g. embeddedsw's Xilinx BSP, or a Linux
userspace build's system `<elf.h>`), not something FuSa or aie-rt
supplies; `xaie_elfloader.h`'s plain `#include <elf.h>` already assumed
this before any of the FuSa work. `shim/elf.h` here is just a
self-contained stand-in for that BSP/toolchain header so this isolated
script doesn't need one installed.

The default-mode (non-FuSa) half of the compile always uses `shim/`
only, never the real FuSa headers -- it exercises the hosted code paths
(`fprintf`, `strerror`, hosted `errno`, ...), which need a full C
library that the restricted FuSa library deliberately isn't.
