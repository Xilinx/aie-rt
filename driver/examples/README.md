//****************************************************************//
// Steps to build reserialize test case and run it locally        //
//****************************************************************//

Build AIE-RT
------------
cd <WORKPSACE_ROOT>
cd driver/src/
make -f Makefile.Linux -j8

Build Test case
---------------
cd ../examples/
cc -Wall -Wextra --std=c11 -I../include -I../internal/xaiengine -L../src -o xaie_txn_reserialize_test xaie_txn_reserialize_test.c -lxaiengine -Wl,-rpath,../src

Run test case with sample txn binaries
--------------------------------------
./xaie_txn_reserialize_test <INPUT_TXN> <OUT_TXN>
