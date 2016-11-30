README

* This test package includes custom build and run scripts because it deals with native libs. Please edit "build" script to specify JAVA_HOME, ANDROID_NDK_HOME and other tools paths
* Tests should be run in JIT mode with post-processing enabled
* By default native tests are built from ASM code: .s files. If you modify JniTest.cpp, you have to run "BUILD_TARGET=CppToAsm VM_RUNTIME=ART ./build PACKAGELIST=AutoFastJNIDetection" first, then check that generated .s files look expected, then run "VM_RUNTIME=ART ./build PACKAGELIST=AutoFastJNIDetection"
* This test package includes tests with JNI calls that check AutoFastJNIDetection optimization: if conditions are met, method is marked as fast JNI. Main limitations are:
    1. < 200 assembly instructions
    2. <=3 nested calls depth
    3. <= 20 basic blocks
    4. no loops
    5. no atomics/mutexes
    6. no interrupts
    7. no indirect jumps
    8. no indirect calls


