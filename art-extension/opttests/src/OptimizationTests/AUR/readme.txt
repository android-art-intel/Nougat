README

This test package for Aggressive Use Remover optimization pass (AUR) includes tests for all instructions that are supported by AUR:
Positive:  BoundsCheck, DivZeroCheck, NullCheck, Throw, Suspend, SuspendCheck, LoadClass, ClinitCheck, InstanceOf, CheckCast, MonitorOperation, NewArray, NewInstance, ArraySet, Invokes (StaticOrDirect, Virtual, Interface)
Negative: Deoptimize (Such nodes are always explicitly not dealt with in AUR); test with -Xcompiler-option --debuggable option.

Post-processing notes: when a new test is added, generate_postproc.sh should be run in results directory (where logcat_runTest is located, it will generate expected AUR messages template, it should be copied to postproc script.

Each test case is run in two modes: simple + stress GC mode: additional threads stressing GC are launched (see "shared/StressGC.java"). To disable stressGC mode: rename 
 // wrapper method for runTest with GC stress threads running
 public String testWithGCStress(int n) {
to
 // wrapper method for runTest with GC stress threads running
 public String __testWithGCStress(int n) {
then it will not be launched. You will also need to remove corresponding output from expected.txt.


The following test scenarios mentioned in QA document are NOT covered:

1. Debuggability is not broken for debuggable apps: relying on other test suites focused on debuggability
2. "implement Self-Verification (several month long project) for compiler by allowing executing with interpreter, reverting state, then running through compiled code to same point to check whether the execution is same (both heap and the stack states)."
3. debuggability is broken due to stack state being incorrect by this patch by testing the following scenarios:
    * Deoptimize for debugger at suspend point
    * Deoptimize for debugger at throwing null pointer exception




