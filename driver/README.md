# AI Engine Driver

The userspace library for ai-engine provides APIs to configure AIE registers.

## Hardware support

The user space library is supports both AIE and AIEML

| BRANCH     	| DEVICE         	|
|------------	|----------------	|
| master     	| AIE and AIE ML 	|
| master-aie 	| AIE only       	|

## IO Backends

The userspace library can be compiled with various IO backends. The library
can execute the low level register IO operations for the following backends:
1. Linux Kernel(-D__AIELINUX__): IO operations are executed by linux kernel.
2. Simulation (-D__AIESIM__): IO operation are executed by simulation functions
			      provided by aie-tools.
3. CDO generator(-D__AIECDO): IO operations are executed by cdo functions
			      provided by aie-tools.
4. Baremetal(-D_AIEBAREMETAL__): IO operations are executed by baremetal
				 functions.
5. Libmetal (-D__AIEMETAL__): IO operations are executed by libmetal library.
			      Libmetal backend uses UIO based kernel driver.
			      For linux platforms, AIEML device is supported
			      with libmetal backend only.
6. Debug: This is the default backend when other flags are not passed. The
	  debug backend prints the register address and corresponding values to
	  stdout.

## Compilation
### Compile library
	make -f Makefile.Linux
### Generate Documentation
	make -f Makefile.Linux doc-generate

HTML docs are populated in ./tmp/api/

PDF doc will be available at ./tmp/latex/refman.pdf
