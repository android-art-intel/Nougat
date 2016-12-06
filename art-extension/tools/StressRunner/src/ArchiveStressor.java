/*
 * Copyright (C) 2015 Intel Corporation.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.util.jar.JarEntry;
import java.util.jar.JarInputStream;
import java.util.jar.JarOutputStream;

public class ArchiveStressor  extends BaseStressor {
    int threadCnt = processorsNmbr * 4;
    boolean append = false;

    public static void main(String[] args) {
        ArchiveStressor as = new ArchiveStressor();
        Run obj = as.new Run(1);
        obj.start();
        try {
            Thread.sleep(200);
            obj.stopIt = 1;
            obj.join();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }
    /*
     * Note, this method called through the Method.invoke() and should return to
     * the main thread ASAP
     */
    public void run(String[] args) {
        if (args.length > 0) {
            try {
                threadCnt = Integer.parseInt(args[0]);
            } catch (Exception e) {
                // just use default value
            }
        }
        if (args.length > 1) {
            try {
                append = Boolean.parseBoolean(args[1]);
            } catch (Exception e) {
                // just use default value
            }
        }
        new DoNothing().start();
    }

    class DoNothing extends Thread {
        public void run() {
            Run[] threads = new Run[threadCnt];
            for (int i = 0; i < threads.length; i++) {
                threads[i] = new Run(i);
                threads[i].start();
            }
             while (!stopStressor) {
                for (int i = 1; i < threads.length; i++)
                    threads[i - 1].i = threads[i].i;
                Thread.yield();
            }
            for (int i = 0; i < threads.length; i++) {
                threads[i].stopIt = 1;
            }
        }
    }

    class Run extends Thread {
        volatile int  stopIt = 0;
        volatile long i      = 0;
        public int    id     = 0;

        public Run(int id) {
            this.id = id;
        }

        public void run() {

/* Before using ArchiveStressor you should push jar file to Android device:
   $ adb push /export/users/qa/tools/java-1.6.0-openjdk-1.6.0.0/jre/lib/jce.jar /data
   ArchiveStressor can work with regular jar files only
   Android jars like StressRunner.jar, /system/framework/core.jar cause
   java.util.zip.ZipException: no Entry
   jars like jsse.jar, rt.jar seem to be too large for this stressor:
   java.util.zip.ZipException: CRC mismatch
        at java.util.zip.ZipOutputStream.closeEntry(ZipOutputStream.java:145)
        at java.util.zip.ZipOutputStream.finish(ZipOutputStream.java:234)
        at java.util.zip.ZipOutputStream.close(ZipOutputStream.java:119)
        at ArchiveStressor$Run.run(ArchiveStressor.java:127)
*/
            String inFileName = "/data/jce.jar";
            String outFileName = "/data/ArchiveStressor.jar." + id;

            try {
                while (stopIt != 1) {
                    JarInputStream in = null;
                    JarOutputStream out = null;
     
                try {
                        in = new JarInputStream(new FileInputStream(
                            inFileName));
                        out = new JarOutputStream(new FileOutputStream(
                            outFileName, append));
                        JarEntry entry;
                        while (true) {
                            entry = in.getNextJarEntry();
                            if (entry == null)
                                break;
                            if (entry.getSize() != -1) {
                                byte[] buff = new byte[(int)entry.getSize()];
                                in.read(buff);
                                out.putNextEntry(entry);
                                out.write(buff);
                                out.flush();
                            }
                        }
                    } finally {
                        if (in != null)
                            in.close();
                        if (out != null)
                            out.close();
                    }
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }
}
