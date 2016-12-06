# ART Stress framework

ART Stress framework is used to execute tests with additional parallel stress operations of ART components.
 
## Launcher usage:        

java -Xms2g -Xmx2g -cp <path to stresser classes>:<test class path> 
	Launcher <Test class name> <Test class entry name> [<iterations>]

stress.param file should be located in the launch directory to specify the type of the stresser

Contents of stress.param:

ArchiveStresser [stress_factor]

or

ClassLoaderStresser [stress_factor]

or

MemoryStresser [stress_factor]  [<chainLength>]

or 

SimpleStresser [stress_factor]

or

IOStresser [stress_factor]

where stress_factor is the number of stress threads in the agent. 
By default stress_factor is the number of host processors.

If there is no stress.param file in the launch directory, default stresser is selected.

## Android

Before building and running tests make sure that you have a valid path to JAVA_HOME and to ANDROID_SDK_ROOT
```bash
$ export JAVA_HOME=<PATH TO JAVA_HOME>
$ export PATH=$JAVA_HOME/bin:$PATH
$ export ANDROID_SDK_ROOT=<PATH TO ANDROID_SDK_ROOT>
$ mkdir classes
$  javac -d classes -cp $ANDROID_SDK_ROOT/platforms/android-22/android.jar src/*.java 
$  dx --dex --debug --verbose --output=StressRunner.jar classes
```
or use build script with corrected paths.

```bash
$  adb push StressRunner.jar /data
$  adb push stress.param /data
$  adb shell "cd /data; dalvikvm -Xmx512m -cp StressRunner.jar:HelloWorld.jar  Launcher HelloWorld main 10"
```

After  [<iterations>] you can list arguments for the stressed class:

```bash
$  adb shell "cd /data; dalvikvm -Xmx512m -cp  JUCrunClasses.jar:StressRunner.jar:commons-math.jar Launcher JUCrunClasses main 1 org.apache.commons.math.ConvergenceExceptionTest org.apache.commons.math.MathExceptionTest"
```

In this case <iterations> is required. Put 1 if you don't need iterative runs.

In case the number of arguments of stressed class is greater than 12 (not sure about the exact number) android shell will throw an error.
Put the arguments in arguments.list file in one column and push the file onto device:

```bash
$ adb shell arguments.list /data
```

Arguments in the command line have the precedence over those in the arguments.list

## Archivestresser notes:

```bash
$ adb push /export/users/qa/tools/java-1.6.0-openjdk-1.6.0.0/jre/lib/jce.jar /data
```

ArchiveStresser can work with regular jar files only

Android jars like StressRunner.jar, /system/framework/core.jar cause

java.util.zip.ZipException: no Entry

jars like jsse.jar, rt.jar seem to be too large for this stresser:

```bash
java.util.zip.ZipException: CRC mismatch
        at java.util.zip.ZipOutputStream.closeEntry(ZipOutputStream.java:145)
        at java.util.zip.ZipOutputStream.finish(ZipOutputStream.java:234)
        at java.util.zip.ZipOutputStream.close(ZipOutputStream.java:119)
        at ArchiveStresser$Run.run(ArchiveStresser.java:127)
```

## To run the test on JDK:

```bash
java -Xms2g -Xmx2g -cp /nfs/ins/proj/slt/qa/users/tdoubtso/Android/repro/StressRunner_DUT/StressRunner/classes:. Launcher Test main
```
